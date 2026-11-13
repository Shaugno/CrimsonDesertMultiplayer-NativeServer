// Game-free host for the 3576 diagnostic module.
// Loads only the fixture DLL. The dummies are in this executable.
// Nothing here launches or attaches to Crimson Desert.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

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

struct Api {
    HMODULE module = nullptr;
    StartFn start = nullptr;
    StopFn stop = nullptr;
    StatusFn status = nullptr;
    FixtureFn fixture = nullptr;
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
    CHECK(api.start != nullptr);
    CHECK(api.stop != nullptr);
    CHECK(api.status != nullptr);
    CHECK(api.fixture != nullptr);
    return api.fixture != nullptr;
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
    uint8_t packet[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    const int calls_before = pktfix::send_calls.load();
    for (int index = 0; index < 5; ++index) {
        CHECK(pkt3576_dummy_send(0x11, 0x22, pktfix::kSkipBytes, packet, 4, 0x30, 0x38) == pktfix::kSendReturn);
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
    int send_lines = 0;
    for (size_t index = 0; index + 1 < records.size(); ++index) {
        if (records[index] == 'S' && records[index + 1] == ' ') ++send_lines;
    }
    CHECK(send_lines == 2);
    const std::string session = read_file(directory + L"\\pkt3576-live-session.txt");
    if (session.find("fixture=1") == std::string::npos || session.find("game_match=0") == std::string::npos ||
        session.find("game_match=1") != std::string::npos || session.find("startup=started") == std::string::npos ||
        session.find("shutdown=removed") == std::string::npos || session.find("gate=fixture") == std::string::npos) {
        std::printf("FAIL fixture session log:\n%s\n", session.c_str());
        ++g_failures;
    }

    CHECK(api.fixture(directory.c_str(), reinterpret_cast<void*>(&pkt3576_dummy_send),
                      reinterpret_cast<void*>(&pkt3576_dummy_recv), 8, 2) == kPkt3576LiveStarted);
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
