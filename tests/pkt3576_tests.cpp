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

#include "pkt3576/admission_gate.h"
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
        // One shot. observe() reads a selected packet three times at the same
        // address (opcode, header, body). Re-arming this wait would leave the
        // callback admitted forever and stall close_and_wait. The first read
        // still holds admission through the later reads.
        if (block_phase != nullptr && address == block_address) {
            std::atomic<int>* phase = block_phase;
            block_phase = nullptr;
            phase->store(1, std::memory_order_release);
            while (phase->load(std::memory_order_acquire) != 2) YieldProcessor();
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

// Ordinary 3576 frame: opcode, one header byte, payload length at offset 3.
// claimed >= 5 makes payload_length + 5 equal the claim.
void plant_frame(Image& image, uint16_t opcode, uint16_t claimed, uint8_t fill) {
    if (claimed == 0) return;
    std::vector<uint8_t> bytes(claimed, fill);
    bytes[0] = static_cast<uint8_t>(opcode & 0xFF);
    if (claimed >= 2) bytes[1] = static_cast<uint8_t>((opcode >> 8) & 0xFF);
    if (claimed >= 5) {
        const uint16_t payload = static_cast<uint16_t>(claimed - 5);
        bytes[2] = 0x5A;
        bytes[3] = static_cast<uint8_t>(payload & 0xFF);
        bytes[4] = static_cast<uint8_t>((payload >> 8) & 0xFF);
    }
    plant_packet(image, bytes.data(), bytes.size());
}

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

// Deletes the observer when the hook no longer owns it. A failed removal
// keeps the heap object alive for the sink.
struct Obs {
    pkt3576::Observer* p = nullptr;
    explicit Obs(uint32_t capacity) : p(pkt3576::Observer::create(capacity)) {}
    ~Obs() { pkt3576::Observer::release_owner(p); }
    pkt3576::Observer* operator->() const { return p; }
};

void test_refuses_without_installing() {
    Sink sink;
    Image image = make_image();
    bool read_called = false;
    // Wrong mapped base is refused before any read or hook.
    {
        Obs observer(4);
        pkt3576::Observer::Deps deps = deps_for(image, sink);
        deps.image_base = 0x150000000;
        deps.read = [](uint64_t, void*, size_t, void* user) {
            *static_cast<bool*>(user) = true;
            return false;
        };
        deps.read_user = &read_called;
        CHECK(observer->install(deps) == pkt3576::Observer::InstallResult::Refused);
        CHECK(!read_called);
        CHECK(sink.install_calls == 0);
        CHECK(!observer->queue_allocated());
        CHECK(!observer->callback_code_released());
    }
    // 2976 timestamp, even with the 3576 entry bytes planted, installs nothing.
    {
        Image production = make_image();
        plant_pe(production, pkt3576::kImageBase, pkt3576::kTimestamp2976);
        plant_entries(production, pkt3576::kImageBase);
        Obs observer(4);
        CHECK(observer->install(deps_for(production, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer->queue_allocated());
    }
    // A one-byte entry mismatch installs neither site and does not allocate the ring.
    {
        Image mismatch = make_image();
        poke_u8(mismatch, pkt3576::kImageBase + pkt3576::kSendRva, 0x00);
        Obs observer(4);
        CHECK(observer->install(deps_for(mismatch, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer->queue_allocated());
    }
    // Unreadable receive entry.
    {
        Image missing = make_image();
        missing.bytes.erase(pkt3576::kImageBase + pkt3576::kRecvRva);
        Obs observer(4);
        CHECK(observer->install(deps_for(missing, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!observer->queue_allocated());
    }
    // Capacity 0 and 1 are refused before any hook or ring allocation.
    {
        Obs zero(0);
        Obs one(1);
        CHECK(zero->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(one->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Refused);
        CHECK(sink.install_calls == 0);
        CHECK(!zero->queue_allocated());
        CHECK(!one->queue_allocated());
    }
}

void test_capacity_two_keeps_both_records() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 1, 2);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    Obs observer(2);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    observer->on_send(args_for(0x11, 0, 0));
    observer->on_send(args_for(0x22, 0, 0));
    observer->on_send(args_for(0x33, 0, 0));
    CHECK(observer->dropped() == 1);
    pkt3576::Record first;
    pkt3576::Record second;
    pkt3576::Record extra;
    CHECK(observer->try_pop(first));
    CHECK(observer->try_pop(second));
    CHECK(!observer->try_pop(extra));
    CHECK(first.rcx == 0x11);
    CHECK(second.rcx == 0x22);
}

void test_consumer_shutdown_waits_for_pop() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 1, 2);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    observer->on_send(args_for(0xA11, 0, 0));

    std::atomic<int> phase{0};
    observer->set_consumer_hold(
        [](void* user) {
            auto* gate = static_cast<std::atomic<int>*>(user);
            gate->store(1, std::memory_order_release);
            while (gate->load(std::memory_order_acquire) != 2) YieldProcessor();
        },
        &phase);

    pkt3576::Record popped{};
    std::atomic<bool> pop_ok{false};
    std::thread consumer([&] { pop_ok.store(observer->try_pop(popped), std::memory_order_release); });
    while (phase.load(std::memory_order_acquire) != 1) YieldProcessor();

    std::atomic<bool> started{false};
    std::atomic<bool> done{false};
    pkt3576::Observer::ShutdownResult shutdown_result = pkt3576::Observer::ShutdownResult::Idle;
    std::thread stopper([&] {
        started.store(true, std::memory_order_release);
        shutdown_result = observer->shutdown();
        done.store(true, std::memory_order_release);
    });
    while (!started.load(std::memory_order_acquire)) YieldProcessor();

    bool finished_early = false;
    for (int i = 0; i < 1000000; ++i) {
        if (done.load(std::memory_order_acquire)) {
            finished_early = true;
            break;
        }
        YieldProcessor();
    }
    CHECK(!finished_early);
    CHECK(observer->queue_allocated());
    phase.store(2, std::memory_order_release);
    consumer.join();
    stopper.join();
    CHECK(pop_ok.load());
    CHECK(popped.rcx == 0xA11);
    CHECK(shutdown_result == pkt3576::Observer::ShutdownResult::Removed);
    CHECK(!observer->queue_allocated());
    CHECK(observer->callback_code_released());
    CHECK(sink.release_calls == 1);
    pkt3576::Record late;
    CHECK(!observer->try_pop(late));
}

void test_failed_removal_keeps_ownership() {
    Image image = make_image();
    Sink sink;
    sink.fail_remove_recv = true;
    sink.fail_remove_send = true;
    plant_stack(image, 5, 0x7, 0x8);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);

    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    const void* queue = observer->queue_address();
    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Retained);

    pkt3576::Observer* kept = observer.p;
    CHECK(!pkt3576::Observer::release_owner(observer.p));
    CHECK(observer.p == kept);
    CHECK(kept->queue_address() == queue);
    CHECK(kept->queue_allocated());
    CHECK(kept->accepting());
    CHECK(kept->recv_installed());
    CHECK(kept->send_installed());
    CHECK(!kept->callback_code_released());
    CHECK(sink.release_calls == 0);
    CHECK(sink.recv_on);
    CHECK(sink.send_on);

    kept->on_send(args_for(0x42, 0, 0));
    pkt3576::Record record;
    CHECK(kept->try_pop(record));
    CHECK(record.rcx == 0x42);
    CHECK(record.copied_length == 5);
    CHECK(record.claimed_length == 5);
    CHECK(record.status == pkt3576::CopyStatus::Complete);
    CHECK(record.bytes[0] == 0x11);
    CHECK(record.bytes[1] == 0x0B);
    CHECK(record.bytes[2] == 0x5A);
    CHECK(record.has_stack_u8_30);
    CHECK(record.stack_u8_30 == 0x7);
    CHECK(kept->queue_address() == queue);

    std::atomic<bool> run{true};
    std::thread producer([&] {
        while (run.load(std::memory_order_acquire)) kept->on_recv(args_for(0x99, 0, 0));
    });
    CHECK(!pkt3576::Observer::release_owner(observer.p));
    CHECK(observer.p == kept);
    CHECK(kept->queue_address() == queue);
    CHECK(kept->accepting());
    CHECK(!kept->callback_code_released());
    run.store(false, std::memory_order_release);
    producer.join();
    pkt3576::Record extra;
    while (kept->try_pop(extra)) {
        CHECK(extra.direction == pkt3576::Direction::Receive);
        CHECK(extra.copied_length == 5);
        CHECK(extra.status == pkt3576::CopyStatus::Complete);
        CHECK(!extra.has_stack_u8_30);
    }
    CHECK(kept->unread() == 0);
    CHECK(sink.release_calls == 0);
}

void test_partial_install_rolls_back() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_send = true;
    sink.fire_address = pkt3576::kImageBase + pkt3576::kRecvRva;
    sink.fire_send = false;
    sink.pop_during_fire = true;
    plant_stack(image, 5, 0x11, 0x22);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    sink.fire_args = args_for(0x111, 0x222, 0x4);

    Obs observer(4);
    const auto result = observer->install(deps_for(image, sink));
    CHECK(result == pkt3576::Observer::InstallResult::RolledBack);
    CHECK(sink.install_calls == 2);
    CHECK(sink.installed_at.size() == 2);
    CHECK(sink.installed_at[0] == pkt3576::kImageBase + pkt3576::kRecvRva);
    CHECK(sink.installed_at[1] == pkt3576::kImageBase + pkt3576::kSendRva);
    CHECK(!sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 1);
    CHECK(observer->callback_code_released());
    CHECK(!observer->queue_allocated());
    CHECK(!observer->recv_installed());
    CHECK(!observer->send_installed());
    CHECK(sink.fired);
    CHECK(sink.fired_popped);
    CHECK(sink.fired_record.direction == pkt3576::Direction::Receive);
    CHECK(sink.fired_record.rcx == 0x111);
    CHECK(sink.fired_record.copied_length == 5);
    CHECK(sink.fired_record.status == pkt3576::CopyStatus::Complete);
    CHECK(sink.fired_record.bytes[0] == 0x11);
    CHECK(!sink.fired_record.has_stack_u8_30);
    CHECK(!sink.fired_record.has_stack_u16_38);
    sink.join_worker();
}

void test_partial_install_retains_on_failed_removal() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_send = true;
    sink.fail_remove_recv = true;
    plant_stack(image, 5, 0, 0);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);

    Obs observer(4);
    const void* none = observer->queue_address();
    CHECK(none == nullptr);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Retained);
    CHECK(sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 0);
    CHECK(!observer->callback_code_released());
    CHECK(observer->queue_allocated());
    const void* kept = observer->queue_address();
    CHECK(kept != nullptr);

    observer->on_recv(args_for(0x55, 0x66, 0x77));
    pkt3576::Record record;
    CHECK(observer->try_pop(record));
    CHECK(record.direction == pkt3576::Direction::Receive);
    CHECK(record.r8 == 0x77);
    CHECK(record.stack_u16_28 == 5);
    CHECK(record.copied_length == 5);
    CHECK(record.opcode == pkt3576::kMoveActorReq);
    CHECK(record.bytes[0] == 0x11);
    CHECK(record.bytes[1] == 0x0B);
    CHECK(!record.has_stack_u8_30);
    CHECK(!record.has_stack_u16_38);
    CHECK(observer->queue_address() == kept);
    CHECK(observer->unread() == 0);
}

