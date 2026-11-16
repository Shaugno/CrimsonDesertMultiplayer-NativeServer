#pragma once
// Guarded, conservative HP selection. No native calls or literal-ID fallback.
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace cdmp::game {
struct HealthDefinitionLayout {
    uintptr_t normal_slot = 0;
    uintptr_t alternate_slot = 0;
    uint32_t root = 0, tables = 0, object = 0, id = 0;
};

inline std::optional<uintptr_t> health_address(uintptr_t base, uintptr_t offset, size_t size) {
    constexpr uintptr_t last = 0x00007FFFFFFFFFFFULL;
    if (base < 0x10000 || base > last || offset > last - base) return std::nullopt;
    const uintptr_t address = base + offset;
    if (!size || size - 1 > last - address) return std::nullopt;
    return address;
}

struct HealthDefinitionSample {
    std::array<uintptr_t, 4> chain{};
    uint16_t key = 0;
    bool operator==(const HealthDefinitionSample&) const = default;
};

struct HealthDefinitionPair {
    HealthDefinitionSample normal, alternate;
    bool operator==(const HealthDefinitionPair&) const = default;
};

template<class ReadFn>
std::optional<HealthDefinitionSample> sample_health_definition(
    uintptr_t base, uintptr_t slot, const HealthDefinitionLayout& layout, ReadFn&& read) {
    if (!slot) return std::nullopt;
    HealthDefinitionSample result;
    uintptr_t pointer = base;
    const uintptr_t offsets[] = {slot, layout.root, layout.tables, layout.object};
    for (size_t i = 0; i < result.chain.size(); ++i) {
        auto address = health_address(pointer, offsets[i], sizeof(uintptr_t));
        if (!address || !read(*address, &pointer, sizeof pointer) ||
            !health_address(pointer, 0, sizeof(uint16_t))) return std::nullopt;
        result.chain[i] = pointer;
    }
    auto address = health_address(pointer, layout.id, sizeof result.key);
    if (!address || !read(*address, &result.key, sizeof result.key)) return std::nullopt;
    return result;
}

template<class ReadFn>
std::optional<HealthDefinitionPair> consensus_health_definition(
    uintptr_t base, const HealthDefinitionLayout& layout, ReadFn&& read) {
    auto normal = sample_health_definition(base, layout.normal_slot, layout, read);
    auto alternate = sample_health_definition(base, layout.alternate_slot, layout, read);
    // The native updater selects a registry by its game-thread TLS mode.
    // An observer may use a key only when both complete registries agree.
    if (!normal || !alternate || normal->key != alternate->key) return std::nullopt;
    return HealthDefinitionPair{*normal, *alternate};
}

struct NativeHealthReading { float current = 0, maximum = 0; };

template<class ReadFn>
std::optional<NativeHealthReading> read_unique_native_health(uintptr_t entries,
    uint32_t count, uint32_t stride, uint32_t id_offset, uint32_t current_offset,
    uint32_t maximum_offset, uint16_t key, float units, ReadFn&& read) {
    // A scan cap is observer policy, independent of any game container limit.
    if (!count || count > 64 || !stride || !std::isfinite(units) || units <= 0)
        return std::nullopt;
    if (id_offset > stride || sizeof(uint16_t) > stride - id_offset ||
        current_offset > stride || sizeof(int64_t) > stride - current_offset ||
        maximum_offset > stride || sizeof(int64_t) > stride - maximum_offset)
        return std::nullopt;
    std::optional<NativeHealthReading> result;
    for (uint32_t i = 0; i < count; ++i) {
        auto entry = health_address(entries, static_cast<uintptr_t>(i) * stride, stride);
        uint16_t id = 0;
        if (!entry || !read(*entry + id_offset, &id, sizeof id)) return std::nullopt;
        if (id != key) continue;
        if (result) return std::nullopt;
        int64_t current = 0, maximum = 0;
        if (!read(*entry + current_offset, &current, sizeof current) ||
            !read(*entry + maximum_offset, &maximum, sizeof maximum) ||
            current < 0 || maximum <= 0 || current > maximum) return std::nullopt;
        NativeHealthReading reading{static_cast<float>(current) / units,
                                    static_cast<float>(maximum) / units};
        if (!std::isfinite(reading.current) || !std::isfinite(reading.maximum)) return std::nullopt;
        result = reading;
    }
    return result;
}
} // namespace cdmp::game
