// Game-free tests for the experimental 3576 packet observer.
// No game process, no sniffer, no compatibility-guard change.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include <windows.h>

#include "pkt3576/observer.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

struct Image {
    std::map<uint64_t, uint8_t> bytes;
    std::vector<std::pair<uint64_t, size_t>> reads;
    bool record = false;
    std::atomic<int>* block_phase = nullptr;
    uint64_t block_address = 0;

    bool read(uint64_t address, void* destination, size_t size) {
        if (record) reads.push_back({address, size});
        if (block_phase != nullptr && address == block_address) {
            block_phase->store(1, std::memory_order_release);
            while (block_phase->load(std::memory_order_acquire) != 2) YieldProcessor();
        }
        auto* out = static_cast<uint8_t*>(destination);
        for (size_t i = 0; i < size; ++i) {
            const auto it = bytes.find(address + i);
            if (it == bytes.end()) return false;
            out[i] = it->second;
        }
        return true;
    }
};

bool image_read(uint64_t address, void* destination, size_t size, void* user) {
    return static_cast<Image*>(user)->read(address, destination, size);
}

void poke(Image& image, uint64_t address, const void* data, size_t size) {
    const auto* src = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) image.bytes[address + i] = src[i];
}

void poke_u8(Image& image, uint64_t address, uint8_t value) { poke(image, address, &value, 1); }

void poke_u16(Image& image, uint64_t address, uint16_t value) { poke(image, address, &value, sizeof value); }

void poke_u32(Image& image, uint64_t address, uint32_t value) { poke(image, address, &value, sizeof value); }

void plant_pe(Image& image, uint64_t base, uint32_t timestamp) {
    poke_u16(image, base, 0x5A4D);
    poke_u32(image, base + 0x3C, 0x80);
    poke_u32(image, base + 0x80, 0x00004550);
    poke_u32(image, base + 0x88, timestamp);
}

void plant_entries(Image& image, uint64_t base) {
    poke(image, base + pkt3576::kSendRva, pkt3576::kSendEntry, sizeof pkt3576::kSendEntry);
    poke(image, base + pkt3576::kRecvRva, pkt3576::kRecvEntry, sizeof pkt3576::kRecvEntry);
}

Image make_image() {
    Image image;
    plant_pe(image, pkt3576::kImageBase, pkt3576::kTimestamp3576);
    plant_entries(image, pkt3576::kImageBase);
    return image;
}

constexpr uint64_t kStack = 0x200000001000;
constexpr uint64_t kPacket = 0x200000002000;

void plant_stack(Image& image, uint16_t slot28, uint8_t slot30, uint16_t slot38) {
    poke_u16(image, kStack + pkt3576::kStackSlot28, slot28);
    poke_u8(image, kStack + pkt3576::kStackSlot30, slot30);
    poke_u16(image, kStack + pkt3576::kStackSlot38, slot38);
}

void plant_packet(Image& image, const uint8_t* bytes, size_t size) { poke(image, kPacket, bytes, size); }

pkt3576::EntryArgs args_for(uint64_t rcx, uint64_t rdx, uint64_t r8) {
    pkt3576::EntryArgs args;
    args.rcx = rcx;
    args.rdx = rdx;
    args.r8 = r8;
    args.r9 = kPacket;
    args.rsp = kStack;
    return args;
}

pkt3576::Observer::Deps deps_for(Image& image, pkt3576::HookSink& hooks) {
    pkt3576::Observer::Deps deps;
    deps.read = image_read;
    deps.read_user = &image;
    deps.hooks = &hooks;
    deps.image_base = pkt3576::kImageBase;
    return deps;
}

struct Sink : pkt3576::HookSink {
    bool fail_install_send = false;
    bool fail_install_recv = false;
    bool fail_remove_send = false;
    bool fail_remove_recv = false;
    bool send_on = false;
    bool recv_on = false;
    int install_calls = 0;
    int release_calls = 0;
    std::vector<uint64_t> installed_at;
    std::vector<uint64_t> remove_at;
    pkt3576::Observer* observer = nullptr;
    uint64_t fire_address = 0;
    bool pop_during_fire = false;
    bool fired = false;
    pkt3576::Record fired_record{};
    bool fired_popped = false;
    pkt3576::EntryArgs fire_args{};
    bool fire_send = true;
    std::atomic<int>* block_phase = nullptr;
    std::atomic<bool>* remove_reached_callback = nullptr;
    std::thread worker;

