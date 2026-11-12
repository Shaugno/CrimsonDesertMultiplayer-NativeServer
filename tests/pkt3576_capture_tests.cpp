// Game-free SafetyHook capture tests. The hooks target dummy functions in this
// executable. They do not open the compatibility guard or attach to a game.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include <windows.h>

#include "pkt3576/capture.h"
#include "pkt3576_capture_targets.h"

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

struct Owned {
    pkt3576::Capture* raw = nullptr;
    explicit Owned(pkt3576::Capture* capture) : raw(capture) {}
    ~Owned() { pkt3576::Capture::release_owner(raw); }
    pkt3576::Capture* operator->() const { return raw; }
    Owned(const Owned&) = delete;
    Owned& operator=(const Owned&) = delete;
};

std::wstring temp_path(const wchar_t* name) {
    wchar_t directory[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, directory);
    return std::wstring(directory) + name;
}

void entry_bytes(const void* function, uint8_t out[16]) { std::memcpy(out, function, 16); }

bool same_bytes(const uint8_t* left, const uint8_t* right) { return std::memcmp(left, right, 16) == 0; }

std::string read_file(const std::wstring& path) {
    FILE* file = nullptr;
    if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr) return {};
    std::string text;
    char buffer[256];
    size_t count = 0;
    while ((count = std::fread(buffer, 1, sizeof buffer, file)) > 0) text.append(buffer, count);
    std::fclose(file);
    return text;
}

void fill_packet(uint8_t* packet, size_t size, uint8_t* canary, size_t canary_size) {
    for (size_t index = 0; index < size; ++index) packet[index] = static_cast<uint8_t>(index + 1);
    std::memset(canary, 0xA5, canary_size);
}

template <typename Pred>
bool wait_until(Pred pred, uint32_t timeout_ms = 3000) {
    const ULONGLONG deadline = GetTickCount64() + timeout_ms;
    while (!pred()) {
        if (GetTickCount64() >= deadline) return false;
        YieldProcessor();
    }
    return true;
}

bool packet_intact(const uint8_t* packet, size_t size, const uint8_t* canary, size_t canary_size) {
    for (size_t index = 0; index < size; ++index) {
        if (packet[index] != static_cast<uint8_t>(index + 1)) return false;
    }
    for (size_t index = 0; index < canary_size; ++index) {
        if (canary[index] != 0xA5) return false;
    }
    return true;
}

void test_refuses_before_any_hook() {
    uint8_t send_before[16] = {};
    uint8_t recv_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_before);

    const std::wstring path = temp_path(L"pkt3576-capture-refuse.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 1;
    limits.output_records = 4;
    Owned small(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(small.raw != nullptr);
    CHECK(small->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                       reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Refused);
    CHECK(pkt3576_dummy_send(1, 2, 3, nullptr, 0, 0, 0) == pktfix::kSendReturn);

    HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    void* sleep_fn = reinterpret_cast<void*>(GetProcAddress(kernel, "Sleep"));
    limits.queue_capacity = 4;
    Owned outside(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(outside->start(GetModuleHandleW(nullptr), sleep_fn, reinterpret_cast<void*>(&pkt3576_dummy_recv)) ==
          pkt3576::Capture::StartResult::Refused);
    CHECK(outside->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send), sleep_fn) ==
          pkt3576::Capture::StartResult::Refused);
    Sleep(0);

    uint8_t send_after[16] = {};
    uint8_t recv_after[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_after);
    CHECK(same_bytes(send_before, send_after));
    CHECK(same_bytes(recv_before, recv_after));
    DeleteFileW(path.c_str());
}

