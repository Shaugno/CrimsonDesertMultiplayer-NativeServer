#pragma once
#include "mod/role_logic.h"
#include <cstdint>

namespace cdmp::coop3663::character_guard {
enum class Actor { Unknown, Host, Guest, Other };

inline Actor identify(uint32_t key, uint32_t host, uint32_t guest) noexcept {
    if (!key) return Actor::Unknown;
    if ((key >> 28) != 0xA) return Actor::Other;
    if (key == host && key == guest) return Actor::Unknown;
    if (key == guest) return Actor::Guest;
    if (key == host) return Actor::Host;
    return host && guest ? Actor::Other : Actor::Unknown;
}

inline const char* label(Actor actor) noexcept {
    switch (actor) {
        case Actor::Host: return "host";
        case Actor::Guest: return "guest";
        case Actor::Other: return "other";
        default: return "unknown";
    }
}

inline bool skip(role::Role process_role, Actor actor, uint32_t local, bool linked,
                 bool native_edge, bool null_playback) noexcept {
    const bool remote = (process_role == role::Role::Guest && actor == Actor::Host) ||
                        (process_role == role::Role::Host && actor == Actor::Guest);
    return remote && (local >> 28) == 0xA && linked && native_edge && null_playback;
}

// R10 Step 3 ([session2] remote_actor_guard, off by default). The 2976
// containment rule (docs/research/data/c1-actor-rechazado.md §3-4,
// caballo-coop.md §9): a client actor's +0x90 holds its controlling user key,
// written from the ActorLoadDataAck record; local actors carry the local user.
// An actor driven by the other player (for example its horse) has the other
// user's key there and, until its first action arrives, can reach the
// CharacterControl helper with a selected layer whose playback is null.
inline bool remote_controlled(uint32_t controller, uint32_t local_user) noexcept {
    return (controller >> 28) == 0x9 && (local_user >> 28) == 0x9 && controller != local_user;
}

// Only at the exact null read (the alternative is an access violation), never
// for the local primary, only while the link is up and the native frame, type,
// backlink and selected layer were verified.
inline bool skip_remote_controlled(bool enabled, uint32_t key, uint32_t local_primary, uint32_t controller,
                                   uint32_t local_user, bool linked, bool native_edge, bool null_playback) noexcept {
    return enabled && key && (local_primary >> 28) == 0xA && key != local_primary &&
           remote_controlled(controller, local_user) && linked && native_edge && null_playback;
}
}
