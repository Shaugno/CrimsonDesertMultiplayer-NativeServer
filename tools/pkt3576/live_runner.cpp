// Explicit start/stop for the experimental 3576 capture.
// Hook installation, the session log, and teardown run here, never in DllMain.
// Capture::start receives pointers in this process only.

#include "pkt3576/live_runner.h"

#include "pkt3576/capture.h"
#include "pkt3576/live_api.h"
#include "pkt3576/live_identity.h"

#include <atomic>
#include <cstdio>
#include <cstring>

#include <windows.h>

namespace pkt3576::live {
namespace {

constexpr uint32_t kPhaseIdle = 0;
constexpr uint32_t kPhaseBusy = 1;
constexpr uint32_t kPhaseActive = 2;
constexpr uint32_t kPhaseHeld = 3;

constexpr uint32_t kProductionQueue = 64;
constexpr uint32_t kProductionRecords = 64;
constexpr uint32_t kMinQueue = 2;
constexpr uint32_t kMaxQueue = 256;
constexpr uint32_t kMinRecords = 1;
constexpr uint32_t kMaxRecords = 256;
constexpr size_t kLogCap = 4096;
constexpr size_t kPathCap = 1024;
constexpr size_t kCapturePathCap = 260;

const wchar_t kSessionFile[] = L"pkt3576-live-session.txt";
const wchar_t kRecordsFile[] = L"pkt3576-live-records.txt";

std::atomic<uint32_t> g_phase{kPhaseIdle};
std::atomic<int> g_start_code{-1};
std::atomic<int> g_stop_code{-1};
std::atomic<int> g_retained{0};
std::atomic<unsigned> g_stored{0};
std::atomic<unsigned> g_omitted{0};
std::atomic<unsigned> g_attempts{0};

Capture* g_capture = nullptr;
uint64_t g_dropped = 0;
uint64_t g_unread = 0;
int g_session_fixture = 0;
int g_game_match = 0;
uint32_t g_queue = 0;
uint32_t g_records = 0;
Identity g_identity{};

char g_log[kLogCap] = {};
size_t g_log_used = 0;
bool g_log_truncated = false;
wchar_t g_log_dir[kPathCap] = {};
wchar_t g_session_path[kCapturePathCap] = {};
bool g_log_bound = false;

// Releases the busy phase on every return. Callers that still own a live
// capture set `phase` to Held before any call that can throw, so an exception
// cannot publish Idle while hooks are installed.
struct Slot {
    bool active = false;
    uint32_t phase = kPhaseIdle;
    ~Slot() {
        if (active) g_phase.store(phase, std::memory_order_release);
    }
    void finish(uint32_t next) { phase = next; }
};

bool enter(int& denied) {
    uint32_t seen = kPhaseIdle;
    if (!g_phase.compare_exchange_strong(seen, kPhaseBusy, std::memory_order_acq_rel, std::memory_order_acquire)) {
        denied = seen == kPhaseHeld ? kPkt3576LiveRetained : kPkt3576LiveRefused;
        return false;
    }
    return true;
}

__declspec(noinline) void pin_anchor() {}

bool pin_module() {
    HMODULE module = nullptr;
    void* address = reinterpret_cast<void*>(&pin_anchor);
    LPCWSTR key = nullptr;
    static_assert(sizeof address == sizeof key);
    std::memcpy(&key, &address, sizeof key);
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, key, &module) !=
           FALSE;
}

bool in_module(void* module, void* address) {
    if (module == nullptr || address == nullptr) return false;
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(address, &info, sizeof info) != sizeof info) return false;
    return info.State == MEM_COMMIT && info.AllocationBase == module;
}

const char* code_name(int code) {
    switch (code) {
    case kPkt3576LiveStarted:
        return "started";
    case kPkt3576LiveRefused:
        return "refused";
    case kPkt3576LiveRolledBack:
        return "rolled-back";
    case kPkt3576LiveRetained:
        return "retained";
    case kPkt3576LiveRemoved:
        return "removed";
    case kPkt3576LiveIdle:
        return "idle";
    case kPkt3576LiveBadLog:
        return "bad-log";
    default:
        return "unknown";
    }
}

