// Isolated controller double. It installs no hooks and does not read game
// memory. The test copies it to the fixture file name and selects a report
// failure with pkt3663_stub_configure. Report modes are 1 error, 2 unreadable,
// 3 timeout, and 4 a readable snapshot that is milder than the command.

#define PKT3663_LIVE_EXPORT
#include "pkt3663/live_api.h"

#include <atomic>
#include <cstdint>

#include <windows.h>

namespace {

std::atomic<int> g_arm{0};
std::atomic<int> g_stop{5};
std::atomic<int> g_mark{0};
std::atomic<int> g_report{1};
std::atomic<int> g_arm_calls{0};
std::atomic<unsigned> g_delay_result{0};

constexpr int kStubDelayStart = 5;

} // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH)
    DisableThreadLibraryCalls(instance);
  return TRUE;
}

extern "C" __declspec(dllexport) unsigned pkt3663_stub_configure(uintptr_t packed) {
  g_arm.store(static_cast<int>(packed & 0xFFu), std::memory_order_release);
  g_stop.store(static_cast<int>((packed >> 8) & 0xFFu), std::memory_order_release);
  g_mark.store(static_cast<int>((packed >> 16) & 0xFFu), std::memory_order_release);
  g_report.store(static_cast<int>((packed >> 24) & 0xFFu), std::memory_order_release);
  return 0;
}

extern "C" int pkt3663_live_start(const wchar_t *) {
  return kPkt3663LiveRefused;
}

extern "C" __declspec(dllexport) unsigned pkt3663_stub_delayed_start_result(void) {
  return g_delay_result.load(std::memory_order_acquire);
}

extern "C" __declspec(dllexport) int pkt3663_live_fixture_arm(const void *request) {
  if (g_report.load(std::memory_order_acquire) == kStubDelayStart) {
    g_arm_calls.fetch_add(1, std::memory_order_acq_rel);
    Sleep(22000);
    unsigned result = 1;
    MEMORY_BASIC_INFORMATION info{};
    if (request != nullptr &&
        VirtualQuery(request, &info, sizeof info) == sizeof info &&
        info.State == MEM_COMMIT &&
        info.RegionSize >= sizeof(Pkt3663LiveFixtureArm)) {
      const auto *arm = static_cast<const Pkt3663LiveFixtureArm *>(request);
      if (arm->log_directory[0] != L'\0' && arm->log_directory[1023] == L'\0')
        result |= 2u;
    }
    const unsigned calls =
        static_cast<unsigned>(g_arm_calls.load(std::memory_order_acquire));
    result |= (calls & 0xFFu) << 8;
    g_delay_result.store(result, std::memory_order_release);
    return kPkt3663LiveStarted;
  }
  return g_arm.load(std::memory_order_acquire);
}

extern "C" int pkt3663_live_stop(void) {
  return g_stop.load(std::memory_order_acquire);
}

extern "C" int pkt3663_live_mark(const char *) {
  return g_mark.load(std::memory_order_acquire);
}

extern "C" int pkt3663_live_report(Pkt3663LiveReport *report) {
  const int mode = g_report.load(std::memory_order_acquire);
  if (mode == 2) {
    // ReadProcessMemory can still copy a no-access page. Releasing the
    // controller's report buffer makes that copy fail while this call
    // itself returns success.
    if (report == nullptr || VirtualFree(report, 0, MEM_RELEASE) == FALSE) {
      return kPkt3663LiveRefused;
    }
    return 0;
  }
  if (mode == 3) {
    Sleep(60000);
    return 0;
  }
  if (mode == 4) {
    if (report == nullptr) {
      return kPkt3663LiveRefused;
    }
    Pkt3663LiveReport mild{};
    mild.start_code = kPkt3663LiveStarted;
    mild.stop_code = kPkt3663LiveRemoved;
    mild.raw_mode = 1;
    mild.qpc_frequency = 1;
    *report = mild;
    return 0;
  }
  if (mode == kStubDelayStart) {
    if (report == nullptr || (g_delay_result.load(std::memory_order_acquire) & 1u) == 0)
      return kPkt3663LiveRefused;
    Pkt3663LiveReport finished{};
    finished.start_code = kPkt3663LiveStarted;
    finished.stop_code = -1;
    finished.raw_mode = 1;
    finished.qpc_frequency = 1;
    *report = finished;
    return 0;
  }
  return kPkt3663LiveRefused;
}