void test_first_install_failure_frees() {
    Image image = make_image();
    Sink sink;
    sink.fail_install_recv = true;
    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Refused);
    CHECK(sink.install_calls == 1);
    CHECK(sink.installed_at[0] == pkt3576::kImageBase + pkt3576::kRecvRva);
    CHECK(!sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(sink.release_calls == 0);
    CHECK(!observer->queue_allocated());
    CHECK(sink.remove_at.empty());
}

void test_failed_removal_retains_resources() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 0x5A, 0x1234);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    sink.fire_address = pkt3576::kImageBase + pkt3576::kRecvRva;
    sink.fire_send = false;
    sink.fire_args = args_for(1, 2, 3);
    sink.fail_remove_recv = true;

    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    CHECK(sink.release_calls == 0);
    const void* queue = observer->queue_address();

    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Retained);
    CHECK(sink.recv_on);
    CHECK(!sink.send_on);
    CHECK(observer->recv_installed());
    CHECK(!observer->send_installed());
    CHECK(sink.release_calls == 0);
    CHECK(!observer->callback_code_released());
    CHECK(observer->queue_address() == queue);
    CHECK(observer->accepting());
    sink.join_worker();

    pkt3576::Record during;
    CHECK(observer->try_pop(during));
    CHECK(during.direction == pkt3576::Direction::Receive);
    CHECK(during.copied_length == 5);
    CHECK(during.status == pkt3576::CopyStatus::Complete);
    CHECK(during.bytes[0] == 0x11);
    CHECK(!during.has_stack_u8_30);

    observer->on_recv(args_for(9, 8, 7));
    pkt3576::Record later;
    CHECK(observer->try_pop(later));
    CHECK(later.rcx == 9);
    CHECK(later.copied_length == 5);
    CHECK(observer->queue_address() == queue);
}

