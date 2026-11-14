// Explicit start/stop runner for the isolated 3663 raw capture module.
// Hook installation and teardown never run in DllMain. Production derives
// both entries internally only after strict mapped-image verification.

#include "pkt3663/live_runner.h"

#include "pkt3663/candidates.h"
#include "pkt3663/capture.h"
#include "pkt3663/live_api.h"
#include "pkt3663/live_identity.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <windows.h>

namespace pkt3663::live {
namespace {

constexpr uint32_t kStateIdle = 0;
constexpr uint32_t kStateBusy = 1;
constexpr uint32_t kStateActive = 2;
constexpr uint32_t kStateHeld = 3;
constexpr uint32_t kProductionQueue = 256;
constexpr uint32_t kProductionRecords = 256;
constexpr uint32_t kMaxQueue = 256;
constexpr uint32_t kMaxRecords = 256;
constexpr size_t kPathCap = 1024;
constexpr size_t kCapturePathCap = 260;
constexpr size_t kLogCap = 8192;
constexpr size_t kLogBlockReserve = 3072;

constexpr wchar_t kSessionFile[] = L"pkt3663-live-session.txt";
constexpr wchar_t kPendingFile[] = L"pkt3663-live-session-pending.txt";
constexpr wchar_t kRecordsFile[] = L"pkt3663-live-records.txt";

SRWLOCK g_control = SRWLOCK_INIT;

struct ControlLock {
  ControlLock() { AcquireSRWLockExclusive(&g_control); }
  ~ControlLock() { ReleaseSRWLockExclusive(&g_control); }
  ControlLock(const ControlLock &) = delete;
  ControlLock &operator=(const ControlLock &) = delete;
};

std::atomic<uint32_t> g_state{kStateIdle};
std::atomic<int> g_start_code{-1};
std::atomic<int> g_stop_code{-1};
std::atomic<int> g_retained{0};
std::atomic<unsigned> g_stored{0};
std::atomic<unsigned> g_omitted{0};
std::atomic<unsigned> g_attempts{0};
std::atomic<uint8_t> g_label{static_cast<uint8_t>(Phase::Unlabeled)};

Capture *g_capture = nullptr;
Identity g_identity{};
uint64_t g_dropped = 0;
uint64_t g_unread = 0;
uint64_t g_truncated = 0;
uint64_t g_pre_ready = 0;
uint64_t g_qpc_frequency = 0;
uint64_t g_ready_qpc = 0;
uint32_t g_queue = 0;
uint32_t g_records = 0;
int g_fixture = 0;
int g_game_match = 0;

char g_log[kLogCap] = {};
size_t g_log_used = 0;
bool g_log_truncated = false;
bool g_log_bound = false;
bool g_log_write_failed = false;
#ifdef PKT3663_LIVE_FIXTURE
int g_flushes_before_failure = -1;
bool g_fail_ready_durable_flush = false;
bool g_force_short_write = false;
#endif
wchar_t g_log_dir[kPathCap] = {};
wchar_t g_session_path[kCapturePathCap] = {};
wchar_t g_attempt_path[kCapturePathCap] = {};
wchar_t g_pending_path[kCapturePathCap] = {};
wchar_t g_records_path[kCapturePathCap] = {};
uint32_t g_attempt_seq = 0;
bool g_attempt_open = false;
char g_last_mark[160] = {};

struct StateSlot {
  bool active = false;
  uint32_t next = kStateIdle;
  ~StateSlot() {
    if (active)
      g_state.store(next, std::memory_order_release);
  }
  void finish(uint32_t value) { next = value; }
};

bool enter_start(int &denied) {
  uint32_t expected = kStateIdle;
  if (g_state.compare_exchange_strong(expected, kStateBusy,
                                      std::memory_order_acq_rel,
                                      std::memory_order_acquire)) {
    return true;
  }
  denied = expected == kStateHeld ? kPkt3663LiveRetained : kPkt3663LiveRefused;
  return false;
}

__declspec(noinline) void pin_anchor() {}

bool pin_module() {
  HMODULE module = nullptr;
  void *address = reinterpret_cast<void *>(&pin_anchor);
  LPCWSTR key = nullptr;
  static_assert(sizeof address == sizeof key);
  std::memcpy(&key, &address, sizeof key);
  return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_PIN,
                            key, &module) != FALSE;
}

const wchar_t *file_name_of(const wchar_t *path) {
  if (path == nullptr)
    return L"";
  const wchar_t *name = path;
  for (const wchar_t *cursor = path; *cursor != L'\0'; ++cursor) {
    if (*cursor == L'\\' || *cursor == L'/')
      name = cursor + 1;
  }
  return name;
}

bool host_path(wchar_t *output, size_t count) {
  const DWORD length =
      GetModuleFileNameW(nullptr, output, static_cast<DWORD>(count));
  return length != 0 && length < count;
}

bool join_file(wchar_t *output, size_t count, const wchar_t *directory,
               const wchar_t *file) {
  const size_t directory_length = wcslen(directory);
  const size_t file_length = wcslen(file);
  const bool slash = directory_length != 0 &&
                     directory[directory_length - 1] != L'\\' &&
                     directory[directory_length - 1] != L'/';
  const size_t total = directory_length + (slash ? 1 : 0) + file_length;
  if (total >= kCapturePathCap || total + 1 > count)
    return false;
  std::memcpy(output, directory, directory_length * sizeof(wchar_t));
  size_t used = directory_length;
  if (slash)
    output[used++] = L'\\';
  std::memcpy(output + used, file, (file_length + 1) * sizeof(wchar_t));
  return true;
}

bool directory_ready(const wchar_t *path) {
  const DWORD existing = GetFileAttributesW(path);
  if (existing != INVALID_FILE_ATTRIBUTES)
    return (existing & FILE_ATTRIBUTE_DIRECTORY) != 0;
  if (CreateDirectoryW(path, nullptr))
    return true;
  if (GetLastError() != ERROR_ALREADY_EXISTS)
    return false;
  const DWORD created = GetFileAttributesW(path);
  return created != INVALID_FILE_ATTRIBUTES &&
         (created & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void reset_truncated_log() {
  constexpr char marker[] = "pkt3663-live\nsession_history_truncated=1\n";
  static_assert(sizeof marker < kLogCap);
  std::memcpy(g_log, marker, sizeof marker);
  g_log_used = sizeof marker - 1;
  g_log_truncated = true;
}

void reserve_log(size_t bytes) {
  if (bytes + 1 >= kLogCap || g_log_used + bytes + 1 >= kLogCap)
    reset_truncated_log();
}

void log_append(const char *text) {
  if (text == nullptr)
    return;
  const size_t length = std::strlen(text);
  if (g_log_used + length + 1 >= kLogCap) {
    reset_truncated_log();
    if (g_log_used + length + 1 >= kLogCap)
      return;
  }
  std::memcpy(g_log + g_log_used, text, length);
  g_log_used += length;
  g_log[g_log_used] = '\0';
}

bool write_log_file(const wchar_t *path, bool fault_here) {
  if (path == nullptr || path[0] == L'\0') {
    g_log_write_failed = true;
    return false;
  }
  FILE *file = nullptr;
  if (_wfopen_s(&file, path, L"wb") != 0 || file == nullptr) {
    g_log_write_failed = true;
    return false;
  }
#ifdef PKT3663_LIVE_FIXTURE
  // The file is already truncated by "wb". This one-shot fault is only armed
  // for the ready-durable timing rewrite.
  if (fault_here && g_force_short_write) {
    g_force_short_write = false;
    std::fclose(file);
    g_log_write_failed = true;
    return false;
  }
#else
  (void)fault_here;
#endif
  const size_t written =
      g_log_used == 0 ? 0 : std::fwrite(g_log, 1, g_log_used, file);
  const int closed = std::fclose(file);
  if (closed != 0 || written != g_log_used) {
    g_log_write_failed = true;
    return false;
  }
  return true;
}

void seal_attempt() noexcept { g_attempt_open = false; }

// Seals the attempt opened by this start unless the capture stays active.
// Destruction covers early returns and exceptions; Started sets keep.
struct AttemptSeal {
  bool keep = false;
  AttemptSeal() = default;
  ~AttemptSeal() {
    if (!keep)
      seal_attempt();
  }
  AttemptSeal(const AttemptSeal &) = delete;
  AttemptSeal &operator=(const AttemptSeal &) = delete;
};

bool log_flush_file() {
  if (!g_log_bound || g_session_path[0] == L'\0') {
    g_log_write_failed = true;
    return false;
  }
  const wchar_t *primary = g_attempt_open ? g_attempt_path : g_pending_path;
  // The fault applies to the attempt or pending file. The legacy mirror is a
  // copy of that same bounded buffer and is not a second evidence source.
  if (!write_log_file(primary, true))
    return false;
  if (wcscmp(primary, g_session_path) == 0)
    return true;
  return write_log_file(g_session_path, false);
}

bool log_flush() {
#ifdef PKT3663_LIVE_FIXTURE
  if (g_flushes_before_failure == 0) {
    g_flushes_before_failure = -1;
    g_log_write_failed = true;
    return false;
  }
  if (g_flushes_before_failure > 0)
    --g_flushes_before_failure;
#endif
  return log_flush_file();
}

uint64_t g_stage_origin = 0;
bool g_stage_write_failed = false;

uint64_t qpc_now() noexcept {
  LARGE_INTEGER now{};
  if (!QueryPerformanceCounter(&now) || now.QuadPart <= 0)
    return 0;
  return static_cast<uint64_t>(now.QuadPart);
}

void ensure_qpc_frequency() noexcept {
  if (g_qpc_frequency != 0)
    return;
  LARGE_INTEGER frequency{};
  if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0)
    g_qpc_frequency = static_cast<uint64_t>(frequency.QuadPart);
}

void begin_stages(uint64_t origin) noexcept {
  ensure_qpc_frequency();
  g_stage_origin = origin;
  g_stage_write_failed = false;
}

// elapsed_us is measured from begin_stages, not from the previous stage.
// A failed timing rewrite is sticky. Callers must not report Started after it.
bool note_stage(const char *name, uint64_t qpc) noexcept {
  try {
    if (name == nullptr || !g_log_bound)
      return false;
    uint64_t elapsed_us = 0;
    if (g_qpc_frequency != 0 && qpc >= g_stage_origin) {
      const uint64_t delta = qpc - g_stage_origin;
      elapsed_us = delta > (UINT64_MAX / 1000000ull)
                       ? UINT64_MAX
                       : (delta * 1000000ull) / g_qpc_frequency;
    }
    char line[160] = {};
    const int wrote = std::snprintf(
        line, sizeof line, "stage=%s qpc=%llu elapsed_us=%llu\n", name,
        static_cast<unsigned long long>(qpc),
        static_cast<unsigned long long>(elapsed_us));
    if (wrote <= 0) {
      g_stage_write_failed = true;
      return false;
    }
    log_append(line);
#ifdef PKT3663_LIVE_FIXTURE
    if (std::strcmp(name, "ready-durable") == 0 && g_fail_ready_durable_flush) {
      g_fail_ready_durable_flush = false;
      g_force_short_write = true;
    }
#endif
    if (!log_flush_file()) {
      g_stage_write_failed = true;
      return false;
    }
    return true;
  } catch (...) {
    g_stage_write_failed = true;
    return false;
  }
}

void capture_stage(uint8_t stage) noexcept {
  const char *name = nullptr;
  if (stage == 1)
    name = "receive-hook";
  else if (stage == 2)
    name = "send-hook";
  else if (stage == 3)
    name = "writer-start";
  else
    return;
  note_stage(name, qpc_now());
}

bool exclusive_create(const wchar_t *path) {
  HANDLE created = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
  if (created == INVALID_HANDLE_VALUE)
    return false;
  CloseHandle(created);
  return true;
}

// Each start owns one exclusive file. Later starts and idle marks must not
// reopen it. The in-memory buffer stays capped at kLogCap.
bool open_attempt_log() {
  if (!g_log_bound || g_attempt_open || g_log_dir[0] == L'\0')
    return false;
  const uint32_t seq = g_attempt_seq + 1;
  wchar_t name[40] = {};
  if (_snwprintf_s(name, _TRUNCATE, L"pkt3663-live-session-%u.txt", seq) < 0)
    return false;
  wchar_t path[kCapturePathCap] = {};
  if (!join_file(path, kCapturePathCap, g_log_dir, name))
    return false;
  if (!exclusive_create(path))
    return false;
  g_attempt_seq = seq;
  wcsncpy_s(g_attempt_path, path, _TRUNCATE);
  g_attempt_open = true;
  g_log_used = 0;
  g_log[0] = '\0';
  g_log_truncated = false;
  log_append("pkt3663-live\n");
  if (g_last_mark[0] != '\0') {
    log_append(g_last_mark);
    g_last_mark[0] = '\0';
  }
  // This is the attempt's first counted flush. A test can then fail the later
  // ready block without losing the exclusive file that holds the failure.
  if (!log_flush()) {
    seal_attempt();
    return false;
  }
  return true;
}

int bind_log(const wchar_t *directory, const wchar_t *host) {
  if (!log_directory_allowed(host, directory))
    return kPkt3663LiveBadLog;
  wchar_t full[kPathCap] = {};
  const DWORD length =
      GetFullPathNameW(directory, static_cast<DWORD>(kPathCap), full, nullptr);
  if (length == 0 || length >= kPathCap)
    return kPkt3663LiveBadLog;
  wchar_t session[kCapturePathCap] = {};
  wchar_t pending[kCapturePathCap] = {};
  if (!join_file(session, kCapturePathCap, full, kSessionFile) ||
      !join_file(pending, kCapturePathCap, full, kPendingFile))
    return kPkt3663LiveBadLog;
  if (g_log_bound && _wcsicmp(g_log_dir, full) != 0)
    return kPkt3663LiveBadLog;
  if (!directory_ready(full))
    return kPkt3663LiveBadLog;
  if (!g_log_bound) {
    wcsncpy_s(g_log_dir, full, _TRUNCATE);
    wcsncpy_s(g_session_path, session, _TRUNCATE);
    wcsncpy_s(g_pending_path, pending, _TRUNCATE);
    g_log_bound = true;
  }
  return kPkt3663LiveStarted;
}

const char *phase_name(uint8_t phase) {
  switch (phase) {
  case static_cast<uint8_t>(Phase::Stand):
    return "stand";
  case static_cast<uint8_t>(Phase::Walk):
    return "walk";
  case static_cast<uint8_t>(Phase::Turn):
    return "turn";
  default:
    return "unlabeled";
  }
}

const wchar_t *records_name(uint8_t phase) {
  switch (phase) {
  case static_cast<uint8_t>(Phase::Stand):
    return L"pkt3663-live-records-stand.txt";
  case static_cast<uint8_t>(Phase::Walk):
    return L"pkt3663-live-records-walk.txt";
  case static_cast<uint8_t>(Phase::Turn):
    return L"pkt3663-live-records-turn.txt";
  default:
    return kRecordsFile;
  }
}

bool parse_phase(const char *text, uint8_t &phase) {
  if (text == nullptr)
    return false;
  if (std::strcmp(text, "unlabeled") == 0)
    phase = static_cast<uint8_t>(Phase::Unlabeled);
  else if (std::strcmp(text, "stand") == 0)
    phase = static_cast<uint8_t>(Phase::Stand);
  else if (std::strcmp(text, "walk") == 0)
    phase = static_cast<uint8_t>(Phase::Walk);
  else if (std::strcmp(text, "turn") == 0)
    phase = static_cast<uint8_t>(Phase::Turn);
  else
    return false;
  return true;
}

bool file_exists(const wchar_t *path) {
  return path != nullptr && path[0] != L'\0' &&
         GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

void remember_counters() {
  if (g_capture == nullptr)
    return;
  g_stored.store(g_capture->stored(), std::memory_order_relaxed);
  g_omitted.store(g_capture->omitted(), std::memory_order_relaxed);
  g_pre_ready = g_capture->pre_ready();
  g_dropped = g_capture->dropped();
  g_unread = g_capture->unread();
  g_truncated = g_capture->truncated();
}

void reset_attempt_observations(bool fixture) {
  g_start_code.store(-1, std::memory_order_relaxed);
  g_stop_code.store(-1, std::memory_order_relaxed);
  g_retained.store(0, std::memory_order_release);
  g_stored.store(0, std::memory_order_relaxed);
  g_omitted.store(0, std::memory_order_relaxed);
  g_dropped = 0;
  g_unread = 0;
  g_truncated = 0;
  g_pre_ready = 0;
  g_ready_qpc = 0;
  g_queue = 0;
  g_records = 0;
  g_fixture = fixture ? 1 : 0;
  g_game_match = 0;
  g_identity = {};
  g_records_path[0] = L'\0';
}

bool release_capture() {
  if (g_capture == nullptr)
    return true;
  try {
    remember_counters();
    if (!Capture::release_owner(g_capture)) {
      g_retained.store(1, std::memory_order_release);
      return false;
    }
  } catch (...) {
    g_retained.store(1, std::memory_order_release);
    return false;
  }
  return true;
}

void append_identity(const Identity &identity) {
  char line[512] = {};
  std::snprintf(
      line, sizeof line,
      "base=%016llX\ntimestamp=%08X\nsize=%08X\nkind=%s\nrejected_2976=%"
      "d\nrejected_3576=%d\n"
      "sections_ok=%d\nfunction_table_ok=%d\nranges_ok=%d\nanchors_ok=%"
      "d\nanchors_unique=%d\n"
      "role_calls_ok=%d\nfailed_anchor_rva=%08X\n",
      static_cast<unsigned long long>(identity.mapped_base), identity.timestamp,
      identity.size_of_image, identity_kind_name(identity.kind),
      identity.rejected_2976 ? 1 : 0, identity.rejected_3576 ? 1 : 0,
      identity.sections_ok ? 1 : 0, identity.function_table_ok ? 1 : 0,
      identity.pump_range_ok && identity.send_range_ok &&
              identity.receive_range_ok
          ? 1
          : 0,
      identity.pump_anchors_ok && identity.send_anchors_ok &&
              identity.receive_anchors_ok
          ? 1
          : 0,
      identity.anchors_unique ? 1 : 0, identity.role_calls_ok ? 1 : 0,
      identity.failed_anchor_rva);
  log_append(line);
}

bool write_block(const wchar_t *exe_name, const char *startup,
                 const char *shutdown) {
  reserve_log(kLogBlockReserve);
  char name[128] = {};
  WideCharToMultiByte(CP_UTF8, 0, exe_name == nullptr ? L"" : exe_name, -1,
                      name, static_cast<int>(sizeof name), nullptr, nullptr);
  for (char *cursor = name; *cursor != '\0'; ++cursor) {
    if (static_cast<unsigned char>(*cursor) < 0x20 || *cursor == '"')
      *cursor = '?';
  }
  if (name[0] == '\0')
    std::memcpy(name, "unknown", 8);
  char line[1024] = {};
  std::snprintf(line, sizeof line,
                "---\nmode=%s\nfixture=%d\ngame_match=%d\nname=%s\n",
                g_fixture != 0 ? "fixture" : "process", g_fixture != 0 ? 1 : 0,
                g_fixture == 0 && g_game_match != 0 &&
                        g_identity.kind == IdentityKind::Match
                    ? 1
                    : 0,
                name);
  log_append(line);
  append_identity(g_identity);
  if (g_qpc_frequency == 0) {
    LARGE_INTEGER frequency{};
    if (QueryPerformanceFrequency(&frequency))
      g_qpc_frequency = static_cast<uint64_t>(frequency.QuadPart);
  }
  std::snprintf(line, sizeof line,
                "gate=%s\nstartup=%s\nshutdown=%s\nstored=%u\nomitted=%"
                "u\npre_ready=%llu\ndropped=%llu\nunread=%llu\n"
                "truncated=%llu\ncapture_attempts=%u\nlimits=%u/"
                "%u\nphase=%s\nqpc_frequency=%llu\n"
                "qpc_timebase=QueryPerformanceCounter\ncapture_mode=raw-"
                "all\nraw=1\nallowlist_count=0\n"
                "classification=unclassified\nlayout=unresolved\nreceive_"
                "transform=possible\nadopted=0\ncopy_cap=%u\n"
                "recording_ready=%d\ncapture_ready_qpc=%llu\n"
                "evidence_cutoff=records-qpc-at-or-after-ready\nsession_"
                "history_truncated=%d\nsession_write_failed=%d\n",
                g_fixture != 0 ? "fixture" : "identity", startup, shutdown,
                g_stored.load(std::memory_order_relaxed),
                g_omitted.load(std::memory_order_relaxed),
                static_cast<unsigned long long>(g_pre_ready),
                static_cast<unsigned long long>(g_dropped),
                static_cast<unsigned long long>(g_unread),
                static_cast<unsigned long long>(g_truncated),
                g_attempts.load(std::memory_order_relaxed), g_queue, g_records,
                phase_name(g_label.load(std::memory_order_relaxed)),
                static_cast<unsigned long long>(g_qpc_frequency), kCopyCap,
                g_ready_qpc != 0 ? 1 : 0,
                static_cast<unsigned long long>(g_ready_qpc),
                g_log_truncated ? 1 : 0, g_log_write_failed ? 1 : 0);
  log_append(line);
  const bool durable = log_flush();
  if (!durable) {
    // Preserve an explicit invalid-session marker when a transient failure
    // clears before the retry. The caller still receives failure.
    log_append("session_write_failed=1\n");
    log_flush();
  }
  return durable;
}

void remember_session(bool fixture, bool match, const Identity &identity) {
  g_fixture = fixture ? 1 : 0;
  g_game_match = fixture ? 0 : (match ? 1 : 0);
  g_identity = identity;
}

struct ReverifyContext {
  uintptr_t base = 0;
  uint32_t timestamp = 0;
  uint32_t size = 0;
};

bool evidence_matches(const InstallEvidence *evidence, const uint8_t *expected,
                      size_t expected_size) noexcept {
  if (evidence == nullptr || evidence->target_patch == nullptr ||
      evidence->original_bytes == nullptr || evidence->target_patch_size == 0 ||
      evidence->target_patch_size != evidence->original_size ||
      evidence->original_size > expected_size) {
    return false;
  }
  if (std::memcmp(evidence->original_bytes, expected,
                  evidence->original_size) != 0)
    return false;
  const size_t size = evidence->target_patch_size;
  const uint8_t *patch = evidence->target_patch;
  if (patch[0] == 0xE9) {
    if (size < 5)
      return false;
    for (size_t index = 5; index < size; ++index) {
      if (patch[index] != 0x90)
        return false;
    }
    return true;
  }
  if (size < 14 || patch[0] != 0xFF || patch[1] != 0x25)
    return false;
  int32_t displacement = 0;
  std::memcpy(&displacement, patch + 2, sizeof displacement);
  if (displacement != 0)
    return false;
  for (size_t index = 14; index < size; ++index) {
    if (patch[index] != 0x90)
      return false;
  }
  return true;
}

bool reverify_before_hook(InstallSite site, const InstallEvidence *evidence,
                          void *user) noexcept {
  if (user == nullptr)
    return false;
  const auto *expected = static_cast<const ReverifyContext *>(user);
  try {
    Identity identity;
    if (site == InstallSite::Receive) {
      if (evidence != nullptr)
        return false;
      identity = identify_process();
      note_stage("receive-admission", qpc_now());
    } else if (site == InstallSite::Send) {
      if (!evidence_matches(evidence, kReceiveEntry, sizeof kReceiveEntry))
        return false;
      wchar_t path[kPathCap] = {};
      if (!host_path(path, kPathCap))
        return false;
      auto reader = [](uintptr_t address, void *destination, size_t size,
                       void *) {
        return read_mapped_bytes(address, destination, size);
      };
      identity = identify_image_after_receive_patch(
          file_name_of(path), expected->base, reader, nullptr,
          evidence->target_patch, evidence->target_patch_size);
      note_stage("send-admission", qpc_now());
    } else if (site == InstallSite::Complete) {
      // Receive was rechecked immediately before the Send factory. The
      // saved, instruction-aligned Send bytes close that final factory
      // window; Capture re-proves both exact live routes after return.
      const bool matched =
          evidence_matches(evidence, kSendEntry, sizeof kSendEntry) &&
          reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) ==
              expected->base;
      note_stage("complete-admission", qpc_now());
      return matched;
    } else {
      return false;
    }
    return identity_allows_hooks(identity) &&
           identity.mapped_base == expected->base &&
           identity.timestamp == expected->timestamp &&
           identity.size_of_image == expected->size;
  } catch (...) {
    return false;
  }
}

bool in_module(void *module, void *address) {
  if (module == nullptr || address == nullptr)
    return false;
  MEMORY_BASIC_INFORMATION info{};
  return VirtualQuery(address, &info, sizeof info) == sizeof info &&
         info.State == MEM_COMMIT && info.AllocationBase == module;
}

void note_retained(StateSlot &slot, const wchar_t *exe_name,
                   const char *startup) {
  g_retained.store(1, std::memory_order_release);
  g_stop_code.store(kPkt3663LiveRetained, std::memory_order_relaxed);
  slot.finish(kStateHeld);
  try {
    write_block(exe_name, startup, "retained");
  } catch (...) {
  }
  seal_attempt();
}

int arm(StateSlot &slot, void *module, void *send_entry, void *receive_entry,
        uint32_t queue, uint32_t records, const wchar_t *exe_name,
        InstallCheck check, void *check_user) {
  g_queue = queue;
  g_records = records;
  if (queue < kMinQueueCapacity || queue > kMaxQueue || records == 0 ||
      records > kMaxRecords || send_entry == receive_entry ||
      !in_module(module, send_entry) || !in_module(module, receive_entry)) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    write_block(exe_name, "refused", "idle");
    return kPkt3663LiveRefused;
  }

  const uint8_t phase = g_label.load(std::memory_order_relaxed);
  wchar_t path[kCapturePathCap] = {};
  if (!join_file(path, kCapturePathCap, g_log_dir, records_name(phase))) {
    g_start_code.store(kPkt3663LiveBadLog, std::memory_order_relaxed);
    return kPkt3663LiveBadLog;
  }
  if (phase != static_cast<uint8_t>(Phase::Unlabeled) && file_exists(path)) {
    g_start_code.store(kPkt3663LiveBadLog, std::memory_order_relaxed);
    write_block(exe_name, "bad-log", "idle");
    return kPkt3663LiveBadLog;
  }
  wcsncpy_s(g_records_path, path, _TRUNCATE);

  CaptureLimits limits;
  limits.queue_capacity = queue;
  limits.output_records = records;
  limits.exclusive_create = phase != static_cast<uint8_t>(Phase::Unlabeled);
  g_capture = Capture::create(limits, path);
  if (g_capture == nullptr) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    write_block(exe_name, "refused", "idle");
    return kPkt3663LiveRefused;
  }
  g_capture->set_phase(phase);
  g_capture->set_stage_mark(&capture_stage);