void test_known_arguments_and_prefix() {
    uint8_t packet[32] = {};
    uint8_t canary[8] = {};
    fill_packet(packet, sizeof packet, canary, sizeof canary);
    uint8_t send_before[16] = {};
    uint8_t recv_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_before);

    const std::wstring path = temp_path(L"pkt3576-capture-known.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 16;
    limits.output_records = 16;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                         reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);

    const int send_before_calls = pktfix::send_calls.load();
    const int recv_before_calls = pktfix::recv_calls.load();
    CHECK(pkt3576_dummy_send(0x1111111111111111ull, 0x2222222222222222ull, 0x3333333333333333ull, packet, 8, 0x5A,
                             0xBEEF) == pktfix::kSendReturn);
    CHECK(pktfix::send_rcx.load() == 0x1111111111111111ull);
    CHECK(pktfix::send_rdx.load() == 0x2222222222222222ull);
    CHECK(pktfix::send_r8.load() == 0x3333333333333333ull);
    CHECK(pktfix::send_slot28.load() == 8);
    CHECK(pktfix::send_slot30.load() == 0x5A);
    CHECK(pktfix::send_slot38.load() == 0xBEEF);

    CHECK(pkt3576_dummy_recv(0xAAAAAAAAAAAAAAAAull, 0xBBBBBBBBBBBBBBBBull, 0xCCCCCCCCCCCCCCCCull, packet, 4,
                             0x1111111111111111ull, 0x2222222222222222ull) == pktfix::kRecvReturn);
    CHECK(pktfix::recv_rcx.load() == 0xAAAAAAAAAAAAAAAAull);
    CHECK(pktfix::recv_slot28.load() == 4);

    CHECK(pkt3576_dummy_send(0x10, 0x20, 0x30, packet, 0, 1, 2) == pktfix::kSendReturn);
    CHECK(pkt3576_dummy_send(0x11, 0x21, 0x31, packet, 1, 3, 4) == pktfix::kSendReturn);
    CHECK(pkt3576_dummy_send(0x12, 0x22, 0x32, packet, 100, 5, 6) == pktfix::kSendReturn);
    CHECK(packet_intact(packet, sizeof packet, canary, sizeof canary));
    CHECK(pktfix::send_calls.load() == send_before_calls + 4);
    CHECK(pktfix::recv_calls.load() == recv_before_calls + 1);

    CHECK(wait_until([&] { return capture->stored() >= 5; }));
    CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
    CHECK(capture->stored() == 5);
    CHECK(capture->dropped() == 0);
    CHECK(capture->unread() == 0);
    CHECK(capture->omitted() == 0);
    CHECK(!capture->queue_allocated());
    CHECK(!capture->accepting());

    pkt3576::Record records[5];
    for (uint32_t index = 0; index < 5; ++index) CHECK(capture->copy_record(index, records[index]));

    CHECK(records[0].direction == pkt3576::Direction::Send);
    CHECK(records[0].rcx == 0x1111111111111111ull);
    CHECK(records[0].rdx == 0x2222222222222222ull);
    CHECK(records[0].r8 == 0x3333333333333333ull);
    CHECK(records[0].r9 == reinterpret_cast<uint64_t>(packet));
    CHECK(records[0].has_stack_u16_28);
    CHECK(records[0].stack_u16_28 == 8);
    CHECK(records[0].has_stack_u8_30);
    CHECK(records[0].stack_u8_30 == 0x5A);
    CHECK(records[0].has_stack_u16_38);
    CHECK(records[0].stack_u16_38 == 0xBEEF);
    CHECK(records[0].prefix_len == 8);
    CHECK(std::memcmp(records[0].prefix, packet, 8) == 0);

    CHECK(records[1].direction == pkt3576::Direction::Receive);
    CHECK(records[1].rcx == 0xAAAAAAAAAAAAAAAAull);
    CHECK(records[1].r8 == 0xCCCCCCCCCCCCCCCCull);
    CHECK(records[1].stack_u16_28 == 4);
    CHECK(records[1].prefix_len == 4);
    CHECK(std::memcmp(records[1].prefix, packet, 4) == 0);
    CHECK(!records[1].has_stack_u8_30);
    CHECK(!records[1].has_stack_u16_38);

    CHECK(records[2].direction == pkt3576::Direction::Send);
    CHECK(records[2].stack_u16_28 == 0);
    CHECK(records[2].prefix_len == 0);
    CHECK(records[3].prefix_len == 1);
    CHECK(records[3].prefix[0] == packet[0]);
    CHECK(records[4].stack_u16_28 == 100);
    CHECK(records[4].prefix_len == 16);
    CHECK(std::memcmp(records[4].prefix, packet, 16) == 0);

    const uint64_t dropped_after = capture->dropped();
    CHECK(pkt3576_dummy_send(9, 9, 9, packet, 1, 1, 1) == pktfix::kSendReturn);
    CHECK(capture->dropped() == dropped_after);
    CHECK(capture->stored() == 5);

    uint8_t send_after[16] = {};
    uint8_t recv_after[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_after);
    CHECK(same_bytes(send_before, send_after));
    CHECK(same_bytes(recv_before, recv_after));

    const std::string log = read_file(path);
    CHECK(log.find("counters dropped=0 unread=0 stored=5 omitted=0") != std::string::npos);
    CHECK(log.find("S 1111111111111111") != std::string::npos);
    CHECK(log.find("R AAAAAAAAAAAAAAAA") != std::string::npos);
    CHECK(log.size() < 4096);
    DeleteFileW(path.c_str());
}

