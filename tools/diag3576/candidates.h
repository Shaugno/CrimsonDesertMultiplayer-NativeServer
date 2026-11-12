#pragma once
// Experimental addresses for the external 1.0.0.3576 position diagnostic.
// These are not production mod offsets. Do not move them into
// src/mod/game/offsets.h. The ASI compatibility guard stays closed and still
// accepts only build 1.0.0.2976.

#include <cstdint>
#include <string_view>

namespace diag3576 {

// Installed Crimson Desert 1.0.0.3576. This is the diagnostic's identity, not
// a decision that the mod may run inside that executable.
inline constexpr uint32_t kDiagnosticTimestamp = 0x6AED2E85;
inline constexpr uint64_t kExpectedImageBase = 0x140000000;
inline constexpr std::wstring_view kExpectedExeName = L"CrimsonDesert.exe";

// Production build 1.0.0.2976. Present only so this tool cannot treat it as
// the diagnostic target.
inline constexpr uint32_t kProductionTimestamp = 0x6AB28F00;
static_assert(kDiagnosticTimestamp != kProductionTimestamp);

// File-trace candidates. Not adopted. The manager slot is selected by the
// operator; none of the image slots has been adopted as a permanent root.
inline constexpr uint64_t kOriginRva = 0x71AC2C0;
inline constexpr uint64_t kStaticRva = 0x71CF140;

// Manager-rooted chain. +0x30 is not applied: the selected slot already
// stores ClientActorManager.
inline constexpr uint64_t kHopActor = 0x50;
inline constexpr uint64_t kHopComponentTable = 0x68;
inline constexpr uint64_t kHopCharacter = 0x40;
inline constexpr uint64_t kHopInner = 0x140;
inline constexpr uint64_t kHopPhysics = 0x2C0;
inline constexpr uint64_t kHopLocal = 0x90;

inline constexpr std::string_view kRttiManager = ".?AVClientActorManager@pa@@";
inline constexpr std::string_view kRttiActor = ".?AVClientChildOnlyInGameActor@pa@@";
inline constexpr std::string_view kRttiCharacter = ".?AVClientCharacterControlActorComponent@pa@@";

// A sample that is still changing, unreadable, or non-finite is tried again
// this many times, then flagged. Process exit is not retried.
inline constexpr int kMaxSampleAttempts = 3;

}  // namespace diag3576