void test_one_removal_fails_other_stays_removed() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 1, 2);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    sink.fail_remove_send = true;
    Obs observer(2);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    const void* queue = observer->queue_address();
    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Retained);
    CHECK(!sink.recv_on);
    CHECK(sink.send_on);
    CHECK(!observer->recv_installed());
    CHECK(observer->send_installed());
    CHECK(sink.release_calls == 0);
    CHECK(observer->queue_address() == queue);

    observer->on_send(args_for(3, 4, 5));
    pkt3576::Record record;
    CHECK(observer->try_pop(record));
    CHECK(record.direction == pkt3576::Direction::Send);
    CHECK(record.copied_length == 5);
    CHECK(record.status == pkt3576::CopyStatus::Complete);
    CHECK(record.has_stack_u16_28);
    CHECK(record.stack_u16_28 == 5);
    CHECK(record.has_stack_u8_30);
    CHECK(record.stack_u8_30 == 1);
    CHECK(record.has_stack_u16_38);
    CHECK(record.stack_u16_38 == 2);
    plant_stack(image, 0, 9, 9);
    const uint64_t filtered_before = observer->filtered();
    observer->on_send(args_for(3, 4, 5));
    CHECK(observer->filtered() == filtered_before + 1);
    CHECK(!observer->try_pop(record));
    CHECK(observer->queue_address() == queue);
}

void test_callback_during_shutdown_then_free() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 0x44, 0x5555);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    sink.fire_address = pkt3576::kImageBase + pkt3576::kSendRva;
    sink.fire_send = true;
    sink.pop_during_fire = true;
    sink.fire_args = args_for(0x10, 0x20, 0x30);

    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    const uint64_t dropped_before = observer->dropped();
    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Removed);
    sink.join_worker();
    CHECK(sink.fired_popped);
    CHECK(sink.fired_record.direction == pkt3576::Direction::Send);
    CHECK(sink.fired_record.rcx == 0x10);
    CHECK(sink.fired_record.r8 == 0x30);
    CHECK(sink.fired_record.stack_u16_28 == 5);
    CHECK(sink.fired_record.stack_u8_30 == 0x44);
    CHECK(sink.fired_record.stack_u16_38 == 0x5555);
    CHECK(sink.fired_record.copied_length == 5);
    CHECK(sink.fired_record.status == pkt3576::CopyStatus::Complete);
    CHECK(sink.fired_record.bytes[2] == 0x5A);
    CHECK(sink.fired_record.qpc_ticks != 0);
    CHECK(!observer->queue_allocated());
    CHECK(observer->queue_address() == nullptr);
    CHECK(sink.release_calls == 1);
    CHECK(observer->callback_code_released());
    CHECK(!observer->accepting());
    CHECK(!sink.send_on);
    CHECK(!sink.recv_on);

    observer->on_send(sink.fire_args);
    CHECK(observer->dropped() == dropped_before + 1);
    pkt3576::Record ignored;
    CHECK(!observer->try_pop(ignored));
}

