#pragma once
// Decisions for the external controller. The diagnostic DLL still performs
// its own identity check after it is loaded. This header does not open a
// process and does not launch the game.

#include "pkt3576/live_api.h"
#include "pkt3576/live_identity.h"

#include <wchar.h>

namespace pkt3576::control {

inline constexpr int kExitOk = 0;
inline constexpr int kExitUsage = 2;
inline constexpr int kExitRetained = 3;
inline constexpr int kExitRejected = 4;
inline constexpr int kExitFailed = 5;

inline constexpr wchar_t kProductionDll[] = L"cdmp_pkt3576_live.dll";
inline constexpr wchar_t kFixtureDll[] = L"cdmp_pkt3576_live_fixture.dll";

inline bool file_name_is(const wchar_t* path, const wchar_t* file_name) {
    if (path == nullptr || file_name == nullptr) return false;
    const wchar_t* name = path;
    for (const wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') name = cursor + 1;
    }
    return _wcsicmp(name, file_name) == 0;
}

inline bool game_file_name(const wchar_t* path) { return file_name_is(path, kExeName); }

// Production load and start require a documented match, including both pristine
// entry signatures. A test controller may arm fixtures only when the target is
// not CrimsonDesert.exe. A signature mismatch is not a match and is not a
// force flag.
inline bool modification_allowed(bool fixture_build, bool fixture_requested, IdentityKind kind, bool game_name) {
    if (fixture_requested) return fixture_build && !game_name;
    return kind == IdentityKind::Match;
}

// Status and stop may call an already loaded production module after its own
// hooks replace the pristine entry bytes. Name, timestamp, base, and size
// still have to match, which is what makes the kind SignatureMismatch rather
// than WrongName, WrongTimestamp, WrongBase, or WrongSize. This does not
// authorize load or start.
inline bool installed_module_control_allowed(IdentityKind kind, bool module_loaded) {
    return module_loaded && kind == IdentityKind::SignatureMismatch;
}

enum class ControlAction { Load, Start, Status, Stop, Mark };

inline bool control_allowed(bool fixture_build, bool fixture_requested, IdentityKind kind, bool game_name,
                            ControlAction action, bool module_loaded) {
    if (modification_allowed(fixture_build, fixture_requested, kind, game_name)) return true;
    if (fixture_requested) return false;
    if (action != ControlAction::Status && action != ControlAction::Stop && action != ControlAction::Mark) {
        return false;
    }
    return installed_module_control_allowed(kind, module_loaded);
}

// Decimal PID in 1..4294967295. Rejects empty text, non-digits, zero, and any
// value that would wrap a 32-bit unsigned conversion. Does not open a process.
inline bool parse_pid(const wchar_t* text, unsigned long& pid) {
    if (text == nullptr || text[0] == L'\0') return false;
    unsigned long value = 0;
    for (const wchar_t* cursor = text; *cursor != L'\0'; ++cursor) {
        if (*cursor < L'0' || *cursor > L'9') return false;
        const unsigned long digit = static_cast<unsigned long>(*cursor - L'0');
        if (value > (0xFFFFFFFFUL - digit) / 10UL) return false;
        value = value * 10UL + digit;
    }
    if (value == 0) return false;
    pid = value;
    return true;
}

// FreeLibrary is allowed only when the module cannot have been pinned and
// the runner has not asked to keep ownership. Retained, a started capture,
// and any Capture::start attempt stay mapped.
inline bool unload_allowed(int start_code, int retained, unsigned attempts) {
    if (retained != 0 || attempts != 0) return false;
    if (start_code == kPkt3576LiveStarted || start_code == kPkt3576LiveRolledBack ||
        start_code == kPkt3576LiveRetained) {
        return false;
    }
    return true;
}

inline bool dll_file_allowed(const wchar_t* path, bool fixture_module) {
    return file_name_is(path, fixture_module ? kFixtureDll : kProductionDll);
}

// Parses the operator command and, only after the identity gate, loads or
// calls the diagnostic module. Does not terminate the target.
// fixture_arm_export is non-null only in the test controller. The production
// binary passes null and never calls the fixture entry.
int run_command(int argc, wchar_t** argv, bool fixture_build, const char* fixture_arm_export);

}  // namespace pkt3576::control
