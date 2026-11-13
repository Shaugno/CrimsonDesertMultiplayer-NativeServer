// Game-free host for the 3576 diagnostic module.
// Loads only the fixture DLL. The dummies are in this executable.
// Nothing here launches or attaches to Crimson Desert.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

#include <windows.h>

#include "pkt3576/live_api.h"
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

using StartFn = int (*)(const wchar_t*);
using StopFn = int (*)();
using StatusFn = int (*)(int*, int*, int*, unsigned*, unsigned*, unsigned*);
using FixtureFn = int (*)(const wchar_t*, void*, void*, unsigned, unsigned);
using MarkFn = int (*)(const char*);
using ReportFn = int (*)(Pkt3576LiveReport*);
using HookSetFn = void (*)(void (*)(void*), void*);

struct Api {
    HMODULE module = nullptr;
    StartFn start = nullptr;
    StopFn stop = nullptr;
    StatusFn status = nullptr;
    FixtureFn fixture = nullptr;
    MarkFn mark = nullptr;
    ReportFn report = nullptr;
    HookSetFn set_open = nullptr;
    HookSetFn set_hold = nullptr;
};

std::wstring beside(const wchar_t* file_name) {
    wchar_t path[1024] = {};
    const DWORD length = GetModuleFileNameW(nullptr, path, 1024);
    if (length == 0 || length >= 1024) return {};
    wchar_t* slash = nullptr;
    for (wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') slash = cursor;
    }
    if (slash == nullptr) return {};
    *(slash + 1) = L'\0';
    return std::wstring(path) + file_name;
}

std::wstring log_dir() {
    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    return std::wstring(temp) + L"pkt3576-live-host-" + std::to_wstring(GetCurrentProcessId());
}

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

void entry_bytes(const void* function, uint8_t out[16]) { std::memcpy(out, function, 16); }

bool same_bytes(const uint8_t* left, const uint8_t* right) { return std::memcmp(left, right, 16) == 0; }

unsigned attempts_of(const Api& api) {
    unsigned attempts = 99;
    CHECK(api.status(nullptr, nullptr, nullptr, nullptr, nullptr, &attempts) == 0);
    return attempts;
}

bool load_fixture(Api& api) {
    const std::wstring path = beside(L"cdmp_pkt3576_live_fixture.dll");
    api.module = LoadLibraryW(path.c_str());
    if (api.module == nullptr) {
        std::printf("FAIL LoadLibrary %ls error %lu\n", path.c_str(), GetLastError());
        ++g_failures;
        return false;
    }
    api.start = reinterpret_cast<StartFn>(GetProcAddress(api.module, "pkt3576_live_start"));
    api.stop = reinterpret_cast<StopFn>(GetProcAddress(api.module, "pkt3576_live_stop"));
    api.status = reinterpret_cast<StatusFn>(GetProcAddress(api.module, "pkt3576_live_status"));
    api.fixture = reinterpret_cast<FixtureFn>(GetProcAddress(api.module, "pkt3576_live_fixture_start"));
    api.mark = reinterpret_cast<MarkFn>(GetProcAddress(api.module, "pkt3576_live_mark"));
    api.report = reinterpret_cast<ReportFn>(GetProcAddress(api.module, "pkt3576_live_report"));
    api.set_open = reinterpret_cast<HookSetFn>(GetProcAddress(api.module, "pkt3576_live_fixture_set_records_open_hook"));
    api.set_hold = reinterpret_cast<HookSetFn>(GetProcAddress(api.module, "pkt3576_live_fixture_set_phase_hold"));
    CHECK(api.start != nullptr);
    CHECK(api.stop != nullptr);
    CHECK(api.status != nullptr);
    CHECK(api.fixture != nullptr);
    CHECK(api.mark != nullptr);
    CHECK(api.report != nullptr);
    CHECK(api.set_open != nullptr);
    CHECK(api.set_hold != nullptr);
    return api.fixture != nullptr && api.mark != nullptr && api.report != nullptr && api.set_open != nullptr &&
           api.set_hold != nullptr;
}

// The hold runs inside set_phase, before the capture is used. The deadline is
// only a hang fuse. The test releases the gate after 200 ms.
void hold_until_released(void* user) {
    auto* gate = static_cast<std::atomic<int>*>(user);
    gate->store(1, std::memory_order_release);
    const ULONGLONG deadline = GetTickCount64() + 5000;
    while (gate->load(std::memory_order_acquire) != 2 && GetTickCount64() < deadline) YieldProcessor();
}

