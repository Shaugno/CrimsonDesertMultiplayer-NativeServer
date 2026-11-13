#include "pkt3663/assess.h"

#include <cstring>
#include <optional>
#include <string>

namespace pkt3663 {
namespace {

constexpr uint32_t kMz = 0x5A4D;
constexpr uint32_t kPe = 0x00004550;
constexpr uint16_t kPe32Plus = 0x020B;
constexpr uint32_t kSecExecute = 0x20000000;
constexpr uint32_t kSecWrite = 0x80000000;
constexpr uint32_t kMaxELfanew = 1024;
constexpr size_t kMaxAllocStored = 64;
constexpr size_t kMaxFormerStored = 8;

const char* kMessageKeys[] = {
    "MoveActorReq",
    "MoveActorAck",
    "RotateActorReq",
    "RotateActorAck",
    "StartActionReq",
    "StartActionAck",
    "ActorLoadDataAck",
    "UpdateVisiblePlayerAck",
    "RemoveActorAck",
};

struct BytePat {
    const char* name;
    const uint8_t* bytes;
    size_t size;
};

const uint8_t kMovEcx[] = {0xB9, 0x30, 0x80, 0x00, 0x00};
const uint8_t kMovEdx[] = {0xBA, 0x30, 0x80, 0x00, 0x00};
const uint8_t kMovR8d[] = {0x41, 0xB8, 0x30, 0x80, 0x00, 0x00};
const uint8_t kMovR9d[] = {0x41, 0xB9, 0x30, 0x80, 0x00, 0x00};

// Samples from the 1.0.0.3576 candidate note. They are searched as bytes.
// No RVA from that note is stored here, and a hit is not an entry candidate.
const uint8_t kFormerSendSample[] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41,
};
const uint8_t kFormerRecvSample[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x18, 0x55,
};

uint16_t load_u16(const uint8_t* p) {
    uint16_t value = 0;
    std::memcpy(&value, p, sizeof value);
    return value;
}

uint32_t load_u32(const uint8_t* p) {
    uint32_t value = 0;
    std::memcpy(&value, p, sizeof value);
    return value;
}

uint64_t load_u64(const uint8_t* p) {
    uint64_t value = 0;
    std::memcpy(&value, p, sizeof value);
    return value;
}

int32_t load_i32(const uint8_t* p) {
    return static_cast<int32_t>(load_u32(p));
}

bool range_ok(size_t size, size_t offset, size_t count) {
    return count <= size && offset <= size - count;
}

struct Slice {
    const uint8_t* data = nullptr;
    size_t size = 0;
    uint32_t va = 0;
    bool exec = false;
    bool writable = false;
};

std::optional<size_t> rva_to_off(const ImageView& image, uint32_t rva) {
    for (const Section& section : image.sections) {
        if (section.raw_size == 0) continue;
        if (rva < section.virtual_address) continue;
        const uint32_t delta = rva - section.virtual_address;
        if (delta >= section.raw_size) continue;
        const size_t off = static_cast<size_t>(section.raw_ptr) + delta;
        if (off >= image.size) return std::nullopt;
        return off;
    }
    if (image.size_of_headers > 0 && rva < image.size_of_headers && rva < image.size) return rva;
    return std::nullopt;
}

bool read_rva(const ImageView& image, uint32_t rva, void* dest, size_t count) {
    const std::optional<size_t> off = rva_to_off(image, rva);
    if (!off || !range_ok(image.size, *off, count)) return false;
    std::memcpy(dest, image.data + *off, count);
    return true;
}

bool exec_rva(const ImageView& image, uint32_t rva) {
    for (const Section& section : image.sections) {
        if ((section.flags & kSecExecute) == 0 || section.virtual_size == 0) continue;
        if (rva >= section.virtual_address && rva - section.virtual_address < section.virtual_size) return true;
    }
    return false;
}

std::vector<Slice> slices_of(const ImageView& image) {
    std::vector<Slice> slices;
    for (const Section& section : image.sections) {
        if (section.raw_size == 0) continue;
        if (section.raw_ptr >= image.size) continue;
        size_t size = section.raw_size;
        if (size > image.size - section.raw_ptr) size = image.size - section.raw_ptr;
        Slice slice;
        slice.data = image.data + section.raw_ptr;
        slice.size = size;
        slice.va = section.virtual_address;
        slice.exec = (section.flags & kSecExecute) != 0;
        slice.writable = (section.flags & kSecWrite) != 0;
        slices.push_back(slice);
    }
    return slices;
}

const uint8_t* find_bytes(const uint8_t* begin, const uint8_t* end, const uint8_t* pat, size_t n) {
    if (n == 0 || begin == nullptr || end < begin || static_cast<size_t>(end - begin) < n) return nullptr;
    const uint8_t* cur = begin;
    const uint8_t* last = end - n;
    while (cur <= last) {
        const void* hit = std::memchr(cur, pat[0], static_cast<size_t>(last - cur) + 1);
        if (hit == nullptr) return nullptr;
        cur = static_cast<const uint8_t*>(hit);
        if (std::memcmp(cur, pat, n) == 0) return cur;
        ++cur;
    }
    return nullptr;
}

bool rip_target(uint32_t insn_rva, int32_t rel, uint32_t insn_len, uint32_t& out) {
    const int64_t target = static_cast<int64_t>(insn_rva) + insn_len + rel;
    if (target < 0 || target > 0xFFFFFFFFLL) return false;
    out = static_cast<uint32_t>(target);
    return true;
}

bool enclosing_type_name(const uint8_t* data, size_t size, size_t key_off, size_t key_len, size_t& name_off,
                         size_t& name_len) {
    if (key_off > size || key_len > size - key_off) return false;
    size_t begin = key_off;
    while (begin > 0) {
        const uint8_t c = data[begin - 1];
        if (c < 0x21 || c > 0x7E) break;
        --begin;
        if (key_off - begin > 4000) return false;
    }
    size_t end = key_off + key_len;
    while (end < size && data[end] != 0) {
        const uint8_t c = data[end];
        if (c < 0x21 || c > 0x7E) return false;
        ++end;
        if (end - begin > 4000) return false;
    }
    if (end >= size || data[end] != 0) return false;
    if (end - begin < 4 || data[begin] != '.' || data[begin + 1] != '?' || data[begin + 2] != 'A') return false;
    name_off = begin;
    name_len = end - begin;
    return true;
}

struct FunctionRange {
    uint32_t begin = 0;
    uint32_t end = 0;
};

std::vector<FunctionRange> exception_functions(const ImageView& image) {
    std::vector<FunctionRange> functions;
    if (image.exception_size < 12 || image.exception_size / 12 > 1000000) return functions;
    const uint32_t count = image.exception_size / 12;
    functions.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint8_t row[12]{};
        if (!read_rva(image, image.exception_rva + i * 12, row, sizeof row)) return {};
        FunctionRange fn;
        fn.begin = load_u32(row);
        fn.end = load_u32(row + 4);
        if (fn.end < fn.begin) return {};
        functions.push_back(fn);
    }
    return functions;
}

