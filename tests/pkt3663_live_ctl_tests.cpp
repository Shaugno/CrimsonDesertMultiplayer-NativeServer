// Game-free controller flow. Targets are copies of cdmp_pkt3663_live_dummy.exe.
// Nothing here launches or attaches to the installed Crimson Desert process.

#include "pkt3663/assess.h"
#include "pkt3663/live_control.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>

namespace {

int g_failures = 0;

#define CHECK(cond)                                                                        \
  do {                                                                                     \
    if (!(cond)) {                                                                         \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                         \
      ++g_failures;                                                                        \
    }                                                                                      \
  } while (0)

std::wstring beside(const wchar_t *file_name) {
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
  *(slash + 1) = L'\0';
  return std::wstring(path) + file_name;
}

std::wstring quote(const std::wstring &text) {
  std::wstring out = L"\"";
  for (wchar_t ch : text) {
    if (ch == L'"')
      out += L'"';
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

std::map<std::string, std::string> parse_fields(const std::string &text) {
  std::map<std::string, std::string> fields;
  size_t pos = 0;
  while (pos < text.size()) {
    size_t end = text.find('\n', pos);
    if (end == std::string::npos)
      end = text.size();
    std::string line = text.substr(pos, end - pos);
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    const size_t eq = line.find('=');
    if (eq != std::string::npos)
      fields[line.substr(0, eq)] = line.substr(eq + 1);
    pos = end + 1;
  }
  return fields;
}

Captured run_exe(const std::wstring &exe, const std::wstring &args, DWORD wait_ms = 30000) {
  Captured captured;
  SECURITY_ATTRIBUTES security = {};
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  HANDLE read_pipe = nullptr;
  HANDLE write_pipe = nullptr;
  if (!CreatePipe(&read_pipe, &write_pipe, &security, 0))
    return captured;
  SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
  HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING,
                           0, nullptr);
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
  const BOOL created = CreateProcessW(exe.c_str(), mutable_command.data(), nullptr, nullptr, TRUE,
                                      CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
  CloseHandle(write_pipe);
  if (nul != nullptr && nul != INVALID_HANDLE_VALUE)
    CloseHandle(nul);
  if (!created) {
    CloseHandle(read_pipe);
    std::printf("FAIL CreateProcess %ls error %lu\n", exe.c_str(), GetLastError());
    ++g_failures;
    return captured;
  }
  char buffer[512];
  DWORD got = 0;
  while (ReadFile(read_pipe, buffer, sizeof buffer, &got, nullptr) && got > 0)
    captured.output.append(buffer, got);
  CloseHandle(read_pipe);
  WaitForSingleObject(process.hProcess, wait_ms);
  DWORD code = 1;
  GetExitCodeProcess(process.hProcess, &code);
  captured.exit_code = code;
  captured.launched = true;
  captured.fields = parse_fields(captured.output);
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
  return captured;
}

void require_keys(const Captured &captured) {
  const char *keys[] = {"ctl_action",        "ctl_pid",           "ctl_identity",
                        "ctl_rejected_2976", "ctl_rejected_3576", "ctl_gate",
                        "ctl_modified",      "ctl_loaded",        "ctl_start",
                        "ctl_stop",          "ctl_retained",      "ctl_stored",
                        "ctl_omitted",       "ctl_unread",        "ctl_truncated",
                        "ctl_raw_mode",      "ctl_allowlist",     "ctl_adopted",
                        "ctl_phase",         "ctl_capture_attempts", "ctl_qpc_frequency",
                        "ctl_unloaded",      "ctl_target_alive",  "ctl_note",
                        "ctl_thread_created", "ctl_remote_wait_ms", "ctl_remote_wait",
                        "ctl_exit"};
  for (const char *key : keys) {
    if (captured.fields.find(key) == captured.fields.end()) {
      std::printf("FAIL missing %s\n%s\n", key, captured.output.c_str());
      ++g_failures;
    }
  }
}

void expect(const Captured &captured, const char *key, const std::string &value) {
  require_keys(captured);
  const auto found = captured.fields.find(key);
  if (found == captured.fields.end() || found->second != value) {
    const char *got = found == captured.fields.end() ? "<missing>" : found->second.c_str();
    std::printf("FAIL %s want %s got %s\n%s\n", key, value.c_str(), got, captured.output.c_str());
    ++g_failures;
  }
}

bool module_mapped_once(DWORD pid, const wchar_t *file_name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (snap == INVALID_HANDLE_VALUE)
    return false;
  MODULEENTRY32W entry = {};
  entry.dwSize = sizeof entry;
  bool found = false;
  if (Module32FirstW(snap, &entry)) {
    do {
      if (_wcsicmp(entry.szModule, file_name) == 0)
        found = true;
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

bool spawn_at(Dummy &dummy, const std::wstring &exe, bool sleepers) {
  SECURITY_ATTRIBUTES security = {};
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  HANDLE read_pipe = nullptr;
  HANDLE write_pipe = nullptr;
  if (!CreatePipe(&read_pipe, &write_pipe, &security, 0))
    return false;
  SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
  HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING,
                           0, nullptr);
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
  const BOOL created = CreateProcessW(exe.c_str(), mutable_command.data(), nullptr, nullptr, TRUE,
                                      CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
  CloseHandle(write_pipe);
  if (nul != nullptr && nul != INVALID_HANDLE_VALUE)
    CloseHandle(nul);
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
    if (!PeekNamedPipe(read_pipe, nullptr, 0, nullptr, &available, nullptr))
      break;
    if (available == 0) {
      Sleep(10);
      continue;
    }
    char buffer[256];
    DWORD got = 0;
    const DWORD want = available > sizeof buffer ? static_cast<DWORD>(sizeof buffer) : available;
    if (!ReadFile(read_pipe, buffer, want, &got, nullptr) || got == 0)
      break;
    text.append(buffer, got);
  }
  CloseHandle(read_pipe);
  const std::map<std::string, std::string> fields = parse_fields(text);
  dummy.process = process.hProcess;
  dummy.pid = process.dwProcessId;
  wchar_t event_name[64] = {};
  swprintf_s(event_name, L"Local\\pkt3663-live-dummy-%lu", dummy.pid);
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

std::wstring log_directory(DWORD pid, const wchar_t *suffix) {
  wchar_t temp[MAX_PATH] = {};
  GetTempPathW(MAX_PATH, temp);
  return std::wstring(temp) + L"pkt3663-live-ctl-" + std::to_wstring(pid) + L"-" + suffix;
}

std::string read_file(const std::wstring &path) {
  FILE *file = nullptr;
  if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || file == nullptr)
    return {};
  std::string text;
  char buffer[256];
  size_t count = 0;
  while ((count = std::fread(buffer, 1, sizeof buffer, file)) > 0)
    text.append(buffer, count);
  std::fclose(file);
  return text;
}

bool file_exists(const std::wstring &path) {
  const DWORD attributes = GetFileAttributesW(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool real_game_running() {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE)
    return false;
  PROCESSENTRY32W entry = {};
  entry.dwSize = sizeof entry;
  bool found = false;
  if (Process32FirstW(snap, &entry)) {
    do {
      if (_wcsicmp(entry.szExeFile, L"CrimsonDesert.exe") != 0)
        continue;
      HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
      if (process == nullptr)
        continue;
      wchar_t path[1024] = {};
      DWORD chars = 1024;
      if (QueryFullProcessImageNameW(process, 0, path, &chars)) {
        if (wcsstr(path, L"\\Crimson Desert\\bin64\\") != nullptr)
          found = true;
      }
      CloseHandle(process);
    } while (!found && Process32NextW(snap, &entry));
  }
  CloseHandle(snap);
  return found;
}

void test_policy() {
  using pkt3663::IdentityKind;
  using pkt3663::control::ControlAction;
  using pkt3663::control::RemoteWait;
  using pkt3663::control::control_allowed;
  using pkt3663::control::dll_file_allowed;
  using pkt3663::control::game_file_name;
  using pkt3663::control::installed_module_control_allowed;
  using pkt3663::control::keep_remote_allocation;
  using pkt3663::control::modification_allowed;
  using pkt3663::control::unload_after_observed;
  using pkt3663::control::unload_allowed;
  CHECK(modification_allowed(false, false, IdentityKind::Match, true));
  CHECK(!modification_allowed(false, false, IdentityKind::WrongName, false));
  CHECK(!modification_allowed(false, false, IdentityKind::AnchorMismatch, true));
  CHECK(!modification_allowed(false, true, IdentityKind::Match, false));
  CHECK(modification_allowed(true, true, IdentityKind::WrongName, false));
  CHECK(!modification_allowed(true, true, IdentityKind::Match, true));
  CHECK(modification_allowed(true, false, IdentityKind::Match, false));
  CHECK(!modification_allowed(true, false, IdentityKind::WrongName, false));
  CHECK(installed_module_control_allowed(IdentityKind::AnchorMismatch, true));
  CHECK(!installed_module_control_allowed(IdentityKind::AnchorMismatch, false));
  CHECK(!installed_module_control_allowed(IdentityKind::Match, true));
  CHECK(control_allowed(false, false, IdentityKind::Match, true, ControlAction::Load, false));
  CHECK(control_allowed(false, false, IdentityKind::Match, true, ControlAction::Start, false));
  CHECK(control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Status, true));
  CHECK(control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Report, true));
  CHECK(control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Stop, true));
  CHECK(control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Mark, true));
  CHECK(control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Unload, true));
  CHECK(!control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Load, true));
  CHECK(!control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Start, true));
  CHECK(!control_allowed(false, false, IdentityKind::AnchorMismatch, true, ControlAction::Status, false));
  const IdentityKind closed[] = {
      IdentityKind::WrongName,         IdentityKind::WrongTimestamp, IdentityKind::WrongBase,
      IdentityKind::WrongSize,         IdentityKind::Unreadable,     IdentityKind::MalformedImage,
      IdentityKind::MissingFunctionTable, IdentityKind::NotExecutable, IdentityKind::FunctionRangeMismatch,
      IdentityKind::AmbiguousAnchor,   IdentityKind::CallTargetMismatch};
  for (IdentityKind kind : closed) {
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Load, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Start, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Status, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Report, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Stop, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Mark, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Unload, true));
    CHECK(!control_allowed(false, false, kind, true, ControlAction::Status, false));
  }
  CHECK(!control_allowed(true, true, IdentityKind::AnchorMismatch, true, ControlAction::Status, true));
  CHECK(!unload_allowed(kPkt3663LiveStarted, 0, 1));
  CHECK(unload_allowed(kPkt3663LiveRefused, 0, 0));
  CHECK(!unload_allowed(kPkt3663LiveRefused, 1, 0));
  CHECK(!unload_allowed(kPkt3663LiveRolledBack, 0, 0));
  CHECK(unload_allowed(kPkt3663LiveBadLog, 0, 0));
  CHECK(unload_allowed(kPkt3663LiveIdle, 0, 0));
  CHECK(unload_allowed(kPkt3663LiveRemoved, 0, 0));
  CHECK(!unload_allowed(kPkt3663LiveRemoved, 0, 1));
  CHECK(!unload_allowed(kPkt3663LiveRetained, 0, 0));
  CHECK(!unload_allowed(-1, 0, 1));
  CHECK(unload_allowed(-1, 0, 0));
  CHECK(keep_remote_allocation(RemoteWait::Timeout));
  CHECK(!keep_remote_allocation(RemoteWait::Ok));
  CHECK(!keep_remote_allocation(RemoteWait::Failed));
  CHECK(!unload_after_observed(true, RemoteWait::Timeout, true, kPkt3663LiveRefused, 0, 0));
  CHECK(!unload_after_observed(true, RemoteWait::Ok, false, kPkt3663LiveRefused, 0, 0));
  CHECK(!unload_after_observed(false, RemoteWait::Ok, true, kPkt3663LiveRefused, 0, 0));
  CHECK(!unload_after_observed(true, RemoteWait::Ok, true, kPkt3663LiveRetained, 1, 0));
  CHECK(unload_after_observed(true, RemoteWait::Ok, true, kPkt3663LiveBadLog, 0, 0));
  CHECK(!unload_after_observed(true, RemoteWait::Ok, true, kPkt3663LiveStarted, 0, 1));
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveBadLog, kPkt3663LiveRemoved) ==
        kPkt3663LiveBadLog);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRetained, kPkt3663LiveIdle) ==
        kPkt3663LiveRetained);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRetained, kPkt3663LiveStarted) ==
        kPkt3663LiveRetained);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRemoved, kPkt3663LiveRemoved) ==
        kPkt3663LiveRemoved);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveStarted, kPkt3663LiveStarted) ==
        kPkt3663LiveStarted);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRefused, kPkt3663LiveStarted) ==
        kPkt3663LiveRefused);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRolledBack, kPkt3663LiveStarted) ==
        kPkt3663LiveRolledBack);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveIdle, kPkt3663LiveRemoved) ==
        kPkt3663LiveIdle);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRemoved, kPkt3663LiveIdle) ==
        kPkt3663LiveRemoved);
  CHECK(pkt3663::control::command_result_after_report(7, kPkt3663LiveStarted) == 7);
  CHECK(pkt3663::control::command_result_after_report(-1, kPkt3663LiveStarted) == -1);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveStarted, kPkt3663LiveRetained) ==
        kPkt3663LiveRetained);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRefused, kPkt3663LiveRetained) ==
        kPkt3663LiveRetained);
  CHECK(pkt3663::control::command_result_after_report(kPkt3663LiveRemoved, kPkt3663LiveRetained) ==
        kPkt3663LiveRetained);
  CHECK(pkt3663::control::exit_without_report(kPkt3663LiveRetained) == pkt3663::control::kExitRetained);
  CHECK(pkt3663::control::exit_without_report(kPkt3663LiveStarted) == pkt3663::control::kExitFailed);
  CHECK(pkt3663::control::exit_without_report(kPkt3663LiveRemoved) == pkt3663::control::kExitFailed);
  CHECK(pkt3663::control::exit_without_report(kPkt3663LiveIdle) == pkt3663::control::kExitFailed);
  CHECK(pkt3663::control::exit_without_report(kPkt3663LiveBadLog) == pkt3663::control::kExitFailed);
  CHECK(pkt3663::control::exit_without_report(-1) == pkt3663::control::kExitFailed);
  CHECK(dll_file_allowed(L"C:\\abs\\cdmp_pkt3663_live.dll", false));
  CHECK(!dll_file_allowed(L"C:\\abs\\cdmp_pkt3663_live_fixture.dll", false));
  CHECK(dll_file_allowed(L"C:\\abs\\cdmp_pkt3663_live_fixture.dll", true));
  CHECK(!dll_file_allowed(L"C:\\abs\\cdmp_pkt3663_live.dll", true));
  CHECK(!dll_file_allowed(L"C:\\abs\\other.dll", false));
  CHECK(!dll_file_allowed(nullptr, false));
  CHECK(game_file_name(L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\CrimsonDesert.exe"));
  CHECK(!game_file_name(L"C:\\temp\\cdmp_pkt3663_live_dummy.exe"));
  CHECK(pkt3663::log_directory_allowed(L"C:\\temp\\cdmp_pkt3663_live_dummy.exe", L"C:\\outside\\logs"));
  CHECK(!pkt3663::log_directory_allowed(L"C:\\temp\\cdmp_pkt3663_live_dummy.exe", L"logs"));
  CHECK(!pkt3663::log_directory_allowed(L"C:\\temp\\cdmp_pkt3663_live_dummy.exe",
                                       L"C:\\Crimson Desert\\bin64\\logs"));
  CHECK(!pkt3663::log_directory_allowed(
      L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\CrimsonDesert.exe",
      L"D:\\SteamLibrary\\steamapps\\common\\Crimson Desert\\bin64\\capture"));
}

void expect_pid_usage(const wchar_t *text) {
  wchar_t *args[] = {const_cast<wchar_t *>(L"ctl"),
                     const_cast<wchar_t *>(L"status"),
                     const_cast<wchar_t *>(L"--pid"),
                     const_cast<wchar_t *>(text),
                     const_cast<wchar_t *>(L"--dll"),
                     const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll")};
  CHECK(pkt3663::control::run_command(6, args, false, nullptr) == pkt3663::control::kExitUsage);
}

void test_pid_bounds() {
  unsigned long pid = 7;
  CHECK(!pkt3663::control::parse_pid(nullptr, pid));
  CHECK(pid == 7);
  CHECK(!pkt3663::control::parse_pid(L"", pid));
  CHECK(!pkt3663::control::parse_pid(L"0", pid));
  CHECK(!pkt3663::control::parse_pid(L"00", pid));
  CHECK(!pkt3663::control::parse_pid(L"-1", pid));
  CHECK(!pkt3663::control::parse_pid(L"12a", pid));
  CHECK(pid == 7);
  CHECK(pkt3663::control::parse_pid(L"1", pid) && pid == 1);
  CHECK(pkt3663::control::parse_pid(L"1234", pid) && pid == 1234);
  CHECK(pkt3663::control::parse_pid(L"4294967295", pid) && pid == 4294967295UL);
  const wchar_t *rejected[] = {L"4294967296", L"4294967297", L"4294967298", L"4294967299",
                               L"4294967300", L"4294967301", L"4294967302", L"4294967303",
                               L"4294967304", L"42949672950", L"99999999999999999999"};
  for (const wchar_t *text : rejected) {
    unsigned long wrapped = 7;
    CHECK(!pkt3663::control::parse_pid(text, wrapped));
    CHECK(wrapped == 7);
    expect_pid_usage(text);
  }
  expect_pid_usage(L"0");
}

void test_usage_parser() {
  wchar_t *none[] = {const_cast<wchar_t *>(L"ctl")};
  CHECK(pkt3663::control::run_command(1, none, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *fixture_flag[] = {const_cast<wchar_t *>(L"ctl"),
                             const_cast<wchar_t *>(L"start"),
                             const_cast<wchar_t *>(L"--fixture"),
                             const_cast<wchar_t *>(L"--pid"),
                             const_cast<wchar_t *>(L"1234"),
                             const_cast<wchar_t *>(L"--dll"),
                             const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll"),
                             const_cast<wchar_t *>(L"--log-dir"),
                             const_cast<wchar_t *>(L"C:\\outside")};
  CHECK(pkt3663::control::run_command(9, fixture_flag, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t self[32] = {};
  swprintf_s(self, L"%lu", GetCurrentProcessId());
  wchar_t *self_args[] = {const_cast<wchar_t *>(L"ctl"),  const_cast<wchar_t *>(L"status"),
                          const_cast<wchar_t *>(L"--pid"), self,
                          const_cast<wchar_t *>(L"--dll"), const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll")};
  CHECK(pkt3663::control::run_command(6, self_args, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *relative[] = {const_cast<wchar_t *>(L"ctl"),
                         const_cast<wchar_t *>(L"load"),
                         const_cast<wchar_t *>(L"--pid"),
                         const_cast<wchar_t *>(L"1234"),
                         const_cast<wchar_t *>(L"--dll"),
                         const_cast<wchar_t *>(L"cdmp_pkt3663_live.dll")};
  CHECK(pkt3663::control::run_command(6, relative, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *fixture_dll[] = {const_cast<wchar_t *>(L"ctl"),
                            const_cast<wchar_t *>(L"load"),
                            const_cast<wchar_t *>(L"--pid"),
                            const_cast<wchar_t *>(L"1234"),
                            const_cast<wchar_t *>(L"--dll"),
                            const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live_fixture.dll")};
  CHECK(pkt3663::control::run_command(6, fixture_dll, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *production_on_fixture[] = {const_cast<wchar_t *>(L"ctl"),
                                      const_cast<wchar_t *>(L"load"),
                                      const_cast<wchar_t *>(L"--fixture"),
                                      const_cast<wchar_t *>(L"--pid"),
                                      const_cast<wchar_t *>(L"1234"),
                                      const_cast<wchar_t *>(L"--dll"),
                                      const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll")};
  CHECK(pkt3663::control::run_command(7, production_on_fixture, true, "pkt3663_live_fixture_arm") ==
        pkt3663::control::kExitUsage);
  wchar_t *rva[] = {const_cast<wchar_t *>(L"ctl"),
                    const_cast<wchar_t *>(L"load"),
                    const_cast<wchar_t *>(L"--pid"),
                    const_cast<wchar_t *>(L"1234"),
                    const_cast<wchar_t *>(L"--dll"),
                    const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll"),
                    const_cast<wchar_t *>(L"--rva"),
                    const_cast<wchar_t *>(L"0x3A9960")};
  CHECK(pkt3663::control::run_command(8, rva, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *force[] = {const_cast<wchar_t *>(L"ctl"),
                      const_cast<wchar_t *>(L"load"),
                      const_cast<wchar_t *>(L"--force"),
                      const_cast<wchar_t *>(L"--pid"),
                      const_cast<wchar_t *>(L"1234"),
                      const_cast<wchar_t *>(L"--dll"),
                      const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll")};
  CHECK(pkt3663::control::run_command(7, force, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *mark_bad[] = {const_cast<wchar_t *>(L"ctl"),
                         const_cast<wchar_t *>(L"mark"),
                         const_cast<wchar_t *>(L"--pid"),
                         const_cast<wchar_t *>(L"1234"),
                         const_cast<wchar_t *>(L"--dll"),
                         const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll"),
                         const_cast<wchar_t *>(L"--phase"),
                         const_cast<wchar_t *>(L"nope")};
  CHECK(pkt3663::control::run_command(8, mark_bad, false, nullptr) == pkt3663::control::kExitUsage);
  wchar_t *phase_on_start[] = {const_cast<wchar_t *>(L"ctl"),
                               const_cast<wchar_t *>(L"start"),
                               const_cast<wchar_t *>(L"--pid"),
                               const_cast<wchar_t *>(L"1234"),
                               const_cast<wchar_t *>(L"--dll"),
                               const_cast<wchar_t *>(L"C:\\cdmp_pkt3663_live.dll"),
                               const_cast<wchar_t *>(L"--log-dir"),
                               const_cast<wchar_t *>(L"C:\\outside"),
                               const_cast<wchar_t *>(L"--phase"),
                               const_cast<wchar_t *>(L"stand")};
  CHECK(pkt3663::control::run_command(10, phase_on_start, false, nullptr) == pkt3663::control::kExitUsage);
}

bool policy_only() {
  wchar_t value[8] = {};
  const DWORD length = GetEnvironmentVariableW(L"PKT3663_CTL_POLICY_ONLY", value, 8);
  return length > 0 && value[0] == L'1';
}

void test_exports() {
  const std::wstring production = beside(L"cdmp_pkt3663_live.dll");
  const std::wstring fixture = beside(L"cdmp_pkt3663_live_fixture.dll");
  HMODULE production_module = LoadLibraryW(production.c_str());
  CHECK(production_module != nullptr);
  if (production_module != nullptr) {
    CHECK(GetProcAddress(production_module, "pkt3663_live_start") != nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_stop") != nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_report") != nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_mark") != nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_fixture_arm") == nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_fixture_start") == nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_fixture_set_phase_hold") == nullptr);
    CHECK(GetProcAddress(production_module, "pkt3663_live_fixture_fail_log_flush_after") == nullptr);
    CHECK(FreeLibrary(production_module));
  }
  HMODULE fixture_module = LoadLibraryW(fixture.c_str());
  CHECK(fixture_module != nullptr);
  if (fixture_module != nullptr) {
    CHECK(GetProcAddress(fixture_module, "pkt3663_live_fixture_arm") != nullptr);
    CHECK(GetProcAddress(fixture_module, "pkt3663_live_fixture_start") != nullptr);
    CHECK(GetProcAddress(fixture_module, "pkt3663_live_report") != nullptr);
    CHECK(FreeLibrary(fixture_module));
  }
}

struct TempExe {
  std::wstring directory;
  std::wstring path;
  ~TempExe() { cleanup(); }
  void cleanup() {
    if (!path.empty())
      DeleteFileW(path.c_str());
    if (!directory.empty())
      RemoveDirectoryW(directory.c_str());
    path.clear();
    directory.clear();
  }
};

bool patch_timestamp(const std::wstring &path, uint32_t timestamp) {
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  uint8_t dos[0x40] = {};
  DWORD got = 0;
  const BOOL read = ReadFile(file, dos, sizeof dos, &got, nullptr);
  uint32_t lfanew = 0;
  std::memcpy(&lfanew, dos + 0x3C, sizeof lfanew);
  uint32_t signature = 0;
  DWORD signature_got = 0;
  bool ok = read && got == sizeof dos && dos[0] == 'M' && dos[1] == 'Z' && lfanew >= 0x40 && lfanew <= 1024;
  if (ok) {
    ok = SetFilePointer(file, lfanew, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
         ReadFile(file, &signature, sizeof signature, &signature_got, nullptr) &&
         signature_got == sizeof signature && signature == 0x00004550u;
  }
  DWORD wrote = 0;
  if (ok) {
    ok = SetFilePointer(file, static_cast<LONG>(lfanew + 8), nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
         WriteFile(file, &timestamp, sizeof timestamp, &wrote, nullptr) && wrote == sizeof timestamp;
  }
  CloseHandle(file);
  return ok;
}

bool make_named_exe(TempExe &temp, const std::wstring &source, bool patch, uint32_t timestamp) {
  wchar_t root[MAX_PATH] = {};
  if (GetTempPathW(MAX_PATH, root) == 0)
    return false;
  static int serial = 0;
  temp.directory = std::wstring(root) + L"pkt3663-ctl-id-" + std::to_wstring(GetCurrentProcessId()) + L"-" +
                   std::to_wstring(++serial);
  if (!CreateDirectoryW(temp.directory.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    return false;
  temp.path = temp.directory + L"\\CrimsonDesert.exe";
  if (temp.path.find(L"\\Crimson Desert\\bin64\\") != std::wstring::npos)
    return false;
  if (!CopyFileW(source.c_str(), temp.path.c_str(), FALSE))
    return false;
  if (patch && !patch_timestamp(temp.path, timestamp))
    return false;
  return true;
}

bool corrupt_lfanew(HANDLE process, uintptr_t &base_out) {
  base_out = 0;
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(process));
  if (snap == INVALID_HANDLE_VALUE)
    return false;
  MODULEENTRY32W entry = {};
  entry.dwSize = sizeof entry;
  const BOOL first = Module32FirstW(snap, &entry);
  CloseHandle(snap);
  if (!first)
    return false;
  const uintptr_t base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
  uint8_t dos[0x40] = {};
  SIZE_T got = 0;
  if (!ReadProcessMemory(process, reinterpret_cast<void *>(base), dos, sizeof dos, &got) || got != sizeof dos)
    return false;
  if (dos[0] != 'M' || dos[1] != 'Z')
    return false;
  DWORD old = 0;
  if (!VirtualProtectEx(process, reinterpret_cast<void *>(base), 0x1000, PAGE_READWRITE, &old))
    return false;
  const uint32_t bad = 0x10;
  SIZE_T wrote = 0;
  const BOOL stored =
      WriteProcessMemory(process, reinterpret_cast<void *>(base + 0x3C), &bad, sizeof bad, &wrote);
  DWORD restored = 0;
  VirtualProtectEx(process, reinterpret_cast<void *>(base), 0x1000, old, &restored);
  if (!stored || wrote != sizeof bad)
    return false;
  uint32_t seen = 0;
  if (!ReadProcessMemory(process, reinterpret_cast<void *>(base + 0x3C), &seen, sizeof seen, &got) ||
      got != sizeof seen || seen != bad)
    return false;
  base_out = base;
  return true;
}

void expect_unmodified_refusal(const Captured &captured, const char *identity) {
  expect(captured, "ctl_exit", "4");
  expect(captured, "ctl_identity", identity);
  expect(captured, "ctl_gate", "rejected");
  expect(captured, "ctl_modified", "0");
  expect(captured, "ctl_loaded", "0");
  expect(captured, "ctl_unloaded", "0");
  expect(captured, "ctl_target_alive", "1");
  CHECK(captured.exit_code == 4);
}

void test_named_identity(const std::wstring &production_ctl, const std::wstring &fixture_ctl,
                         const std::wstring &production_dll, const std::wstring &fixture_dll,
                         const std::wstring &source) {
  struct Case {
    const char *label;
    bool patch;
    uint32_t timestamp;
    const char *identity;
    const char *rejected_2976;
    const char *rejected_3576;
  };
  const Case cases[] = {
      {"timestamp-2976", true, pkt3663::kTimestamp2976, "wrong-timestamp", "1", "0"},
      {"timestamp-3576", true, pkt3663::kTimestamp3576, "wrong-timestamp", "0", "1"},
      {"timestamp-3663-forged", true, pkt3663::kTimestamp3663, nullptr, nullptr, nullptr},
  };
  for (const Case &item : cases) {
    TempExe temp;
    CHECK(make_named_exe(temp, source, item.patch, item.timestamp));
    if (temp.path.empty())
      continue;
    Dummy named;
    if (!spawn_at(named, temp.path, false))
      continue;
    const std::wstring pid = std::to_wstring(named.pid);
    const std::wstring log_dir = log_directory(named.pid, L"id");
    Captured load = run_exe(production_ctl, L"load --pid " + pid + L" --dll " + quote(production_dll));
    Captured start = run_exe(production_ctl, L"start --pid " + pid + L" --dll " + quote(production_dll) +
                                                 L" --log-dir " + quote(log_dir));
    Captured status = run_exe(production_ctl, L"status --pid " + pid + L" --dll " + quote(production_dll));
    std::printf("observed %s load=%s start=%s status=%s rejected_2976=%s rejected_3576=%s\n", item.label,
                load.fields["ctl_identity"].c_str(), start.fields["ctl_identity"].c_str(),
                status.fields["ctl_identity"].c_str(), load.fields["ctl_rejected_2976"].c_str(),
                load.fields["ctl_rejected_3576"].c_str());
    if (item.identity != nullptr) {
      expect_unmodified_refusal(load, item.identity);
      expect_unmodified_refusal(start, item.identity);
      expect_unmodified_refusal(status, item.identity);
      expect(load, "ctl_rejected_2976", item.rejected_2976);
      expect(load, "ctl_rejected_3576", item.rejected_3576);
    } else {
      expect(load, "ctl_exit", "4");
      expect(load, "ctl_modified", "0");
      expect(load, "ctl_loaded", "0");
      expect(start, "ctl_exit", "4");
      expect(start, "ctl_modified", "0");
      expect(status, "ctl_exit", "4");
      expect(status, "ctl_modified", "0");
      CHECK(load.fields["ctl_identity"] != "match");
      CHECK(load.fields["ctl_identity"] == start.fields["ctl_identity"]);
    }
    CHECK(!module_mapped_once(named.pid, L"cdmp_pkt3663_live.dll"));
    CHECK(!module_mapped_once(named.pid, L"cdmp_pkt3663_live_fixture.dll"));
    Captured fixture = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(fixture, "ctl_exit", "4");
    expect(fixture, "ctl_note", "game-name");
    expect(fixture, "ctl_modified", "0");
    expect(fixture, "ctl_loaded", "0");
    CHECK(!module_mapped_once(named.pid, L"cdmp_pkt3663_live_fixture.dll"));
  }

  TempExe malformed_file;
  CHECK(make_named_exe(malformed_file, source, false, 0));
  Dummy malformed;
  if (!malformed_file.path.empty() && spawn_at(malformed, malformed_file.path, false)) {
    uintptr_t base = 0;
    CHECK(corrupt_lfanew(malformed.process, base));
    const std::wstring pid = std::to_wstring(malformed.pid);
    Captured load =
        run_exe(production_ctl, L"load --pid " + pid + L" --dll " + quote(production_dll));
    std::printf("observed malformed-image identity=%s note=%s\n", load.fields["ctl_identity"].c_str(),
                load.fields["ctl_note"].c_str());
    expect_unmodified_refusal(load, "malformed-image");
    expect(load, "ctl_rejected_2976", "0");
    expect(load, "ctl_rejected_3576", "0");
    uint32_t seen = 0;
    SIZE_T got = 0;
    CHECK(ReadProcessMemory(malformed.process, reinterpret_cast<void *>(base + 0x3C), &seen, sizeof seen, &got) &&
          got == sizeof seen && seen == 0x10);
    CHECK(!module_mapped_once(malformed.pid, L"cdmp_pkt3663_live.dll"));
    Captured fixture =
        run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
    expect(fixture, "ctl_exit", "4");
    expect(fixture, "ctl_note", "game-name");
    expect(fixture, "ctl_modified", "0");
    expect(fixture, "ctl_loaded", "0");
  }
}

constexpr int kStubReportError = 1;
constexpr int kStubReportUnreadable = 2;
constexpr int kStubReportTimeout = 3;
constexpr int kStubReportStale = 4;
constexpr int kStubDelayStart = 5;
constexpr DWORD kReportTimeoutWaitMs = 45000;

struct StagedDll {
  std::wstring directory;
  std::wstring path;
};

struct Launched {
  HANDLE process = nullptr;
  HANDLE thread = nullptr;
  std::wstring output_path;
  bool launched = false;
};

uintptr_t stub_pack(int arm, int stop_code, int mark, int mode) {
  return static_cast<uintptr_t>(arm & 0xFF) | (static_cast<uintptr_t>(stop_code & 0xFF) << 8) |
         (static_cast<uintptr_t>(mark & 0xFF) << 16) | (static_cast<uintptr_t>(mode & 0xFF) << 24);
}

uintptr_t remote_module_base(DWORD pid, const wchar_t *file_name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
  if (snap == INVALID_HANDLE_VALUE)
    return 0;
  MODULEENTRY32W entry = {};
  entry.dwSize = sizeof entry;
  uintptr_t base = 0;
  if (Module32FirstW(snap, &entry)) {
    do {
      if (_wcsicmp(entry.szModule, file_name) == 0) {
        base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
        break;
      }
    } while (Module32NextW(snap, &entry));
  }
  CloseHandle(snap);
  return base;
}

bool stage_fixture_copy(StagedDll &staged, const std::wstring &source, const wchar_t *tag) {
  wchar_t root[MAX_PATH] = {};
  if (GetTempPathW(MAX_PATH, root) == 0)
    return false;
  staged.directory = std::wstring(root) + L"pkt3663-ctl-stub-" + tag + L"-" + std::to_wstring(GetCurrentProcessId());
  if (!CreateDirectoryW(staged.directory.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    return false;
  staged.path = staged.directory + L"\\cdmp_pkt3663_live_fixture.dll";
  return CopyFileW(source.c_str(), staged.path.c_str(), FALSE) != FALSE;
}

void remove_staged(StagedDll &staged) {
  if (!staged.path.empty())
    DeleteFileW(staged.path.c_str());
  if (!staged.directory.empty())
    RemoveDirectoryW(staged.directory.c_str());
  staged.path.clear();
  staged.directory.clear();
}

bool call_remote_export(HANDLE process, DWORD pid, const std::wstring &dll_path, const char *export_name,
                        uintptr_t argument, DWORD *exit_code = nullptr) {
  HMODULE local = LoadLibraryW(dll_path.c_str());
  if (local == nullptr) {
    std::printf("FAIL LoadLibrary %ls %lu\n", dll_path.c_str(), GetLastError());
    ++g_failures;
    return false;
  }
  const auto proc = reinterpret_cast<uintptr_t>(GetProcAddress(local, export_name));
  const auto local_base = reinterpret_cast<uintptr_t>(local);
  FreeLibrary(local);
  if (proc <= local_base) {
    std::printf("FAIL export %s\n", export_name);
    ++g_failures;
    return false;
  }
  const uintptr_t rva = proc - local_base;
  uintptr_t remote_base = 0;
  for (int attempt = 0; attempt < 50 && remote_base == 0; ++attempt) {
    remote_base = remote_module_base(pid, L"cdmp_pkt3663_live_fixture.dll");
    if (remote_base == 0)
      Sleep(10);
  }
  if (remote_base == 0 || remote_base > UINTPTR_MAX - rva) {
    std::printf("FAIL remote module %s\n", export_name);
    ++g_failures;
    return false;
  }
  HANDLE thread =
      CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(remote_base + rva),
                         reinterpret_cast<void *>(argument), 0, nullptr);
  if (thread == nullptr) {
    std::printf("FAIL CreateRemoteThread %s %lu\n", export_name, GetLastError());
    ++g_failures;
    return false;
  }
  const DWORD waited = WaitForSingleObject(thread, 15000);
  DWORD code = 1;
  GetExitCodeThread(thread, &code);
  CloseHandle(thread);
  if (exit_code != nullptr)
    *exit_code = code;
  if (waited != WAIT_OBJECT_0) {
    std::printf("FAIL remote export %s wait %lu\n", export_name, waited);
    ++g_failures;
    return false;
  }
  return true;
}

void expect_exit(const Captured &captured, int code) {
  expect(captured, "ctl_exit", std::to_string(code));
  CHECK(captured.launched);
  CHECK(captured.exit_code == static_cast<DWORD>(code));
}

void expect_report_loss(const Captured &captured, int code, const char *note) {
  expect_exit(captured, code);
  expect(captured, "ctl_note", note);
  expect(captured, "ctl_raw_mode", "-1");
  expect(captured, "ctl_allowlist", "-1");
  expect(captured, "ctl_adopted", "-1");
  expect(captured, "ctl_retained", "0");
  expect(captured, "ctl_stored", "0");
  expect(captured, "ctl_unloaded", "0");
  expect(captured, "ctl_loaded", "1");
  expect(captured, "ctl_target_alive", "1");
}

bool load_and_configure(Dummy &dummy, const std::wstring &fixture_ctl, const std::wstring &dll, int arm, int stop_code,
                        int mark, int mode) {
  const std::wstring pid = std::to_wstring(dummy.pid);
  Captured loaded = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(dll));
  expect(loaded, "ctl_exit", "0");
  expect(loaded, "ctl_note", "loaded");
  expect(loaded, "ctl_loaded", "1");
  if (loaded.fields["ctl_exit"] != "0")
    return false;
  if (!call_remote_export(dummy.process, dummy.pid, dll, "pkt3663_stub_configure",
                          stub_pack(arm, stop_code, mark, mode)))
    return false;
  const bool mapped = module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll");
  CHECK(mapped);
  return mapped;
}

std::wstring temp_output_path(const wchar_t *tag) {
  wchar_t root[MAX_PATH] = {};
  GetTempPathW(MAX_PATH, root);
  static int serial = 0;
  return std::wstring(root) + L"pkt3663-ctl-out-" + tag + L"-" + std::to_wstring(GetCurrentProcessId()) + L"-" +
         std::to_wstring(++serial) + L".txt";
}

Launched launch_exe(const std::wstring &exe, const std::wstring &args, const std::wstring &output_path) {
  Launched launched;
  launched.output_path = output_path;
  SECURITY_ATTRIBUTES security = {};
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  HANDLE file = CreateFileW(output_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
  HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0,
                           nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    if (nul != nullptr && nul != INVALID_HANDLE_VALUE)
      CloseHandle(nul);
    std::printf("FAIL output file %lu\n", GetLastError());
    ++g_failures;
    return launched;
  }
  STARTUPINFOW startup = {};
  startup.cb = sizeof startup;
  startup.dwFlags = STARTF_USESTDHANDLES;
  startup.hStdInput = nul == INVALID_HANDLE_VALUE ? nullptr : nul;
  startup.hStdOutput = file;
  startup.hStdError = file;
  PROCESS_INFORMATION process = {};
  std::wstring command = quote(exe) + L" " + args;
  std::vector<wchar_t> mutable_command(command.begin(), command.end());
  mutable_command.push_back(L'\0');
  const BOOL created = CreateProcessW(exe.c_str(), mutable_command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                                      nullptr, nullptr, &startup, &process);
  CloseHandle(file);
  if (nul != nullptr && nul != INVALID_HANDLE_VALUE)
    CloseHandle(nul);
  if (!created) {
    std::printf("FAIL CreateProcess %ls error %lu\n", exe.c_str(), GetLastError());
    ++g_failures;
    return launched;
  }
  launched.process = process.hProcess;
  launched.thread = process.hThread;
  launched.launched = true;
  return launched;
}

Captured join_launched(Launched &launched, DWORD wait_ms) {
  Captured captured;
  if (!launched.launched || launched.process == nullptr)
    return captured;
  if (WaitForSingleObject(launched.process, wait_ms) == WAIT_TIMEOUT)
    TerminateProcess(launched.process, 1);
  DWORD code = 1;
  GetExitCodeProcess(launched.process, &code);
  captured.exit_code = code == STILL_ACTIVE ? 1 : code;
  captured.launched = true;
  captured.output = read_file(launched.output_path);
  captured.fields = parse_fields(captured.output);
  if (launched.thread != nullptr)
    CloseHandle(launched.thread);
  CloseHandle(launched.process);
  launched.thread = nullptr;
  launched.process = nullptr;
  DeleteFileW(launched.output_path.c_str());
  launched.output_path.clear();
  launched.launched = false;
  return captured;
}

void close_launched(Launched &launched) {
  if (launched.thread != nullptr) {
    CloseHandle(launched.thread);
    launched.thread = nullptr;
  }
  if (launched.process != nullptr) {
    CloseHandle(launched.process);
    launched.process = nullptr;
  }
  if (!launched.output_path.empty()) {
    DeleteFileW(launched.output_path.c_str());
    launched.output_path.clear();
  }
  launched.launched = false;
}

std::wstring make_log_dir(DWORD pid, const wchar_t *suffix) {
  const std::wstring directory = log_directory(pid, suffix);
  CreateDirectoryW(directory.c_str(), nullptr);
  return directory;
}

void test_stop_flush_failure(const std::wstring &fixture_ctl, const std::wstring &fixture_dll,
                             const std::wstring &dummy_exe) {
  Dummy dummy;
  if (!spawn_at(dummy, dummy_exe, false))
    return;
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log = make_log_dir(dummy.pid, L"badlog");
  Captured started = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                              L" --log-dir " + quote(log));
  expect(started, "ctl_exit", "0");
  expect(started, "ctl_start", "0");
  expect(started, "ctl_note", "started");
  if (started.fields["ctl_exit"] != "0")
    return;
  if (!call_remote_export(dummy.process, dummy.pid, fixture_dll, "pkt3663_live_fixture_fail_log_flush_after", 0))
    return;
  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect_exit(stopped, 5);
  expect(stopped, "ctl_stop", "6");
  expect(stopped, "ctl_note", "bad-log");
  expect(stopped, "ctl_retained", "0");
  expect(stopped, "ctl_raw_mode", "1");
  expect(stopped, "ctl_capture_attempts", "1");
  expect(stopped, "ctl_unloaded", "0");
  expect(stopped, "ctl_loaded", "1");
  expect(stopped, "ctl_target_alive", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  const std::string session = read_file(log + L"\\pkt3663-live-session.txt");
  const bool removed = session.find("shutdown=removed") != std::string::npos;
  const bool write_failed = session.find("session_write_failed=1") != std::string::npos;
  CHECK(removed);
  CHECK(write_failed);
  Captured report = run_exe(fixture_ctl, L"report --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(report, "ctl_exit", "0");
  expect(report, "ctl_stop", "4");
  expect(report, "ctl_note", "ok");
  expect(report, "ctl_retained", "0");
  Captured pinned = run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(pinned, "ctl_exit", "5");
  expect(pinned, "ctl_note", "pinned");
  expect(pinned, "ctl_unloaded", "0");
  expect(pinned, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  std::printf("observed stop-bad-log exit=%s stop=%s note=%s report_stop=%s shutdown_removed=%d "
              "session_write_failed=%d\n",
              stopped.fields["ctl_exit"].c_str(), stopped.fields["ctl_stop"].c_str(),
              stopped.fields["ctl_note"].c_str(), report.fields["ctl_stop"].c_str(), removed ? 1 : 0,
              write_failed ? 1 : 0);
}

void check_loss_commands(const std::wstring &fixture_ctl, Dummy &dummy, const std::wstring &dll, const char *note,
                         int arm, int stop_code, int expected_exit, DWORD wait_ms) {
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log = make_log_dir(dummy.pid, L"loss");
  Captured started =
      run_exe(fixture_ctl,
              L"start --fixture --pid " + pid + L" --dll " + quote(dll) + L" --log-dir " + quote(log), wait_ms);
  expect_report_loss(started, expected_exit, note);
  expect(started, "ctl_start", std::to_string(arm));
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(dll), wait_ms);
  expect_report_loss(stopped, expected_exit, note);
  expect(stopped, "ctl_stop", std::to_string(stop_code));
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  Captured marked = run_exe(fixture_ctl,
                            L"mark --fixture --pid " + pid + L" --dll " + quote(dll) + L" --phase stand", wait_ms);
  expect_report_loss(marked, expected_exit, note);
  CHECK(marked.fields["ctl_note"] != "marked");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
}

void test_repeated_start(const std::wstring &fixture_ctl, const std::wstring &fixture_dll,
                         const std::wstring &dummy_exe) {
  Dummy dummy;
  if (!spawn_at(dummy, dummy_exe, false))
    return;
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log = make_log_dir(dummy.pid, L"repeat");
  const std::wstring start_args = L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                  L" --log-dir " + quote(log);
  Captured started = run_exe(fixture_ctl, start_args);
  expect(started, "ctl_exit", "0");
  expect(started, "ctl_start", "0");
  expect(started, "ctl_note", "started");
  expect(started, "ctl_capture_attempts", "1");
  expect(started, "ctl_loaded", "1");
  expect(started, "ctl_unloaded", "0");
  if (started.fields["ctl_exit"] != "0")
    return;
  CHECK(file_exists(log + L"\\pkt3663-live-records.txt"));
  Captured again = run_exe(fixture_ctl, start_args);
  expect_exit(again, 5);
  expect(again, "ctl_start", "1");
  expect(again, "ctl_note", "refused");
  expect(again, "ctl_capture_attempts", "1");
  expect(again, "ctl_retained", "0");
  expect(again, "ctl_loaded", "1");
  expect(again, "ctl_unloaded", "0");
  expect(again, "ctl_target_alive", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  CHECK(file_exists(log + L"\\pkt3663-live-records.txt"));
  Captured report = run_exe(fixture_ctl, L"report --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(report, "ctl_exit", "0");
  expect(report, "ctl_start", "0");
  expect(report, "ctl_note", "ok");
  expect(report, "ctl_capture_attempts", "1");
  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(stopped, "ctl_exit", "0");
  expect(stopped, "ctl_stop", "4");
  expect(stopped, "ctl_note", "removed");
  expect(stopped, "ctl_capture_attempts", "1");
  expect(stopped, "ctl_loaded", "1");
  expect(stopped, "ctl_unloaded", "0");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  std::printf("observed repeated-start exit=%s start=%s note=%s attempts=%s stop=%s\n",
              again.fields["ctl_exit"].c_str(), again.fields["ctl_start"].c_str(),
              again.fields["ctl_note"].c_str(), again.fields["ctl_capture_attempts"].c_str(),
              stopped.fields["ctl_stop"].c_str());
}

void test_required_reports(const std::wstring &fixture_ctl, const std::wstring &stub, const std::wstring &dummy_exe) {
  StagedDll error_dll;
  StagedDll unreadable_dll;
  StagedDll retained_dll;
  StagedDll timeout_dll;
  if (!stage_fixture_copy(error_dll, stub, L"error") || !stage_fixture_copy(unreadable_dll, stub, L"unreadable") ||
      !stage_fixture_copy(retained_dll, stub, L"retained") || !stage_fixture_copy(timeout_dll, stub, L"timeout")) {
    std::printf("FAIL stage report stub\n");
    ++g_failures;
    remove_staged(error_dll);
    remove_staged(unreadable_dll);
    remove_staged(retained_dll);
    remove_staged(timeout_dll);
    return;
  }

  Dummy error_dummy;
  if (spawn_at(error_dummy, dummy_exe, false) &&
      load_and_configure(error_dummy, fixture_ctl, error_dll.path, kPkt3663LiveStarted, kPkt3663LiveIdle, 0,
                         kStubReportError)) {
    check_loss_commands(fixture_ctl, error_dummy, error_dll.path, "report-unreadable", kPkt3663LiveStarted,
                        kPkt3663LiveIdle, 5, 30000);
    const std::wstring pid = std::to_wstring(error_dummy.pid);
    Captured unload =
        run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(error_dll.path));
    expect(unload, "ctl_exit", "5");
    expect(unload, "ctl_note", "report-unreadable");
    expect(unload, "ctl_unloaded", "0");
    expect(unload, "ctl_loaded", "1");
    CHECK(module_mapped_once(error_dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
    std::printf("observed report-error start=%s stop=%s unload=%s\n", "5", "5",
                unload.fields["ctl_exit"].c_str());
  }

  Dummy unreadable_dummy;
  if (spawn_at(unreadable_dummy, dummy_exe, false) &&
      load_and_configure(unreadable_dummy, fixture_ctl, unreadable_dll.path, kPkt3663LiveStarted, kPkt3663LiveIdle, 0,
                         kStubReportUnreadable)) {
    check_loss_commands(fixture_ctl, unreadable_dummy, unreadable_dll.path, "report-unreadable", kPkt3663LiveStarted,
                        kPkt3663LiveIdle, 5, 30000);
    std::printf("observed report-unreadable exit=5 note=report-unreadable\n");
  }

  Dummy retained_dummy;
  if (spawn_at(retained_dummy, dummy_exe, false) &&
      load_and_configure(retained_dummy, fixture_ctl, retained_dll.path, kPkt3663LiveRetained, kPkt3663LiveRetained,
                         kPkt3663LiveRetained, kStubReportError)) {
    check_loss_commands(fixture_ctl, retained_dummy, retained_dll.path, "report-unreadable", kPkt3663LiveRetained,
                        kPkt3663LiveRetained, 3, 30000);
    std::printf("observed report-error-retained exit=3 note=report-unreadable\n");
  }

  Dummy timeout_start;
  Dummy timeout_stop;
  Dummy timeout_mark;
  Dummy timeout_retained;
  const bool timeout_ready =
      spawn_at(timeout_start, dummy_exe, false) && spawn_at(timeout_stop, dummy_exe, false) &&
      spawn_at(timeout_mark, dummy_exe, false) && spawn_at(timeout_retained, dummy_exe, false) &&
      load_and_configure(timeout_start, fixture_ctl, timeout_dll.path, kPkt3663LiveStarted, kPkt3663LiveIdle, 0,
                         kStubReportTimeout) &&
      load_and_configure(timeout_stop, fixture_ctl, timeout_dll.path, kPkt3663LiveStarted, kPkt3663LiveIdle, 0,
                         kStubReportTimeout) &&
      load_and_configure(timeout_mark, fixture_ctl, timeout_dll.path, kPkt3663LiveStarted, kPkt3663LiveIdle, 0,
                         kStubReportTimeout) &&
      load_and_configure(timeout_retained, fixture_ctl, timeout_dll.path, kPkt3663LiveRetained, kPkt3663LiveRetained,
                         kPkt3663LiveRetained, kStubReportTimeout);
  if (timeout_ready) {
    const std::wstring start_log = make_log_dir(timeout_start.pid, L"timeout");
    Launched start_job = launch_exe(
        fixture_ctl, L"start --fixture --pid " + std::to_wstring(timeout_start.pid) + L" --dll " +
                         quote(timeout_dll.path) + L" --log-dir " + quote(start_log),
        temp_output_path(L"timeout-start"));
    Launched stop_job =
        launch_exe(fixture_ctl,
                   L"stop --fixture --pid " + std::to_wstring(timeout_stop.pid) + L" --dll " + quote(timeout_dll.path),
                   temp_output_path(L"timeout-stop"));
    Launched mark_job = launch_exe(fixture_ctl,
                                   L"mark --fixture --pid " + std::to_wstring(timeout_mark.pid) + L" --dll " +
                                       quote(timeout_dll.path) + L" --phase walk",
                                   temp_output_path(L"timeout-mark"));
    Launched retained_job = launch_exe(
        fixture_ctl,
        L"stop --fixture --pid " + std::to_wstring(timeout_retained.pid) + L" --dll " + quote(timeout_dll.path),
        temp_output_path(L"timeout-retained"));
    Captured started = join_launched(start_job, kReportTimeoutWaitMs);
    Captured stopped = join_launched(stop_job, kReportTimeoutWaitMs);
    Captured marked = join_launched(mark_job, kReportTimeoutWaitMs);
    Captured retained = join_launched(retained_job, kReportTimeoutWaitMs);
    expect_report_loss(started, 5, "remote-timeout");
    expect(started, "ctl_start", "0");
    expect_report_loss(stopped, 5, "remote-timeout");
    expect(stopped, "ctl_stop", "5");
    expect_report_loss(marked, 5, "remote-timeout");
    CHECK(marked.fields["ctl_note"] != "marked");
    expect_report_loss(retained, 3, "remote-timeout");
    expect(retained, "ctl_stop", "3");
    CHECK(module_mapped_once(timeout_start.pid, L"cdmp_pkt3663_live_fixture.dll"));
    CHECK(module_mapped_once(timeout_stop.pid, L"cdmp_pkt3663_live_fixture.dll"));
    CHECK(module_mapped_once(timeout_mark.pid, L"cdmp_pkt3663_live_fixture.dll"));
    CHECK(module_mapped_once(timeout_retained.pid, L"cdmp_pkt3663_live_fixture.dll"));
    std::printf("observed report-timeout start=%s stop=%s mark=%s retained_exit=%s retained_stop=%s\n",
                started.fields["ctl_exit"].c_str(), stopped.fields["ctl_exit"].c_str(),
                marked.fields["ctl_exit"].c_str(), retained.fields["ctl_exit"].c_str(),
                retained.fields["ctl_stop"].c_str());
    close_launched(start_job);
    close_launched(stop_job);
    close_launched(mark_job);
    close_launched(retained_job);
  }

  error_dummy.close();
  unreadable_dummy.close();
  retained_dummy.close();
  timeout_start.close();
  timeout_stop.close();
  timeout_mark.close();
  timeout_retained.close();
  remove_staged(error_dll);
  remove_staged(unreadable_dll);
  remove_staged(retained_dll);
  remove_staged(timeout_dll);
}

void test_delayed_start(const std::wstring &fixture_ctl, const std::wstring &stub,
                        const std::wstring &dummy_exe) {
  StagedDll staged;
  if (!stage_fixture_copy(staged, stub, L"delay")) {
    std::printf("FAIL stage delayed stub\n");
    ++g_failures;
    return;
  }
  Dummy dummy;
  if (!spawn_at(dummy, dummy_exe, false) ||
      !load_and_configure(dummy, fixture_ctl, staged.path, 0, kPkt3663LiveIdle, 0, kStubDelayStart)) {
    dummy.close();
    remove_staged(staged);
    return;
  }
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log = make_log_dir(dummy.pid, L"delay");
  Captured started =
      run_exe(fixture_ctl,
              L"start --fixture --pid " + pid + L" --dll " + quote(staged.path) + L" --log-dir " + quote(log),
              40000);
  expect_report_loss(started, 5, "remote-timeout");
  expect(started, "ctl_start", "-1");
  expect(started, "ctl_thread_created", "1");
  expect(started, "ctl_remote_wait", "timeout");
  const int wait_ms = std::atoi(started.fields["ctl_remote_wait_ms"].c_str());
  CHECK(wait_ms >= 20000 && wait_ms < 30000);
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  DWORD alive = 0;
  CHECK(GetExitCodeProcess(dummy.process, &alive) && alive == STILL_ACTIVE);

  DWORD witness = 0;
  bool finished = false;
  for (int attempt = 0; attempt < 40 && !finished; ++attempt) {
    DWORD code = 0;
    if (call_remote_export(dummy.process, dummy.pid, staged.path, "pkt3663_stub_delayed_start_result", 0, &code) &&
        (code & 1u) != 0) {
      witness = code;
      finished = true;
      break;
    }
    Sleep(500);
  }
  CHECK(finished);
  CHECK((witness & 2u) != 0);
  CHECK(((witness >> 8) & 0xFFu) == 1u);
  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(staged.path));
  expect(stopped, "ctl_exit", "0");
  expect(stopped, "ctl_stop", "5");
  expect(stopped, "ctl_note", "idle");
  expect(stopped, "ctl_unloaded", "0");
  expect(stopped, "ctl_loaded", "1");
  Captured unload = run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(staged.path));
  expect(unload, "ctl_exit", "5");
  expect(unload, "ctl_note", "pinned");
  expect(unload, "ctl_unloaded", "0");
  expect(unload, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  CHECK(GetExitCodeProcess(dummy.process, &alive) && alive == STILL_ACTIVE);
  std::printf("observed delayed-start exit=%s wait=%s witness=0x%lx\n", started.fields["ctl_exit"].c_str(),
              started.fields["ctl_remote_wait"].c_str(), witness);
  dummy.close();
  remove_staged(staged);
}

void test_stale_report(const std::wstring &fixture_ctl, const std::wstring &stub, const std::wstring &dummy_exe) {
  StagedDll staged;
  if (!stage_fixture_copy(staged, stub, L"stale")) {
    std::printf("FAIL stage stale stub\n");
    ++g_failures;
    return;
  }
  Dummy dummy;
  if (!spawn_at(dummy, dummy_exe, false) ||
      !load_and_configure(dummy, fixture_ctl, staged.path, kPkt3663LiveRetained, kPkt3663LiveRetained, 0,
                          kStubReportStale)) {
    dummy.close();
    remove_staged(staged);
    return;
  }
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log = make_log_dir(dummy.pid, L"stale");
  Captured started = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(staged.path) +
                                              L" --log-dir " + quote(log));
  expect_exit(started, 3);
  expect(started, "ctl_start", "3");
  expect(started, "ctl_note", "retained");
  expect(started, "ctl_raw_mode", "1");
  expect(started, "ctl_qpc_frequency", "1");
  expect(started, "ctl_retained", "0");
  expect(started, "ctl_unloaded", "0");
  expect(started, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(staged.path));
  expect_exit(stopped, 3);
  expect(stopped, "ctl_stop", "3");
  expect(stopped, "ctl_note", "retained");
  expect(stopped, "ctl_raw_mode", "1");
  expect(stopped, "ctl_unloaded", "0");
  expect(stopped, "ctl_loaded", "1");
  if (!call_remote_export(dummy.process, dummy.pid, staged.path, "pkt3663_stub_configure",
                          stub_pack(kPkt3663LiveRetained, kPkt3663LiveBadLog, 0, kStubReportStale))) {
    dummy.close();
    remove_staged(staged);
    return;
  }
  Captured bad_log = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(staged.path));
  expect_exit(bad_log, 5);
  expect(bad_log, "ctl_stop", "6");
  expect(bad_log, "ctl_note", "bad-log");
  expect(bad_log, "ctl_raw_mode", "1");
  expect(bad_log, "ctl_qpc_frequency", "1");
  expect(bad_log, "ctl_retained", "0");
  expect(bad_log, "ctl_unloaded", "0");
  expect(bad_log, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  std::printf("observed stale-report start=%s stop=%s badlog=%s badlog_stop=%s\n",
              started.fields["ctl_exit"].c_str(), stopped.fields["ctl_stop"].c_str(),
              bad_log.fields["ctl_exit"].c_str(), bad_log.fields["ctl_stop"].c_str());
  dummy.close();
  remove_staged(staged);
}

void test_flow() {
  if (real_game_running()) {
    std::printf("FAIL Crimson Desert is running; controller tests will not attach\n");
    ++g_failures;
    return;
  }
  const std::wstring production_ctl = beside(L"cdmp_pkt3663_live_ctl.exe");
  const std::wstring fixture_ctl = beside(L"cdmp_pkt3663_live_ctl_fixture.exe");
  const std::wstring production_dll = beside(L"cdmp_pkt3663_live.dll");
  const std::wstring fixture_dll = beside(L"cdmp_pkt3663_live_fixture.dll");
  const std::wstring dummy_exe = beside(L"cdmp_pkt3663_live_dummy.exe");
  CHECK(!production_ctl.empty() && !fixture_ctl.empty() && !production_dll.empty() && !fixture_dll.empty() &&
        !dummy_exe.empty());

  test_named_identity(production_ctl, fixture_ctl, production_dll, fixture_dll, dummy_exe);

  Dummy dummy;
  if (!spawn_at(dummy, dummy_exe, false))
    return;
  const std::wstring pid = std::to_wstring(dummy.pid);
  const std::wstring log_dir = log_directory(dummy.pid, L"main");
  CreateDirectoryW(log_dir.c_str(), nullptr);

  Captured rejected = run_exe(production_ctl, L"load --pid " + pid + L" --dll " + quote(production_dll));
  expect(rejected, "ctl_exit", "4");
  expect(rejected, "ctl_identity", "wrong-name");
  expect(rejected, "ctl_gate", "rejected");
  expect(rejected, "ctl_modified", "0");
  expect(rejected, "ctl_loaded", "0");
  expect(rejected, "ctl_note", "wrong-name");
  CHECK(!module_mapped_once(dummy.pid, L"cdmp_pkt3663_live.dll"));

  Captured rejected_start = run_exe(production_ctl, L"start --pid " + pid + L" --dll " + quote(production_dll) +
                                                        L" --log-dir " + quote(log_dir));
  expect(rejected_start, "ctl_exit", "4");
  expect(rejected_start, "ctl_modified", "0");
  expect(rejected_start, "ctl_loaded", "0");

  Captured fixture_flag = run_exe(production_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                                      L" --log-dir " + quote(log_dir));
  expect(fixture_flag, "ctl_exit", "2");
  expect(fixture_flag, "ctl_note", "usage");
  expect(fixture_flag, "ctl_modified", "0");
  expect(fixture_flag, "ctl_target_alive", "-1");

  Captured relative_dll = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll cdmp_pkt3663_live_fixture.dll");
  expect(relative_dll, "ctl_exit", "2");
  expect(relative_dll, "ctl_note", "bad-dll");
  expect(relative_dll, "ctl_modified", "0");
  expect(relative_dll, "ctl_target_alive", "-1");

  std::wstring long_dir = L"C:\\";
  long_dir.append(1100, L'a');
  Captured long_log = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                               L" --log-dir " + quote(long_dir));
  expect(long_log, "ctl_exit", "5");
  expect(long_log, "ctl_start", "6");
  expect(long_log, "ctl_note", "bad-log");
  expect(long_log, "ctl_modified", "0");
  expect(long_log, "ctl_target_alive", "-1");

  Captured bad_log = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                              L" --log-dir logs");
  expect(bad_log, "ctl_exit", "5");
  expect(bad_log, "ctl_start", "6");
  expect(bad_log, "ctl_modified", "0");
  expect(bad_log, "ctl_loaded", "0");
  expect(bad_log, "ctl_note", "bad-log");
  expect(bad_log, "ctl_gate", "fixture");
  Captured bin64 = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                            L" --log-dir " + quote(std::wstring(L"C:\\Crimson Desert\\bin64\\logs")));
  expect(bin64, "ctl_exit", "5");
  expect(bin64, "ctl_start", "6");
  expect(bin64, "ctl_modified", "0");
  expect(bin64, "ctl_loaded", "0");
  CHECK(!module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));

  Captured not_loaded = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(not_loaded, "ctl_exit", "0");
  expect(not_loaded, "ctl_note", "not-loaded");
  expect(not_loaded, "ctl_modified", "0");
  expect(not_loaded, "ctl_loaded", "0");
  expect(not_loaded, "ctl_raw_mode", "-1");
  Captured report_missing = run_exe(fixture_ctl, L"report --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(report_missing, "ctl_exit", "5");
  expect(report_missing, "ctl_note", "not-loaded");
  expect(report_missing, "ctl_modified", "0");
  Captured stop_missing = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(stop_missing, "ctl_exit", "5");
  expect(stop_missing, "ctl_note", "not-loaded");
  expect(stop_missing, "ctl_modified", "0");
  Captured unload_missing = run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(unload_missing, "ctl_exit", "5");
  expect(unload_missing, "ctl_note", "not-loaded");
  expect(unload_missing, "ctl_modified", "0");
  expect(unload_missing, "ctl_unloaded", "0");

  Captured loaded = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(loaded, "ctl_exit", "0");
  expect(loaded, "ctl_loaded", "1");
  expect(loaded, "ctl_modified", "1");
  expect(loaded, "ctl_gate", "fixture");
  expect(loaded, "ctl_note", "loaded");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  CHECK(!module_mapped_once(dummy.pid, L"cdmp_pkt3663_live.dll"));

  Captured unloaded = run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(unloaded, "ctl_exit", "0");
  expect(unloaded, "ctl_unloaded", "1");
  expect(unloaded, "ctl_loaded", "0");
  expect(unloaded, "ctl_note", "unloaded");
  expect(unloaded, "ctl_modified", "1");
  CHECK(!module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  DWORD alive = 0;
  CHECK(GetExitCodeProcess(dummy.process, &alive) && alive == STILL_ACTIVE);

  Captured loaded_again = run_exe(fixture_ctl, L"load --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(loaded_again, "ctl_exit", "0");
  expect(loaded_again, "ctl_note", "loaded");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));

  wchar_t temp_root[MAX_PATH] = {};
  GetTempPathW(MAX_PATH, temp_root);
  const std::wstring copied_dll =
      std::wstring(temp_root) + L"pkt3663-ctl-copy-" + pid + L"\\cdmp_pkt3663_live_fixture.dll";
  CreateDirectoryW((std::wstring(temp_root) + L"pkt3663-ctl-copy-" + pid).c_str(), nullptr);
  CHECK(CopyFileW(fixture_dll.c_str(), copied_dll.c_str(), FALSE));
  Captured mismatch = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(copied_dll));
  expect(mismatch, "ctl_exit", "4");
  expect(mismatch, "ctl_note", "module-mismatch");
  expect(mismatch, "ctl_modified", "0");
  expect(mismatch, "ctl_loaded", "0");
  Captured still = run_exe(fixture_ctl, L"status --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(still, "ctl_exit", "0");
  expect(still, "ctl_loaded", "1");
  expect(still, "ctl_note", "ok");
  expect(still, "ctl_capture_attempts", "0");
  expect(still, "ctl_raw_mode", "1");
  expect(still, "ctl_allowlist", "0");
  expect(still, "ctl_adopted", "0");

  const std::wstring stand_file = log_dir + L"\\pkt3663-live-records-stand.txt";
  CHECK(CreateDirectoryW(log_dir.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS);
  FILE *stand = nullptr;
  CHECK(_wfopen_s(&stand, stand_file.c_str(), L"wb") == 0 && stand != nullptr);
  if (stand != nullptr) {
    std::fputs("preexisting\n", stand);
    std::fclose(stand);
  }
  Captured marked = run_exe(fixture_ctl, L"mark --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                             L" --phase stand");
  expect(marked, "ctl_exit", "0");
  expect(marked, "ctl_note", "marked");
  expect(marked, "ctl_phase", "1");
  expect(marked, "ctl_modified", "1");
  Captured blocked = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                              L" --log-dir " + quote(log_dir));
  expect(blocked, "ctl_exit", "5");
  expect(blocked, "ctl_start", "6");
  expect(blocked, "ctl_note", "bad-log");
  expect(blocked, "ctl_capture_attempts", "0");
  expect(blocked, "ctl_loaded", "1");
  expect(blocked, "ctl_unloaded", "0");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  const std::string preexisting = read_file(stand_file);
  CHECK(preexisting.find("preexisting") != std::string::npos);

  Captured walk = run_exe(fixture_ctl, L"mark --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                           L" --phase walk");
  expect(walk, "ctl_exit", "0");
  expect(walk, "ctl_note", "marked");
  expect(walk, "ctl_phase", "2");
  Captured started = run_exe(fixture_ctl, L"start --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                              L" --log-dir " + quote(log_dir));
  expect(started, "ctl_exit", "0");
  expect(started, "ctl_start", "0");
  expect(started, "ctl_note", "started");
  expect(started, "ctl_capture_attempts", "1");
  expect(started, "ctl_phase", "2");
  expect(started, "ctl_raw_mode", "1");
  expect(started, "ctl_allowlist", "0");
  expect(started, "ctl_adopted", "0");
  expect(started, "ctl_retained", "0");
  expect(started, "ctl_loaded", "1");
  expect(started, "ctl_unloaded", "0");
  CHECK(file_exists(log_dir + L"\\pkt3663-live-records-walk.txt"));
  CHECK(!file_exists(log_dir + L"\\pkt3663-live-records-turn.txt"));
  const std::string session = read_file(log_dir + L"\\pkt3663-live-session.txt");
  CHECK(session.find("fixture=1") != std::string::npos);
  CHECK(session.find("game_match=0") != std::string::npos);
  CHECK(session.find("game_match=1") == std::string::npos);
  CHECK(session.find("raw=1") != std::string::npos);
  CHECK(session.find("allowlist_count=0") != std::string::npos);
  CHECK(session.find("adopted=0") != std::string::npos);
  CHECK(session.find("recording_ready=1") != std::string::npos);
  CHECK(session.find("mark phase=walk qpc=") != std::string::npos);
  if (g_failures != 0 && session.empty())
    std::printf("FAIL empty fixture session\n");

  Captured report = run_exe(fixture_ctl, L"report --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(report, "ctl_exit", "0");
  expect(report, "ctl_note", "ok");
  expect(report, "ctl_capture_attempts", "1");
  expect(report, "ctl_phase", "2");
  expect(report, "ctl_raw_mode", "1");
  expect(report, "ctl_allowlist", "0");
  expect(report, "ctl_adopted", "0");
  expect(report, "ctl_start", "0");

  Captured mark_active = run_exe(fixture_ctl, L"mark --fixture --pid " + pid + L" --dll " + quote(fixture_dll) +
                                                  L" --phase turn");
  expect(mark_active, "ctl_exit", "5");
  expect(mark_active, "ctl_note", "refused");
  expect(mark_active, "ctl_phase", "2");
  CHECK(!file_exists(log_dir + L"\\pkt3663-live-records-turn.txt"));

  Captured stopped = run_exe(fixture_ctl, L"stop --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(stopped, "ctl_exit", "0");
  expect(stopped, "ctl_stop", "4");
  expect(stopped, "ctl_note", "removed");
  expect(stopped, "ctl_retained", "0");
  expect(stopped, "ctl_unloaded", "0");
  expect(stopped, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  Captured pinned = run_exe(fixture_ctl, L"unload --fixture --pid " + pid + L" --dll " + quote(fixture_dll));
  expect(pinned, "ctl_exit", "5");
  expect(pinned, "ctl_note", "pinned");
  expect(pinned, "ctl_unloaded", "0");
  expect(pinned, "ctl_loaded", "1");
  CHECK(module_mapped_once(dummy.pid, L"cdmp_pkt3663_live_fixture.dll"));
  CHECK(GetExitCodeProcess(dummy.process, &alive) && alive == STILL_ACTIVE);

  Dummy crowded;
  if (spawn_at(crowded, dummy_exe, true)) {
    CHECK(crowded.spawned >= 280);
    const std::wstring crowded_pid = std::to_wstring(crowded.pid);
    const std::wstring crowded_log = log_directory(crowded.pid, L"retain");
    Captured armed = run_exe(fixture_ctl, L"start --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll) +
                                              L" --log-dir " + quote(crowded_log));
    std::printf("observed crowded-start exit=%s start=%s note=%s\n", armed.fields["ctl_exit"].c_str(),
                armed.fields["ctl_start"].c_str(), armed.fields["ctl_note"].c_str());
    const bool started_ok = armed.fields["ctl_exit"] == "0" && armed.fields["ctl_start"] == "0";
    const bool start_retained = armed.fields["ctl_exit"] == "3" && armed.fields["ctl_start"] == "3";
    CHECK(started_ok || start_retained);
    expect(armed, "ctl_loaded", "1");
    expect(armed, "ctl_unloaded", "0");
    expect(armed, "ctl_target_alive", "1");
    const std::string retained_session = read_file(crowded_log + L"\\pkt3663-live-session.txt");
    CHECK(retained_session.find("fixture=1") != std::string::npos);
    CHECK(retained_session.find("game_match=0") != std::string::npos);
    CHECK(retained_session.find("game_match=1") == std::string::npos);
    Captured retained = run_exe(fixture_ctl, L"stop --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll));
    std::printf("observed crowded-stop exit=%s stop=%s retained=%s note=%s\n", retained.fields["ctl_exit"].c_str(),
                retained.fields["ctl_stop"].c_str(), retained.fields["ctl_retained"].c_str(),
                retained.fields["ctl_note"].c_str());
    expect(retained, "ctl_exit", "3");
    expect(retained, "ctl_retained", "1");
    expect(retained, "ctl_unloaded", "0");
    expect(retained, "ctl_loaded", "1");
    expect(retained, "ctl_target_alive", "1");
    expect(retained, "ctl_note", "retained");
    CHECK(module_mapped_once(crowded.pid, L"cdmp_pkt3663_live_fixture.dll"));
    Captured retained_unload =
        run_exe(fixture_ctl, L"unload --fixture --pid " + crowded_pid + L" --dll " + quote(fixture_dll));
    expect(retained_unload, "ctl_exit", "3");
    expect(retained_unload, "ctl_note", "retained");
    expect(retained_unload, "ctl_unloaded", "0");
    expect(retained_unload, "ctl_loaded", "1");
    DWORD crowded_alive = 0;
    CHECK(GetExitCodeProcess(crowded.process, &crowded_alive) && crowded_alive == STILL_ACTIVE);
  }

  Dummy exiting;
  if (spawn_at(exiting, dummy_exe, false)) {
    const std::wstring exiting_pid = std::to_wstring(exiting.pid);
    Captured exit_loaded =
        run_exe(fixture_ctl, L"load --fixture --pid " + exiting_pid + L" --dll " + quote(fixture_dll));
    expect(exit_loaded, "ctl_exit", "0");
    expect(exit_loaded, "ctl_loaded", "1");
    CHECK(TerminateProcess(exiting.process, 77));
    WaitForSingleObject(exiting.process, 5000);
    Captured after = run_exe(fixture_ctl, L"unload --fixture --pid " + exiting_pid + L" --dll " + quote(fixture_dll));
    std::printf("observed process-exit note=%s modified=%s unloaded=%s alive=%s\n", after.fields["ctl_note"].c_str(),
                after.fields["ctl_modified"].c_str(), after.fields["ctl_unloaded"].c_str(),
                after.fields["ctl_target_alive"].c_str());
    expect(after, "ctl_exit", "5");
    expect(after, "ctl_note", "process-exited");
    expect(after, "ctl_modified", "0");
    expect(after, "ctl_unloaded", "0");
    expect(after, "ctl_target_alive", "0");
    DWORD exit_code = 0;
    CHECK(GetExitCodeProcess(exiting.process, &exit_code) && exit_code == 77);
    Captured after_report =
        run_exe(fixture_ctl, L"report --fixture --pid " + exiting_pid + L" --dll " + quote(fixture_dll));
    expect(after_report, "ctl_exit", "5");
    expect(after_report, "ctl_note", "process-exited");
    expect(after_report, "ctl_modified", "0");
    expect(after_report, "ctl_unloaded", "0");
  }

  CHECK(!real_game_running());
}

void test_controller_regressions() {
  if (real_game_running()) {
    std::printf("FAIL Crimson Desert is running; controller tests will not attach\n");
    ++g_failures;
    return;
  }
  const std::wstring fixture_ctl = beside(L"cdmp_pkt3663_live_ctl_fixture.exe");
  const std::wstring fixture_dll = beside(L"cdmp_pkt3663_live_fixture.dll");
  const std::wstring dummy_exe = beside(L"cdmp_pkt3663_live_dummy.exe");
  const std::wstring stub = beside(L"cdmp_pkt3663_live_ctl_stub.dll");
  CHECK(!fixture_ctl.empty() && !fixture_dll.empty() && !dummy_exe.empty() && !stub.empty());
  if (fixture_ctl.empty() || fixture_dll.empty() || dummy_exe.empty() || stub.empty())
    return;
  test_stop_flush_failure(fixture_ctl, fixture_dll, dummy_exe);
  test_repeated_start(fixture_ctl, fixture_dll, dummy_exe);
  test_delayed_start(fixture_ctl, stub, dummy_exe);
  test_required_reports(fixture_ctl, stub, dummy_exe);
  test_stale_report(fixture_ctl, stub, dummy_exe);
  CHECK(!real_game_running());
}

} // namespace

int main() {
  test_policy();
  test_pid_bounds();
  test_usage_parser();
  if (!policy_only()) {
    test_exports();
    test_flow();
    test_controller_regressions();
  }
  if (g_failures != 0) {
    std::printf("pkt3663 live ctl tests failed: %d\n", g_failures);
    return 1;
  }
  std::printf("%s\n", policy_only() ? "pkt3663 live ctl policy ok" : "pkt3663 live ctl tests ok");
  return 0;
}