  if (!pin_module()) {
    if (!release_capture()) {
      g_start_code.store(kPkt3663LiveRetained, std::memory_order_relaxed);
      slot.finish(kStateHeld);
      write_block(exe_name, "retained", "retained");
      return kPkt3663LiveRetained;
    }
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    write_block(exe_name, "refused", "idle");
    return kPkt3663LiveRefused;
  }

  g_attempts.fetch_add(1, std::memory_order_acq_rel);
  Capture::StartResult result = Capture::StartResult::Refused;
  try {
    result =
        g_capture->start(module, send_entry, receive_entry, check, check_user);
  } catch (...) {
    if (!release_capture()) {
      g_start_code.store(kPkt3663LiveRetained, std::memory_order_relaxed);
      slot.finish(kStateHeld);
      write_block(exe_name, "retained", "retained");
      return kPkt3663LiveRetained;
    }
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    write_block(exe_name, "refused", "idle");
    return kPkt3663LiveRefused;
  }

  remember_counters();
  if (result == Capture::StartResult::Started) {
    LARGE_INTEGER ready{};
    if (QueryPerformanceCounter(&ready))
      g_ready_qpc = static_cast<uint64_t>(ready.QuadPart);
    g_start_code.store(kPkt3663LiveStarted, std::memory_order_relaxed);
    g_stop_code.store(-1, std::memory_order_relaxed);
    const bool ready_logged =
        write_block(exe_name, "started", "pending") && !g_stage_write_failed &&
        note_stage("ready-durable", qpc_now());
    // Publication is the admission store. It runs only after the ready block
    // and its timing line have reached this attempt's file.
    const bool published =
        ready_logged && g_ready_qpc != 0 && g_capture != nullptr &&
        g_capture->publish_ready(g_ready_qpc);
    if (!published) {
      // The QPC cutoff is evidence only after it reaches the session file.
      // Refuse the attempt and remove the newly installed hooks while their
      // ownership is still known instead of recording unanchored raw rows.
      g_ready_qpc = 0;
      g_start_code.store(kPkt3663LiveBadLog, std::memory_order_relaxed);
      Capture::StopResult cleanup = Capture::StopResult::Retained;
      try {
        cleanup = g_capture->stop();
        remember_counters();
      } catch (...) {
        note_retained(slot, exe_name, "bad-log");
        return kPkt3663LiveRetained;
      }
      if (cleanup == Capture::StopResult::Removed && release_capture()) {
        g_stop_code.store(kPkt3663LiveRemoved, std::memory_order_relaxed);
        slot.finish(kStateIdle);
        write_block(exe_name, "bad-log", "removed");
        return kPkt3663LiveBadLog;
      }
      note_retained(slot, exe_name, "bad-log");
      return kPkt3663LiveRetained;
    }
    slot.finish(kStateActive);
    return kPkt3663LiveStarted;
  }
  if (result == Capture::StartResult::Retained || !release_capture()) {
    g_retained.store(1, std::memory_order_release);
    g_start_code.store(kPkt3663LiveRetained, std::memory_order_relaxed);
    slot.finish(kStateHeld);
    write_block(exe_name, "retained", "retained");
    return kPkt3663LiveRetained;
  }
  const bool rolled = result == Capture::StartResult::RolledBack;
  const int code = rolled ? kPkt3663LiveRolledBack : kPkt3663LiveRefused;
  g_start_code.store(code, std::memory_order_relaxed);
  write_block(exe_name, rolled ? "rolled-back" : "refused",
              rolled ? "rolled-back" : "idle");
  return code;
}

} // namespace