bool function_contains(const std::vector<FunctionRange>& functions, uint32_t rva, FunctionRange& hit) {
    size_t lo = 0;
    size_t hi = functions.size();
    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (functions[mid].begin <= rva) lo = mid + 1;
        else hi = mid;
    }
    if (lo == 0) return false;
    const FunctionRange& fn = functions[lo - 1];
    if (rva < fn.begin || rva >= fn.end) return false;
    hit = fn;
    return true;
}

bool read_slot(const ImageView& image, uint32_t vtable_rva, uint32_t index, uint32_t& rva) {
    uint64_t slot = 0;
    if (!read_rva(image, vtable_rva + index * 8, &slot, sizeof slot)) return false;
    if (slot < image.image_base) return false;
    const uint64_t delta = slot - image.image_base;
    if (delta > 0xFFFFFFFFULL) return false;
    const uint32_t slot_rva = static_cast<uint32_t>(delta);
    if (!exec_rva(image, slot_rva)) return false;
    rva = slot_rva;
    return true;
}

VtableRead read_vtable(const ImageView& image, uint32_t rva) {
    VtableRead vt;
    vt.rva = rva;
    vt.slot0 = read_slot(image, rva, 0, vt.slot0_rva);
    vt.slot1 = read_slot(image, rva, 1, vt.slot1_rva);
    vt.slot2 = read_slot(image, rva, 2, vt.slot2_rva);
    return vt;
}

bool read_instance(const ImageView& image, uint32_t rva, InstanceRead& instance) {
    uint8_t raw[0x1C]{};
    if (!read_rva(image, rva, raw, sizeof raw)) return false;
    instance.rva = rva;
    instance.first_qword = load_u64(raw);
    instance.kind_u32 = load_u32(raw + 0x08);
    instance.opcode_u16 = load_u16(raw + 0x0C);
    instance.flag_u32 = load_u32(raw + 0x14);
    instance.min_u16 = load_u16(raw + 0x18);
    instance.max_u16 = load_u16(raw + 0x1A);
    return true;
}