void plant_prior_archive(void* user) {
    const auto* path = static_cast<const wchar_t*>(user);
    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    const char marker[] = "PRIOR-ARCHIVE\n";
    DWORD written = 0;
    WriteFile(file, marker, static_cast<DWORD>(sizeof marker - 1), &written, nullptr);
    CloseHandle(file);
}

DWORD WINAPI sleeper_proc(LPVOID param) {
    WaitForSingleObject(static_cast<HANDLE>(param), 60000);
    return 0;
}

void test_host(const std::wstring& directory) {
    uint8_t send_before[16] = {};
    uint8_t recv_before[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_before);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_before);

    Api early;
    if (!load_fixture(early)) return;
    uint8_t send_loaded[16] = {};
    uint8_t recv_loaded[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_loaded);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_loaded);
    CHECK(same_bytes(send_before, send_loaded));
    CHECK(same_bytes(recv_before, recv_loaded));
    CHECK(early.stop() == kPkt3576LiveIdle);
    CHECK(attempts_of(early) == 0);

    CHECK(early.fixture(L"pkt3576-live-relative-must-not-exist", reinterpret_cast<void*>(&pkt3576_dummy_send),
                        reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveBadLog);
    CHECK(GetFileAttributesW(L"pkt3576-live-relative-must-not-exist") == INVALID_FILE_ATTRIBUTES);
    CHECK(early.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                        reinterpret_cast<void*>(&pkt3576_dummy_recv), 1, 2) == kPkt3576LiveRefused);
    CHECK(attempts_of(early) == 0);

    HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    void* sleep_fn = reinterpret_cast<void*>(GetProcAddress(kernel, "Sleep"));
    CHECK(early.fixture(directory.c_str(), sleep_fn, reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) ==
          kPkt3576LiveRefused);
    CHECK(attempts_of(early) == 0);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_loaded);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_loaded);
    CHECK(same_bytes(send_before, send_loaded));
    CHECK(same_bytes(recv_before, recv_loaded));

    CHECK(FreeLibrary(early.module));
    CHECK(GetModuleHandleW(L"cdmp_pkt3576_live_fixture.dll") == nullptr);

    Api api;
    if (!load_fixture(api)) return;
    CHECK(attempts_of(api) == 0);
    const int rolled = api.fixture(directory.c_str(), pkt3576_bad_entry, reinterpret_cast<void*>(&pkt3576_dummy_recv), 8,
                                   4);
    CHECK(rolled == kPkt3576LiveRolledBack);
    CHECK(attempts_of(api) == 1);
    CHECK(api.stop() == kPkt3576LiveIdle);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_loaded);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_loaded);
    CHECK(same_bytes(send_before, send_loaded));
    CHECK(same_bytes(recv_before, recv_loaded));

    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveStarted);
    CHECK(attempts_of(api) == 2);
    uint8_t packet[8] = {0x11, 0x0B, 0x5A, 0x03, 0x00, 0x11, 0x22, 0x33};
    const int calls_before = pktfix::send_calls.load();
    for (int index = 0; index < 5; ++index) {
        CHECK(pkt3576_dummy_send(0x11, 0x22, pktfix::kSkipBytes, packet, 8, 0x30, 0x38) == pktfix::kSendReturn);
    }
    CHECK(pktfix::send_calls.load() == calls_before + 5);
    CHECK(api.stop() == kPkt3576LiveRemoved);

    unsigned stored = 0;
    unsigned omitted = 0;
    int retained = 1;
    int stop_code = -1;
    CHECK(api.status(nullptr, &stop_code, &retained, &stored, &omitted, nullptr) == 0);
    CHECK(stop_code == kPkt3576LiveRemoved);
    CHECK(retained == 0);
    CHECK(stored == 2);
    CHECK(omitted == 3);
    const std::string records = read_file(directory + L"\\pkt3576-live-records.txt");
    CHECK(records.find("0000000000000011") != std::string::npos);
    CHECK(records.find("status=complete") != std::string::npos);
    CHECK(records.find("phase=unlabeled") != std::string::npos);
    CHECK(records.find("qpc_frequency=") != std::string::npos);
    CHECK(records.find("actor_load_max=unknown") != std::string::npos);
    CHECK(records.find("interval_guaranteed=0") != std::string::npos);
    int send_lines = 0;
    for (size_t index = 0; index + 1 < records.size(); ++index) {
        if (records[index] == 'S' && records[index + 1] == ' ') ++send_lines;
    }
    CHECK(send_lines == 2);
    const std::string session = read_file(directory + L"\\pkt3576-live-session.txt");
    if (session.find("fixture=1") == std::string::npos || session.find("game_match=0") == std::string::npos ||
        session.find("game_match=1") != std::string::npos || session.find("startup=started") == std::string::npos ||
        session.find("shutdown=removed") == std::string::npos || session.find("gate=fixture") == std::string::npos ||
        session.find("qpc_frequency=") == std::string::npos ||
        session.find("qpc_timebase=QueryPerformanceCounter") == std::string::npos ||
        session.find("actor_load_max=unknown") == std::string::npos) {
        std::printf("FAIL fixture session log:\n%s\n", session.c_str());
        ++g_failures;
    }

    CHECK(api.mark != nullptr);
    CHECK(api.mark("stand") == 0);
    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveStarted);
    CHECK(attempts_of(api) == 3);
    CHECK(pkt3576_dummy_send(0x11, 0x22, pktfix::kSkipBytes, packet, 8, 0x30, 0x38) == pktfix::kSendReturn);
    CHECK(api.stop() == kPkt3576LiveRemoved);
    const std::wstring stand_path = directory + L"\\pkt3576-live-records-stand.txt";
    const std::string stand_records = read_file(stand_path);
    CHECK(stand_records.find("status=complete") != std::string::npos);
    CHECK(stand_records.find("phase=stand") != std::string::npos);
    CHECK(stand_records.find(" qpc=") != std::string::npos);
    CHECK(stand_records.find("opcode=0B11") != std::string::npos);
    CHECK(stand_records.find("qpc_frequency=") != std::string::npos);
    const std::string stand_bytes = stand_records;
    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveBadLog);
    CHECK(attempts_of(api) == 3);
    CHECK(read_file(stand_path) == stand_bytes);

    CHECK(api.mark("walk") == 0);
    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveStarted);
    CHECK(attempts_of(api) == 4);
    CHECK(pkt3576_dummy_send(0x11, 0x22, pktfix::kSkipBytes, packet, 8, 0x30, 0x38) == pktfix::kSendReturn);

    std::atomic<int> stress_marks{0};
    std::atomic<int> stress_reports{0};
    std::atomic<bool> stress_run{true};
    std::thread stress([&] {
        const char* names[] = {"stand", "walk"};
        int index = 0;
        while (stress_run.load(std::memory_order_acquire)) {
            if (api.mark(names[index % 2]) == 0) stress_marks.fetch_add(1, std::memory_order_relaxed);
            Pkt3576LiveReport report{};
            if (api.report(&report) == 0) stress_reports.fetch_add(1, std::memory_order_relaxed);
            ++index;
        }
    });
    const ULONGLONG stress_deadline = GetTickCount64() + 2000;
    while ((stress_marks.load(std::memory_order_acquire) == 0 || stress_reports.load(std::memory_order_acquire) == 0) &&
           GetTickCount64() < stress_deadline) {
        YieldProcessor();
    }

    std::atomic<int> hold_gate{0};
    api.set_hold(hold_until_released, &hold_gate);
    std::atomic<int> marker_result{-1};
    std::thread marker([&] { marker_result.store(api.mark("walk"), std::memory_order_release); });
    const ULONGLONG hold_deadline = GetTickCount64() + 2000;
    while (hold_gate.load(std::memory_order_acquire) != 1 && GetTickCount64() < hold_deadline) YieldProcessor();
    const bool held = hold_gate.load(std::memory_order_acquire) == 1;
    api.set_hold(nullptr, nullptr);
    if (!held) {
        std::printf("FAIL mark did not enter the phase hold\n");
        ++g_failures;
        hold_gate.store(2, std::memory_order_release);
        marker.join();
        stress_run.store(false, std::memory_order_release);
        stress.join();
        CHECK(api.stop() == kPkt3576LiveRemoved);
    } else {
        std::atomic<int> stop_result{-1};
        std::thread stopper([&] { stop_result.store(api.stop(), std::memory_order_release); });
        Sleep(200);
        if (stop_result.load(std::memory_order_acquire) != -1) {
            std::printf("FAIL stop finished while mark held the capture\n");
            ++g_failures;
        }
        hold_gate.store(2, std::memory_order_release);
        marker.join();
        stopper.join();
        stress_run.store(false, std::memory_order_release);
        stress.join();
        CHECK(marker_result.load(std::memory_order_acquire) == 0);
        CHECK(stop_result.load(std::memory_order_acquire) == kPkt3576LiveRemoved);
        CHECK(stress_marks.load(std::memory_order_acquire) > 0);
        CHECK(stress_reports.load(std::memory_order_acquire) > 0);
        Pkt3576LiveReport after{};
        CHECK(api.report(&after) == 0);
        CHECK(after.stop_code == kPkt3576LiveRemoved);
        CHECK(after.retained == 0);
        CHECK(after.capture_attempts == 4);
        CHECK(after.phase <= 3);
    }

    const std::string walk_records = read_file(directory + L"\\pkt3576-live-records-walk.txt");
    CHECK(walk_records.find("phase=walk") != std::string::npos);
    CHECK(walk_records.find("status=complete") != std::string::npos);
    CHECK(read_file(stand_path) == stand_bytes);

    uint8_t send_open[16] = {};
    uint8_t recv_open[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_open);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_open);
    const std::wstring turn_path = directory + L"\\pkt3576-live-records-turn.txt";
    api.set_open(plant_prior_archive, const_cast<wchar_t*>(turn_path.c_str()));
    CHECK(api.mark("turn") == 0);
    const int exclusive = api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2);
    api.set_open(nullptr, nullptr);
    if (exclusive != kPkt3576LiveRefused) {
        const std::string body = read_file(turn_path);
        std::printf("FAIL exclusive start returned %d file_bytes %zu\n", exclusive, body.size());
        if (!body.empty()) std::printf("FAIL exclusive prefix %.80s\n", body.c_str());
        ++g_failures;
    }
    CHECK(attempts_of(api) == 5);
    CHECK(read_file(turn_path) == "PRIOR-ARCHIVE\n");
    uint8_t send_after_open[16] = {};
    uint8_t recv_after_open[16] = {};
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_send), send_after_open);
    entry_bytes(reinterpret_cast<void*>(&pkt3576_dummy_recv), recv_after_open);
    CHECK(same_bytes(send_open, send_after_open));
    CHECK(same_bytes(recv_open, recv_after_open));

    CHECK(api.mark("nope") == kPkt3576LiveBadLog);
    CHECK(api.mark(nullptr) == kPkt3576LiveBadLog);
    CHECK(api.mark("unlabeled") == 0);
    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveStarted);
    CHECK(attempts_of(api) == 6);
    HANDLE wake = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    CHECK(wake != nullptr);
    constexpr int kSleepers = 320;
    HANDLE threads[kSleepers] = {};
    int spawned = 0;
    for (int index = 0; index < kSleepers; ++index) {
        threads[index] = CreateThread(nullptr, 64 * 1024, sleeper_proc, wake, 0, nullptr);
        if (threads[index] != nullptr) ++spawned;
    }
    CHECK(spawned >= 280);
    const int retained_stop = api.stop();
    CHECK(retained_stop == kPkt3576LiveRetained);
    int retained_flag = 0;
    CHECK(api.status(nullptr, &stop_code, &retained_flag, nullptr, nullptr, nullptr) == 0);
    CHECK(stop_code == kPkt3576LiveRetained);
    CHECK(retained_flag == 1);
    CHECK(api.stop() == kPkt3576LiveRetained);
    const HMODULE pinned = GetModuleHandleW(L"cdmp_pkt3576_live_fixture.dll");
    CHECK(pinned == api.module);
    CHECK(FreeLibrary(api.module));
    CHECK(GetModuleHandleW(L"cdmp_pkt3576_live_fixture.dll") == pinned);

    if (wake != nullptr) SetEvent(wake);
    for (int index = 0; index < kSleepers; ++index) {
        if (threads[index] == nullptr) continue;
        if (WaitForSingleObject(threads[index], 2000) != WAIT_OBJECT_0) {
            std::printf("FAIL sleeper %d did not exit\n", index);
            ++g_failures;
            break;
        }
        CloseHandle(threads[index]);
        threads[index] = nullptr;
    }
    if (wake != nullptr) CloseHandle(wake);
}

}  // namespace

int main() {
    const std::wstring directory = log_dir();
    test_host(directory);
    if (g_failures != 0) {
        std::printf("pkt3576 live host tests failed: %d\n", g_failures);
        return 1;
    }
    std::printf("pkt3576 live host tests ok\n");
    return 0;
}
