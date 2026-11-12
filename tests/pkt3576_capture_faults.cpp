// Deterministic faults for the capture backend. This translation unit compiles
// capture.cpp so snapshot and allocation failures can be substituted. It does
// not attach to a game or open the compatibility guard.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <thread>

#include <windows.h>
#include <tlhelp32.h>

#include "pkt3576/observer.h"
#include "pkt3576_capture_targets.h"

namespace {

int g_failures = 0;

void fail(const char* message) {
    std::printf("FAIL %s\n", message);
    ++g_failures;
}

enum class Mode { Pass, Vanish, FailScan, FailSuspend };

Mode g_mode = Mode::Pass;
std::atomic<DWORD> g_vanished_id{0};
std::atomic<int> g_vanished_attempts{0};
bool g_scan_failed = false;
HANDLE g_opened[256]{};
int g_opened_count = 0;
int g_closed_count = 0;
int g_suspend_ok = 0;
int g_resume_calls = 0;
std::atomic<size_t> g_fail_size{0};
std::atomic<int> g_injections{0};
std::atomic<int> g_hook_installs{0};
std::atomic<int> g_hook_fail_install{0};
std::atomic<size_t> g_injection_size{0};
unsigned char g_recv_original[16]{};
unsigned char g_send_original[16]{};
bool g_have_originals = false;
bool g_patched_at_inject = false;

bool entry_changed(const void* entry, const unsigned char* original) {
    if (!g_have_originals) return false;
    return std::memcmp(entry, original, 16) != 0;
}

void reset_scan() {
    g_mode = Mode::Pass;
    g_vanished_id.store(0);
    g_vanished_attempts.store(0);
    g_scan_failed = false;
    g_opened_count = 0;
    g_closed_count = 0;
    g_suspend_ok = 0;
    g_resume_calls = 0;
}

HANDLE WINAPI review_OpenThread(DWORD access, BOOL inherit, DWORD id) {
    if (g_mode == Mode::Vanish && id == g_vanished_id.load()) {
        const int attempt = g_vanished_attempts.fetch_add(1);
        if (attempt == 0) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return nullptr;
        }
    }
    const HANDLE handle = ::OpenThread(access, inherit, id);
    if (handle != nullptr && g_opened_count < 256) g_opened[g_opened_count++] = handle;
    return handle;
}

BOOL WINAPI review_Thread32Next(HANDLE snapshot, LPTHREADENTRY32 entry) {
    if (g_mode == Mode::FailScan && g_opened_count >= 1 && !g_scan_failed) {
        g_scan_failed = true;
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    return ::Thread32Next(snapshot, entry);
}

BOOL WINAPI review_CloseHandle(HANDLE handle) {
    for (int index = 0; index < g_opened_count; ++index) {
        if (g_opened[index] == handle) {
            ++g_closed_count;
            break;
        }
    }
    return ::CloseHandle(handle);
}

DWORD WINAPI review_SuspendThread(HANDLE handle) {
    if (g_mode == Mode::FailSuspend && g_suspend_ok == 1) {
        SetLastError(ERROR_ACCESS_DENIED);
        return static_cast<DWORD>(-1);
    }
    const DWORD result = ::SuspendThread(handle);
    if (result != static_cast<DWORD>(-1)) ++g_suspend_ok;
    return result;
}

DWORD WINAPI review_ResumeThread(HANDLE handle) {
    ++g_resume_calls;
    return ::ResumeThread(handle);
}

struct Worker {
    std::atomic<bool> run{true};
    std::atomic<DWORD> id{0};
    std::thread thread;

    void start() {
        thread = std::thread([this] {
            id.store(GetCurrentThreadId());
            while (run.load(std::memory_order_acquire)) Sleep(1);
        });
        while (id.load() == 0) YieldProcessor();
    }

    void join() {
        run.store(false, std::memory_order_release);
        if (thread.joinable()) thread.join();
    }
};

}  // namespace

void* operator new(std::size_t size) {
    const std::size_t trigger = g_fail_size.load();
    if (trigger != 0 && size >= trigger && size <= trigger + 128) {
        g_injections.fetch_add(1);
        throw std::bad_alloc();
    }
    const int hook_fail = g_hook_fail_install.load();
    if (hook_fail != 0 && g_hook_installs.load() == hook_fail) {
        g_hook_fail_install.store(0);
        g_injections.fetch_add(1);
        g_injection_size.store(size);
        g_patched_at_inject = entry_changed(reinterpret_cast<const void*>(&pkt3576_dummy_recv), g_recv_original);
        throw std::bad_alloc();
    }
    if (void* pointer = std::malloc(size)) return pointer;
    throw std::bad_alloc();
}