    bool install(uint64_t address, pkt3576::Observer* installed) override {
        ++install_calls;
        observer = installed;
        installed_at.push_back(address);
        if (address == pkt3576::kImageBase + pkt3576::kSendRva) {
            if (fail_install_send) return false;
            send_on = true;
            return true;
        }
        if (address == pkt3576::kImageBase + pkt3576::kRecvRva) {
            if (fail_install_recv) return false;
            recv_on = true;
            return true;
        }
        return false;
    }

    bool remove(uint64_t address) override {
        remove_at.push_back(address);
        if (fire_address == address && observer != nullptr && !fired) {
            fired = true;
            worker = std::thread([this] {
                if (fire_send) observer->on_send(fire_args);
                else observer->on_recv(fire_args);
                if (pop_during_fire) fired_popped = observer->try_pop(fired_record);
            });
            if (block_phase != nullptr) {
                while (block_phase->load(std::memory_order_acquire) != 1) YieldProcessor();
                if (remove_reached_callback != nullptr) {
                    remove_reached_callback->store(true, std::memory_order_release);
                }
            } else {
                worker.join();
            }
        }
        if (address == pkt3576::kImageBase + pkt3576::kSendRva) {
            if (fail_remove_send) return false;
            send_on = false;
            return true;
        }
        if (address == pkt3576::kImageBase + pkt3576::kRecvRva) {
            if (fail_remove_recv) return false;
            recv_on = false;
            return true;
        }
        return false;
    }

    void release_callback_code() override { ++release_calls; }

    void join_worker() {
        if (worker.joinable()) worker.join();
    }
};

bool saw_read(const Image& image, uint64_t address, size_t size) {
    for (const auto& read : image.reads) {
        if (read.first == address && read.second == size) return true;
    }
    return false;
}

bool saw_address(const Image& image, uint64_t address) {
    for (const auto& read : image.reads) {
        if (read.first == address) return true;
    }
    return false;
}

