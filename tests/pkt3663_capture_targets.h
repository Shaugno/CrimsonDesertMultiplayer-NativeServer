#pragma once
// Dummy packet entries for the game-free 3663 fixture DLL. These functions
// live in the test executable, not in Crimson Desert.

#include <atomic>
#include <cstdint>

namespace pkt3663fix {

inline constexpr int kSendReturn = 0x3663;
inline constexpr int kReceiveReturn = 0x3664;

extern std::atomic<int> send_calls;
extern std::atomic<int> receive_calls;

}  // namespace pkt3663fix

extern "C" __declspec(noinline) int pkt3663_dummy_send(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* frame,
                                                       uint16_t slot28, uint8_t slot30, uint16_t slot38);
extern "C" __declspec(noinline) int pkt3663_dummy_receive(uint64_t rcx, uint64_t rdx, uint64_t r8, uint8_t* frame,
                                                          uint16_t slot28, uint64_t poison30, uint64_t poison38);

#pragma section(".pkt3663bad", read, execute)
extern "C" __declspec(allocate(".pkt3663bad")) unsigned char pkt3663_bad_entry[8];
