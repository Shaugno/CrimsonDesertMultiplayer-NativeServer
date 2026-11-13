// Loaded-executable identity for the experimental 3576 live runner.
// This is not the ASI compatibility guard and it does not open that guard.

#include "pkt3576/live_identity.h"

#include <cstring>

#include <windows.h>

namespace pkt3576 {
namespace {

constexpr uint32_t kMaxELfanew = 1024;
constexpr size_t kMaxRead = 2048;
constexpr uint16_t kDosMagic = 0x5A4D;
constexpr uint32_t kPeSignature = 0x00004550;
constexpr uint16_t kPe32PlusMagic = 0x020B;
constexpr size_t kPathCap = 1024;

uint16_t load_u16(const uint8_t* data) {
    uint16_t value = 0;
    std::memcpy(&value, data, sizeof value);
    return value;
}

uint32_t load_u32(const uint8_t* data) {
    uint32_t value = 0;
    std::memcpy(&value, data, sizeof value);
    return value;
}

bool copy_range(ImageRead read, void* user, uintptr_t address, void* destination, size_t size) {
    if (read == nullptr || destination == nullptr || size == 0 || size > kMaxRead) return false;
    if (address + size < address) return false;
    return read(address, destination, size, user);
}

bool name_is_game(const wchar_t* exe_name) {
    if (exe_name == nullptr || exe_name[0] == L'\0') return false;
    return _wcsicmp(exe_name, kExeName) == 0;
}

bool protect_readable(DWORD protect) {
    if ((protect & PAGE_GUARD) != 0) return false;
    switch (protect & 0xFF) {
    case PAGE_READONLY:
    case PAGE_READWRITE:
    case PAGE_WRITECOPY:
    case PAGE_EXECUTE_READ:
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
        return true;
    default:
        return false;
    }
}

// No C++ objects with destructors: this function uses SEH.
bool seh_copy(uintptr_t address, void* destination, size_t size) {
    __try {
        std::memcpy(destination, reinterpret_cast<const void*>(address), size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool range_readable(uintptr_t address, size_t size) {
    if (size == 0 || address + size < address) return false;
    uintptr_t cursor = address;
    const uintptr_t end = address + size;
    while (cursor < end) {
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<const void*>(cursor), &info, sizeof info) != sizeof info) return false;
        if (info.State != MEM_COMMIT || !protect_readable(info.Protect)) return false;
        const auto region = reinterpret_cast<uintptr_t>(info.BaseAddress);
        if (info.RegionSize == 0 || region + info.RegionSize <= cursor) return false;
        cursor = region + info.RegionSize;
    }
    return true;
}

const wchar_t* file_name_of(const wchar_t* path) {
    const wchar_t* name = path;
    if (path == nullptr) return nullptr;
    for (const wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') name = cursor + 1;
    }
    return name;
}

bool is_absolute_path(const wchar_t* path) {
    if (path == nullptr || path[0] == L'\0') return false;
    if ((path[0] == L'\\' && path[1] == L'\\') || (path[0] == L'/' && path[1] == L'/')) return true;
    const bool drive = (path[0] >= L'A' && path[0] <= L'Z') || (path[0] >= L'a' && path[0] <= L'z');
    return drive && path[1] == L':' && (path[2] == L'\\' || path[2] == L'/');
}

bool normalize_path(const wchar_t* input, wchar_t* output, size_t output_count) {
    if (input == nullptr || output == nullptr || output_count < 4) return false;
    const DWORD written = GetFullPathNameW(input, static_cast<DWORD>(output_count), output, nullptr);
    return written != 0 && written < output_count;
}

bool component_pair(const wchar_t* path, const wchar_t* parent, const wchar_t* child) {
    wchar_t previous[128] = {};
    const wchar_t* cursor = path;
    while (*cursor != L'\0') {
        const wchar_t* start = cursor;
        while (*cursor != L'\0' && *cursor != L'\\' && *cursor != L'/') ++cursor;
        const size_t length = static_cast<size_t>(cursor - start);
        wchar_t current[128] = {};
        if (length > 0 && length < sizeof current / sizeof current[0]) {
            std::memcpy(current, start, length * sizeof(wchar_t));
            current[length] = L'\0';
            if (previous[0] != L'\0' && _wcsicmp(previous, parent) == 0 && _wcsicmp(current, child) == 0) {
                return true;
            }
            std::memcpy(previous, current, (length + 1) * sizeof(wchar_t));
        } else {
            previous[0] = L'\0';
        }
        if (*cursor != L'\0') ++cursor;
    }
    return false;
}

void directory_of(const wchar_t* full_path, wchar_t* output, size_t output_count) {
    if (output_count == 0) return;
    output[0] = L'\0';
    if (full_path == nullptr) return;
    wcsncpy_s(output, output_count, full_path, _TRUNCATE);
    wchar_t* slash = nullptr;
    for (wchar_t* cursor = output; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') slash = cursor;
    }
    if (slash == nullptr) {
        output[0] = L'\0';
        return;
    }
    if (slash == output + 2 && output[1] == L':') {
        slash[1] = L'\0';
        return;
    }
    *slash = L'\0';
}

bool same_or_under(const wchar_t* path, const wchar_t* root) {
    if (path == nullptr || root == nullptr || root[0] == L'\0') return false;
    wchar_t prefix[kPathCap] = {};
    wcsncpy_s(prefix, root, _TRUNCATE);
    size_t length = wcslen(prefix);
    while (length > 3 && (prefix[length - 1] == L'\\' || prefix[length - 1] == L'/')) {
        prefix[--length] = L'\0';
    }
    if (length == 0 || _wcsnicmp(path, prefix, length) != 0) return false;
    return path[length] == L'\0' || path[length] == L'\\' || path[length] == L'/';
}

enum class SignatureRead { Failed, Mismatch, Match };

SignatureRead read_signature(ImageRead read, void* user, uintptr_t mapped_base, uint32_t image_size, uint64_t rva,
                             const uint8_t* expected, bool& ok) {
    ok = false;
    if (rva > image_size || image_size - static_cast<uint32_t>(rva) < 16) return SignatureRead::Failed;
    if (rva > static_cast<uint64_t>(UINTPTR_MAX)) return SignatureRead::Failed;
    const auto rva_value = static_cast<uintptr_t>(rva);
    if (mapped_base > UINTPTR_MAX - rva_value || mapped_base + rva_value > UINTPTR_MAX - 16) {
        return SignatureRead::Failed;
    }
    uint8_t bytes[16] = {};
    if (!copy_range(read, user, mapped_base + rva_value, bytes, sizeof bytes)) return SignatureRead::Failed;
    ok = std::memcmp(bytes, expected, sizeof bytes) == 0;
    return ok ? SignatureRead::Match : SignatureRead::Mismatch;
}

}  // namespace

bool read_mapped_bytes(uintptr_t address, void* destination, size_t size) {
    if (destination == nullptr || size == 0 || size > kMaxRead) return false;
    if (!range_readable(address, size)) return false;
    return seh_copy(address, destination, size);
}

Identity identify_image(const wchar_t* exe_name, uintptr_t mapped_base, ImageRead read, void* user) {
    Identity id;
    id.mapped_base = mapped_base;
    if (read == nullptr || mapped_base == 0) return id;

    uint8_t dos[0x40] = {};
    if (!copy_range(read, user, mapped_base, dos, sizeof dos)) return id;
    if (load_u16(dos) != kDosMagic) return id;

    const uint32_t e_lfanew = load_u32(dos + 0x3C);
    if (e_lfanew < 0x40 || e_lfanew > kMaxELfanew) return id;

    // PE signature, file header, and PE32+ SizeOfImage. 84 bytes past e_lfanew.
    const size_t need = static_cast<size_t>(e_lfanew) + 84;
    if (need > kMaxRead) return id;
    uint8_t headers[kMaxRead] = {};
    if (!copy_range(read, user, mapped_base, headers, need)) return id;
    if (load_u32(headers + e_lfanew) != kPeSignature) return id;

    id.timestamp_known = true;
    id.timestamp = load_u32(headers + e_lfanew + 8);
    id.rejected_2976 = id.timestamp == kTimestamp2976;

    const size_t magic_at = static_cast<size_t>(e_lfanew) + 24;
    if (load_u16(headers + magic_at) != kPe32PlusMagic) return id;

    id.size_known = true;
    id.size_of_image = load_u32(headers + magic_at + 56);

    // Name, timestamp, base, and size have to match before either entry is read.
    if (!name_is_game(exe_name)) {
        id.kind = IdentityKind::WrongName;
        return id;
    }
    if (id.timestamp != kTimestamp3576) {
        id.kind = IdentityKind::WrongTimestamp;
        return id;
    }
    if (mapped_base != static_cast<uintptr_t>(kImageBase)) {
        id.kind = IdentityKind::WrongBase;
        return id;
    }
    if (id.size_of_image != kImageSize) {
        id.kind = IdentityKind::WrongSize;
        return id;
    }

    const SignatureRead send = read_signature(read, user, mapped_base, id.size_of_image, kSendRva, kSendEntry,
                                              id.send_signature_ok);
    const SignatureRead recv = read_signature(read, user, mapped_base, id.size_of_image, kRecvRva, kRecvEntry,
                                              id.recv_signature_ok);
    if (send == SignatureRead::Failed || recv == SignatureRead::Failed) return id;
    if (send != SignatureRead::Match || recv != SignatureRead::Match) {
        id.kind = IdentityKind::SignatureMismatch;
        return id;
    }
    id.kind = IdentityKind::Match;
    return id;
}

Identity identify_process() {
    Identity unread;
    HMODULE module = GetModuleHandleW(nullptr);
    if (module == nullptr) return unread;
    unread.mapped_base = reinterpret_cast<uintptr_t>(module);

    wchar_t path[kPathCap] = {};
    const DWORD length = GetModuleFileNameW(module, path, kPathCap);
    if (length == 0 || length >= kPathCap) return unread;
    const wchar_t* name = file_name_of(path);

    auto read = [](uintptr_t address, void* destination, size_t size, void*) {
        return read_mapped_bytes(address, destination, size);
    };
    return identify_image(name, unread.mapped_base, read, nullptr);
}

bool identity_allows_hooks(const Identity& identity) { return identity.kind == IdentityKind::Match; }

const char* identity_kind_name(IdentityKind kind) {
    switch (kind) {
    case IdentityKind::Match:
        return "match";
    case IdentityKind::Unreadable:
        return "unreadable";
    case IdentityKind::WrongName:
        return "wrong-name";
    case IdentityKind::WrongTimestamp:
        return "wrong-timestamp";
    case IdentityKind::WrongBase:
        return "wrong-base";
    case IdentityKind::WrongSize:
        return "wrong-size";
    case IdentityKind::SignatureMismatch:
        return "signature-mismatch";
    }
    return "unreadable";
}

bool log_directory_allowed(const wchar_t* host_exe_path, const wchar_t* log_directory) {
    if (!is_absolute_path(log_directory)) return false;

    wchar_t full[kPathCap] = {};
    if (!normalize_path(log_directory, full, kPathCap)) return false;
    if (component_pair(full, L"Crimson Desert", L"bin64")) return false;

    if (host_exe_path == nullptr || !name_is_game(file_name_of(host_exe_path))) return true;
    if (!is_absolute_path(host_exe_path)) return false;

    wchar_t host[kPathCap] = {};
    if (!normalize_path(host_exe_path, host, kPathCap)) return false;
    wchar_t host_dir[kPathCap] = {};
    directory_of(host, host_dir, kPathCap);
    if (host_dir[0] == L'\0') return false;
    return !same_or_under(full, host_dir);
}

}  // namespace pkt3576
