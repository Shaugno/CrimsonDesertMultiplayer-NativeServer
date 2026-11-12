#pragma once
// Dummy send/receive entries for the game-free SafetyHook capture test.
// Their argument positions match the documented 3576 entry shape. They are
// not the game functions.

#include <atomic>
#include <cstdint>

namespace pktfix {

inline constexpr int kSendReturn = 0x3516;
inline constexpr int kRecvReturn = 0x3517;
inline constexpr uint64_t kSkipBytes = 0x51EB;

extern std::atomic<int> send_calls;
extern std::atomic<int> recv_calls;
extern std::atomic<uint64_t> send_rcx;
extern std::atomic<uint64_t> send_rdx;
extern std::atomic<uint64_t> send_r8;
extern std::atomic<uint16_t> send_slot28;
extern std::atomic<uint8_t> send_slot30;
extern std::atomic<uint16_t> send_slot38;
extern std::atomic<uint64_t> recv_rcx;
extern std::atomic<uint64_t> recv_rdx;
extern std::atomic<uint64_t> recv_r8;
extern std::atomic<uint16_t> recv_slot28;

}  // namespace pktfix

extern "C" __declspec(noinline) int pkt3576_dummy_send(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* packet,
                                                       uint16_t slot28, uint8_t slot30, uint16_t slot38);

extern "C" __declspec(noinline) int pkt3576_dummy_recv(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* packet,
                                                       uint16_t slot28, uint64_t poison30, uint64_t poison38);

// Eight illegal bytes inside this executable. Hooking them should fail.
extern "C" unsigned char pkt3576_bad_entry[8];
