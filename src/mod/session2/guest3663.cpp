// Minimal 3663 guest. The legacy guest installer and every optional callback
// remain unreachable. Native delivery/allocation happens on NetworkThread.
#include "mod/session2/guest3663.h"
#include "mod/session2/character_guard3663.h"
#include "mod/session2/crash3663.h"
#include "mod/session2/tunnel_layout.h"
#include "mod/session2/file_sandbox3663.h"
#include "mod/session2/r4_v1_bindings.h"
#include "mod/session2/report_guard.h"
#include "mod/session2/r5_guards3663.h"
#include "mod/game/game_fault.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/game/player_layout.h"
#include "mod/log.h"
#include <windows.h>
#include <safetyhook.hpp>
#include <array>
#include <atomic>
#include <cstring>
#include <format>
#include <mutex>

namespace cdmp::session2::guest3663 {
namespace {
constexpr auto& layout=kTunnelLayout3663;
constexpr uintptr_t base=game::build_3663::kImageBase;
SafetyHookInline send_hook;SafetyHookMid pump_hook;
std::atomic<bool> on{},broken{},ready{},remote{},refresh_pending{},refreshed{};
std::atomic<uint32_t> owner_thread{};
std::atomic<uint64_t> epoch{},refresh_started{},sent{},delivered{},filtered{};
troc::PacketQueue outgoing(32u<<20);
struct Item{uint64_t epoch;std::vector<uint8_t> packet;};
std::mutex incoming_mutex;
std::deque<Item> incoming;size_t incoming_bytes=0;
// R9: NetworkThread-only, read-only sampling after the second native field ack.
// The class vtable is independently identified by 3663 RTTI; a failed hop is
// reported as unreadable and never replaced with a 3576 pointer.
uint32_t field_acks=0,load_samples=0;
uint64_t next_load_sample=0;
uint64_t guest_install_tick=0;
bool startup_capture_armed=false;
constexpr uint32_t max_load_samples=180;
constexpr uintptr_t client_sequencer_vt=0x59D57E8;
template<class T>bool read(uintptr_t a,T& v){return a && game::safe_read(a,&v,sizeof v);}
uintptr_t ptr(uintptr_t a){uintptr_t v=0;return read(a,v)?v:0;}
void sample_loading() {
    if(field_acks<2 || load_samples>=max_load_samples)return;
    const auto now=GetTickCount64();
    if(now<next_load_sample)return;
    next_load_sample=now+1000;
    ++load_samples;
    const auto world=ptr(base+game::kPlayerLayout3663.kWorldSystemSlot);
    const auto controller=ptr(world+0x60);
    uint8_t state=0,marker=0;
    const bool state_ok=controller && read(controller+0x15C,state);
    const auto manager=ptr(world+game::kPlayerLayout3663.kWorldSystem_ActorManager);
    const bool manager_ok=ptr(manager)==base+game::kPlayerLayout3663.kVtClientActorManager;
    const auto actor=manager_ok?ptr(manager+game::kPlayerLayout3663.kActorManager_ControlledActor):0;
    const bool actor_ok=ptr(actor)==base+game::kPlayerLayout3663.kVtControlledActor;
    const auto components=actor_ok?ptr(actor+game::kPlayerLayout3663.kActor_ComponentTable):0;
    const auto component=ptr(components+0xA8);
    const auto sequencer=ptr(component+0x18);
    const bool sequencer_ok=ptr(sequencer)==base+client_sequencer_vt;
    const bool marker_ok=sequencer_ok && read(sequencer+0x259,marker);
    uint32_t own_key=0,local_user=0,field_owner=0;
    if(actor_ok){(void)read(actor+0x60,own_key);(void)read(actor+0x90,local_user);}
    const auto holder=ptr(controller+0xC0),field=ptr(holder);
    if(field)(void)read(field+0x180,field_owner);
    log::info("3663 guest load sample {}/{}: state={} 0x{:02X} marker={} {} own_key=0x{:X} local_user=0x{:X} field_owner=0x{:X} field_is_own={} chain world={} controller={} actor={} sequencer_vt={}",
        load_samples,max_load_samples,state_ok,state,marker_ok,marker,own_key,local_user,field_owner,
        field_owner!=0 && field_owner==local_user,world!=0,controller!=0,actor_ok,sequencer_ok);
}
bool result_code(int* p,int value){if(!p)return false;
    __try{*p=value;return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
void fail(const char* reason){if(!broken.exchange(true))log::error("3663 guest stopped: {}; no local fallback/reconnect",reason);}
std::vector<uint8_t> packet(uintptr_t session,uintptr_t pb){
    uintptr_t frame=0,allocation=0,crypto=0;uint16_t size=0,other=0,capacity=0;uint8_t encrypted=0;
    if(!read(pb+layout.kPb_FramePtr,frame) || frame!=pb || !read(pb+layout.kPb_Base,allocation) || allocation!=pb ||
        !read(pb+layout.kPb_Len2,size) || !read(pb+layout.kPb_Len1,other) || size!=other ||
        !read(pb+layout.kPb_Cap,capacity) || capacity<size || size<layout.kFrameOverhead+troc::kInternalHeaderSize ||
        size-layout.kFrameOverhead>layout.packet_max || !read(session+layout.kSession_Crypto,crypto) ||
        (crypto && (!read(crypto+layout.crypto_enabled_offset,encrypted) || encrypted)))return {};
    std::vector<uint8_t> bytes(size-layout.kFrameOverhead);
    if(!game::safe_read(pb+layout.kFrameHeader,bytes.data(),bytes.size()) ||
        !parse_tunnel_packet(bytes,troc::Dir::ToHostServer,layout))return {};
    return bytes;
}
bool recycle(uintptr_t pb){
    const uintptr_t net=ptr(base+layout.kClientNetSlot),queue=ptr(net+layout.kClientNet_FreeC2S);
    if(!queue || !pb)return false;
    void* item=reinterpret_cast<void*>(pb);
    reinterpret_cast<void(__fastcall*)(void*,void**)>(base+layout.kQueuePush)(reinterpret_cast<void*>(queue),&item);
    return true;
}
int* __fastcall send(void* self,int* result,void* buffer){
    if(!on.load())return send_hook.unsafe_call<int*>(self,result,buffer);
    const uintptr_t session=reinterpret_cast<uintptr_t>(self),pb=reinterpret_cast<uintptr_t>(buffer);
    if(!pb){(void)result_code(result,0);return result;}
    uint64_t current=0,session_epoch=0;
    if(!read(pb+layout.packet_epoch_offset,current) || !read(session+layout.kSyncSession_Epoch,session_epoch))fail("unreadable packet/session epoch");
    if(current==layout.no_epoch)current=session_epoch;
    if(remote.load() && current!=epoch.load()){
        fail("reconnect is unsupported in v1; ending this guest before a second conversation");
        log::flush(2000);TerminateProcess(GetCurrentProcess(),0xCD67);return result;
    }
    if(current!=session_epoch)fail("stale native packet epoch");
    const uint32_t thread=owner_thread.load();
    if(!thread || thread!=GetCurrentThreadId())fail("Send did not run on the verified NetworkThread");
    if(!broken.load()){
        auto bytes=packet(session,pb);
        const auto view=parse_tunnel_packet(bytes,troc::Dir::ToHostServer,layout);
        if(!view)fail("unreadable/encrypted/invalid native frame");
        else{
            if(!remote.load()){
                if(view->opcode!=layout.connect_opcode || epoch.load())fail("missed initial cryptogram or attempted reconnect");
                else{r5_guards3663::mark_remote();epoch.store(current);remote.store(true);}
            }
            if(remote.load() && current!=epoch.load())fail("native connection changed; restart required");
            if(!broken.load()){
                if(blocked_guest_control_3663(view->opcode)){++filtered;}
                else if(!outgoing.push(bytes))fail("outgoing queue full");
                else{
                    ++sent;
                    if(view->opcode==3081)crash3663::arm();
                    crash3663::returned(crash3663::packet(crash3663::Direction::ToHost,
                        view->opcode,static_cast<uint16_t>(bytes.size())));
                    if(view->opcode==2554){refresh_started.store(GetTickCount64());refresh_pending.store(true);}
                }
            }
        }
    }
    // Return each native buffer exactly once, including refused controls.
    // A failed recycle returns an error so native FlushSendQueue can stop.
    const bool returned=recycle(pb);if(!returned)fail("native Q3 recycle unavailable");
    (void)result_code(result,returned?0:1);return result;
}
bool lock3(uintptr_t manager,bool take){
    const uintptr_t lock=ptr(manager);
    if(lock!=manager+8 || ptr(lock)!=base+0x59874C8 ||
        ptr(base+0x59874C8+8)!=base+r4_native::kLock3.rva ||
        ptr(base+0x59874C8+16)!=base+r4_native::kUnlock3.rva)return false;
    reinterpret_cast<void(__fastcall*)(uintptr_t)>(base+(take?r4_native::kLock3.rva:r4_native::kUnlock3.rva))(lock);return true;
}
int busy_byte(uintptr_t manager,bool take){
    if(!lock3(manager,true))return -1;
    uint8_t before=1;const bool readable=read(manager+0x60,before);int result=-1;
    if(readable){
        if(take && before)result=1;
        else{
            __try{*reinterpret_cast<uint8_t*>(manager+0x60)=take?1:0;result=0;}
            __except(EXCEPTION_EXECUTE_HANDLER){result=-1;}
        }
    }
    if(!lock3(manager,false))return -1;return result;
}
int refresh_fault(EXCEPTION_POINTERS* exception){
    broken.store(true);
    return game::game_fault_continue(exception,"3663 guest","native sandbox save-slot refresh");
}
void guarded_refresh(uintptr_t manager){
    __try{reinterpret_cast<void(__fastcall*)(uintptr_t)>(base+r4_v1_native::kRefresh.rva)(manager);}
    __except(refresh_fault(GetExceptionInformation())){return;} // filter passes native faults on
    if(busy_byte(manager,false)!=0)broken.store(true);
}
void title_refresh(){
    if(!refresh_pending.load() || refreshed.load() || broken.load())return;
    if(GetTickCount64()-refresh_started.load()>15000){fail("native title refresh readiness timed out");return;}
    const auto world=ptr(base+0x72451C0),manager=ptr(world+0x88),lookup=ptr(base+0x72456D0);
    if(!world || !manager || !ptr(manager+0x50) || !lookup)return;
    if(ptr(lookup+0x48)!=manager){fail("title/server save-manager identity mismatch");return;}
    const int busy=busy_byte(manager,true);if(busy==1)return;if(busy<0){fail("native save-manager lock/busy guard");return;}
    // Native parsing only; no custom slot-eligibility predicate or title patch.
    // Native faults propagate. Do not try another native lock/call after a
    // fault: its lock/heap state may be damaged. Normal returns release busy.
    guarded_refresh(manager);
    refreshed.store(true);refresh_pending.store(false);
    uint32_t count=0;(void)read(manager+0x48,count);
    log::info("3663 guest: native sandbox title slots refreshed; native entry count {} (not an eligibility claim)",count);
}
void pump(safetyhook::Context& c){
    if(!on.load() || broken.load())return;
    if(c.r15!=ptr(base+layout.kClientNetSlot)){fail("pump ClientNet mismatch");return;}
    uint32_t empty=0;(void)owner_thread.compare_exchange_strong(empty,GetCurrentThreadId());
    if(owner_thread.load()!=GetCurrentThreadId()){fail("second native NetworkThread");return;}
    // R9 A1 exited at the title before LoginCharacter, so the prior
    // after-3081 arm point left no fault site. Arm after startup has settled.
    if(!startup_capture_armed && GetTickCount64()-guest_install_tick>=25000){
        crash3663::arm();startup_capture_armed=true;
        log::info("3663 guest startup crash capture armed before character login");
    }
    title_refresh();
    if(!remote.load() || broken.load())return;
    const uintptr_t session=ptr(c.r15+layout.kClientNet_Session);uint64_t current=0;
    if(!session || !read(session+layout.kSyncSession_Epoch,current) || current!=epoch.load())return;
    // Move a bounded batch out of the leaf queue; never call native code while
    // holding the mod mutex. Packet storage lives through each native copy.
    std::array<Item,256> batch{};size_t count=0;
    {std::lock_guard lock(incoming_mutex);
     while(count<batch.size() && !incoming.empty()){
         incoming_bytes-=incoming.front().packet.size();batch[count++]=std::move(incoming.front());incoming.pop_front();}}
    using Receive=void(__fastcall*)(void*,void*,uint64_t,const uint8_t*,uint16_t);
    for(size_t i=0;i<count && !broken.load();++i){
        if(batch[i].epoch!=current){fail("stale host packet epoch");break;}
        const auto& bytes=batch[i].packet;
        const auto size=static_cast<uint16_t>(bytes.size());
        const auto opcode=static_cast<uint16_t>(bytes[0] | (bytes[1]<<8));
        const auto ticket=crash3663::packet(crash3663::Direction::ToClient,opcode,size);
        reinterpret_cast<Receive>(base+layout.kClientOnRecvPacket)(reinterpret_cast<void*>(c.r15),
            reinterpret_cast<void*>(session),0,bytes.data(),size);
        crash3663::returned(ticket);
        ++delivered;
        if(opcode==game::build_3663::field_layout::kOpMoveFieldAck && field_acks<2) {
            ++field_acks;
            if(field_acks==2) {next_load_sample=0;log::info("3663 guest load sampler armed after second native MoveFieldAck");}
        }
    }
    sample_loading();
}
bool matches(uintptr_t rva,std::span<const uint8_t> expected){
    std::vector<uint8_t> bytes(expected.size());return game::safe_read(base+rva,bytes.data(),bytes.size()) &&
        std::equal(bytes.begin(),bytes.end(),expected.begin());
}
}
bool install(const game::build_guard::ImageIdentity& id){
    if(on.load())return true;
    if(!file_sandbox3663::verified() || !game::build_guard::select_3663_tiers(id,{true,true,true,true},
        {game::build_3663::kTierAComplete,game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete,game::build_3663::kTierDComplete}).tier_d_lifecycle)return false;
    for(const auto& pair:{std::pair{layout.kSyncSessionSend,layout.kSyncSessionSendHead},
        std::pair{layout.kNetLoopPump,layout.kNetLoopPumpAob},std::pair{layout.kClientOnRecvPacket,layout.kClientOnRecvPacketHead},
        std::pair{layout.kQueuePush,layout.kQueuePushHead}})if(!matches(pair.first,pair.second))return false;
    for(const auto& s:{r4_native::kLock3,r4_native::kUnlock3,r4_v1_native::kRefresh})if(!matches(s.rva,s.bytes))return false;
    if(!r5_guards3663::guest_install(id))return false;
    if(!report_guard_install_at(reinterpret_cast<void*>(base+r4_native::kReporter.rva),
        r4_native::kReporter.bytes.data(),r4_native::kReporter.bytes.size()))return false;
    // Startup must capture the very first cryptogram. An already connected
    // client cannot be converted halfway through a local conversation.
    const auto net=ptr(base+layout.kClientNetSlot);
    if(net && ptr(net+layout.kClientNet_Slot4))return false;
    if(!crash3663::install_for_session(role::Role::Guest)){
        log::error("3663 guest: crash and packet diagnostics could not open their session evidence files");
        return false;
    }
    if(!character_guard3663::install(id,role::Role::Guest))return false;
    auto send_result=game::prepare_inline(base+layout.kSyncSessionSend,reinterpret_cast<void*>(&send),
        [](const auto& bytes){return bytes.size()==6 && std::equal(bytes.begin(),bytes.end(),layout.kSyncSessionSendHead.begin());},"3663 guest","native Send entry");
    auto pump_result=game::prepare_mid(base+layout.kNetLoopPump,&pump,
        [](const auto& bytes){return bytes.size()==layout.kNetLoopPumpPatch &&
            std::equal(bytes.begin(),bytes.end(),layout.kNetLoopPumpAob.begin());},"3663 guest","verified native pump");
    if(!send_result || !pump_result)return false;
    send_hook=std::move(*send_result);pump_hook=std::move(*pump_result);
    if(!pump_hook.enable() || !send_hook.enable()){
        if(pump_hook)(void)pump_hook.disable();if(send_hook)(void)send_hook.disable();return false;}
    guest_install_tick=GetTickCount64();
    on.store(true,std::memory_order_release);
    const auto after=ptr(base+layout.kClientNetSlot);
    if(after && ptr(after+layout.kClientNet_Slot4) && !remote.load()){fail("client connected during hook installation");return false;}
    return true;
}
bool active(){return on.load();}
bool failed(){return broken.load();}
void set_host_ready(bool value){
    character_guard3663::set_link_ready(value);
    ready.store(value);
}
void deliver(std::vector<uint8_t>&& bytes){
    if(!on.load() || broken.load() || !remote.load() || !parse_tunnel_packet(bytes,troc::Dir::ToGuestClient,layout)){
        fail("unexpected/malformed host delivery");return;}
    std::lock_guard lock(incoming_mutex);
    if(incoming.size()>=65536 || bytes.size()>(32u<<20)-incoming_bytes){broken.store(true);return;}
    incoming_bytes+=bytes.size();incoming.push_back({epoch.load(),std::move(bytes)});
}
std::deque<std::vector<uint8_t>> take_outgoing(){return ready.load() && !broken.load()?outgoing.take_all():std::deque<std::vector<uint8_t>>{};}
std::string describe(){return std::format("native guest active {} remote {} sent {} delivered {} filtered {} refreshed {} failed {}",
    on.load(),remote.load(),sent.load(),delivered.load(),filtered.load(),refreshed.load(),broken.load())+
    std::format(" | character null-playback skips {}",character_guard3663::skips())+" | "+r5_guards3663::describe();}
}
