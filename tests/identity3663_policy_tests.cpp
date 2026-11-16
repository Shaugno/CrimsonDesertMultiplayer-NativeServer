#include "coop3663/identity3663_policy.h"
#include <array>
#include <cstdlib>
#include <cstdio>
using namespace cdmp::coop3663::identity;
static void check(bool v){if(!v)std::abort();}
int main(){
    std::array<uint64_t,2> values{kNative,123};
    check(lobby(values).edit==Edit::Replace && lobby(values).index==0);
    values={123,kNative};check(lobby(values).edit==Edit::Replace && lobby(values).index==1);
    values={123,kGuest};check(lobby(values).edit==Edit::Already && lobby(values).index==1);
    for(const auto bad: {std::array<uint64_t,2>{kNative,kGuest},std::array<uint64_t,2>{kNative,kNative},
         std::array<uint64_t,2>{kGuest,kGuest},std::array<uint64_t,2>{0,123}})check(lobby(bad).edit==Edit::Refuse);
    check(lobby({}).edit==Edit::Refuse);
    check(status(kNative)==Edit::Replace && status(kGuest)==Edit::Already && status(123)==Edit::Refuse);
    check(!affinity(0,1) && !affinity(1,2) && affinity(1,1));
    State s{2,100,200,7,true,true,true,false};
    s.user_key=0x90100002;s.actor_key=0xA0100003;
    auto is_ready=[&](uint64_t epoch,uintptr_t user,uintptr_t actor){return ready(s,epoch,user,actor,0x90100002,0xA0100003);};
    check(is_ready(2,100,200) && !is_ready(3,100,200) && !is_ready(2,101,200) && !is_ready(2,100,201));
    check(!ready(s,2,100,200,0x90200002,0xA0100003)); // same pointers, new user generation
    check(!ready(s,2,100,200,0x90100002,0xA0200003)); // same pointers, new actor generation
    std::array<uint8_t,13> payload{};for(unsigned i=0;i<8;++i)payload[1+i]=static_cast<uint8_t>(kGuest>>(8*i));
    check(login_character(s,2,100,payload));
    for(size_t n=0;n<payload.size();++n)check(!login_character(s,2,100,{payload.data(),n}));
    payload[9]=3;check(!login_character(s,2,100,payload));payload[9]=206;check(!login_character(s,2,100,payload));
    payload[9]=0;payload[1]^=1;check(!login_character(s,2,100,payload));payload[1]^=1;
    s.lobby_ready=false;check(!login_character(s,2,100,payload) && !is_ready(2,100,200));
    s.lobby_ready=true;s.failed=true;check(!login_character(s,2,100,payload) && !is_ready(2,100,200));
    std::puts("3663 identity policy: ambiguous/duplicate identities, phase and epoch gates, affinity, malformed login and forbidden slots PASS");
}
