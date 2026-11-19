#include "mod/session2/v1_3663.h"
#include "mod/session2/r4_v1_bindings.h"
#include "mod/session2/server.h"
#include "mod/session2/guest3663.h"
#include "mod/session2/character_guard3663.h"
#include "mod/session2/crash3663.h"
#include "mod/session2/ride_filter_logic.h"
#include "mod/session2/identity3663.h"
#include "mod/session2/report_guard.h"
#include "mod/session2/r5_guards3663.h"
#include "mod/session2/tunnel_layout.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <windows.h>
#include <safetyhook.hpp>
#include <array>
#include <atomic>
#include <format>

namespace cdmp::session2::v1_3663 {
namespace {
namespace p=coop3663::v1;
namespace n=r4_v1_native;
constexpr uintptr_t base=game::build_3663::kImageBase;
SafetyHookInline control_hook,self_hook,loading_hook;
std::atomic<uintptr_t> known_guest{};
std::atomic<bool> installed{},fault{};
std::atomic<uint64_t> relayed{},refused{};
std::atomic<uint32_t> move_owner_reports{};
std::mutex life_mutex; // leaf lock, no native calls while held
p::Lifecycle life;
template<class T>bool read(uintptr_t a,T& v){return a && game::safe_read(a,&v,sizeof v);}
uintptr_t ptr(uintptr_t a){uintptr_t v=0;return read(a,v)?v:0;}
uint32_t key(uintptr_t a){uint32_t v=0;return read(a+0x60,v)?v:0;}
bool remove(uintptr_t array,uintptr_t except){
    uintptr_t data=0;uint32_t count=0,capacity=0;std::array<uintptr_t,64> values{};
    if(!read(array,data) || !read(array+8,count) || !read(array+12,capacity) || count>64 || count>capacity ||
        (count && (!data || !game::safe_read(data,values.data(),count*sizeof(uintptr_t)))))return false;
    size_t kept=0;for(uint32_t i=0;i<count;++i)if(values[i]!=except)values[kept++]=values[i];
    if(kept==count)return true;
    __try{std::memcpy(reinterpret_cast<void*>(data),values.data(),kept*sizeof(uintptr_t));
        *reinterpret_cast<uint32_t*>(array+8)=static_cast<uint32_t>(kept);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
uintptr_t finder(SafetyHookInline& hook,void* f,void* out){
    const uintptr_t original=hook.unsafe_call<uintptr_t>(f,out);
    if(!installed.load() || fault.load())return original;
    field_share3663::Context c;
    if(!host_context(c) || !c.guest)return original;
    const uintptr_t actor=ptr(reinterpret_cast<uintptr_t>(f)+0x10),owner=ptr(actor+0xA0);
    uint32_t controller=0;
    // Only the two ordinary primary characters. No invented actor-session
    // virtual offset, NPC/controller aliasing or optional actor ownership path.
    if((key(actor)>>28)!=0xA || (owner!=c.host && owner!=c.guest) ||
        !read(actor+0x90,controller) || controller!=key(owner))return original;
    const uintptr_t except=owner==c.host?server_host_session():server_live_session2();
    if(!except)return original;
    const auto result=reinterpret_cast<uintptr_t(__fastcall*)(void*,void*)>(base+n::kToNear.rva)(f,out);
    if(!remove(reinterpret_cast<uintptr_t>(out),except)){fault.store(true);++refused;return result;}
    ++relayed;return result;
}
uintptr_t __fastcall control(void* f,void* out){return finder(control_hook,f,out);}
uintptr_t __fastcall self(void* f,void* out){return finder(self_hook,f,out);}
uint32_t* __fastcall loading(void* ts_arg,uint32_t* result,void* field,void* fourth,uint8_t flag,uint64_t* now){
    const auto ts=reinterpret_cast<uintptr_t>(ts_arg),owner=ptr(ptr(ts+8)+0xA0);
    field_share3663::Context c;bool guest=owner && owner==known_guest.load();
    if(host_context(c) && c.guest){known_guest.store(c.guest);guest=owner==c.guest;}
    // Initial join still reaches LoadingComplete. Its fifth argument controls
    // a separate world-clock publisher; filtering travel/time requests alone
    // is insufficient. Preserve all six native arguments except this flag.
    return loading_hook.unsafe_call<uint32_t*>(ts_arg,result,field,fourth,p::loading_flag(guest,flag),now);
}
bool matches(const r4_native::Site& s){std::array<uint8_t,16> bytes{};return game::safe_read(base+s.rva,bytes.data(),bytes.size()) && bytes==s.bytes;}
bool prepare(const r4_native::Site& s,void* fn,SafetyHookInline& h){
    auto result=game::prepare_inline(base+s.rva,fn,[s](const auto& bytes){return bytes.size()==6 &&
        std::equal(bytes.begin(),bytes.end(),s.bytes.begin());},"3663 v1","named recipient entry");
    if(!result)return false;h=std::move(*result);return true;
}
}
bool host_context(field_share3663::Context& c){
    c={};c.epoch=server_epoch();
    if(!server_host_user(c.host) || !c.host || (key(c.host)>>28)!=9)return false;
    (void)server_user2(c.guest);
    return !c.guest || ((key(c.guest)>>28)==9 && c.guest!=c.host);
}
bool host_ready(){
    field_share3663::Context c;if(!installed.load() || stopped() || !host_context(c))return false;
    const uintptr_t actor=ptr(c.host+0xD0),ts=ptr(ptr(actor+0x68)+0x1A0),fm=ts+0x508,field=ptr(fm+0x10);
    uint32_t fieldkey=0,copy=0,owner=0;uint8_t open=0,moving=1,ts_moving=1;uint16_t busy=1;
    return actor && ts && field && (key(actor)>>28)==0xA && ptr(actor+0xA0)==c.host && ptr(ts+8)==actor &&
        read(field+0x2D0,fieldkey) && read(fm+8,copy) && fieldkey==copy &&
        read(field+0x180,owner) && owner==key(c.host) && read(field+0x4A,open) && open==1 &&
        read(fm+0x48,moving) && !moving && read(fm+0x4A,busy) && !busy && read(ts+0x3DC,ts_moving) && !ts_moving;
}
bool host_install(const game::build_guard::ImageIdentity& id){
    if(installed.load())return true;
    if(!game::build_guard::select_3663_tiers(id,{true,true,true,true},
        {game::build_3663::kTierAComplete,game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete,game::build_3663::kTierDComplete}).tier_d_lifecycle)return false;
    for(const auto& s:{n::kExceptControl,n::kExceptSelf,n::kToNear,n::kLoading,n::kLoadingFlag})if(!matches(s))return false;
    if(ptr(base+0x5F6DAC0+8)!=base+n::kExceptControl.rva || ptr(base+0x5F6DAA0+8)!=base+n::kExceptSelf.rva ||
        ptr(base+0x5F6DA80+8)!=base+n::kToNear.rva)return false;
    if(!crash3663::install_for_session(role::Role::Host)){
        log::error("3663 host crash capture could not be installed in the session evidence directory");
        return false;
    }
    crash3663::arm();
    if(!character_guard3663::install(id,role::Role::Host))return false;
    if(!r5_guards3663::host_install(id) || !identity3663::install(id,&host_context) || !field_share3663::install(id,&host_context,&identity3663::ready,&server_halt_core_native_fault))return false;
    auto load=game::prepare_inline(base+n::kLoading.rva,reinterpret_cast<void*>(&loading),
        [](const auto& bytes){return bytes.size()==5 && std::equal(bytes.begin(),bytes.end(),n::kLoading.bytes.begin());},
        "3663 v1","six-argument LoadingComplete entry");
    if(!load)return false;loading_hook=std::move(*load);
    // Native logout may reach failed-save reporting. Use the separately
    // adopted target reporter/guard; never the legacy target RVA.
    if(!report_guard_install_at(reinterpret_cast<void*>(base+r4_native::kReporter.rva),
        r4_native::kReporter.bytes.data(),r4_native::kReporter.bytes.size()))return false;
    if(!prepare(n::kExceptControl,reinterpret_cast<void*>(&control),control_hook) ||
        !prepare(n::kExceptSelf,reinterpret_cast<void*>(&self),self_hook))return false;
    if(!loading_hook.enable() || !control_hook.enable() || !self_hook.enable()){
        if(loading_hook)(void)loading_hook.disable();
        if(control_hook)(void)control_hook.disable();if(self_hook)(void)self_hook.disable();return false;}
    installed.store(true,std::memory_order_release);return true;
}
void note_request(uint16_t op){std::lock_guard lock(life_mutex);life.request(op);}
void note_response(uint16_t op){
    std::lock_guard lock(life_mutex);
    life.response(op,field_share3663::shared_entry_committed());
}
void prepare_shared_move_for_guest(std::span<uint8_t> payload, int8_t slot){
    if(!field_share3663::shared_entry_committed())return;
    field_share3663::Context c;
    p::MoveFieldKeys keys;
    if(host_context(c) && c.guest){
        const auto actor=ptr(c.guest+0xD0);
        if(actor && ptr(actor+0xA0)==c.guest)keys={key(c.host),key(c.guest),key(actor)};
    }
    const auto result=p::alias_shared_move_owner(payload,slot,keys);
    if(move_owner_reports.fetch_add(1,std::memory_order_relaxed)<8)try{
        constexpr std::array labels{"unchanged","unexpected_shape","unverified_owner","aliased"};
        log::info("3663 MoveFieldAck 2364 guest copy: owner_alias={} payload_bytes={} host=0x{:X} guest=0x{:X} guest_actor=0x{:X}",
            labels[static_cast<size_t>(result)],payload.size(),keys.host,keys.guest,keys.guest_actor);
    }catch(...){}
}
bool admit_request(uint16_t op){std::lock_guard lock(life_mutex);
    const bool allowed=life.admit(op,field_share3663::private_entry_committed() && !field_share3663::shared_entry_committed());
    if(op==2001 && !allowed)life.lost();return allowed;}
bool allow_disconnect_logout(){std::lock_guard lock(life_mutex);return life.can_logout();}
void note_lost(){std::lock_guard lock(life_mutex);life.lost();}
bool stopped(){bool quarantined=false;{std::lock_guard lock(life_mutex);quarantined=life.phase==p::Phase::Quarantine;}
    return quarantined || fault.load() || identity3663::failed() || field_share3663::quarantined();}
std::string describe(){int phase=0;{std::lock_guard lock(life_mutex);phase=static_cast<int>(life.phase);}
    return std::format("v1 phase {} recipients {} refused {} stopped {} | {} | {}",phase,relayed.load(),refused.load(),
        stopped(),identity3663::describe(),field_share3663::describe())+" | "+r5_guards3663::describe();}

Bridge::Bridge(ClientSession& session,bool host):host_(host),
    link_(session,host?troc::Dir::ToGuestClient:troc::Dir::ToHostServer,
        host?troc::Dir::ToHostServer:troc::Dir::ToGuestClient,
        [this](std::vector<uint8_t>&& packet,uint32_t now){receive(std::move(packet),now);}){
    link_.set_compress(true);if(!host_)link_.set_announce_caps(true);
}
Bridge::~Bridge(){
    if(host_)character_guard3663::set_link_ready(false);
    else guest3663::set_host_ready(false);
}
void Bridge::receive(std::vector<uint8_t>&& packet,uint32_t now){
    const auto view=parse_tunnel_packet(packet,host_?troc::Dir::ToHostServer:troc::Dir::ToGuestClient,kTunnelLayout3663);
    if(failed_ || !view){failed_=true;return;}
    if(!host_){
        if(view->opcode==troc::kControlOpcode){
            if(view->payload.size()==1 && view->payload[0]==troc::kControlHostReady){last_ready_=now;guest3663::set_host_ready(true);}
            else if(view->payload.size()==1 && view->payload[0]==troc::kControlHostDead){failed_=true;guest3663::set_host_ready(false);}
            else if(view->slot==-1){
                const auto key=rfl::host_character_from_control(view->payload.data(),view->payload.size());
                if(key)character_guard3663::note_peer_character(key);
            }
            return;
        }
        guest3663::deliver(std::move(packet));return;
    }
    const auto received=crash3663::packet(crash3663::Direction::ToHost,view->opcode,
                                          static_cast<uint16_t>(packet.size()));
    crash3663::returned(received);
    if(blocked_guest_control_3663(view->opcode)){++refused_;return;}
    if(pending_.size()>=65536 || packet.size()>(32u<<20)-pending_bytes_){failed_=true;return;}
    if(pending_.empty())first_pending_=now;
    pending_bytes_+=packet.size();pending_.push_back(std::move(packet));
}
void Bridge::tick(uint32_t now){
    link_.tick(now);
    if(host_){
        const bool current=host_ready();
        if(current && !world_ready_)log::info("3663 v1 host world ready: native primary/field identity, owner and idle flags agree");
        world_ready_=current;
    }
    const bool connected=link_.connected();
    const bool lost=was_connected_ && (!connected || link_.broken() || (epoch_ && epoch_!=link_.session_epoch()));
    if(connected && !epoch_)epoch_=link_.session_epoch();
    if(host_ && (lost || (failed_ && was_connected_) || stopped() || server_inject_halted() || server_outgoing_lost())){
        if(!logout_pending_ && allow_disconnect_logout())logout_pending_=true;
        note_lost();failed_=true;
    }
    if(!host_ && (lost || guest3663::failed())){failed_=true;guest3663::set_host_ready(false);}
    if(host_)character_guard3663::set_link_ready(connected && !failed_);
    if(host_ && logout_pending_ && !server_inject_halted()){
        const auto result=server_inject_ex(troc::make_logout_session_req());
        if(inj::disposition(result)!=inj::Disposition::Hold){
            logout_pending_=false;log::info("3663 v1 disconnect native logout: {}",inj::outcome_name(result));}
    }
    was_connected_=connected;
    if(!connected || failed_)return;
    if(host_){
        if(host_ready() && now-last_ready_>=1000){
            const uint8_t state=troc::kControlHostReady;
            if(!link_.push(troc::make(troc::kControlOpcode,-1,{&state,1})))failed_=true;
            last_ready_=now;
            field_share3663::Context c;
            if(host_context(c)){
                const auto actor=ptr(c.host+0xD0);
                const auto primary=actor && ptr(actor+0xA0)==c.host?key(actor):0;
                if((primary>>28)==0xA && primary!=advertised_host_){
                    std::array<uint8_t,5> payload{troc::kControlHostCharacter};
                    std::memcpy(payload.data()+1,&primary,sizeof primary);
                    if(!link_.push(troc::make(troc::kControlOpcode,-1,payload)))failed_=true;
                    else advertised_host_=primary;
                }
            }
        }
        while(!pending_.empty()){
            const auto outcome=server_inject_ex(pending_.front());
            const auto disposition=inj::disposition(outcome);
            if(disposition==inj::Disposition::Hold){if(now-first_pending_>15000)failed_=true;break;}
            if(disposition==inj::Disposition::Stop){failed_=true;break;}
            if(disposition==inj::Disposition::Done)++queued_;else ++refused_;
            pending_bytes_-=pending_.front().size();pending_.pop_front();
        }
        for(auto& packet:server_take_outgoing()){
            const auto view=parse_tunnel_packet(packet,troc::Dir::ToGuestClient,kTunnelLayout3663);
            const auto sent=view?crash3663::packet(crash3663::Direction::ToClient,view->opcode,
                                                    static_cast<uint16_t>(packet.size())):0;
            if(!link_.push(packet)){failed_=true;break;}
            crash3663::returned(sent);
        }
    }else{
        if(last_ready_ && now-last_ready_>5000)guest3663::set_host_ready(false);
        for(auto& packet:guest3663::take_outgoing())if(!link_.push(packet)){failed_=true;break;}
    }
    link_.tick(now);
    if(host_ && (failed_ || link_.broken() || !link_.connected()))character_guard3663::set_link_ready(false);
}
std::string Bridge::describe()const{return std::format("3663 v1 {} queued {} refused {} pending {} failed {} | {} | {}",
    host_?"host":"guest",queued_,refused_,pending_.size(),failed_,host_?v1_3663::describe():guest3663::describe(),link_.describe());}
}
