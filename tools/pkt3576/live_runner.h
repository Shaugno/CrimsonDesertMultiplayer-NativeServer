#pragma once
// In-process controller for the experimental 3576 diagnostic module.
// The C exports live in live_dll.cpp. This header is not a game API.

#include <cstdint>

namespace pkt3576::live {

int start(const wchar_t* log_directory);
int stop();
int status(int* start_code, int* stop_code, int* retained, unsigned* stored, unsigned* omitted,
           unsigned* capture_attempts);

#ifdef PKT3576_LIVE_FIXTURE
int fixture_start(const wchar_t* log_directory, void* send_entry, void* recv_entry, unsigned queue_capacity,
                  unsigned output_records);
#endif

}  // namespace pkt3576::live