bool same_name(const ClassChain& chain, const std::string& mangled) { return chain.mangled == mangled; }

bool exact_type_name(const char* key, const std::string& mangled) {
    const std::string plain = std::string(".?AV") + key + "@@";
    const std::string troc = std::string(".?AVTrocTr") + key + "@pa@@";
    return mangled == plain || mangled == troc;
}

void scan_messages(const ImageView& image, const std::vector<Slice>& slices, std::vector<MessageHit>& messages) {
    messages.clear();
    messages.reserve(std::size(kMessageKeys));
    for (const char* key : kMessageKeys) {
        MessageHit hit;
        hit.key = key;
        const size_t key_len = std::strlen(key);
        for (const Slice& slice : slices) {
            if (slice.exec && !slice.writable) continue;
            const uint8_t* begin = slice.data;
            const uint8_t* end = slice.data + slice.size;
            const uint8_t* cur = begin;
            while ((cur = find_bytes(cur, end, reinterpret_cast<const uint8_t*>(key), key_len)) != nullptr) {
                const size_t key_off = static_cast<size_t>(cur - begin);
                size_t name_off = 0;
                size_t name_len = 0;
                if (enclosing_type_name(slice.data, slice.size, key_off, key_len, name_off, name_len) && name_off >= 0x10) {
                    const std::string mangled(reinterpret_cast<const char*>(slice.data + name_off), name_len);
                    bool seen = false;
                    for (const ClassChain& chain : hit.classes) seen = seen || same_name(chain, mangled);
                    if (!seen) {
                        ClassChain chain;
                        chain.exact_name = exact_type_name(key, mangled);
                        chain.mangled = mangled;
                        chain.type_descriptor_rva = slice.va + static_cast<uint32_t>(name_off - 0x10);
                        hit.classes.push_back(std::move(chain));
                    }
                }
                cur += key_len;
            }
        }
        hit.found = !hit.classes.empty();
        messages.push_back(std::move(hit));
    }

    for (const Slice& slice : slices) {
        if (slice.exec && !slice.writable) continue;
        const uint8_t sig[] = {0x01, 0x00, 0x00, 0x00};
        const uint8_t* begin = slice.data;
        const uint8_t* end = slice.data + slice.size;
        const uint8_t* cur = begin;
        while ((cur = find_bytes(cur, end, sig, sizeof sig)) != nullptr) {
            const size_t pos = static_cast<size_t>(cur - begin);
            const uint32_t col_rva = slice.va + static_cast<uint32_t>(pos);
            if ((col_rva % 4) == 0 && pos + 24 <= slice.size) {
                const uint32_t self = load_u32(slice.data + pos + 20);
                const uint32_t td = load_u32(slice.data + pos + 12);
                if (self == col_rva) {
                    for (MessageHit& hit : messages) {
                        for (ClassChain& chain : hit.classes) {
                            if (td == chain.type_descriptor_rva) chain.col_rvas.push_back(col_rva);
                        }
                    }
                }
            }
            ++cur;
        }
    }

    for (MessageHit& hit : messages) {
        for (ClassChain& chain : hit.classes) {
            for (uint32_t col_rva : chain.col_rvas) {
                uint8_t wanted[8]{};
                const uint64_t va = image.image_base + col_rva;
                std::memcpy(wanted, &va, sizeof wanted);
                for (const Slice& slice : slices) {
                    if (slice.exec && !slice.writable) continue;
                    const uint8_t* begin = slice.data;
                    const uint8_t* end = slice.data + slice.size;
                    const uint8_t* cur = begin;
                    while ((cur = find_bytes(cur, end, wanted, sizeof wanted)) != nullptr) {
                        const size_t pos = static_cast<size_t>(cur - begin);
                        const uint32_t ptr_rva = slice.va + static_cast<uint32_t>(pos);
                        if ((ptr_rva % 8) == 0) chain.vtables.push_back(read_vtable(image, ptr_rva + 8));
                        cur += sizeof wanted;
                    }
                }
            }
        }
    }

    struct LeaStore {
        uint8_t lea0, lea1, lea2;
        uint8_t store0, store1, store2;
    };
    const LeaStore kPairs[] = {
        {0x48, 0x8D, 0x05, 0x48, 0x89, 0x05},
        {0x48, 0x8D, 0x0D, 0x48, 0x89, 0x0D},
        {0x48, 0x8D, 0x15, 0x48, 0x89, 0x15},
    };

    for (const Slice& slice : slices) {
        if (!slice.exec || slice.size < 14) continue;
        for (size_t i = 0; i + 7 <= slice.size; ++i) {
            const LeaStore* pair = nullptr;
            for (const LeaStore& item : kPairs) {
                if (slice.data[i] == item.lea0 && slice.data[i + 1] == item.lea1 && slice.data[i + 2] == item.lea2) {
                    pair = &item;
                    break;
                }
            }
            if (pair == nullptr) continue;
            uint32_t vtable_rva = 0;
            if (!rip_target(slice.va + static_cast<uint32_t>(i), load_i32(slice.data + i + 3), 7, vtable_rva)) continue;
            ClassChain* owner = nullptr;
            for (MessageHit& hit : messages) {
                for (ClassChain& chain : hit.classes) {
                    for (const VtableRead& vt : chain.vtables) {
                        if (vt.rva == vtable_rva) owner = &chain;
                    }
                }
            }
            if (owner == nullptr) continue;
            const size_t window = i + 7;
            const size_t window_end = window + 32 < slice.size ? window + 32 : slice.size;
            for (size_t j = window; j + 7 <= window_end; ++j) {
                if (slice.data[j] != pair->store0 || slice.data[j + 1] != pair->store1 || slice.data[j + 2] != pair->store2) {
                    continue;
                }
                uint32_t instance_rva = 0;
                if (!rip_target(slice.va + static_cast<uint32_t>(j), load_i32(slice.data + j + 3), 7, instance_rva)) continue;
                bool seen = false;
                for (const InstanceRead& existing : owner->instances) {
                    if (existing.rva == instance_rva) seen = true;
                }
                InstanceRead instance;
                if (!seen && read_instance(image, instance_rva, instance)) owner->instances.push_back(instance);
                break;
            }
        }
    }
}

