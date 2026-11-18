#include "mod/session2/crash3663.h"
#include "mod/game/fault_line.h"

#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>

#include <array>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <string>

namespace cdmp::session2::crash3663 {
namespace {
constexpr uint32_t kPackets = 131072;
constexpr uint32_t kTextBytes = 65536;
constexpr uint32_t kFirstChance = 24;
struct PacketRow {
    uint64_t utc_filetime;
    uint16_t opcode, bytes;
    uint8_t direction, returned;
    uint16_t reserved;
};
struct PacketTrace {
    char magic[8];
    uint32_t capacity, count, truncated, pid;
    PacketRow rows[kPackets];
};
static_assert(sizeof(PacketRow) == 16 && offsetof(PacketTrace, rows) == 24);
PacketTrace* trace{};
char* crash_text{};
std::atomic<uint32_t> text_used{}, first_chance{};
std::atomic<bool> armed{}, dump_requested{};
std::atomic<LPTOP_LEVEL_EXCEPTION_FILTER> previous_filter{};
thread_local bool in_filter{};

struct Module { uintptr_t base{}; DWORD size{}; char name[96]{}; };
std::array<Module, 256> modules;
size_t module_count{};

decltype(&MiniDumpWriteDump) write_minidump{};
HANDLE dump_go{}, dump_done{};
std::wstring dump_path;
DWORD dump_tid{};
EXCEPTION_RECORD dump_record{};
CONTEXT dump_context{};

uint64_t utc_filetime() noexcept {
    FILETIME time{};
    GetSystemTimePreciseAsFileTime(&time);
    return (static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
}

void* create_mapping(const std::wstring& path, DWORD bytes) noexcept {
    const HANDLE file = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                                   nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return nullptr;
    const HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READWRITE, 0, bytes, nullptr);
    CloseHandle(file);
    if (!mapping) return nullptr;
    void* view = MapViewOfFile(mapping, FILE_MAP_WRITE, 0, 0, bytes);
    CloseHandle(mapping);
    if (view) std::memset(view, 0, bytes);
    return view;
}

void snapshot_modules() noexcept {
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (snapshot == INVALID_HANDLE_VALUE) return;
    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Module32FirstW(snapshot, &entry)) {
        do {
            auto& mod = modules[module_count++];
            mod.base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
            mod.size = entry.modBaseSize;
            WideCharToMultiByte(CP_UTF8, 0, entry.szModule, -1, mod.name, sizeof mod.name, nullptr, nullptr);
        } while (module_count < modules.size() && Module32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

void append(game::FixedLine& line) noexcept {
    line.str("\n");
    const auto text = line.view();
    const auto size = static_cast<uint32_t>(text.size());
    const uint32_t offset = text_used.fetch_add(size, std::memory_order_relaxed);
    if (offset <= kTextBytes && size <= kTextBytes - offset) {
        std::memcpy(crash_text + offset, text.data(), size);
    }
}

void location(game::FixedLine& line, uintptr_t address) noexcept {
    MEMORY_BASIC_INFORMATION info{};
    if (VirtualQuery(reinterpret_cast<const void*>(address), &info, sizeof info) && info.Type == MEM_IMAGE) {
        const auto base = reinterpret_cast<uintptr_t>(info.AllocationBase);
        for (size_t index = 0; index < module_count; ++index) {
            const auto& mod = modules[index];
            if (base == mod.base && address - base < mod.size && mod.name[0]) {
                line.str(mod.name).str("+").hex(address - base);
                return;
            }
        }
        line.str("image@").hex(base).str("+").hex(address - base);
        return;
    }
    line.hex(address);
}

bool unwind(CONTEXT& context, uintptr_t low, uintptr_t high) noexcept {
    if (context.Rsp < low || context.Rsp > high - sizeof(uint64_t)) return false;
    const auto previous_rsp = context.Rsp;
    __try {
        DWORD64 image{};
        const auto function = RtlLookupFunctionEntry(context.Rip, &image, nullptr);
        if (function) {
            void* handler_data{};
            DWORD64 frame{};
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, image, context.Rip, function, &context,
                             &handler_data, &frame, nullptr);
        } else {
            // x64 leaf frame. Stop on an invalid next instruction address.
            context.Rip = *reinterpret_cast<const uint64_t*>(context.Rsp);
            context.Rsp += sizeof(uint64_t);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    MEMORY_BASIC_INFORMATION next{};
    return context.Rsp > previous_rsp && context.Rsp <= high &&
        VirtualQuery(reinterpret_cast<const void*>(context.Rip), &next, sizeof next) &&
        next.State == MEM_COMMIT &&
        (next.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) &&
        !(next.Protect & PAGE_GUARD);
}

void capture(EXCEPTION_POINTERS* exception, const char* stage) noexcept {
    const auto& record = *exception->ExceptionRecord;
    const auto& context = *exception->ContextRecord;
    const auto tid = GetCurrentThreadId();
    const auto tick = utc_filetime();
    game::FixedLine fault;
    fault.str(stage).str(" utc_filetime=").dec(tick).str(" tid=").dec(tid)
        .str(" code=").hex32(record.ExceptionCode).str(" at=");
    location(fault, reinterpret_cast<uintptr_t>(record.ExceptionAddress));
    if ((record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION || record.ExceptionCode == EXCEPTION_IN_PAGE_ERROR) &&
        record.NumberParameters >= 2) {
        fault.str(" access=").dec(record.ExceptionInformation[0]).str(" address=").hex(record.ExceptionInformation[1]);
    }
    append(fault);

    game::FixedLine registers;
    registers.str("registers tid=").dec(tid).str(" rip=").hex(context.Rip)
        .str(" rsp=").hex(context.Rsp).str(" rbp=").hex(context.Rbp).str(" rax=").hex(context.Rax)
        .str(" rbx=").hex(context.Rbx).str(" rcx=").hex(context.Rcx).str(" rdx=").hex(context.Rdx)
        .str(" rsi=").hex(context.Rsi).str(" rdi=").hex(context.Rdi).str(" r8=").hex(context.R8)
        .str(" r9=").hex(context.R9).str(" r10=").hex(context.R10).str(" r11=").hex(context.R11)
        .str(" r12=").hex(context.R12).str(" r13=").hex(context.R13).str(" r14=").hex(context.R14)
        .str(" r15=").hex(context.R15).str(" eflags=").hex32(context.EFlags);
    append(registers);

    game::FixedLine stack;
    stack.str("stack tid=").dec(tid).str(" ");
    CONTEXT frame = context;
    ULONG_PTR low{}, high{};
    GetCurrentThreadStackLimits(&low, &high);
    location(stack, frame.Rip);
    for (unsigned depth = 1; depth < 7 && unwind(frame, low, high); ++depth) {
        stack.str(" <- ");
        location(stack, frame.Rip);
    }
    append(stack);
}

DWORD WINAPI write_dump(void*) noexcept {
    if (WaitForSingleObject(dump_go, INFINITE) != WAIT_OBJECT_0) return 0;
    const HANDLE file = CreateFileW(dump_path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
    BOOL ok = FALSE;
    DWORD error = ERROR_SUCCESS;
    if (file != INVALID_HANDLE_VALUE) {
        EXCEPTION_POINTERS exception{&dump_record, &dump_context};
        MINIDUMP_EXCEPTION_INFORMATION info{dump_tid, &exception, FALSE};
        ok = write_minidump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, nullptr, nullptr);
        if (!ok) error = GetLastError();
        CloseHandle(file);
    } else {
        error = GetLastError();
    }
    game::FixedLine note;
    note.str("minidump=").str(ok ? "written" : "failed").str(" error=").hex(error);
    append(note);
    SetEvent(dump_done);
    return 0;
}

bool has_context(EXCEPTION_POINTERS* exception) noexcept {
    return exception && exception->ExceptionRecord && exception->ContextRecord;
}

LONG WINAPI vectored(EXCEPTION_POINTERS* exception) noexcept {
    if (!armed.load(std::memory_order_acquire) || in_filter || !has_context(exception))
        return EXCEPTION_CONTINUE_SEARCH;
    const auto code = exception->ExceptionRecord->ExceptionCode;
    if ((code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_IN_PAGE_ERROR) ||
        first_chance.fetch_add(1, std::memory_order_relaxed) >= kFirstChance)
        return EXCEPTION_CONTINUE_SEARCH;
    const DWORD error = GetLastError();
    in_filter = true;
    capture(exception, "first_chance");
    in_filter = false;
    SetLastError(error);
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI unhandled(EXCEPTION_POINTERS* exception) noexcept {
    if (in_filter) return EXCEPTION_CONTINUE_SEARCH; // a previous filter may chain back
    const DWORD error = GetLastError();
    in_filter = true;
    if (armed.load(std::memory_order_acquire) && has_context(exception) &&
        exception->ExceptionRecord->ExceptionCode != EXCEPTION_STACK_OVERFLOW &&
        !dump_requested.exchange(true, std::memory_order_relaxed)) {
        capture(exception, "unhandled");
        if (dump_go) {
            dump_tid = GetCurrentThreadId();
            dump_record = *exception->ExceptionRecord;
            dump_record.ExceptionRecord = nullptr;
            dump_context = *exception->ContextRecord;
            SetEvent(dump_go);
            if (WaitForSingleObject(dump_done, 1800) != WAIT_OBJECT_0) {
                game::FixedLine note;
                note.str("minidump=wait_expired");
                append(note);
            }
        }
    }
    SetLastError(error);
    const auto prior = previous_filter.load(std::memory_order_acquire);
    const auto disposition = prior && prior != unhandled ? prior(exception) : EXCEPTION_CONTINUE_SEARCH;
    in_filter = false;
    return disposition;
}

void prepare_dump() noexcept {
    const auto dbghelp = LoadLibraryExW(L"dbghelp.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!dbghelp) return;
    write_minidump = reinterpret_cast<decltype(write_minidump)>(GetProcAddress(dbghelp, "MiniDumpWriteDump"));
    if (write_minidump) {
        dump_go = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        dump_done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (dump_go && dump_done) {
            const auto thread = CreateThread(nullptr, 0, write_dump, nullptr, 0, nullptr);
            if (thread) {
                CloseHandle(thread);
                return;
            }
        }
    }
    if (dump_go) CloseHandle(dump_go);
    if (dump_done) CloseHandle(dump_done);
    dump_go = dump_done = nullptr;
    FreeLibrary(dbghelp);
}
}

bool install(const wchar_t* directory, role::Role process_role) noexcept {
    try {
        const bool guest = process_role == role::Role::Guest;
        if (crash_text || !directory || !*directory || (!guest && process_role != role::Role::Host)) return false;
        const auto root = std::filesystem::path(directory);
        const auto pid = GetCurrentProcessId();
        const auto suffix = L"-" + std::to_wstring(pid);
        const auto stem = (guest ? L"guest-crash" : L"host-crash") + suffix;
        dump_path = (root / (stem + L".dmp")).wstring();
        crash_text = static_cast<char*>(create_mapping((root / (stem + L".log")).wstring(), kTextBytes));
        if (!crash_text) return false;
        if (guest) {
            trace = static_cast<PacketTrace*>(create_mapping((root / (L"guest-packets" + suffix + L".bin")).wstring(),
                                                             sizeof(PacketTrace)));
            if (!trace) return false;
            std::memcpy(trace->magic, "CDMPPKT1", sizeof trace->magic);
            trace->capacity = kPackets;
            trace->pid = pid;
        }
        snapshot_modules();
        if (!AddVectoredExceptionHandler(1, vectored)) return false;
        prepare_dump();
        previous_filter.store(SetUnhandledExceptionFilter(unhandled), std::memory_order_release);
        game::FixedLine note;
        note.str(guest ? "3663 guest diagnostics installed pid=" : "3663 host diagnostics installed pid=").dec(pid)
            .str(" minidump_worker=").str(dump_go ? "ready" : "unavailable");
        append(note);
        return true;
    } catch (...) {
        return false;
    }
}

bool install_for_session(role::Role process_role) {
    // The NT sandbox already verified the frozen backup supplied by the
    // launcher for the guest. The host receives the evidence directory directly.
    const bool guest = process_role == role::Role::Guest;
    const auto variable = guest ? L"CDMP_SAVE_BACKUP" : L"CDMP_HOST_CRASH_DIR";
    const auto capacity = GetEnvironmentVariableW(variable, nullptr, 0);
    if (!capacity) return false;
    std::wstring path(capacity, L'\0');
    const auto length = GetEnvironmentVariableW(variable, path.data(), capacity);
    if (!length || length >= capacity) return false;
    path.resize(length);
    const std::filesystem::path source(path);
    if (!source.is_absolute() || source.filename() != (guest ? L"guest-backup" : L"evidence")) return false;
    const auto directory = guest ? source.parent_path() / L"evidence" : source;
    std::error_code error;
    std::filesystem::create_directory(directory, error);
    if (error || !std::filesystem::is_directory(directory, error)) return false;
    return install(directory.c_str(), process_role);
}

void arm() noexcept {
    if (crash_text) armed.store(true, std::memory_order_release);
}

uint32_t packet(Direction direction, uint16_t opcode, uint16_t bytes) noexcept {
    if (!trace || !armed.load(std::memory_order_acquire)) return 0;
    // The validated native NetworkThread is the only writer. A receive ticket
    // is published before the call so an exception can leave a pending row.
    const uint32_t index = trace->count;
    if (index == kPackets) {
        trace->truncated = 1;
        return 0;
    }
    trace->rows[index] = {utc_filetime(), opcode, bytes, static_cast<uint8_t>(direction), 0, 0};
    trace->count = index + 1;
    return index + 1;
}

void returned(uint32_t ticket) noexcept {
    if (ticket) trace->rows[ticket - 1].returned = 1;
}

void note_null_playback(uint32_t key, uintptr_t vtable, uint32_t host, uint32_t guest,
                        uint64_t selector, const char* identity) noexcept {
    if (!crash_text) return;
    game::FixedLine line;
    line.str("null_playback utc_filetime=").dec(utc_filetime()).str(" tid=").dec(GetCurrentThreadId())
        .str(" site=CrimsonDesert.exe+0x258C8FD actor=").hex32(key)
        .str(" class=").hex(key >> 28).str(" identity=").str(identity)
        .str(" host_key=").hex32(host).str(" guest_key=").hex32(guest)
        .str(" layer=").dec(selector).str(" actor_vtable=");
    location(line, vtable);
    append(line);
}

void note_playback_skip(uint32_t key) noexcept {
    if (!crash_text) return;
    game::FixedLine line;
    line.str("null_playback_skip utc_filetime=").dec(utc_filetime()).str(" actor=").hex32(key)
        .str(" continuation=CrimsonDesert.exe+0x258C99A");
    append(line);
}
}
