// Game-free controller flow. The target is cdmp_pkt3576_live_dummy.exe.
// Nothing here launches or attaches to Crimson Desert.

#include "pkt3576/live_control.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <safetyhook.hpp>
#include <windows.h>
#include <tlhelp32.h>

namespace {

int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

std::wstring beside(const wchar_t* file_name) {
    wchar_t path[1024] = {};
    const DWORD length = GetModuleFileNameW(nullptr, path, 1024);
    if (length == 0 || length >= 1024) return {};
    wchar_t* slash = nullptr;
    for (wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') slash = cursor;
    }
    if (slash == nullptr) return {};
    *(slash + 1) = L'\0';
    return std::wstring(path) + file_name;
}

std::wstring quote(const std::wstring& text) {
    std::wstring out = L"\"";
    for (wchar_t ch : text) {
        if (ch == L'"') out += L'"';
        out += ch;
    }
    out += L'"';
    return out;
}

struct Captured {
    DWORD exit_code = 0xFFFFFFFF;
    std::string output;
    std::map<std::string, std::string> fields;
    bool launched = false;
};

std::map<std::string, std::string> parse_fields(const std::string& text) {
    std::map<std::string, std::string> fields;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const size_t eq = line.find('=');
        if (eq != std::string::npos) fields[line.substr(0, eq)] = line.substr(eq + 1);
        pos = end + 1;
    }
    return fields;
}

Captured run_exe(const std::wstring& exe, const std::wstring& args) {
    Captured captured;
    SECURITY_ATTRIBUTES security = {};
    security.nLength = sizeof security;
    security.bInheritHandle = TRUE;
    HANDLE read_pipe = nullptr;
    HANDLE write_pipe = nullptr;
    if (!CreatePipe(&read_pipe, &write_pipe, &security, 0)) return captured;
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0,
                             nullptr);
    STARTUPINFOW startup = {};
    startup.cb = sizeof startup;
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = nul;
    startup.hStdOutput = write_pipe;
    startup.hStdError = write_pipe;
    PROCESS_INFORMATION process = {};
    std::wstring command = quote(exe) + L" " + args;
    std::vector<wchar_t> mutable_command(command.begin(), command.end());
    mutable_command.push_back(L'\0');
    const BOOL created =
        CreateProcessW(exe.c_str(), mutable_command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
                       &startup, &process);
    CloseHandle(write_pipe);
    if (nul != nullptr && nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!created) {
        CloseHandle(read_pipe);
        std::printf("FAIL CreateProcess %ls error %lu\n", exe.c_str(), GetLastError());
        ++g_failures;
        return captured;
    }
    char buffer[512];
    DWORD got = 0;
    while (ReadFile(read_pipe, buffer, sizeof buffer, &got, nullptr) && got > 0) captured.output.append(buffer, got);
    CloseHandle(read_pipe);
    WaitForSingleObject(process.hProcess, 30000);
    DWORD code = 1;
    GetExitCodeProcess(process.hProcess, &code);
    captured.exit_code = code;
    captured.launched = true;
    captured.fields = parse_fields(captured.output);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return captured;
}

void require_keys(const Captured& captured) {
    const char* keys[] = {"ctl_action",        "ctl_pid",
                          "ctl_identity",       "ctl_rejected_2976",
                          "ctl_gate",           "ctl_modified",
                          "ctl_loaded",         "ctl_start",
                          "ctl_stop",           "ctl_retained",
                          "ctl_stored",         "ctl_omitted",
                          "ctl_capture_attempts", "ctl_unloaded",
                          "ctl_target_alive",   "ctl_note",
                          "ctl_exit"};
    for (const char* key : keys) {
        if (captured.fields.find(key) == captured.fields.end()) {
            std::printf("FAIL missing %s\n%s\n", key, captured.output.c_str());
            ++g_failures;
        }
    }
}

void expect(const Captured& captured, const char* key, const std::string& value) {
    require_keys(captured);
    const auto found = captured.fields.find(key);
    if (found == captured.fields.end() || found->second != value) {
        const char* got = found == captured.fields.end() ? "<missing>" : found->second.c_str();
        std::printf("FAIL %s want %s got %s\n%s\n", key, value.c_str(), got, captured.output.c_str());
        ++g_failures;
    }
}

bool module_mapped(DWORD pid, const wchar_t* file_name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof entry;
    bool found = false;
    if (Module32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, file_name) == 0) found = true;
        } while (!found && Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return found;
}

