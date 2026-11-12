#pragma once
// Central executable-identity gate. Startup may read a build-specific game
// address or install a hook only after this classifies the mapped image as
// Crimson Desert 1.0.0.2976. Copying the PE headers to classify the image is
// not a gameplay RVA read. A test host (anything that is not CrimsonDesert.exe,
// including cdmp_host with CDMP_TEST_HOST) never takes the in-game path.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <span>
#include <string>
#include <vector>

#include "mod/game/offsets.h"

namespace cdmp::game::build_guard {

// A corrupt e_lfanew must not turn the identity copy into an unbounded read.
inline constexpr uint32_t kMaxELfanew = 1024;
inline constexpr uint16_t kDosMagic = 0x5A4D;
inline constexpr uint32_t kPeSignature = 0x00004550;
inline constexpr uint16_t kPe32PlusMagic = 0x020B;

enum class IdentityKind {
    Supported,    // mapped base and PE timestamp match build 1.0.0.2976
    Unsupported,  // headers are readable, but the image is a different build
    Unreadable,   // headers are missing, truncated, or not a PE32+ image we can trust
};

struct ImageIdentity {
    IdentityKind kind = IdentityKind::Unreadable;
    uintptr_t mapped_base = 0;
    bool timestamp_known = false;
    uint32_t timestamp = 0;
    bool size_known = false;
    uint32_t size_of_image = 0;
};

struct StartupRequest {
    bool in_game = false;       // process image is CrimsonDesert.exe
    bool test_overlay = false;  // CDMP_TEST_OVERLAY, explicit and off the game path
    bool guest_role = false;    // CDMP_ROLE=guest
    ImageIdentity identity{};
};

// Where a player sample may come from. Off means the process must not sample a
// player at all: no game reader and no synthetic state.
enum class PlayerFeed {
    Off,
    Game,
    Synthetic,
};

// What the worker is allowed to do. read_game_addresses and install_hooks are
// both true only for a supported in-game image. install_test_overlay is the
// explicit test-host overlay and does not authorize game addresses or hooks.
// run_network is true for a supported in-game image and for a test host.
// A rejected in-game image leaves networking off and player_feed Off.
struct StartupDecision {
    bool read_game_addresses = false;
    bool install_hooks = false;
    bool install_test_overlay = false;
    bool run_network = false;
    bool end_guest_process = false;
    PlayerFeed player_feed = PlayerFeed::Off;
};

inline uint16_t read_u16(std::span<const uint8_t> bytes, size_t offset) {
    uint16_t value = 0;
    std::memcpy(&value, bytes.data() + offset, sizeof value);
    return value;
}

inline uint32_t read_u32(std::span<const uint8_t> bytes, size_t offset) {
    uint32_t value = 0;
    std::memcpy(&value, bytes.data() + offset, sizeof value);
    return value;
}

// `headers` is a bounded copy of the mapped image, starting at `mapped_base`.
// The supported check uses that mapped base (GetModuleHandle), matching the
// existing per-feature checks, not the optional-header preferred ImageBase.
inline ImageIdentity identify_image(std::span<const uint8_t> headers, uintptr_t mapped_base) {
    ImageIdentity id;
    id.mapped_base = mapped_base;

    if (headers.size() < 0x40) return id;
    if (read_u16(headers, 0) != kDosMagic) return id;

    const uint32_t e_lfanew = read_u32(headers, 0x3C);
    if (e_lfanew < 0x40 || e_lfanew > kMaxELfanew) return id;

    const size_t file_header_end = static_cast<size_t>(e_lfanew) + 24;
    if (headers.size() < file_header_end) return id;
    if (read_u32(headers, e_lfanew) != kPeSignature) return id;

    const size_t magic_at = static_cast<size_t>(e_lfanew) + 24;
    if (headers.size() < magic_at + sizeof(uint16_t)) return id;

    id.timestamp_known = true;
    id.timestamp = read_u32(headers, static_cast<size_t>(e_lfanew) + 8);

    if (read_u16(headers, magic_at) != kPe32PlusMagic) {
        id.kind = IdentityKind::Unsupported;
        return id;
    }

    // PE32+ SizeOfImage is 56 bytes into the optional header.
    const size_t size_at = magic_at + 56;
    if (headers.size() < size_at + sizeof(uint32_t)) return id;

    id.size_known = true;
    id.size_of_image = read_u32(headers, size_at);
    const bool base_ok = mapped_base == build_2976::kImageBase;
    const bool time_ok = id.timestamp == build_2976::kPeTimestamp;
    id.kind = (base_ok && time_ok) ? IdentityKind::Supported : IdentityKind::Unsupported;
    return id;
}

// `read_bytes(address, out, size)` copies from the mapped image and returns
// false when the range cannot be read. A failed copy is an unreadable identity.
template <class ReadFn>
ImageIdentity identify_mapped(uintptr_t mapped_base, ReadFn&& read_bytes) {
    ImageIdentity unread;
    unread.mapped_base = mapped_base;

    uint8_t dos[0x40]{};
    if (!read_bytes(mapped_base, dos, sizeof dos)) return unread;
    if (dos[0] != 'M' || dos[1] != 'Z') return unread;

    uint32_t e_lfanew = 0;
    std::memcpy(&e_lfanew, dos + 0x3C, sizeof e_lfanew);
    if (e_lfanew < 0x40 || e_lfanew > kMaxELfanew) return unread;

    const size_t need = static_cast<size_t>(e_lfanew) + 24 + 60;
    std::vector<uint8_t> headers(need);
    if (!read_bytes(mapped_base, headers.data(), headers.size())) return unread;
    return identify_image(headers, mapped_base);
}

inline std::string expected_build_text() {
    return std::format("Crimson Desert 1.0.0.2976 (PE timestamp 0x{:08X}, image base 0x{:X})",
                       build_2976::kPeTimestamp, build_2976::kImageBase);
}

inline std::string diagnostic(const ImageIdentity& id) {
    const std::string expected = expected_build_text();
    if (id.kind == IdentityKind::Supported) {
        return std::format("Supported game build {}.", expected);
    }
    if (id.kind == IdentityKind::Unreadable) {
        return std::format(
            "Executable identity is unreadable. Expected {}. Game addresses, hooks, and networking stay off.",
            expected);
    }
    std::string actual = "This image";
    if (id.timestamp_known) actual += std::format(" has PE timestamp 0x{:08X}", id.timestamp);
    actual += std::format(" and image base 0x{:X}", id.mapped_base);
    return std::format(
        "Unsupported game build. Expected {}. {}. Game addresses, hooks, and networking stay off.", expected,
        actual);
}

inline StartupDecision decide_startup(const StartupRequest& req) {
    StartupDecision decision;
    if (!req.in_game) {
        decision.install_test_overlay = req.test_overlay;
        decision.run_network = true;
        decision.player_feed = PlayerFeed::Synthetic;
        return decision;
    }
    const bool supported = req.identity.kind == IdentityKind::Supported;
    decision.read_game_addresses = supported;
    decision.install_hooks = supported;
    decision.run_network = supported;
    decision.player_feed = supported ? PlayerFeed::Game : PlayerFeed::Off;
    decision.end_guest_process = !supported && req.guest_role;
    return decision;
}

// Invokes each side effect only when the decision allows it. Callers that
// touch game addresses or install hooks go through this so a rejected identity
// cannot reach them.
template <class ReadFn, class HookFn, class OverlayFn>
void apply_startup(const StartupDecision& decision, ReadFn&& read_game_addresses, HookFn&& install_hooks,
                   OverlayFn&& install_test_overlay) {
    if (decision.read_game_addresses) read_game_addresses();
    if (decision.install_hooks) install_hooks();
    if (decision.install_test_overlay) install_test_overlay();
}

// The worker's only start of a player source and of networking. run_network in
// the mod calls LocalSource::sample, which is the only generator of a synthetic
// player state, and that call happens inside `start_network`. A rejected
// in-game identity has feed Off and run_network false, so none of the callbacks
// run. The test host is the only caller that gets a synthetic source.
template <class GameFn, class SyntheticFn, class NetworkFn>
void start_multiplayer(const StartupDecision& decision, bool net_enabled, GameFn&& use_game,
                       SyntheticFn&& use_synthetic, NetworkFn&& start_network) {
    const bool game = decision.player_feed == PlayerFeed::Game;
    const bool synthetic = decision.player_feed == PlayerFeed::Synthetic;
    if (game) use_game();
    else if (synthetic) use_synthetic();
    if (decision.run_network && net_enabled && (game || synthetic)) start_network();
}

}  // namespace cdmp::game::build_guard
