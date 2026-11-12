#include "diag3576/reader.h"

#include <cmath>
#include <format>
#include <utility>

namespace diag3576 {
namespace {

bool same_vec(Vec4 a, Vec4 b) { return a == b; }

std::optional<uint64_t> read_u64(const Memory& mem, uint64_t addr) {
    uint64_t value = 0;
    if (!mem.read(addr, &value, sizeof value)) return std::nullopt;
    return value;
}

std::optional<Vec4> read_vec4(const Memory& mem, uint64_t addr) {
    float f[4] = {};
    if (!mem.read(addr, f, sizeof f)) return std::nullopt;
    return Vec4{f[0], f[1], f[2], f[3]};
}

struct Hop {
    SampleStatus status = SampleStatus::Unreadable;
    uint64_t value = 0;
    std::string detail;
};

Hop read_object(const Memory& mem, uint64_t addr, const char* name, std::string_view expect_rtti) {
    Hop hop;
    if (!mem.alive()) {
        hop.status = SampleStatus::ProcessExited;
        hop.detail = "process exited";
        return hop;
    }
    const auto value = read_u64(mem, addr);
    if (!mem.alive()) {
        hop.status = SampleStatus::ProcessExited;
        hop.detail = "process exited";
        return hop;
    }
    if (!value) {
        hop.status = SampleStatus::Unreadable;
        hop.detail = std::format("unreadable {}", name);
        return hop;
    }
    if (*value == 0) {
        hop.status = SampleStatus::ChainInvalid;
        hop.detail = std::format("null {}", name);
        return hop;
    }
    if (!expect_rtti.empty()) {
        const std::string actual = mem.rtti_name(*value);
        if (actual != expect_rtti) {
            hop.status = SampleStatus::ChainInvalid;
            hop.detail = std::format("{} rtti '{}' expected '{}'", name, actual, expect_rtti);
            return hop;
        }
    }
    hop.status = SampleStatus::Coherent;
    hop.value = *value;
    return hop;
}

struct Chain {
    SampleStatus status = SampleStatus::ChainInvalid;
    Pointers pointers{};
    std::string detail;
};

Chain resolve_chain(const Memory& mem, uint64_t slot_address) {
    Chain chain;
    const Hop manager = read_object(mem, slot_address, "manager", kRttiManager);
    if (manager.status != SampleStatus::Coherent) {
        chain.status = manager.status;
        chain.detail = manager.detail;
        return chain;
    }
    chain.pointers.manager = manager.value;

    const Hop actor = read_object(mem, manager.value + kHopActor, "actor", kRttiActor);
    if (actor.status != SampleStatus::Coherent) {
        chain.status = actor.status;
        chain.detail = actor.detail;
        chain.pointers.actor = actor.value;
        return chain;
    }
    chain.pointers.actor = actor.value;

    const Hop table = read_object(mem, actor.value + kHopComponentTable, "component-table", {});
    if (table.status != SampleStatus::Coherent) {
        chain.status = table.status;
        chain.detail = table.detail;
        return chain;
    }
    chain.pointers.component_table = table.value;

    const Hop character = read_object(mem, table.value + kHopCharacter, "character", kRttiCharacter);
    if (character.status != SampleStatus::Coherent) {
        chain.status = character.status;
        chain.detail = character.detail;
        return chain;
    }
    chain.pointers.character = character.value;

    const Hop inner = read_object(mem, character.value + kHopInner, "inner", {});
    if (inner.status != SampleStatus::Coherent) {
        chain.status = inner.status;
        chain.detail = inner.detail;
        return chain;
    }
    chain.pointers.inner = inner.value;

    const Hop physics = read_object(mem, inner.value + kHopPhysics, "physics", {});
    if (physics.status != SampleStatus::Coherent) {
        chain.status = physics.status;
        chain.detail = physics.detail;
        return chain;
    }
    chain.pointers.physics = physics.value;
    chain.pointers.local_field = physics.value + kHopLocal;
    chain.status = SampleStatus::Coherent;
    return chain;
}

Sample fail(SampleStatus status, std::string detail) {
    Sample sample;
    sample.status = status;
    sample.detail = std::move(detail);
    return sample;
}

Sample attempt_once(const Memory& mem, uint64_t slot_address) {
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");

    const uint64_t origin_addr = mem.module_base() + kOriginRva;
    const uint64_t static_addr = mem.module_base() + kStaticRva;

    const auto origin_before = read_vec4(mem, origin_addr);
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");
    if (!origin_before) return fail(SampleStatus::Unreadable, "unreadable origin");

    const Chain before = resolve_chain(mem, slot_address);
    if (before.status != SampleStatus::Coherent) {
        Sample sample = fail(before.status, before.detail);
        sample.origin = origin_before;
        sample.pointers = before.pointers;
        return sample;
    }

    const auto local = read_vec4(mem, before.pointers.local_field);
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");
    if (!local) return fail(SampleStatus::Unreadable, "unreadable local position");

    const auto static_pos = read_vec4(mem, static_addr);
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");
    if (!static_pos) return fail(SampleStatus::Unreadable, "unreadable static position");

    const auto origin_after = read_vec4(mem, origin_addr);
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");
    if (!origin_after) return fail(SampleStatus::Unreadable, "unreadable origin");

    const Chain after = resolve_chain(mem, slot_address);
    if (after.status == SampleStatus::ProcessExited) return fail(after.status, after.detail);
    if (!mem.alive()) return fail(SampleStatus::ProcessExited, "process exited");

    Sample sample;
    sample.pointers = before.pointers;
    sample.local = local;
    sample.origin = origin_before;
    sample.origin_after = origin_after;
    sample.static_pos = static_pos;
    // Measured residual. Y is not removed or replaced.
    sample.world = add(*local, *origin_before);
    sample.residual = sub(*static_pos, *sample.world);

    if (!finite(*local) || !finite(*origin_before) || !finite(*origin_after) || !finite(*static_pos) ||
        !finite(*sample.world) || !finite(*sample.residual)) {
        sample.status = SampleStatus::NonFinite;
        sample.detail = "a component is not finite";
        return sample;
    }
    if (after.status != SampleStatus::Coherent) {
        sample.status = after.status;
        sample.detail = after.detail;
        return sample;
    }
    if (!same_vec(*origin_before, *origin_after)) {
        sample.status = SampleStatus::OriginChanged;
        sample.detail = "origin candidate changed during the sample";
        return sample;
    }
    if (!(before.pointers == after.pointers)) {
        sample.status = SampleStatus::PointersChanged;
        sample.detail = "pointer identity changed during the sample";
        return sample;
    }
    sample.status = SampleStatus::Coherent;
    return sample;
}

bool exe_name_matches(const std::wstring& path) {
    const std::wstring name{kExpectedExeName};
    if (path.size() < name.size()) return false;
    if (path.size() > name.size()) {
        const wchar_t sep = path[path.size() - name.size() - 1];
        if (sep != L'\\' && sep != L'/') return false;
    }
    for (size_t i = 0; i < name.size(); ++i) {
        wchar_t a = path[path.size() - name.size() + i];
        wchar_t b = name[i];
        if (a >= L'A' && a <= L'Z') a = static_cast<wchar_t>(a - L'A' + L'a');
        if (b >= L'A' && b <= L'Z') b = static_cast<wchar_t>(b - L'A' + L'a');
        if (a != b) return false;
    }
    return true;
}

std::string fmt_vec(const std::optional<Vec4>& value) {
    if (!value) return "unreadable";
    return std::format("({:.4f}, {:.4f}, {:.4f}, {:.4f})", value->x, value->y, value->z, value->w);
}

}  // namespace

Vec4 add(Vec4 a, Vec4 b) { return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }

Vec4 sub(Vec4 a, Vec4 b) { return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }

bool finite(Vec4 v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && std::isfinite(v.w);
}

Sample sample_position(const Memory& mem, uint64_t slot_address, int max_attempts) {
    if (max_attempts < 1) max_attempts = 1;
    if (max_attempts > 8) max_attempts = 8;

    Sample last;
    std::optional<SampleStatus> flagged;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        last = attempt_once(mem, slot_address);
        last.attempts = attempt;
        if (last.status == SampleStatus::Coherent) {
            last.recovered_from = flagged;
            return last;
        }
        if (last.status == SampleStatus::ProcessExited) return last;
        flagged = last.status;
    }
    return last;
}