void test_refuses_without_installing() {
    Sink sink;
    Image image = make_image();
    bool read_called = false;
    // Wrong mapped base is refused before any read or hook.
    {
        pkt3576::Observer observer(4);
        pkt3576::Observer::Deps deps = deps_for(image, sink);
        deps.image_base = 0x150000000;
        deps.read = [](uint64_t, void*, size_t, void* user) {
            *static_cast<bool*>(user) = true;
            return false;
        };
        deps.read_user = &read_called;
        CHECK(observer.install(deps) == pkt3576::Observer::InstallResult::Refused);
        CHECK(!read_called);
        CHECK(sink.install_calls == 0);
        CHECK(!observer.queue_allocated());
        CHECK(!observer.callback_code_released());
    }
    // 2976 timestamp, even with the 3576 entry bytes planted, installs nothing.
    {
        Image production = make_image();
        plant_pe(production, pkt3576::kImageBase, pkt3576::kTimestamp2976);
        plant_entries(production, pkt3576::kImageBase);
        pkt3576::Observer observer(4);
        CHECK(observer.install(deps_for(production, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer.queue_allocated());
    }
    // A one-byte entry mismatch installs neither site and does not allocate the ring.
    {
        Image mismatch = make_image();
        poke_u8(mismatch, pkt3576::kImageBase + pkt3576::kSendRva, 0x00);
        pkt3576::Observer observer(4);
        CHECK(observer.install(deps_for(mismatch, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer.queue_allocated());
    }
    // Unreadable receive entry.
    {
        Image missing = make_image();
        missing.bytes.erase(pkt3576::kImageBase + pkt3576::kRecvRva);
        pkt3576::Observer observer(4);
        CHECK(observer.install(deps_for(missing, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer.queue_allocated());
    }
    // Capacity zero and a null sink refuse before hooks.
    {
        pkt3576::Observer observer(0);
        CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
    }
}

void test_partial_install_rolls_back() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_send = true;
    sink.fire_address = pkt3576::kImageBase + pkt3576::kRecvRva;
    sink.fire_send = false;
    sink.pop_during_fire = true;
    plant_stack(image, 4, 0x11, 0x22);
    const uint8_t packet[4] = {0x10, 0x20, 0x30, 0x40};
    plant_packet(image, packet, sizeof packet);
    sink.fire_args = args_for(0x111, 0x222, 0x4);

    pkt3576::Observer observer(4);
    const auto result = observer.install(deps_for(image, sink));
    CHECK(result == pkt3576::Observer::InstallResult::RolledBack);
    CHECK(sink.install_calls == 2);
    CHECK(sink.installed_at.size() == 2);
    CHECK(sink.installed_at[0] == pkt3576::kImageBase + pkt3576::kRecvRva);
    CHECK(sink.installed_at[1] == pkt3576::kImageBase + pkt3576::kSendRva);
    CHECK(!sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 1);
    CHECK(observer.callback_code_released());
    CHECK(!observer.queue_allocated());
    CHECK(!observer.recv_installed());
    CHECK(!observer.send_installed());
    CHECK(sink.fired);
    CHECK(sink.fired_popped);
    CHECK(sink.fired_record.direction == pkt3576::Direction::Receive);
    CHECK(sink.fired_record.rcx == 0x111);
    CHECK(sink.fired_record.prefix_len == 4);
    CHECK(sink.fired_record.prefix[0] == 0x10);
    CHECK(!sink.fired_record.has_stack_u8_30);
    CHECK(!sink.fired_record.has_stack_u16_38);
    sink.join_worker();
}

void test_partial_install_retains_on_failed_removal() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_send = true;
    sink.fail_remove_recv = true;
    plant_stack(image, 2, 0, 0);
    const uint8_t packet[2] = {0xAB, 0xCD};
    plant_packet(image, packet, sizeof packet);

    pkt3576::Observer observer(4);
    const void* none = observer.queue_address();
    CHECK(none == nullptr);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Retained);
    CHECK(sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 0);
    CHECK(!observer.callback_code_released());
    CHECK(observer.queue_allocated());
    const void* kept = observer.queue_address();
    CHECK(kept != nullptr);

    observer.on_recv(args_for(0x55, 0x66, 0x77));
    pkt3576::Record record;
    CHECK(observer.try_pop(record));
    CHECK(record.direction == pkt3576::Direction::Receive);
    CHECK(record.r8 == 0x77);
    CHECK(record.stack_u16_28 == 2);
    CHECK(record.prefix_len == 2);
    CHECK(record.prefix[0] == 0xAB);
    CHECK(record.prefix[1] == 0xCD);
    CHECK(!record.has_stack_u8_30);
    CHECK(!record.has_stack_u16_38);
    CHECK(observer.queue_address() == kept);
    CHECK(observer.unread() == 0);
}

void test_first_install_failure_frees() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_recv = true;
    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Refused);
    CHECK(sink.install_calls == 1);
    CHECK(sink.installed_at[0] == pkt3576::kImageBase + pkt3576::kRecvRva);
    CHECK(!sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 0);
    CHECK(!observer.queue_allocated());
    CHECK(sink.remove_at.empty());
}

void test_failed_removal_retains_resources() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 1, 0x5A, 0x1234);
    const uint8_t packet[1] = {0xEE};
    plant_packet(image, packet, sizeof packet);
    sink.fire_address = pkt3576::kImageBase + pkt3576::kRecvRva;
    sink.fire_send = false;
    sink.fire_args = args_for(1, 2, 3);
    sink.fail_remove_recv = true;

    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    CHECK(sink.release_calls == 0);
    const void* queue = observer.queue_address();

    CHECK(observer.shutdown() == pkt3576::Observer::ShutdownResult::Retained);
    CHECK(sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(observer.recv_installed());
    CHECK(!observer.send_installed());
    CHECK(sink.release_calls == 0);
    CHECK(!observer.callback_code_released());
    CHECK(observer.queue_address() == queue);
    CHECK(observer.accepting());
    sink.join_worker();

    pkt3576::Record during;
    CHECK(observer.try_pop(during));
    CHECK(during.direction == pkt3576::Direction::Receive);
    CHECK(during.prefix_len == 1);
    CHECK(during.prefix[0] == 0xEE);
    CHECK(!during.has_stack_u8_30);

    observer.on_recv(args_for(9, 8, 7));
    pkt3576::Record later;
    CHECK(observer.try_pop(later));
    CHECK(later.rcx == 9);
    CHECK(later.prefix_len == 1);
    CHECK(observer.queue_address() == queue);
}

