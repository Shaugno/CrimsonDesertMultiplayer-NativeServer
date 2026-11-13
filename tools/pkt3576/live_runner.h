#pragma once
// In-process controller for the experimental 3576 diagnostic module.
// The C exports live in live_dll.cpp. This header is not a game API.

#include <cstdint>

struct Pkt3576LiveReport;

namespace pkt3576::live {

int start(const wchar_t* log_directory);
int stop();
int status(int* start_code, int* stop_code, int* retained, unsigned* stored, unsigned* omitted,
           unsigned* capture_attempts);
// stand, walk, turn, or unlabeled. Same QueryPerformanceCounter time base as
// each stored record. Shares the control lock with start, stop, status, and
// report. Does not retarget the log directory and does not overwrite an
// existing phase archive.
int mark(const char* phase);
void write_report(Pkt3576LiveReport* report);

#ifdef PKT3576_LIVE_FIXTURE
int fixture_start(const wchar_t* log_directory, void* send_entry, void* recv_entry, unsigned queue_capacity,
                  unsigned output_records);

// Test scheduling only. A hook must not call start, stop, mark, status, or report.
void set_records_open_hook(void (*hook)(void*), void* user);
void set_phase_hold(void (*hold)(void*), void* user);
#endif

}  // namespace pkt3576::live