void* operator new(std::size_t size, std::align_val_t) { return operator new(size); }

void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::align_val_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept { std::free(pointer); }

struct HookInstallScope {
    HookInstallScope() { g_hook_installs.fetch_add(1); }
};

#define PKT3576_HOOK_SCOPE() HookInstallScope pkt3576_hook_install_scope
#define OpenThread review_OpenThread
#define Thread32Next review_Thread32Next
#define CloseHandle review_CloseHandle
#define SuspendThread review_SuspendThread
#define ResumeThread review_ResumeThread
#include "../tools/pkt3576/capture.cpp"
#undef OpenThread
#undef Thread32Next
#undef CloseHandle
#undef SuspendThread
#undef ResumeThread

namespace {

bool handles_closed() { return g_opened_count > 0 && g_closed_count == g_opened_count; }

void test_vanished_thread_advances() {
    reset_scan();
    Worker vanished;
    Worker other;
    vanished.start();
    other.start();
    g_mode = Mode::Vanish;
    g_vanished_id.store(vanished.id.load());
    {
        const auto paused = pkt3576::PausedThreads::suspend_others();
        if (!paused.complete) fail("vanished-thread scan did not complete");
    }
    if (g_vanished_attempts.load() != 1) {
        std::printf("FAIL vanished attempts=%d\n", g_vanished_attempts.load());
        ++g_failures;
    }
    if (!handles_closed()) {
        std::printf("FAIL vanished handles opened=%d closed=%d\n", g_opened_count, g_closed_count);
        ++g_failures;
    }
    if (g_resume_calls != g_suspend_ok) {
        std::printf("FAIL vanished suspend=%d resume=%d\n", g_suspend_ok, g_resume_calls);
        ++g_failures;
    }
    vanished.join();
    other.join();
}

void test_scan_error_closes_handles() {
    reset_scan();
    Worker worker;
    worker.start();
    g_mode = Mode::FailScan;
    {
        const auto paused = pkt3576::PausedThreads::suspend_others();
        if (paused.complete) fail("scan error was treated as a complete snapshot");
    }
    if (!g_scan_failed) fail("scan error was not injected");
    if (g_suspend_ok != 0 || g_resume_calls != 0) fail("scan error suspended a thread");
    if (!handles_closed()) fail("scan error leaked a thread handle");
    worker.join();
}

void test_suspend_failure_resumes_and_closes() {
    reset_scan();
    Worker first;
    Worker second;
    first.start();
    second.start();
    g_mode = Mode::FailSuspend;
    {
        const auto paused = pkt3576::PausedThreads::suspend_others();
        if (paused.complete) fail("suspend failure was treated as quiesced");
    }
    if (g_suspend_ok != 1) fail("suspend failure did not follow one successful suspend");
    if (g_resume_calls != 1) fail("suspend failure left a thread suspended");
    if (!handles_closed()) fail("suspend failure leaked a thread handle");
    first.join();
    second.join();
}

struct Owned {
    pkt3576::Capture* raw = nullptr;
    explicit Owned(pkt3576::Capture* capture) : raw(capture) {}
    ~Owned() { pkt3576::Capture::release_owner(raw); }
    pkt3576::Capture* operator->() const { return raw; }
};

void test_allocation_failure_can_retry() {
    reset_scan();
    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 4;
    limits.output_records = 257;
    g_injections.store(0);
    g_fail_size.store(sizeof(pkt3576::Record) * limits.output_records);

    Owned first(pkt3576::Capture::create(limits, nullptr));
    if (first.raw == nullptr) {
        fail("capture create failed");
        g_fail_size.store(0);
        return;
    }
    bool escaped = false;
    pkt3576::Capture::StartResult refused = pkt3576::Capture::StartResult::Started;
    try {
        refused = first->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                               reinterpret_cast<void*>(&pkt3576_dummy_recv));
    } catch (const std::bad_alloc&) {
        escaped = true;
    }
    g_fail_size.store(0);
    if (escaped) fail("allocation failure escaped start");
    if (g_injections.load() < 1) fail("reserve allocation was not injected");
    if (refused != pkt3576::Capture::StartResult::Refused) fail("allocation failure was not refused");

