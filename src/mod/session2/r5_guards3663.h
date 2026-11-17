#pragma once
#include "mod/game/build_guard.h"
#include <string>
namespace cdmp::session2::r5_guards3663 {
bool host_install(const game::build_guard::ImageIdentity&);
bool guest_install(const game::build_guard::ImageIdentity&);
void mark_remote() noexcept;
std::string describe();
}
