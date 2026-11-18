#pragma once
#include "mod/game/build_guard.h"
#include <cstdint>

namespace cdmp::session2::guest_character3663 {
bool install(const game::build_guard::ImageIdentity&);
void note_host_character(uint32_t key) noexcept;
void set_link_ready(bool ready) noexcept;
uint64_t skips() noexcept;
}
