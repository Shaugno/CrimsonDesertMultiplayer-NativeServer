#pragma once
#include "mod/role_logic.h"
#include <cstdint>

namespace cdmp::session2::crash3663 {
// Process-lifetime diagnostics for a verified 3663 instance. Fixture uses its
// own output directory and never loads or launches the game.
bool install(const wchar_t* directory, role::Role role) noexcept;
bool install_for_session(role::Role role);
void arm() noexcept;
enum class Direction : uint8_t { ToClient = 1, ToHost = 2 };
uint32_t packet(Direction, uint16_t opcode, uint16_t bytes) noexcept;
void returned(uint32_t ticket) noexcept;
void note_null_playback(uint32_t key, uintptr_t vtable, uint32_t host, uint32_t guest,
                        uint64_t selector, const char* identity) noexcept;
void note_playback_skip(uint32_t key) noexcept;
// R10 Step 3: the controller evidence for each sampled null-playback actor.
void note_playback_controller(uint32_t key, uint32_t controller, uint32_t local_user, bool exact_frame,
                              bool remote_controlled, bool contain_enabled) noexcept;
}