IdentityCheck check_identity(const Memory& mem) {
    IdentityCheck out;
    if (!mem.alive()) {
        out.reason = "process is not running";
        return out;
    }
    if (mem.module_base() != kExpectedImageBase) {
        out.reason = "mapped base is not 0x140000000";
        return out;
    }
    if (!exe_name_matches(mem.module_path())) {
        out.reason = "module is not CrimsonDesert.exe";
        return out;
    }
    const uint64_t base = mem.module_base();
    uint16_t mz = 0;
    int32_t lfanew = 0;
    uint32_t signature = 0;
    uint32_t timestamp = 0;
    const bool headers = mem.read(base, &mz, sizeof mz) && mz == 0x5A4D &&
                         mem.read(base + 0x3C, &lfanew, sizeof lfanew) && lfanew >= 0x40 && lfanew <= 1024 &&
                         mem.read(base + static_cast<uint64_t>(lfanew), &signature, sizeof signature) &&
                         signature == 0x00004550 &&
                         mem.read(base + static_cast<uint64_t>(lfanew) + 8, &timestamp, sizeof timestamp);
    if (!mem.alive()) {
        out.reason = "process is not running";
        return out;
    }
    if (!headers) {
        out.reason = "PE headers unreadable";
        return out;
    }
    out.timestamp = timestamp;
    out.timestamp_known = true;
    if (timestamp != kDiagnosticTimestamp) {
        out.reason = std::format("timestamp 0x{:08X} is not the 3576 diagnostic timestamp 0x{:08X}", timestamp,
                                 kDiagnosticTimestamp);
        return out;
    }
    if (kOriginRva + 16 > mem.module_size() || kStaticRva + 16 > mem.module_size()) {
        out.reason = "candidate RVAs lie outside the module";
        return out;
    }
    out.accepted = true;
    return out;
}