    Owned second(pkt3576::Capture::create(limits, nullptr));
    if (second.raw == nullptr) {
        fail("second capture create failed");
        return;
    }
    const auto started = second->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                                        reinterpret_cast<void*>(&pkt3576_dummy_recv));
    if (started != pkt3576::Capture::StartResult::Started) fail("start after allocation failure was refused");
    if (started == pkt3576::Capture::StartResult::Started &&
        pkt3576_dummy_send(1, 2, 3, nullptr, 0, 0, 0) != pktfix::kSendReturn) {
        fail("dummy send return changed after recovery");
    }
    if (started == pkt3576::Capture::StartResult::Started &&
        second->stop() != pkt3576::Capture::StopResult::Removed) {
        fail("stop after recovery did not remove the hooks");
    }
}

void test_hook_allocation(int which) {
    reset_scan();
    g_hook_installs.store(0);
    g_hook_fail_install.store(which);
    g_injections.store(0);
    g_injection_size.store(0);
    g_patched_at_inject = false;

    pkt3576::CaptureLimits limits;
    limits.queue_capacity = 4;
    limits.output_records = 4;
    Owned owned(pkt3576::Capture::create(limits, nullptr));
    if (owned.raw == nullptr) {
        fail("hook-allocation capture create failed");
        g_hook_fail_install.store(0);
        return;
    }

    bool escaped = false;
    auto result = pkt3576::Capture::StartResult::Started;
    try {
        result = owned->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                              reinterpret_cast<void*>(&pkt3576_dummy_recv));
    } catch (const std::bad_alloc&) {
        escaped = true;
    }
    g_hook_fail_install.store(0);

    const bool recv_intact = !entry_changed(reinterpret_cast<const void*>(&pkt3576_dummy_recv), g_recv_original);
    const bool send_intact = !entry_changed(reinterpret_cast<const void*>(&pkt3576_dummy_send), g_send_original);
    const auto stopped = owned->stop();
    const char* which_name = which == 1 ? "first" : "second";
    if (escaped) fail(which == 1 ? "first-hook allocation escaped start" : "second-hook allocation escaped start");
    if (g_injections.load() != 1) {
        std::printf("FAIL %s-hook injections=%d size=%zu\n", which_name, g_injections.load(),
                    g_injection_size.load());
        ++g_failures;
    }
    if (which == 1 && g_patched_at_inject) fail("first-hook allocation ran after receive was patched");
    if (which == 2 && !g_patched_at_inject) fail("second-hook allocation ran before receive was patched");
    if (!recv_intact || !send_intact) {
        std::printf("FAIL %s-hook left patched recv=%d send=%d\n", which_name, !recv_intact, !send_intact);
        ++g_failures;
    }
    const auto expected = which == 1 ? pkt3576::Capture::StartResult::Refused
                                      : pkt3576::Capture::StartResult::RolledBack;
    if (result != expected) {
        std::printf("FAIL %s-hook result=%d\n", which_name, static_cast<int>(result));
        ++g_failures;
    }
    if (stopped != pkt3576::Capture::StopResult::Idle) {
        std::printf("FAIL %s-hook stop=%d\n", which_name, static_cast<int>(stopped));
        ++g_failures;
    }

    g_hook_installs.store(0);
    const auto started = owned->start(GetModuleHandleW(nullptr), reinterpret_cast<void*>(&pkt3576_dummy_send),
                                       reinterpret_cast<void*>(&pkt3576_dummy_recv));
    if (started != pkt3576::Capture::StartResult::Started) {
        fail(which == 1 ? "retry after first-hook allocation was refused" : "retry after second-hook allocation was refused");
        return;
    }
    if (pkt3576_dummy_send(1, 2, 3, nullptr, 0, 0, 0) != pktfix::kSendReturn) {
        fail(which == 1 ? "dummy send return changed after first-hook recovery" : "dummy send return changed after second-hook recovery");
    }
    if (owned->stop() != pkt3576::Capture::StopResult::Removed) {
        fail(which == 1 ? "stop after first-hook recovery did not remove the hooks" : "stop after second-hook recovery did not remove the hooks");
    }
}

}  // namespace

int main() {
    std::memcpy(g_recv_original, reinterpret_cast<const void*>(&pkt3576_dummy_recv), 16);
    std::memcpy(g_send_original, reinterpret_cast<const void*>(&pkt3576_dummy_send), 16);
    g_have_originals = true;
    test_vanished_thread_advances();
    test_scan_error_closes_handles();
    test_suspend_failure_resumes_and_closes();
    test_allocation_failure_can_retry();
    test_hook_allocation(1);
    test_hook_allocation(2);
    if (g_failures != 0) {
        std::printf("%d capture fault failure(s)\n", g_failures);
        return 1;
    }
    std::puts("pkt3576 capture fault tests ok");
    return 0;
}