int start(const wchar_t *log_directory) {
  ControlLock control;
  const uint64_t entered = qpc_now();
  begin_stages(entered);
  int denied = 0;
  if (!enter_start(denied))
    return denied;
  StateSlot slot;
  slot.active = true;
  reset_attempt_observations(false);

  wchar_t host[kPathCap] = {};
  if (!host_path(host, kPathCap)) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    return kPkt3663LiveRefused;
  }
  const int bound = bind_log(log_directory, host);
  if (bound != kPkt3663LiveStarted) {
    g_start_code.store(bound, std::memory_order_relaxed);
    return bound;
  }
  if (!open_attempt_log()) {
    g_start_code.store(kPkt3663LiveBadLog, std::memory_order_relaxed);
    return kPkt3663LiveBadLog;
  }
  AttemptSeal attempt;
  note_stage("export-entry", entered);

  const Identity identity = identify_process();
  note_stage("identity", qpc_now());
  remember_session(false, false, identity);
  if (!identity_allows_hooks(identity)) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    g_queue = kProductionQueue;
    g_records = kProductionRecords;
    write_block(file_name_of(host), "refused", "idle");
    return kPkt3663LiveRefused;
  }

  HMODULE module = GetModuleHandleW(nullptr);
  const uintptr_t base = reinterpret_cast<uintptr_t>(module);
  if (module == nullptr || base != identity.mapped_base ||
      base > UINTPTR_MAX - kSendRva || base > UINTPTR_MAX - kReceiveRva) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    write_block(file_name_of(host), "refused", "idle");
    return kPkt3663LiveRefused;
  }
  remember_session(false, true, identity);
  ReverifyContext verification{base, identity.timestamp,
                               identity.size_of_image};
  void *send =
      reinterpret_cast<void *>(base + static_cast<uintptr_t>(kSendRva));
  void *receive =
      reinterpret_cast<void *>(base + static_cast<uintptr_t>(kReceiveRva));
  const int code =
      arm(slot, module, send, receive, kProductionQueue, kProductionRecords,
          file_name_of(host), &reverify_before_hook, &verification);
  attempt.keep = code == kPkt3663LiveStarted;
  return code;
}

