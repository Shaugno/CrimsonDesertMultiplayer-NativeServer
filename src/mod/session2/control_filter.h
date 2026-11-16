#pragma once
#include <cstdint>

namespace cdmp::session2 {
struct NativeControlOpcodes {
    uint16_t reserve_shutdown = 0, shutdown_timer = 0, pointer_request = 0, set_field_time = 0;
};

constexpr bool blocked_native_control(uint16_t opcode, NativeControlOpcodes native) {
    if (!native.reserve_shutdown || !native.shutdown_timer || !native.pointer_request || !native.set_field_time)
        return true; // An incomplete build table cannot admit guest requests.
    // Conservative tunnel policy: reject the whole interval, including any
    // unused/reserved values. This makes no claim about individual handlers.
    const bool local_connection_policy = opcode >= 1001 && opcode <= 1008;
    return local_connection_policy || opcode == native.reserve_shutdown ||
        opcode == native.shutdown_timer || opcode == native.pointer_request ||
        opcode == native.set_field_time;
}
} // namespace cdmp::session2
