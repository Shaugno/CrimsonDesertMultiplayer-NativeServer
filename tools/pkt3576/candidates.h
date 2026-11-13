#pragma once
// Experimental file candidates for a passive 1.0.0.3576 packet observer.
// These are not production mod offsets. Do not move them into
// src/mod/game/offsets.h or src/mod/session2/offsets_s2.h. The ASI
// compatibility guard stays closed and still accepts only build 1.0.0.2976.

#include <cstdint>

namespace pkt3576 {

// Installed Crimson Desert 1.0.0.3576. Identity for this experiment only.
// SizeOfImage and the file name are from docs/COMPATIBILITY-3576.md and the
// live probe logs. They are not hook addresses and are not adopted offsets.
inline constexpr uint32_t kTimestamp3576 = 0x6AED2E85;
inline constexpr uint64_t kImageBase = 0x140000000;
inline constexpr uint32_t kImageSize = 0x13CC4000;
inline constexpr wchar_t kExeName[] = L"CrimsonDesert.exe";

// Production build 1.0.0.2976. This observer must refuse it.
inline constexpr uint32_t kTimestamp2976 = 0x6AB28F00;
static_assert(kTimestamp3576 != kTimestamp2976);

// Send-like entry exe+0x3A6310 and receive-like entry exe+0x3A66A0.
// File candidates from docs/probe-3576/PACKET-CANDIDATES-3576.md. Not adopted.
inline constexpr uint64_t kSendRva = 0x3A6310;
inline constexpr uint64_t kRecvRva = 0x3A66A0;

// Copy bound for the bytes at r9. This is not a packet-format field.
inline constexpr uint32_t kPrefixCap = 16;

// Entry [rsp+28h] is evidenced on both candidates.
// Entry [rsp+30h] and [rsp+38h] are evidenced on the send candidate only.
inline constexpr uint64_t kStackSlot28 = 0x28;
inline constexpr uint64_t kStackSlot30 = 0x30;
inline constexpr uint64_t kStackSlot38 = 0x38;

inline constexpr uint8_t kSendEntry[16] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C,
    0x24, 0x20, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41,
};

inline constexpr uint8_t kRecvEntry[16] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
    0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x18, 0x55,
};

}  // namespace pkt3576