int stop() {
  ControlLock control;
  uint32_t current = g_state.load(std::memory_order_acquire);
  if (current == kStateHeld)
    return kPkt3663LiveRetained;
  if (current == kStateIdle)
    return kPkt3663LiveIdle;
  if (current != kStateActive ||
      !g_state.compare_exchange_strong(current, kStateBusy,
                                       std::memory_order_acq_rel,
                                       std::memory_order_acquire)) {
    return g_state.load(std::memory_order_acquire) == kStateHeld
               ? kPkt3663LiveRetained
               : kPkt3663LiveRefused;
  }

  StateSlot slot;
  slot.active = true;
  wchar_t host[kPathCap] = {};
  const wchar_t *exe_name =
      host_path(host, kPathCap) ? file_name_of(host) : L"";
  if (g_capture == nullptr) {
    g_stop_code.store(kPkt3663LiveIdle, std::memory_order_relaxed);
    seal_attempt();
    return kPkt3663LiveIdle;
  }

  slot.finish(kStateHeld);
  Capture::StopResult result = Capture::StopResult::Retained;
  try {
    result = g_capture->stop();
    remember_counters();
  } catch (...) {
    note_retained(slot, exe_name, "started");
    return kPkt3663LiveRetained;
  }
  if (result == Capture::StopResult::Removed) {
    if (!release_capture()) {
      note_retained(slot, exe_name, "started");
      return kPkt3663LiveRetained;
    }
    g_retained.store(0, std::memory_order_release);
    g_stop_code.store(kPkt3663LiveRemoved, std::memory_order_relaxed);
    slot.finish(kStateIdle);
    try {
      if (!write_block(exe_name, "started", "removed")) {
        seal_attempt();
        return kPkt3663LiveBadLog;
      }
    } catch (...) {
      seal_attempt();
      return kPkt3663LiveBadLog;
    }
    seal_attempt();
    return kPkt3663LiveRemoved;
  }
  if (result == Capture::StopResult::Retained || !release_capture()) {
    note_retained(slot, exe_name, "started");
    return kPkt3663LiveRetained;
  }
  g_stop_code.store(kPkt3663LiveIdle, std::memory_order_relaxed);
  slot.finish(kStateIdle);
  seal_attempt();
  return kPkt3663LiveIdle;
}