void log_append(const char* text) {
    if (text == nullptr || g_log_truncated) return;
    const size_t length = std::strlen(text);
    if (g_log_used + length + 1 >= kLogCap) {
        g_log_truncated = true;
        const char marker[] = "truncated=1\n";
        const size_t mark = sizeof marker - 1;
        if (g_log_used + mark + 1 < kLogCap) {
            std::memcpy(g_log + g_log_used, marker, mark);
            g_log_used += mark;
            g_log[g_log_used] = '\0';
        }
        return;
    }
    std::memcpy(g_log + g_log_used, text, length);
    g_log_used += length;
    g_log[g_log_used] = '\0';
}

bool log_flush() {
    if (!g_log_bound || g_session_path[0] == L'\0') return false;
    FILE* file = nullptr;
    if (_wfopen_s(&file, g_session_path, L"wb") != 0 || file == nullptr) return false;
    size_t wrote = 0;
    if (g_log_used > 0) wrote = std::fwrite(g_log, 1, g_log_used, file);
    const int closed = std::fclose(file);
    return closed == 0 && wrote == g_log_used;
}

void append_sanitized_name(const wchar_t* name) {
    wchar_t clipped[80] = {};
    if (name != nullptr) wcsncpy_s(clipped, name, _TRUNCATE);
    char narrow[96] = {};
    if (clipped[0] != L'\0') {
        const int converted =
            WideCharToMultiByte(CP_UTF8, 0, clipped, -1, narrow, static_cast<int>(sizeof narrow), nullptr, nullptr);
        if (converted <= 0) narrow[0] = '\0';
    }
    for (char* cursor = narrow; *cursor != '\0'; ++cursor) {
        const unsigned char byte = static_cast<unsigned char>(*cursor);
        if (byte < 0x20 || *cursor == '"') *cursor = '?';
    }
    if (narrow[0] == '\0') std::memcpy(narrow, "unknown", 8);
    char line[128] = {};
    std::snprintf(line, sizeof line, "name=%s\n", narrow);
    log_append(line);
}

void append_hex_field(const char* label, bool known, uint32_t value) {
    char line[64] = {};
    if (known) std::snprintf(line, sizeof line, "%s=%08X\n", label, value);
    else std::snprintf(line, sizeof line, "%s=unknown\n", label);
    log_append(line);
}

void write_block(const char* mode, int fixture, int game_match, const Identity& identity, const char* startup,
                 const char* shutdown, uint32_t queue, uint32_t records, const wchar_t* exe_name) {
    // Fixture output is never acceptance of the host as build 3576.
    if (fixture != 0 || identity.kind != IdentityKind::Match) game_match = 0;
    char line[512] = {};
    std::snprintf(line, sizeof line, "---\nmode=%s\nfixture=%d\ngame_match=%d\n", mode, fixture != 0 ? 1 : 0,
                  game_match != 0 ? 1 : 0);
    log_append(line);
    append_sanitized_name(exe_name);
    std::snprintf(line, sizeof line, "base=%016llX\n", static_cast<unsigned long long>(identity.mapped_base));
    log_append(line);
    append_hex_field("timestamp", identity.timestamp_known, identity.timestamp);
    append_hex_field("size", identity.size_known, identity.size_of_image);
    std::snprintf(line, sizeof line,
                  "kind=%s\nrejected_2976=%d\ngate=%s\nstartup=%s\nshutdown=%s\nstored=%u\nomitted=%u\n"
                  "dropped=%llu\nunread=%llu\ncapture_attempts=%u\npid=%lu\ntick=%llu\nlimits=%u/%u\n",
                  identity_kind_name(identity.kind), identity.rejected_2976 ? 1 : 0,
                  fixture != 0 ? "fixture" : "identity", startup, shutdown, g_stored.load(std::memory_order_relaxed),
                  g_omitted.load(std::memory_order_relaxed), static_cast<unsigned long long>(g_dropped),
                  static_cast<unsigned long long>(g_unread), g_attempts.load(std::memory_order_relaxed),
                  GetCurrentProcessId(), static_cast<unsigned long long>(GetTickCount64()), queue, records);
    log_append(line);
    log_flush();
}

