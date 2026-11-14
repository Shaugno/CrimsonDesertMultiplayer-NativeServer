#include "pkt3663/entry_verify.h"

#include <array>
#include <cstring>
#include <limits>
#include <optional>

#include "pkt3663/assess.h"
#include "pkt3663/candidates.h"

namespace pkt3663 {
namespace {

constexpr uint32_t kSecExecute = 0x20000000;

uint32_t load_u32(const uint8_t* p) {
    uint32_t value = 0;
    std::memcpy(&value, p, sizeof value);
    return value;
}

int32_t load_i32(const uint8_t* p) {
    int32_t value = 0;
    std::memcpy(&value, p, sizeof value);
    return value;
}

bool range_ok(size_t size, size_t offset, size_t count) {
    return count <= size && offset <= size - count;
}

bool add_u32_ok(uint32_t value, uint32_t count, uint64_t& end) {
    end = static_cast<uint64_t>(value) + count;
    return end <= static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;
}

bool validate_sections(const ImageView& image) {
    for (size_t i = 0; i < image.sections.size(); ++i) {
        const Section& left = image.sections[i];
        uint64_t left_file_end = 0;
        uint64_t left_virtual_end = 0;
        if (left.raw_size != 0) {
            if (!range_ok(image.size, left.raw_ptr, left.raw_size) || left.raw_ptr < image.size_of_headers ||
                !add_u32_ok(left.raw_ptr, left.raw_size, left_file_end)) {
                return false;
            }
        }
        const uint32_t left_extent = left.virtual_size > left.raw_size ? left.virtual_size : left.raw_size;
        if (!add_u32_ok(left.virtual_address, left_extent, left_virtual_end) || left_virtual_end > image.size_of_image) {
            return false;
        }

        for (size_t j = i + 1; j < image.sections.size(); ++j) {
            const Section& right = image.sections[j];
            uint64_t right_file_end = 0;
            uint64_t right_virtual_end = 0;
            if (right.raw_size != 0 && !add_u32_ok(right.raw_ptr, right.raw_size, right_file_end)) {
                return false;
            }
            const uint32_t right_extent = right.virtual_size > right.raw_size ? right.virtual_size : right.raw_size;
            if (!add_u32_ok(right.virtual_address, right_extent, right_virtual_end)) return false;
            const bool file_overlap = left.raw_size != 0 && right.raw_size != 0 && left.raw_ptr < right_file_end &&
                                      right.raw_ptr < left_file_end;
            const bool virtual_overlap = left_extent != 0 && right_extent != 0 &&
                                         left.virtual_address < right_virtual_end && right.virtual_address < left_virtual_end;
            if (file_overlap || virtual_overlap) return false;
        }
    }
    return true;
}

struct Mapping {
    size_t offset = 0;
    uint32_t flags = 0;
};

std::optional<Mapping> strict_mapping(const ImageView& image, uint32_t rva, size_t count) {
    Mapping found;
    size_t matches = 0;
    for (const Section& section : image.sections) {
        if (rva < section.virtual_address) continue;
        const uint64_t delta = static_cast<uint64_t>(rva) - section.virtual_address;
        if (delta > section.raw_size || count > static_cast<uint64_t>(section.raw_size) - delta) continue;
        const uint64_t raw = static_cast<uint64_t>(section.raw_ptr) + delta;
        if (raw > std::numeric_limits<size_t>::max() || !range_ok(image.size, static_cast<size_t>(raw), count)) continue;
        found.offset = static_cast<size_t>(raw);
        found.flags = section.flags;
        ++matches;
    }
    if (matches != 1) return std::nullopt;
    return found;
}

bool bytes_equal(const ImageView& image, const ByteAnchor& anchor) {
    const std::optional<Mapping> mapping = strict_mapping(image, anchor.rva, anchor.size);
    return mapping && (mapping->flags & kSecExecute) != 0 &&
           std::memcmp(image.data + mapping->offset, anchor.bytes, anchor.size) == 0;
}

template <size_t N>
bool anchors_equal(const ImageView& image, const std::array<ByteAnchor, N>& anchors, uint32_t& failed_rva) {
    for (const ByteAnchor& anchor : anchors) {
        if (bytes_equal(image, anchor)) continue;
        failed_rva = anchor.rva;
        return false;
    }
    return true;
}

size_t count_executable_pattern(const ImageView& image, const uint8_t* pattern, size_t pattern_size,
                                uint32_t expected_rva, bool& expected_seen) {
    expected_seen = false;
    if (pattern_size == 0) return 0;
    size_t count = 0;
    for (const Section& section : image.sections) {
        if ((section.flags & kSecExecute) == 0 || section.raw_size < pattern_size) continue;
        const uint8_t* begin = image.data + section.raw_ptr;
        const uint8_t* end = begin + section.raw_size;
        const uint8_t* cursor = begin;
        const uint8_t* last = end - pattern_size;
        while (cursor <= last) {
            const void* raw_hit = std::memchr(cursor, pattern[0], static_cast<size_t>(last - cursor) + 1);
            if (!raw_hit) break;
            const uint8_t* hit = static_cast<const uint8_t*>(raw_hit);
            if (std::memcmp(hit, pattern, pattern_size) == 0) {
                const uint64_t rva = static_cast<uint64_t>(section.virtual_address) + static_cast<size_t>(hit - begin);
                if (rva == expected_rva) expected_seen = true;
                ++count;
            }
            cursor = hit + 1;
        }
    }
    return count;
}

struct FunctionTable {
    bool readable = false;
    bool valid = false;
    bool pump = false;
    bool send = false;
    bool receive = false;
};

FunctionTable inspect_functions(const ImageView& image) {
    FunctionTable out;
    if (image.exception_size < 12 || image.exception_size % 12 != 0 || image.exception_size / 12 > 1000000) return out;
    const std::optional<Mapping> table = strict_mapping(image, image.exception_rva, image.exception_size);
    if (!table) return out;
    out.readable = true;
    uint32_t previous_begin = 0;
    uint32_t pump_count = 0;
    uint32_t send_count = 0;
    uint32_t receive_count = 0;
    uint32_t pump_begin_count = 0;
    uint32_t send_begin_count = 0;
    uint32_t receive_begin_count = 0;
    const uint32_t count = image.exception_size / 12;
    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t* row = image.data + table->offset + static_cast<size_t>(i) * 12;
        const uint32_t begin = load_u32(row);
        const uint32_t end = load_u32(row + 4);
        if (begin >= end || end > image.size_of_image || (i != 0 && begin < previous_begin)) return out;
        previous_begin = begin;
        if (begin == kPumpRva) {
            ++pump_begin_count;
            if (end == kPumpEndRva) ++pump_count;
        }
        if (begin == kSendRva) {
            ++send_begin_count;
            if (end == kSendEndRva) ++send_count;
        }
        if (begin == kReceiveRva) {
            ++receive_begin_count;
            if (end == kReceiveEndRva) ++receive_count;
        }
    }
    if (pump_begin_count > 1 || send_begin_count > 1 || receive_begin_count > 1) return out;
    out.pump = pump_begin_count == 1 && pump_count == 1;
    out.send = send_begin_count == 1 && send_count == 1;
    out.receive = receive_begin_count == 1 && receive_count == 1;
    out.valid = true;
    return out;
}

bool executable_range(const ImageView& image, uint32_t begin, uint32_t end) {
    if (end <= begin) return false;
    const std::optional<Mapping> mapping = strict_mapping(image, begin, static_cast<size_t>(end - begin));
    return mapping && (mapping->flags & kSecExecute) != 0;
}

bool direct_call_targets(const ImageView& image, uint32_t call_rva, uint32_t wanted) {
    const std::optional<Mapping> mapping = strict_mapping(image, call_rva, 5);
    if (!mapping || (mapping->flags & kSecExecute) == 0) return false;
    const uint8_t* instruction = image.data + mapping->offset;
    if (instruction[0] != 0xE8) return false;
    const int64_t target = static_cast<int64_t>(call_rva) + 5 + load_i32(instruction + 1);
    return target >= 0 && target <= std::numeric_limits<uint32_t>::max() && static_cast<uint32_t>(target) == wanted;
}

}  // namespace

