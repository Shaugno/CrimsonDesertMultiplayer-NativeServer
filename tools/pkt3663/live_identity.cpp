// Loaded-image identity and entry-evidence gate for the experimental 3663
// passive collector. This is deliberately independent of the file-layout
// verifier because a loaded PE is addressed by RVA, not PointerToRawData.

#include "pkt3663/live_identity.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <memory>
#include <new>

#include <windows.h>

#include "pkt3663/assess.h"
#include "pkt3663/candidates.h"

namespace pkt3663 {
namespace {

constexpr wchar_t kExeName[] = L"CrimsonDesert.exe";
constexpr uint16_t kDosMagic = 0x5A4D;
constexpr uint32_t kPeSignature = 0x00004550;
constexpr uint16_t kPe32PlusMagic = 0x020B;
constexpr uint32_t kSecExecute = 0x20000000;
constexpr uint32_t kMaxELfanew = 1024;
constexpr uint16_t kMaxSections = 96;
constexpr uint16_t kMinOptionalSize = 144;
constexpr uint16_t kMaxOptionalSize = 512;
constexpr uint32_t kMaxFunctionRows = 1000000;
constexpr size_t kScanChunk = 64 * 1024;
constexpr size_t kMaxAnchorSize = 128;
constexpr size_t kMaxMappedRead = kScanChunk + kMaxAnchorSize;
constexpr size_t kPathCap = 1024;

struct MappedSection {
  uint32_t virtual_address = 0;
  uint32_t extent = 0;
  uint32_t flags = 0;
};

struct Parsed {
  std::array<MappedSection, kMaxSections> sections{};
  uint16_t count = 0;
  uint32_t exception_rva = 0;
  uint32_t exception_size = 0;
};

uint16_t load_u16(const uint8_t *data) {
  uint16_t value = 0;
  std::memcpy(&value, data, sizeof value);
  return value;
}

uint32_t load_u32(const uint8_t *data) {
  uint32_t value = 0;
  std::memcpy(&value, data, sizeof value);
  return value;
}

uint64_t load_u64(const uint8_t *data) {
  uint64_t value = 0;
  std::memcpy(&value, data, sizeof value);
  return value;
}

int32_t load_i32(const uint8_t *data) {
  int32_t value = 0;
  std::memcpy(&value, data, sizeof value);
  return value;
}

bool checked_address(uintptr_t base, uint64_t offset, size_t size,
                     uintptr_t &address) {
  if (offset > static_cast<uint64_t>(UINTPTR_MAX) || size > UINTPTR_MAX)
    return false;
  const uintptr_t off = static_cast<uintptr_t>(offset);
  if (base > UINTPTR_MAX - off)
    return false;
  address = base + off;
  return address <= UINTPTR_MAX - size;
}

bool copy_absolute(ImageRead read, void *user, uintptr_t address,
                   void *destination, size_t size) {
  if (read == nullptr || destination == nullptr || size == 0 ||
      address > UINTPTR_MAX - size)
    return false;
  return read(address, destination, size, user);
}

bool copy_offset(ImageRead read, void *user, uintptr_t base, uint64_t offset,
                 void *destination, size_t size) {
  uintptr_t address = 0;
  return checked_address(base, offset, size, address) &&
         copy_absolute(read, user, address, destination, size);
}

bool copy_rva(ImageRead read, void *user, uintptr_t base, uint32_t image_size,
              uint32_t rva, void *destination, size_t size) {
  if (size > image_size || rva > image_size - size)
    return false;
  return copy_offset(read, user, base, rva, destination, size);
}

bool name_is_game(const wchar_t *exe_name) {
  return exe_name != nullptr && exe_name[0] != L'\0' &&
         _wcsicmp(exe_name, kExeName) == 0;
}

bool protect_readable(DWORD protect) {
  if ((protect & PAGE_GUARD) != 0)
    return false;
  switch (protect & 0xFF) {
  case PAGE_READONLY:
  case PAGE_READWRITE:
  case PAGE_WRITECOPY:
  case PAGE_EXECUTE_READ:
  case PAGE_EXECUTE_READWRITE:
  case PAGE_EXECUTE_WRITECOPY:
    return true;
  default:
    return false;
  }
}

// No C++ objects with destructors: this function uses SEH.
bool seh_copy(uintptr_t address, void *destination, size_t size) {
  __try {
    std::memcpy(destination, reinterpret_cast<const void *>(address), size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

bool range_readable(uintptr_t address, size_t size) {
  if (size == 0 || address > UINTPTR_MAX - size)
    return false;
  uintptr_t cursor = address;
  const uintptr_t end = address + size;
  while (cursor < end) {
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(reinterpret_cast<const void *>(cursor), &info,
                     sizeof info) != sizeof info)
      return false;
    if (info.State != MEM_COMMIT || !protect_readable(info.Protect) ||
        info.RegionSize == 0)
      return false;
    const uintptr_t region = reinterpret_cast<uintptr_t>(info.BaseAddress);
    if (region > UINTPTR_MAX - info.RegionSize ||
        region + info.RegionSize <= cursor)
      return false;
    cursor = std::min(end, region + info.RegionSize);
  }
  return true;
}

bool validate_sections(const Parsed &parsed, uint32_t size_of_headers,
                       uint32_t size_of_image) {
  for (uint16_t i = 0; i < parsed.count; ++i) {
    const MappedSection &left = parsed.sections[i];
    if (left.extent == 0)
      continue;
    const uint64_t left_end =
        static_cast<uint64_t>(left.virtual_address) + left.extent;
    if (left.virtual_address < size_of_headers || left_end > size_of_image)
      return false;
    for (uint16_t j = static_cast<uint16_t>(i + 1); j < parsed.count; ++j) {
      const MappedSection &right = parsed.sections[j];
      if (right.extent == 0)
        continue;
      const uint64_t right_end =
          static_cast<uint64_t>(right.virtual_address) + right.extent;
      if (right.virtual_address < size_of_headers || right_end > size_of_image)
        return false;
      if (left.virtual_address < right_end && right.virtual_address < left_end)
        return false;
    }
  }
  return true;
}

const MappedSection *unique_section(const Parsed &parsed, uint32_t rva,
                                    size_t size) {
  const MappedSection *found = nullptr;
  size_t matches = 0;
  for (uint16_t i = 0; i < parsed.count; ++i) {
    const MappedSection &section = parsed.sections[i];
    if (rva < section.virtual_address)
      continue;
    const uint64_t delta = static_cast<uint64_t>(rva) - section.virtual_address;
    if (delta > section.extent ||
        size > static_cast<uint64_t>(section.extent) - delta)
      continue;
    found = &section;
    ++matches;
  }
  return matches == 1 ? found : nullptr;
}

bool executable_range(const Parsed &parsed, uint32_t begin, uint32_t end) {
  if (end <= begin)
    return false;
  const MappedSection *section =
      unique_section(parsed, begin, static_cast<size_t>(end - begin));
  return section != nullptr && (section->flags & kSecExecute) != 0;
}

enum class ReadResult { Match, Mismatch, Unreadable };

ReadResult anchor_equal(const ByteAnchor &anchor, const Parsed &parsed,
                        uintptr_t base, uint32_t image_size, ImageRead read,
                        void *user) {
  if (anchor.size == 0 || anchor.size > kMaxAnchorSize)
    return ReadResult::Mismatch;
  const MappedSection *section =
      unique_section(parsed, anchor.rva, anchor.size);
  if (section == nullptr || (section->flags & kSecExecute) == 0)
    return ReadResult::Mismatch;
  uint8_t bytes[kMaxAnchorSize] = {};
  if (!copy_rva(read, user, base, image_size, anchor.rva, bytes, anchor.size))
    return ReadResult::Unreadable;
  return std::memcmp(bytes, anchor.bytes, anchor.size) == 0
             ? ReadResult::Match
             : ReadResult::Mismatch;
}

template <size_t N>
ReadResult anchors_equal(const std::array<ByteAnchor, N> &anchors,
                         const Parsed &parsed, uintptr_t base,
                         uint32_t image_size, ImageRead read, void *user,
                         uint32_t &failed_rva) {
  for (const ByteAnchor &anchor : anchors) {
    const ReadResult result =
        anchor_equal(anchor, parsed, base, image_size, read, user);
    if (result == ReadResult::Match)
      continue;
    failed_rva = anchor.rva;
    return result;
  }
  return ReadResult::Match;
}

struct AnchorCount {
  const ByteAnchor *anchor = nullptr;
  uint32_t count = 0;
  bool expected_seen = false;
};

enum class UniqueResult { Unique, Ambiguous, Unreadable };

UniqueResult anchors_unique(const Parsed &parsed, uintptr_t base,
                            uint32_t image_size, ImageRead read, void *user) {
  constexpr size_t kCount =
      kSendAnchors.size() + kReceiveAnchors.size() + kPumpAnchors.size();
  std::array<AnchorCount, kCount> counts{};
  size_t used = 0;
  for (const ByteAnchor &anchor : kSendAnchors)
    counts[used++].anchor = &anchor;
  for (const ByteAnchor &anchor : kReceiveAnchors)
    counts[used++].anchor = &anchor;
  for (const ByteAnchor &anchor : kPumpAnchors)
    counts[used++].anchor = &anchor;

  std::unique_ptr<uint8_t[]> buffer(new (std::nothrow)
                                        uint8_t[kScanChunk + kMaxAnchorSize]);
  if (!buffer)
    return UniqueResult::Unreadable;
  for (uint16_t section_index = 0; section_index < parsed.count;
       ++section_index) {
    const MappedSection &section = parsed.sections[section_index];
    if ((section.flags & kSecExecute) == 0 || section.extent == 0)
      continue;
    uint32_t offset = 0;
    while (offset < section.extent) {
      const size_t remaining = static_cast<size_t>(section.extent - offset);
      const size_t primary = std::min(kScanChunk, remaining);
      const size_t available =
          std::min(remaining, primary + kMaxAnchorSize - 1);
      const uint64_t rva64 =
          static_cast<uint64_t>(section.virtual_address) + offset;
      if (rva64 > std::numeric_limits<uint32_t>::max() ||
          !copy_rva(read, user, base, image_size, static_cast<uint32_t>(rva64),
                    buffer.get(), available)) {
        return UniqueResult::Unreadable;
      }

      for (AnchorCount &scan : counts) {
        if (scan.count > 1 || scan.anchor == nullptr ||
            scan.anchor->size == 0 || scan.anchor->size > available) {
          continue;
        }
        const size_t possible = available - scan.anchor->size + 1;
        const size_t starts = std::min(primary, possible);
        const uint8_t *cursor = buffer.get();
        const uint8_t *const limit = buffer.get() + starts;
        while (cursor < limit) {
          const void *raw = std::memchr(cursor, scan.anchor->bytes[0],
                                        static_cast<size_t>(limit - cursor));
          if (raw == nullptr)
            break;
          const auto *hit = static_cast<const uint8_t *>(raw);
          if (std::memcmp(hit, scan.anchor->bytes, scan.anchor->size) == 0) {
            const uint64_t hit_rva =
                rva64 + static_cast<size_t>(hit - buffer.get());
            if (hit_rva == scan.anchor->rva)
              scan.expected_seen = true;
            ++scan.count;
            if (scan.count > 1)
              break;
          }
          cursor = hit + 1;
        }
      }
      offset += static_cast<uint32_t>(primary);
    }
  }
  for (const AnchorCount &scan : counts) {
    if (scan.count != 1 || !scan.expected_seen)
      return UniqueResult::Ambiguous;
  }
  return UniqueResult::Unique;
}

enum class FunctionResult { Valid, Missing, Malformed, Unreadable };

FunctionResult inspect_functions(const Parsed &parsed, uintptr_t base,
                                 uint32_t image_size, ImageRead read,
                                 void *user, Identity &identity) {
  if (parsed.exception_size == 0)
    return FunctionResult::Missing;
  if (parsed.exception_size < 12 || parsed.exception_size % 12 != 0 ||
      parsed.exception_size / 12 > kMaxFunctionRows ||
      unique_section(parsed, parsed.exception_rva, parsed.exception_size) ==
          nullptr) {
    return FunctionResult::Malformed;
  }

  uint32_t previous_begin = 0;
  uint32_t pump_begin_count = 0;
  uint32_t send_begin_count = 0;
  uint32_t receive_begin_count = 0;
  uint32_t pump_exact_count = 0;
  uint32_t send_exact_count = 0;
  uint32_t receive_exact_count = 0;
  const uint32_t count = parsed.exception_size / 12;
  for (uint32_t index = 0; index < count; ++index) {
    uint8_t row[12] = {};
    const uint64_t row_rva = static_cast<uint64_t>(parsed.exception_rva) +
                             static_cast<uint64_t>(index) * 12;
    if (row_rva > std::numeric_limits<uint32_t>::max() ||
        !copy_rva(read, user, base, image_size, static_cast<uint32_t>(row_rva),
                  row, sizeof row)) {
      return FunctionResult::Unreadable;
    }
    const uint32_t begin = load_u32(row);
    const uint32_t end = load_u32(row + 4);
    if (begin >= end || end > image_size ||
        (index != 0 && begin <= previous_begin)) {
      return FunctionResult::Malformed;
    }
    previous_begin = begin;
    if (begin == kPumpRva) {
      ++pump_begin_count;
      if (end == kPumpEndRva)
        ++pump_exact_count;
    }
    if (begin == kSendRva) {
      ++send_begin_count;
      if (end == kSendEndRva)
        ++send_exact_count;
    }
    if (begin == kReceiveRva) {
      ++receive_begin_count;
      if (end == kReceiveEndRva)
        ++receive_exact_count;
    }
  }
  identity.pump_range_ok = pump_begin_count == 1 && pump_exact_count == 1;
  identity.send_range_ok = send_begin_count == 1 && send_exact_count == 1;
  identity.receive_range_ok =
      receive_begin_count == 1 && receive_exact_count == 1;
  identity.function_table_ok = true;
  return FunctionResult::Valid;
}

ReadResult direct_call_targets(const Parsed &parsed, uintptr_t base,
                               uint32_t image_size, ImageRead read, void *user,
                               uint32_t call_rva, uint32_t target_rva) {
  const MappedSection *section = unique_section(parsed, call_rva, 5);
  if (section == nullptr || (section->flags & kSecExecute) == 0)
    return ReadResult::Mismatch;
  uint8_t instruction[5] = {};
  if (!copy_rva(read, user, base, image_size, call_rva, instruction,
                sizeof instruction)) {
    return ReadResult::Unreadable;
  }
  if (instruction[0] != 0xE8)
    return ReadResult::Mismatch;
  const int64_t target =
      static_cast<int64_t>(call_rva) + 5 + load_i32(instruction + 1);
  return target >= 0 && target <= std::numeric_limits<uint32_t>::max() &&
                 static_cast<uint32_t>(target) == target_rva
             ? ReadResult::Match
             : ReadResult::Mismatch;
}

const wchar_t *file_name_of(const wchar_t *path) {
  if (path == nullptr)
    return L"";
  const wchar_t *name = path;
  for (const wchar_t *cursor = path; *cursor != L'\0'; ++cursor) {
    if (*cursor == L'\\' || *cursor == L'/')
      name = cursor + 1;
  }
  return name;
}

} // namespace

bool read_mapped_bytes(uintptr_t address, void *destination, size_t size) {
  if (destination == nullptr || size == 0 || size > kMaxMappedRead)
    return false;
  if (!range_readable(address, size))
    return false;
  return seh_copy(address, destination, size);
}

Identity identify_image(const wchar_t *exe_name, uintptr_t mapped_base,
                        ImageRead read, void *user) {
  Identity identity;
  identity.mapped_base = mapped_base;
  if (read == nullptr || mapped_base == 0)
    return identity;

  uint8_t dos[0x40] = {};
  if (!copy_offset(read, user, mapped_base, 0, dos, sizeof dos) ||
      load_u16(dos) != kDosMagic)
    return identity;
  const uint32_t e_lfanew = load_u32(dos + 0x3C);
  if (e_lfanew < 0x40 || e_lfanew > kMaxELfanew) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }

  uint8_t file_header[24] = {};
  if (!copy_offset(read, user, mapped_base, e_lfanew, file_header,
                   sizeof file_header))
    return identity;
  if (load_u32(file_header) != kPeSignature) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }
  const uint16_t section_count = load_u16(file_header + 6);
  identity.timestamp_known = true;
  identity.timestamp = load_u32(file_header + 8);
  identity.rejected_2976 = identity.timestamp == kTimestamp2976;
  identity.rejected_3576 = identity.timestamp == kTimestamp3576;
  const uint16_t optional_size = load_u16(file_header + 20);
  if (section_count == 0 || section_count > kMaxSections ||
      optional_size < kMinOptionalSize || optional_size > kMaxOptionalSize) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }

