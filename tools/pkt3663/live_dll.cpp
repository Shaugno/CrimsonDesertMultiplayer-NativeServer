#include "pkt3663/live_api.h"
#include "pkt3663/live_runner.h"

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH)
    DisableThreadLibraryCalls(instance);
  return TRUE;
}

extern "C" int pkt3663_live_start(const wchar_t *log_directory) {
  try {
    return pkt3663::live::start(log_directory);
  } catch (...) {
    return kPkt3663LiveRefused;
  }
}

extern "C" int pkt3663_live_stop() {
  try {
    return pkt3663::live::stop();
  } catch (...) {
    // An exception during teardown cannot prove that callback code is no
    // longer reachable, so the exported boundary reports Retained.
    return kPkt3663LiveRetained;
  }
}

extern "C" int pkt3663_live_report(Pkt3663LiveReport *report) {
  if (report == nullptr)
    return kPkt3663LiveRefused;
  try {
    pkt3663::live::write_report(report);
    return 0;
  } catch (...) {
    return kPkt3663LiveRefused;
  }
}

extern "C" int pkt3663_live_mark(const char *phase) {
  try {
    return pkt3663::live::mark(phase);
  } catch (...) {
    return kPkt3663LiveRefused;
  }
}

#ifdef PKT3663_LIVE_FIXTURE
extern "C" int pkt3663_live_fixture_start(const wchar_t *log_directory,
                                          void *send_entry, void *recv_entry,
                                          unsigned queue_capacity,
                                          unsigned output_records) {
  try {
    return pkt3663::live::fixture_start(log_directory, send_entry, recv_entry,
                                        queue_capacity, output_records);
  } catch (...) {
    return kPkt3663LiveRefused;
  }
}

extern "C" int pkt3663_live_fixture_arm(const Pkt3663LiveFixtureArm *request) {
  if (request == nullptr || request->reserved != 0 ||
      request->log_directory[1023] != L'\0') {
    return kPkt3663LiveRefused;
  }
  if (request->log_directory[0] == L'\0' || request->send_entry == 0 ||
      request->recv_entry == 0) {
    return kPkt3663LiveRefused;
  }
  try {
    return pkt3663::live::fixture_start(
        request->log_directory, reinterpret_cast<void *>(request->send_entry),
        reinterpret_cast<void *>(request->recv_entry), request->queue_capacity,
        request->output_records);
  } catch (...) {
    return kPkt3663LiveRefused;
  }
}

extern "C" void
pkt3663_live_fixture_set_records_open_hook(Pkt3663LiveFixtureHook hook,
                                           void *user) {
  pkt3663::live::set_records_open_hook(hook, user);
}

extern "C" void pkt3663_live_fixture_set_phase_hold(Pkt3663LiveFixtureHook hold,
                                                    void *user) {
  pkt3663::live::set_phase_hold(hold, user);
}

extern "C" void
pkt3663_live_fixture_fail_log_flush_after(unsigned successful_flushes) {
  pkt3663::live::fail_log_flush_after(successful_flushes);
}
#endif
