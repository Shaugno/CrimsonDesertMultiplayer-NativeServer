#pragma once
#include <cstdint>
// Explicit loopback diagnostic; never replays login/world/save requests.
int run_r3_tunnel_peer(uint16_t port, int seconds, volatile bool& running);
