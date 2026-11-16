#pragma once
#include "mod/game/build_guard.h"
namespace cdmp::game::r3_join_probe {
bool install(const build_guard::ImageIdentity& identity);
void drain();
}
