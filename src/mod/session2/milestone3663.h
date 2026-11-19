#pragma once
// R10 Step 2, HOST only: the guest's own PlayAllPreLoadSequencerAck 2941 when it
// enters the host's field (coop3663/milestone_policy.h has the logic and the
// evidence trail). Build 1.0.0.3732. [session2] core_milestone = 0 (off,
// default) | 1 (decide and log) | 2 (send).
#include "coop3663/milestone_policy.h"
#include "mod/game/build_guard.h"
#include <cstdint>
#include <string>

namespace cdmp::session2::milestone3663 {

// Verifies the exact 3732 code and installs the mid hook at 0x2ED8852 unless
// mode is Off. False when a requested mode cannot be installed exactly.
bool install(const game::build_guard::ImageIdentity& identity, coop3663::milestone::Mode mode);

// take_frame (the core route) reports each followed opcode after its push to
// the live guest session. `owner` is the server's 2364 owner before the guest
// copy is rewritten.
void on_out(uint16_t opcode, uintptr_t session, bool pushed, coop3663::milestone::MoveOwner owner = {}) noexcept;

// RAII marker: this thread is inside take_frame (the hook never asks the game
// to send from there).
struct TakeFrameScope {
    TakeFrameScope() noexcept;
    ~TakeFrameScope();
    TakeFrameScope(const TakeFrameScope&) = delete;
    TakeFrameScope& operator=(const TakeFrameScope&) = delete;
};

// Logs the decision notes the hook recorded (mod thread only).
void drain();
std::string describe();
}  // namespace cdmp::session2::milestone3663
