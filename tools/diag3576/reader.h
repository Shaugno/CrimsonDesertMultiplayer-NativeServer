#pragma once
// External read-only position sample for the 3576 diagnostic.
// A coherent sample means the chain, origin, and floats were stable for one
// attempt. It does not mean the player is ready or in the world. Loading
// detection is unresolved.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "diag3576/candidates.h"
#include "diag3576/memory.h"

namespace diag3576 {

struct Vec4 {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;
    bool operator==(const Vec4&) const = default;
};

enum class SampleStatus {
    Coherent,
    Unreadable,
    NonFinite,
    OriginChanged,
    PointersChanged,
    ChainInvalid,
    ProcessExited,
};

struct Pointers {
    uint64_t manager = 0;
    uint64_t actor = 0;
    uint64_t component_table = 0;
    uint64_t character = 0;
    uint64_t inner = 0;
    uint64_t physics = 0;
    uint64_t local_field = 0;
    bool operator==(const Pointers&) const = default;
};

struct Sample {
    SampleStatus status = SampleStatus::Unreadable;
    int attempts = 0;
    // Set when an earlier attempt in this call was flagged and a later
    // attempt was coherent.
    std::optional<SampleStatus> recovered_from;
    std::string detail;
    Pointers pointers{};
    std::optional<Vec4> local;
    std::optional<Vec4> origin;
    std::optional<Vec4> origin_after;
    std::optional<Vec4> world;
    std::optional<Vec4> static_pos;
    std::optional<Vec4> residual;
};

struct IdentityCheck {
    bool accepted = false;
    bool timestamp_known = false;
    uint32_t timestamp = 0;
    std::string reason;
};

Vec4 add(Vec4 a, Vec4 b);
Vec4 sub(Vec4 a, Vec4 b);
bool finite(Vec4 v);

// `slot_address` is the absolute address of a manager-rooted image slot.
// Retries at most kMaxSampleAttempts. Process exit returns immediately.
// Residual is static - (local + origin) with no component adjusted.
Sample sample_position(const Memory& mem, uint64_t slot_address, int max_attempts = kMaxSampleAttempts);

IdentityCheck check_identity(const Memory& mem);
std::string check_slot(const Memory& mem, uint64_t slot_rva);

std::string_view status_name(SampleStatus status);
std::string format_banner(uint64_t slot_rva);
std::string format_sample(const Sample& sample, std::string_view timestamp);

}  // namespace diag3576