void test_unreadable_prefix() {
    uint8_t* page = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_NOACCESS));
    CHECK(page != nullptr);
    const std::wstring path = temp_path(L"pkt3576-capture-unread.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 4;
    limits.output_records = 4;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                         reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);
    CHECK(pkt3576_dummy_send(1, 2, pktfix::kSkipBytes, page, 4, 7, 8) == pktfix::kSendReturn);
    CHECK(pktfix::send_r8.load() == pktfix::kSkipBytes);
    CHECK(pktfix::send_slot28.load() == 4);
    CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
    CHECK(capture->stored() == 0);
    CHECK(capture->unread() == 1);
    CHECK(capture->dropped() == 0);
    VirtualFree(page, 0, MEM_RELEASE);
    DeleteFileW(path.c_str());
}

void test_queue_saturation() {
    uint8_t packet[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    const std::wstring path = temp_path(L"pkt3576-capture-full.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 2;
    limits.output_records = 8;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    capture->set_writer_blocked(true);
    CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                         reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);
    constexpr int kCalls = 20;
    for (int index = 0; index < kCalls; ++index) {
        CHECK(pkt3576_dummy_send(static_cast<uint64_t>(index), 0, 0, packet, 4, 1, 2) == pktfix::kSendReturn);
    }
    CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
    CHECK(capture->stored() + capture->dropped() == static_cast<uint64_t>(kCalls));
    CHECK(capture->stored() == 2);
    CHECK(capture->dropped() == static_cast<uint64_t>(kCalls - 2));
    CHECK(capture->unread() == 0);
    pkt3576::Record first;
    CHECK(capture->copy_record(0, first));
    CHECK(first.prefix_len == 4);
    CHECK(first.prefix[0] == 1);
    CHECK(std::memcmp(packet, "\x01\x02\x03\x04\x05\x06\x07\x08", 8) == 0);
    DeleteFileW(path.c_str());
}

void test_output_bound() {
    uint8_t packet[4] = {9, 8, 7, 6};
    const std::wstring path = temp_path(L"pkt3576-capture-bound.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 64;
    limits.output_records = 4;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                         reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);
    constexpr int kCalls = 20;
    for (int index = 0; index < kCalls; ++index) {
        CHECK(pkt3576_dummy_recv(static_cast<uint64_t>(index), 0, 0, packet, 4, 0xABCDEF, 0x1234) ==
              pktfix::kRecvReturn);
    }
    CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
    CHECK(capture->stored() == 4);
    CHECK(capture->omitted() == 16);
    CHECK(capture->dropped() == 0);
    CHECK(capture->unread() == 0);
    pkt3576::Record sample;
    CHECK(capture->copy_record(0, sample));
    CHECK(sample.direction == pkt3576::Direction::Receive);
    CHECK(!sample.has_stack_u8_30);
    CHECK(!sample.has_stack_u16_38);
    const std::string log = read_file(path);
    CHECK(log.find("counters dropped=0 unread=0 stored=4 omitted=16") != std::string::npos);
    CHECK(log.size() < 4096);
    DeleteFileW(path.c_str());
}

void test_concurrent_calls_during_teardown() {
    uint8_t packet[16] = {};
    uint8_t canary[8] = {};
    fill_packet(packet, sizeof packet, canary, sizeof canary);
    uint8_t send_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);

    const std::wstring path = temp_path(L"pkt3576-capture-race.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 128;
    limits.output_records = 128;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                         reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);

    std::atomic<bool> run{true};
    std::atomic<int> bad{0};
    std::atomic<int> calls{0};
    std::vector<std::thread> threads;
    for (int index = 0; index < 4; ++index) {
        threads.emplace_back([&, index] {
            while (run.load(std::memory_order_acquire)) {
                const int result = pkt3576_dummy_send(static_cast<uint64_t>(index), 0x44, 0x55, packet, 8, 0x20, 0x30);
                if (result != pktfix::kSendReturn) bad.fetch_add(1, std::memory_order_relaxed);
                calls.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    CHECK(wait_until([&] { return capture->stored() > 0; }));
    CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
    run.store(false, std::memory_order_release);
    for (auto& thread : threads) thread.join();
    CHECK(bad.load() == 0);
    CHECK(calls.load() > 0);
    CHECK(packet_intact(packet, sizeof packet, canary, sizeof canary));
    CHECK(capture->stored() > 0);
    CHECK(capture->unread() == 0);
    CHECK(capture->stored() + capture->omitted() + capture->dropped() <= static_cast<uint64_t>(calls.load()));
    uint8_t send_after[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after);
    CHECK(same_bytes(send_before, send_after));
    DeleteFileW(path.c_str());
}

void test_partial_install_restores_the_first_hook() {
    uint8_t recv_before[16] = {};
    uint8_t send_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_before);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);
    const int recv_calls = pktfix::recv_calls.load();
    const std::wstring path = temp_path(L"pkt3576-capture-partial.log");
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 4;
    limits.output_records = 4;
    Owned capture(pkt3576::Capture::create(limits, path.c_str()));
    CHECK(capture->start(GetModuleHandleW(nullptr), pkt3576_bad_entry, reinterpret_cast<void*>(&pkt3576_dummy_recv)) ==
          pkt3576::Capture::StartResult::RolledBack);
    CHECK(pkt3576_dummy_recv(1, 2, 3, nullptr, 0, 0, 0) == pktfix::kRecvReturn);
    CHECK(pktfix::recv_calls.load() == recv_calls + 1);
    uint8_t recv_after[16] = {};
    uint8_t send_after[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_after);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after);
    CHECK(same_bytes(recv_before, recv_after));
    CHECK(same_bytes(send_before, send_after));
    DeleteFileW(path.c_str());
}

void test_repeated_install() {
    uint8_t send_before[16] = {};
    uint8_t recv_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_before);
    uint8_t packet[4] = {0xAB, 0xCD, 0xEF, 0x10};
    const std::wstring path = temp_path(L"pkt3576-capture-repeat.log");
    for (int round = 0; round < 8; ++round) {
        pkt3576::CaptureLimits limits;
        limits.queue_capacity = 4;
        limits.output_records = 4;
        Owned capture(pkt3576::Capture::create(limits, path.c_str()));
        CHECK(capture->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                             reinterpret_cast<void*>(&pkt3576_dummy_recv)) == pkt3576::Capture::StartResult::Started);
        CHECK(pkt3576_dummy_send(0x42, 0, 0, packet, 2, 0x7, 0x8) == pktfix::kSendReturn);
        CHECK(pkt3576_dummy_recv(0x43, 0, 0, packet, 2, 0, 0) == pktfix::kRecvReturn);
        CHECK(capture->stop() == pkt3576::Capture::StopResult::Removed);
        CHECK(capture->stored() == 2);
        CHECK(capture->dropped() == 0);
        CHECK(capture->unread() == 0);
        pkt3576::Record send;
        pkt3576::Record recv;
        CHECK(capture->copy_record(0, send));
        CHECK(capture->copy_record(1, recv));
        CHECK(send.rcx == 0x42);
        CHECK(send.prefix_len == 2);
        CHECK(send.prefix[0] == 0xAB);
        CHECK(send.prefix[1] == 0xCD);
        CHECK(recv.direction == pkt3576::Direction::Receive);
        CHECK(recv.prefix_len == 2);
        CHECK(!recv.has_stack_u8_30);
    }
    uint8_t send_after[16] = {};
    uint8_t recv_after[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_after);
    CHECK(same_bytes(send_before, send_after));
    CHECK(same_bytes(recv_before, recv_after));
    CHECK(std::memcmp(packet, "\xAB\xCD\xEF\x10", 4) == 0);
    DeleteFileW(path.c_str());
}

}  // namespace

int main() {
    test_refuses_before_any_hook();
    test_known_arguments_and_prefix();
    test_unreadable_prefix();
    test_queue_saturation();
    test_output_bound();
    test_concurrent_calls_during_teardown();
    test_partial_install_restores_the_first_hook();
    test_repeated_install();
    if (g_failures) {
        std::printf("%d failure(s)\n", g_failures);
        return 1;
    }
    std::puts("pkt3576 capture tests ok");
    return 0;
}
