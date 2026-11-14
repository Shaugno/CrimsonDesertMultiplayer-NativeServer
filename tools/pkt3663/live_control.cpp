// External loader for cdmp_pkt3663_live.dll.
// An explicit PID is identified before any remote allocation, write, or thread.
// Retained and pinned modules stay mapped. This file never terminates the
// target and does not launch Crimson Desert.

#include "pkt3663/live_control.h"

#include "pkt3663/assess.h"

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>

namespace pkt3663::control {
namespace {

constexpr DWORD kRemoteWaitMs = 20000;
constexpr int kMaxModules = 1024;
constexpr uint32_t kFixtureQueue = 8;
constexpr uint32_t kFixtureRecords = 8;
constexpr uint64_t kRemoteReadBudget =
    static_cast<uint64_t>(kSizeOfImage3663) * 2ull + (1024ull * 1024ull);
constexpr DWORD kPathChars = 32768;

struct Outcome {
  const char *action = "";
  unsigned long pid = 0;
  const char *identity = "none";
  int rejected_2976 = 0;
  int rejected_3576 = 0;
  const char *gate = "none";
  int modified = 0;
  int loaded = 0;
  int start = -1;
  int stop = -1;
  int retained = 0;
  unsigned stored = 0;
  unsigned omitted = 0;
  unsigned unread = 0;
  unsigned truncated = 0;
  int raw_mode = -1;
  int allowlist = -1;
  int adopted = -1;
  unsigned phase = 0;
  unsigned attempts = 0;
  unsigned long long qpc_frequency = 0;
  int unloaded = 0;
  int alive = -1;
  int thread_created = 0;
  int remote_wait_ms = -1;
  const char *remote_wait = "none";
  const char *note = "";
  int exit_code = kExitFailed;
  bool have_base = false;
  unsigned long long dll_base = 0;
};

struct RemoteModule {
  uintptr_t base = 0;
  wchar_t path[MAX_PATH] = {};
  wchar_t name[MAX_MODULE_NAME32 + 1] = {};
};

struct RemoteBytes {
  HANDLE process = nullptr;
  uint64_t used = 0;
};

struct Session {
  Outcome out;
  HANDLE read = nullptr;
  HANDLE write = nullptr;
  ~Session();
};

struct RemoteAlloc {
  HANDLE process = nullptr;
  void *remote = nullptr;
  bool keep = false;
  void release() {
    if (keep || remote == nullptr || process == nullptr) {
      remote = nullptr;
      return;
    }
    DWORD code = 0;
    if (!GetExitCodeProcess(process, &code) || code != STILL_ACTIVE) {
      remote = nullptr;
      return;
    }
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    remote = nullptr;
  }
  ~RemoteAlloc() { release(); }
};

struct Call {
  RemoteWait wait = RemoteWait::Failed;
  int code = -1;
  int thread_created = 0;
  int wait_ms = -1;
  const char *wait_name = "none";
};

void remember_command_wait(Outcome &out, const Call &call) {
  out.thread_created = call.thread_created;
  out.remote_wait_ms = call.wait_ms;
  out.remote_wait = call.wait_name;
}

uint64_t qpc_now() {
  LARGE_INTEGER now{};
  if (!QueryPerformanceCounter(&now) || now.QuadPart <= 0)
    return 0;
  return static_cast<uint64_t>(now.QuadPart);
}

int elapsed_ms(uint64_t began, uint64_t ended) {
  LARGE_INTEGER frequency{};
  if (began == 0 || ended < began || !QueryPerformanceFrequency(&frequency) ||
      frequency.QuadPart <= 0) {
    return 0;
  }
  const uint64_t delta = ended - began;
  const uint64_t freq = static_cast<uint64_t>(frequency.QuadPart);
  if (delta > UINT64_MAX / 1000ull)
    return INT_MAX;
  const uint64_t ms = (delta * 1000ull) / freq;
  return ms > static_cast<uint64_t>(INT_MAX) ? INT_MAX : static_cast<int>(ms);
}

struct Command {
  DWORD pid = 0;
  const wchar_t *dll = nullptr;
  const wchar_t *log_dir = nullptr;
  const wchar_t *phase = nullptr;
  bool fixture = false;
};

enum class ReportRead { Ok, Timeout, Failed };

void set_note(Outcome &out, const char *note) {
  if (out.note == nullptr || out.note[0] == '\0')
    out.note = note;
}

void print_outcome(const Outcome &out) {
  std::printf("ctl_action=%s\n", out.action);
  std::printf("ctl_pid=%lu\n", out.pid);
  std::printf("ctl_identity=%s\n", out.identity);
  std::printf("ctl_rejected_2976=%d\n", out.rejected_2976);
  std::printf("ctl_rejected_3576=%d\n", out.rejected_3576);
  std::printf("ctl_gate=%s\n", out.gate);
  std::printf("ctl_modified=%d\n", out.modified);
  std::printf("ctl_loaded=%d\n", out.loaded);
  std::printf("ctl_start=%d\n", out.start);
  std::printf("ctl_stop=%d\n", out.stop);
  std::printf("ctl_retained=%d\n", out.retained);
  std::printf("ctl_stored=%u\n", out.stored);
  std::printf("ctl_omitted=%u\n", out.omitted);
  std::printf("ctl_unread=%u\n", out.unread);
  std::printf("ctl_truncated=%u\n", out.truncated);
  std::printf("ctl_raw_mode=%d\n", out.raw_mode);
  std::printf("ctl_allowlist=%d\n", out.allowlist);
  std::printf("ctl_adopted=%d\n", out.adopted);
  std::printf("ctl_phase=%u\n", out.phase);
  std::printf("ctl_capture_attempts=%u\n", out.attempts);
  std::printf("ctl_qpc_frequency=%llu\n", out.qpc_frequency);
  std::printf("ctl_unloaded=%d\n", out.unloaded);
  std::printf("ctl_target_alive=%d\n", out.alive);
  std::printf("ctl_note=%s\n", out.note);
  std::printf("ctl_thread_created=%d\n", out.thread_created);
  std::printf("ctl_remote_wait_ms=%d\n", out.remote_wait_ms);
  std::printf("ctl_remote_wait=%s\n", out.remote_wait);
  if (out.have_base)
    std::printf("ctl_dll_base=0x%llx\n", out.dll_base);
  std::printf("ctl_exit=%d\n", out.exit_code);
}

bool process_still_alive(HANDLE process) {
  if (process == nullptr)
    return false;
  DWORD code = 0;
  return GetExitCodeProcess(process, &code) && code == STILL_ACTIVE;
}

void observe_alive(Outcome &out, HANDLE process) {
  if (process == nullptr)
    return;
  out.alive = process_still_alive(process) ? 1 : 0;
}

Session::~Session() {
  observe_alive(out, read != nullptr ? read : write);
  print_outcome(out);
  if (write != nullptr)
    CloseHandle(write);
  if (read != nullptr)
    CloseHandle(read);
}

int finish(Session &session, int code) {
  session.out.exit_code = code;
  return code;
}

bool absolute_path(const wchar_t *path) {
  if (path == nullptr || path[0] == L'\0')
    return false;
  if ((path[0] == L'\\' && path[1] == L'\\') ||
      (path[0] == L'/' && path[1] == L'/'))
    return true;
  const bool drive = (path[0] >= L'A' && path[0] <= L'Z') ||
                     (path[0] >= L'a' && path[0] <= L'z');
  return drive && path[1] == L':' && (path[2] == L'\\' || path[2] == L'/');
}

bool same_text(const wchar_t *left, const wchar_t *right) {
  return left != nullptr && right != nullptr && _wcsicmp(left, right) == 0;
}

const char *note_for_code(int code) {
  switch (code) {
  case kPkt3663LiveStarted:
    return "started";
  case kPkt3663LiveRefused:
    return "refused";
  case kPkt3663LiveRolledBack:
    return "rolled-back";
  case kPkt3663LiveRetained:
    return "retained";
  case kPkt3663LiveRemoved:
    return "removed";
  case kPkt3663LiveIdle:
    return "idle";
  case kPkt3663LiveBadLog:
    return "bad-log";
  default:
    return "failed";
  }
}

bool take_value(int argc, wchar_t **argv, int &index, const wchar_t *&value) {
  if (index + 1 >= argc || argv[index + 1] == nullptr ||
      argv[index + 1][0] == L'\0')
    return false;
  value = argv[++index];
  return true;
}

// Usage returns before any process is opened.
bool parse_command(int argc, wchar_t **argv, bool fixture_build,
                   const char *fixture_arm_export, Command &command, Outcome &out,
                   const char *&note) {
  note = "usage";
  if (argv == nullptr || argc < 2 || argv[1] == nullptr)
    return false;
  if (same_text(argv[1], L"load"))
    out.action = "load";
  else if (same_text(argv[1], L"start"))
    out.action = "start";
  else if (same_text(argv[1], L"status"))
    out.action = "status";
  else if (same_text(argv[1], L"report"))
    out.action = "report";
  else if (same_text(argv[1], L"stop"))
    out.action = "stop";
  else if (same_text(argv[1], L"mark"))
    out.action = "mark";
  else if (same_text(argv[1], L"unload"))
    out.action = "unload";
  else
    return false;

  const wchar_t *pid_text = nullptr;
  bool saw_pid = false;
  bool saw_dll = false;
  bool saw_log = false;
  bool saw_fixture = false;
  bool saw_phase = false;
  for (int index = 2; index < argc; ++index) {
    const wchar_t *arg = argv[index];
    if (arg == nullptr)
      return false;
    if (same_text(arg, L"--pid")) {
      if (saw_pid || !take_value(argc, argv, index, pid_text))
        return false;
      saw_pid = true;
    } else if (same_text(arg, L"--dll")) {
      if (saw_dll || !take_value(argc, argv, index, command.dll))
        return false;
      saw_dll = true;
    } else if (same_text(arg, L"--log-dir")) {
      if (saw_log || !take_value(argc, argv, index, command.log_dir))
        return false;
      saw_log = true;
    } else if (same_text(arg, L"--phase")) {
      if (saw_phase || !take_value(argc, argv, index, command.phase))
        return false;
      saw_phase = true;
    } else if (same_text(arg, L"--fixture")) {
      if (saw_fixture)
        return false;
      saw_fixture = true;
    } else {
      return false;
    }
  }

  command.fixture = saw_fixture;
  if (saw_fixture && (!fixture_build || fixture_arm_export == nullptr))
    return false;
  if (!saw_pid || !parse_pid(pid_text, command.pid)) {
    note = "bad-pid";
    return false;
  }
  out.pid = command.pid;
  if (command.pid == GetCurrentProcessId()) {
    note = "self-pid";
    return false;
  }
  if (!saw_dll || !absolute_path(command.dll) || wcslen(command.dll) >= kPathChars ||
      !dll_file_allowed(command.dll, command.fixture)) {
    note = "bad-dll";
    return false;
  }

  const bool starting = same_text(argv[1], L"start");
  const bool marking = same_text(argv[1], L"mark");
  if (marking) {
    if (!saw_phase || command.phase == nullptr || saw_log)
      return false;
    const bool known = same_text(command.phase, L"stand") ||
                       same_text(command.phase, L"walk") ||
                       same_text(command.phase, L"turn") ||
                       same_text(command.phase, L"unlabeled");
    if (!known) {
      note = "bad-phase";
      return false;
    }
  } else if (saw_phase) {
    return false;
  } else if (starting) {
    if (!saw_log || command.log_dir == nullptr)
      return false;
    if (wcslen(command.log_dir) >= 1024) {
      note = "bad-log";
      out.start = kPkt3663LiveBadLog;
      out.exit_code = kExitFailed;
      return false;
    }
  } else if (saw_log) {
    return false;
  }
  return true;
}

const wchar_t *file_name_of_copy(const wchar_t *path) {
  const wchar_t *name = path;
  if (path == nullptr)
    return L"";
  for (const wchar_t *cursor = path; *cursor != L'\0'; ++cursor) {
    if (*cursor == L'\\' || *cursor == L'/')
      name = cursor + 1;
  }
  return name;
}

bool snapshot_modules(DWORD pid, std::vector<RemoteModule> &modules) {
  for (int attempt = 0; attempt < 4; ++attempt) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE) {
      const DWORD error = GetLastError();
      if (error == ERROR_BAD_LENGTH || error == ERROR_PARTIAL_COPY)
        continue;
      return false;
    }
    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof entry;
    if (!Module32FirstW(snap, &entry)) {
      const DWORD error = GetLastError();
      CloseHandle(snap);
      if (error == ERROR_BAD_LENGTH || error == ERROR_PARTIAL_COPY)
        continue;
      return false;
    }
    modules.clear();
    bool partial = false;
    do {
      if (modules.size() >= static_cast<size_t>(kMaxModules)) {
        partial = true;
        break;
      }
      RemoteModule item;
      item.base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
      wcsncpy_s(item.path, entry.szExePath, _TRUNCATE);
      wcsncpy_s(item.name, entry.szModule, _TRUNCATE);
      modules.push_back(item);
    } while (Module32NextW(snap, &entry));
    const DWORD error = GetLastError();
    CloseHandle(snap);
    if (partial)
      return false;
    if (error == ERROR_NO_MORE_FILES)
      return !modules.empty();
    if (error == ERROR_BAD_LENGTH || error == ERROR_PARTIAL_COPY)
      continue;
    return false;
  }
  return false;
}