void test_inflight_callback_drained_before_free() {
    Image image = make_image();
    plant_stack(image, 8, 9, 8);
    plant_frame(image, pkt3576::kMoveActorReq, 8, 0x40);

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

    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::thread releaser([&] {
        while (!remove_reached.load(std::memory_order_acquire)) YieldProcessor();
        phase.store(2, std::memory_order_release);
    });
    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Removed);
    releaser.join();
    sink.join_worker();
    CHECK(remove_reached.load());
    CHECK(observer->dropped() == 0);
    CHECK(observer->unread() == 0);
    CHECK(!observer->queue_allocated());
    CHECK(sink.release_calls == 1);
}

void test_full_queue() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 2, 3);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    Obs observer(2);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    observer->on_send(args_for(1, 0, 0));
    observer->on_recv(args_for(2, 0, 0));
    observer->on_send(args_for(3, 0, 0));
    observer->on_send(args_for(4, 0, 0));
    observer->on_recv(args_for(5, 0, 0));
    CHECK(observer->dropped() == 3);
    CHECK(observer->unread() == 0);
    CHECK(observer->filtered() == 0);

    pkt3576::Record first;
    pkt3576::Record second;
    pkt3576::Record extra;
    CHECK(observer->try_pop(first));
    CHECK(observer->try_pop(second));
    CHECK(!observer->try_pop(extra));
    CHECK(first.rcx == 1);
    CHECK(first.direction == pkt3576::Direction::Send);
    CHECK(second.rcx == 2);
    CHECK(second.direction == pkt3576::Direction::Receive);
    CHECK(!second.has_stack_u8_30);
    CHECK(first.has_stack_u8_30);

    observer->on_send(args_for(6, 0, 0));
    pkt3576::Record third;
    CHECK(observer->try_pop(third));
    CHECK(third.rcx == 6);
    CHECK(observer->dropped() == 3);
}

void test_concurrent_producers() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 0xAB, 0xCDEF);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);

    constexpr int kThreads = 4;
    constexpr int kEach = 250;
    constexpr int kTotal = kThreads * kEach;
    Obs observer(1024);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::atomic<bool> producers_done{false};
    std::atomic<int> popped{0};
    std::thread consumer([&] {
        pkt3576::Record record;
        while (!producers_done.load(std::memory_order_acquire)) {
            if (observer->try_pop(record)) {
                if (record.direction == pkt3576::Direction::Receive) {
                    if (record.has_stack_u8_30 || record.has_stack_u16_38) popped.store(-1000000);
                }
                popped.fetch_add(1, std::memory_order_relaxed);
            }
        }
        while (observer->try_pop(record)) {
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
                if ((thread + i) % 2 == 0) observer->on_send(args);
                else observer->on_recv(args);
            }
        });
    }
    for (auto& producer : producers) producer.join();
    producers_done.store(true, std::memory_order_release);
    consumer.join();

    CHECK(observer->unread() == 0);
    CHECK(observer->dropped() == 0);
    CHECK(popped.load() == kTotal);
}

void test_concurrent_full_queue() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 1, 1);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);

    constexpr int kThreads = 4;
    constexpr int kEach = 50;
    constexpr int kTotal = kThreads * kEach;
    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::vector<std::thread> producers;
    for (int thread = 0; thread < kThreads; ++thread) {
        producers.emplace_back([&, thread] {
            for (int i = 0; i < kEach; ++i) {
                observer->on_send(args_for(static_cast<uint64_t>(thread), static_cast<uint64_t>(i), 4));
            }
        });
    }
    for (auto& producer : producers) producer.join();

    int popped = 0;
    pkt3576::Record record;
    while (observer->try_pop(record)) {
        CHECK(record.copied_length == 5);
        CHECK(record.status == pkt3576::CopyStatus::Complete);
        CHECK(record.bytes[0] == 0x11);
        CHECK(record.r8 == 4);
        ++popped;
    }
    CHECK(popped == 4);
    CHECK(observer->dropped() == static_cast<uint64_t>(kTotal - popped));
    CHECK(observer->unread() == 0);
    CHECK(observer->filtered() == 0);
    CHECK(observer->malformed() == 0);
    CHECK(observer->truncated() == 0);
    CHECK(popped + static_cast<int>(observer->dropped()) == kTotal);
}