struct Dummy {
    HANDLE process = nullptr;
    DWORD pid = 0;
    HANDLE event = nullptr;
    int spawned = 0;
    ~Dummy() { close(); }
    void close() {
        if (event != nullptr) {
            SetEvent(event);
            CloseHandle(event);
            event = nullptr;
        }
        if (process != nullptr) {
            if (WaitForSingleObject(process, 15000) != WAIT_OBJECT_0) {
                std::printf("FAIL dummy %lu did not exit; harness is stopping it\n", pid);
                ++g_failures;
                TerminateProcess(process, 1);
                WaitForSingleObject(process, 5000);
            }
            CloseHandle(process);
            process = nullptr;
        }
    }
};

bool spawn_dummy(Dummy& dummy, bool sleepers) {
    SECURITY_ATTRIBUTES security = {};
    security.nLength = sizeof security;
    security.bInheritHandle = TRUE;
    HANDLE read_pipe = nullptr;
    HANDLE write_pipe = nullptr;
    if (!CreatePipe(&read_pipe, &write_pipe, &security, 0)) return false;
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0,
                             nullptr);
    const std::wstring exe = beside(L"cdmp_pkt3576_live_dummy.exe");
    STARTUPINFOW startup = {};
    startup.cb = sizeof startup;
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = nul;
    startup.hStdOutput = write_pipe;
    startup.hStdError = write_pipe;
    PROCESS_INFORMATION process = {};
    std::wstring command = quote(exe) + (sleepers ? L" sleepers" : L"");
    std::vector<wchar_t> mutable_command(command.begin(), command.end());
    mutable_command.push_back(L'\0');
    const BOOL created =
        CreateProcessW(exe.c_str(), mutable_command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
                       &startup, &process);
    CloseHandle(write_pipe);
    if (nul != nullptr && nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!created) {
        CloseHandle(read_pipe);
        std::printf("FAIL dummy CreateProcess %lu\n", GetLastError());
        ++g_failures;
        return false;
    }
    CloseHandle(process.hThread);
    std::string text;
    const DWORD start = GetTickCount();
    while (text.find("ready") == std::string::npos && GetTickCount() - start < 10000) {
        DWORD available = 0;
        if (!PeekNamedPipe(read_pipe, nullptr, 0, nullptr, &available, nullptr)) break;
        if (available == 0) {
            Sleep(10);
            continue;
        }
        char buffer[256];
        DWORD got = 0;
        const DWORD want = available > sizeof buffer ? static_cast<DWORD>(sizeof buffer) : available;
        if (!ReadFile(read_pipe, buffer, want, &got, nullptr) || got == 0) break;
        text.append(buffer, got);
    }
    CloseHandle(read_pipe);
    const std::map<std::string, std::string> fields = parse_fields(text);
    dummy.process = process.hProcess;
    dummy.pid = process.dwProcessId;
    wchar_t event_name[64] = {};
    swprintf_s(event_name, L"Local\\pkt3576-live-dummy-%lu", dummy.pid);
    dummy.event = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, event_name);
    if (fields.find("pid") == fields.end() || fields.find("spawned") == fields.end() ||
        text.find("ready") == std::string::npos) {
        std::printf("FAIL dummy banner:\n%s\n", text.c_str());
        ++g_failures;
        return false;
    }
    dummy.spawned = std::atoi(fields.at("spawned").c_str());
    if (std::to_string(dummy.pid) != fields.at("pid")) {
        std::printf("FAIL dummy pid banner %s process %lu\n", fields.at("pid").c_str(), dummy.pid);
        ++g_failures;
        return false;
    }
    if (dummy.event == nullptr) {
        std::printf("FAIL OpenEvent %lu\n", GetLastError());
        ++g_failures;
        return false;
    }
    return true;
}

std::wstring log_directory(DWORD pid, const wchar_t* suffix) {
    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    return std::wstring(temp) + L"pkt3576-live-ctl-" + std::to_wstring(pid) + L"-" + suffix;
}

std::string read_file(const std::wstring& path) {
    FILE* file = nullptr;
    if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr) return {};
    std::string text;
    char buffer[256];
    size_t count = 0;
    while ((count = std::fread(buffer, 1, sizeof buffer, file)) > 0) text.append(buffer, count);
    std::fclose(file);
    return text;
}

