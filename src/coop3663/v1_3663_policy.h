#pragma once
#include <cstdint>
#include <string_view>
namespace cdmp::coop3663::v1 {
inline uint8_t loading_flag(bool guest,uint8_t native){return guest?0:native;}
inline bool reject_test_process(bool marker,bool in_game,bool exact_target,bool tagged_role,bool sandbox_environment){
    return marker && in_game && (!exact_target || !tagged_role || !sandbox_environment);
}
enum class Route { Refuse, Host, Guest };
struct RouteRequest {
    bool permissions{},sandbox{},network{},session{},host_role{},guest_role{},net_host{},native_guest{},create{},accept{};
    bool steam{},loopback{},local_peer{},coop{},diagnostic{};
};
inline Route route(const RouteRequest& r){
    if(!r.permissions || !r.sandbox || !r.network || !r.session || r.coop || r.diagnostic ||
        (r.host_role==r.guest_role) || (!r.steam && (!r.loopback || !r.local_peer)))return Route::Refuse;
    if(r.host_role && r.net_host && !r.native_guest && r.create && r.accept)return Route::Host;
    if(r.guest_role && !r.net_host && r.native_guest && !r.create && !r.accept)return Route::Guest;
    return Route::Refuse;
}
// Exact native descriptor names; no substring-based admission. This is the
// deliberately small v1 service/loading/movement surface, not every kind-4 Req.
inline bool request_name(std::string_view n){
    for(const auto allowed:{"ConnectCryptogramToServerReq","LogoutSessionToServerReq",
        "LoginUserToFieldServerReq","LoginCharacterToFieldServerReq","PreLoadSequencerReq",
        "GameLoadingStartReq","MoveFieldCompleteReq","LoadingCompleteReq",
        "MoveActorReq","RotateActorReq","StartActionReq","EndActionReq",
        "WorldObserverDisableLoadCompleteReq","SceneLevelUnloadReq",
        "SceneLevelLoadCompleteSyncSceneObjectListReq"})if(n==allowed)return true;
    return false;
}
enum class Phase { Title, Loading, Stable, Closing, Closed, Quarantine };
struct Lifecycle {
    Phase phase=Phase::Title;bool logout_queued=false;
    bool admit(uint16_t op)const{
        if(phase==Phase::Closed || phase==Phase::Quarantine || logout_queued)return false;
        if(op==2001 && !can_logout())return false;
        if((op==3081 || op==2350) && phase==Phase::Stable)return false;
        return true;
    }
    void request(uint16_t op){
        if(phase==Phase::Closed || phase==Phase::Quarantine)return;
        if(op==3081 || op==2350)phase=Phase::Loading;
        if(op==2001){logout_queued=true;phase=Phase::Closing;}
    }
    void response(uint16_t op){
        if(phase==Phase::Closed || phase==Phase::Quarantine)return;
        if(op==2891 && phase==Phase::Loading)phase=Phase::Stable;
        if(op==2002 && phase==Phase::Closing)phase=Phase::Closed;
    }
    // No guessed completion payload if the peer disappears while loading.
    bool can_logout()const{return !logout_queued && (phase==Phase::Title || phase==Phase::Stable);}
    void lost(){if(!can_logout() && !logout_queued && phase!=Phase::Closed)phase=Phase::Quarantine;}
};
}