std::string check_slot(const Memory& mem, uint64_t slot_rva) {
    if (slot_rva == 0 || slot_rva % 8 != 0) return "manager slot is not an 8-byte-aligned RVA";
    const uint64_t module_size = mem.module_size();
    if (slot_rva > module_size || module_size - slot_rva < 8) return "manager slot lies outside the module";
    return {};
}

std::string_view status_name(SampleStatus status) {
    switch (status) {
    case SampleStatus::Coherent: return "coherent";
    case SampleStatus::Unreadable: return "unreadable";
    case SampleStatus::NonFinite: return "non-finite";
    case SampleStatus::OriginChanged: return "origin-changed";
    case SampleStatus::PointersChanged: return "pointers-changed";
    case SampleStatus::ChainInvalid: return "chain-invalid";
    case SampleStatus::ProcessExited: return "process-exited";
    }
    return "unreadable";
}

std::string format_banner(uint64_t slot_rva) {
    return std::format(
        "experimental manager slot exe+0x{:X} is a candidate, not a production offset\n"
        "origin candidate exe+0x{:X} and static candidate exe+0x{:X} are not adopted\n"
        "3576 diagnostic only. The mod compatibility guard stays closed.\n"
        "A coherent sample is not player-ready. Loading detection is unresolved.\n",
        slot_rva, kOriginRva, kStaticRva);
}

std::string format_sample(const Sample& sample, std::string_view timestamp) {
    std::string line = std::format(
        "{} status={} attempts={} local={} origin={} world={} static={} residual={} "
        "manager=0x{:X} actor=0x{:X} component-table=0x{:X} character=0x{:X} inner=0x{:X} "
        "physics=0x{:X} local-field=0x{:X} player-ready=unresolved loading=unresolved",
        timestamp, status_name(sample.status), sample.attempts, fmt_vec(sample.local), fmt_vec(sample.origin),
        fmt_vec(sample.world), fmt_vec(sample.static_pos), fmt_vec(sample.residual), sample.pointers.manager,
        sample.pointers.actor, sample.pointers.component_table, sample.pointers.character, sample.pointers.inner,
        sample.pointers.physics, sample.pointers.local_field);
    if (sample.origin_after && sample.origin && !same_vec(*sample.origin, *sample.origin_after)) {
        line += std::format(" origin-after={}", fmt_vec(sample.origin_after));
    }
    if (sample.recovered_from) {
        line += std::format(" recovered-from={}", status_name(*sample.recovered_from));
    }
    if (!sample.detail.empty() && sample.status != SampleStatus::Coherent) {
        line += " detail=";
        line += sample.detail;
    }
    return line;
}

}  // namespace diag3576