  std::array<uint8_t, kMaxOptionalSize> optional{};
  if (!copy_offset(read, user, mapped_base,
                   static_cast<uint64_t>(e_lfanew) + 24, optional.data(),
                   optional_size)) {
    return identity;
  }
  if (load_u16(optional.data()) != kPe32PlusMagic) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }
  identity.preferred_base_known = true;
  identity.preferred_base = load_u64(optional.data() + 24);
  identity.size_known = true;
  identity.size_of_image = load_u32(optional.data() + 56);
  const uint32_t size_of_headers = load_u32(optional.data() + 60);
  const uint32_t directory_count = load_u32(optional.data() + 108);

  // No candidate-RVA read occurs before all four identity fields pass.
  if (!name_is_game(exe_name)) {
    identity.kind = IdentityKind::WrongName;
    return identity;
  }
  if (identity.timestamp != kTimestamp3663) {
    identity.kind = IdentityKind::WrongTimestamp;
    return identity;
  }
  if (mapped_base != static_cast<uintptr_t>(kImageBase3663) ||
      identity.preferred_base != kImageBase3663) {
    identity.kind = IdentityKind::WrongBase;
    return identity;
  }
  if (identity.size_of_image != kSizeOfImage3663) {
    identity.kind = IdentityKind::WrongSize;
    return identity;
  }
  if (directory_count < 4 || size_of_headers < 0x40 ||
      size_of_headers > identity.size_of_image) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }

  Parsed parsed;
  parsed.count = section_count;
  parsed.exception_rva = load_u32(optional.data() + 136);
  parsed.exception_size = load_u32(optional.data() + 140);
  const uint64_t section_table =
      static_cast<uint64_t>(e_lfanew) + 24 + optional_size;
  const uint64_t section_table_end =
      section_table + static_cast<uint64_t>(section_count) * 40;
  if (section_table_end > size_of_headers ||
      section_table_end > identity.size_of_image) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }
  std::array<uint8_t, static_cast<size_t>(kMaxSections) * 40> section_bytes{};
  const size_t section_bytes_size = static_cast<size_t>(section_count) * 40;
  if (!copy_offset(read, user, mapped_base, section_table, section_bytes.data(),
                   section_bytes_size))
    return identity;
  for (uint16_t index = 0; index < section_count; ++index) {
    const uint8_t *row = section_bytes.data() + static_cast<size_t>(index) * 40;
    MappedSection &section = parsed.sections[index];
    const uint32_t virtual_size = load_u32(row + 8);
    section.virtual_address = load_u32(row + 12);
    const uint32_t raw_size = load_u32(row + 16);
    section.extent = std::max(virtual_size, raw_size);
    section.flags = load_u32(row + 36);
  }
  if (!validate_sections(parsed, size_of_headers, identity.size_of_image)) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }
  identity.sections_ok = true;

  const FunctionResult functions = inspect_functions(
      parsed, mapped_base, identity.size_of_image, read, user, identity);
  if (functions == FunctionResult::Unreadable)
    return identity;
  if (functions == FunctionResult::Missing) {
    identity.kind = IdentityKind::MissingFunctionTable;
    return identity;
  }
  if (functions == FunctionResult::Malformed) {
    identity.kind = IdentityKind::MalformedImage;
    return identity;
  }
  if (!identity.pump_range_ok || !identity.send_range_ok ||
      !identity.receive_range_ok) {
    identity.kind = IdentityKind::FunctionRangeMismatch;
    return identity;
  }
  if (!executable_range(parsed, kPumpRva, kPumpEndRva) ||
      !executable_range(parsed, kSendRva, kSendEndRva) ||
      !executable_range(parsed, kReceiveRva, kReceiveEndRva)) {
    identity.kind = IdentityKind::NotExecutable;
    return identity;
  }

  ReadResult anchors =
      anchors_equal(kPumpAnchors, parsed, mapped_base, identity.size_of_image,
                    read, user, identity.failed_anchor_rva);
  if (anchors == ReadResult::Unreadable)
    return identity;
  identity.pump_anchors_ok = anchors == ReadResult::Match;
  if (!identity.pump_anchors_ok) {
    identity.kind = IdentityKind::AnchorMismatch;
    return identity;
  }
  anchors =
      anchors_equal(kSendAnchors, parsed, mapped_base, identity.size_of_image,
                    read, user, identity.failed_anchor_rva);
  if (anchors == ReadResult::Unreadable)
    return identity;
  identity.send_anchors_ok = anchors == ReadResult::Match;
  if (!identity.send_anchors_ok) {
    identity.kind = IdentityKind::AnchorMismatch;
    return identity;
  }
  anchors = anchors_equal(kReceiveAnchors, parsed, mapped_base,
                          identity.size_of_image, read, user,
                          identity.failed_anchor_rva);
  if (anchors == ReadResult::Unreadable)
    return identity;
  identity.receive_anchors_ok = anchors == ReadResult::Match;
  if (!identity.receive_anchors_ok) {
    identity.kind = IdentityKind::AnchorMismatch;
    return identity;
  }

  const UniqueResult unique =
      anchors_unique(parsed, mapped_base, identity.size_of_image, read, user);
  if (unique == UniqueResult::Unreadable)
    return identity;
  identity.anchors_unique = unique == UniqueResult::Unique;
  if (!identity.anchors_unique) {
    identity.kind = IdentityKind::AmbiguousAnchor;
    return identity;
  }

  const ReadResult receive_call =
      direct_call_targets(parsed, mapped_base, identity.size_of_image, read,
                          user, kPumpReceiveCallRva, kReceiveRva);
  const ReadResult send_call =
      direct_call_targets(parsed, mapped_base, identity.size_of_image, read,
                          user, kPumpSendCallRva, kSendRva);
  if (receive_call == ReadResult::Unreadable ||
      send_call == ReadResult::Unreadable)
    return identity;
  identity.role_calls_ok =
      receive_call == ReadResult::Match && send_call == ReadResult::Match;
  if (!identity.role_calls_ok) {
    identity.kind = IdentityKind::CallTargetMismatch;
    return identity;
  }

  identity.kind = IdentityKind::Match;
  return identity;
}

