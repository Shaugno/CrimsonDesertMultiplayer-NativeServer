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
}
