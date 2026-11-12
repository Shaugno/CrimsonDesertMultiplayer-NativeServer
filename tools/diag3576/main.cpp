// External read-only position diagnostic for Crimson Desert 1.0.0.3576.
// It attaches with the probe's PROCESS_VM_READ | PROCESS_QUERY_INFORMATION
// handle. It does not inject, hook, write, or open a network connection.
// The mod compatibility guard is not consulted and is not opened.

#include "diag3576/reader.h"
#include "probe/process.h"

#include <windows.h>

#include <charconv>
#include <cstdio>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace {

void usage() {
    std::fputs(
        "cdmp_diag3576 --pid N --slot exe+0xRVA [--count N] [--interval MS]\n"
        "\n"
        "Read-only 3576 position diagnostic. The slot is an experimental\n"
        "manager-rooted candidate, not a production offset. The mod guard\n"
        "stays closed. A coherent sample is not player-ready.\n",
        stderr);
}

std::optional<uint32_t> parse_u32(std::string_view text) {
    uint32_t value = 0;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto parsed = std::from_chars(begin, end, value);
    if (parsed.ec != std::errc{} || parsed.ptr != end) return std::nullopt;
    return value;
}

std::optional<uint64_t> parse_slot(std::string_view text) {
    constexpr std::string_view prefix = "exe+0x";
    if (text.size() <= prefix.size()) return std::nullopt;
    for (size_t i = 0; i < prefix.size(); ++i) {
        char c = text[i];
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        if (c != prefix[i]) return std::nullopt;
    }
    uint64_t rva = 0;
    const char* begin = text.data() + prefix.size();
    const char* end = text.data() + text.size();
    const auto parsed = std::from_chars(begin, end, rva, 16);
    if (parsed.ec != std::errc{} || parsed.ptr != end || rva == 0) return std::nullopt;
    return rva;
}

std::string now_stamp() {
    SYSTEMTIME t{};
    GetLocalTime(&t);
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}", t.wYear, t.wMonth, t.wDay, t.wHour,
                       t.wMinute, t.wSecond, t.wMilliseconds);
}

std::string utf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0,
                                          nullptr, nullptr);
    if (bytes <= 0) return {};
    std::string out(static_cast<size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(), bytes, nullptr,
                        nullptr);
    return out;
}

class ProbeMemory final : public diag3576::Memory {
public:
    explicit ProbeMemory(probe::Process& process) : process_(process) {}

    bool alive() const override { return process_.alive(); }
    uint64_t module_base() const override { return process_.module_base(); }
    uint64_t module_size() const override { return process_.module_size(); }
    std::wstring module_path() const override { return process_.module_path(); }
    bool read(uint64_t addr, void* out, size_t size) const override { return process_.read(addr, out, size); }
    std::string rtti_name(uint64_t object) const override { return process_.rtti_name(object); }

private:
    probe::Process& process_;
};

}  // namespace

int main(int argc, char** argv) {
    uint32_t pid = 0;
    std::optional<uint64_t> slot_rva;
    int count = 1;
    int interval_ms = 0;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        auto need = [&](std::string_view flag) -> std::optional<std::string_view> {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "missing value for %.*s\n", static_cast<int>(flag.size()), flag.data());
                return std::nullopt;
            }
            return std::string_view{argv[++i]};
        };
        if (arg == "--pid") {
            const auto value = need(arg);
            const auto parsed = value ? parse_u32(*value) : std::nullopt;
            if (!parsed || *parsed == 0) return usage(), 2;
            pid = *parsed;
        } else if (arg == "--slot") {
            const auto value = need(arg);
            slot_rva = value ? parse_slot(*value) : std::nullopt;
            if (!slot_rva) {
                std::fputs("slot must be exe+0xRVA, an 8-byte-aligned image slot\n", stderr);
                return 2;
            }
        } else if (arg == "--count") {
            const auto value = need(arg);
            const auto parsed = value ? parse_u32(*value) : std::nullopt;
            if (!parsed || *parsed == 0 || *parsed > 1000) return usage(), 2;
            count = static_cast<int>(*parsed);
        } else if (arg == "--interval") {
            const auto value = need(arg);
            const auto parsed = value ? parse_u32(*value) : std::nullopt;
            if (!parsed || *parsed > 60000) return usage(), 2;
            interval_ms = static_cast<int>(*parsed);
        } else {
            return usage(), 2;
        }
    }
    if (pid == 0 || !slot_rva) return usage(), 2;

    probe::Process process;
    if (!process.attach_by_pid(pid)) {
        std::fprintf(stderr, "could not open pid %u read-only\n", pid);
        return 2;
    }
    if (process.pid() != pid) {
        std::fputs("attached pid does not match --pid\n", stderr);
        return 2;
    }

    ProbeMemory memory(process);
    const diag3576::IdentityCheck identity = diag3576::check_identity(memory);
    std::printf("pid %u\n", process.pid());
    std::printf("module %s\n", utf8(process.module_path()).c_str());
    std::printf("base 0x%llX size 0x%llX\n", static_cast<unsigned long long>(process.module_base()),
                static_cast<unsigned long long>(process.module_size()));
    if (identity.timestamp_known) std::printf("timestamp 0x%08X\n", identity.timestamp);
    if (!identity.accepted) {
        std::printf("identity rejected: %s\n", identity.reason.c_str());
        std::fputs("The mod compatibility guard was not opened.\n", stdout);
        return 2;
    }
    if (const std::string slot_error = diag3576::check_slot(memory, *slot_rva); !slot_error.empty()) {
        std::printf("slot rejected: %s\n", slot_error.c_str());
        return 2;
    }

    std::fputs(diag3576::format_banner(*slot_rva).c_str(), stdout);
    std::fflush(stdout);

    const uint64_t slot_address = process.module_base() + *slot_rva;
    for (int i = 0; i < count; ++i) {
        if (!memory.alive()) {
            const diag3576::Sample exited = diag3576::sample_position(memory, slot_address, 1);
            std::puts(diag3576::format_sample(exited, now_stamp()).c_str());
            return 0;
        }
        const diag3576::Sample sample = diag3576::sample_position(memory, slot_address);
        std::puts(diag3576::format_sample(sample, now_stamp()).c_str());
        std::fflush(stdout);
        if (sample.status == diag3576::SampleStatus::ProcessExited) return 0;
        if (i + 1 < count && interval_ms > 0) Sleep(static_cast<DWORD>(interval_ms));
    }
    return 0;
}