void test_policy() {
    using pkt3576::IdentityKind;
    using pkt3576::control::ControlAction;
    using pkt3576::control::control_allowed;
    using pkt3576::control::dll_file_allowed;
    using pkt3576::control::game_file_name;
    using pkt3576::control::installed_module_control_allowed;
    using pkt3576::control::modification_allowed;
    using pkt3576::control::unload_allowed;
    CHECK(modification_allowed(false, false, IdentityKind::Match, true));
    CHECK(!modification_allowed(false, false, IdentityKind::WrongName, false));
    CHECK(!modification_allowed(false, true, IdentityKind::Match, false));
    CHECK(modification_allowed(true, true, IdentityKind::WrongName, false));
    CHECK(!modification_allowed(true, true, IdentityKind::Match, true));
    CHECK(modification_allowed(true, false, IdentityKind::Match, false));
    CHECK(!modification_allowed(true, false, IdentityKind::WrongName, false));
    CHECK(!modification_allowed(false, false, IdentityKind::SignatureMismatch, true));
    CHECK(installed_module_control_allowed(IdentityKind::SignatureMismatch, true));
    CHECK(!installed_module_control_allowed(IdentityKind::SignatureMismatch, false));
    CHECK(!installed_module_control_allowed(IdentityKind::Match, true));
    CHECK(!installed_module_control_allowed(IdentityKind::WrongName, true));
    CHECK(!installed_module_control_allowed(IdentityKind::WrongTimestamp, true));
    CHECK(!installed_module_control_allowed(IdentityKind::WrongBase, true));
    CHECK(!installed_module_control_allowed(IdentityKind::WrongSize, true));
    CHECK(!installed_module_control_allowed(IdentityKind::Unreadable, true));
    CHECK(control_allowed(false, false, IdentityKind::Match, true, ControlAction::Load, false));
    CHECK(control_allowed(false, false, IdentityKind::Match, true, ControlAction::Start, false));
    CHECK(control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Status, true));
    CHECK(control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Stop, true));
    CHECK(!control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Load, true));
    CHECK(!control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Start, true));
    CHECK(!control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Status, false));
    CHECK(!control_allowed(false, false, IdentityKind::SignatureMismatch, true, ControlAction::Stop, false));
    CHECK(!control_allowed(false, true, IdentityKind::SignatureMismatch, false, ControlAction::Status, true));
    CHECK(!control_allowed(true, true, IdentityKind::SignatureMismatch, true, ControlAction::Status, true));
    const IdentityKind rejected_kinds[] = {IdentityKind::WrongName, IdentityKind::WrongTimestamp, IdentityKind::WrongBase,
                                           IdentityKind::WrongSize, IdentityKind::Unreadable};
    for (IdentityKind kind : rejected_kinds) {
        CHECK(!control_allowed(false, false, kind, true, ControlAction::Load, true));
        CHECK(!control_allowed(false, false, kind, true, ControlAction::Start, true));
        CHECK(!control_allowed(false, false, kind, true, ControlAction::Status, true));
        CHECK(!control_allowed(false, false, kind, true, ControlAction::Stop, true));
        CHECK(!control_allowed(false, false, kind, true, ControlAction::Status, false));
    }
    CHECK(!unload_allowed(kPkt3576LiveStarted, 0, 1));
    CHECK(unload_allowed(kPkt3576LiveRefused, 0, 0));
    CHECK(!unload_allowed(kPkt3576LiveRefused, 1, 0));
    CHECK(!unload_allowed(kPkt3576LiveRolledBack, 0, 0));
    CHECK(unload_allowed(kPkt3576LiveBadLog, 0, 0));
    CHECK(unload_allowed(kPkt3576LiveIdle, 0, 0));
    CHECK(unload_allowed(kPkt3576LiveRemoved, 0, 0));
    CHECK(!unload_allowed(kPkt3576LiveRemoved, 0, 1));
    CHECK(!unload_allowed(kPkt3576LiveRetained, 0, 0));
    CHECK(!unload_allowed(-1, 0, 1));
    CHECK(dll_file_allowed(L"C:\\abs\\cdmp_pkt3576_live.dll", false));
    CHECK(!dll_file_allowed(L"C:\\abs\\cdmp_pkt3576_live_fixture.dll", false));
    CHECK(dll_file_allowed(L"C:\\abs\\cdmp_pkt3576_live_fixture.dll", true));
    CHECK(!dll_file_allowed(L"C:\\abs\\cdmp_pkt3576_live.dll", true));
    CHECK(!dll_file_allowed(L"C:\\abs\\other.dll", false));
    CHECK(!dll_file_allowed(nullptr, false));
    CHECK(game_file_name(L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\CrimsonDesert.exe"));
    CHECK(!game_file_name(L"C:\\temp\\cdmp_pkt3576_live_dummy.exe"));
}

struct PolicyImage {
    uint8_t header[0x200]{};
    uint8_t* send = nullptr;
    uint8_t* recv = nullptr;
    uintptr_t base = static_cast<uintptr_t>(pkt3576::kImageBase);
    int send_reads = 0;
    int recv_reads = 0;
};

void policy_u16(uint8_t* destination, uint16_t value) { std::memcpy(destination, &value, sizeof value); }
void policy_u32(uint8_t* destination, uint32_t value) { std::memcpy(destination, &value, sizeof value); }

void fill_policy_header(PolicyImage& image, uint32_t timestamp, uint32_t size_of_image) {
    std::memset(image.header, 0, sizeof image.header);
    image.header[0] = 'M';
    image.header[1] = 'Z';
    policy_u32(image.header + 0x3C, 0x80);
    policy_u32(image.header + 0x80, 0x00004550);
    policy_u32(image.header + 0x88, timestamp);
    policy_u16(image.header + 0x98, 0x020B);
    policy_u32(image.header + 0xD0, size_of_image);
}

bool policy_read(uintptr_t address, void* destination, size_t size, void* user) {
    auto* image = static_cast<PolicyImage*>(user);
    if (address >= image->base && address - image->base < sizeof image->header) {
        const size_t offset = static_cast<size_t>(address - image->base);
        if (size > sizeof image->header || offset + size > sizeof image->header) return false;
        std::memcpy(destination, image->header + offset, size);
        return true;
    }
    if (address == image->base + static_cast<uintptr_t>(pkt3576::kSendRva) && size == 16 && image->send != nullptr) {
        ++image->send_reads;
        std::memcpy(destination, image->send, 16);
        return true;
    }
    if (address == image->base + static_cast<uintptr_t>(pkt3576::kRecvRva) && size == 16 && image->recv != nullptr) {
        ++image->recv_reads;
        std::memcpy(destination, image->recv, 16);
        return true;
    }
    return false;
}

pkt3576::Identity identify_policy(PolicyImage& image, const wchar_t* name) {
    return pkt3576::identify_image(name, image.base, policy_read, &image);
}

void policy_mid(SafetyHookContext&) {}

bool production_query_allowed(pkt3576::IdentityKind kind, bool module_loaded) {
    using pkt3576::control::ControlAction;
    using pkt3576::control::control_allowed;
    const bool status = control_allowed(false, false, kind, true, ControlAction::Status, module_loaded);
    const bool stop = control_allowed(false, false, kind, true, ControlAction::Stop, module_loaded);
    return status && stop;
}

bool production_arm_allowed(pkt3576::IdentityKind kind) {
    using pkt3576::control::ControlAction;
    using pkt3576::control::control_allowed;
    return control_allowed(false, false, kind, true, ControlAction::Load, true) &&
           control_allowed(false, false, kind, true, ControlAction::Start, true);
}

void test_patched_production_policy() {
    using pkt3576::IdentityKind;
    uint8_t* send = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    uint8_t* recv = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    CHECK(send != nullptr && recv != nullptr);
    if (send == nullptr || recv == nullptr) {
        if (send != nullptr) VirtualFree(send, 0, MEM_RELEASE);
        if (recv != nullptr) VirtualFree(recv, 0, MEM_RELEASE);
        return;
    }
    std::memset(send, 0x90, 4096);
    std::memset(recv, 0x90, 4096);
    std::memcpy(send, pkt3576::kSendEntry, 16);
    std::memcpy(recv, pkt3576::kRecvEntry, 16);

    PolicyImage image;
    image.send = send;
    image.recv = recv;
    fill_policy_header(image, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity before = identify_policy(image, pkt3576::kExeName);
    CHECK(before.kind == IdentityKind::Match);
    CHECK(pkt3576::identity_allows_hooks(before));
    CHECK(production_arm_allowed(before.kind));
    CHECK(production_query_allowed(before.kind, false));

    safetyhook::MidHook send_hook;
    safetyhook::MidHook recv_hook;
    try {
        send_hook = safetyhook::create_mid(send, policy_mid);
        recv_hook = safetyhook::create_mid(recv, policy_mid);
    } catch (...) {
        CHECK(false);
    }
    CHECK(static_cast<bool>(send_hook));
    CHECK(static_cast<bool>(recv_hook));
    if (send_hook && recv_hook) {
        const pkt3576::Identity active = identify_policy(image, pkt3576::kExeName);
        CHECK(active.kind == IdentityKind::SignatureMismatch);
        CHECK(!active.send_signature_ok);
        CHECK(!active.recv_signature_ok);
        CHECK(!pkt3576::identity_allows_hooks(active));
        CHECK(!production_arm_allowed(active.kind));
        CHECK(!pkt3576::control::modification_allowed(false, false, active.kind, true));
        CHECK(production_query_allowed(active.kind, true));
        CHECK(!production_query_allowed(active.kind, false));
        send_hook.reset();
        recv_hook.reset();
        const pkt3576::Identity after = identify_policy(image, pkt3576::kExeName);
        CHECK(after.kind == IdentityKind::Match);
        CHECK(pkt3576::identity_allows_hooks(after));
        CHECK(production_arm_allowed(after.kind));
    }
    VirtualFree(send, 0, MEM_RELEASE);
    VirtualFree(recv, 0, MEM_RELEASE);

    uint8_t send_bytes[16] = {};
    uint8_t recv_bytes[16] = {};
    std::memcpy(send_bytes, pkt3576::kSendEntry, sizeof send_bytes);
    std::memcpy(recv_bytes, pkt3576::kRecvEntry, sizeof recv_bytes);
    send_bytes[0] ^= 0xFF;

    PolicyImage mismatch;
    mismatch.send = send_bytes;
    mismatch.recv = recv_bytes;
    fill_policy_header(mismatch, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity patched = identify_policy(mismatch, pkt3576::kExeName);
    CHECK(patched.kind == IdentityKind::SignatureMismatch);
    CHECK(!production_arm_allowed(patched.kind));
    CHECK(production_query_allowed(patched.kind, true));
    CHECK(!production_query_allowed(patched.kind, false));

    PolicyImage old_build;
    old_build.send = send_bytes;
    old_build.recv = recv_bytes;
    fill_policy_header(old_build, pkt3576::kTimestamp2976, pkt3576::kImageSize);
    const pkt3576::Identity rejected = identify_policy(old_build, pkt3576::kExeName);
    CHECK(rejected.kind == IdentityKind::WrongTimestamp);
    CHECK(rejected.rejected_2976);
    CHECK(old_build.send_reads == 0);
    CHECK(old_build.recv_reads == 0);
    CHECK(!production_arm_allowed(rejected.kind));
    CHECK(!production_query_allowed(rejected.kind, true));
    CHECK(!production_query_allowed(rejected.kind, false));

    PolicyImage wrong_name;
    wrong_name.send = send_bytes;
    wrong_name.recv = recv_bytes;
    fill_policy_header(wrong_name, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity named = identify_policy(wrong_name, L"cdmp_pkt3576_live_dummy.exe");
    CHECK(named.kind == IdentityKind::WrongName);
    CHECK(wrong_name.send_reads == 0);
    CHECK(!production_arm_allowed(named.kind));
    CHECK(!production_query_allowed(named.kind, true));

    PolicyImage wrong_base;
    wrong_base.send = send_bytes;
    wrong_base.recv = recv_bytes;
    wrong_base.base = 0x180000000ull;
    fill_policy_header(wrong_base, pkt3576::kTimestamp3576, pkt3576::kImageSize);
    const pkt3576::Identity based = identify_policy(wrong_base, pkt3576::kExeName);
    CHECK(based.kind == IdentityKind::WrongBase);
    CHECK(wrong_base.send_reads == 0);
    CHECK(!production_query_allowed(based.kind, true));

    PolicyImage wrong_size;
    wrong_size.send = send_bytes;
    wrong_size.recv = recv_bytes;
    fill_policy_header(wrong_size, pkt3576::kTimestamp3576, 0x1000u);
    const pkt3576::Identity sized = identify_policy(wrong_size, pkt3576::kExeName);
    CHECK(sized.kind == IdentityKind::WrongSize);
    CHECK(wrong_size.send_reads == 0);
    CHECK(!production_query_allowed(sized.kind, true));
}

void expect_pid_usage(const wchar_t* text) {
    wchar_t* args[] = {const_cast<wchar_t*>(L"ctl"),
                       const_cast<wchar_t*>(L"status"),
                       const_cast<wchar_t*>(L"--pid"),
                       const_cast<wchar_t*>(text),
                       const_cast<wchar_t*>(L"--dll"),
                       const_cast<wchar_t*>(L"C:\\cdmp_pkt3576_live.dll")};
    CHECK(pkt3576::control::run_command(6, args, false, nullptr) == pkt3576::control::kExitUsage);
}

void test_pid_bounds() {
    unsigned long pid = 7;
    CHECK(!pkt3576::control::parse_pid(nullptr, pid));
    CHECK(pid == 7);
    CHECK(!pkt3576::control::parse_pid(L"", pid));
    CHECK(!pkt3576::control::parse_pid(L"0", pid));
    CHECK(!pkt3576::control::parse_pid(L"00", pid));
    CHECK(!pkt3576::control::parse_pid(L"-1", pid));
    CHECK(!pkt3576::control::parse_pid(L"12a", pid));
    CHECK(pid == 7);
    CHECK(pkt3576::control::parse_pid(L"1", pid) && pid == 1);
    CHECK(pkt3576::control::parse_pid(L"1234", pid) && pid == 1234);
    CHECK(pkt3576::control::parse_pid(L"4294967295", pid) && pid == 4294967295UL);
    const wchar_t* rejected[] = {L"4294967296", L"4294967297", L"4294967298", L"4294967299", L"4294967300",
                                 L"4294967301", L"4294967302", L"4294967303", L"4294967304", L"42949672950",
                                 L"99999999999999999999"};
    for (const wchar_t* text : rejected) {
        unsigned long wrapped = 7;
        CHECK(!pkt3576::control::parse_pid(text, wrapped));
        CHECK(wrapped == 7);
        expect_pid_usage(text);
    }
    expect_pid_usage(L"0");
}

void test_usage_parser() {
    wchar_t* none[] = {const_cast<wchar_t*>(L"ctl")};
    CHECK(pkt3576::control::run_command(1, none, false, nullptr) == pkt3576::control::kExitUsage);
    wchar_t* fixture_flag[] = {const_cast<wchar_t*>(L"ctl"),      const_cast<wchar_t*>(L"start"),
                               const_cast<wchar_t*>(L"--fixture"), const_cast<wchar_t*>(L"--pid"),
                               const_cast<wchar_t*>(L"1234"),      const_cast<wchar_t*>(L"--dll"),
                               const_cast<wchar_t*>(L"C:\\cdmp_pkt3576_live.dll"),
                               const_cast<wchar_t*>(L"--log-dir"), const_cast<wchar_t*>(L"C:\\outside")};
    CHECK(pkt3576::control::run_command(9, fixture_flag, false, nullptr) == pkt3576::control::kExitUsage);
    wchar_t self[32] = {};
    swprintf_s(self, L"%lu", GetCurrentProcessId());
    wchar_t* self_args[] = {const_cast<wchar_t*>(L"ctl"),
                            const_cast<wchar_t*>(L"status"),
                            const_cast<wchar_t*>(L"--pid"),
                            self,
                            const_cast<wchar_t*>(L"--dll"),
                            const_cast<wchar_t*>(L"C:\\cdmp_pkt3576_live.dll")};
    CHECK(pkt3576::control::run_command(6, self_args, false, nullptr) == pkt3576::control::kExitUsage);
    wchar_t* zero[] = {const_cast<wchar_t*>(L"ctl"), const_cast<wchar_t*>(L"load"), const_cast<wchar_t*>(L"--pid"),
                       const_cast<wchar_t*>(L"0"), const_cast<wchar_t*>(L"--dll"),
                       const_cast<wchar_t*>(L"C:\\cdmp_pkt3576_live.dll")};
    CHECK(pkt3576::control::run_command(6, zero, false, nullptr) == pkt3576::control::kExitUsage);
}

bool policy_only() {
    wchar_t value[8] = {};
    const DWORD length = GetEnvironmentVariableW(L"PKT3576_CTL_POLICY_ONLY", value, 8);
    return length > 0 && value[0] == L'1';
}

void test_exports() {
    const std::wstring path = beside(L"cdmp_pkt3576_live_fixture.dll");
    HMODULE module = LoadLibraryW(path.c_str());
    CHECK(module != nullptr);
    if (module == nullptr) return;
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_arm") != nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_fixture_start") != nullptr);
    CHECK(GetProcAddress(module, "pkt3576_live_report") != nullptr);
    CHECK(FreeLibrary(module));
}

void test_flow() {
    const std::wstring production_ctl = beside(L"cdmp_pkt3576_live_ctl.exe");
    const std::wstring fixture_ctl = beside(L"cdmp_pkt3576_live_ctl_fixture.exe");
    const std::wstring production_dll = beside(L"cdmp_pkt3576_live.dll");
    const std::wstring fixture_dll = beside(L"cdmp_pkt3576_live_fixture.dll");
    CHECK(!production_ctl.empty() && !fixture_ctl.empty() && !production_dll.empty() && !fixture_dll.empty());

    Dummy dummy;
    if (!spawn_dummy(dummy, false)) return;
    const std::wstring pid = std::to_wstring(dummy.pid);
    const std::wstring log_dir = log_directory(dummy.pid, L"main");

    Captured rejected = run_exe(production_ctl, L"load --pid " + pid + L" --dll " + quote(production_dll));
    expect(rejected, "ctl_exit", "4");
    expect(rejected, "ctl_identity", "wrong-name");
    expect(rejected, "ctl_gate", "rejected");
    expect(rejected, "ctl_modified", "0");
    expect(rejected, "ctl_loaded", "0");
    expect(rejected, "ctl_target_alive", "1");
    expect(rejected, "ctl_note", "wrong-name");
    CHECK(rejected.exit_code == 4);
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live.dll"));

    Captured rejected_start =
        run_exe(production_ctl, L"start --pid " + pid + L" --dll " + quote(production_dll) + L" --log-dir " +
                                    quote(log_dir));
    expect(rejected_start, "ctl_exit", "4");
    expect(rejected_start, "ctl_modified", "0");
    expect(rejected_start, "ctl_loaded", "0");
    expect(rejected_start, "ctl_identity", "wrong-name");
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live.dll"));

    Captured rejected_status = run_exe(production_ctl, L"status --pid " + pid + L" --dll " + quote(production_dll));
    expect(rejected_status, "ctl_exit", "4");
    expect(rejected_status, "ctl_note", "wrong-name");
    expect(rejected_status, "ctl_modified", "0");

    Captured fixture_flag =
        run_exe(production_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(production_dll) + L" --log-dir " +
                                    quote(log_dir));
    expect(fixture_flag, "ctl_exit", "2");
    expect(fixture_flag, "ctl_modified", "0");
    expect(fixture_flag, "ctl_note", "usage");
    CHECK(fixture_flag.exit_code == 2);
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live.dll"));
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live_fixture.dll"));

    Captured relative_dll = run_exe(production_ctl, L"load --pid " + pid + L" --dll cdmp_pkt3576_live.dll");
    expect(relative_dll, "ctl_exit", "2");
    expect(relative_dll, "ctl_note", "bad-dll");
    expect(relative_dll, "ctl_modified", "0");

    Captured bad_log = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                               L" --log-dir logs");
    expect(bad_log, "ctl_exit", "5");
    expect(bad_log, "ctl_start", "6");
    expect(bad_log, "ctl_modified", "0");
    expect(bad_log, "ctl_loaded", "0");
    expect(bad_log, "ctl_note", "bad-log");
    expect(bad_log, "ctl_gate", "fixture");
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live_fixture.dll"));

    Captured not_loaded = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(not_loaded, "ctl_exit", "0");
    expect(not_loaded, "ctl_note", "not-loaded");
    expect(not_loaded, "ctl_modified", "0");
    expect(not_loaded, "ctl_loaded", "0");
    Captured stop_missing = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(stop_missing, "ctl_exit", "5");
    expect(stop_missing, "ctl_note", "not-loaded");
    expect(stop_missing, "ctl_modified", "0");

    Captured loaded = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(loaded, "ctl_exit", "0");
    expect(loaded, "ctl_loaded", "1");
    expect(loaded, "ctl_modified", "1");
    expect(loaded, "ctl_gate", "fixture");
    expect(loaded, "ctl_note", "loaded");
    CHECK(module_mapped(dummy.pid, L"cdmp_pkt3576_live_fixture.dll"));
    CHECK(!module_mapped(dummy.pid, L"cdmp_pkt3576_live.dll"));

    Captured idle = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(idle, "ctl_exit", "0");
    expect(idle, "ctl_capture_attempts", "0");
    expect(idle, "ctl_loaded", "1");
    expect(idle, "ctl_start", "-1");

    Captured started =
        run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) + L" --log-dir " +
                                 quote(log_dir));
    expect(started, "ctl_exit", "0");
    expect(started, "ctl_start", "0");
    expect(started, "ctl_capture_attempts", "1");
    expect(started, "ctl_gate", "fixture");
    expect(started, "ctl_loaded", "1");
    expect(started, "ctl_retained", "0");
    expect(started, "ctl_stored", "0");
    expect(started, "ctl_omitted", "0");
    expect(started, "ctl_target_alive", "1");
    expect(started, "ctl_note", "started");
    const std::string session = read_file(log_dir + L"\\pkt3576-live-session.txt");
    if (session.find("fixture=1") == std::string::npos || session.find("game_match=0") == std::string::npos ||
        session.find("game_match=1") != std::string::npos || session.find("gate=fixture") == std::string::npos ||
        session.find("startup=started") == std::string::npos) {
        std::printf("FAIL fixture session:\n%s\n", session.c_str());
        ++g_failures;
    }

    Captured status = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(status, "ctl_exit", "0");
    expect(status, "ctl_capture_attempts", "1");
    expect(status, "ctl_start", "0");
    expect(status, "ctl_loaded", "1");
    expect(status, "ctl_note", "ok");

    Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(stopped, "ctl_exit", "0");
    expect(stopped, "ctl_stop", "4");
    expect(stopped, "ctl_retained", "0");
    expect(stopped, "ctl_unloaded", "0");
    expect(stopped, "ctl_loaded", "1");
    expect(stopped, "ctl_target_alive", "1");
    expect(stopped, "ctl_note", "removed");
    CHECK(module_mapped(dummy.pid, L"cdmp_pkt3576_live_fixture.dll"));
    DWORD alive = 0;
    CHECK(GetExitCodeProcess(dummy.process, &alive) && alive == STILL_ACTIVE);

    Dummy crowded;
    if (!spawn_dummy(crowded, true)) return;
    CHECK(crowded.spawned >= 280);
    const std::wstring crowded_pid = std::to_wstring(crowded.pid);
    const std::wstring crowded_log = log_directory(crowded.pid, L"retain");
    Captured armed =
        run_exe(fixture_ctl, L"start --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll) +
                                 L" --log-dir " + quote(crowded_log));
    expect(armed, "ctl_exit", "0");
    expect(armed, "ctl_start", "0");
    expect(armed, "ctl_capture_attempts", "1");
    const std::string retained_session = read_file(crowded_log + L"\\pkt3576-live-session.txt");
    CHECK(retained_session.find("fixture=1") != std::string::npos);
    CHECK(retained_session.find("game_match=0") != std::string::npos);
    CHECK(retained_session.find("game_match=1") == std::string::npos);

    Captured retained = run_exe(fixture_ctl, L"stop --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll));
    expect(retained, "ctl_exit", "3");
    expect(retained, "ctl_stop", "3");
    expect(retained, "ctl_retained", "1");
    expect(retained, "ctl_unloaded", "0");
    expect(retained, "ctl_loaded", "1");
    expect(retained, "ctl_target_alive", "1");
    expect(retained, "ctl_note", "retained");
    CHECK(retained.exit_code == 3);
    CHECK(module_mapped(crowded.pid, L"cdmp_pkt3576_live_fixture.dll"));
    DWORD crowded_alive = 0;
    CHECK(GetExitCodeProcess(crowded.process, &crowded_alive) && crowded_alive == STILL_ACTIVE);

    Captured retained_again =
        run_exe(fixture_ctl, L"stop --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll));
    expect(retained_again, "ctl_exit", "3");
    expect(retained_again, "ctl_stop", "3");
    expect(retained_again, "ctl_retained", "1");
    expect(retained_again, "ctl_unloaded", "0");
    expect(retained_again, "ctl_target_alive", "1");
    CHECK(module_mapped(crowded.pid, L"cdmp_pkt3576_live_fixture.dll"));
    CHECK(GetExitCodeProcess(crowded.process, &crowded_alive) && crowded_alive == STILL_ACTIVE);
}

}  // namespace

int main() {
    test_policy();
    test_patched_production_policy();
    test_pid_bounds();
    test_usage_parser();
    if (!policy_only()) {
        test_exports();
        test_flow();
    }
    if (g_failures != 0) {
        std::printf("pkt3576 live ctl tests failed: %d\n", g_failures);
        return 1;
    }
    std::printf("%s\n", policy_only() ? "pkt3576 live ctl policy ok" : "pkt3576 live ctl tests ok");
    return 0;
}
