#pragma once
#include <cstdint>
#include <optional>

namespace cdmp::game {
// The native 3663 initializer writes a u16 ID. Never include adjacent padding
// in that comparison. The legacy reader's u64 contract remains selectable.
template<class ReadFn>
std::optional<uint64_t> read_stat_id(uintptr_t address, uint32_t bytes, ReadFn&& read) {
    if (bytes == 2) {
        uint16_t value = 0;
        if (read(address, &value, sizeof value)) return value;
    } else if (bytes == 8) {
        uint64_t value = 0;
        if (read(address, &value, sizeof value)) return value;
    }
    return std::nullopt;
}
} // namespace cdmp::game
