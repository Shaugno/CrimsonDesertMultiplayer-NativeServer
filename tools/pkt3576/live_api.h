#pragma once
// C ABI for the experimental 3576 live-capture diagnostic module.
// The production DLL does not export a fixture entry and has no force flag.
// Not linked into cdmp.asi. Not a game-compatibility result.

#include <stdint.h>

#if defined(PKT3576_LIVE_EXPORT)
#define PKT3576_LIVE_EXPORT_API __declspec(dllexport)
#else
#define PKT3576_LIVE_EXPORT_API
#endif

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
PKT3576_LIVE_EXPORT_API int pkt3576_live_start(const wchar_t* log_directory);
PKT3576_LIVE_EXPORT_API int pkt3576_live_stop(void);

// Out-parameters may be null. capture_attempts counts Capture::start calls.
// It stays 0 when identity or the log path is rejected.
PKT3576_LIVE_EXPORT_API int pkt3576_live_status(int* start_code, int* stop_code, int* retained, unsigned* stored,
                                                unsigned* omitted, unsigned* capture_attempts);

// One-pointer form of status. The controller calls this inside the target.
typedef struct Pkt3576LiveReport {
    int32_t start_code;
    int32_t stop_code;
    int32_t retained;
    uint32_t stored;
    uint32_t omitted;
    uint32_t capture_attempts;
} Pkt3576LiveReport;

PKT3576_LIVE_EXPORT_API int pkt3576_live_report(Pkt3576LiveReport* report);

// Layout for the test-only fixture arm export. The production DLL does not
// export a function that reads this.
typedef struct Pkt3576LiveFixtureArm {
    uint64_t send_entry;
    uint64_t recv_entry;
    uint32_t queue_capacity;
    uint32_t output_records;
    uint32_t reserved;
    wchar_t log_directory[1024];
} Pkt3576LiveFixtureArm;

#ifdef PKT3576_LIVE_FIXTURE
// Test builds only. Arms the approved backend on dummy entries in this
// process. The session log records fixture=1 and game_match=0. This is not
// acceptance of the host as Crimson Desert 1.0.0.3576.
PKT3576_LIVE_EXPORT_API int pkt3576_live_fixture_start(const wchar_t* log_directory, void* send_entry,
                                                       void* recv_entry, unsigned queue_capacity,
                                                       unsigned output_records);

// One-pointer form so a test controller can pass every fixture argument
// without a remote code stub. Production builds do not export this.
PKT3576_LIVE_EXPORT_API int pkt3576_live_fixture_arm(const Pkt3576LiveFixtureArm* request);
#endif

#ifdef __cplusplus
}
#endif
