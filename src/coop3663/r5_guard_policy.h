#pragma once
// Derived from Samuel Cardillo's field_leave_guard_logic.h and
// achievement_guard_logic.h, ByJunter PR #1, a3904af3e1a32056dcf7a63bc7a469eb4231084b.
// GPL-3.0: preserve original project attribution. The 3663 addresses and
// additional runtime identity checks are this continuation's verified port.
#include <cstdint>

namespace cdmp::coop3663::r5_guard {
// Leave the owner's manager loaded, but continue the native own-manager
// unload. Missing identities and the owner's own characters stay native.
template<class Context>
bool preserve_owner(Context& c, uintptr_t own, uint32_t leaver, uint32_t owner,
                    uintptr_t continuation) noexcept {
    if (!own || !c.rax || own == c.rax || !leaver || !owner || leaver == owner) return false;
    c.rip = continuation;
    return true;
}
// At the verified post-prologue midpoint, both by-value arguments still
// belong to the native frame. Its cleanup path frees them and returns success.
template<class Context>
bool skip_achievement(Context& c, bool remote_seen, uintptr_t cleanup) noexcept {
    if (!remote_seen) return false;
    c.rip = cleanup;
    return true;
}
}