Identity identify_image_after_receive_patch(const wchar_t *exe_name,
                                            uintptr_t mapped_base,
                                            ImageRead read, void *user,
                                            const uint8_t *expected_patch,
                                            size_t expected_patch_size) {
  Identity refused;
  refused.mapped_base = mapped_base;
  if (read == nullptr || expected_patch == nullptr ||
      expected_patch_size == 0 || expected_patch_size > sizeof kReceiveEntry) {
    return refused;
  }
  if (expected_patch[0] == 0xE9) {
    if (expected_patch_size < 5)
      return refused;
    for (size_t index = 5; index < expected_patch_size; ++index) {
      if (expected_patch[index] != 0x90)
        return refused;
    }
  } else if (expected_patch_size >= 14 && expected_patch[0] == 0xFF &&
             expected_patch[1] == 0x25) {
    int32_t displacement = 0;
    std::memcpy(&displacement, expected_patch + 2, sizeof displacement);
    if (displacement != 0)
      return refused;
    for (size_t index = 14; index < expected_patch_size; ++index) {
      if (expected_patch[index] != 0x90)
        return refused;
    }
  } else {
    return refused;
  }

  struct MaskedReader {
    uintptr_t base = 0;
    ImageRead read = nullptr;
    void *user = nullptr;
    const uint8_t *expected_patch = nullptr;
    size_t expected_patch_size = 0;
  } masked{mapped_base, read, user, expected_patch, expected_patch_size};
  auto reader = [](uintptr_t address, void *destination, size_t size,
                   void *opaque) -> bool {
    auto *state = static_cast<MaskedReader *>(opaque);
    if (state == nullptr || state->read == nullptr ||
        state->expected_patch == nullptr || destination == nullptr ||
        size == 0 || address > UINTPTR_MAX - size) {
      return false;
    }
    if (!state->read(address, destination, size, state->user))
      return false;
    if (state->base > UINTPTR_MAX - static_cast<uintptr_t>(kReceiveRva))
      return false;
    const uintptr_t patch_begin =
        state->base + static_cast<uintptr_t>(kReceiveRva);
    if (patch_begin > UINTPTR_MAX - state->expected_patch_size)
      return false;
    const uintptr_t patch_end = patch_begin + state->expected_patch_size;
    const uintptr_t read_end = address + size;
    const uintptr_t overlap_begin = std::max(address, patch_begin);
    const uintptr_t overlap_end = std::min(read_end, patch_end);
    if (overlap_begin < overlap_end) {
      auto *output = static_cast<uint8_t *>(destination);
      const size_t output_at = static_cast<size_t>(overlap_begin - address);
      const size_t patch_at = static_cast<size_t>(overlap_begin - patch_begin);
      const size_t count = static_cast<size_t>(overlap_end - overlap_begin);
      if (std::memcmp(output + output_at, state->expected_patch + patch_at,
                      count) != 0) {
        return false;
      }
      std::memcpy(output + output_at, kReceiveEntry + patch_at, count);
    }
    return true;
  };
  return identify_image(exe_name, mapped_base, reader, &masked);
}