bool normalized_path(const wchar_t *path, wchar_t *output, DWORD output_chars) {
  if (path == nullptr || output == nullptr || output_chars < 2)
    return false;
  const DWORD length = GetFullPathNameW(path, output_chars, output, nullptr);
  return length != 0 && length < output_chars;
}

bool paths_equal(const wchar_t *left, const wchar_t *right) {
  wchar_t left_full[kPathChars] = {};
  wchar_t right_full[kPathChars] = {};
  if (!normalized_path(left, left_full, kPathChars) ||
      !normalized_path(right, right_full, kPathChars))
    return false;
  return _wcsicmp(left_full, right_full) == 0;
}

bool query_image_path(HANDLE process, wchar_t *output, DWORD output_chars) {
  DWORD chars = output_chars;
  if (process == nullptr || output == nullptr)
    return false;
  if (!QueryFullProcessImageNameW(process, 0, output, &chars))
    return false;
  return chars > 0 && chars < output_chars;
}

int count_named(const std::vector<RemoteModule> &modules, const wchar_t *file_name,
                uintptr_t &base, const wchar_t *&path) {
  int matches = 0;
  base = 0;
  path = nullptr;
  for (const RemoteModule &module : modules) {
    if (_wcsicmp(module.name, file_name) != 0)
      continue;
    ++matches;
    base = module.base;
    path = module.path;
  }
  return matches;
}

