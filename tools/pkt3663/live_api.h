#pragma once
// C ABI for the isolated 1.0.0.3663 raw packet-capture diagnostic.
// This module is not cdmp.asi, does not open the mod compatibility guard,
// and has no packet injection, replay, or caller-supplied production RVAs.

#include <stdint.h>

#if defined(PKT3663_LIVE_EXPORT)
#define PKT3663_LIVE_EXPORT_API __declspec(dllexport)
#else
#define PKT3663_LIVE_EXPORT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum Pkt3663LiveCode {
  kPkt3663LiveStarted = 0,
  kPkt3663LiveRefused = 1,
  kPkt3663LiveRolledBack = 2,
  kPkt3663LiveRetained = 3,
  kPkt3663LiveRemoved = 4,
  kPkt3663LiveIdle = 5,
  kPkt3663LiveBadLog = 6,
};

// The production entry derives both packet addresses internally, after a
// strict mapped-image verification of the current process. It accepts no PID,
// RVA, force flag, or identity override.
PKT3663_LIVE_EXPORT_API int pkt3663_live_start(const wchar_t *log_directory);
PKT3663_LIVE_EXPORT_API int pkt3663_live_stop(void);

typedef struct Pkt3663LiveReport {
  int32_t start_code;
  int32_t stop_code;
  int32_t retained;
  uint32_t stored;
  uint32_t omitted;
  uint32_t capture_attempts;
  uint32_t unread;
  uint32_t truncated;
  uint32_t phase;
  uint32_t raw_mode;
  uint32_t allowlist_count;
  uint32_t adopted;
  uint64_t qpc_frequency;
} Pkt3663LiveReport;

PKT3663_LIVE_EXPORT_API int pkt3663_live_report(Pkt3663LiveReport *report);

// A phase is selected before start and cannot be changed while a capture is
// active. Accepted values are stand, walk, turn, and unlabeled.
PKT3663_LIVE_EXPORT_API int pkt3663_live_mark(const char *phase);

typedef struct Pkt3663LiveFixtureArm {
  uint64_t send_entry;
  uint64_t recv_entry;
  uint32_t queue_capacity;
  uint32_t output_records;
  uint32_t reserved;
  wchar_t log_directory[1024];
} Pkt3663LiveFixtureArm;

#ifdef PKT3663_LIVE_FIXTURE
// Test-only dummy-host entry. A fixture is always logged as fixture=1 and
// game_match=0. The production DLL does not export either fixture function.
PKT3663_LIVE_EXPORT_API int
pkt3663_live_fixture_start(const wchar_t *log_directory, void *send_entry,
                           void *recv_entry, unsigned queue_capacity,
                           unsigned output_records);
PKT3663_LIVE_EXPORT_API int
pkt3663_live_fixture_arm(const Pkt3663LiveFixtureArm *request);

typedef void (*Pkt3663LiveFixtureHook)(void *user);
PKT3663_LIVE_EXPORT_API void
pkt3663_live_fixture_set_records_open_hook(Pkt3663LiveFixtureHook hook,
                                           void *user);
PKT3663_LIVE_EXPORT_API void
pkt3663_live_fixture_set_phase_hold(Pkt3663LiveFixtureHook hold, void *user);
// Game-free fault seam. The next flush fails after this many successful
// flushes; values above one million disable the injected failure.
PKT3663_LIVE_EXPORT_API void
pkt3663_live_fixture_fail_log_flush_after(unsigned successful_flushes);
// The next session rewrite whose buffer contains this text opens "wb" and
// then fails the write, so a timing line cannot erase a committed block
// while start still reports success.
PKT3663_LIVE_EXPORT_API void
pkt3663_live_fixture_fail_session_short_write_containing(const char *needle);
#endif

#ifdef __cplusplus
}
#endif