Identity identify_process() {
  Identity identity;
  HMODULE module = GetModuleHandleW(nullptr);
  if (module == nullptr)
    return identity;
  const uintptr_t base = reinterpret_cast<uintptr_t>(module);
  identity.mapped_base = base;
  wchar_t path[kPathCap] = {};
  const DWORD length =
      GetModuleFileNameW(module, path, static_cast<DWORD>(kPathCap));
  if (length == 0 || length >= kPathCap)
    return identity;
  auto reader = [](uintptr_t address, void *destination, size_t size, void *) {
    return read_mapped_bytes(address, destination, size);
  };
  return identify_image(file_name_of(path), base, reader, nullptr);
}

bool identity_allows_hooks(const Identity &identity) {
  return identity.kind == IdentityKind::Match;
}

const char *identity_kind_name(IdentityKind kind) {
  switch (kind) {
  case IdentityKind::Match:
    return "match";
  case IdentityKind::Unreadable:
    return "unreadable";
  case IdentityKind::WrongName:
    return "wrong-name";
  case IdentityKind::WrongTimestamp:
    return "wrong-timestamp";
  case IdentityKind::WrongBase:
    return "wrong-base";
  case IdentityKind::WrongSize:
    return "wrong-size";
  case IdentityKind::MalformedImage:
    return "malformed-image";
  case IdentityKind::MissingFunctionTable:
    return "missing-function-table";
  case IdentityKind::NotExecutable:
    return "not-executable";
  case IdentityKind::FunctionRangeMismatch:
    return "function-range-mismatch";
  case IdentityKind::AnchorMismatch:
    return "anchor-mismatch";
  case IdentityKind::AmbiguousAnchor:
    return "ambiguous-anchor";
  case IdentityKind::CallTargetMismatch:
    return "call-target-mismatch";
  }
  return "unreadable";
}

} // namespace pkt3663