void scan_alloc(const std::vector<Slice>& slices, const std::vector<FunctionRange>& functions, Assessment& out) {
    const BytePat pats[] = {
        {"mov_ecx", kMovEcx, sizeof kMovEcx},
        {"mov_edx", kMovEdx, sizeof kMovEdx},
        {"mov_r8d", kMovR8d, sizeof kMovR8d},
        {"mov_r9d", kMovR9d, sizeof kMovR9d},
    };
    for (const Slice& slice : slices) {
        if (!slice.exec) continue;
        for (const BytePat& pat : pats) {
            const uint8_t* begin = slice.data;
            const uint8_t* end = slice.data + slice.size;
            const uint8_t* cur = begin;
            while ((cur = find_bytes(cur, end, pat.bytes, pat.size)) != nullptr) {
                const size_t pos = static_cast<size_t>(cur - begin);
                ++out.alloc_site_count;
                if (out.alloc_sites.size() < kMaxAllocStored) {
                    AllocSite site;
                    site.rva = slice.va + static_cast<uint32_t>(pos);
                    site.encoding = pat.name;
                    FunctionRange fn;
                    if (function_contains(functions, site.rva, fn)) {
                        site.in_function = true;
                        site.function_begin = fn.begin;
                        site.function_end = fn.end;
                    }
                    out.alloc_sites.push_back(site);
                } else {
                    out.alloc_sites_truncated = true;
                }
                cur += pat.size;
            }
        }
    }
}

void scan_former_samples(const std::vector<Slice>& slices, Assessment& out) {
    const BytePat pats[] = {
        {"former_3576_send_sample", kFormerSendSample, sizeof kFormerSendSample},
        {"former_3576_recv_sample", kFormerRecvSample, sizeof kFormerRecvSample},
    };
    for (const Slice& slice : slices) {
        if (!slice.exec) continue;
        for (const BytePat& pat : pats) {
            const uint8_t* begin = slice.data;
            const uint8_t* end = slice.data + slice.size;
            const uint8_t* cur = begin;
            while ((cur = find_bytes(cur, end, pat.bytes, pat.size)) != nullptr) {
                const size_t pos = static_cast<size_t>(cur - begin);
                ++out.former_sample_count;
                if (std::strcmp(pat.name, "former_3576_send_sample") == 0) ++out.former_send_count;
                else ++out.former_recv_count;
                if (out.former_samples.size() < kMaxFormerStored) {
                    FormerSampleHit hit;
                    hit.label = pat.name;
                    hit.rva = slice.va + static_cast<uint32_t>(pos);
                    out.former_samples.push_back(hit);
                } else {
                    out.former_samples_truncated = true;
                }
                cur += pat.size;
            }
        }
    }
}

}  // namespace

