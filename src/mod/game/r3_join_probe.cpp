// Passive R3 candidate entry/exit trace. Never invokes a native function,
// changes an argument/register, resolves an actor through native lookup, or
// writes game data. Hook objects and TLS live until process exit.
#include "mod/game/r3_join_probe.h"
#include "mod/game/r3_join_sites.h"
#include "mod/game/r3_trace_stack.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <windows.h>
#include <atomic>
#include <cstring>
#include <mutex>

namespace cdmp::game::r3_join_probe {
namespace {
struct Roster { uint16_t kind{}; uint32_t key{}; };
struct Event {
    uint64_t sequence{}; uint32_t thread{}, site{};
    Frame frame{}; uintptr_t returned{}, field{}, actor{}, owner{};
    uint32_t result{}, owner_key{}, actor_key{}, field_owner{}, users{}, actors{}, count{};
    bool valid = true;
    std::array<Roster,64> roster{};
};
std::array<safetyhook::MidHook,8> hooks;
std::array<Event,1024> events;
std::mutex event_mutex;
size_t head = 0, tail = 0;
std::atomic<bool> active{false}, stopped{false};
std::atomic<uint64_t> next_sequence{}, dropped{}, unmatched{};
std::atomic<size_t> waiting{};
thread_local TraceFrames frames;
constexpr std::array<uintptr_t,4> exit_stack_adjust{0x98,0x1C8,0x218,0x348};

template<class T> bool read(uintptr_t address, T& out) { return fast_read(address,&out,sizeof(out)); }
void primary(Event& event, uintptr_t mover) {
    const auto holder = fast_ptr(mover);
    const auto actor = holder ? fast_ptr(*holder+8) : std::nullopt;
    if (!actor) { event.valid = false; return; }
    event.actor = *actor;
    const auto owner = fast_ptr(*actor+0xA0); // paired AddActor native owner operand
    if (!owner || !read(*actor+0x60,event.actor_key) || !read(*owner+0x60,event.owner_key)) {
        event.valid = false; return;
    }
    event.owner = *owner;
    if (fast_ptr(mover) != holder || fast_ptr(*holder+8) != actor || fast_ptr(*actor+0xA0) != owner)
        event.valid = false;
}
void field(Event& event) {
    if (!event.field) return;
    if (!read(event.field+0x180,event.field_owner) || !read(event.field+0x2B0,event.users) ||
        !read(event.field+0x2B8,event.actors)) event.valid = false;
}
void observe(size_t site, safetyhook::Context& context) noexcept {
    if (!active.load(std::memory_order_acquire) || stopped.load()) return;
    Event event;
    event.site = static_cast<uint32_t>(site); event.thread = GetCurrentThreadId();
    event.sequence = ++next_sequence;
    const size_t role = site/2;
    if (!(site&1)) {
        event.frame = {{context.rcx,context.rdx,context.r8,context.r9,0},context.rsp};
        if (role == 0 && !read(context.rsp+0x28,event.frame.args[4])) event.valid = false;
        if (!frames.enter(role,event.frame)) { ++dropped; stopped.store(true); return; }
    } else {
        const auto frame = frames.leave(role,context.rsp+exit_stack_adjust[role]);
        if (!frame) { ++unmatched; return; } // native call already in flight, or skipped epilogue
        event.frame = *frame;
        event.returned = context.rax;
        if (!read(event.frame.args[1],event.result)) event.valid = false;
    }
    if (role == 0 && (site&1)) {
        uint8_t valid = 0;
        if (!read(event.frame.args[3]+0x10,valid)) event.valid = false;
        if (valid && !read(event.frame.args[3]+8,event.field)) event.valid = false;
    } else if (role == 1) {
        event.field = event.frame.args[2];
        const uintptr_t mover = event.frame.args[0];
        primary(event,mover);
        uintptr_t vector = 0;
        if (!read(mover+0x68,vector) || !read(mover+0x70,event.count) || event.count>64) {
            event.valid = false;
        } else {
            for (size_t i=0;i<event.count;++i) {
                if (!read(vector+i*0x3C,event.roster[i].kind) || !read(vector+i*0x3C+4,event.roster[i].key))
                    event.valid = false;
            }
            uintptr_t again = 0; uint32_t count = 0;
            if (!read(mover+0x68,again) || !read(mover+0x70,count) || again!=vector || count!=event.count)
                event.valid = false;
        }
    } else if (role == 2) primary(event,event.frame.args[0]);
    field(event);
    std::unique_lock lock(event_mutex,std::try_to_lock);
    if (!lock.owns_lock() || head-tail==events.size()) { ++dropped; stopped.store(true); return; }
    events[head++%events.size()] = event;
    waiting.fetch_add(1,std::memory_order_release);
    if (!event.valid) stopped.store(true); // ordinary native execution always continues
}
void a0(safetyhook::Context& c){observe(0,c);} void a1(safetyhook::Context& c){observe(1,c);}
void b0(safetyhook::Context& c){observe(2,c);} void b1(safetyhook::Context& c){observe(3,c);}
void c0(safetyhook::Context& c){observe(4,c);} void c1(safetyhook::Context& c){observe(5,c);}
void d0(safetyhook::Context& c){observe(6,c);} void d1(safetyhook::Context& c){observe(7,c);}
constexpr std::array<safetyhook::MidHookFn,8> callbacks{a0,a1,b0,b1,c0,c1,d0,d1};
}
bool install(const build_guard::ImageIdentity& identity) {
    if (active.load() || build_3663::kTierCComplete || !build_guard::select_3663_tiers(identity,{true,false,false,false},
        {build_3663::kTierAComplete,false,false,false}).tier_a_reads) return false;
    for (size_t i=0;i<kSites.size();++i) {
        const auto& site = kSites[i]; std::array<uint8_t,16> actual{};
        if (!safe_read(identity.mapped_base+site.rva,actual.data(),actual.size()) || actual!=site.bytes) return false;
        auto hook = prepare_mid(identity.mapped_base+site.rva,callbacks[i],
            [site](const auto& bytes){return bytes.size()==site.relocated &&
                std::equal(bytes.begin(),bytes.end(),site.bytes.begin());},"R3Join","passive candidate entry/exit");
        if (!hook) return false;
        hooks[i] = std::move(*hook);
    }
    for (auto& hook:hooks) if (!hook.enable()) {
        for (auto& owned:hooks) if (owned) (void)owned.disable();
        return false;
    }
    active.store(true,std::memory_order_release);
    log::info("R3Join: four candidate entry/exit pairs active; guarded reads only; C remains off");
    return true;
}
void drain() {
    while (waiting.load(std::memory_order_acquire)) {
        Event event;
        { std::lock_guard lock(event_mutex); if(head==tail)break;
          event=events[tail++%events.size()]; waiting.fetch_sub(1); }
        log::info("R3Join: seq {} site {} thread {} valid {} args {:X},{:X},{:X},{:X},{:X} return {:X} result {:X} field {:X} field_owner {:X} users {} actors {} primary {:X} key {:X} owner {:X} owner_key {:X} roster {}",
            event.sequence,event.site,event.thread,event.valid,event.frame.args[0],event.frame.args[1],event.frame.args[2],
            event.frame.args[3],event.frame.args[4],event.returned,event.result,event.field,event.field_owner,event.users,
            event.actors,event.actor,event.actor_key,event.owner,event.owner_key,event.count);
        for(size_t i=0;i<event.count && i<64;++i)
            log::info("R3Join: seq {} roster {} kind {} key {:X}",event.sequence,i,event.roster[i].kind,event.roster[i].key);
    }
    if (stopped.load() && active.exchange(false)) {
        log::warn("R3Join: collection stopped on a guard/depth/drop condition; dropped {} unmatched {}",dropped.load(),unmatched.load());
    }
}
}
