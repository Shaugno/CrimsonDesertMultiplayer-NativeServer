#pragma once
#include "mod/game/build_guard.h"
#include <cstdint>
#include <string>

namespace cdmp::session2::field_share3663 {
// Supplied by the target server route. Revalidated before publication, after
// pinning and under field locks. Never supplied by a packet or environment RVA.
struct Context { uintptr_t host{}, guest{}; uint64_t epoch{}; };
using ReadContext = bool(*)(Context&);
using IdentityReady = bool(*)(uintptr_t guest, uintptr_t primary, uint64_t epoch);
bool install(const game::build_guard::ImageIdentity&, ReadContext, IdentityReady);
bool active();
bool quarantined();
std::string describe();
}
