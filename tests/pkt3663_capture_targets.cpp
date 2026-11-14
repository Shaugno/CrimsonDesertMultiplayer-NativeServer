#include "pkt3663_capture_targets.h"

namespace pkt3663fix {

std::atomic<int> send_calls{0};
std::atomic<int> receive_calls{0};

}  // namespace pkt3663fix

#pragma section(".pkt3663bad", read, execute)
extern "C" __declspec(allocate(".pkt3663bad")) unsigned char pkt3663_bad_entry[8] = {
    0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6,
};

extern "C" __declspec(noinline) int pkt3663_dummy_send(uint64_t, uint64_t, uint64_t, uint8_t*, uint16_t, uint8_t,
                                                       uint16_t) {
    pkt3663fix::send_calls.fetch_add(1, std::memory_order_relaxed);
    return pkt3663fix::kSendReturn;
}

extern "C" __declspec(noinline) int pkt3663_dummy_receive(uint64_t, uint64_t, uint64_t, uint8_t*, uint16_t,
                                                          uint64_t, uint64_t) {
    pkt3663fix::receive_calls.fetch_add(1, std::memory_order_relaxed);
    return pkt3663fix::kReceiveReturn;
}