void test_filter_before_copy() {
    const uint64_t queue_budget = 256ull * (sizeof(pkt3576::Record) + sizeof(uint64_t));
    const uint64_t output_budget = 256ull * sizeof(pkt3576::Record);
    std::printf("record_bytes=%zu queue_budget=%llu output_budget=%llu\n", sizeof(pkt3576::Record),
                static_cast<unsigned long long>(queue_budget), static_cast<unsigned long long>(output_budget));
    CHECK(sizeof(pkt3576::Record) >= pkt3576::kCopyCap);
    CHECK(queue_budget + output_budget < 3ull * 1024ull * 1024ull);
    CHECK(pkt3576::kCopyCap == 4096);
    CHECK(pkt3576::kActorLoadDataAckMax == 0);

    Image image = make_image();
    Sink sink;
    Obs observer(8);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    image.record = true;
    observer->set_phase(static_cast<uint8_t>(pkt3576::Phase::Stand));

    plant_stack(image, 0, 0x3C, 0x0A11);
    observer->on_send(args_for(10, 20, 4));
    CHECK(!saw_address(image, kPacket));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));
    pkt3576::Record none;
    CHECK(!observer->try_pop(none));
    CHECK(observer->filtered() == 1);
    CHECK(observer->malformed() == 0);

    image.reads.clear();
    plant_stack(image, 1, 0x01, 0x0002);
    observer->on_send(args_for(1, 2, 3));
    CHECK(!saw_address(image, kPacket));
    CHECK(observer->filtered() == 2);

    image.reads.clear();
    const uint8_t internal_message[3] = {0xD1, 0x07, 0xFF};
    plant_packet(image, internal_message, sizeof internal_message);
    plant_stack(image, 3, 0x11, 0x22);
    observer->on_send(args_for(4, 5, 6));
    CHECK(saw_read(image, kPacket, pkt3576::kOpcodeBytes));
    CHECK(!saw_read(image, kPacket, 3));
    CHECK(!saw_read(image, kPacket, pkt3576::kHeaderBytes));
    CHECK(observer->filtered() == 3);
    CHECK(observer->malformed() == 0);
    CHECK(!observer->try_pop(none));

    image.reads.clear();
    uint8_t other[16] = {};
    other[0] = 0xE0;
    other[1] = 0x08;
    plant_packet(image, other, sizeof other);
    plant_stack(image, 100, 0x02, 0x0003);
    observer->on_recv(args_for(8, 9, 10));
    CHECK(saw_read(image, kPacket, pkt3576::kOpcodeBytes));
    CHECK(!saw_read(image, kPacket, 100));
    CHECK(!saw_read(image, kPacket, pkt3576::kCopyCap));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));
    CHECK(observer->filtered() == 4);

    image.reads.clear();
    const uint8_t short_selected[4] = {0x11, 0x0B, 0x00, 0x00};
    plant_packet(image, short_selected, sizeof short_selected);
    plant_stack(image, 4, 0x3C, 0x0A11);
    observer->on_send(args_for(11, 12, 13));
    pkt3576::Record malformed;
    CHECK(observer->try_pop(malformed));
    CHECK(malformed.status == pkt3576::CopyStatus::Malformed);
    CHECK(malformed.copied_length == 4);
    CHECK(malformed.claimed_length == 4);
    CHECK(!malformed.header_checked);
    CHECK(malformed.bytes[0] == 0x11);
    CHECK(malformed.bytes[4] == 0);
    CHECK(malformed.has_stack_u8_30);
    CHECK(malformed.stack_u8_30 == 0x3C);
    CHECK(malformed.phase == static_cast<uint8_t>(pkt3576::Phase::Stand));
    CHECK(malformed.qpc_ticks != 0);
    CHECK(observer->malformed() == 1);
    CHECK(observer->filtered() == 4);

    image.reads.clear();
    plant_frame(image, pkt3576::kMoveActorReq, 8, 0x40);
    poke_u8(image, kPacket + 3, 0x01);
    plant_stack(image, 8, 0x44, 0x5555);
    observer->on_send(args_for(21, 22, 23));
    pkt3576::Record mismatch;
    CHECK(observer->try_pop(mismatch));
    CHECK(mismatch.status == pkt3576::CopyStatus::Malformed);
    CHECK(mismatch.header_checked);
    CHECK(mismatch.header_payload == 1);
    CHECK(mismatch.copied_length == 8);
    CHECK(mismatch.claimed_length == 8);
    CHECK(mismatch.bytes[8] == 0);
    CHECK(observer->malformed() == 2);

    image.bytes[kPacket + 5] = 0x00;
    CHECK(mismatch.bytes[5] == 0x40);

    plant_frame(image, pkt3576::kStartActionAck, 5, 0x40);
    plant_stack(image, 5, 1, 2);
    observer->set_phase(static_cast<uint8_t>(pkt3576::Phase::Walk));
    observer->on_recv(args_for(30, 31, 32));
    pkt3576::Record complete;
    CHECK(observer->try_pop(complete));
    CHECK(complete.direction == pkt3576::Direction::Receive);
    CHECK(complete.status == pkt3576::CopyStatus::Complete);
    CHECK(complete.opcode == pkt3576::kStartActionAck);
    CHECK(complete.copied_length == 5);
    CHECK(complete.claimed_length == 5);
    CHECK(complete.header_payload == 0);
    CHECK(!complete.has_stack_u8_30);
    CHECK(!complete.has_stack_u16_38);
    CHECK(complete.phase == static_cast<uint8_t>(pkt3576::Phase::Walk));
    CHECK(complete.qpc_ticks >= mismatch.qpc_ticks);
    CHECK(observer->truncated() == 0);
    CHECK(observer->unread() == 0);
    CHECK(observer->dropped() == 0);
}

void test_unreadable_buffers() {
    Image image = make_image();
    Sink sink;
    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);
    image.record = true;

    plant_stack(image, 8, 1, 2);
    observer->on_send(args_for(1, 2, 3));
    CHECK(observer->unread() == 1);
    CHECK(saw_read(image, kPacket, pkt3576::kOpcodeBytes));
    CHECK(!saw_read(image, kPacket, pkt3576::kHeaderBytes));
    CHECK(!saw_read(image, kPacket, 8));
    pkt3576::Record none;
    CHECK(!observer->try_pop(none));
    CHECK(observer->filtered() == 0);

    image.reads.clear();
    image.bytes.erase(kStack + pkt3576::kStackSlot28);
    image.bytes.erase(kStack + pkt3576::kStackSlot28 + 1);
    observer->on_recv(args_for(1, 2, 3));
    CHECK(observer->unread() == 2);
    CHECK(!saw_address(image, kPacket));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));
    CHECK(!observer->try_pop(none));

    image.reads.clear();
    plant_frame(image, pkt3576::kMoveActorReq, 8, 0x40);
    plant_stack(image, 8, 1, 2);
    image.bytes.erase(kStack + pkt3576::kStackSlot30);
    observer->on_send(args_for(1, 2, 3));
    CHECK(observer->unread() == 3);
    CHECK(saw_read(image, kPacket, pkt3576::kOpcodeBytes));
    CHECK(saw_read(image, kPacket, pkt3576::kHeaderBytes));
    CHECK(!saw_read(image, kPacket, 8));
    CHECK(observer->malformed() == 0);
    CHECK(observer->filtered() == 0);
    CHECK(!observer->try_pop(none));
    CHECK(observer->dropped() == 0);

    image.reads.clear();
    const uint8_t other[2] = {0xE0, 0x08};
    plant_packet(image, other, sizeof other);
    plant_stack(image, 8, 1, 2);
    image.bytes.erase(kStack + pkt3576::kStackSlot30);
    observer->on_recv(args_for(4, 5, 6));
    CHECK(observer->filtered() == 1);
    CHECK(observer->unread() == 3);
    CHECK(saw_read(image, kPacket, pkt3576::kOpcodeBytes));
    CHECK(!saw_read(image, kPacket, pkt3576::kHeaderBytes));
    CHECK(!saw_address(image, kStack + pkt3576::kStackSlot30));

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

