#include "pkt3576_capture_targets.h"

namespace pktfix {

std::atomic<int> send_calls{0};
std::atomic<int> recv_calls{0};
std::atomic<uint64_t> send_rcx{0};
std::atomic<uint64_t> send_rdx{0};
std::atomic<uint64_t> send_r8{0};
std::atomic<uint16_t> send_slot28{0};
std::atomic<uint8_t> send_slot30{0};
std::atomic<uint16_t> send_slot38{0};
std::atomic<uint64_t> recv_rcx{0};
std::atomic<uint64_t> recv_rdx{0};
std::atomic<uint64_t> recv_r8{0};
std::atomic<uint16_t> recv_slot28{0};

}  // namespace pktfix

#pragma section(".pktbad", read, execute)
__declspec(allocate(".pktbad")) extern "C" unsigned char pkt3576_bad_entry[8] = {
    0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6, 0xD6,
};

extern "C" __declspec(noinline) int pkt3576_dummy_send(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* packet,
                                                       uint16_t slot28, uint8_t slot30, uint16_t slot38) {
    pktfix::send_calls.fetch_add(1, std::memory_order_relaxed);
    pktfix::send_rcx.store(rcx, std::memory_order_relaxed);
    pktfix::send_rdx.store(rdx, std::memory_order_relaxed);
    pktfix::send_r8.store(r8, std::memory_order_relaxed);
    pktfix::send_slot28.store(slot28, std::memory_order_relaxed);
    pktfix::send_slot30.store(slot30, std::memory_order_relaxed);
    pktfix::send_slot38.store(slot38, std::memory_order_relaxed);
    (void)packet;
    return pktfix::kSendReturn;
}

extern "C" __declspec(noinline) int pkt3576_dummy_recv(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* packet,
                                                       uint16_t slot28, uint64_t poison30, uint64_t poison38) {
    pktfix::recv_calls.fetch_add(1, std::memory_order_relaxed);
    pktfix::recv_rcx.store(rcx, std::memory_order_relaxed);
    pktfix::recv_rdx.store(rdx, std::memory_order_relaxed);
    pktfix::recv_r8.store(r8, std::memory_order_relaxed);
    pktfix::recv_slot28.store(slot28, std::memory_order_relaxed);
    (void)packet;
    (void)poison30;
    (void)poison38;
    return pktfix::kRecvReturn;
}
