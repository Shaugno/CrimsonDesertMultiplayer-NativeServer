#pragma once
#include <cstdint>

namespace cdmp::coop3663::character_guard {
enum class Actor { Unknown, Host, Guest, Other };

inline Actor identify(uint32_t key, uint32_t host, uint32_t guest) noexcept {
    if (!key) return Actor::Unknown;
    if ((key >> 28) != 0xA) return Actor::Other;
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

inline bool skip(Actor actor, uint32_t guest, bool linked, bool native_edge, bool null_playback) noexcept {
    return actor == Actor::Host && (guest >> 28) == 0xA && linked && native_edge && null_playback;
}
}
