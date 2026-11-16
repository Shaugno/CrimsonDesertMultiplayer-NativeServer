#pragma once
// Native IDs only, not handler addresses or permission to replay requests.
// Reproduced by tools/port3663/r4_scope.py from paired descriptor/handler evidence.
#include <array>
#include <cstdint>
namespace cdmp::session2::scope3663 {
inline constexpr uint16_t kReloadStageStartReq = 3018;
inline constexpr uint16_t kMoveFieldCheatReq = 2647;
inline constexpr uint16_t kSetFieldTimeReq = 2150;
inline constexpr uint16_t kAITeleportReq = 2236;
inline constexpr std::array<uint16_t,4> kRefusedRequests{
    kReloadStageStartReq,kMoveFieldCheatReq,kSetFieldTimeReq,kAITeleportReq};
}