const wchar_t* file_name_of(const wchar_t* path) {
    const wchar_t* name = path;
    if (path == nullptr) return L"";
    for (const wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') name = cursor + 1;
    }
    return name;
}

bool host_path(wchar_t* output, size_t output_count) {
    const DWORD length = GetModuleFileNameW(nullptr, output, static_cast<DWORD>(output_count));
    return length != 0 && length < output_count;
}

bool join_file(wchar_t* output, size_t output_count, const wchar_t* directory, const wchar_t* file_name) {
    const size_t dir_len = wcslen(directory);
    const size_t file_len = wcslen(file_name);
    const bool slash = dir_len > 0 && directory[dir_len - 1] != L'\\' && directory[dir_len - 1] != L'/';
    const size_t total = dir_len + (slash ? 1 : 0) + file_len;
    if (total >= kCapturePathCap || total + 1 > output_count) return false;
    std::memcpy(output, directory, dir_len * sizeof(wchar_t));
    size_t used = dir_len;
    if (slash) output[used++] = L'\\';
    std::memcpy(output + used, file_name, (file_len + 1) * sizeof(wchar_t));
    return true;
}

// True when `full` is an existing directory, or when this call creates one.
// A file at that path is rejected. Nothing here writes the session globals.
bool directory_ready(const wchar_t* full) {
    const DWORD existing = GetFileAttributesW(full);
    if (existing != INVALID_FILE_ATTRIBUTES) return (existing & FILE_ATTRIBUTE_DIRECTORY) != 0;
    if (CreateDirectoryW(full, nullptr)) return true;
    if (GetLastError() != ERROR_ALREADY_EXISTS) return false;
    const DWORD created = GetFileAttributesW(full);
    return created != INVALID_FILE_ATTRIBUTES && (created & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void rollback_initial_log(const wchar_t* directory, const wchar_t* session, size_t used, bool truncated) {
    wcsncpy_s(g_log_dir, directory, _TRUNCATE);
    wcsncpy_s(g_session_path, session, _TRUNCATE);
    g_log_bound = false;
    g_log_used = used;
    g_log_truncated = truncated;
    if (g_log_used < kLogCap) g_log[g_log_used] = '\0';
}

// Creates one directory level. Rejected locations are not created.
// The initial binding is committed only after the session file is written.
// A failed first attempt leaves the runner free to use another directory.
int bind_log(const wchar_t* log_directory, const wchar_t* host) {
    if (!log_directory_allowed(host, log_directory)) return kPkt3576LiveBadLog;

    wchar_t full[kPathCap] = {};
    const DWORD written = GetFullPathNameW(log_directory, static_cast<DWORD>(kPathCap), full, nullptr);
    if (written == 0 || written >= kPathCap) return kPkt3576LiveBadLog;

    wchar_t records[kCapturePathCap] = {};
    wchar_t session[kCapturePathCap] = {};
    if (!join_file(records, kCapturePathCap, full, kRecordsFile) ||
        !join_file(session, kCapturePathCap, full, kSessionFile)) {
        return kPkt3576LiveBadLog;
    }
    if (g_log_bound && _wcsicmp(g_log_dir, full) != 0) return kPkt3576LiveBadLog;
    if (!directory_ready(full)) return kPkt3576LiveBadLog;

    if (g_log_bound) {
        if (!log_flush()) return kPkt3576LiveBadLog;
        return kPkt3576LiveStarted;
    }

    wchar_t previous_dir[kPathCap] = {};
    wchar_t previous_session[kCapturePathCap] = {};
    wcsncpy_s(previous_dir, g_log_dir, _TRUNCATE);
    wcsncpy_s(previous_session, g_session_path, _TRUNCATE);
    const size_t previous_used = g_log_used;
    const bool previous_truncated = g_log_truncated;

    wcsncpy_s(g_log_dir, full, _TRUNCATE);
    wcsncpy_s(g_session_path, session, _TRUNCATE);
    g_log_bound = true;
    if (g_log_used == 0) log_append("pkt3576-live\n");
    if (!log_flush()) {
        rollback_initial_log(previous_dir, previous_session, previous_used, previous_truncated);
        return kPkt3576LiveBadLog;
    }
    return kPkt3576LiveStarted;
}

void remember_counters() {
    if (g_capture == nullptr) return;
    g_stored.store(g_capture->stored(), std::memory_order_relaxed);
    g_omitted.store(g_capture->omitted(), std::memory_order_relaxed);
    g_dropped = g_capture->dropped();
    g_unread = g_capture->unread();
}

// Deletes the capture only when the backend no longer needs it.
// A failed or throwing release leaves the object and reports retention.
bool release_capture() {
    if (g_capture == nullptr) return true;
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

const char* mode_name();

// Held is committed before the log write. A throw from the log cannot publish Idle.
void note_retained(Slot& slot, const wchar_t* exe_name, const char* startup) {
    g_retained.store(1, std::memory_order_release);
    g_stop_code.store(kPkt3576LiveRetained, std::memory_order_relaxed);
    slot.finish(kPhaseHeld);
    try {
        write_block(mode_name(), g_session_fixture, g_game_match, g_identity, startup, "retained", g_queue, g_records,
                    exe_name);
    } catch (...) {
        // The Held phase and the capture owner are already committed.
    }
}

const char* mode_name() { return g_session_fixture != 0 ? "fixture" : "process"; }

int arm(Slot& slot, void* module, void* send_entry, void* recv_entry, uint32_t queue, uint32_t records,
        const wchar_t* exe_name) {
    g_queue = queue;
    g_records = records;
    const int game_match = g_game_match;
    if (queue < kMinQueue || queue > kMaxQueue || records < kMinRecords || records > kMaxRecords ||
        !in_module(module, send_entry) || !in_module(module, recv_entry) || send_entry == recv_entry) {
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "refused", "idle", queue, records, exe_name);
        return kPkt3576LiveRefused;
    }

    wchar_t records_path[kCapturePathCap] = {};
    if (!join_file(records_path, kCapturePathCap, g_log_dir, kRecordsFile)) {
        g_start_code.store(kPkt3576LiveBadLog, std::memory_order_relaxed);
        return kPkt3576LiveBadLog;
    }

    g_capture = Capture::create(CaptureLimits{queue, records}, records_path);
    if (g_capture == nullptr) {
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "refused", "idle", queue, records, exe_name);
        return kPkt3576LiveRefused;
    }

    // The module stays mapped for the rest of the process once callbacks can run.
    // GET_MODULE_HANDLE_EX_FLAG_PIN cannot be undone, including after a clean stop.
    if (!pin_module()) {
        if (!release_capture()) {
            g_start_code.store(kPkt3576LiveRetained, std::memory_order_relaxed);
            slot.finish(kPhaseHeld);
            write_block(mode_name(), g_session_fixture, game_match, g_identity, "retained", "retained", queue, records,
                        exe_name);
            return kPkt3576LiveRetained;
        }
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "refused", "idle", queue, records, exe_name);
        return kPkt3576LiveRefused;
    }

    g_attempts.fetch_add(1, std::memory_order_acq_rel);
    Capture::StartResult result = Capture::StartResult::Refused;
    try {
        result = g_capture->start(module, send_entry, recv_entry);
    } catch (...) {
        if (!release_capture()) {
            g_start_code.store(kPkt3576LiveRetained, std::memory_order_relaxed);
            slot.finish(kPhaseHeld);
            write_block(mode_name(), g_session_fixture, game_match, g_identity, "retained", "retained", queue, records,
                        exe_name);
            return kPkt3576LiveRetained;
        }
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "refused", "idle", queue, records, exe_name);
        return kPkt3576LiveRefused;
    }

    remember_counters();
    if (result == Capture::StartResult::Started) {
        g_start_code.store(kPkt3576LiveStarted, std::memory_order_relaxed);
        g_stop_code.store(-1, std::memory_order_relaxed);
        slot.finish(kPhaseActive);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "started", "pending", queue, records,
                    exe_name);
        return kPkt3576LiveStarted;
    }
    if (result == Capture::StartResult::Retained || !release_capture()) {
        g_retained.store(1, std::memory_order_release);
        g_start_code.store(kPkt3576LiveRetained, std::memory_order_relaxed);
        slot.finish(kPhaseHeld);
        write_block(mode_name(), g_session_fixture, game_match, g_identity, "retained", "retained", queue, records,
                    exe_name);
        return kPkt3576LiveRetained;
    }

    const bool rolled = result == Capture::StartResult::RolledBack;
    const int code = rolled ? kPkt3576LiveRolledBack : kPkt3576LiveRefused;
    g_start_code.store(code, std::memory_order_relaxed);
    write_block(mode_name(), g_session_fixture, game_match, g_identity, code_name(code), rolled ? "rolled-back" : "idle",
                queue, records, exe_name);
    return code;
}

