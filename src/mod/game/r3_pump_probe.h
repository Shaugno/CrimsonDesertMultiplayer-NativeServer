#pragma once
#include "mod/game/build_guard.h"
namespace cdmp::game::r3_pump_probe {
inline bool allowed(const build_guard::ImageIdentity& id, bool requested) {
    return requested && !build_3663::kTierBComplete &&
        build_guard::select_3663_tiers(id, {true,false,false,false},
            {build_3663::kTierAComplete,false,false,false}).tier_a_reads;
}
bool install(const build_guard::ImageIdentity& id);
void report();
}
