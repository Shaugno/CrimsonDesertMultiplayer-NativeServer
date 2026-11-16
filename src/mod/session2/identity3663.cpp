// 3663-only identity adapter. No legacy patches, TLS realm offset or optional
// identity_probe hooks are installed by this module.
#include "mod/session2/identity3663.h"
#include "mod/session2/r4_native_bindings.h"
#include "coop3663/identity3663_policy.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <windows.h>
#include <safetyhook.hpp>
#include <array>
#include <atomic>
#include <format>
#include <mutex>

namespace cdmp::session2::identity3663 {
namespace {
namespace p=coop3663::identity;
namespace n=r4_native;
constexpr uintptr_t base=game::build_3663::kImageBase;
field_share3663::ReadContext context_reader{};
SafetyHookInline lobby_hook,load_hook,phases_hook;
std::atomic<bool> on{},broken{};
std::atomic<uint32_t> affinity_thread{};
std::atomic<uint64_t> lobby_edits{},status_edits{},phase_checks{},refusals{};
std::mutex state_mutex; // leaf lock; never across an original or native call
p::State state;
template<class T>bool read(uintptr_t a,T& v){return a && game::safe_read(a,&v,sizeof v);}
uintptr_t ptr(uintptr_t a){uintptr_t v=0;return read(a,v)?v:0;}
uint32_t key(uintptr_t a){uint32_t v=0;return read(a+0x60,v)?v:0;}
bool replace(uintptr_t at,uint64_t expected,uint64_t wanted){
    if(!at || (at&7))return false;
    __try{return static_cast<uint64_t>(InterlockedCompareExchange64(reinterpret_cast<volatile LONG64*>(at),
        static_cast<LONG64>(wanted),static_cast<LONG64>(expected)))==expected;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
bool set_error(uint32_t* result){
    uint32_t error=0;if(!read(base+0x71D0D3C,error) || !error || !result)return false;
    __try{*result=error;return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void fail(const char* reason){
    if(!broken.exchange(true))log::error("3663 identity failed: {}; further guest admission refused",reason);
    std::lock_guard lock(state_mutex);state.failed=true;
}
bool context(field_share3663::Context& c){return context_reader && context_reader(c) && c.host && c.epoch;}
bool same(const field_share3663::Context& c){field_share3663::Context now;return context(now) &&
    c.host==now.host && c.guest==now.guest && c.epoch==now.epoch;}
// Affinity is learned only from a VALIDATED HOST native lobby/load/phase
// callback. Never the worker, network thread, first arbitrary callback or an
// environment thread ID. The passive trace independently showed these native
// callbacks share one thread, distinct from packet receipt.
bool affinity(uintptr_t owner,const field_share3663::Context& c){
    const uint32_t current=GetCurrentThreadId();
    if(owner==c.host){uint32_t empty=0;(void)affinity_thread.compare_exchange_strong(empty,current);}
    return p::affinity(affinity_thread.load(),current);
}
uintptr_t lobby_for(uintptr_t user){
    const uintptr_t component=ptr(ptr(user+0x68)+0x10);
    return ptr(component)==base+0x5F6F2E8 && ptr(component+8)==user?component:0;
}
bool lobby_numbers(uintptr_t component,std::array<uint64_t,64>& values,uintptr_t& data,uint32_t& count){
    const uintptr_t list=ptr(component+0x18);uint32_t capacity=0;
    if(!list || !read(list,data) || !read(list+8,count) || !read(list+12,capacity) || count>64 || count>capacity || (count && !data))return false;
    for(uint32_t i=0;i<count;++i)if(!read(data+i*0xD0ull,values[i]))return false;
    return true;
}
uintptr_t status_number(uintptr_t actor){
    const uintptr_t status=ptr(ptr(actor+0x68)+0x20);
    return ptr(status)==base+0x5F63430 && ptr(status+8)==actor?ptr(status+0x90):0;
}
// A conservative bounded uniqueness check on the adopted logged-in list.
// Native identity callbacks serialize its mutation on the learned affinity.
bool unique(uintptr_t except){
    const uintptr_t manager=ptr(ptr(base+0x72456E0));
    uint32_t count=0;uintptr_t data=0;
    if(!manager || ptr(manager)!=base+0x5F69100 || !read(manager+0xC0,count) || count>0x4000 ||
        !read(manager+0xB8,data) || (count && !data))return false;
    for(uint32_t i=0;i<count;++i){
        uintptr_t actor=0;if(!read(data+i*8ull,actor))return false;
        if(!actor || actor==except)continue;
        const auto number=status_number(actor);uint64_t value=0;
        if(number && (!read(number,value) || value==p::kGuest))return false;
    }
    return true;
}
bool normalize_lobby(uintptr_t component,const field_share3663::Context& c){
    if(broken.load() || !affinity(c.guest,c) || !same(c) || !unique(0))return false;
    std::array<uint64_t,64> values{};uintptr_t data=0;uint32_t count=0;
    if(!lobby_numbers(component,values,data,count))return false;
    const auto plan=p::lobby({values.data(),count});
    if(plan.edit==p::Edit::Refuse)return false;
    const auto at=data+plan.index*0xD0ull;
    if(plan.edit==p::Edit::Replace && !replace(at,p::kNative,p::kGuest))return false;
    uint64_t check=0;if(!read(at,check) || check!=p::kGuest || !same(c))return false;
    {std::lock_guard lock(state_mutex);
     if(state.epoch && (state.epoch!=c.epoch || state.user!=c.guest))return false; // reconnect not silently reused
     state.epoch=c.epoch;state.user=c.guest;state.thread=GetCurrentThreadId();state.lobby_ready=true;}
    ++lobby_edits;return true;
}
bool normalize_status(uintptr_t actor,const field_share3663::Context& c){
    if(broken.load() || !affinity(c.guest,c) || !same(c) || !unique(actor))return false;
    {std::lock_guard lock(state_mutex);if(!p::bound(state,c.epoch,c.guest) || !state.lobby_ready)return false;}
    const uintptr_t at=status_number(actor);uint64_t before=0,after=0;
    if(!at || !read(at,before))return false;
    const auto plan=p::status(before);
    if(plan==p::Edit::Refuse || (plan==p::Edit::Replace && !replace(at,p::kNative,p::kGuest)) ||
        !read(at,after) || after!=p::kGuest || !same(c))return false;
    {std::lock_guard lock(state_mutex);state.actor=actor;state.status_ready=true;}
    ++status_edits;return true;
}
bool invariant(uintptr_t actor,const field_share3663::Context& c){
    if(broken.load() || !affinity(c.guest,c) || !same(c))return false;
    std::array<uint64_t,64> values{};uintptr_t data=0;uint32_t count=0;uint64_t number=0;
    if(!lobby_numbers(lobby_for(c.guest),values,data,count) ||
        p::lobby({values.data(),count}).edit!=p::Edit::Already || !read(status_number(actor),number) || number!=p::kGuest)return false;
    {std::lock_guard lock(state_mutex);
     if(!p::bound(state,c.epoch,c.guest) || state.actor!=actor || !state.status_ready || !state.lobby_ready)return false;
     state.phases_ready=true;}
    ++phase_checks;return true;
}
uint32_t* __fastcall lobby_detour(void* component,uint32_t* result,void* unused,void* data){
    auto* returned=lobby_hook.unsafe_call<uint32_t*>(component,result,unused,data);
    if(!on.load())return returned;
    field_share3663::Context c;uint32_t code=~0u;
    const auto comp=reinterpret_cast<uintptr_t>(component),owner=ptr(comp+8);
    if(ptr(comp)!=base+0x5F6F2E8 || !context(c))return returned;
    if(owner==c.host){(void)affinity(owner,c);return returned;}
    if(!c.guest || owner!=c.guest)return returned;
    if(returned!=result || !read(reinterpret_cast<uintptr_t>(returned),code) || code)return returned;
    if(!normalize_lobby(comp,c)){fail("guest lobby normalization/affinity/uniqueness");(void)set_error(result);}
    return returned;
}
uint32_t* __fastcall load_detour(void* actor_arg,uint32_t* result,void* store,void* name,uint32_t slot){
    auto* returned=load_hook.unsafe_call<uint32_t*>(actor_arg,result,store,name,slot);
    if(!on.load())return returned;
    field_share3663::Context c;uint32_t code=~0u;
    const auto actor=reinterpret_cast<uintptr_t>(actor_arg),owner=ptr(actor+0xA0);
    if((key(actor)>>28)!=0xA || !context(c))return returned;
    if(owner==c.host){(void)affinity(owner,c);return returned;}
    if(!c.guest || owner!=c.guest)return returned;
    if(returned!=result || !read(reinterpret_cast<uintptr_t>(returned),code) || code)return returned;
    if(!normalize_status(actor,c)){fail("guest status normalization/affinity/uniqueness");(void)set_error(result);}
    return returned;
}
uint32_t* __fastcall phases_detour(void* status,uint32_t* result,void* third,uint32_t fourth){
    if(on.load()){
        field_share3663::Context c;const uintptr_t actor=ptr(reinterpret_cast<uintptr_t>(status)+8),owner=ptr(actor+0xA0);
        if((key(actor)>>28)==0xA && context(c)){
            if(owner==c.host)(void)affinity(owner,c);
            else if(c.guest && owner==c.guest && !invariant(actor,c)){
                fail("guest identity invariant before login phases");(void)set_error(result);return result;
            }
        }
    }
    return phases_hook.unsafe_call<uint32_t*>(status,result,third,fourth);
}
bool matches(const n::Site& s){std::array<uint8_t,16> bytes{};return game::safe_read(base+s.rva,bytes.data(),bytes.size()) && bytes==s.bytes;}
bool prepare(const n::Site& s,void* fn,SafetyHookInline& target){
    auto h=game::prepare_inline(base+s.rva,fn,[s](const auto& bytes){return bytes.size()==5 &&
        std::equal(bytes.begin(),bytes.end(),s.bytes.begin());},"3663 identity","native entry");
    if(!h)return false;target=std::move(*h);return true;
}
}
bool install(const game::build_guard::ImageIdentity& id,field_share3663::ReadContext read_context){
    if(on.load())return true;
    if(!read_context || !game::build_guard::select_3663_tiers(id,{true,true,true,true},
        {game::build_3663::kTierAComplete,game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete,game::build_3663::kTierDComplete}).tier_c_join)return false;
    for(const auto& s:{n::kLobbyLoad,n::kLoadComponents,n::kPhases,n::kClearLobby,n::kClearList,n::kLoginMgr,n::kLobbyConstant,n::kLobbyAdd})if(!matches(s))return false;
    if(ptr(base+0x5F6F2E8+0x50)!=base+n::kLobbyLoad.rva || ptr(base+0x5F63430+0x50)!=base+0x2C160B0)return false;
    context_reader=read_context;
    if(!prepare(n::kLobbyLoad,reinterpret_cast<void*>(&lobby_detour),lobby_hook) ||
        !prepare(n::kLoadComponents,reinterpret_cast<void*>(&load_detour),load_hook) ||
        !prepare(n::kPhases,reinterpret_cast<void*>(&phases_detour),phases_hook))return false;
    if(!phases_hook.enable() || !load_hook.enable() || !lobby_hook.enable()){
        if(phases_hook)(void)phases_hook.disable();if(load_hook)(void)load_hook.disable();
        if(lobby_hook)(void)lobby_hook.disable();return false;
    }
    on.store(true,std::memory_order_release);return true;
}
bool active(){return on.load();}
bool failed(){return broken.load();}
bool ready(uintptr_t user,uintptr_t actor,uint64_t epoch){std::lock_guard lock(state_mutex);return p::ready(state,epoch,user,actor);}
bool admit(uint16_t opcode,std::span<const uint8_t> payload){
    if(!on.load() || broken.load())return false;
    if(opcode!=3081)return true;
    field_share3663::Context c;if(!context(c))return false;
    std::lock_guard lock(state_mutex);const bool ok=p::login_character(state,c.epoch,c.guest,payload);
    if(!ok)++refusals;return ok;
}
std::string describe(){return std::format("3663 identity: lobby {} status {} phases {} refused {} affinity {} failed {}",
    lobby_edits.load(),status_edits.load(),phase_checks.load(),refusals.load(),affinity_thread.load(),broken.load());}
}