void test_one_removal_fails_other_stays_removed() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 0, 1, 2);
    sink.fail_remove_send = true;
    pkt3576::Observer observer(2);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    const void* queue = observer.queue_address();
    CHECK(observer.shutdown() == pkt3576::Observer::ShutdownResult::Retained);
    CHECK(!sink.recv_on);
    CHECK(sink.send_on);
    CHECK(!observer.recv_installed());
    CHECK(observer.send_installed());
    CHECK(sink.release_calls == 0);
    CHECK(observer.queue_address() == queue);

    observer.on_send(args_for(3, 4, 5));
    pkt3576::Record record;
    CHECK(observer.try_pop(record));
    CHECK(record.direction == pkt3576::Direction::Send);
    CHECK(record.prefix_len == 0);
    CHECK(record.has_stack_u16_28);
    CHECK(record.stack_u16_28 == 0);
    CHECK(record.has_stack_u8_30);
    CHECK(record.stack_u8_30 == 1);
    CHECK(record.has_stack_u16_38);
    CHECK(record.stack_u16_38 == 2);
}

void test_callback_during_shutdown_then_free() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 3, 0x44, 0x5555);
    const uint8_t packet[3] = {1, 2, 3};
    plant_packet(image, packet, sizeof packet);
    sink.fire_address = pkt3576::kImageBase + pkt3576::kSendRva;
    sink.fire_send = true;
    sink.pop_during_fire = true;
    sink.fire_args = args_for(0x10, 0x20, 0x30);

    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    const uint64_t dropped_before = observer.dropped();
    CHECK(observer.shutdown() == pkt3576::Observer::ShutdownResult::Removed);
    sink.join_worker();
    CHECK(sink.fired_popped);
    CHECK(sink.fired_record.direction == pkt3576::Direction::Send);
    CHECK(sink.fired_record.rcx == 0x10);
    CHECK(sink.fired_record.r8 == 0x30);
    CHECK(sink.fired_record.stack_u16_28 == 3);
    CHECK(sink.fired_record.stack_u8_30 == 0x44);
    CHECK(sink.fired_record.stack_u16_38 == 0x5555);
    CHECK(sink.fired_record.prefix_len == 3);
    CHECK(sink.fired_record.prefix[2] == 3);
    CHECK(!observer.queue_allocated());
    CHECK(observer.queue_address() == nullptr);
    CHECK(sink.release_calls == 1);
    CHECK(observer.callback_code_released());
    CHECK(!observer.accepting());
    CHECK(!sink.send_on);
    CHECK(!sink.recv_on);

    observer.on_send(sink.fire_args);
    CHECK(observer.dropped() == dropped_before + 1);
    pkt3576::Record ignored;
    CHECK(!observer.try_pop(ignored));
}

void test_inflight_callback_drained_before_free() {
    Image image = make_image();
    plant_stack(image, 4, 9, 8);
    const uint8_t packet[4] = {9, 8, 7, 6};
    plant_packet(image, packet, sizeof packet);

    std::atomic<int> phase{0};
    std::atomic<bool> remove_reached{false};
    image.block_phase = &phase;
    image.block_address = kPacket;

    Sink sink;
    sink.fire_address = pkt3576::kImageBase + pkt3576::kRecvRva;
    sink.fire_send = false;
    sink.fire_args = args_for(1, 2, 3);
    sink.block_phase = &phase;
    sink.remove_reached_callback = &remove_reached;

    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::thread releaser([&] {
        while (!remove_reached.load(std::memory_order_acquire)) YieldProcessor();
        phase.store(2, std::memory_order_release);
    });
    CHECK(observer.shutdown() == pkt3576::Observer::ShutdownResult::Removed);
    releaser.join();
    sink.join_worker();
    CHECK(remove_reached.load());
    CHECK(observer.dropped() == 0);
    CHECK(observer.unread() == 0);
    CHECK(!observer.queue_allocated());
    CHECK(sink.release_calls == 1);
}

void test_full_queue() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 1, 2, 3);
    const uint8_t packet[1] = {0x01};
    plant_packet(image, packet, sizeof packet);
    pkt3576::Observer observer(2);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    observer.on_send(args_for(1, 0, 0));
    observer.on_recv(args_for(2, 0, 0));
    observer.on_send(args_for(3, 0, 0));
    observer.on_send(args_for(4, 0, 0));
    observer.on_recv(args_for(5, 0, 0));
    CHECK(observer.dropped() == 3);
    CHECK(observer.unread() == 0);

    pkt3576::Record first;
    pkt3576::Record second;
    pkt3576::Record extra;
    CHECK(observer.try_pop(first));
    CHECK(observer.try_pop(second));
    CHECK(!observer.try_pop(extra));
    CHECK(first.rcx == 1);
    CHECK(first.direction == pkt3576::Direction::Send);
    CHECK(second.rcx == 2);
    CHECK(second.direction == pkt3576::Direction::Receive);
    CHECK(!second.has_stack_u8_30);
    CHECK(first.has_stack_u8_30);

    observer.on_send(args_for(6, 0, 0));
    pkt3576::Record third;
    CHECK(observer.try_pop(third));
    CHECK(third.rcx == 6);
    CHECK(observer.dropped() == 3);
}

