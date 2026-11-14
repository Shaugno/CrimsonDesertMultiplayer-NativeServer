// Game-free host for the separate 3663 production and fixture DLLs.
// It never starts, opens, or attaches to Crimson Desert.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <windows.h>

#include "pkt3663/live_api.h"
#include "pkt3663_capture_targets.h"

namespace {

int g_failures = 0;

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);         \
      ++g_failures;                                                            \
    }                                                                          \
  } while (0)

using StartFn = int (*)(const wchar_t *);
using StopFn = int (*)();
using MarkFn = int (*)(const char *);
using ReportFn = int (*)(Pkt3663LiveReport *);
using FixtureFn = int (*)(const wchar_t *, void *, void *, unsigned, unsigned);
using FailFlushFn = void (*)(unsigned);

struct Api {
  HMODULE module = nullptr;
  StartFn start = nullptr;
  StopFn stop = nullptr;
  MarkFn mark = nullptr;
  ReportFn report = nullptr;
  FixtureFn fixture = nullptr;
  FailFlushFn fail_flush_after = nullptr;
};

std::wstring beside(const wchar_t *file) {
  wchar_t path[1024] = {};
  const DWORD length = GetModuleFileNameW(nullptr, path, 1024);
  if (length == 0 || length >= 1024)
    return {};
  wchar_t *slash = nullptr;
  for (wchar_t *cursor = path; *cursor != L'\0'; ++cursor) {
    if (*cursor == L'\\' || *cursor == L'/')
      slash = cursor;
  }
  if (slash == nullptr)
    return {};
  slash[1] = L'\0';
  return std::wstring(path) + file;
}

std::wstring temp_directory(const wchar_t *suffix) {
  wchar_t temp[MAX_PATH] = {};
  GetTempPathW(MAX_PATH, temp);
  return std::wstring(temp) + L"pkt3663-live-" +
         std::to_wstring(GetCurrentProcessId()) + L"-" +
         std::to_wstring(GetTickCount64()) + L"-" + suffix;
}

std::string read_file(const std::wstring &path) {
  FILE *file = nullptr;
  if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr)
    return {};
  std::string result;
  char buffer[512] = {};
  size_t count = 0;
  while ((count = std::fread(buffer, 1, sizeof buffer, file)) != 0)
    result.append(buffer, count);
  std::fclose(file);
  return result;
}

Api load_api(const wchar_t *name, bool fixture) {
  Api api;
  const std::wstring path = beside(name);
  api.module = LoadLibraryW(path.c_str());
  CHECK(api.module != nullptr);
  if (api.module == nullptr)
    return api;
  api.start = reinterpret_cast<StartFn>(
      GetProcAddress(api.module, "pkt3663_live_start"));
  api.stop =
      reinterpret_cast<StopFn>(GetProcAddress(api.module, "pkt3663_live_stop"));
  api.mark =
      reinterpret_cast<MarkFn>(GetProcAddress(api.module, "pkt3663_live_mark"));
  api.report = reinterpret_cast<ReportFn>(
      GetProcAddress(api.module, "pkt3663_live_report"));
  api.fixture = reinterpret_cast<FixtureFn>(
      GetProcAddress(api.module, "pkt3663_live_fixture_start"));
  api.fail_flush_after = reinterpret_cast<FailFlushFn>(
      GetProcAddress(api.module, "pkt3663_live_fixture_fail_log_flush_after"));
  CHECK(api.start != nullptr && api.stop != nullptr && api.mark != nullptr &&
        api.report != nullptr);
  CHECK((api.fixture != nullptr) == fixture);
  CHECK((api.fail_flush_after != nullptr) == fixture);
  CHECK((GetProcAddress(api.module, "pkt3663_live_fixture_arm") != nullptr) ==
        fixture);
  CHECK((GetProcAddress(api.module,
                        "pkt3663_live_fixture_set_records_open_hook") !=
         nullptr) == fixture);
  CHECK((GetProcAddress(api.module, "pkt3663_live_fixture_set_phase_hold") !=
         nullptr) == fixture);
  return api;
}

Pkt3663LiveReport report_of(const Api &api) {
  Pkt3663LiveReport report{};
  CHECK(api.report != nullptr && api.report(&report) == 0);
  CHECK(report.raw_mode == 1);
  CHECK(report.allowlist_count == 0);
  CHECK(report.adopted == 0);
  return report;
}

