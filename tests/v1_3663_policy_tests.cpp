#include "coop3663/v1_3663_policy.h"
#include <cstdlib>
#include <cstdio>
using namespace cdmp::coop3663::v1;
static void check(bool value){if(!value)std::abort();}
int main(){
    for(unsigned flags=0;flags<128;++flags){
        const bool marker=(flags&1)!=0,game=(flags&2)!=0,target=(flags&4)!=0,host=(flags&8)!=0,
            guest=(flags&16)!=0,sandbox=(flags&32)!=0,real=(flags&64)!=0;
        const bool allowed=!marker || !game || (target && host!=guest && (guest?sandbox:real&&!sandbox));
        check(reject_test_process(marker,game,target,host,guest,sandbox,real)==!allowed);
    }
    RouteRequest host{true,false,true,true,true,false,true,false,true,true,false,true,true,false,false};
    check(route(host)==Route::Host);
    auto guest=host;guest.sandbox=true;guest.host_role=false;guest.guest_role=true;guest.net_host=false;guest.native_guest=true;guest.create=guest.accept=false;
    check(route(guest)==Route::Guest);
    auto illegal_host=host;illegal_host.sandbox=true;check(route(illegal_host)==Route::Refuse);
    auto isolated=guest;isolated.guest_startup_only=true;isolated.network=isolated.session=false;
    check(route(isolated)==Route::GuestStartup);
    auto unisolated=isolated;unisolated.sandbox=false;check(route(unisolated)==Route::Refuse);
    auto networked=isolated;networked.network=true;check(route(networked)==Route::Refuse);
    auto sessioned=isolated;sessioned.session=true;check(route(sessioned)==Route::Refuse);
    auto host_startup=host;host_startup.guest_startup_only=true;check(route(host_startup)==Route::Refuse);
    for(int field=0;field<12;++field){auto bad=guest;
        if(field==0)bad.permissions=false;if(field==1)bad.sandbox=false;if(field==2)bad.network=false;
        if(field==3)bad.session=false;if(field==4)bad.host_role=true;if(field==5)bad.net_host=true;
        if(field==6)bad.native_guest=false;if(field==7)bad.create=true;if(field==8)bad.accept=true;
        if(field==9)bad.local_peer=false;if(field==10)bad.coop=true;if(field==11)bad.diagnostic=true;
        check(route(bad)==Route::Refuse);
    }
    guest.loopback=false;check(route(guest)==Route::Refuse);guest.steam=true;check(route(guest)==Route::Guest);
    for(unsigned flag=0;flag<=255;++flag){check(loading_flag(true,static_cast<uint8_t>(flag))==0);
        check(loading_flag(false,static_cast<uint8_t>(flag))==flag);}
    for(const auto name:{"ConnectCryptogramToServerReq","LoginUserToFieldServerReq","LoginCharacterToFieldServerReq",
        "LoginCharacterToGameReq","PreLoadSequencerReq","GameLoadingStartReq","MoveFieldCompleteReq","LoadingCompleteReq","MoveActorReq",
        "RotateActorReq","StartActionReq","LogoutSessionToServerReq"})check(request_name(name));
    for(const auto name:{"SaveAutoReq","DeleteSaveReq","ReloadStageStartReq","SetFieldTimeReq","MoveFieldCheatReq",
        "AITeleportReq","AttackByCollisionReq","MountBeginReq","RegisterGlobalStageSequencerReq",
        "LoginCharacterToFieldServerReqForDev","MoveActorReqCheat",""})check(!request_name(name));
    check(initial_game_login_packet(0,0));
    check(!initial_game_login_packet(-1,0) && !initial_game_login_packet(1,0) && !initial_game_login_packet(0,1));
    Lifecycle s;check(s.can_logout() && s.admit(3081) && !s.admit(kLoginCharacterToGame,true));s.request(3081);
    check(s.phase==Phase::Loading && !s.can_logout() && !s.admit(2001));s.response(2268,false);check(s.phase==Phase::Loading);
    s.response(2891,false);check(s.phase==Phase::Loading && s.admit(2350) && !s.can_logout());
    check(!s.admit(kLoginCharacterToGame,false) && s.admit(kLoginCharacterToGame,true));
    s.request(kLoginCharacterToGame);
    check(!s.admit(kLoginCharacterToGame,true));
    s.request(2350);
    s.response(2891,true);check(s.phase==Phase::Stable && s.can_logout());
    check(!s.admit(3081) && !s.admit(2350) && s.admit(2790) && s.admit(2296));
    s.request(2001);check(s.phase==Phase::Closing && !s.can_logout() && !s.admit(2001) && !s.admit(2790));
    s.response(2003,true);check(s.phase==Phase::Closing);s.response(2002,true);check(s.phase==Phase::Closed && !s.admit(3081));
    s.response(2891,true);check(s.phase==Phase::Closed);
    Lifecycle lost;lost.request(3081);lost.lost();check(lost.phase==Phase::Quarantine && !lost.admit(2790));
    lost.response(2891,true);check(lost.phase==Phase::Quarantine);
    std::puts("3663 v1: exact request-name allowlist, initial loading versus travel, stable disconnect, duplicate logout and lost-loading quarantine PASS");
}
