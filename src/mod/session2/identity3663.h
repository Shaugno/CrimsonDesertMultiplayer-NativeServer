#pragma once
#include "mod/session2/field_share3663.h"
#include <span>
namespace cdmp::session2::identity3663 {
bool install(const game::build_guard::ImageIdentity&,field_share3663::ReadContext);
bool active();
bool failed();
bool ready(uintptr_t guest,uintptr_t actor,uint64_t epoch);
bool admit(uint16_t opcode,std::span<const uint8_t> payload);
bool canonicalize_login(std::span<uint8_t> owned_payload);
std::string describe();
}
