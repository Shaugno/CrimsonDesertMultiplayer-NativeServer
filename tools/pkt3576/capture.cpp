#include "pkt3576/capture.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>

#include "safetyhook/mid_hook.hpp"

namespace pkt3576 {
namespace {

// SafetyHook v0.7.0, as built with third_party/safetyhook-patch:
// InlineHook::disable restores the target bytes inside trap_threads. That trap
// covers the stolen-byte window only. InlineHook::destroy then frees the
// trampoline. MidHook::~MidHook also frees its stub. A thread inside the mid-hook
// destination, or in the stub outside that window, is not waited on. InlineHook::call
// holds the hook mutex across the trampoline; the mid-hook stub does not use it.
// Removal here disables first, then frees only after every other thread is
// suspended outside the callback, the stub, and the trampoline.

constexpr uint32_t kQuiesceMs = 2000;
constexpr size_t kMaxHookRegion = 1024 * 1024;
constexpr size_t kMaxCallbackSpan = 65536;
// safetyhook v0.7.0 mid-hook stub (include/used by src/mid_hook.cpp asm_data).
constexpr size_t kMidStubBytes = 391;

struct Range {
    uintptr_t begin = 0;
    uintptr_t end = 0;

    bool contains(uintptr_t ip) const { return ip >= begin && ip < end; }
};

// x64 unwind data, not a linker-ordered section. The range is the function
// that SafetyHook calls, including the prologue before the entry count and
// the epilogue after it. Callees are covered by that count instead.
bool function_extent(const void* address, Range& out) {
    DWORD64 image_base = 0;
    const PRUNTIME_FUNCTION entry =
        RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(address), &image_base, nullptr);
    if (entry == nullptr || entry->EndAddress <= entry->BeginAddress) return false;
    const auto begin = static_cast<uintptr_t>(image_base + entry->BeginAddress);
    const auto end = static_cast<uintptr_t>(image_base + entry->EndAddress);
    if (end <= begin || end - begin > kMaxCallbackSpan) return false;
    out.begin = begin;
    out.end = end;
    return true;
}

bool follows_e9(const void* address, void*& target) {
    const auto* bytes = static_cast<const uint8_t*>(address);
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
    if (!jump) return false;
    target = const_cast<uint8_t*>(bytes) + 5 + relative;
    return target != address;
}

bool collect_callback(const void* symbol, Range& primary, Range& extra, bool& have_extra) {
    have_extra = false;
    if (!function_extent(symbol, primary)) return false;
    void* body = nullptr;
    if (!follows_e9(symbol, body)) return true;
    if (!function_extent(body, extra)) return false;
    have_extra = extra.begin != primary.begin || extra.end != primary.end;
    return true;
}

bool region_of(const void* address, Range& out) {
    if (address == nullptr) return false;
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(address, &info, sizeof info) == 0) return false;
    if (info.State != MEM_COMMIT || info.RegionSize == 0 || info.RegionSize > kMaxHookRegion) return false;
    out.begin = reinterpret_cast<uintptr_t>(info.BaseAddress);
    out.end = out.begin + info.RegionSize;
    return out.contains(reinterpret_cast<uintptr_t>(address));
}

bool read_u64(const void* address, uint64_t& out) {
    std::memcpy(&out, address, sizeof out);
    return true;
}

