#pragma once
// File identity for Crimson Desert 1.0.0.3576. These three values were read
// from the installed executable. This header has no gameplay address.
//
// The send and receive candidates exe+0x3A6310 and exe+0x3A66A0 stay in
// tools/pkt3576/candidates.h. They are not copied here until a live capture
// shows those 16-byte entries still match. Do not include offsets.h from here.

#include <cstdint>

namespace cdmp::game::build_3576 {

inline constexpr uint32_t kPeTimestamp = 0x6AED2E85;
inline constexpr uintptr_t kImageBase = 0x140000000;
inline constexpr uint32_t kSizeOfImage = 0x13CC4000;

}  // namespace cdmp::game::build_3576