int mark(const char *phase_text) {
  uint8_t phase = 0;
  if (!parse_phase(phase_text, phase))
    return kPkt3663LiveBadLog;
  ControlLock control;
  if (g_state.load(std::memory_order_acquire) != kStateIdle)
    return kPkt3663LiveRefused;
  const uint8_t previous = g_label.load(std::memory_order_relaxed);
  g_label.store(phase, std::memory_order_release);
  if (g_log_bound) {
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    char line[128] = {};
    std::snprintf(line, sizeof line, "mark phase=%s qpc=%llu\n",
                  phase_name(phase),
                  static_cast<unsigned long long>(now.QuadPart));
    std::snprintf(g_last_mark, sizeof g_last_mark, "%s", line);
    reserve_log(sizeof line);
    log_append(line);
    if (!log_flush()) {
      g_last_mark[0] = '\0';
      g_label.store(previous, std::memory_order_release);
      log_append("session_write_failed=1\nmark_log_write_failed=1\n");
      log_flush();
      return kPkt3663LiveBadLog;
    }
  }
  return 0;
}

void write_report(Pkt3663LiveReport *report) {
  if (report == nullptr)
    return;
  ControlLock control;
  if (g_capture != nullptr)
    remember_counters();
  report->start_code = g_start_code.load(std::memory_order_acquire);
  report->stop_code = g_stop_code.load(std::memory_order_acquire);
  report->retained = g_retained.load(std::memory_order_acquire);
  report->stored = g_stored.load(std::memory_order_acquire);
  report->omitted = g_omitted.load(std::memory_order_acquire);
  report->capture_attempts = g_attempts.load(std::memory_order_acquire);
  report->unread =
      g_unread > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(g_unread);
  report->truncated = g_truncated > UINT32_MAX
                          ? UINT32_MAX
                          : static_cast<uint32_t>(g_truncated);
  report->phase = g_label.load(std::memory_order_relaxed);
  report->raw_mode = 1;
  report->allowlist_count = kAllowlistCount;
  report->adopted = 0;
  report->qpc_frequency = g_qpc_frequency;
}