bool note_ranges(void* target, Range& stub, Range& trampoline) {
    auto* bytes = static_cast<uint8_t*>(target);
    uint8_t* stub_code = nullptr;
    uint8_t* tramp_code = nullptr;
    __try {
        if (bytes[0] == 0xE9) {
            int32_t relative = 0;
            std::memcpy(&relative, bytes + 1, sizeof relative);
            uint8_t* jump = bytes + 5 + relative;
            if (jump[0] != 0xFF || jump[1] != 0x25) return false;
            int32_t displacement = 0;
            std::memcpy(&displacement, jump + 2, sizeof displacement);
            uint64_t destination = 0;
            if (!read_u64(jump + 6 + displacement, destination)) return false;
            stub_code = reinterpret_cast<uint8_t*>(destination);
            tramp_code = jump;
        } else if (bytes[0] == 0xFF && bytes[1] == 0x25) {
            int32_t displacement = 0;
            std::memcpy(&displacement, bytes + 2, sizeof displacement);
            uint64_t destination = 0;
            if (!read_u64(bytes + 6 + displacement, destination)) return false;
            stub_code = reinterpret_cast<uint8_t*>(destination);
            uint64_t trampoline_address = 0;
            if (!read_u64(stub_code + kMidStubBytes - 8, trampoline_address)) return false;
            tramp_code = reinterpret_cast<uint8_t*>(trampoline_address);
        } else {
            return false;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return region_of(stub_code, stub) && region_of(tramp_code, trampoline);
}

bool in_module(void* module, const void* address) {
    if (module == nullptr || address == nullptr) return false;
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(address, &info, sizeof info) == 0) return false;
    return info.State == MEM_COMMIT && info.AllocationBase == module;
}

bool read_thunk(uint64_t address, void* destination, size_t size, void*) {
    return guarded_read(address, destination, size);
}

struct PausedThreads {
    HANDLE handles[256]{};
    int count = 0;
    bool complete = false;
    bool suspended = false;

    PausedThreads() = default;
    PausedThreads(const PausedThreads&) = delete;
    PausedThreads& operator=(const PausedThreads&) = delete;
    PausedThreads(PausedThreads&& other) noexcept { *this = std::move(other); }
    PausedThreads& operator=(PausedThreads&& other) noexcept {
        if (this != &other) {
            resume();
            count = other.count;
            complete = other.complete;
            suspended = other.suspended;
            for (int index = 0; index < count; ++index) handles[index] = other.handles[index];
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
        if (snapshot == INVALID_HANDLE_VALUE) return paused;

        THREADENTRY32 entry{};
        entry.dwSize = sizeof entry;
        const DWORD pid = GetCurrentProcessId();
        const DWORD self = GetCurrentThreadId();
        bool failed = false;
        if (Thread32First(snapshot, &entry) == FALSE) {
            const DWORD error = GetLastError();
            CloseHandle(snapshot);
            // ERROR_NO_MORE_FILES is an empty snapshot. Any other failure
            // leaves the scan incomplete, so the caller must not free hooks.
            if (error != ERROR_NO_MORE_FILES) return paused;
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
                const HANDLE handle = OpenThread(
                    THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE,
                    entry.th32ThreadID);
                if (handle == nullptr) {
                    // The id died between the snapshot and the open. A thread
                    // that no longer exists cannot be inside the hook. Advance
                    // to the next entry; retrying this one never reaches it.
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
                if (GetLastError() != ERROR_NO_MORE_FILES) failed = true;
                break;
            }
        }
        CloseHandle(snapshot);
        if (failed) {
            for (int index = 0; index < paused.count; ++index) CloseHandle(paused.handles[index]);
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
            const bool exited = GetExitCodeThread(paused.handles[index], &code) != 0 && code != STILL_ACTIVE;
            if (exited) {
                CloseHandle(paused.handles[index]);
                paused.handles[index] = paused.handles[paused.count - 1];
                --paused.count;
                continue;
            }
            for (int done = 0; done < index; ++done) ResumeThread(paused.handles[done]);
            for (int handle = 0; handle < paused.count; ++handle) CloseHandle(paused.handles[handle]);
            paused.count = 0;
            return paused;
        }
        paused.suspended = true;
        paused.complete = true;
        return paused;
    }

    void resume() {
        if (suspended) {
            for (int index = 0; index < count; ++index) ResumeThread(handles[index]);
            suspended = false;
        }
        for (int index = 0; index < count; ++index) CloseHandle(handles[index]);
        count = 0;
        complete = false;
    }

    bool any_rip(const Range* ranges, int range_count) const {
        for (int index = 0; index < count; ++index) {
            CONTEXT context{};
            context.ContextFlags = CONTEXT_CONTROL;
            if (GetThreadContext(handles[index], &context) == 0) return true;
            for (int range = 0; range < range_count; ++range) {
                if (ranges[range].contains(static_cast<uintptr_t>(context.Rip))) return true;
            }
        }
        return false;
    }
};

void append_hex(char* destination, size_t capacity, size_t& used, uint64_t value, int digits) {
    char scratch[32] = {};
    std::snprintf(scratch, sizeof scratch, "%0*llX", digits, static_cast<unsigned long long>(value));
    const size_t length = std::strlen(scratch);
    if (used + length >= capacity) return;
    std::memcpy(destination + used, scratch, length);
    used += length;
}

void append_text(char* destination, size_t capacity, size_t& used, const char* text) {
    const size_t length = std::strlen(text);
    if (used + length >= capacity) return;
    std::memcpy(destination + used, text, length);
    used += length;
}

}  // namespace

struct Capture::State {
    CaptureLimits limits{};
    wchar_t path[260] = {};
    bool have_path = false;
    Observer* observer = nullptr;
    FILE* file = nullptr;
    safetyhook::MidHook recv_hook{};
    safetyhook::MidHook send_hook{};
    Range recv_stub{};
    Range recv_trampoline{};
    Range send_stub{};
    Range send_trampoline{};
    bool recv_noted = false;
    bool send_noted = false;
    Range send_callback{};
    Range recv_callback{};
    Range send_callback_extra{};
    Range recv_callback_extra{};
    bool send_extra = false;
    bool recv_extra = false;
    std::vector<Record> records;
    std::thread writer;
    std::atomic<uint32_t> stored{0};
    std::atomic<uint32_t> omitted{0};
    std::atomic<bool> writer_blocked{false};
    std::atomic<bool> writer_stop{false};
    volatile long entries = 0;
    bool running = false;
    bool release_blocked = false;

    void on_hook(bool send, const safetyhook::Context& context) {
        if (observer == nullptr) return;
        EntryArgs args;
        args.rcx = context.rcx;
        args.rdx = context.rdx;
        args.r8 = context.r8;
        args.r9 = context.r9;
        args.rsp = context.rsp;
        if (send) observer->on_send(args);
        else observer->on_recv(args);
    }

    void publish_record(const Record& record) {
        if (stored.load(std::memory_order_relaxed) >= limits.output_records) {
            omitted.fetch_add(1, std::memory_order_relaxed);
        } else {
            records.push_back(record);
            stored.store(static_cast<uint32_t>(records.size()), std::memory_order_release);
            write_line(record);
        }
    }

    void write_line(const Record& record) {
        if (file == nullptr) return;
        char line[384] = {};
        size_t used = 0;
        append_text(line, sizeof line, used, record.direction == Direction::Send ? "S " : "R ");
        append_hex(line, sizeof line, used, record.rcx, 16);
        append_text(line, sizeof line, used, " ");
        append_hex(line, sizeof line, used, record.rdx, 16);
        append_text(line, sizeof line, used, " ");
        append_hex(line, sizeof line, used, record.r8, 16);
        append_text(line, sizeof line, used, " ");
        append_hex(line, sizeof line, used, record.r9, 16);
        append_text(line, sizeof line, used, " ");
        append_hex(line, sizeof line, used, record.stack_u16_28, 4);
        if (record.direction == Direction::Send) {
            append_text(line, sizeof line, used, " ");
            append_hex(line, sizeof line, used, record.stack_u8_30, 2);
            append_text(line, sizeof line, used, " ");
            append_hex(line, sizeof line, used, record.stack_u16_38, 4);
        }
        append_text(line, sizeof line, used, " ");
        for (uint8_t index = 0; index < record.prefix_len; ++index) {
            append_hex(line, sizeof line, used, record.prefix[index], 2);
        }
        append_text(line, sizeof line, used, "\n");
        if (used < sizeof line) line[used] = '\0';
        std::fputs(line, file);
    }

    void write_summary() {
        if (file == nullptr || observer == nullptr) return;
        std::fprintf(file, "counters dropped=%llu unread=%llu stored=%u omitted=%u\n",
                     static_cast<unsigned long long>(observer->dropped()),
                     static_cast<unsigned long long>(observer->unread()),
                     stored.load(std::memory_order_relaxed),
                     omitted.load(std::memory_order_relaxed));
    }
};

namespace {

std::atomic<Capture::State*> g_active{nullptr};

enum class SlotCode { Ok, FailedClosed, FailedLive };

__declspec(noinline) void send_hook(safetyhook::Context& context) {
    Capture::State* state = g_active.load(std::memory_order_acquire);
    if (state == nullptr) return;
    _InterlockedIncrement(&state->entries);
    state->on_hook(true, context);
    _InterlockedDecrement(&state->entries);
}

__declspec(noinline) void recv_hook(safetyhook::Context& context) {
    Capture::State* state = g_active.load(std::memory_order_acquire);
    if (state == nullptr) return;
    _InterlockedIncrement(&state->entries);
    state->on_hook(false, context);
    _InterlockedDecrement(&state->entries);
}

// The fault executable defines this to observe each factory call. The normal
// capture library leaves it empty.
#ifndef PKT3576_HOOK_SCOPE
#define PKT3576_HOOK_SCOPE() ((void)0)
#endif

SlotCode install_slot(safetyhook::MidHook& hook, void* entry, safetyhook::MidHookFn function, Range& stub,
                      Range& trampoline, bool& noted) {
    PKT3576_HOOK_SCOPE();
    // MidHook::create and enable allocate and can throw. The object is stored
    // only after create returns, so a throw inside the factory does not leave
    // this slot installed. A throw from enable can leave it installed.
    auto created = safetyhook::MidHook::create(entry, function, safetyhook::MidHook::StartDisabled);
    if (!created) return SlotCode::FailedClosed;
    hook = std::move(*created);
    if (!hook.enable()) {
        hook.reset();
        return SlotCode::FailedClosed;
    }
    if (!note_ranges(entry, stub, trampoline)) {
        noted = false;
        return SlotCode::FailedLive;
    }
    noted = true;
    return SlotCode::Ok;
}

void fill_ranges(const Capture::State& state, Range* ranges, int& count) {
    count = 0;
    ranges[count++] = state.send_callback;
    ranges[count++] = state.recv_callback;
    if (state.send_extra) ranges[count++] = state.send_callback_extra;
    if (state.recv_extra) ranges[count++] = state.recv_callback_extra;
    if (state.recv_noted) {
        ranges[count++] = state.recv_stub;
        ranges[count++] = state.recv_trampoline;
    }
    if (state.send_noted) {
        ranges[count++] = state.send_stub;
        ranges[count++] = state.send_trampoline;
    }
}

bool quiesce(Capture::State& state) {
    if (state.recv_hook && state.recv_hook.enabled()) {
        if (!state.recv_hook.disable()) return false;
    }
    if (state.send_hook && state.send_hook.enabled()) {
        if (!state.send_hook.disable()) return false;
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
        if (quiet) return true;
        if (GetTickCount64() >= deadline) return false;
        YieldProcessor();
    }
}

bool destroy_hooks(Capture::State& state) {
    if (!quiesce(state)) return false;
    state.recv_hook.reset();
    state.send_hook.reset();
    state.recv_noted = false;
    state.send_noted = false;
    return true;
}

bool hook_present(const safetyhook::MidHook& hook) { return static_cast<bool>(hook); }

// True when the target bytes are no longer the bytes SafetyHook saved, or the
// hook reports itself enabled. disable() restores bytes only when enabled()
// is true, so a patched-but-disabled hook cannot be cleared here.
bool entry_still_patched(const safetyhook::MidHook& hook) {
    if (!hook_present(hook) || hook.target() == nullptr) return false;
    if (hook.enabled()) return true;
    const auto& original = hook.original_bytes();
    if (original.empty()) return false;
    return std::memcmp(hook.target(), original.data(), original.size()) != 0;
}

// Disable, quiesce, and reset every installed hook. Returns false when a hook
// is live but its stub range is unknown, the bytes are patched while SafetyHook
// will not restore them, or quiesce cannot finish. The caller then retains.
bool retire_installed_hooks(Capture::State& state) {
    const bool recv = hook_present(state.recv_hook);
    const bool send = hook_present(state.send_hook);
    if (!recv && !send) return true;
    if ((recv && entry_still_patched(state.recv_hook) && !state.recv_hook.enabled()) ||
        (send && entry_still_patched(state.send_hook) && !state.send_hook.enabled())) {
        return false;
    }
    if ((state.recv_hook.enabled() && !state.recv_noted) || (state.send_hook.enabled() && !state.send_noted)) {
        return false;
    }
    return destroy_hooks(state);
}

void close_file(Capture::State& state) {
    if (state.file != nullptr) {
        std::fclose(state.file);
        state.file = nullptr;
    }
}

}  // namespace

Capture::Capture(State* state) : state_(state) {}

Capture::~Capture() {
    if (state_ == nullptr) return;
    close_file(*state_);
    if (!state_->release_blocked) Observer::release_owner(state_->observer);
    delete state_;
    state_ = nullptr;
}

Capture* Capture::create(const CaptureLimits& limits, const wchar_t* output_path) {
    auto* state = new (std::nothrow) State();
    if (state == nullptr) return nullptr;
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
    auto* capture = new (std::nothrow) Capture(state);
    if (capture == nullptr) {
        Observer::release_owner(state->observer);
        delete state;
        return nullptr;
    }
    return capture;
}

bool Capture::release_owner(Capture*& capture) {
    if (capture == nullptr) return true;
    if (capture->state_ != nullptr && capture->state_->release_blocked) return false;
    if (capture->state_ != nullptr && capture->state_->running) {
        if (capture->stop() != StopResult::Removed) return false;
    }
    delete capture;
    capture = nullptr;
    return true;
}

void Capture::set_writer_blocked(bool blocked) {
    if (state_ == nullptr) return;
    state_->writer_blocked.store(blocked, std::memory_order_release);
}

uint32_t Capture::stored() const {
    return state_ == nullptr ? 0 : state_->stored.load(std::memory_order_acquire);
}

uint32_t Capture::omitted() const {
    return state_ == nullptr ? 0 : state_->omitted.load(std::memory_order_acquire);
}

uint64_t Capture::dropped() const {
    return state_ == nullptr || state_->observer == nullptr ? 0 : state_->observer->dropped();
}

uint64_t Capture::unread() const {
    return state_ == nullptr || state_->observer == nullptr ? 0 : state_->observer->unread();
}

bool Capture::queue_allocated() const {
    return state_ != nullptr && state_->observer != nullptr && state_->observer->queue_allocated();
}

bool Capture::accepting() const {
    return state_ != nullptr && state_->observer != nullptr && state_->observer->accepting();
}

bool Capture::copy_record(uint32_t index, Record& out) const {
    if (state_ == nullptr || index >= state_->records.size()) return false;
    out = state_->records[index];
    return true;
}

Capture::StartResult Capture::start(void* module, void* send_entry, void* recv_entry) {
    if (state_ == nullptr || state_->running || state_->release_blocked) return StartResult::Refused;
    if (state_->limits.queue_capacity < kMinQueueCapacity || state_->limits.output_records < 1) {
        return StartResult::Refused;
    }
    if (send_entry == nullptr || recv_entry == nullptr || send_entry == recv_entry) return StartResult::Refused;
    if (!in_module(module, send_entry) || !in_module(module, recv_entry)) return StartResult::Refused;
    if (!collect_callback(&send_hook, state_->send_callback, state_->send_callback_extra, state_->send_extra) ||
        !collect_callback(&recv_hook, state_->recv_callback, state_->recv_callback_extra, state_->recv_extra)) {
        return StartResult::Refused;
    }
    const auto send_address = reinterpret_cast<uintptr_t>(send_entry);
    const auto recv_address = reinterpret_cast<uintptr_t>(recv_entry);
    const bool overlaps = state_->send_callback.contains(send_address) || state_->send_callback.contains(recv_address) ||
                          state_->recv_callback.contains(send_address) || state_->recv_callback.contains(recv_address) ||
                          (state_->send_extra && (state_->send_callback_extra.contains(send_address) ||
                                                  state_->send_callback_extra.contains(recv_address))) ||
                          (state_->recv_extra && (state_->recv_callback_extra.contains(send_address) ||
                                                  state_->recv_callback_extra.contains(recv_address)));
    if (overlaps) return StartResult::Refused;

    // Fallible storage is reserved before global ownership is claimed. A
    // throw here must not leave g_active pointing at this object.
    try {
        state_->records.reserve(state_->limits.output_records);
    } catch (const std::bad_alloc&) {
        return StartResult::Refused;
    }

    Capture::State* expected = nullptr;
    if (!g_active.compare_exchange_strong(expected, state_, std::memory_order_acq_rel, std::memory_order_acquire)) {
        return StartResult::Refused;
    }

    // Cleans file, ring, and g_active only after every installed hook has been
    // disabled and quiesced. A hook that cannot be removed stays owned.
    struct PreHookRollback {
        Capture::State* state = nullptr;
        bool active = true;
        bool armed = false;
        ~PreHookRollback() {
            if (!active || state == nullptr) return;
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
            if (armed && state->observer != nullptr) state->observer->disarm();
            close_file(*state);
            g_active.store(nullptr, std::memory_order_release);
        }
    } rollback{state_};

    if (state_->have_path) {
        FILE* file = nullptr;
        if (_wfopen_s(&file, state_->path, L"wb") != 0 || file == nullptr) return StartResult::Refused;
        state_->file = file;
    }

    state_->records.clear();
    state_->stored.store(0, std::memory_order_relaxed);
    state_->omitted.store(0, std::memory_order_relaxed);
    state_->writer_stop.store(false, std::memory_order_relaxed);
    state_->entries = 0;

    if (!state_->observer->arm(&read_thunk, nullptr)) return StartResult::Refused;
    rollback.armed = true;

    auto retain = [&]() {
        rollback.active = false;
        state_->release_blocked = true;
        return StartResult::Retained;
    };
    // Factory and enable allocate. Either call can throw after the receive
    // hook is already live. Retire that hook before the rollback clears
    // ownership. If retire cannot finish, keep the hook, the queue, and g_active.
    try {
        const SlotCode recv_code = install_slot(state_->recv_hook, recv_entry, &recv_hook, state_->recv_stub,
                                                 state_->recv_trampoline, state_->recv_noted);
        if (recv_code != SlotCode::Ok) {
            if (recv_code == SlotCode::FailedLive) return retain();
            return StartResult::Refused;
        }

        const SlotCode send_code = install_slot(state_->send_hook, send_entry, &send_hook, state_->send_stub,
                                                 state_->send_trampoline, state_->send_noted);
        if (send_code != SlotCode::Ok) {
            if (send_code == SlotCode::FailedLive || !retire_installed_hooks(*state_)) return retain();
            return StartResult::RolledBack;
        }
    } catch (...) {
        const bool installed = hook_present(state_->recv_hook) || hook_present(state_->send_hook);
        bool retired = false;
        try {
            retired = retire_installed_hooks(*state_);
        } catch (...) {
            retired = false;
        }
        if (!retired) return retain();
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

    rollback.active = false;
    state_->running = true;
    return StartResult::Started;
}

Capture::StopResult Capture::stop() {
    if (state_ == nullptr) return StopResult::Idle;
    if (state_->release_blocked) return StopResult::Retained;
    if (!state_->running) return StopResult::Idle;
    if (!destroy_hooks(*state_)) {
        state_->release_blocked = true;
        state_->running = false;
        return StopResult::Retained;
    }
    g_active.store(nullptr, std::memory_order_release);
    state_->writer_blocked.store(false, std::memory_order_release);
    state_->writer_stop.store(true, std::memory_order_release);
    if (state_->writer.joinable()) state_->writer.join();
    if (state_->observer != nullptr) state_->observer->disarm();
    close_file(*state_);
    state_->running = false;
    return StopResult::Removed;
}

void Capture::writer_main() {
    if (state_ == nullptr) return;
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
        if (state_->writer_stop.load(std::memory_order_acquire)) break;
        YieldProcessor();
    }
    state_->write_summary();
    if (state_->file != nullptr) std::fflush(state_->file);
}

}  // namespace pkt3576
