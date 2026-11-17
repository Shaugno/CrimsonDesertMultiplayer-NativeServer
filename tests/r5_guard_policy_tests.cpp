#include "coop3663/r5_guard_policy.h"
#include <array>
#include <cstdlib>
#include <cstdio>
namespace p=cdmp::coop3663::r5_guard;
struct Context {uintptr_t rax=0x2000,rdi=0x3000,rbx=0x4000,rsp=0x5000,r15=0x6000,rsi=0x7000,rip=0x8000;};
static void check(bool v){if(!v)std::abort();}
int main(){
    for(auto own:std::array<uintptr_t,3>{0,0x1000,0x2000})
    for(auto manager:std::array<uintptr_t,2>{0,0x2000})
    for(auto leaver:std::array<uint32_t,3>{0,0x90000001,0x90000002})
    for(auto owner:std::array<uint32_t,3>{0,0x90000001,0x90000002}){
        Context c;c.rax=manager;const auto before=c;
        const bool expected=own && manager && own!=manager && leaver && owner && leaver!=owner;
        check(p::preserve_owner(c,own,leaver,owner,0x9000)==expected);
        check(c.rip==(expected?0x9000:before.rip));
        check(c.rax==before.rax && c.rdi==before.rdi && c.rbx==before.rbx && c.rsp==before.rsp);
    }
    // Preserve native frame/owned arguments, and choose cleanup only after a
    // remote conversation. No direct return or fabricated destructor calls.
    for(bool remote:{false,true}){
        Context c;const auto before=c;
        check(p::skip_achievement(c,remote,0xA000)==remote);
        check(c.rip==(remote?0xA000:before.rip));
        check(c.rsp==before.rsp && c.r15==before.r15 && c.rsi==before.rsi && c.rax==before.rax);
    }
    std::puts("R5 owner unload and native achievement cleanup policy PASS");
}
