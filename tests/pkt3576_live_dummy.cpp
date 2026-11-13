// Dedicated host for the live-capture controller test.
// It is not CrimsonDesert.exe and it does not launch the game.
// Optional argv "sleepers" blocks enough threads to force a Retained stop.

#include "pkt3576_capture_targets.h"

#include <cstdio>
#include <cstring>

#include <windows.h>

#pragma comment(linker, "/EXPORT:pkt3576_dummy_send")
#pragma comment(linker, "/EXPORT:pkt3576_dummy_recv")

namespace {

DWORD WINAPI sleeper_proc(LPVOID param) {
    WaitForSingleObject(static_cast<HANDLE>(param), 120000);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const void* keep_send = reinterpret_cast<const void*>(&pkt3576_dummy_send);
    const void* keep_recv = reinterpret_cast<const void*>(&pkt3576_dummy_recv);
    (void)keep_send;
    (void)keep_recv;

    const bool sleepers = argc >= 2 && argv[1] != nullptr && std::strcmp(argv[1], "sleepers") == 0;
    const DWORD pid = GetCurrentProcessId();
    wchar_t event_name[64] = {};
    swprintf_s(event_name, L"Local\\pkt3576-live-dummy-%lu", pid);
    HANDLE wake = CreateEventW(nullptr, TRUE, FALSE, event_name);
    if (wake == nullptr) return 1;
    ResetEvent(wake);

    constexpr int kSleepers = 320;
    HANDLE threads[kSleepers] = {};
    int spawned = 0;
    if (sleepers) {
        for (int index = 0; index < kSleepers; ++index) {
            threads[index] = CreateThread(nullptr, 64 * 1024, sleeper_proc, wake, 0, nullptr);
            if (threads[index] != nullptr) ++spawned;
        }
    }

    std::printf("pid=%lu\nspawned=%d\nready\n", pid, spawned);
    std::fflush(stdout);
    WaitForSingleObject(wake, 120000);
    SetEvent(wake);
    for (int index = 0; index < kSleepers; ++index) {
        if (threads[index] == nullptr) continue;
        WaitForSingleObject(threads[index], 5000);
        CloseHandle(threads[index]);
    }
    CloseHandle(wake);
    return 0;
}