bool remote_read(HANDLE process, uintptr_t address, void *destination, size_t size) {
  if (process == nullptr || destination == nullptr || size == 0 || size > kMappedReadCap)
    return false;
  if (address == 0 || address > UINTPTR_MAX - size)
    return false;
  SIZE_T got = 0;
  if (!ReadProcessMemory(process, reinterpret_cast<const void *>(address), destination, size, &got))
    return false;
  return got == size;
}

bool remote_image_read(uintptr_t address, void *destination, size_t size, void *user) {
  auto *state = static_cast<RemoteBytes *>(user);
  if (state == nullptr || size == 0 || size > kMappedReadCap)
    return false;
  if (state->used > std::numeric_limits<uint64_t>::max() - size)
    return false;
  if (state->used + static_cast<uint64_t>(size) > kRemoteReadBudget)
    return false;
  if (!remote_read(state->process, address, destination, size))
    return false;
  state->used += static_cast<uint64_t>(size);
  return true;
}

bool native_x64(HANDLE process) {
  USHORT process_machine = IMAGE_FILE_MACHINE_UNKNOWN;
  USHORT native_machine = IMAGE_FILE_MACHINE_UNKNOWN;
  if (IsWow64Process2(process, &process_machine, &native_machine)) {
    return process_machine == IMAGE_FILE_MACHINE_UNKNOWN &&
           native_machine == IMAGE_FILE_MACHINE_AMD64;
  }
  BOOL wow = TRUE;
  if (!IsWow64Process(process, &wow))
    return false;
  return wow == FALSE;
}

