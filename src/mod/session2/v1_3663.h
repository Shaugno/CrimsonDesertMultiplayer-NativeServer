#pragma once
#include "mod/game/build_guard.h"
#include "mod/session2/field_share3663.h"
#include "common/troc_link.h"
#include "coop3663/v1_3663_policy.h"
#include <deque>
#include <mutex>
namespace cdmp::session2::v1_3663 {
bool host_install(const game::build_guard::ImageIdentity&);
bool host_context(field_share3663::Context&);
bool host_ready();
void note_request(uint16_t);
void note_response(uint16_t);
void prepare_shared_move_for_guest(std::span<uint8_t> payload, int8_t slot);
bool admit_request(uint16_t);
bool allow_disconnect_logout();
void note_lost();
bool stopped();
std::string describe();
// Both ends use this restricted bridge; no legacy tunnel_tick or optional
// observer/save/rejoin callback is reachable from it.
class Bridge {
public:
    Bridge(ClientSession&,bool host);
    ~Bridge();
    void tick(uint32_t now);
    std::string describe()const;
private:
    void receive(std::vector<uint8_t>&&,uint32_t now);
    bool host_,failed_=false,was_connected_=false,logout_pending_=false,world_ready_=false;
    troc::TrocLink link_;
    std::deque<std::vector<uint8_t>> pending_;
    size_t pending_bytes_=0;
    uint32_t epoch_=0,last_ready_=0,first_pending_=0;
    uint32_t advertised_host_=0;
    uint64_t queued_=0,refused_=0;
};
}