ImageView parse_image(std::span<const uint8_t> bytes) {
    ImageView image;
    image.data = bytes.data();
    image.size = bytes.size();
    if (bytes.size() < 0x40 || load_u16(bytes.data()) != kMz) return image;
    const uint32_t e_lfanew = load_u32(bytes.data() + 0x3C);
    if (e_lfanew < 0x40 || e_lfanew > kMaxELfanew) return image;
    if (!range_ok(bytes.size(), e_lfanew, 24)) return image;
    if (load_u32(bytes.data() + e_lfanew) != kPe) return image;

    const size_t coff = e_lfanew + 4;
    const uint16_t section_count = load_u16(bytes.data() + coff + 2);
    image.timestamp_known = true;
    image.timestamp = load_u32(bytes.data() + coff + 4);
    const uint16_t optional_size = load_u16(bytes.data() + coff + 16);
    const size_t optional = e_lfanew + 24;
    if (!range_ok(bytes.size(), optional, optional_size) || optional_size < 60) return image;
    if (load_u16(bytes.data() + optional) != kPe32Plus) return image;
    if (optional_size < 112) return image;

    image.image_base_known = true;
    image.image_base = load_u64(bytes.data() + optional + 24);
    image.size_known = true;
    image.size_of_image = load_u32(bytes.data() + optional + 56);
    image.size_of_headers = load_u32(bytes.data() + optional + 60);
    const uint32_t directory_count = load_u32(bytes.data() + optional + 108);
    if (directory_count > 3 && optional_size >= 112 + 4 * 8) {
        image.exception_rva = load_u32(bytes.data() + optional + 112 + 3 * 8);
        image.exception_size = load_u32(bytes.data() + optional + 112 + 3 * 8 + 4);
    }

    const size_t section_table = optional + optional_size;
    if (section_count > 96 || !range_ok(bytes.size(), section_table, static_cast<size_t>(section_count) * 40)) return image;
    image.sections.reserve(section_count);
    for (uint16_t i = 0; i < section_count; ++i) {
        const uint8_t* row = bytes.data() + section_table + static_cast<size_t>(i) * 40;
        Section section;
        std::memcpy(section.name, row, 8);
        section.virtual_size = load_u32(row + 8);
        section.virtual_address = load_u32(row + 12);
        section.raw_size = load_u32(row + 16);
        section.raw_ptr = load_u32(row + 20);
        section.flags = load_u32(row + 36);
        image.sections.push_back(section);
    }
    image.headers_ok = true;
    return image;
}

FileKind classify(const ImageView& image) {
    if (!image.headers_ok || !image.timestamp_known) return FileKind::Unreadable;
    if (image.timestamp == kTimestamp2976) return FileKind::Rejected2976;
    if (image.timestamp == kTimestamp3576) return FileKind::Rejected3576;
    if (image.timestamp == kTimestamp3663 && image.image_base_known && image.image_base == kImageBase3663 &&
        image.size_known && image.size_of_image == kSizeOfImage3663) {
        return FileKind::Build3663;
    }
    return FileKind::RejectedOther;
}

const char* kind_name(FileKind kind) {
    switch (kind) {
    case FileKind::Unreadable: return "unreadable";
    case FileKind::Rejected2976: return "rejected-2976";
    case FileKind::Rejected3576: return "rejected-3576";
    case FileKind::RejectedOther: return "rejected-other";
    case FileKind::Build3663: return "build-3663";
    }
    return "unreadable";
}

Assessment assess(std::span<const uint8_t> bytes) {
    Assessment result;
    const ImageView image = parse_image(bytes);
    result.kind = classify(image);
    if (result.kind != FileKind::Build3663) return result;

    const std::vector<Slice> slices = slices_of(image);
    const std::vector<FunctionRange> functions = exception_functions(image);
    scan_messages(image, slices, result.messages);
    scan_alloc(slices, functions, result);
    scan_former_samples(slices, result);
    result.scanned = true;
    result.adopted = false;
    return result;
}

}  // namespace pkt3663