bool wrap_guard(uint64_t address, void* destination, size_t size, void*) {
    return pkt3576::guarded_read(address, destination, size);
}

void test_copy_cap_and_truncation() {
    Image image = make_image();
    Sink sink;
    Obs observer(4);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    plant_frame(image, pkt3576::kMoveActorReq, 4096, 0x40);
    plant_stack(image, 4096, 1, 2);
    observer->on_send(args_for(1, 0, 0));
    pkt3576::Record exact;
    CHECK(observer->try_pop(exact));
    CHECK(exact.status == pkt3576::CopyStatus::Complete);
    CHECK(exact.claimed_length == 4096);
    CHECK(exact.copied_length == 4096);
    CHECK(exact.header_payload == 4091);
    CHECK(exact.bytes[2] == 0x5A);
    CHECK(exact.bytes[4095] == 0x40);
    CHECK(exact.qpc_ticks != 0);
    CHECK(observer->truncated() == 0);

    plant_frame(image, pkt3576::kMoveActorReq, 4097, 0x41);
    plant_stack(image, 4097, 1, 2);
    observer->on_send(args_for(2, 0, 0));
    pkt3576::Record over;
    CHECK(observer->try_pop(over));
    CHECK(over.status == pkt3576::CopyStatus::Truncated);
    CHECK(over.claimed_length == 4097);
    CHECK(over.copied_length == 4096);
    CHECK(over.bytes[4095] == 0x41);
    CHECK(over.qpc_ticks >= exact.qpc_ticks);
    CHECK(observer->truncated() == 1);

    plant_frame(image, pkt3576::kActorLoadDataAck, 4097, 0x42);
    plant_stack(image, 4097, 3, 4);
    observer->on_send(args_for(3, 0, 0));
    pkt3576::Record actor;
    CHECK(observer->try_pop(actor));
    CHECK(actor.opcode == pkt3576::kActorLoadDataAck);
    CHECK(actor.status == pkt3576::CopyStatus::Truncated);
    CHECK(actor.status != pkt3576::CopyStatus::Complete);
    CHECK(actor.claimed_length == 4097);
    CHECK(actor.copied_length == pkt3576::kCopyCap);
    CHECK(pkt3576::kCopyCap == 4096);
    CHECK(pkt3576::kActorLoadDataAckMax == 0);
    CHECK(observer->truncated() == 2);
    CHECK(observer->malformed() == 0);
    CHECK(observer->unread() == 0);
    CHECK(observer->filtered() == 0);
    CHECK(observer->dropped() == 0);
}

void test_page_boundary() {
    SYSTEM_INFO info{};
    GetSystemInfo(&info);
    const size_t page = info.dwPageSize == 0 ? 4096 : info.dwPageSize;
    uint8_t* block = static_cast<uint8_t*>(VirtualAlloc(nullptr, page * 2, MEM_RESERVE, PAGE_NOACCESS));
    CHECK(block != nullptr);
    if (block == nullptr) return;
    uint8_t* committed = static_cast<uint8_t*>(VirtualAlloc(block, page, MEM_COMMIT, PAGE_READWRITE));
    CHECK(committed == block);
    if (committed != block) {
        VirtualFree(block, 0, MEM_RELEASE);
        return;
    }

    Obs observer(4);
    CHECK(observer->arm(wrap_guard, nullptr));
    const uint64_t stack = reinterpret_cast<uint64_t>(block);
    auto* stack_bytes = block;
    auto plant_slots = [&](uint16_t claimed) {
        *reinterpret_cast<uint16_t*>(stack_bytes + pkt3576::kStackSlot28) = claimed;
        stack_bytes[pkt3576::kStackSlot30] = 0x3C;
        *reinterpret_cast<uint16_t*>(stack_bytes + pkt3576::kStackSlot38) = 0x0A11;
    };
    pkt3576::EntryArgs args;
    args.rcx = 9;
    args.rdx = 8;
    args.r8 = 7;
    args.rsp = stack;

    uint8_t* inside = block + 0x200;
    inside[0] = 0x11;
    inside[1] = 0x0B;
    inside[2] = 0x5A;
    inside[3] = 0x03;
    inside[4] = 0x00;
    inside[5] = 0x40;
    inside[6] = 0x41;
    inside[7] = 0x42;
    plant_slots(8);
    args.r9 = reinterpret_cast<uint64_t>(inside);
    observer->on_send(args);
    pkt3576::Record complete;
    CHECK(observer->try_pop(complete));
    CHECK(complete.status == pkt3576::CopyStatus::Complete);
    CHECK(complete.copied_length == 8);
    CHECK(complete.bytes[5] == 0x40);
    inside[5] = 0x00;
    CHECK(complete.bytes[5] == 0x40);

    uint8_t* edge = block + page - 2;
    edge[0] = 0x11;
    edge[1] = 0x0B;
    plant_slots(8);
    args.r9 = reinterpret_cast<uint64_t>(edge);
    observer->on_send(args);
    pkt3576::Record none;
    CHECK(observer->unread() == 1);
    CHECK(!observer->try_pop(none));

    edge[0] = 0xD1;
    edge[1] = 0x07;
    plant_slots(3);
    observer->on_send(args);
    CHECK(observer->filtered() == 1);
    CHECK(observer->unread() == 1);
    CHECK(observer->malformed() == 0);
    CHECK(!observer->try_pop(none));

    uint8_t* split = block + page - 1;
    split[0] = 0x11;
    plant_slots(2);
    args.r9 = reinterpret_cast<uint64_t>(split);
    observer->on_send(args);
    CHECK(observer->unread() == 2);
    CHECK(observer->filtered() == 1);
    CHECK(!observer->try_pop(none));

    CHECK(observer->disarm());
    VirtualFree(block, 0, MEM_RELEASE);
}

