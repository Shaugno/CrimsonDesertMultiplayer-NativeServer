#include "pkt3663/capture.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#include <utility>
#include <vector>

#include <windows.h>

// Toolhelp depends on Windows base declarations and must follow windows.h.
#include <tlhelp32.h>

#include "safetyhook/mid_hook.hpp"

namespace pkt3663 {
namespace {

// SafetyHook v0.7.0 restores target bytes while disabling, but freeing a
// MidHook also frees its stub and trampoline. Disabling alone cannot prove
// that another thread has left those allocations. Removal therefore suspends
// the other threads and proves their RIPs are outside every callback, stub,
// and trampoline range before releasing any hook-owned memory.

struct SchedHook {
  std::atomic<uintptr_t> function{0};
  std::atomic<uintptr_t> user{0};

  void set(void (*next)(void *), void *next_user) {
    user.store(reinterpret_cast<uintptr_t>(next_user),
               std::memory_order_relaxed);
    function.store(reinterpret_cast<uintptr_t>(next),
                   std::memory_order_release);
  }

  void run() const {
    const auto callback = reinterpret_cast<void (*)(void *)>(
        function.load(std::memory_order_acquire));
    if (callback != nullptr)
      callback(reinterpret_cast<void *>(user.load(std::memory_order_relaxed)));
  }
};

SchedHook g_records_open_hook;
SchedHook g_phase_hold;
SchedHook g_pre_send_hook;

constexpr uint32_t kQuiesceMs = 2000;
constexpr size_t kMaxHookRegion = 1024 * 1024;
constexpr size_t kMaxCallbackSpan = 65536;
// Pinned to SafetyHook v0.7.0's Windows x64 MidHook stub layout.
constexpr size_t kMidStubBytes = 391;
constexpr size_t kMidCallbackOffset = kMidStubBytes - 16;
constexpr size_t kMidContinuationOffset = kMidStubBytes - 8;
constexpr size_t kMaxTargetPatch = 64;

struct Range {
  uintptr_t begin = 0;
  uintptr_t end = 0;

