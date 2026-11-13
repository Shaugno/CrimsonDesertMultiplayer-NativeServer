// Diagnostic module for an explicit 3576 capture session.
// DllMain does not install hooks, open logs, or start threads.

#include "pkt3576/live_runner.h"

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(instance);
    return TRUE;
}

extern "C" __declspec(dllexport) int pkt3576_live_start(const wchar_t* log_directory) {
    return pkt3576::live::start(log_directory);
}

extern "C" __declspec(dllexport) int pkt3576_live_stop(void) {
    try {
        return pkt3576::live::stop();
    } catch (...) {
        // kPkt3576LiveRetained. An exception must not cross the C ABI.
        return 3;
    }
}

extern "C" __declspec(dllexport) int pkt3576_live_status(int* start_code, int* stop_code, int* retained,
                                                        unsigned* stored, unsigned* omitted,
                                                        unsigned* capture_attempts) {
    return pkt3576::live::status(start_code, stop_code, retained, stored, omitted, capture_attempts);
}

#ifdef PKT3576_LIVE_FIXTURE
extern "C" __declspec(dllexport) int pkt3576_live_fixture_start(const wchar_t* log_directory, void* send_entry,
                                                               void* recv_entry, unsigned queue_capacity,
                                                               unsigned output_records) {
    return pkt3576::live::fixture_start(log_directory, send_entry, recv_entry, queue_capacity, output_records);
}
#endif
