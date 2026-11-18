#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace cdmp::coop3663::v1 {
// Verified in the exact 3663 RTTI descriptor and initializer. The native
// 2976 request at this point in login had a slot-0 header and empty body.
inline constexpr uint16_t kLoginCharacterToGame = 2140;
inline bool initial_game_login_packet(int8_t slot,std::size_t payload_size){
    return slot==0 && payload_size==0;
}
inline uint8_t loading_flag(bool guest,uint8_t native){return guest?0:native;}
inline bool reject_test_process(bool marker,bool in_game,bool exact_target,bool host,bool guest,bool sandbox_environment,bool host_real_saves){
    return marker && in_game && (!exact_target || host==guest ||
        (guest?!sandbox_environment:(!host_real_saves || sandbox_environment)));
}
enum class Route { Refuse, Host, Guest, GuestStartup };
struct RouteRequest {
    bool permissions{},sandbox{},network{},session{},host_role{},guest_role{},net_host{},native_guest{},create{},accept{};
    bool steam{},loopback{},local_peer{},coop{},diagnostic{};
    bool guest_startup_only{};
};
inline Route route(const RouteRequest& r){
    if(!r.permissions || r.coop || r.diagnostic || (r.host_role==r.guest_role) ||
        (r.guest_role && !r.sandbox) || (r.host_role && r.sandbox))return Route::Refuse;
    if(r.guest_startup_only)return r.guest_role && !r.network && !r.session && !r.net_host &&
        r.native_guest && !r.create && !r.accept?Route::GuestStartup:Route::Refuse;
    if(!r.network || !r.session || (!r.steam && (!r.loopback || !r.local_peer)))return Route::Refuse;
    if(r.host_role && r.net_host && !r.native_guest && r.create && r.accept)return Route::Host;
    if(r.guest_role && !r.net_host && r.native_guest && !r.create && !r.accept)return Route::Guest;
    return Route::Refuse;
}
// Exact native descriptor names; no substring-based admission. This is the
// deliberately small v1 service/loading/movement surface, not every kind-4 Req.
inline bool request_name(std::string_view n){
    for(const auto allowed:{"ConnectCryptogramToServerReq","LogoutSessionToServerReq",
        "LoginUserToFieldServerReq","LoginCharacterToFieldServerReq","LoginCharacterToGameReq","PreLoadSequencerReq",
        "GameLoadingStartReq","MoveFieldCompleteReq","LoadingCompleteReq",
        "MoveActorReq","RotateActorReq","StartActionReq","EndActionReq",
        "WorldObserverDisableLoadCompleteReq","SceneLevelUnloadReq",
        "SceneLevelLoadCompleteSyncSceneObjectListReq"})if(n==allowed)return true;
    return false;
}
enum class Phase { Title, Loading, Stable, Closing, Closed, Quarantine };
struct Lifecycle {
    Phase phase=Phase::Title;bool logout_queued=false,world_login_queued=false;
    bool admit(uint16_t op,bool in_private_field=false)const{
        if(phase==Phase::Closed || phase==Phase::Quarantine || logout_queued)return false;
        if(op==kLoginCharacterToGame)return phase==Phase::Loading && in_private_field && !world_login_queued;
        if(op==2001 && !can_logout())return false;
        if((op==3081 || op==2350) && phase==Phase::Stable)return false;
        return true;
    }
    void request(uint16_t op){
        if(phase==Phase::Closed || phase==Phase::Quarantine)return;
        if(op==3081 || op==2350)phase=Phase::Loading;
        if(op==kLoginCharacterToGame)world_login_queued=true;
        if(op==2001){logout_queued=true;phase=Phase::Closing;}
    }
    void response(uint16_t op,bool entered_shared_field){
        if(phase==Phase::Closed || phase==Phase::Quarantine)return;
        // The private staging field can emit the same acknowledgement.
        if(op==2891 && phase==Phase::Loading && entered_shared_field)phase=Phase::Stable;
        if(op==2002 && phase==Phase::Closing)phase=Phase::Closed;
    }
    // No guessed completion payload if the peer disappears while loading.
    bool can_logout()const{return !logout_queued && (phase==Phase::Title || phase==Phase::Stable);}
    void lost(){if(!can_logout() && !logout_queued && phase!=Phase::Closed)phase=Phase::Quarantine;}
};
}