  bool contains(uintptr_t ip) const { return ip >= begin && ip < end; }
};

enum class PatchForm : uint8_t { E9, FF25 };

struct HookRoute {
  PatchForm form = PatchForm::E9;
  uintptr_t relay = 0;
  uintptr_t indirect_slot = 0;
  uintptr_t stub = 0;
  uintptr_t callback = 0;
  uintptr_t continuation = 0;
  Range stub_region{};
  Range trampoline_region{};
};

bool same_range(const Range &left, const Range &right) {
  return left.begin == right.begin && left.end == right.end;
}

bool same_route(const HookRoute &left, const HookRoute &right) {
  return left.form == right.form && left.relay == right.relay &&
         left.indirect_slot == right.indirect_slot && left.stub == right.stub &&
         left.callback == right.callback &&
         left.continuation == right.continuation &&
         same_range(left.stub_region, right.stub_region) &&
         same_range(left.trampoline_region, right.trampoline_region);
}

bool function_extent(const void *address, Range &out) {
  DWORD64 image_base = 0;
  const PRUNTIME_FUNCTION entry = RtlLookupFunctionEntry(
      reinterpret_cast<DWORD64>(address), &image_base, nullptr);
  if (entry == nullptr || entry->EndAddress <= entry->BeginAddress)
    return false;
  const auto begin = static_cast<uintptr_t>(image_base + entry->BeginAddress);
  const auto end = static_cast<uintptr_t>(image_base + entry->EndAddress);
  if (end <= begin || end - begin > kMaxCallbackSpan)
    return false;
  out.begin = begin;
  out.end = end;
  return true;
}

bool follows_e9(const void *address, void *&target) {
  const auto *bytes = static_cast<const uint8_t *>(address);
  int32_t relative = 0;
  bool jump = false;
  __try {
    if (bytes[0] == 0xE9) {
      std::memcpy(&relative, bytes + 1, sizeof relative);
      jump = true;
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
  if (!jump)
    return false;
  target = const_cast<uint8_t *>(bytes) + 5 + relative;
  return target != address;
}

bool collect_callback(const void *symbol, Range &primary, Range &extra,
                      bool &have_extra) {
  have_extra = false;
  if (!function_extent(symbol, primary))
    return false;
  void *body = nullptr;
  if (!follows_e9(symbol, body))
    return true;
  if (!function_extent(body, extra))
    return false;
  have_extra = extra.begin != primary.begin || extra.end != primary.end;
  return true;
}

bool region_of(const void *address, Range &out) {
  if (address == nullptr)
    return false;
  MEMORY_BASIC_INFORMATION info{};
  if (VirtualQuery(address, &info, sizeof info) == 0)
    return false;
  if (info.State != MEM_COMMIT || info.RegionSize == 0 ||
      info.RegionSize > kMaxHookRegion)
    return false;
  out.begin = reinterpret_cast<uintptr_t>(info.BaseAddress);
  out.end = out.begin + info.RegionSize;
  return out.contains(reinterpret_cast<uintptr_t>(address));
}

bool read_u64(const void *address, uint64_t &out) {
  std::memcpy(&out, address, sizeof out);
  return true;
}

bool note_route(void *target, HookRoute &out) {
  if (target == nullptr)
    return false;
  auto *bytes = static_cast<uint8_t *>(target);
  HookRoute route;
  __try {
    if (bytes[0] == 0xE9) {
      route.form = PatchForm::E9;
      int32_t relative = 0;
      std::memcpy(&relative, bytes + 1, sizeof relative);
      auto *relay = bytes + 5 + relative;
      if (relay == bytes || relay[0] != 0xFF || relay[1] != 0x25)
        return false;
      route.relay = reinterpret_cast<uintptr_t>(relay);
      int32_t displacement = 0;
      std::memcpy(&displacement, relay + 2, sizeof displacement);
      auto *indirect = relay + 6 + displacement;
      route.indirect_slot = reinterpret_cast<uintptr_t>(indirect);
      uint64_t destination = 0;
      if (!read_u64(indirect, destination))
        return false;
      route.stub = static_cast<uintptr_t>(destination);
    } else if (bytes[0] == 0xFF && bytes[1] == 0x25) {
      route.form = PatchForm::FF25;
      route.relay = reinterpret_cast<uintptr_t>(bytes);
      int32_t displacement = 0;
      std::memcpy(&displacement, bytes + 2, sizeof displacement);
      auto *indirect = bytes + 6 + displacement;
      route.indirect_slot = reinterpret_cast<uintptr_t>(indirect);
      uint64_t destination = 0;
      if (!read_u64(indirect, destination))
        return false;
      route.stub = static_cast<uintptr_t>(destination);
    } else {
      return false;
    }
    if (route.stub == 0)
      return false;
    uint64_t callback = 0;
    uint64_t continuation = 0;
    auto *stub = reinterpret_cast<uint8_t *>(route.stub);
    if (!read_u64(stub + kMidCallbackOffset, callback) ||
        !read_u64(stub + kMidContinuationOffset, continuation)) {
      return false;
    }
    route.callback = static_cast<uintptr_t>(callback);
    route.continuation = static_cast<uintptr_t>(continuation);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
  if (route.callback == 0 || route.continuation == 0 ||
      !region_of(reinterpret_cast<void *>(route.stub), route.stub_region) ||
      !region_of(reinterpret_cast<void *>(route.continuation),
                 route.trampoline_region)) {
    return false;
  }
  // In the near form, the target's E9 lands on the FF25 destination relay
  // inside the same inline-trampoline allocation whose base is stored in the
  // MidHook stub tail.
  if (route.form == PatchForm::E9 &&
      !route.trampoline_region.contains(route.relay))
    return false;
  out = route;
  return true;
}

bool snapshot_target_patch(const safetyhook::MidHook &hook, void *target,
                           uint8_t (&bytes)[kMaxTargetPatch], uint8_t &size) {
  size = 0;
  if (target == nullptr || hook.target() != target)
    return false;
  const auto &original = hook.original_bytes();
  if (original.empty() || original.size() > kMaxTargetPatch ||
      original.size() > UINT8_MAX)
    return false;
  auto *source = static_cast<const uint8_t *>(target);
  __try {
    size = static_cast<uint8_t>(original.size());
    std::memcpy(bytes, source, size);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    size = 0;
    return false;
  }
  if (bytes[0] == 0xE9) {
    if (size < 5)
      return false;
    for (uint8_t index = 5; index < size; ++index) {
      if (bytes[index] != 0x90)
        return false;
    }
    return true;
  }
  if (size < 14 || bytes[0] != 0xFF || bytes[1] != 0x25)
    return false;
  int32_t displacement = 0;
  std::memcpy(&displacement, bytes + 2, sizeof displacement);
  if (displacement != 0)
    return false;
  for (uint8_t index = 14; index < size; ++index) {
    if (bytes[index] != 0x90)
      return false;
  }
  return true;
}

bool in_module(void *module, const void *address) {
  if (module == nullptr || address == nullptr)
    return false;
  MEMORY_BASIC_INFORMATION info{};
  if (VirtualQuery(address, &info, sizeof info) == 0)
    return false;
  return info.State == MEM_COMMIT && info.AllocationBase == module;
}

bool read_thunk(uint64_t address, void *destination, size_t size,
                void *) noexcept {
  return guarded_read(address, destination, size);
}

struct PausedThreads {
  HANDLE handles[256]{};
  int count = 0;
  bool complete = false;
  bool suspended = false;

  PausedThreads() = default;
  PausedThreads(const PausedThreads &) = delete;
  PausedThreads &operator=(const PausedThreads &) = delete;
  PausedThreads(PausedThreads &&other) noexcept { *this = std::move(other); }
  PausedThreads &operator=(PausedThreads &&other) noexcept {
    if (this != &other) {
      resume();
      count = other.count;
      complete = other.complete;
      suspended = other.suspended;
      for (int index = 0; index < count; ++index)
        handles[index] = other.handles[index];
      other.count = 0;
      other.complete = false;
      other.suspended = false;
    }
    return *this;
  }

  ~PausedThreads() { resume(); }

  static PausedThreads suspend_others() {
    PausedThreads paused;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
      return paused;

    THREADENTRY32 entry{};
    entry.dwSize = sizeof entry;
    const DWORD pid = GetCurrentProcessId();
    const DWORD self = GetCurrentThreadId();
    bool failed = false;
    if (Thread32First(snapshot, &entry) == FALSE) {
      const DWORD error = GetLastError();
      CloseHandle(snapshot);
      if (error != ERROR_NO_MORE_FILES)
        return paused;
      paused.suspended = true;
      paused.complete = true;
      return paused;
    }

    for (;;) {
      if (entry.th32OwnerProcessID == pid && entry.th32ThreadID != self) {
        if (paused.count >= 256) {
          failed = true;
          break;
        }
        const HANDLE handle =
            OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                           THREAD_QUERY_INFORMATION,
                       FALSE, entry.th32ThreadID);
        if (handle == nullptr) {
          if (GetLastError() != ERROR_INVALID_PARAMETER) {
            failed = true;
            break;
          }
        } else {
          paused.handles[paused.count++] = handle;
        }
      }
      SetLastError(ERROR_SUCCESS);
      if (Thread32Next(snapshot, &entry) == FALSE) {
        if (GetLastError() != ERROR_NO_MORE_FILES)
          failed = true;
        break;
      }
    }
    CloseHandle(snapshot);
    if (failed) {
      for (int index = 0; index < paused.count; ++index)
        CloseHandle(paused.handles[index]);
      paused.count = 0;
      return paused;
    }

    int index = 0;
    while (index < paused.count) {
      if (SuspendThread(paused.handles[index]) != static_cast<DWORD>(-1)) {
        ++index;
        continue;
      }
      DWORD code = 0;
      const bool exited =
          GetExitCodeThread(paused.handles[index], &code) != 0 &&
          code != STILL_ACTIVE;
      if (exited) {
        CloseHandle(paused.handles[index]);
        paused.handles[index] = paused.handles[paused.count - 1];
        --paused.count;
        continue;
      }
      for (int done = 0; done < index; ++done)
        ResumeThread(paused.handles[done]);
      for (int handle = 0; handle < paused.count; ++handle)
        CloseHandle(paused.handles[handle]);
      paused.count = 0;
      return paused;
    }
    paused.suspended = true;
    paused.complete = true;
    return paused;
  }

  void resume() {
    if (suspended) {
      for (int index = 0; index < count; ++index)
        ResumeThread(handles[index]);
      suspended = false;
    }
    for (int index = 0; index < count; ++index)
      CloseHandle(handles[index]);
    count = 0;
    complete = false;
  }

  bool any_rip(const Range *ranges, int range_count) const {
    for (int index = 0; index < count; ++index) {
      CONTEXT context{};
      context.ContextFlags = CONTEXT_CONTROL;
      if (GetThreadContext(handles[index], &context) == 0)
        return true;
      for (int range = 0; range < range_count; ++range) {
        if (ranges[range].contains(static_cast<uintptr_t>(context.Rip)))
          return true;
      }
    }
    return false;
  }
};

void append_hex(char *destination, size_t capacity, size_t &used,
                uint64_t value, int digits) {
  char scratch[32] = {};
  std::snprintf(scratch, sizeof scratch, "%0*llX", digits,
                static_cast<unsigned long long>(value));
  const size_t length = std::strlen(scratch);
  if (used + length >= capacity)
    return;
  std::memcpy(destination + used, scratch, length);
  used += length;
}

void append_text(char *destination, size_t capacity, size_t &used,
                 const char *text) {
  const size_t length = std::strlen(text);
  if (used + length >= capacity)
    return;
  std::memcpy(destination + used, text, length);
  used += length;
}

constexpr size_t kLineCap = static_cast<size_t>(kCopyCap) * 2 + 1024;
static_assert(kLineCap >= 8192 + 512);

const char *status_name(RawStatus status) {
  return status == RawStatus::Truncated ? "truncated" : "complete";
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

} // namespace

struct Capture::State {
  CaptureLimits limits{};
  wchar_t path[260] = {};
  bool have_path = false;
  Observer *observer = nullptr;
  FILE *file = nullptr;
  safetyhook::MidHook recv_hook{};
  safetyhook::MidHook send_hook{};
  HookRoute recv_route{};
  HookRoute send_route{};
  bool recv_noted = false;
  bool send_noted = false;
  uint8_t recv_patch[kMaxTargetPatch] = {};
  uint8_t recv_patch_size = 0;
  uint8_t send_patch[kMaxTargetPatch] = {};
  uint8_t send_patch_size = 0;
  Range send_callback{};
  Range recv_callback{};
  Range send_callback_extra{};
  Range recv_callback_extra{};
  bool send_extra = false;
  bool recv_extra = false;
  std::vector<Record> records;
  char *line = nullptr;
  std::thread writer;
  std::atomic<uint32_t> stored{0};
  std::atomic<uint32_t> omitted{0};
  std::atomic<bool> writer_blocked{false};
  std::atomic<bool> writer_stop{false};
  volatile long entries = 0;
  bool running = false;
  bool release_blocked = false;
  void (*stage_mark)(uint8_t stage) noexcept = nullptr;

  void on_hook(bool send, const safetyhook::Context &context) {
    if (observer == nullptr)
      return;
    EntryArgs args;
    args.rcx = context.rcx;
    args.rdx = context.rdx;
    args.r8 = context.r8;
    args.r9 = context.r9;
    args.rsp = context.rsp;
    if (send)
      observer->on_send(args);
    else
      observer->on_recv(args);
  }

  void publish_record(const Record &record) {
    if (stored.load(std::memory_order_relaxed) >= limits.output_records) {
      omitted.fetch_add(1, std::memory_order_relaxed);
    } else {
      records.push_back(record);
      stored.store(static_cast<uint32_t>(records.size()),
                   std::memory_order_release);
      write_line(record);
    }
  }

  void write_line(const Record &record) {
    if (file == nullptr || line == nullptr)
      return;
    size_t used = 0;
    append_text(line, kLineCap, used,
                record.direction == Direction::Send ? "S " : "R ");
    append_hex(line, kLineCap, used, record.rcx, 16);
    append_text(line, kLineCap, used, " ");
    append_hex(line, kLineCap, used, record.rdx, 16);
    append_text(line, kLineCap, used, " ");
    append_hex(line, kLineCap, used, record.r8, 16);
    append_text(line, kLineCap, used, " ");
    append_hex(line, kLineCap, used, record.r9, 16);
    append_text(line, kLineCap, used, " ");
    append_hex(line, kLineCap, used, record.stack_u16_28, 4);
    if (record.direction == Direction::Send) {
      append_text(line, kLineCap, used, " ");
      append_hex(line, kLineCap, used, record.stack_u8_30, 2);
      append_text(line, kLineCap, used, " ");
      append_hex(line, kLineCap, used, record.stack_u16_38, 4);
    }
    char meta[256] = {};
    if (record.has_raw_word0) {
      std::snprintf(
          meta, sizeof meta,
          " claimed=%u copied=%u status=%s raw_word0=%04X qpc=%llu phase=%s "
          "classification=unclassified receive_pre_transform_possible=%u ",
          record.claimed_length, record.copied_length,
          status_name(record.status), record.raw_word0,
          static_cast<unsigned long long>(record.qpc_ticks),
          phase_name(record.phase),
          record.receive_pre_transform_possible ? 1u : 0u);
    } else {
      std::snprintf(
          meta, sizeof meta,
          " claimed=%u copied=%u status=%s raw_word0=unavailable qpc=%llu "
          "phase=%s "
          "classification=unclassified receive_pre_transform_possible=%u ",
          record.claimed_length, record.copied_length,
          status_name(record.status),
          static_cast<unsigned long long>(record.qpc_ticks),
          phase_name(record.phase),
          record.receive_pre_transform_possible ? 1u : 0u);
    }
    append_text(line, kLineCap, used, meta);
    const uint32_t count =
        record.copied_length <= kCopyCap ? record.copied_length : kCopyCap;
    for (uint32_t index = 0; index < count; ++index)
      append_hex(line, kLineCap, used, record.bytes[index], 2);
    append_text(line, kLineCap, used, "\n");
    if (used < kLineCap)
      line[used] = '\0';
    std::fputs(line, file);
  }

  void write_summary() {
    if (file == nullptr || observer == nullptr)
      return;
    std::fprintf(file,
                 "counters dropped=%llu unread=%llu truncated=%llu stored=%u "
                 "omitted=%u\n",
                 static_cast<unsigned long long>(observer->dropped()),
                 static_cast<unsigned long long>(observer->unread()),
                 static_cast<unsigned long long>(observer->truncated()),
                 stored.load(std::memory_order_relaxed),
                 omitted.load(std::memory_order_relaxed));
  }
};

namespace {
extern std::atomic<Capture::State *> g_active;
enum class SlotCode { Ok, FailedClosed, FailedLive };
void send_hook(safetyhook::Context &context);
void recv_hook(safetyhook::Context &context);
SlotCode install_slot(safetyhook::MidHook &hook, void *entry,
                      safetyhook::MidHookFn function, HookRoute &route,
                      bool &noted, uint8_t (&patch)[kMaxTargetPatch],
                      uint8_t &patch_size);
bool hook_present(const safetyhook::MidHook &hook);
bool installed_slot_intact(const safetyhook::MidHook &hook, void *target,
                           const HookRoute &expected_route, bool noted,
                           const uint8_t *expected_patch,
                           uint8_t expected_patch_size);
bool entry_still_patched(const safetyhook::MidHook &hook);
bool retire_installed_hooks(Capture::State &state);
bool destroy_hooks(Capture::State &state);
void close_file(Capture::State &state);
} // namespace

Capture::Capture(State *state) : state_(state) {}

Capture::~Capture() {
  if (state_ == nullptr)
    return;
  close_file(*state_);
  delete[] state_->line;
  state_->line = nullptr;
  if (!state_->release_blocked)
    Observer::release_owner(state_->observer);
  delete state_;
  state_ = nullptr;
}

Capture *Capture::create(const CaptureLimits &limits,
                         const wchar_t *output_path) {
  auto *state = new (std::nothrow) State();
  if (state == nullptr)
    return nullptr;
  state->limits = limits;
  if (output_path != nullptr && output_path[0] != L'\0') {
    wcsncpy_s(state->path, output_path, _TRUNCATE);
    state->have_path = state->path[0] != L'\0';
  }
  state->observer = Observer::create(limits.queue_capacity);
  if (state->observer == nullptr) {
    delete state;
    return nullptr;
  }
  auto *capture = new (std::nothrow) Capture(state);
  if (capture == nullptr) {
    Observer::release_owner(state->observer);
    delete state;
    return nullptr;
  }
  return capture;
}

bool Capture::release_owner(Capture *&capture) {
  if (capture == nullptr)
    return true;
  if (capture->state_ != nullptr && capture->state_->release_blocked)
    return false;
  if (capture->state_ != nullptr && capture->state_->running) {
    if (capture->stop() != StopResult::Removed)
      return false;
  }
  delete capture;
  capture = nullptr;
  return true;
}

void Capture::set_writer_blocked(bool blocked) {
  if (state_ != nullptr)
    state_->writer_blocked.store(blocked, std::memory_order_release);
}

void Capture::set_records_open_hook(void (*hook)(void *), void *user) {
  g_records_open_hook.set(hook, user);
}

void Capture::set_phase_hold(void (*hold)(void *), void *user) {
  g_phase_hold.set(hold, user);
}

void Capture::set_pre_send_hook(void (*hook)(void *), void *user) {
  g_pre_send_hook.set(hook, user);
}

void Capture::set_stage_mark(void (*mark)(uint8_t stage) noexcept) noexcept {
  if (state_ != nullptr)
    state_->stage_mark = mark;
}

void mark_capture_stage(Capture::State &state, uint8_t stage) noexcept {
  if (state.stage_mark == nullptr)
    return;
  try {
    state.stage_mark(stage);
  } catch (...) {
  }
}

void Capture::set_phase(uint8_t phase) {
  g_phase_hold.run();
  if (state_ != nullptr && state_->observer != nullptr)
    state_->observer->set_phase(phase);
}

uint32_t Capture::stored() const {
  return state_ == nullptr ? 0 : state_->stored.load(std::memory_order_acquire);
}

uint32_t Capture::omitted() const {
  return state_ == nullptr ? 0
                           : state_->omitted.load(std::memory_order_acquire);
}

uint64_t Capture::dropped() const {
  return state_ == nullptr || state_->observer == nullptr
             ? 0
             : state_->observer->dropped();
}

uint64_t Capture::unread() const {
  return state_ == nullptr || state_->observer == nullptr
             ? 0
             : state_->observer->unread();
}

uint64_t Capture::truncated() const {
  return state_ == nullptr || state_->observer == nullptr
             ? 0
             : state_->observer->truncated();
}

bool Capture::queue_allocated() const {
  return state_ != nullptr && state_->observer != nullptr &&
         state_->observer->queue_allocated();
}

bool Capture::accepting() const {
  return state_ != nullptr && state_->observer != nullptr &&
         state_->observer->accepting();
}

bool Capture::copy_record(uint32_t index, Record &out) const {
  if (state_ == nullptr || index >= state_->records.size())
    return false;
  out = state_->records[index];
  return true;
}

Capture::StartResult Capture::start(void *module, void *send_entry,
                                    void *recv_entry, InstallCheck check,
                                    void *check_user) {
  if (state_ == nullptr || state_->running || state_->release_blocked)
    return StartResult::Refused;
  if (state_->limits.queue_capacity < kMinQueueCapacity ||
      state_->limits.output_records < 1) {
    return StartResult::Refused;
  }
  if (send_entry == nullptr || recv_entry == nullptr ||
      send_entry == recv_entry)
    return StartResult::Refused;
  if (!in_module(module, send_entry) || !in_module(module, recv_entry))
    return StartResult::Refused;
  if (!collect_callback(&send_hook, state_->send_callback,
                        state_->send_callback_extra, state_->send_extra) ||
      !collect_callback(&recv_hook, state_->recv_callback,
                        state_->recv_callback_extra, state_->recv_extra)) {
    return StartResult::Refused;
  }
  const auto send_address = reinterpret_cast<uintptr_t>(send_entry);
  const auto recv_address = reinterpret_cast<uintptr_t>(recv_entry);
  const bool overlaps =
      state_->send_callback.contains(send_address) ||
      state_->send_callback.contains(recv_address) ||
      state_->recv_callback.contains(send_address) ||
      state_->recv_callback.contains(recv_address) ||
      (state_->send_extra &&
       (state_->send_callback_extra.contains(send_address) ||
        state_->send_callback_extra.contains(recv_address))) ||
      (state_->recv_extra &&
       (state_->recv_callback_extra.contains(send_address) ||
        state_->recv_callback_extra.contains(recv_address)));
  if (overlaps)
    return StartResult::Refused;

  try {
    state_->records.reserve(state_->limits.output_records);
  } catch (const std::bad_alloc &) {
    return StartResult::Refused;
  }
  if (state_->line == nullptr) {
    state_->line = new (std::nothrow) char[kLineCap];
    if (state_->line == nullptr)
      return StartResult::Refused;
  }

  Capture::State *expected = nullptr;
  if (!g_active.compare_exchange_strong(expected, state_,
                                        std::memory_order_acq_rel,
                                        std::memory_order_acquire)) {
    return StartResult::Refused;
  }

  struct PreHookRollback {
    Capture::State *state = nullptr;
    bool active = true;
    bool armed = false;
    ~PreHookRollback() {
      if (!active || state == nullptr)
        return;
      bool clean = false;
      try {
        clean = retire_installed_hooks(*state);
      } catch (...) {
        clean = false;
      }
      if (!clean) {
        state->release_blocked = true;
        return;
      }
      if (armed && state->observer != nullptr)
        state->observer->disarm();
      close_file(*state);
      g_active.store(nullptr, std::memory_order_release);
    }
  } rollback{state_};

  if (state_->have_path) {
    g_records_open_hook.run();
    FILE *file = nullptr;
    const wchar_t *mode = state_->limits.exclusive_create ? L"wbx" : L"wb";
    if (_wfopen_s(&file, state_->path, mode) != 0 || file == nullptr)
      return StartResult::Refused;
    state_->file = file;
    LARGE_INTEGER frequency{};
    QueryPerformanceFrequency(&frequency);
    std::fprintf(
        file,
        "qpc_frequency=%llu\nqpc_timebase=QueryPerformanceCounter\n"
        "capture_mode=raw-all\nclassification=unclassified\nlayout=unresolved\n"
        "receive_pre_transform_possible=1\nadopted=0\nallowlist_count=%u\n"
        "copy_cap=%u\noutput_records=%u\ninterval_guaranteed=0\n",
        static_cast<unsigned long long>(frequency.QuadPart), kAllowlistCount,
        kCopyCap, state_->limits.output_records);
  }

  state_->records.clear();
  state_->stored.store(0, std::memory_order_relaxed);
  state_->omitted.store(0, std::memory_order_relaxed);
  state_->writer_stop.store(false, std::memory_order_relaxed);
  state_->entries = 0;

  if (!state_->observer->arm(&read_thunk, nullptr))
    return StartResult::Refused;
  rollback.armed = true;

  auto retain = [&]() {
    rollback.active = false;
    state_->release_blocked = true;
    return StartResult::Retained;
  };

  try {
    // The production verifier runs at the last possible point before each
    // hook factory. The callback is typed noexcept; false fails closed.
    if (check != nullptr && !check(InstallSite::Receive, nullptr, check_user))
      return StartResult::Refused;
    const SlotCode recv_code = install_slot(
        state_->recv_hook, recv_entry, &recv_hook, state_->recv_route,
        state_->recv_noted, state_->recv_patch, state_->recv_patch_size);
    if (recv_code != SlotCode::Ok) {
      if (recv_code == SlotCode::FailedLive)
        return retain();
      return StartResult::Refused;
    }
    mark_capture_stage(*state_, 1);

    g_pre_send_hook.run();
    // The send-side verifier may substitute the original receive entry
    // bytes while rechecking the full mapped image. First prove the live
    // bytes still route to the exact SafetyHook allocations recorded at
    // receive installation; otherwise that substitution would mask an
    // unexpected patch and cleanup is no longer proven safe.
    if (!installed_slot_intact(state_->recv_hook, recv_entry,
                               state_->recv_route, state_->recv_noted,
                               state_->recv_patch, state_->recv_patch_size)) {
      return retain();
    }
    const auto &recv_original = state_->recv_hook.original_bytes();
    const InstallEvidence recv_evidence{
        state_->recv_patch, state_->recv_patch_size, recv_original.data(),
        static_cast<uint8_t>(recv_original.size())};
    const bool send_allowed =
        check == nullptr ||
        check(InstallSite::Send, &recv_evidence, check_user);
    // Re-prove Receive even when the callback rejects the install. The
    // callback is external to this backend and cleanup may restore bytes
    // only while the exact installed route still belongs to this hook.
    if (!installed_slot_intact(state_->recv_hook, recv_entry,
                               state_->recv_route, state_->recv_noted,
                               state_->recv_patch, state_->recv_patch_size)) {
      return retain();
    }
    if (!send_allowed) {
      if (!retire_installed_hooks(*state_))
        return retain();
      return StartResult::RolledBack;
    }
    const SlotCode send_code = install_slot(
        state_->send_hook, send_entry, &send_hook, state_->send_route,
        state_->send_noted, state_->send_patch, state_->send_patch_size);
    if (send_code != SlotCode::Ok) {
      if (send_code == SlotCode::FailedLive || !retire_installed_hooks(*state_))
        return retain();
      return StartResult::RolledBack;
    }
    mark_capture_stage(*state_, 2);
    const auto &send_original = state_->send_hook.original_bytes();
    const InstallEvidence send_evidence{
        state_->send_patch, state_->send_patch_size, send_original.data(),
        static_cast<uint8_t>(send_original.size())};
    const bool complete_allowed =
        check == nullptr ||
        check(InstallSite::Complete, &send_evidence, check_user);
    // The Complete callback closes the Send verifier/factory window. Both
    // hooks must still have their full target bytes and exact routes before
    // capture is allowed to start or rollback may restore either entry.
    if (!installed_slot_intact(state_->recv_hook, recv_entry,
                               state_->recv_route, state_->recv_noted,
                               state_->recv_patch, state_->recv_patch_size) ||
        !installed_slot_intact(state_->send_hook, send_entry,
                               state_->send_route, state_->send_noted,
                               state_->send_patch, state_->send_patch_size)) {
      return retain();
    }
    if (!complete_allowed) {
      if (!retire_installed_hooks(*state_))
        return retain();
      return StartResult::RolledBack;
    }
  } catch (...) {
    const bool installed =
        hook_present(state_->recv_hook) || hook_present(state_->send_hook);
    bool retired = false;
    try {
      retired = retire_installed_hooks(*state_);
    } catch (...) {
      retired = false;
    }
    if (!retired)
      return retain();
    return installed ? StartResult::RolledBack : StartResult::Refused;
  }

  try {
    state_->writer = std::thread([this] { this->writer_main(); });
  } catch (...) {
    if (!destroy_hooks(*state_)) {
      rollback.active = false;
      state_->release_blocked = true;
      return StartResult::Retained;
    }
    return StartResult::Refused;
  }
  mark_capture_stage(*state_, 3);

  rollback.active = false;
  state_->running = true;
  return StartResult::Started;
}

Capture::StopResult Capture::stop() {
  if (state_ == nullptr)
    return StopResult::Idle;
  if (state_->release_blocked)
    return StopResult::Retained;
  if (!state_->running)
    return StopResult::Idle;
  if (!destroy_hooks(*state_)) {
    state_->release_blocked = true;
    state_->running = false;
    return StopResult::Retained;
  }
  g_active.store(nullptr, std::memory_order_release);
  state_->writer_blocked.store(false, std::memory_order_release);
  state_->writer_stop.store(true, std::memory_order_release);
  if (state_->writer.joinable())
    state_->writer.join();
  if (state_->observer != nullptr)
    state_->observer->disarm();
  close_file(*state_);
  state_->running = false;
  return StopResult::Removed;
}

void Capture::writer_main() {
  if (state_ == nullptr)
    return;
  for (;;) {
    if (state_->writer_blocked.load(std::memory_order_acquire) &&
        !state_->writer_stop.load(std::memory_order_acquire)) {
      YieldProcessor();
      continue;
    }
    Record record;
    if (state_->observer != nullptr && state_->observer->try_pop(record)) {
      state_->publish_record(record);
      continue;
    }
    if (state_->writer_stop.load(std::memory_order_acquire))
      break;
    YieldProcessor();
  }
  state_->write_summary();
  if (state_->file != nullptr)
    std::fflush(state_->file);
}

namespace {

std::atomic<Capture::State *> g_active{nullptr};

__declspec(noinline) void send_hook(safetyhook::Context &context) {
  Capture::State *state = g_active.load(std::memory_order_acquire);
  if (state == nullptr)
    return;
  _InterlockedIncrement(&state->entries);
  state->on_hook(true, context);
  _InterlockedDecrement(&state->entries);
}

__declspec(noinline) void recv_hook(safetyhook::Context &context) {
  Capture::State *state = g_active.load(std::memory_order_acquire);
  if (state == nullptr)
    return;
  _InterlockedIncrement(&state->entries);
  state->on_hook(false, context);
  _InterlockedDecrement(&state->entries);
}

#ifndef PKT3663_HOOK_SCOPE
#define PKT3663_HOOK_SCOPE() ((void)0)
#endif

SlotCode install_slot(safetyhook::MidHook &hook, void *entry,
                      safetyhook::MidHookFn function, HookRoute &route,
                      bool &noted, uint8_t (&patch)[kMaxTargetPatch],
                      uint8_t &patch_size) {
  PKT3663_HOOK_SCOPE();
  noted = false;
  route = {};
  patch_size = 0;
  auto created = safetyhook::MidHook::create(
      entry, function, safetyhook::MidHook::StartDisabled);
  if (!created)
    return SlotCode::FailedClosed;
  hook = std::move(*created);
  const auto &original = hook.original_bytes();
  if (hook.target() != entry || hook.destination() != function ||
      original.empty() || original.size() > kMaxTargetPatch ||
      original.size() > UINT8_MAX) {
    hook.reset();
    return SlotCode::FailedClosed;
  }
  if (!hook.enable()) {
    hook.reset();
    return SlotCode::FailedClosed;
  }
  HookRoute installed{};
  if (!note_route(entry, installed) ||
      installed.callback != reinterpret_cast<uintptr_t>(function) ||
      installed.callback != reinterpret_cast<uintptr_t>(hook.destination()) ||
      !snapshot_target_patch(hook, entry, patch, patch_size)) {
    return SlotCode::FailedLive;
  }
  route = installed;
  noted = true;
  return SlotCode::Ok;
}

void fill_ranges(const Capture::State &state, Range *ranges, int &count) {
  count = 0;
  ranges[count++] = state.send_callback;
  ranges[count++] = state.recv_callback;
  if (state.send_extra)
    ranges[count++] = state.send_callback_extra;
  if (state.recv_extra)
    ranges[count++] = state.recv_callback_extra;
  if (state.recv_noted) {
    ranges[count++] = state.recv_route.stub_region;
    ranges[count++] = state.recv_route.trampoline_region;
  }
  if (state.send_noted) {
    ranges[count++] = state.send_route.stub_region;
    ranges[count++] = state.send_route.trampoline_region;
  }
}

bool quiesce(Capture::State &state) {
  const bool receive_enabled = state.recv_hook && state.recv_hook.enabled();
  const bool send_enabled = state.send_hook && state.send_hook.enabled();
  // Prove every live slot before restoring either target. If one slot has
  // lost ownership, Retained must leave the complete installed set alone.
  if (receive_enabled) {
    if (!installed_slot_intact(state.recv_hook, state.recv_hook.target(),
                               state.recv_route, state.recv_noted,
                               state.recv_patch, state.recv_patch_size)) {
      return false;
    }
  }
  if (send_enabled) {
    if (!installed_slot_intact(state.send_hook, state.send_hook.target(),
                               state.send_route, state.send_noted,
                               state.send_patch, state.send_patch_size)) {
      return false;
    }
  }
  if (receive_enabled) {
    if (!state.recv_hook.disable())
      return false;
    if (entry_still_patched(state.recv_hook))
      return false;
  }
  if (send_enabled) {
    if (!state.send_hook.disable())
      return false;
    if (entry_still_patched(state.send_hook))
      return false;
  }

  const ULONGLONG deadline = GetTickCount64() + kQuiesceMs;
  for (;;) {
    bool quiet = false;
    {
      PausedThreads paused = PausedThreads::suspend_others();
      if (paused.complete && state.entries == 0) {
        Range ranges[8] = {};
        int count = 0;
        fill_ranges(state, ranges, count);
        quiet = !paused.any_rip(ranges, count);
      }
    }
    if (quiet)
      return true;
    if (GetTickCount64() >= deadline)
      return false;
    YieldProcessor();
  }
}

bool destroy_hooks(Capture::State &state) {
  if (!quiesce(state))
    return false;
  state.recv_hook.reset();
  state.send_hook.reset();
  state.recv_route = {};
  state.send_route = {};
  state.recv_noted = false;
  state.send_noted = false;
  std::memset(state.recv_patch, 0, sizeof state.recv_patch);
  std::memset(state.send_patch, 0, sizeof state.send_patch);
  state.recv_patch_size = 0;
  state.send_patch_size = 0;
  return true;
}

bool hook_present(const safetyhook::MidHook &hook) {
  return static_cast<bool>(hook);
}

bool installed_slot_intact(const safetyhook::MidHook &hook, void *target,
                           const HookRoute &expected_route, bool noted,
                           const uint8_t *expected_patch,
                           uint8_t expected_patch_size) {
  if (!noted || !hook_present(hook) || !hook.enabled() ||
      expected_patch == nullptr || expected_patch_size == 0 ||
      expected_patch_size > kMaxTargetPatch || hook.target() != target ||
      hook.destination() == nullptr ||
      expected_route.callback !=
          reinterpret_cast<uintptr_t>(hook.destination())) {
    return false;
  }
  const auto &original = hook.original_bytes();
  if (original.size() != expected_patch_size)
    return false;
  uint8_t current[kMaxTargetPatch] = {};
  uint8_t current_size = 0;
  if (!snapshot_target_patch(hook, target, current, current_size) ||
      current_size != expected_patch_size ||
      std::memcmp(current, expected_patch, expected_patch_size) != 0) {
    return false;
  }
  HookRoute current_route{};
  return note_route(target, current_route) &&
         same_route(current_route, expected_route);
}

bool entry_still_patched(const safetyhook::MidHook &hook) {
  if (!hook_present(hook) || hook.target() == nullptr)
    return false;
  if (hook.enabled())
    return true;
  const auto &original = hook.original_bytes();
  if (original.empty())
    return false;
  __try {
    return std::memcmp(hook.target(), original.data(), original.size()) != 0;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return true;
  }
}

bool retire_installed_hooks(Capture::State &state) {
  const bool recv = hook_present(state.recv_hook);
  const bool send = hook_present(state.send_hook);
  if (!recv && !send)
    return true;
  if ((recv && entry_still_patched(state.recv_hook) &&
       !state.recv_hook.enabled()) ||
      (send && entry_still_patched(state.send_hook) &&
       !state.send_hook.enabled())) {
    return false;
  }
  if ((state.recv_hook.enabled() &&
       !installed_slot_intact(state.recv_hook, state.recv_hook.target(),
                              state.recv_route, state.recv_noted,
                              state.recv_patch, state.recv_patch_size)) ||
      (state.send_hook.enabled() &&
       !installed_slot_intact(state.send_hook, state.send_hook.target(),
                              state.send_route, state.send_noted,
                              state.send_patch, state.send_patch_size))) {
    return false;
  }
  return destroy_hooks(state);
}

void close_file(Capture::State &state) {
  if (state.file != nullptr) {
    std::fclose(state.file);
    state.file = nullptr;
  }
}

} // namespace

} // namespace pkt3663
