#pragma once
// C ABI for the experimental 3576 live-capture diagnostic module.
// The production DLL does not export a fixture entry and has no force flag.
// Not linked into cdmp.asi. Not a game-compatibility result.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum Pkt3576LiveCode {
    kPkt3576LiveStarted = 0,
    kPkt3576LiveRefused = 1,
    kPkt3576LiveRolledBack = 2,
    kPkt3576LiveRetained = 3,
    kPkt3576LiveRemoved = 4,
    kPkt3576LiveIdle = 5,
    kPkt3576LiveBadLog = 6,
};

// Absolute directory outside the game bin64. Hooks run only after the loaded
// executable matches the documented 1.0.0.3576 identity. Does not launch a
// process and does not accept another process's pointers.
int pkt3576_live_start(const wchar_t* log_directory);
int pkt3576_live_stop(void);

// Out-parameters may be null. capture_attempts counts Capture::start calls.
// It stays 0 when identity or the log path is rejected.
int pkt3576_live_status(int* start_code, int* stop_code, int* retained, unsigned* stored, unsigned* omitted,
                        unsigned* capture_attempts);

#ifdef PKT3576_LIVE_FIXTURE
// Test builds only. Arms the approved backend on dummy entries in this
// process. The session log records fixture=1 and game_match=0. This is not
// acceptance of the host as Crimson Desert 1.0.0.3576.
int pkt3576_live_fixture_start(const wchar_t* log_directory, void* send_entry, void* recv_entry,
                               unsigned queue_capacity, unsigned output_records);
#endif

#ifdef __cplusplus
}
#endif