void test_production_refuses_dummy_host() {
  Api api = load_api(L"cdmp_pkt3663_live.dll", false);
  if (api.module == nullptr || api.start == nullptr)
    return;
  const std::wstring directory = temp_directory(L"production");

  CHECK(api.stop() == kPkt3663LiveIdle);
  CHECK(api.start(directory.c_str()) == kPkt3663LiveRefused);
  const std::string sealed_refusal =
      read_file(directory + L"\\pkt3663-live-session-1.txt");
  CHECK(GetProcAddress(api.module,
                       "pkt3663_live_fixture_fail_session_short_write_containing") ==
        nullptr);
  const Pkt3663LiveReport report = report_of(api);
  CHECK(report.capture_attempts == 0);
  CHECK(report.retained == 0);
  const std::string session =
      read_file(directory + L"\\pkt3663-live-session.txt");
  CHECK(session == sealed_refusal);
  CHECK(session.find("fixture=0") != std::string::npos);
  CHECK(session.find("game_match=0") != std::string::npos);
  CHECK(session.find("kind=wrong-name") != std::string::npos);
  CHECK(session.find("startup=refused") != std::string::npos);
  CHECK(session.find("capture_attempts=0") != std::string::npos);
  CHECK(session.find("recording_ready=0") != std::string::npos);
  CHECK(session.find("stage=export-entry qpc=") != std::string::npos);
  CHECK(session.find("stage=identity qpc=") != std::string::npos);
  CHECK(session.find("elapsed_us=") != std::string::npos);
  CHECK(session.find("stage=ready-durable") == std::string::npos);
  CHECK(session.find("capture_ready_qpc=0") != std::string::npos);
  CHECK(session.find("capture_mode=raw-all") != std::string::npos);
  CHECK(session.find("allowlist_count=0") != std::string::npos);
  CHECK(session.find("adopted=0") != std::string::npos);

  CHECK(api.mark("walk") == 0);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_refusal);
  const std::string pending =
      read_file(directory + L"\\pkt3663-live-session-pending.txt");
  CHECK(pending.find("mark phase=walk qpc=") != std::string::npos);
  const int retried = api.start(directory.c_str());
  CHECK(retried == kPkt3663LiveRefused);
  const Pkt3663LiveReport retried_report = report_of(api);
  CHECK(retried_report.start_code == kPkt3663LiveRefused);
  CHECK(retried_report.capture_attempts == 0);
  CHECK(retried_report.retained == 0);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_refusal);
  const std::string second =
      read_file(directory + L"\\pkt3663-live-session-2.txt");
  CHECK(second.find("startup=refused") != std::string::npos);
  CHECK(second.find("stage=identity qpc=") != std::string::npos);
  CHECK(second.find("recording_ready=0") != std::string::npos);
  std::printf("observed refusal-retry start=%d session1_bytes=%zu pending_bytes=%zu "
              "session2_bytes=%zu\n",
              retried, sealed_refusal.size(), pending.size(), second.size());

  CHECK(FreeLibrary(api.module) != FALSE);
  CHECK(GetModuleHandleW(L"cdmp_pkt3663_live.dll") == nullptr);
}