void remember_session(int fixture, int game_match, const Identity& identity) {
    g_session_fixture = fixture;
    g_game_match = fixture != 0 ? 0 : game_match;
    g_identity = identity;
}

}  // namespace

int start(const wchar_t* log_directory) {
    int denied = 0;
    if (!enter(denied)) return denied;
    Slot slot;
    slot.active = true;

    wchar_t host[kPathCap] = {};
    if (!host_path(host, kPathCap)) {
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        return kPkt3576LiveRefused;
    }
    const int bound = bind_log(log_directory, host);
    if (bound != kPkt3576LiveStarted) {
        g_start_code.store(bound, std::memory_order_relaxed);
        return bound;
    }

    const Identity identity = identify_process();
    const wchar_t* exe_name = file_name_of(host);
    remember_session(0, 0, identity);
    if (!identity_allows_hooks(identity)) {
        g_stored.store(0, std::memory_order_relaxed);
        g_omitted.store(0, std::memory_order_relaxed);
        g_dropped = 0;
        g_unread = 0;
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block("process", 0, 0, identity, "refused", "idle", kProductionQueue, kProductionRecords, exe_name);
        return kPkt3576LiveRefused;
    }

    auto* module = GetModuleHandleW(nullptr);
    const auto base = reinterpret_cast<uintptr_t>(module);
    if (module == nullptr || base != identity.mapped_base || base > UINTPTR_MAX - static_cast<uintptr_t>(kSendRva) ||
        base > UINTPTR_MAX - static_cast<uintptr_t>(kRecvRva)) {
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        write_block("process", 0, 0, identity, "refused", "idle", kProductionQueue, kProductionRecords, exe_name);
        return kPkt3576LiveRefused;
    }
    remember_session(0, 1, identity);
    void* send_entry = reinterpret_cast<void*>(base + static_cast<uintptr_t>(kSendRva));
    void* recv_entry = reinterpret_cast<void*>(base + static_cast<uintptr_t>(kRecvRva));
    return arm(slot, module, send_entry, recv_entry, kProductionQueue, kProductionRecords, exe_name);
}

