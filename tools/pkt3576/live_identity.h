#pragma once
// Reads the loaded executable's identity before any packet hook.
// A match is not production support and does not open the ASI guard.

#include <cstddef>
#include <cstdint>

#include "pkt3576/candidates.h"

namespace pkt3576 {

enum class IdentityKind {
    Match,
    Unreadable,
    WrongName,
    WrongTimestamp,
    WrongBase,
    WrongSize,
    SignatureMismatch,
};

struct Identity {
    IdentityKind kind = IdentityKind::Unreadable;
    uintptr_t mapped_base = 0;
    uint32_t timestamp = 0;
    uint32_t size_of_image = 0;
    bool timestamp_known = false;
    bool size_known = false;
    bool rejected_2976 = false;
    bool send_signature_ok = false;
    bool recv_signature_ok = false;
};

// `read` copies from the mapped image and returns false when that range cannot
// be read. `exe_name` is the file name only, not a directory.
using ImageRead = bool (*)(uintptr_t address, void* destination, size_t size, void* user);

Identity identify_image(const wchar_t* exe_name, uintptr_t mapped_base, ImageRead read, void* user);

// The process executable (GetModuleHandle nullptr). SEH guards the reads.
Identity identify_process();

// VirtualQuery plus SEH. False when the range is empty, wraps, exceeds 2048
// bytes, is not a committed readable region, or the copy faults.
bool read_mapped_bytes(uintptr_t address, void* destination, size_t size);

// True only for IdentityKind::Match. Every other kind stays off the hook path.
bool identity_allows_hooks(const Identity& identity);

const char* identity_kind_name(IdentityKind kind);

// Absolute log directories only. Rejects a relative path, the directory of a
// loaded CrimsonDesert.exe, and any path under a "Crimson Desert\bin64" folder.
bool log_directory_allowed(const wchar_t* host_exe_path, const wchar_t* log_directory);

}  // namespace pkt3576
