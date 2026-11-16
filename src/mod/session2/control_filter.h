#pragma once
#include <cstdint>
#include "mod/session2/scope3663.h"

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
// R4 v1: the guest stays in the host's field. This denies control requests;
// false is not general admission (the core stage/descriptor gates still apply).
constexpr bool blocked_guest_control_3663(uint16_t opcode) {
    if (blocked_native_control(opcode,{2447,2042,2004,scope3663::kSetFieldTimeReq})) return true;
    for (const auto refused:scope3663::kRefusedRequests) if(opcode==refused) return true;
    return false;
}
} // namespace cdmp::session2
