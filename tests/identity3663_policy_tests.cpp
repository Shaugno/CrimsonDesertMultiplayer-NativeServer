#include "coop3663/identity3663_policy.h"
#include "coop3663/identity3663_diagnostic.h"
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
    s.failed=false;payload[0]=4;
    const auto diagnose=[&](std::span<const uint8_t> bytes){return diagnose_login(s,2,100,s.user_key,bytes,true,false,true,11,12,13);};
    auto d=diagnose(payload);
    check(d.discriminator==4 && d.number==kGuest && d.slot==0 && d.bound && d.size_ok && d.generation_ok);
    auto line=format_login_diagnostic(d);
    for(const auto* expected:{"length=13","prefix13=0402F0FFFFFFFFFF7F00000000","discriminator=0x04",
        "number=0x7FFFFFFFFFFFF002","slot=0","bound=true","lobby_ready=true","size=true",
        "number=true","slot=true","generation=true","lobby_edits=11 status_edits=12 phase_edits=13","state_epoch=2"})
        check(line.find(expected)!=std::string::npos);
    for(size_t n=0;n<13;++n){
        d=diagnose({payload.data(),n});check(!d.size_ok && d.prefix_size==n);
        check(d.discriminator_available==(n>=1) && d.number_available==(n>=9) && !d.slot_available);
    }
    std::array<uint8_t,19> extended{};std::copy(payload.begin(),payload.end(),extended.begin());
    d=diagnose(extended);check(!d.size_ok && d.number_ok && d.slot_ok && d.prefix_size==13);
    check(!login_character(s,2,100,extended)); // diagnostics do not relax the exact-length gate
    payload[9]=3;d=diagnose(payload);check(!d.slot_ok && d.slot==3);
    payload[9]=206;d=diagnose(payload);check(!d.slot_ok && d.slot==206);
    payload[9]=0;payload[1]^=1;d=diagnose(payload);check(!d.number_ok && d.size_ok);
    payload[1]^=1;s.lobby_ready=false;d=diagnose(payload);check(!d.lobby_ready && d.bound);
    s.lobby_ready=true;s.failed=true;d=diagnose(payload);check(!d.bound && !d.state_ok);
    s.failed=false;d=diagnose_login(s,3,101,s.user_key+1,payload,true,false,true,0,0,0);
    check(!d.bound && !d.epoch_match && !d.user_match && !d.generation_ok);
    d=diagnose_login(s,0,0,0,{},false,true,false,0,0,0);
    line=format_login_diagnostic(d);check(line.find("number=unavailable")!=std::string::npos &&
        line.find("active=false broken=true context=false")!=std::string::npos && !d.epoch_nonzero && !d.guest_nonzero);
    // The observed predicates agree with the pre-existing decision for all
    // state gates and every bounded length; the runtime still calls that policy.
    for(unsigned mask=0;mask<32;++mask)for(size_t n=0;n<=extended.size();++n){
        State candidate=s;candidate.failed=(mask&1)!=0;candidate.lobby_ready=(mask&2)!=0;
        const uint64_t epoch=(mask&4)?2:0;const uintptr_t user=(mask&8)?100:101;
        const uint32_t current=(mask&16)?candidate.user_key:candidate.user_key+1;
        const std::span<const uint8_t> bytes(extended.data(),n);
        const auto report=diagnose_login(candidate,epoch,user,current,bytes,true,false,true,0,0,0);
        check((current==candidate.user_key && login_character(candidate,epoch,user,bytes))==
            (report.generation_ok && report.bound && report.lobby_ready && report.size_ok && report.number_ok && report.slot_ok));
    }
    RefusalDiagnosticOnce once;check(once.claim());for(unsigned i=0;i<100;++i)check(!once.claim());
    std::puts("3663 identity policy: ambiguous/duplicate identities, phase and epoch gates, affinity, malformed login and forbidden slots PASS");
}