const char* entry_status_name(EntryStatus status) {
    switch (status) {
    case EntryStatus::WrongIdentity: return "wrong-identity";
    case EntryStatus::MalformedImage: return "malformed-image";
    case EntryStatus::MissingFunctionTable: return "missing-function-table";
    case EntryStatus::NotExecutable: return "not-executable";
    case EntryStatus::SendRangeMismatch: return "send-range-mismatch";
    case EntryStatus::ReceiveRangeMismatch: return "receive-range-mismatch";
    case EntryStatus::PumpRangeMismatch: return "pump-range-mismatch";
    case EntryStatus::SendAnchorMismatch: return "send-anchor-mismatch";
    case EntryStatus::ReceiveAnchorMismatch: return "receive-anchor-mismatch";
    case EntryStatus::PumpAnchorMismatch: return "pump-anchor-mismatch";
    case EntryStatus::AmbiguousAnchor: return "ambiguous-anchor";
    case EntryStatus::CallTargetMismatch: return "call-target-mismatch";
    case EntryStatus::VerifiedFileCandidates: return "verified-file-candidates";
    }
    return "wrong-identity";
}

EntryVerification verify_entries(std::span<const uint8_t> bytes) {
    EntryVerification out;
    const ImageView image = parse_image(bytes);
    if (classify(image) != FileKind::Build3663) return out;
    out.identity_ok = true;
    if (!validate_sections(image)) {
        out.status = EntryStatus::MalformedImage;
        return out;
    }

    const FunctionTable functions = inspect_functions(image);
    if (!functions.readable) {
        out.status = EntryStatus::MissingFunctionTable;
        return out;
    }
    if (!functions.valid) {
        out.status = EntryStatus::MalformedImage;
        return out;
    }
    out.send_range_ok = functions.send;
    if (!out.send_range_ok) {
        out.status = EntryStatus::SendRangeMismatch;
        return out;
    }
    out.receive_range_ok = functions.receive;
    if (!out.receive_range_ok) {
        out.status = EntryStatus::ReceiveRangeMismatch;
        return out;
    }
    out.pump_range_ok = functions.pump;
    if (!out.pump_range_ok) {
        out.status = EntryStatus::PumpRangeMismatch;
        return out;
    }
    if (!executable_range(image, kSendRva, kSendEndRva) || !executable_range(image, kReceiveRva, kReceiveEndRva) ||
        !executable_range(image, kPumpRva, kPumpEndRva)) {
        out.status = EntryStatus::NotExecutable;
        return out;
    }

    out.send_anchors_ok = anchors_equal(image, kSendAnchors, out.failed_anchor_rva);
    if (!out.send_anchors_ok) {
        out.status = EntryStatus::SendAnchorMismatch;
        return out;
    }
    out.receive_anchors_ok = anchors_equal(image, kReceiveAnchors, out.failed_anchor_rva);
    if (!out.receive_anchors_ok) {
        out.status = EntryStatus::ReceiveAnchorMismatch;
        return out;
    }
    out.pump_anchors_ok = anchors_equal(image, kPumpAnchors, out.failed_anchor_rva);
    if (!out.pump_anchors_ok) {
        out.status = EntryStatus::PumpAnchorMismatch;
        return out;
    }

    bool expected_seen = false;
    const size_t send_count = count_executable_pattern(image, kSendEntry, sizeof kSendEntry, kSendRva, expected_seen);
    if (!expected_seen || send_count != 1) {
        out.status = EntryStatus::AmbiguousAnchor;
        return out;
    }
    const size_t receive_count =
        count_executable_pattern(image, kReceiveEntry, sizeof kReceiveEntry, kReceiveRva, expected_seen);
    if (!expected_seen || receive_count != 1) {
        out.status = EntryStatus::AmbiguousAnchor;
        return out;
    }
    const uint32_t pump_anchor_rva = kPumpAnchors[0].rva;
    const size_t pump_count = count_executable_pattern(image, kPumpLoop, sizeof kPumpLoop, pump_anchor_rva, expected_seen);
    if (!expected_seen || pump_count != 1) {
        out.status = EntryStatus::AmbiguousAnchor;
        return out;
    }

    out.role_calls_ok = direct_call_targets(image, kPumpReceiveCallRva, kReceiveRva) &&
                        direct_call_targets(image, kPumpSendCallRva, kSendRva);
    if (!out.role_calls_ok) {
        out.status = EntryStatus::CallTargetMismatch;
        return out;
    }

    out.status = EntryStatus::VerifiedFileCandidates;
    out.passive_capture_ready = true;
    out.adopted = false;
    return out;
}

}  // namespace pkt3663
