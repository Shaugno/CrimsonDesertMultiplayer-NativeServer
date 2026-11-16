#pragma once
#include "mod/game/build_guard.h"
namespace cdmp::game::r4_core_probe {
bool install(const build_guard::ImageIdentity& identity);
void drain();
}