void test_concurrent_producers() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 4, 0xAB, 0xCDEF);
    const uint8_t packet[4] = {0x01, 0x02, 0x03, 0x04};
    plant_packet(image, packet, sizeof packet);

    constexpr int kThreads = 4;
    constexpr int kEach = 250;
    constexpr int kTotal = kThreads * kEach;
    pkt3576::Observer observer(1024);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::atomic<bool> producers_done{false};
    std::atomic<int> popped{0};
    std::thread consumer([&] {
        pkt3576::Record record;
        while (!producers_done.load(std::memory_order_acquire)) {
            if (observer.try_pop(record)) {
                if (record.direction == pkt3576::Direction::Receive) {
                    if (record.has_stack_u8_30 || record.has_stack_u16_38) popped.store(-1000000);
                }
                popped.fetch_add(1, std::memory_order_relaxed);
            }
        }
        while (observer.try_pop(record)) {
            if (record.direction == pkt3576::Direction::Receive &&
                (record.has_stack_u8_30 || record.has_stack_u16_38)) {
                popped.store(-1000000);
            }
            popped.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::vector<std::thread> producers;
    for (int thread = 0; thread < kThreads; ++thread) {
        producers.emplace_back([&, thread] {
            for (int i = 0; i < kEach; ++i) {
                const auto args = args_for(static_cast<uint64_t>(thread), static_cast<uint64_t>(i), 4);
                if ((thread + i) % 2 == 0) observer.on_send(args);
                else observer.on_recv(args);
            }
        });
    }
    for (auto& producer : producers) producer.join();
    producers_done.store(true, std::memory_order_release);
    consumer.join();

    CHECK(observer.unread() == 0);
    CHECK(observer.dropped() == 0);
    CHECK(popped.load() == kTotal);
}

void test_concurrent_full_queue() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 1, 1, 1);
    const uint8_t packet[1] = {0x7F};
    plant_packet(image, packet, sizeof packet);

    constexpr int kThreads = 4;
    constexpr int kEach = 50;
    constexpr int kTotal = kThreads * kEach;
    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::vector<std::thread> producers;
    for (int thread = 0; thread < kThreads; ++thread) {
        producers.emplace_back([&, thread] {
            for (int i = 0; i < kEach; ++i) {
                observer.on_send(args_for(static_cast<uint64_t>(thread), static_cast<uint64_t>(i), 4));
            }
        });
    }
    for (auto& producer : producers) producer.join();

    int popped = 0;
    pkt3576::Record record;
    while (observer.try_pop(record)) {
        CHECK(record.prefix_len == 1);
        CHECK(record.prefix[0] == 0x7F);
        CHECK(record.r8 == 4);
        ++popped;
    }
    CHECK(popped == 4);
    CHECK(observer.dropped() == static_cast<uint64_t>(kTotal - popped));
    CHECK(observer.unread() == 0);
    CHECK(popped + static_cast<int>(observer.dropped()) == kTotal);
}

