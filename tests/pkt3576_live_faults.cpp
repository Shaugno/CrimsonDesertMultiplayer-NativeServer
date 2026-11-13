// Game-free regressions for the live runner's stop and log-bind faults.
// Links the fixture runner into this executable so one operator new sees the
// SafetyHook allocation. Does not load a DLL, launch the game, or attach.

#include "pkt3576/live_api.h"
#include "pkt3576/live_runner.h"
#include "pkt3576_capture_targets.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>

#include <windows.h>

namespace {

std::atomic<bool> g_fail_next{false};
std::atomic<size_t> g_fail_bytes{0};
int g_failures = 0;

// The trap-map node is a scalar allocation inside disable(), before entry bytes
// are restored. Aligned and array forms stay available but do not consume the
// one-shot: a later noexcept free uses a different allocation and must not throw.
void* allocate_or_fail(std::size_t size) {
    if (g_fail_next.exchange(false, std::memory_order_acq_rel)) {
        g_fail_bytes.store(size, std::memory_order_relaxed);
        throw std::bad_alloc();
    }
    if (void* pointer = std::malloc(size)) return pointer;
    throw std::bad_alloc();
}

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

void entry_bytes(const void* entry, unsigned char out[16]) { std::memcpy(out, entry, 16); }

bool same_bytes(const unsigned char* left, const unsigned char* right) { return std::memcmp(left, right, 16) == 0; }

bool is_directory(const std::wstring& path) {
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool ensure_directory(const std::wstring& path) {
    if (CreateDirectoryW(path.c_str(), nullptr)) return true;
    return GetLastError() == ERROR_ALREADY_EXISTS && is_directory(path);
}

unsigned attempts() {
    unsigned value = 99;
    pkt3576::live::status(nullptr, nullptr, nullptr, nullptr, nullptr, &value);
    return value;
}

void finish(int code) {
    std::printf("failures=%d\n", g_failures);
    std::fflush(stdout);
    ExitProcess(code);
}

}  // namespace

void* operator new(std::size_t size) { return allocate_or_fail(size); }
void* operator new(std::size_t size, std::align_val_t) {
    if (void* pointer = std::malloc(size)) return pointer;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) {
    if (void* pointer = std::malloc(size)) return pointer;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size, std::align_val_t) {
    if (void* pointer = std::malloc(size)) return pointer;
    throw std::bad_alloc();
}

void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::align_val_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept { std::free(pointer); }

int main() {
    wchar_t temp[MAX_PATH] = {};
    const DWORD temp_len = GetTempPathW(MAX_PATH, temp);
    if (temp_len == 0 || temp_len >= MAX_PATH) return 2;

    const std::wstring root = std::wstring(temp) + L"pkt3576-live-fault-" + std::to_wstring(GetCurrentProcessId());
    const std::wstring file_path = root + L"\\not-a-directory";
    const std::wstring blocked = root + L"\\session-blocked";
    const std::wstring blocked_session = blocked + L"\\pkt3576-live-session.txt";
    const std::wstring good = root + L"\\good";
    void* send = reinterpret_cast<void*>(&pkt3576_dummy_send);
    void* recv = reinterpret_cast<void*>(&pkt3576_dummy_recv);

    unsigned char send_original[16] = {};
    unsigned char recv_original[16] = {};
    entry_bytes(send, send_original);
    entry_bytes(recv, recv_original);

    if (!ensure_directory(root) || !ensure_directory(blocked)) return 3;
    HANDLE file = CreateFileW(file_path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (file == INVALID_HANDLE_VALUE) return 4;
    CloseHandle(file);
    if (!ensure_directory(blocked_session)) return 5;

    const int bad_path = pkt3576::live::fixture_start(file_path.c_str(), send, recv, 8, 8);
    const int session_open = pkt3576::live::fixture_start(blocked.c_str(), send, recv, 8, 8);
    const unsigned attempts_after_reject = attempts();
    unsigned char send_rejected[16] = {};
    unsigned char recv_rejected[16] = {};
    entry_bytes(send, send_rejected);
    entry_bytes(recv, recv_rejected);
    std::printf("bad_path_code=%d session_open_code=%d capture_attempts=%u\n", bad_path, session_open,
                attempts_after_reject);
    CHECK(bad_path == kPkt3576LiveBadLog);
    CHECK(session_open == kPkt3576LiveBadLog);
    CHECK(attempts_after_reject == 0);
    CHECK(same_bytes(send_original, send_rejected));
    CHECK(same_bytes(recv_original, recv_rejected));
    if (g_failures != 0) return 1;

    const int started = pkt3576::live::fixture_start(good.c_str(), send, recv, 8, 8);
    unsigned char send_live[16] = {};
    unsigned char recv_live[16] = {};
    entry_bytes(send, send_live);
    entry_bytes(recv, recv_live);
    const unsigned attempts_started = attempts();
    std::printf("good_start=%d\n", started);
    CHECK(started == kPkt3576LiveStarted);
    CHECK(attempts_started == 1);
    CHECK(!same_bytes(send_original, send_live));
    CHECK(!same_bytes(recv_original, recv_live));
    if (g_failures != 0) finish(1);

    // First stop after arm. Disable allocates the trap node before restoring bytes.
    g_fail_next.store(true, std::memory_order_release);
    bool escaped = false;
    int stopped = -1;
    try {
        stopped = pkt3576::live::stop();
    } catch (const std::bad_alloc&) {
        escaped = true;
    } catch (...) {
        escaped = true;
    }
    g_fail_next.store(false, std::memory_order_release);

    unsigned char send_after[16] = {};
    unsigned char recv_after[16] = {};
    entry_bytes(send, send_after);
    entry_bytes(recv, recv_after);
    const int next_stop = pkt3576::live::stop();
    int start_code = -1;
    int stop_code = -1;
    int retained = -1;
    unsigned capture_attempts = 0;
    pkt3576::live::status(&start_code, &stop_code, &retained, nullptr, nullptr, &capture_attempts);
    const int follow_start = pkt3576::live::fixture_start(good.c_str(), send, recv, 8, 8);
    unsigned follow_attempts = 0;
    pkt3576::live::status(nullptr, nullptr, nullptr, nullptr, nullptr, &follow_attempts);

    std::printf(
        "stop_exception_escaped=%d allocation_size=%zu receive_still_patched=%d send_still_patched=%d next_stop=%d "
        "retained=%d stop_code=%d follow_start=%d follow_attempts=%u\n",
        escaped ? 1 : 0, g_fail_bytes.load(std::memory_order_relaxed),
        same_bytes(recv_original, recv_after) ? 0 : 1, same_bytes(send_original, send_after) ? 0 : 1, next_stop,
        retained, stop_code, follow_start, follow_attempts);

    CHECK(!escaped);
    CHECK(stopped == kPkt3576LiveRetained);
    CHECK(g_fail_bytes.load(std::memory_order_relaxed) != 0);
    CHECK(!same_bytes(recv_original, recv_after));
    CHECK(!same_bytes(send_original, send_after));
    CHECK(next_stop == kPkt3576LiveRetained);
    CHECK(retained == 1);
    CHECK(stop_code == kPkt3576LiveRetained);
    CHECK(start_code == kPkt3576LiveStarted);
    CHECK(follow_start == kPkt3576LiveRetained);
    CHECK(follow_attempts == attempts_started);
    CHECK(capture_attempts == attempts_started);
    finish(g_failures == 0 ? 0 : 1);
    return 1;
}
