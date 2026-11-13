// Diagnostic module for an explicit 3576 capture session.
// DllMain does not install hooks, open logs, or start threads.

#include "pkt3576/live_api.h"
#include "pkt3576/live_runner.h"

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(instance);
    return TRUE;
}

int pkt3576_live_start(const wchar_t* log_directory) { return pkt3576::live::start(log_directory); }

int pkt3576_live_stop(void) {
    try {
        return pkt3576::live::stop();
    } catch (...) {
        // kPkt3576LiveRetained. An exception must not cross the C ABI.
        return 3;
    }
}

int pkt3576_live_status(int* start_code, int* stop_code, int* retained, unsigned* stored, unsigned* omitted,
                        unsigned* capture_attempts) {
    return pkt3576::live::status(start_code, stop_code, retained, stored, omitted, capture_attempts);
}

int pkt3576_live_report(Pkt3576LiveReport* report) {
    if (report == nullptr) return 1;
    unsigned stored = 0;
    unsigned omitted = 0;
    unsigned attempts = 0;
    const int code = pkt3576::live::status(&report->start_code, &report->stop_code, &report->retained, &stored,
                                           &omitted, &attempts);
    report->stored = stored;
    report->omitted = omitted;
    report->capture_attempts = attempts;
    return code;
}

#ifdef PKT3576_LIVE_FIXTURE
int pkt3576_live_fixture_start(const wchar_t* log_directory, void* send_entry, void* recv_entry,
                               unsigned queue_capacity, unsigned output_records) {
    return pkt3576::live::fixture_start(log_directory, send_entry, recv_entry, queue_capacity, output_records);
}

int pkt3576_live_fixture_arm(const Pkt3576LiveFixtureArm* request) {
    if (request == nullptr || request->log_directory[0] == L'\0') return 6;
    if (request->send_entry == 0 || request->recv_entry == 0) return 1;
    return pkt3576::live::fixture_start(request->log_directory, reinterpret_cast<void*>(request->send_entry),
                                        reinterpret_cast<void*>(request->recv_entry), request->queue_capacity,
                                        request->output_records);
}
#endif