bool add_rva(uintptr_t base, uint32_t rva, uint32_t size_of_image, uintptr_t &out) {
  if (rva == 0 || size_of_image == 0 || rva >= size_of_image)
    return false;
  if (base > UINTPTR_MAX - static_cast<uintptr_t>(rva))
    return false;
  out = base + static_cast<uintptr_t>(rva);
  return true;
}

// Walks the remote export table. Forwarded exports are refused.
bool find_export(HANDLE process, uintptr_t module, const char *name, uintptr_t &out) {
  out = 0;
  if (process == nullptr || module == 0 || name == nullptr || name[0] == '\0')
    return false;
  IMAGE_DOS_HEADER dos = {};
  if (!remote_read(process, module, &dos, sizeof dos))
    return false;
  if (dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0x40 || dos.e_lfanew > 1024)
    return false;
  const uintptr_t nt_address = module + static_cast<uintptr_t>(dos.e_lfanew);
  IMAGE_NT_HEADERS64 headers = {};
  if (!remote_read(process, nt_address, &headers, sizeof headers))
    return false;
  if (headers.Signature != IMAGE_NT_SIGNATURE || headers.OptionalHeader.Magic != 0x20B)
    return false;
  const IMAGE_DATA_DIRECTORY &directory =
      headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  const uint32_t size_of_image = headers.OptionalHeader.SizeOfImage;
  if (directory.VirtualAddress == 0 || directory.Size < sizeof(IMAGE_EXPORT_DIRECTORY))
    return false;
  if (directory.Size > size_of_image || directory.VirtualAddress > size_of_image - directory.Size)
    return false;
  uintptr_t export_address = 0;
  if (!add_rva(module, directory.VirtualAddress, size_of_image, export_address))
    return false;
  IMAGE_EXPORT_DIRECTORY exports = {};
  if (!remote_read(process, export_address, &exports, sizeof exports))
    return false;
  if (exports.NumberOfNames == 0 || exports.NumberOfNames > 4096 ||
      exports.NumberOfFunctions == 0 || exports.NumberOfFunctions > 4096)
    return false;

  uintptr_t names = 0;
  uintptr_t ordinals = 0;
  uintptr_t functions = 0;
  if (!add_rva(module, exports.AddressOfNames, size_of_image, names) ||
      !add_rva(module, exports.AddressOfNameOrdinals, size_of_image, ordinals) ||
      !add_rva(module, exports.AddressOfFunctions, size_of_image, functions)) {
    return false;
  }

  for (uint32_t index = 0; index < exports.NumberOfNames; ++index) {
    uint32_t name_rva = 0;
    const uintptr_t name_slot = names + static_cast<uintptr_t>(index) * sizeof(uint32_t);
    if (name_slot < names || !remote_read(process, name_slot, &name_rva, sizeof name_rva))
      return false;
    uintptr_t name_address = 0;
    if (!add_rva(module, name_rva, size_of_image, name_address))
      continue;
    char remote_name[256] = {};
    if (!remote_read(process, name_address, remote_name, sizeof remote_name - 1))
      continue;
    remote_name[sizeof remote_name - 1] = '\0';
    if (std::strcmp(remote_name, name) != 0)
      continue;

    uint16_t ordinal = 0;
    const uintptr_t ordinal_slot = ordinals + static_cast<uintptr_t>(index) * sizeof(uint16_t);
    if (ordinal_slot < ordinals || !remote_read(process, ordinal_slot, &ordinal, sizeof ordinal))
      return false;
    if (ordinal >= exports.NumberOfFunctions)
      return false;
    uint32_t function_rva = 0;
    const uintptr_t function_slot = functions + static_cast<uintptr_t>(ordinal) * sizeof(uint32_t);
    if (function_slot < functions ||
        !remote_read(process, function_slot, &function_rva, sizeof function_rva)) {
      return false;
    }
    if (function_rva >= directory.VirtualAddress &&
        function_rva < directory.VirtualAddress + directory.Size) {
      return false;
    }
    return add_rva(module, function_rva, size_of_image, out);
  }
  return false;
}

// Uses the module that actually contains the procedure. GetProcAddress resolves
// a forwarder, so the owner can be kernelbase rather than kernel32.
uintptr_t kernel_export(const std::vector<RemoteModule> &modules, const char *name) {
  HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
  if (kernel == nullptr || name == nullptr)
    return 0;
  const auto procedure = reinterpret_cast<void *>(GetProcAddress(kernel, name));
  if (procedure == nullptr)
    return 0;
  HMODULE owner = nullptr;
  void *address = procedure;
  LPCWSTR key = nullptr;
  static_assert(sizeof address == sizeof key);
  std::memcpy(&key, &address, sizeof key);
  if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         key, &owner) == FALSE ||
      owner == nullptr) {
    return 0;
  }
  wchar_t owner_path[MAX_PATH] = {};
  if (GetModuleFileNameW(owner, owner_path, MAX_PATH) == 0)
    return 0;
  uintptr_t remote_base = 0;
  const wchar_t *remote_path = nullptr;
  if (count_named(modules, file_name_of_copy(owner_path), remote_base, remote_path) != 1)
    return 0;
  const auto local_proc = reinterpret_cast<uintptr_t>(procedure);
  const auto local_base = reinterpret_cast<uintptr_t>(owner);
  if (local_proc <= local_base)
    return 0;
  const uintptr_t delta = local_proc - local_base;
  if (remote_base > UINTPTR_MAX - delta)
    return 0;
  return remote_base + delta;
}