int stop() {
    uint32_t seen = g_phase.load(std::memory_order_acquire);
    if (seen == kPhaseHeld) return kPkt3576LiveRetained;
    if (seen == kPhaseIdle) return kPkt3576LiveIdle;
    if (seen != kPhaseActive ||
        !g_phase.compare_exchange_strong(seen, kPhaseBusy, std::memory_order_acq_rel, std::memory_order_acquire)) {
        const uint32_t now = g_phase.load(std::memory_order_acquire);
        if (seen == kPhaseHeld || now == kPhaseHeld) return kPkt3576LiveRetained;
        if (seen == kPhaseIdle || now == kPhaseIdle) return kPkt3576LiveIdle;
        return kPkt3576LiveRefused;
    }

    Slot slot;
    slot.active = true;
    wchar_t host[kPathCap] = {};
    const wchar_t* exe_name = L"";
    if (host_path(host, kPathCap)) exe_name = file_name_of(host);
    const char* startup = code_name(g_start_code.load(std::memory_order_relaxed));
    if (g_capture == nullptr) {
        g_stop_code.store(kPkt3576LiveIdle, std::memory_order_relaxed);
        return kPkt3576LiveIdle;
    }

    // Publish Held before any call that can throw. Slot's destructor must not
    // store Idle while this capture, its hooks, or its writer may still be live.
    slot.finish(kPhaseHeld);
    Capture::StopResult result = Capture::StopResult::Retained;
    try {
        result = g_capture->stop();
        remember_counters();
    } catch (...) {
        // Do not call release_owner. Cleanup was not proved, and another stop
        // would run while the hook may still be patched.
        note_retained(slot, exe_name, startup);
        return kPkt3576LiveRetained;
    }

    if (result == Capture::StopResult::Removed) {
        if (!release_capture()) {
            note_retained(slot, exe_name, startup);
            return kPkt3576LiveRetained;
        }
        g_retained.store(0, std::memory_order_release);
        g_stop_code.store(kPkt3576LiveRemoved, std::memory_order_relaxed);
        slot.finish(kPhaseIdle);
        try {
            write_block(mode_name(), g_session_fixture, g_game_match, g_identity, startup, "removed", g_queue,
                        g_records, exe_name);
        } catch (...) {
        }
        return kPkt3576LiveRemoved;
    }
    if (result == Capture::StopResult::Retained || !release_capture()) {
        note_retained(slot, exe_name, startup);
        return kPkt3576LiveRetained;
    }
    g_retained.store(0, std::memory_order_release);
    g_stop_code.store(kPkt3576LiveIdle, std::memory_order_relaxed);
    slot.finish(kPhaseIdle);
    return kPkt3576LiveIdle;
}