void test_short_prefix_and_raw_slots() {
    Image image = make_image();
    Sink sink;
    uint8_t packet[32];
    for (uint8_t i = 0; i < 32; ++i) packet[i] = static_cast<uint8_t>(0xA0 + i);
    plant_packet(image, packet, sizeof packet);
    pkt3576::Observer observer(8);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    image.record = true;
    plant_stack(image, 0, 0x3C, 0x0A11);
    observer.on_send(args_for(10, 20, 4));
    CHECK(!saw_address(image, kPacket));
    pkt3576::Record zero;
    CHECK(observer.try_pop(zero));
    CHECK(zero.prefix_len == 0);
    CHECK(zero.stack_u16_28 == 0);
    CHECK(zero.stack_u8_30 == 0x3C);
    CHECK(zero.stack_u16_38 == 0x0A11);
    CHECK(zero.r8 == 4);
    CHECK(zero.r9 == kPacket);

    image.reads.clear();
    plant_stack(image, 1, 0x01, 0x0002);
    observer.on_send(args_for(1, 2, 3));
    CHECK(saw_read(image, kPacket, 1));
    CHECK(!saw_read(image, kPacket, 2));
    pkt3576::Record one;
    CHECK(observer.try_pop(one));
    CHECK(one.prefix_len == 1);
    CHECK(one.prefix[0] == 0xA0);
    CHECK(one.prefix[1] == 0);

    image.reads.clear();
    plant_stack(image, 100, 0x02, 0x0003);
    observer.on_recv(args_for(8, 9, 10));
    CHECK(saw_read(image, kPacket, pkt3576::kPrefixCap));
    CHECK(!saw_read(image, kPacket, 100));
    CHECK(!saw_read(image, kPacket, 2));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot38));
    CHECK(saw_read(image, kStack + pkt3576::kStackSlot28, 2));
    pkt3576::Record capped;
    CHECK(observer.try_pop(capped));
    CHECK(capped.direction == pkt3576::Direction::Receive);
    CHECK(capped.prefix_len == pkt3576::kPrefixCap);
    CHECK(capped.prefix[0] == 0xA0);
    CHECK(capped.prefix[15] == static_cast<uint8_t>(0xA0 + 15));
    CHECK(!capped.has_stack_u8_30);
    CHECK(!capped.has_stack_u16_38);
    CHECK(capped.stack_u16_28 == 100);
    CHECK(observer.unread() == 0);
    CHECK(observer.dropped() == 0);
}

void test_unreadable_buffers() {
    Image image = make_image();
    Sink sink;
    pkt3576::Observer observer(4);
    CHECK(observer.install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    image.record = true;

    plant_stack(image, 8, 1, 2);
    observer.on_send(args_for(1, 2, 3));
    CHECK(observer.unread() == 1);
    CHECK(saw_read(image, kPacket, 8));
    pkt3576::Record none;
    CHECK(!observer.try_pop(none));

    image.reads.clear();
    image.bytes.erase(kStack + pkt3576::kStackSlot28);
    image.bytes.erase(kStack + pkt3576::kStackSlot28 + 1);
    observer.on_recv(args_for(1, 2, 3));
    CHECK(observer.unread() == 2);
    CHECK(!saw_address(image, kPacket));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));
    CHECK(!observer.try_pop(none));

    image.reads.clear();
    plant_stack(image, 4, 1, 2);
    image.bytes.erase(kStack + pkt3576::kStackSlot30);
    const uint8_t packet[4] = {1, 2, 3, 4};
    plant_packet(image, packet, sizeof packet);
    observer.on_send(args_for(1, 2, 3));
    CHECK(observer.unread() == 3);
    CHECK(!saw_address(image, kPacket));
    CHECK(!observer.try_pop(none));
    CHECK(observer.dropped() == 0);

    uint8_t local[4] = {4, 5, 6, 7};
    uint8_t copied[4] = {};
    CHECK(pkt3576::guarded_read(reinterpret_cast<uint64_t>(local), copied, 4));
    CHECK(std::memcmp(local, copied, 4) == 0);
    CHECK(pkt3576::guarded_read(reinterpret_cast<uint64_t>(local), copied, 1));
    CHECK(!pkt3576::guarded_read(0, copied, 4));
    CHECK(!pkt3576::guarded_read(reinterpret_cast<uint64_t>(local), nullptr, 4));
    CHECK(!pkt3576::guarded_read(reinterpret_cast<uint64_t>(local), copied, 0));
    CHECK(!pkt3576::guarded_read(~uint64_t{0} - 4, copied, 16));
    uint8_t one = 0xFF;
    CHECK(!pkt3576::guarded_read(1, &one, 1));
    CHECK(one == 0xFF);
}

}  // namespace

int main() {
    test_refuses_without_installing();
    test_partial_install_rolls_back();
    test_partial_install_retains_on_failed_removal();
    test_first_install_failure_frees();
    test_failed_removal_retains_resources();
    test_one_removal_fails_other_stays_removed();
    test_callback_during_shutdown_then_free();
    test_inflight_callback_drained_before_free();
    test_full_queue();
    test_concurrent_producers();
    test_concurrent_full_queue();
    test_short_prefix_and_raw_slots();
    test_unreadable_buffers();
    if (g_failures) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::puts("pkt3576 tests ok");
    return 0;
}