#ifdef PKT3663_LIVE_FIXTURE
int fixture_start(const wchar_t *log_directory, void *send_entry,
                  void *recv_entry, unsigned queue_capacity,
                  unsigned output_records) {
  ControlLock control;
  const uint64_t entered = qpc_now();
  begin_stages(entered);
  int denied = 0;
  if (!enter_start(denied))
    return denied;
  StateSlot slot;
  slot.active = true;
  reset_attempt_observations(true);

  wchar_t host[kPathCap] = {};
  if (!host_path(host, kPathCap)) {
    g_start_code.store(kPkt3663LiveRefused, std::memory_order_relaxed);
    return kPkt3663LiveRefused;
  }
  const int bound = bind_log(log_directory, host);
  if (bound != kPkt3663LiveStarted) {
    g_start_code.store(bound, std::memory_order_relaxed);
    return bound;
  }
  if (!open_attempt_log()) {
    g_start_code.store(kPkt3663LiveBadLog, std::memory_order_relaxed);
    return kPkt3663LiveBadLog;
  }
  AttemptSeal attempt;
  note_stage("export-entry", entered);
  const Identity observed = identify_process();
  note_stage("identity", qpc_now());
  remember_session(true, false, observed);
  const int code = arm(slot, GetModuleHandleW(nullptr), send_entry, recv_entry,
                       queue_capacity, output_records, file_name_of(host),
                       nullptr, nullptr);
  attempt.keep = code == kPkt3663LiveStarted;
  return code;
}

void set_records_open_hook(void (*hook)(void *), void *user) {
  Capture::set_records_open_hook(hook, user);
}
void set_phase_hold(void (*hold)(void *), void *user) {
  Capture::set_phase_hold(hold, user);
}
void fail_log_flush_after(unsigned successful_flushes) {
  ControlLock control;
  g_flushes_before_failure =
      successful_flushes > 1000000u ? -1 : static_cast<int>(successful_flushes);
}

void fail_session_short_write_containing(const char *needle) {
  ControlLock control;
  g_fail_ready_durable_flush =
      needle != nullptr && std::strcmp(needle, "stage=ready-durable") == 0;
}
#endif

} // namespace pkt3663::live