Call call_remote(Session &session, HANDLE process, uintptr_t procedure, void *argument) {
  Call call;
  if (process == nullptr || procedure == 0 || !process_still_alive(process))
    return call;
  const uint64_t began = qpc_now();
  HANDLE thread = CreateRemoteThread(process, nullptr, 0,
                                     reinterpret_cast<LPTHREAD_START_ROUTINE>(procedure),
                                     argument, 0, nullptr);
  if (thread == nullptr) {
    call.thread_created = 0;
    call.wait_ms = elapsed_ms(began, qpc_now());
    call.wait_name = "failed";
    return call;
  }
  // The wait expiring does not cancel this thread. Its argument stays allocated.
  call.thread_created = 1;
  session.out.modified = 1;
  const DWORD waited = WaitForSingleObject(thread, kRemoteWaitMs);
  call.wait_ms = elapsed_ms(began, qpc_now());
  if (waited != WAIT_OBJECT_0) {
    CloseHandle(thread);
    call.wait = RemoteWait::Timeout;
    call.wait_name = "timeout";
    return call;
  }
  DWORD code = 0;
  GetExitCodeThread(thread, &code);
  CloseHandle(thread);
  call.wait = RemoteWait::Ok;
  call.wait_name = "ok";
  call.code = static_cast<int>(code);
  return call;
}

bool ensure_write(Session &session, DWORD pid) {
  if (session.write != nullptr)
    return true;
  if (!process_still_alive(session.read))
    return false;
  session.write =
      OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
                      PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION,
                  FALSE, pid);
  return session.write != nullptr;
}

void remember_module(Outcome &out, uintptr_t base) {
  out.loaded = 1;
  out.have_base = base != 0;
  out.dll_base = base;
}

void refresh_loaded(Session &session, DWORD pid, const wchar_t *dll_path) {
  std::vector<RemoteModule> modules;
  if (!snapshot_modules(pid, modules))
    return;
  uintptr_t base = 0;
  const wchar_t *path = nullptr;
  const int matches = count_named(modules, file_name_of_copy(dll_path), base, path);
  if (matches == 1 && path != nullptr && paths_equal(path, dll_path))
    remember_module(session.out, base);
  else {
    session.out.loaded = 0;
    session.out.have_base = false;
    session.out.dll_base = 0;
  }
}

bool write_bytes(Session &session, HANDLE process, void *remote, const void *data, size_t size) {
  SIZE_T wrote = 0;
  if (remote == nullptr || data == nullptr || size == 0 || size > kPathChars * sizeof(wchar_t))
    return false;
  if (!process_still_alive(process))
    return false;
  if (!WriteProcessMemory(process, remote, data, size, &wrote) || wrote != size)
    return false;
  session.out.modified = 1;
  return true;
}

