#pragma once
#include "mod/game/build_guard.h"
#include <deque>
#include <vector>
namespace cdmp::session2::guest3663 {
bool install(const game::build_guard::ImageIdentity&);
bool active();
bool failed();
void set_host_ready(bool);
void deliver(std::vector<uint8_t>&&);
std::deque<std::vector<uint8_t>> take_outgoing();
std::string describe();
}