int status(int* start_code, int* stop_code, int* retained, unsigned* stored, unsigned* omitted,
           unsigned* capture_attempts) {
    if (start_code != nullptr) *start_code = g_start_code.load(std::memory_order_acquire);
    if (stop_code != nullptr) *stop_code = g_stop_code.load(std::memory_order_acquire);
    if (retained != nullptr) *retained = g_retained.load(std::memory_order_acquire);
    if (stored != nullptr) *stored = g_stored.load(std::memory_order_acquire);
    if (omitted != nullptr) *omitted = g_omitted.load(std::memory_order_acquire);
    if (capture_attempts != nullptr) *capture_attempts = g_attempts.load(std::memory_order_acquire);
    return 0;
}

#ifdef PKT3576_LIVE_FIXTURE
int fixture_start(const wchar_t* log_directory, void* send_entry, void* recv_entry, unsigned queue_capacity,
                  unsigned output_records) {
    int denied = 0;
    if (!enter(denied)) return denied;
    Slot slot;
    slot.active = true;

    wchar_t host[kPathCap] = {};
    if (!host_path(host, kPathCap)) {
        g_start_code.store(kPkt3576LiveRefused, std::memory_order_relaxed);
        return kPkt3576LiveRefused;
    }
    const int bound = bind_log(log_directory, host);
    if (bound != kPkt3576LiveStarted) {
        g_start_code.store(bound, std::memory_order_relaxed);
        return bound;
    }

    // Observed identity is logged and is not the hook decision. game_match stays 0.
    Identity identity = identify_process();
    remember_session(1, 0, identity);
    return arm(slot, GetModuleHandleW(nullptr), send_entry, recv_entry, queue_capacity, output_records,
               file_name_of(host));
}
#endif

}  // namespace pkt3576::live