void test_fixture_raw_capture() {
  Api api = load_api(L"cdmp_pkt3663_live_fixture.dll", true);
  if (api.module == nullptr || api.fixture == nullptr)
    return;
  const std::wstring directory = temp_directory(L"fixture");

  CHECK(api.fixture(L"pkt3663-relative-must-not-exist",
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    4) == kPkt3663LiveBadLog);
  CHECK(GetFileAttributesW(L"pkt3663-relative-must-not-exist") ==
        INVALID_FILE_ATTRIBUTES);
  CHECK(api.fixture(L"C:\\diagnostic\\Crimson Desert\\bin64\\blocked",
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    4) == kPkt3663LiveBadLog);
  CHECK(report_of(api).capture_attempts == 0);
  CHECK(api.mark("not-a-phase") == kPkt3663LiveBadLog);
  CHECK(api.mark("stand") == 0);

  CHECK(api.fixture(directory.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    4) == kPkt3663LiveStarted);
  CHECK(report_of(api).capture_attempts == 1);
  CHECK(api.mark("walk") == kPkt3663LiveRefused);

  uint8_t frame3663[4] = {0xE6, 0x0A, 0xAB, 0xCD}; // raw word 2790
  uint8_t frame3576[4] = {0x11, 0x0B, 0x55, 0x66}; // raw word 2833
  CHECK(pkt3663_dummy_send(0x11, 0x22, 0x33, frame3663, 4, 0x30, 0x3838) ==
        pkt3663fix::kSendReturn);
  CHECK(pkt3663_dummy_send(0x44, 0x55, 0x66, frame3576, 4, 0x31, 0x3939) ==
        pkt3663fix::kSendReturn);
  CHECK(pkt3663_dummy_receive(0x77, 0x88, 0x99, frame3663, 4, 0, 0) ==
        pkt3663fix::kReceiveReturn);
  Pkt3663LiveReport active{};
  const ULONGLONG counter_deadline = GetTickCount64() + 3000;
  do {
    active = report_of(api);
    if (active.stored == 3)
      break;
    SwitchToThread();
  } while (GetTickCount64() < counter_deadline);
  CHECK(active.stored == 3);
  CHECK(active.omitted == 0);
  CHECK(api.stop() == kPkt3663LiveRemoved);

  const Pkt3663LiveReport report = report_of(api);
  CHECK(report.stop_code == kPkt3663LiveRemoved);
  CHECK(report.retained == 0);
  CHECK(report.stored == 3);
  CHECK(report.omitted == 0);
  CHECK(report.phase == 1);
  const std::string records =
      read_file(directory + L"\\pkt3663-live-records-stand.txt");
  CHECK(records.find("capture_mode=raw-all") != std::string::npos);
  CHECK(records.find("allowlist_count=0") != std::string::npos);
  CHECK(records.find("layout=unresolved") != std::string::npos);
  CHECK(records.find("receive_pre_transform_possible=1") != std::string::npos);
  CHECK(records.find("raw_word0=0AE6") != std::string::npos);
  CHECK(records.find("raw_word0=0B11") != std::string::npos);
  CHECK(records.find("classification=unclassified") != std::string::npos);

  const std::string session =
      read_file(directory + L"\\pkt3663-live-session.txt");
  CHECK(session.find("fixture=1") != std::string::npos);
  CHECK(session.find("game_match=0") != std::string::npos);
  CHECK(session.find("gate=fixture") != std::string::npos);
  CHECK(session.find("startup=started") != std::string::npos);
  CHECK(session.find("shutdown=removed") != std::string::npos);
  CHECK(session.find("capture_mode=raw-all") != std::string::npos);
  CHECK(session.find("layout=unresolved") != std::string::npos);
  CHECK(session.find("receive_transform=possible") != std::string::npos);
  CHECK(session.find("adopted=0") != std::string::npos);
  CHECK(session.find("recording_ready=1") != std::string::npos);
  CHECK(session.find("stage=export-entry qpc=") != std::string::npos);
  CHECK(session.find("stage=identity qpc=") != std::string::npos);
  CHECK(session.find("stage=receive-hook qpc=") != std::string::npos);
  CHECK(session.find("stage=send-hook qpc=") != std::string::npos);
  CHECK(session.find("stage=writer-start qpc=") != std::string::npos);
  CHECK(session.find("stage=ready-durable qpc=") != std::string::npos);
  CHECK(session.find("capture_ready_qpc=0") == std::string::npos);
  CHECK(session.find("evidence_cutoff=records-qpc-at-or-after-ready") !=
        std::string::npos);
  const std::string sealed_stand =
      read_file(directory + L"\\pkt3663-live-session-1.txt");
  CHECK(sealed_stand == session);
  CHECK(sealed_stand.find("stage=ready-durable qpc=") != std::string::npos);

  // The phase file is exclusive and must not be overwritten.
  CHECK(api.fixture(directory.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    4) == kPkt3663LiveBadLog);
  const Pkt3663LiveReport failed_attempt = report_of(api);
  CHECK(failed_attempt.capture_attempts == 1);
  CHECK(failed_attempt.stored == 0);
  CHECK(failed_attempt.omitted == 0);
  CHECK(failed_attempt.unread == 0);
  CHECK(failed_attempt.truncated == 0);

  // Repeated idle marks exercise the bounded session-history reset. The
  // reset is explicit in the retained file instead of silently dropping
  // earlier blocks.
  for (int index = 0; index < 300; ++index)
    CHECK(api.mark("walk") == 0);
  CHECK(read_file(directory + L"\\pkt3663-live-session.txt")
            .find("session_history_truncated=1") != std::string::npos);
  CHECK(read_file(directory + L"\\pkt3663-live-session-pending.txt")
            .find("session_history_truncated=1") != std::string::npos);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_stand);
  CHECK(api.fixture(directory.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    2) == kPkt3663LiveStarted);
  CHECK(api.stop() == kPkt3663LiveRemoved);
  const Pkt3663LiveReport second_attempt = report_of(api);
  CHECK(second_attempt.capture_attempts == 2);
  CHECK(second_attempt.stored == 0);
  CHECK(second_attempt.omitted == 0);
  CHECK(read_file(directory + L"\\pkt3663-live-records-walk.txt")
            .find("capture_mode=raw-all") != std::string::npos);
  const std::string sealed_walk =
      read_file(directory + L"\\pkt3663-live-session-3.txt");
  CHECK(sealed_walk.find("recording_ready=1") != std::string::npos);
  CHECK(sealed_walk.find("stage=ready-durable qpc=") != std::string::npos);
  CHECK(sealed_walk.find("phase=walk") != std::string::npos);
  CHECK(sealed_walk.find("session_history_truncated=0") != std::string::npos);
  CHECK(sealed_walk.find("session_history_truncated=1") == std::string::npos);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_stand);

  CHECK(api.mark("unlabeled") == 0);
  CHECK(api.fixture(directory.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    2) == kPkt3663LiveStarted);
  CHECK(api.stop() == kPkt3663LiveRemoved);
  const std::string sealed_third =
      read_file(directory + L"\\pkt3663-live-session-4.txt");
  CHECK(sealed_third.find("recording_ready=1") != std::string::npos);
  CHECK(sealed_third.find("stage=ready-durable qpc=") != std::string::npos);
  CHECK(sealed_third.find("phase=unlabeled") != std::string::npos);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_stand);
  CHECK(read_file(directory + L"\\pkt3663-live-session-3.txt") == sealed_walk);

  // The recording-ready block is part of the evidence boundary. A failure to
  // persist it removes the just-installed hooks, returns BadLog, and leaves a
  // durable invalid-session marker on the retry flush.
  CHECK(api.mark("turn") == 0);
  CHECK(api.fail_flush_after != nullptr);
  if (api.fail_flush_after != nullptr)
    api.fail_flush_after(1);
  CHECK(api.fixture(directory.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    2) == kPkt3663LiveBadLog);
  const Pkt3663LiveReport failed_ready = report_of(api);
  CHECK(failed_ready.start_code == kPkt3663LiveBadLog);
  CHECK(failed_ready.stop_code == kPkt3663LiveRemoved);
  CHECK(failed_ready.retained == 0);
  CHECK(api.stop() == kPkt3663LiveIdle);
  const std::string failed_session =
      read_file(directory + L"\\pkt3663-live-session.txt");
  CHECK(failed_session.find("session_write_failed=1") != std::string::npos);
  CHECK(failed_session.find("startup=bad-log") != std::string::npos);
  CHECK(failed_session.find("shutdown=removed") != std::string::npos);
  CHECK(failed_session.find("recording_ready=0") != std::string::npos);
  CHECK(read_file(directory + L"\\pkt3663-live-session-1.txt") == sealed_stand);
  CHECK(read_file(directory + L"\\pkt3663-live-session-3.txt") == sealed_walk);
  CHECK(read_file(directory + L"\\pkt3663-live-session-4.txt") == sealed_third);

  // Idle phase marking reports its own durable write failure and restores the
  // previous label instead of silently selecting an unlogged phase.
  if (api.fail_flush_after != nullptr)
    api.fail_flush_after(0);
  CHECK(api.mark("unlabeled") == kPkt3663LiveBadLog);
  CHECK(report_of(api).phase == 3);

  // A different directory cannot retarget a module after its first bind.
  const std::wstring different = temp_directory(L"different");
  CHECK(api.fixture(different.c_str(),
                    reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8,
                    2) == kPkt3663LiveBadLog);

  using ShortWriteFn = void (*)(const char *);
  const auto arm_short_write = reinterpret_cast<ShortWriteFn>(GetProcAddress(
      api.module, "pkt3663_live_fixture_fail_session_short_write_containing"));
  CHECK(arm_short_write != nullptr);
  if (arm_short_write != nullptr) {
    CHECK(api.mark("unlabeled") == 0);
    arm_short_write("stage=ready-durable");
    const int ready_short =
        api.fixture(directory.c_str(), reinterpret_cast<void *>(&pkt3663_dummy_send),
                    reinterpret_cast<void *>(&pkt3663_dummy_receive), 8, 2);
    CHECK(ready_short == kPkt3663LiveBadLog);
    const Pkt3663LiveReport ready_report = report_of(api);
    CHECK(ready_report.start_code == kPkt3663LiveBadLog);
    CHECK(ready_report.stop_code == kPkt3663LiveRemoved);
    CHECK(ready_report.retained == 0);
    CHECK(api.stop() == kPkt3663LiveIdle);
    const std::string ready_session = read_file(directory + L"\\pkt3663-live-session.txt");
    CHECK(!ready_session.empty());
    CHECK(ready_session.find("session_write_failed=1") != std::string::npos);
    CHECK(ready_session.find("startup=bad-log") != std::string::npos);
    CHECK(ready_session.find("shutdown=removed") != std::string::npos);
    std::printf("observed ready-short-write start=%d session_bytes=%zu\n", ready_short,
                ready_session.size());
  }

  // The first successful arm pins the diagnostic DLL until process exit.
  CHECK(FreeLibrary(api.module) != FALSE);
  CHECK(GetModuleHandleW(L"cdmp_pkt3663_live_fixture.dll") != nullptr);
}

} // namespace

int main() {
  test_production_refuses_dummy_host();
  test_fixture_raw_capture();
  if (g_failures != 0) {
    std::printf("pkt3663 live host tests: %d failure(s)\n", g_failures);
    return 1;
  }
  std::printf("pkt3663 live host tests ok\n");
  return 0;
}
