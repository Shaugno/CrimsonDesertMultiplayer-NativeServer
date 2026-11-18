#pragma once
#include <cstdint>

namespace cdmp::session2::guest_crash3663 {
// Process-lifetime diagnostics for a verified 3663 guest. Fixture uses its
// own output directory and never loads or launches the game.
bool install(const wchar_t* directory) noexcept;
bool install_for_session();
void arm() noexcept;
enum class Direction : uint8_t { ToClient = 1, ToHost = 2 };
uint32_t packet(Direction, uint16_t opcode, uint16_t bytes) noexcept;
void returned(uint32_t ticket) noexcept;
}
