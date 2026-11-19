// Samuel Cardillo's 3576 remote character update guard (ByJunter PR #1,
// a3904af, GPL v3) establishes the update family and host-side peer-key gating.
// This version independently verifies the 3663 null read and skips only the
// helper for the verified remote primary, in either process.
#include "mod/session2/character_guard3663.h"
#include "coop3663/character_guard_policy.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/player_layout.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include "mod/session2/crash3663.h"

#include <algorithm>
#include <array>
#include <atomic>

namespace cdmp::session2::character_guard3663 {
namespace {
namespace policy = coop3663::character_guard;
constexpr uintptr_t base = game::build_3663::kImageBase;
constexpr uintptr_t update_call = 0x9732A2, helper_call = 0x977848;
constexpr uintptr_t helper = 0x258ED50, fault_read = 0x258ED8D, epilogue = 0x258EE2A;
constexpr auto& layout = game::kPlayerLayout3663;
constexpr std::array<uint8_t, 5> update_edge{0xE8,0x39,0x45,0x00,0x00};
constexpr std::array<uint8_t, 5> helper_edge{0xE8,0x03,0x75,0xC1,0x01};
constexpr std::array<uint8_t, 61> frame_and_layer{
    0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C,0x24,0x10,0x48,0x89,0x74,0x24,0x18,
    0x48,0x89,0x7C,0x24,0x20,0x41,0x54,0x41,0x56,0x41,0x57,0x48,0x83,0xEC,0x20,
    0x48,0x8B,0x41,0x30,0x48,0x8D,0xB1,0x88,0x00,0x00,0x00,0x4C,0x8B,0xF1,
    0x44,0x0F,0xB6,0x60,0x1B,0x4A,0x8B,0x84,0xE1,0x88,0x00,0x00,0x00,0x48,0x8B,0x50,0x18};
constexpr std::array<uint8_t, 8> read_bytes{0x48,0x8B,0x42,0x28,0x0F,0xB6,0x68,0x7E};
constexpr std::array<uint8_t, 31> release_bytes{
    0x48,0x8B,0x5C,0x24,0x40,0x48,0x8B,0x6C,0x24,0x48,0x48,0x8B,0x74,0x24,0x50,
    0x48,0x8B,0x7C,0x24,0x58,0x48,0x83,0xC4,0x20,0x41,0x5F,0x41,0x5E,0x41,0x5C,0xC3};
static_assert(helper + frame_and_layer.size() == fault_read);
SafetyHookMid hook;
std::atomic<bool> on{}, linked{};
role::Role process_role{};
std::atomic<uint32_t> peer_character{}; // 1 seals a conflicting peer key for this process
std::atomic<uint64_t> skipped{};
std::array<std::atomic<uint32_t>, 4> samples{};

template<class T> T read(uintptr_t at) noexcept {
    T value{};
    return at && game::safe_read(at, &value, sizeof value) ? value : T{};
}
template<size_t N> bool matches(uintptr_t rva, const std::array<uint8_t, N>& wanted) {
    std::array<uint8_t, N> actual{};
    return game::safe_read(base + rva, actual.data(), N) && actual == wanted;
}

uintptr_t local_actor() noexcept {
    const auto world = read<uintptr_t>(base + layout.kWorldSystemSlot);
    const auto manager = world ? read<uintptr_t>(world + layout.kWorldSystem_ActorManager) : 0;
    if (read<uintptr_t>(manager) != base + layout.kVtClientActorManager) return 0;
    const auto actor = read<uintptr_t>(manager + layout.kActorManager_ControlledActor);
    return read<uintptr_t>(actor) == base + layout.kVtControlledActor ? actor : 0;
}
uint32_t local_character() noexcept {
    const auto actor = local_actor();
    return actor ? read<uint32_t>(actor + 0x60) : 0;
}
// R10 Step 3: client actor +0x90 is its controlling user key (2976 research,
// docs/research/caballo-coop.md §9.1); the local controlled actor carries ours.
constexpr uint32_t kActorController = 0x90;
std::atomic<bool> contain_remote{};
std::atomic<uint64_t> remote_skipped{};
std::array<std::atomic<uint32_t>, 2> controller_samples{};

void on_null_playback(safetyhook::Context& c) noexcept {
    if (c.rdx || !on.load(std::memory_order_acquire)) return;
    const auto cc = c.r14;
    const auto actor = cc ? read<uintptr_t>(cc + 8) : 0;
    const auto component_table = actor ? read<uintptr_t>(actor + layout.kActor_ComponentTable) : 0;
    const bool confirmed_control = cc && c.rcx == cc && actor && component_table &&
        read<uintptr_t>(component_table + layout.kComponents_CharacterControl) == cc &&
        read<uintptr_t>(cc) == base + layout.kVtCharacterControl;
    // The native consumer itself obtains its actor from cc+8. Capture that
    // key and vtable even if a derived control type fails our guard checks.
    const auto key = actor ? read<uint32_t>(actor + 0x60) : 0;
    const auto actor_vtable = read<uintptr_t>(actor);
    const auto raw_peer = peer_character.load(std::memory_order_acquire);
    const uint32_t peer = (raw_peer >> 28) == 0xA ? raw_peer : 0;
    const uint32_t local = local_character();
    const bool on_host = process_role == role::Role::Host;
    const uint32_t host = on_host ? local : peer;
    const uint32_t guest = on_host ? peer : local;
    const auto identity = policy::identify(key, host, guest);
    if (samples[static_cast<size_t>(identity)].fetch_add(1, std::memory_order_relaxed) < 8) {
        crash3663::note_null_playback(key, actor_vtable, host, guest, c.r12, policy::label(identity));
    }

    // Three pushes and 0x20 bytes of locals put the call's return at rsp+0x38.
    // No native state was changed before this load. Resume at the original
    // frame teardown, leaving the remainder of CharacterControl::Update alone.
    const bool exact = confirmed_control && c.r12 < 2 && c.rax &&
        read<uintptr_t>(cc + 0x88 + c.r12 * sizeof(uintptr_t)) == c.rax &&
        read<uintptr_t>(c.rsp + 0x38) == base + helper_call + helper_edge.size();
    const bool link = linked.load(std::memory_order_acquire);
    if (policy::skip(process_role, identity, local, link, exact, c.rdx == 0)) {
        if (skipped.fetch_add(1, std::memory_order_relaxed) == 0) crash3663::note_playback_skip(key);
        c.rip = base + epilogue;
        return;
    }
    // R10 Step 3: actors driven by the other player (2976 containment rule).
    const auto controller = actor ? read<uint32_t>(actor + kActorController) : 0;
    const auto me = local_actor();
    const auto local_user = me ? read<uint32_t>(me + kActorController) : 0;
    const bool remote = policy::remote_controlled(controller, local_user);
    const bool enabled = contain_remote.load(std::memory_order_acquire);
    if (controller_samples[remote ? 1 : 0].fetch_add(1, std::memory_order_relaxed) < 8) {
        crash3663::note_playback_controller(key, controller, local_user, exact, remote, enabled);
    }
    if (policy::skip_remote_controlled(enabled, key, local, controller, local_user, link, exact, c.rdx == 0)) {
        if (remote_skipped.fetch_add(1, std::memory_order_relaxed) == 0) crash3663::note_playback_skip(key);
        c.rip = base + epilogue;
    }
}
}

bool install(const game::build_guard::ImageIdentity& identity, role::Role current_role) {
    if (on.load()) return process_role == current_role;
    if (current_role != role::Role::Host && current_role != role::Role::Guest) return false;
    if (!game::build_guard::select_3663_tiers(identity, {true,true,true,true},
        {game::build_3663::kTierAComplete, game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete, game::build_3663::kTierDComplete}).tier_d_lifecycle ||
        !matches(update_call, update_edge) || !matches(helper_call, helper_edge) ||
        !matches(helper, frame_and_layer) || !matches(fault_read, read_bytes) ||
        !matches(epilogue, release_bytes)) return false;
    auto prepared = game::prepare_mid(base + fault_read, &on_null_playback, [](const auto& bytes) {
        return bytes.size() == read_bytes.size() && std::equal(bytes.begin(), bytes.end(), read_bytes.begin());
    }, "3663 character", "null-playback read and native return");
    if (!prepared) return false;
    hook = std::move(*prepared);
    process_role = current_role;
    on.store(true, std::memory_order_release);
    if (!hook.enable()) {
        on.store(false, std::memory_order_release);
        return false;
    }
    log::info("3663 character guard active: exact 258C8FD null read, verified remote {} character only",
              current_role == role::Role::Host ? "guest" : "host");
    return true;
}

void note_peer_character(uint32_t key) noexcept {
    if ((key >> 28) != 0xA) return;
    uint32_t expected = 0;
    if (!peer_character.compare_exchange_strong(expected, key) && expected != key) peer_character.store(1);
}
void set_link_ready(bool ready) noexcept { linked.store(ready, std::memory_order_release); }
uint64_t skips() noexcept { return skipped.load(std::memory_order_relaxed) + remote_skipped.load(std::memory_order_relaxed); }
void set_remote_containment(bool enabled) noexcept {
    contain_remote.store(enabled, std::memory_order_release);
    log::info("3663 character guard: remote-controlled actor containment {} (remote_actor_guard={})",
              enabled ? "ON" : "off", enabled ? 1 : 0);
}
}
