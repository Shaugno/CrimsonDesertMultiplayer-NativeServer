// Game-free tests for the isolated 3663 raw packet observer.
// No game process is opened and no hook backend is linked.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include <windows.h>

#include "pkt3663/observer.h"

namespace {

int g_failures = 0;

#define CHECK(condition)                                                \
    do {                                                                \
        if (!(condition)) {                                             \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

constexpr uint64_t kStack = 0x200000001000;
constexpr uint64_t kFrame = 0x200000002000;

struct Memory {
    std::map<uint64_t, uint8_t> bytes;
    std::vector<std::pair<uint64_t, size_t>> reads;
    bool record_reads = false;
    std::atomic<int>* block = nullptr;
    uint64_t block_address = 0;

    bool read(uint64_t address, void* destination, size_t size) {
        if (record_reads) reads.push_back({address, size});
        if (block != nullptr && address == block_address) {
            std::atomic<int>* state = block;
            block = nullptr;
            state->store(1, std::memory_order_release);
            while (state->load(std::memory_order_acquire) != 2) YieldProcessor();
        }
        auto* output = static_cast<uint8_t*>(destination);
        for (size_t index = 0; index < size; ++index) {
            const auto found = bytes.find(address + index);
            if (found == bytes.end()) return false;
            output[index] = found->second;
        }
        return true;
    }
};

bool read_memory(uint64_t address, void* destination, size_t size, void* user) {
    return static_cast<Memory*>(user)->read(address, destination, size);
}

void poke(Memory& memory, uint64_t address, const void* data, size_t size) {
    const auto* source = static_cast<const uint8_t*>(data);
    for (size_t index = 0; index < size; ++index) memory.bytes[address + index] = source[index];
}

void poke_u8(Memory& memory, uint64_t address, uint8_t value) { poke(memory, address, &value, sizeof value); }
void poke_u16(Memory& memory, uint64_t address, uint16_t value) { poke(memory, address, &value, sizeof value); }

void plant_stack(Memory& memory, uint16_t length, uint8_t raw30 = 0x5A, uint16_t raw38 = 0xBEEF) {
    poke_u16(memory, kStack + pkt3663::kStackSlot28, length);
    poke_u8(memory, kStack + pkt3663::kStackSlot30, raw30);
    poke_u16(memory, kStack + pkt3663::kStackSlot38, raw38);
}

void plant_frame(Memory& memory, const std::vector<uint8_t>& bytes) {
    if (!bytes.empty()) poke(memory, kFrame, bytes.data(), bytes.size());
}

pkt3663::EntryArgs entry(uint64_t frame = kFrame) {
    pkt3663::EntryArgs args;
    args.rcx = 0x1111111111111111;
    args.rdx = 0x2222222222222222;
    args.r8 = 0x3333333333333333;
    args.r9 = frame;
    args.rsp = kStack;
    return args;
}

struct Owned {
    pkt3663::Observer* observer = nullptr;
    explicit Owned(uint32_t capacity) : observer(pkt3663::Observer::create(capacity)) {}
    ~Owned() { pkt3663::Observer::release_owner(observer); }
    pkt3663::Observer* operator->() const { return observer; }
};

bool saw_read(const Memory& memory, uint64_t address, size_t size) {
    for (const auto& read : memory.reads) {
        if (read.first == address && read.second == size) return true;
    }
    return false;
}

void test_gate_and_guarded_copy() {
    pkt3663::AdmissionGate gate;
    CHECK(gate.closed());
    CHECK(gate.open());
    CHECK(!gate.closed());
    CHECK(gate.admit());
    CHECK(gate.admitted() == 1);
    gate.leave();
    gate.close_and_wait();
    CHECK(gate.closed());
    CHECK(!gate.admit());

    uint8_t source[3] = {1, 2, 3};
    uint8_t destination[3] = {};
    CHECK(pkt3663::guarded_read(reinterpret_cast<uint64_t>(source), destination, sizeof destination));
    CHECK(std::memcmp(source, destination, sizeof source) == 0);
    CHECK(!pkt3663::guarded_read(0, destination, sizeof destination));
    CHECK(!pkt3663::guarded_read(reinterpret_cast<uint64_t>(source), destination, 0));
    CHECK(!pkt3663::guarded_read(UINT64_MAX, destination, 2));
}

void test_claims_zero_through_five_are_raw_records() {
    static_assert(pkt3663::kAllowlistCount == 0);
    Memory memory;
    Owned owned(8);
    CHECK(owned.observer != nullptr);
    CHECK(owned->arm(read_memory, &memory));

    for (uint16_t length = 0; length <= 5; ++length) {
        plant_stack(memory, length);
        std::vector<uint8_t> bytes(length);
        for (uint16_t index = 0; index < length; ++index) bytes[index] = static_cast<uint8_t>(0x40 + index);
        plant_frame(memory, bytes);
        const uint64_t frame = length == 0 ? 0 : kFrame;
        owned->on_recv(entry(frame));
    }

    for (uint16_t length = 0; length <= 5; ++length) {
        pkt3663::Record record;
        CHECK(owned->try_pop(record));
        CHECK(record.direction == pkt3663::Direction::Receive);
        CHECK(record.claimed_length == length);
        CHECK(record.copied_length == length);
        CHECK(record.status == pkt3663::RawStatus::Complete);
        CHECK(record.receive_pre_transform_possible);
        CHECK(record.has_stack_u16_28);
        CHECK(!record.has_stack_u8_30);
        CHECK(!record.has_stack_u16_38);
        CHECK(record.has_raw_word0 == (length >= 2));
        if (length >= 2) CHECK(record.raw_word0 == 0x4140);
        for (uint16_t index = 0; index < length; ++index) {
            CHECK(record.bytes[index] == static_cast<uint8_t>(0x40 + index));
        }
    }
    pkt3663::Record none;
    CHECK(!owned->try_pop(none));
    CHECK(owned->unread() == 0);
    CHECK(owned->truncated() == 0);
}

void test_every_numeric_word_is_unclassified_and_captured() {
    // Includes 3663 MoveActorReq (2790), 3576 MoveActorReq (2833), and
    // values with no known file descriptor. None is selected or named here.
    const uint16_t words[] = {2790, 2833, 0x0000, 0xFFFF};
    Memory memory;
    Owned owned(8);
    CHECK(owned->arm(read_memory, &memory));
    for (uint16_t word : words) {
        plant_stack(memory, 2, 0xA5, 0x1234);
        std::vector<uint8_t> bytes = {static_cast<uint8_t>(word & 0xFF), static_cast<uint8_t>(word >> 8)};
        plant_frame(memory, bytes);
        owned->on_send(entry());
    }

    for (uint16_t word : words) {
        pkt3663::Record record;
        CHECK(owned->try_pop(record));
        CHECK(record.direction == pkt3663::Direction::Send);
        CHECK(record.has_raw_word0);
        CHECK(record.raw_word0 == word);
        CHECK(record.status == pkt3663::RawStatus::Complete);
        CHECK(!record.receive_pre_transform_possible);
        CHECK(record.has_stack_u8_30 && record.stack_u8_30 == 0xA5);
        CHECK(record.has_stack_u16_38 && record.stack_u16_38 == 0x1234);
    }
}

void test_receive_does_not_read_send_only_slots() {
    Memory memory;
    memory.record_reads = true;
    poke_u16(memory, kStack + pkt3663::kStackSlot28, 2);
    const uint8_t bytes[] = {0x34, 0x12};
    poke(memory, kFrame, bytes, sizeof bytes);
    Owned owned(4);
    CHECK(owned->arm(read_memory, &memory));
    owned->on_recv(entry());
    pkt3663::Record record;
    CHECK(owned->try_pop(record));
    CHECK(saw_read(memory, kStack + pkt3663::kStackSlot28, 2));
    CHECK(!saw_read(memory, kStack + pkt3663::kStackSlot30, 1));
    CHECK(!saw_read(memory, kStack + pkt3663::kStackSlot38, 2));
}

void test_phase_registers_and_truncation() {
    Memory memory;
    plant_stack(memory, 5000, 0x77, 0x9876);
    std::vector<uint8_t> bytes(pkt3663::kCopyCap, 0xCC);
    bytes[0] = 0x78;
    bytes[1] = 0x56;
    plant_frame(memory, bytes);
    Owned owned(4);
    CHECK(owned->arm(read_memory, &memory));
    owned->set_phase(static_cast<uint8_t>(pkt3663::Phase::Walk));
    const pkt3663::EntryArgs args = entry();
    owned->on_send(args);

    pkt3663::Record record;
    CHECK(owned->try_pop(record));
    CHECK(record.rcx == args.rcx && record.rdx == args.rdx && record.r8 == args.r8 && record.r9 == args.r9);
    CHECK(record.phase == static_cast<uint8_t>(pkt3663::Phase::Walk));
    CHECK(record.claimed_length == 5000);
    CHECK(record.copied_length == pkt3663::kCopyCap);
    CHECK(record.status == pkt3663::RawStatus::Truncated);
    CHECK(record.has_raw_word0 && record.raw_word0 == 0x5678);
    CHECK(record.bytes[pkt3663::kCopyCap - 1] == 0xCC);
    CHECK(owned->truncated() == 1);

    owned->set_phase(200);
    plant_stack(memory, 0);
    owned->on_recv(entry(0));
    CHECK(owned->try_pop(record));
    CHECK(record.phase == static_cast<uint8_t>(pkt3663::Phase::Unlabeled));
}

void test_unreadable_and_full_ring_counters() {
    Memory missing;
    Owned unread(4);
    CHECK(unread->arm(read_memory, &missing));
    unread->on_recv(entry());
    CHECK(unread->unread() == 1);
    CHECK(unread->dropped() == 0);

    missing.record_reads = true;
    pkt3663::EntryArgs wrapping = entry();
    wrapping.rsp = UINT64_MAX - 0x10;
    unread->on_recv(wrapping);
    CHECK(unread->unread() == 2);
    CHECK(missing.reads.empty());

    plant_stack(missing, 3);
    unread->on_recv(entry());
    CHECK(unread->unread() == 3);

    Memory send_missing;
    poke_u16(send_missing, kStack + pkt3663::kStackSlot28, 0);
    unread->disarm();
    CHECK(unread->arm(read_memory, &send_missing));
    unread->on_send(entry(0));
    CHECK(unread->unread() == 4);

    Memory full;
    plant_stack(full, 0);
    Owned two(2);
    CHECK(two->arm(read_memory, &full));
    two->on_recv(entry(0));
    two->on_recv(entry(0));
    two->on_recv(entry(0));
    CHECK(two->dropped() == 1);
    pkt3663::Record record;
    CHECK(two->try_pop(record));
    CHECK(two->try_pop(record));
    CHECK(!two->try_pop(record));
}

void test_arm_bounds_and_disarm_waits_for_callback() {
    Memory memory;
    Owned one(1);
    CHECK(!one->arm(read_memory, &memory));
    CHECK(!one->queue_allocated());

    std::atomic<int> block{0};
    plant_stack(memory, 0);
    memory.block = &block;
    memory.block_address = kStack + pkt3663::kStackSlot28;
    Owned owned(4);
    CHECK(owned->arm(read_memory, &memory));
    const void* ring = owned->queue_address();
    CHECK(ring != nullptr && owned->accepting());

    std::thread callback([&] { owned->on_recv(entry(0)); });
    while (block.load(std::memory_order_acquire) != 1) YieldProcessor();
    std::atomic<bool> retired{false};
    std::thread teardown([&] {
        owned->disarm();
        retired.store(true, std::memory_order_release);
    });
    for (int spin = 0; spin < 10000; ++spin) YieldProcessor();
    CHECK(!retired.load(std::memory_order_acquire));
    CHECK(owned->queue_address() == ring);
    block.store(2, std::memory_order_release);
    callback.join();
    teardown.join();
    CHECK(retired.load(std::memory_order_acquire));
    CHECK(!owned->queue_allocated());
    CHECK(!owned->accepting());

    CHECK(owned->arm(read_memory, &memory));
    CHECK(owned->queue_allocated());
}

}  // namespace

int main() {
    test_gate_and_guarded_copy();
    test_claims_zero_through_five_are_raw_records();
    test_every_numeric_word_is_unclassified_and_captured();
    test_receive_does_not_read_send_only_slots();
    test_phase_registers_and_truncation();
    test_unreadable_and_full_ring_counters();
    test_arm_bounds_and_disarm_waits_for_callback();
    if (g_failures != 0) {
        std::printf("pkt3663 observer tests: %d failure(s)\n", g_failures);
        return 1;
    }
    std::printf("pkt3663 observer tests ok\n");
    return 0;
}