void test_gate_entry_races_close() {
    for (int round = 0; round < 8; ++round) {
        pkt3576::AdmissionGate gate;
        CHECK(gate.closed());
        CHECK(!gate.admit());
        CHECK(gate.open());
        CHECK(!gate.closed());

        std::atomic<int> inside{0};
        std::atomic<bool> run{true};
        std::atomic<int> admits{0};
        std::vector<std::thread> threads;
        for (int t = 0; t < 4; ++t) {
            threads.emplace_back([&] {
                while (run.load(std::memory_order_relaxed)) {
                    if (!gate.admit()) continue;
                    inside.fetch_add(1, std::memory_order_acq_rel);
                    for (int spin = 0; spin < 32; ++spin) YieldProcessor();
                    inside.fetch_sub(1, std::memory_order_acq_rel);
                    gate.leave();
                    admits.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        while (admits.load(std::memory_order_relaxed) < 200) YieldProcessor();
        gate.close_and_wait();
        CHECK(inside.load(std::memory_order_acquire) == 0);
        CHECK(gate.admitted() == 0);
        CHECK(gate.closed());
        CHECK(!gate.admit());
        run.store(false, std::memory_order_release);
        for (auto& thread : threads) thread.join();
        CHECK(inside.load() == 0);
        CHECK(!gate.admit());
    }
}

void test_observer_entry_races_shutdown() {
    Image image = make_image();
    Sink sink;
    plant_stack(image, 5, 2, 3);
    plant_frame(image, pkt3576::kMoveActorReq, 5, 0x40);
    Obs observer(8);
    CHECK(observer->install(deps_for(image, sink)) == pkt3576::Observer::InstallResult::Installed);

    std::atomic<bool> run{true};
    std::atomic<int> popped{0};
    std::atomic<int> bad{0};
    std::vector<std::thread> producers;
    for (int t = 0; t < 4; ++t) {
        producers.emplace_back([&] {
            while (run.load(std::memory_order_acquire)) observer->on_send(args_for(0x5151, 0, 0));
        });
    }
    std::thread consumer([&] {
        pkt3576::Record record;
        while (run.load(std::memory_order_acquire)) {
            if (!observer->try_pop(record)) continue;
            if (record.rcx != 0x5151 || record.copied_length != 5 || record.bytes[0] != 0x11 ||
                record.status != pkt3576::CopyStatus::Complete) {
                bad.fetch_add(1, std::memory_order_relaxed);
            } else {
                popped.fetch_add(1, std::memory_order_relaxed);
            }
        }
        while (observer->try_pop(record)) {
            if (record.rcx != 0x5151 || record.copied_length != 5 || record.bytes[0] != 0x11 ||
                record.status != pkt3576::CopyStatus::Complete) {
                bad.fetch_add(1, std::memory_order_relaxed);
            } else {
                popped.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    while (popped.load(std::memory_order_acquire) == 0) YieldProcessor();
    CHECK(observer->shutdown() == pkt3576::Observer::ShutdownResult::Removed);
    run.store(false, std::memory_order_release);
    for (auto& producer : producers) producer.join();
    consumer.join();
    CHECK(bad.load() == 0);
    CHECK(popped.load() > 0);
    CHECK(!observer->queue_allocated());
    CHECK(observer->callback_code_released());
    CHECK(!observer->accepting());
    const uint64_t dropped_before = observer->dropped();
    observer->on_send(args_for(0x5151, 0, 0));
    CHECK(observer->dropped() == dropped_before + 1);
    pkt3576::Record late;
    CHECK(!observer->try_pop(late));
}

struct MutatingSource {
    Image image;
    int packet_reads = 0;
    void (*mutate)(Image&) = nullptr;
};

bool mutating_read(uint64_t address, void* destination, size_t size, void* user) {
    auto* source = static_cast<MutatingSource*>(user);
    if (address == kPacket) {
        if (source->packet_reads == 2 && source->mutate != nullptr) source->mutate(source->image);
        ++source->packet_reads;
    }
    return source->image.read(address, destination, size);
}

void mutate_payload_ffff(Image& image) { poke_u16(image, kPacket + 3, 0xFFFF); }

void mutate_opcode_unselected(Image& image) { poke_u16(image, kPacket, 0x1234); }

void mutate_opcode_rotate(Image& image) { poke_u16(image, kPacket, pkt3576::kRotateActorReq); }

void mutate_payload_one(Image& image) { poke_u16(image, kPacket + 3, 1); }

// The third packet read is the bulk copy. The opcode and header probes are the
// first two. A change at the copy must not stay Complete or Truncated.
void test_copied_header_follows_the_stored_bytes() {
    auto run = [](void (*mutate)(Image&), uint16_t claimed, pkt3576::Record& record, uint64_t& malformed,
                  uint64_t& truncated) {
        MutatingSource source;
        source.image = make_image();
        source.mutate = mutate;
        plant_frame(source.image, pkt3576::kMoveActorReq, claimed, 0x40);
        plant_stack(source.image, claimed, 0x30, 0x38);
        Sink sink;
        Obs observer(4);
        pkt3576::Observer::Deps deps;
        deps.read = mutating_read;
        deps.read_user = &source;
        deps.hooks = &sink;
        deps.image_base = pkt3576::kImageBase;
        CHECK(observer->install(deps) == pkt3576::Observer::InstallResult::Installed);
        observer->on_send(args_for(1, 2, 3));
        CHECK(source.packet_reads == 3);
        CHECK(observer->try_pop(record));
        CHECK(observer->filtered() == 0);
        CHECK(observer->unread() == 0);
        CHECK(observer->dropped() == 0);
        malformed = observer->malformed();
        truncated = observer->truncated();
    };

    pkt3576::Record stable;
    uint64_t malformed = 99;
    uint64_t truncated = 99;
    run(nullptr, 8, stable, malformed, truncated);
    CHECK(stable.status == pkt3576::CopyStatus::Complete);
    CHECK(stable.opcode == pkt3576::kMoveActorReq);
    CHECK(stable.header_checked);
    CHECK(stable.header_payload == 3);
    CHECK(stable.copied_length == 8);
    CHECK(stable.claimed_length == 8);
    CHECK(malformed == 0);
    CHECK(truncated == 0);

    pkt3576::Record length_changed;
    run(mutate_payload_ffff, 8, length_changed, malformed, truncated);
    CHECK(length_changed.status == pkt3576::CopyStatus::Malformed);
    CHECK(length_changed.status != pkt3576::CopyStatus::Complete);
    CHECK(length_changed.status != pkt3576::CopyStatus::Truncated);
    CHECK(length_changed.header_checked);
    CHECK(length_changed.header_payload == 65535);
    CHECK(length_changed.opcode == pkt3576::kMoveActorReq);
    CHECK(length_changed.bytes[3] == 0xFF);
    CHECK(length_changed.bytes[4] == 0xFF);
    CHECK(length_changed.copied_length == 8);
    CHECK(length_changed.claimed_length == 8);
    CHECK(malformed == 1);
    CHECK(truncated == 0);

    pkt3576::Record opcode_changed;
    run(mutate_opcode_unselected, 8, opcode_changed, malformed, truncated);
    CHECK(opcode_changed.status == pkt3576::CopyStatus::Malformed);
    CHECK(opcode_changed.status != pkt3576::CopyStatus::Complete);
    CHECK(opcode_changed.opcode == 0x1234);
    CHECK(opcode_changed.bytes[0] == 0x34);
    CHECK(opcode_changed.bytes[1] == 0x12);
    CHECK(opcode_changed.header_checked);
    CHECK(opcode_changed.header_payload == 3);
    CHECK(malformed == 1);
    CHECK(truncated == 0);

    pkt3576::Record selected_swap;
    run(mutate_opcode_rotate, 8, selected_swap, malformed, truncated);
    CHECK(selected_swap.status == pkt3576::CopyStatus::Complete);
    CHECK(selected_swap.opcode == pkt3576::kRotateActorReq);
    CHECK(selected_swap.bytes[0] == 0x24);
    CHECK(selected_swap.bytes[1] == 0x0B);
    CHECK(selected_swap.header_payload == 3);
    CHECK(malformed == 0);
    CHECK(truncated == 0);

    pkt3576::Record over;
    run(nullptr, 4097, over, malformed, truncated);
    CHECK(over.status == pkt3576::CopyStatus::Truncated);
    CHECK(over.copied_length == 4096);
    CHECK(over.claimed_length == 4097);
    CHECK(over.header_payload == 4092);
    CHECK(malformed == 0);
    CHECK(truncated == 1);

    pkt3576::Record over_changed;
    run(mutate_payload_one, 4097, over_changed, malformed, truncated);
    CHECK(over_changed.status == pkt3576::CopyStatus::Malformed);
    CHECK(over_changed.status != pkt3576::CopyStatus::Truncated);
    CHECK(over_changed.copied_length == 4096);
    CHECK(over_changed.claimed_length == 4097);
    CHECK(over_changed.header_checked);
    CHECK(over_changed.header_payload == 1);
    CHECK(over_changed.bytes[3] == 1);
    CHECK(over_changed.bytes[4] == 0);
    CHECK(malformed == 1);
    CHECK(truncated == 0);
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
    test_filter_before_copy();
    test_unreadable_buffers();
    test_copy_cap_and_truncation();
    test_copied_header_follows_the_stored_bytes();
    test_page_boundary();
    test_capacity_two_keeps_both_records();
    test_consumer_shutdown_waits_for_pop();
    test_failed_removal_keeps_ownership();
    test_gate_entry_races_close();
    test_observer_entry_races_shutdown();
    if (g_failures) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::puts("pkt3576 tests ok");
    return 0;
}
