// R4 passive callsite/midpoint evidence. No native calls, argument/register
// changes, packet replay or game-data writes. Storage is bounded and owned.
#include "mod/game/r4_core_probe.h"
#include "mod/game/r4_probe_sites.h"
#include "mod/game/r3_trace_stack.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <windows.h>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <utility>

namespace cdmp::game::r4_core_probe {
namespace {
struct Event {
    uint64_t sequence{}; uint32_t point{},thread{}; bool valid=true;
    std::array<uintptr_t,5> context{};
    std::array<uint64_t,24> data{};
    std::array<uint64_t,16> common{};
};
std::array<safetyhook::MidHook,kSites.size()> hooks;
std::array<std::atomic<uint64_t>,kSites.size()> seen{};
std::array<Event,1024> events;
std::mutex queue_mutex;
size_t head{},tail{};
std::atomic<bool> active{},stopped{};
std::atomic<uint64_t> sequence{},lost{};
uintptr_t base{};
thread_local r3_join_probe::TraceFrames frames;

template<class T> T get(uintptr_t address,Event& event) {
    T value{};
    if(!address || !fast_read(address,&value,sizeof(value)))event.valid=false;
    return value;
}
uintptr_t pointer(uintptr_t address,Event& event){return get<uintptr_t>(address,event);}
void actor(Event& e,uintptr_t a) {
    e.data[0]=a;if(!a)return;
    e.data[1]=pointer(a,e);e.data[2]=get<uint32_t>(a+0x60,e);
    e.data[3]=get<uint32_t>(a+0x90,e);
    const auto kind=e.data[2]>>28;
    if(kind!=0xA && kind!=0xB)return; // diagnostic classification only
    e.data[4]=pointer(a+0xA0,e);
    if(e.data[4])e.data[5]=get<uint32_t>(e.data[4]+0x60,e);
    const auto table=pointer(a+0x68,e);
    const auto ts=table?pointer(table+0x1A0,e):0;
    e.data[6]=ts;
    if(!ts)return;
    e.data[7]=get<uint8_t>(ts+0x3DC,e);
    e.data[8]=get<uint8_t>(ts+0x508+0x48,e);
    e.data[9]=get<uint16_t>(ts+0x508+0x4A,e);
    e.data[10]=pointer(ts+0x508+0x10,e);
    if(e.data[10]) {
        e.data[11]=get<uint32_t>(e.data[10]+0x180,e);
        e.data[12]=get<uint32_t>(e.data[10]+0x2B0,e);
        e.data[13]=get<uint32_t>(e.data[10]+0x2B8,e);
    }
}
void lobby(Event& e,uintptr_t comp) {
    e.data[0]=comp;if(!comp)return;
    e.data[1]=pointer(comp,e);e.data[2]=pointer(comp+8,e);
    if(e.data[2])e.data[3]=get<uint32_t>(e.data[2]+0x60,e);
    e.data[4]=pointer(comp+0x18,e);
    if(e.data[4]) {
        const auto list=pointer(e.data[4],e);
        e.data[5]=get<uint32_t>(e.data[4]+8,e);
        if(e.data[5]>64){e.valid=false;return;}
        if(e.data[5])e.data[6]=get<uint64_t>(list,e);
    }
}
void common(Event& e) {
    // Optional roots can legitimately be absent at title. Every non-null
    // dereference is guarded. These samples make no native lookup/lock call.
    e.common[0]=pointer(base+0x72895E8,e);
    if(e.common[0]) {
        e.common[1]=pointer(e.common[0],e);
        e.common[2]=get<uint32_t>(e.common[0]+0x9C,e);
        e.common[3]=get<uint32_t>(e.common[0]+0xD0,e);
    }
    e.common[4]=pointer(base+0x724B870,e);
    if(e.common[4])e.common[5]=pointer(e.common[4],e);
    if(e.common[5])e.common[6]=pointer(e.common[5],e);
    e.common[7]=pointer(base+0x724B860,e);
    if(e.common[7])e.common[8]=pointer(e.common[7]+0x48,e);
    if(e.common[8])e.common[9]=pointer(e.common[8],e);
    if(e.common[9])e.common[10]=pointer(e.common[9],e);
    if(e.common[10]) {
        e.common[11]=pointer(e.common[10]+8,e);
        e.common[12]=pointer(e.common[10]+0x10,e);
    }
}
void vector(Event& e,uintptr_t v) {
    e.data[0]=v;e.data[1]=pointer(v,e);
    e.data[2]=get<uint32_t>(v+8,e);e.data[3]=get<uint32_t>(v+12,e);
    if(e.data[2]>e.data[3] || e.data[3]>64)e.valid=false;
    const auto field=v-0x2A8;e.data[7]=field;
    e.data[8]=get<uint32_t>(field+0x2D0,e);
    e.data[9]=get<uint32_t>(field+0x180,e);
}
void observe(size_t point,safetyhook::Context& c) noexcept {
    if(!active.load(std::memory_order_acquire) || stopped.load())return;
    Event e;e.point=static_cast<uint32_t>(point);e.thread=GetCurrentThreadId();
    e.context={c.rcx,c.rdx,c.r8,c.r9,c.rsp};
    r3_join_probe::Frame frame;
    if(point==0) {
        const auto caller=get<uintptr_t>(c.rsp,e);
        if(!e.valid || caller!=base+0xE3F1A00)return; // exact native AddActor caller only
        if(seen[point].fetch_add(1)>=32)return;
        frame={{c.rcx,get<uint64_t>(c.rdx,e),get<uint32_t>(c.rdx+8,e),caller,0},c.rsp};
        if(!frames.enter(0,frame)){++lost;stopped.store(true);return;}
        vector(e,c.rcx);e.data[4]=frame.args[1];e.data[5]=frame.args[2];e.data[6]=caller;
    } else if(point==1) {
        const auto found=frames.leave(0,c.rsp+0x38);
        if(!found)return;
        frame=*found;++seen[point];vector(e,frame.args[0]);
        e.data[4]=frame.args[1];e.data[5]=frame.args[2];e.data[6]=frame.args[3];
    } else {
        if(seen[point].fetch_add(1)>=32)return;
        switch(point) {
        case 2:
            actor(e,c.r14);e.data[14]=c.rsi;
            e.data[15]=get<uint32_t>(c.rsi+0x60,e);e.data[16]=get<uint32_t>(c.rax,e);
            e.data[17]=c.rbp-0x20;e.data[18]=pointer(e.data[17],e);
            e.data[19]=pointer(e.data[18]+0x20,e);e.data[20]=get<uint8_t>(e.data[17]+0x10,e);
            e.data[21]=get<uint8_t>(e.data[17]+0x11,e);e.data[22]=pointer(e.data[17]+0x18,e);break;
        case 3: {
            e.data[0]=pointer(base+0x724B870,e);
            if(e.data[0])e.data[1]=pointer(e.data[0],e);
            if(e.data[1]) {
                e.data[2]=pointer(e.data[1],e);e.data[3]=pointer(e.data[1]+0xB8,e);
                e.data[4]=get<uint32_t>(e.data[1]+0xC0,e);
                if(e.data[4]>0x4000)e.valid=false;
            }
            break;
        }
        case 4: {
            const auto table=pointer(c.r15+0x68,e);
            lobby(e,table?pointer(table+0x10,e):0);e.data[10]=c.r15;break;
        }
        case 5:lobby(e,c.rbx);break;
        case 6:lobby(e,c.r15);e.data[7]=c.rax;break;
        case 7:lobby(e,c.rcx);e.data[7]=get<uint64_t>(c.r8,e);e.data[8]=c.r8;break;
        case 8:
            e.data[0]=c.r14;e.data[1]=get<uint32_t>(c.r14+0x60,e);e.data[2]=static_cast<uint32_t>(c.rdx);
            e.data[3]=c.r15;e.data[4]=get<uint8_t>(c.r15,e);e.data[5]=c.r13;
            e.data[6]=get<uint32_t>(c.r13+0x60,e);e.data[7]=pointer(c.r14,e);break;
        case 9:
            e.data[0]=c.r15;e.data[1]=pointer(c.r15,e);e.data[2]=pointer(c.r15+0x88,e);
            if(e.data[2])e.data[3]=get<uint32_t>(e.data[2],e);
            e.data[4]=pointer(c.r15+0x90,e);break;
        case 10:actor(e,pointer(c.rax+8,e));e.data[14]=c.rax;break;
        case 11:
            e.data[0]=c.r15;e.data[1]=pointer(c.r15,e);
            e.data[2]=pointer(e.data[1]+0x50,e);e.data[3]=c.rax;e.data[4]=get<uint32_t>(c.rax,e);
            e.data[5]=pointer(c.r15+8,e);
            if(e.data[5])e.data[6]=get<uint32_t>(e.data[5]+0x60,e);
            if(e.data[1]==base+build_3663::r3_join::kStatusVtable) {
                e.data[7]=pointer(c.r15+0x90,e);
                if(e.data[7])e.data[8]=get<uint64_t>(e.data[7],e);
            }
            break;
        case 12:
            e.data[0]=c.rbp;e.data[1]=get<uint32_t>(c.rbp+0x2D0,e);
            e.data[2]=get<uint32_t>(c.rbp+0x180,e);e.data[3]=get<uint32_t>(c.rbp+0x10,e);
            e.data[4]=c.rcx;e.data[5]=pointer(c.rcx,e);
            e.data[6]=pointer(e.data[5]+8,e);e.data[7]=pointer(e.data[5]+0x10,e);
            e.data[8]=pointer(c.rbp+8,e);
            if(e.data[8])e.data[9]=pointer(e.data[8],e);
            if(e.data[9]) {e.data[10]=pointer(e.data[9]+8,e);e.data[11]=pointer(e.data[9]+0x10,e);}
            break;
        case 13:
            actor(e,pointer(c.rcx+8,e));e.data[14]=c.rcx;
            if(pointer(c.rcx,e)==base+build_3663::r3_join::kStatusVtable) {
                e.data[15]=pointer(c.rcx+0x90,e);
                if(e.data[15])e.data[16]=get<uint64_t>(e.data[15],e);
            }
            break;
        }
    }
    if(seen[point].load()<=2)common(e);
    e.sequence=++sequence;
    std::unique_lock lock(queue_mutex,std::try_to_lock);
    if(!lock.owns_lock() || head-tail==events.size()){++lost;stopped.store(true);return;}
    events[head++%events.size()]=e;
    if(!e.valid)stopped.store(true); // native execution always proceeds unchanged
}
#define R4_POINT(N) void p##N(safetyhook::Context& c){observe(N,c);}
R4_POINT(0) R4_POINT(1) R4_POINT(2) R4_POINT(3) R4_POINT(4) R4_POINT(5) R4_POINT(6)
R4_POINT(7) R4_POINT(8) R4_POINT(9) R4_POINT(10) R4_POINT(11) R4_POINT(12) R4_POINT(13)
#undef R4_POINT
constexpr std::array<safetyhook::MidHookFn,14> callbacks{p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13};
}
bool install(const build_guard::ImageIdentity& id) {
    if(active.load() || build_3663::kTierCComplete || !build_guard::select_3663_tiers(id,{true,false,false,false},
        {build_3663::kTierAComplete,false,false,false}).tier_a_reads)return false;
    base=id.mapped_base;
    for(size_t i=0;i<kSites.size();++i) {
        const auto site=kSites[i];std::array<uint8_t,16> actual{};
        if(!safe_read(base+site.rva,actual.data(),actual.size()) || actual!=site.bytes)return false;
        auto hook=prepare_mid(base+site.rva,callbacks[i],[site](const auto& bytes){return bytes.size()==site.relocated &&
            std::equal(bytes.begin(),bytes.end(),site.bytes.begin());},"R4Core",site.name);
        if(!hook)return false;hooks[i]=std::move(*hook);
    }
    for(auto& hook:hooks)if(!hook.enable()) {
        for(auto& owned:hooks)if(owned)(void)owned.disable();
        return false;
    }
    active.store(true,std::memory_order_release);
    log::info("R4Core: 14 verified passive sites active; 32 records/site; C/D off; no native calls or context/data changes");
    return true;
}
void drain() {
    for(;;) {
        Event e;
        {std::lock_guard lock(queue_mutex);if(head==tail)break;e=events[tail++%events.size()];}
        log::info("R4Core: seq {} point {} {} T{} valid {} context {:X},{:X},{:X},{:X},{:X}",
            e.sequence,e.point,kSites[e.point].name,e.thread,e.valid,e.context[0],e.context[1],e.context[2],e.context[3],e.context[4]);
        for(size_t i=0;i<e.data.size();i+=8)
            log::info("R4Core: seq {} data {} {:X},{:X},{:X},{:X},{:X},{:X},{:X},{:X}",e.sequence,i,
                e.data[i],e.data[i+1],e.data[i+2],e.data[i+3],e.data[i+4],e.data[i+5],e.data[i+6],e.data[i+7]);
        for(size_t i=0;i<e.common.size();i+=8)
            log::info("R4Core: seq {} roots {} {:X},{:X},{:X},{:X},{:X},{:X},{:X},{:X}",e.sequence,i,
                e.common[i],e.common[i+1],e.common[i+2],e.common[i+3],e.common[i+4],e.common[i+5],e.common[i+6],e.common[i+7]);
    }
    if(stopped.load() && active.exchange(false))log::warn("R4Core: collection stopped on guarded-read/depth/drop failure; lost {}",lost.load());
}
}