bool load_module(Session &session, DWORD pid, HANDLE process, uintptr_t load_library,
                 const wchar_t *dll_path, uintptr_t &base) {
  const size_t chars = wcslen(dll_path) + 1;
  RemoteAlloc memory;
  memory.process = process;
  memory.remote =
      VirtualAllocEx(process, nullptr, chars * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (memory.remote == nullptr)
    return false;
  session.out.modified = 1;
  if (!write_bytes(session, process, memory.remote, dll_path, chars * sizeof(wchar_t)))
    return false;
  const Call call = call_remote(session, process, load_library, memory.remote);
  remember_command_wait(session.out, call);
  if (keep_remote_allocation(call.wait)) {
    memory.keep = true;
    set_note(session.out, "remote-timeout");
    return false;
  }
  if (call.wait != RemoteWait::Ok)
    return false;
  for (int attempt = 0; attempt < 5; ++attempt) {
    std::vector<RemoteModule> modules;
    uintptr_t found = 0;
    const wchar_t *path = nullptr;
    if (snapshot_modules(pid, modules) &&
        count_named(modules, file_name_of_copy(dll_path), found, path) == 1 && path != nullptr &&
        paths_equal(path, dll_path)) {
      base = found;
      remember_module(session.out, base);
      return true;
    }
    Sleep(10);
  }
  return false;
}

ReportRead read_report(Session &session, HANDLE process, uintptr_t report_export,
                       Pkt3663LiveReport &report) {
  RemoteAlloc memory;
  memory.process = process;
  memory.remote =
      VirtualAllocEx(process, nullptr, sizeof report, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (memory.remote == nullptr) {
    set_note(session.out, "remote-alloc");
    return ReportRead::Failed;
  }
  session.out.modified = 1;
  const Pkt3663LiveReport zero{};
  if (!write_bytes(session, process, memory.remote, &zero, sizeof zero)) {
    set_note(session.out, "remote-write");
    return ReportRead::Failed;
  }
  const Call call = call_remote(session, process, report_export, memory.remote);
  if (keep_remote_allocation(call.wait)) {
    memory.keep = true;
    set_note(session.out, "remote-timeout");
    return ReportRead::Timeout;
  }
  if (call.wait != RemoteWait::Ok || call.code != 0) {
    set_note(session.out, "report-unreadable");
    return ReportRead::Failed;
  }
  SIZE_T got = 0;
  if (!process_still_alive(process) ||
      !ReadProcessMemory(process, memory.remote, &report, sizeof report, &got) ||
      got != sizeof report) {
    set_note(session.out, process_still_alive(process) ? "report-unreadable" : "process-exited");
    return ReportRead::Failed;
  }
  return ReportRead::Ok;
}

void apply_report(Outcome &out, const Pkt3663LiveReport &report) {
  out.start = report.start_code;
  out.stop = report.stop_code;
  out.retained = report.retained;
  out.stored = report.stored;
  out.omitted = report.omitted;
  out.unread = report.unread;
  out.truncated = report.truncated;
  out.raw_mode = static_cast<int>(report.raw_mode);
  out.allowlist = static_cast<int>(report.allowlist_count);
  out.adopted = static_cast<int>(report.adopted);
  out.phase = report.phase;
  out.attempts = report.capture_attempts;
  out.qpc_frequency = report.qpc_frequency;
}

int unload_mapped(Session &session, DWORD pid, HANDLE process, uintptr_t free_library,
                  uintptr_t base, const wchar_t *dll_path, bool success_exit) {
  if (!process_still_alive(process)) {
    set_note(session.out, "process-exited");
    return finish(session, kExitFailed);
  }
  const Call call = call_remote(session, process, free_library, reinterpret_cast<void *>(base));
  remember_command_wait(session.out, call);
  if (keep_remote_allocation(call.wait)) {
    set_note(session.out, "remote-timeout");
    return finish(session, kExitFailed);
  }
  if (call.wait != RemoteWait::Ok) {
    set_note(session.out, "unload-failed");
    return finish(session, kExitFailed);
  }
  for (int attempt = 0; attempt < 5; ++attempt) {
    refresh_loaded(session, pid, dll_path);
    if (session.out.loaded == 0) {
      session.out.unloaded = 1;
      session.out.note = "unloaded";
      return finish(session, success_exit ? kExitOk : kExitFailed);
    }
    Sleep(10);
  }
  set_note(session.out, "unload-failed");
  return finish(session, kExitFailed);
}

bool action_is(const Outcome &out, const char *name) {
  return std::strcmp(out.action, name) == 0;
}

int execute(Session &session, const Command &command, bool fixture_build,
            const char *fixture_arm_export) {
  constexpr DWORD kReadAccess = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ;
  session.read = OpenProcess(kReadAccess, FALSE, command.pid);
  if (session.read == nullptr) {
    set_note(session.out, "open-failed");
    return finish(session, kExitFailed);
  }
  if (!process_still_alive(session.read)) {
    set_note(session.out, "process-exited");
    return finish(session, kExitFailed);
  }
  if (!native_x64(session.read)) {
    set_note(session.out, "not-64-bit");
    session.out.gate = "rejected";
    return finish(session, kExitRejected);
  }

  std::vector<RemoteModule> modules;
  if (!snapshot_modules(command.pid, modules)) {
    set_note(session.out, "snapshot-failed");
    return finish(session, kExitFailed);
  }
  wchar_t image_path[kPathChars] = {};
  if (!query_image_path(session.read, image_path, kPathChars) ||
      !paths_equal(modules[0].path, image_path)) {
    set_note(session.out, "image-mismatch");
    session.out.gate = "rejected";
    return finish(session, kExitRejected);
  }

  RemoteBytes bytes;
  bytes.process = session.read;
  const Identity identity =
      identify_image(modules[0].name, modules[0].base, remote_image_read, &bytes);
  session.out.identity = identity_kind_name(identity.kind);
  session.out.rejected_2976 = identity.rejected_2976 ? 1 : 0;
  session.out.rejected_3576 = identity.rejected_3576 ? 1 : 0;
  const bool game_name = game_file_name(image_path);

  uintptr_t dll_base = 0;
  const wchar_t *dll_path = nullptr;
  const int dll_matches =
      count_named(modules, file_name_of_copy(command.dll), dll_base, dll_path);
  if (dll_matches > 1) {
    set_note(session.out, "module-ambiguous");
    session.out.gate = "rejected";
    return finish(session, kExitRejected);
  }
  if (dll_matches == 1 && (dll_path == nullptr || !paths_equal(dll_path, command.dll))) {
    set_note(session.out, "module-mismatch");
    session.out.gate = "rejected";
    return finish(session, kExitRejected);
  }
  const bool present = dll_matches == 1;

  ControlAction action = ControlAction::Load;
  if (action_is(session.out, "start"))
    action = ControlAction::Start;
  else if (action_is(session.out, "status"))
    action = ControlAction::Status;
  else if (action_is(session.out, "report"))
    action = ControlAction::Report;
  else if (action_is(session.out, "stop"))
    action = ControlAction::Stop;
  else if (action_is(session.out, "mark"))
    action = ControlAction::Mark;
  else if (action_is(session.out, "unload"))
    action = ControlAction::Unload;

  const bool allowed =
      control_allowed(fixture_build, command.fixture, identity.kind, game_name, action, present);
  if (!allowed) {
    session.out.gate = "rejected";
    set_note(session.out, command.fixture && game_name ? "game-name" : identity_kind_name(identity.kind));
    return finish(session, kExitRejected);
  }
  const bool pristine =
      modification_allowed(fixture_build, command.fixture, identity.kind, game_name);
  session.out.gate = command.fixture ? "fixture" : (pristine ? "match" : "installed");
  if (present)
    remember_module(session.out, dll_base);

  const bool starting = action == ControlAction::Start;
  if (starting && !log_directory_allowed(image_path, command.log_dir)) {
    session.out.start = kPkt3663LiveBadLog;
    set_note(session.out, "bad-log");
    return finish(session, kExitFailed);
  }

  uintptr_t send_entry = 0;
  uintptr_t recv_entry = 0;
  if (starting && command.fixture) {
    if (!find_export(session.read, modules[0].base, "pkt3663_dummy_send", send_entry) ||
        !find_export(session.read, modules[0].base, "pkt3663_dummy_receive", recv_entry) ||
        send_entry == recv_entry) {
      set_note(session.out, "missing-entry");
      return finish(session, kExitFailed);
    }
  }

  const bool querying = action == ControlAction::Status || action == ControlAction::Report ||
                        action == ControlAction::Stop || action == ControlAction::Mark ||
                        action == ControlAction::Unload;
  if (querying && !present) {
    set_note(session.out, "not-loaded");
    return finish(session, action == ControlAction::Status ? kExitOk : kExitFailed);
  }
  if (action == ControlAction::Load && present) {
    set_note(session.out, "already-loaded");
    return finish(session, kExitOk);
  }
  if (!process_still_alive(session.read)) {
    set_note(session.out, "process-exited");
    return finish(session, kExitFailed);
  }
  if (!ensure_write(session, command.pid)) {
    set_note(session.out, process_still_alive(session.read) ? "open-failed" : "process-exited");
    return finish(session, kExitFailed);
  }

  const uintptr_t load_library = kernel_export(modules, "LoadLibraryW");
  const uintptr_t free_library = kernel_export(modules, "FreeLibrary");
  if (load_library == 0 || free_library == 0) {
    set_note(session.out, "missing-kernel");
    return finish(session, kExitFailed);
  }

  const bool loaded_here = !present;
  if (!present) {
    if (!load_module(session, command.pid, session.write, load_library, command.dll, dll_base)) {
      set_note(session.out, "load-failed");
      refresh_loaded(session, command.pid, command.dll);
      return finish(session, kExitFailed);
    }
  }

  if (action == ControlAction::Load) {
    session.out.note = "loaded";
    return finish(session, kExitOk);
  }

  auto cleanup_failed_load = [&](int start_code, int retained, unsigned attempts) -> int {
    if (loaded_here && process_still_alive(session.write) &&
        unload_after_observed(true, RemoteWait::Ok, true, start_code, retained, attempts)) {
      return unload_mapped(session, command.pid, session.write, free_library, dll_base, command.dll,
                           false);
    }
    return finish(session, kExitFailed);
  };

  uintptr_t report_export = 0;
  if (!find_export(session.read, dll_base, "pkt3663_live_report", report_export)) {
    set_note(session.out, "missing-export");
    return cleanup_failed_load(session.out.start, session.out.retained, session.out.attempts);
  }

  if (action == ControlAction::Status || action == ControlAction::Report) {
    Pkt3663LiveReport report{};
    const ReportRead read = read_report(session, session.write, report_export, report);
    if (read != ReportRead::Ok)
      return finish(session, kExitFailed);
    apply_report(session.out, report);
    session.out.note = "ok";
    return finish(session, kExitOk);
  }

  if (action == ControlAction::Unload) {
    Pkt3663LiveReport report{};
    const ReportRead read = read_report(session, session.write, report_export, report);
    if (read == ReportRead::Timeout)
      return finish(session, kExitFailed);
    if (read != ReportRead::Ok)
      return finish(session, kExitFailed);
    apply_report(session.out, report);
    if (session.out.retained != 0 || session.out.start == kPkt3663LiveRetained) {
      session.out.note = "retained";
      return finish(session, kExitRetained);
    }
    if (!unload_allowed(session.out.start, session.out.retained, session.out.attempts)) {
      session.out.note = "pinned";
      return finish(session, kExitFailed);
    }
    return unload_mapped(session, command.pid, session.write, free_library, dll_base, command.dll,
                         true);
  }

  if (action == ControlAction::Stop) {
    uintptr_t stop_export = 0;
    if (!find_export(session.read, dll_base, "pkt3663_live_stop", stop_export)) {
      set_note(session.out, "missing-export");
      return finish(session, kExitFailed);
    }
    const Call call = call_remote(session, session.write, stop_export, nullptr);
    remember_command_wait(session.out, call);
    if (keep_remote_allocation(call.wait)) {
      set_note(session.out, "remote-timeout");
      return finish(session, kExitFailed);
    }
    if (call.wait != RemoteWait::Ok) {
      set_note(session.out, "failed");
      return finish(session, kExitFailed);
    }
    const int invoked = call.code;
    session.out.stop = invoked;
    Pkt3663LiveReport report{};
    const ReportRead read = read_report(session, session.write, report_export, report);
    if (read != ReportRead::Ok)
      return finish(session, exit_without_report(invoked));
    apply_report(session.out, report);
    // The snapshot can still describe older cleanup. It does not replace the
    // stop call's own return, except that Retained on either side stays.
    session.out.stop = command_result_after_report(invoked, report.stop_code);
    if (session.out.stop == kPkt3663LiveRetained || session.out.retained != 0) {
      session.out.note = "retained";
      return finish(session, kExitRetained);
    }
    session.out.note = note_for_code(session.out.stop);
    if (session.out.stop == kPkt3663LiveRemoved || session.out.stop == kPkt3663LiveIdle)
      return finish(session, kExitOk);
    return finish(session, kExitFailed);
  }

  if (action == ControlAction::Mark) {
    uintptr_t mark_export = 0;
    if (!find_export(session.read, dll_base, "pkt3663_live_mark", mark_export)) {
      set_note(session.out, "missing-export");
      return cleanup_failed_load(-1, 0, 0);
    }
    const char *phase_text = nullptr;
    if (same_text(command.phase, L"stand"))
      phase_text = "stand";
    else if (same_text(command.phase, L"walk"))
      phase_text = "walk";
    else if (same_text(command.phase, L"turn"))
      phase_text = "turn";
    else if (same_text(command.phase, L"unlabeled"))
      phase_text = "unlabeled";
    if (phase_text == nullptr) {
      set_note(session.out, "bad-phase");
      return finish(session, kExitUsage);
    }
    const size_t phase_bytes = std::strlen(phase_text) + 1;
    RemoteAlloc argument;
    argument.process = session.write;
    argument.remote = VirtualAllocEx(session.write, nullptr, phase_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (argument.remote == nullptr) {
      set_note(session.out, "remote-alloc");
      return finish(session, kExitFailed);
    }
    session.out.modified = 1;
    if (!write_bytes(session, session.write, argument.remote, phase_text, phase_bytes)) {
      set_note(session.out, "remote-write");
      return finish(session, kExitFailed);
    }
    const Call call = call_remote(session, session.write, mark_export, argument.remote);
    remember_command_wait(session.out, call);
    if (keep_remote_allocation(call.wait)) {
      argument.keep = true;
      set_note(session.out, "remote-timeout");
      return finish(session, kExitFailed);
    }
    if (call.wait != RemoteWait::Ok) {
      set_note(session.out, "failed");
      return finish(session, kExitFailed);
    }
    const int invoked = call.code;
    Pkt3663LiveReport report{};
    const ReportRead read = read_report(session, session.write, report_export, report);
    if (read != ReportRead::Ok)
      return finish(session, exit_without_report(invoked));
    apply_report(session.out, report);
    if (invoked == 0) {
      session.out.note = "marked";
      return finish(session, kExitOk);
    }
    if (invoked == kPkt3663LiveRetained) {
      session.out.note = "retained";
      return finish(session, kExitRetained);
    }
    session.out.note = note_for_code(invoked);
    return finish(session, kExitFailed);
  }

  const char *start_name = command.fixture ? fixture_arm_export : "pkt3663_live_start";
  uintptr_t start_export = 0;
  if (start_name == nullptr || !find_export(session.read, dll_base, start_name, start_export)) {
    set_note(session.out, "missing-export");
    return cleanup_failed_load(session.out.start, session.out.retained, session.out.attempts);
  }

  RemoteAlloc argument;
  argument.process = session.write;
  void *argument_pointer = nullptr;
  if (command.fixture) {
    Pkt3663LiveFixtureArm arm{};
    arm.send_entry = send_entry;
    arm.recv_entry = recv_entry;
    arm.queue_capacity = kFixtureQueue;
    arm.output_records = kFixtureRecords;
    wcsncpy_s(arm.log_directory, command.log_dir, _TRUNCATE);
    argument.remote =
        VirtualAllocEx(session.write, nullptr, sizeof arm, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (argument.remote == nullptr) {
      set_note(session.out, "remote-alloc");
      return finish(session, kExitFailed);
    }
    session.out.modified = 1;
    if (!write_bytes(session, session.write, argument.remote, &arm, sizeof arm)) {
      set_note(session.out, "remote-write");
      return finish(session, kExitFailed);
    }
    argument_pointer = argument.remote;
  } else {
    const size_t chars = wcslen(command.log_dir) + 1;
    argument.remote = VirtualAllocEx(session.write, nullptr, chars * sizeof(wchar_t),
                                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (argument.remote == nullptr) {
      set_note(session.out, "remote-alloc");
      return finish(session, kExitFailed);
    }
    session.out.modified = 1;
    if (!write_bytes(session, session.write, argument.remote, command.log_dir,
                     chars * sizeof(wchar_t))) {
      set_note(session.out, "remote-write");
      return finish(session, kExitFailed);
    }
    argument_pointer = argument.remote;
  }

  const Call call = call_remote(session, session.write, start_export, argument_pointer);
  remember_command_wait(session.out, call);
  if (keep_remote_allocation(call.wait)) {
    argument.keep = true;
    set_note(session.out, "remote-timeout");
    return finish(session, kExitFailed);
  }
  if (call.wait != RemoteWait::Ok) {
    set_note(session.out, "failed");
    return finish(session, kExitFailed);
  }
  const int invoked = call.code;
  session.out.start = invoked;

  Pkt3663LiveReport report{};
  const ReportRead read = read_report(session, session.write, report_export, report);
  if (read != ReportRead::Ok)
    return finish(session, exit_without_report(invoked));
  apply_report(session.out, report);
  session.out.start = command_result_after_report(invoked, report.start_code);
  const int start_code = session.out.start;
  if (start_code == kPkt3663LiveRetained || report.retained != 0) {
    session.out.note = "retained";
    return finish(session, kExitRetained);
  }
  if (start_code == kPkt3663LiveStarted) {
    session.out.note = "started";
    return finish(session, kExitOk);
  }
  session.out.note = note_for_code(start_code);
  if (loaded_here &&
      unload_after_observed(process_still_alive(session.write), RemoteWait::Ok, true, start_code,
                            report.retained, report.capture_attempts)) {
    return unload_mapped(session, command.pid, session.write, free_library, dll_base, command.dll,
                         false);
  }
  return finish(session, kExitFailed);
}

} // namespace

int run_command(int argc, wchar_t **argv, bool fixture_build, const char *fixture_arm_export) {
  Session session;
  try {
    Command command;
    const char *note = "usage";
    if (!parse_command(argc, argv, fixture_build, fixture_arm_export, command, session.out, note)) {
      set_note(session.out, note);
      if (session.out.exit_code == kExitFailed && session.out.start != kPkt3663LiveBadLog)
        session.out.exit_code = kExitUsage;
      return session.out.exit_code;
    }
    return execute(session, command, fixture_build, fixture_arm_export);
  } catch (...) {
    session.out.note = "failed";
    session.out.exit_code = kExitFailed;
    return kExitFailed;
  }
}

} // namespace pkt3663::control
