#pragma once
#include "mod/game/build_guard.h"
#include "mod/role_logic.h"
#include <cstdint>

namespace cdmp::session2::character_guard3663 {
bool install(const game::build_guard::ImageIdentity&, role::Role);
void note_peer_character(uint32_t key) noexcept;
void set_link_ready(bool ready) noexcept;
uint64_t skips() noexcept;
// R10 Step 3 ([session2] remote_actor_guard): also skip the helper at the exact
// null read for actors controlled by the other player (actor+0x90).
void set_remote_containment(bool enabled) noexcept;
}
