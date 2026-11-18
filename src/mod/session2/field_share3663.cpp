// Independent 3663 adapter. The original 2976 field_share implementation and
// its four-argument EnterField detour remain untouched.
#include "mod/session2/field_share3663.h"
#include "mod/session2/r4_native_bindings.h"
#include "coop3663/join3663_transaction.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/game/native_call_scope.h"
#include "mod/game/game_fault.h"
#include "mod/log.h"
#include <windows.h>
#include <safetyhook.hpp>
#include <array>
#include <atomic>
#include <cstring>
#include <format>

namespace cdmp::session2::field_share3663 {
namespace {
namespace policy=coop3663::join;
namespace n=r4_native;
constexpr uintptr_t base=game::build_3663::kImageBase;
struct Scope { uintptr_t vt{}, object{}; uint8_t valid{}, flag{}; uint8_t pad[6]{}; uintptr_t holder{}; };
struct Access { Scope scope; uintptr_t sector{}, actor_holder{}; };
struct Row { uintptr_t user{}; uint32_t actors{}, pad{}; bool operator==(const Row&) const=default; };
static_assert(sizeof(Scope)==0x20 && sizeof(Access)==0x30 && sizeof(Row)==16);
struct Pins { Scope host_user, guest_user, host_actor, primary, field; Access access; };
// One guest, one join, no reconnect in this adapter. A quarantine retains all
// owning wrappers until process exit; nothing unloads this code in the game.
Pins quarantine_pins;
std::atomic<bool> on{}, sealed{}, busy{};
std::atomic<uintptr_t> known_guest{};
std::atomic<uint64_t> clock{1}, generations{1}, inserted{}, committed{}, undone{}, refused{};
std::atomic<uint32_t> diagnostic_lines{};
std::atomic<uint32_t> acquire_diagnostic_lines{};
std::atomic<bool> native_acquire_reported{}, native_enter_reported{};
std::atomic<bool> native_entry_committed{};
ReadContext read_context{}; IdentityReady identity_ready{};
NativeFatal native_fatal{};
SafetyHookInline acquire_hook,enter_hook;
SafetyHookMid add_observer,remove_observer;

template<class T> bool read(uintptr_t a,T& v){return a && game::safe_read(a,&v,sizeof v);}
uintptr_t ptr(uintptr_t a){uintptr_t p=0;return read(a,p)?p:0;}
uint32_t key(uintptr_t a){uint32_t k=0;return read(a+0x60,k)?k:0;}
bool write(uintptr_t a,const void* p,size_t size){
    if(!a)return false;
    __try {std::memcpy(reinterpret_cast<void*>(a),p,size);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
template<class T> bool store(uintptr_t a,const T& v){return write(a,&v,sizeof v);}
using Dispose=void(__fastcall*)(Scope*);
void release(Scope& s){
    if(s.valid)reinterpret_cast<Dispose>(base+n::kDispose.rva)(&s);
    s={};
}
void release(Pins& p){
    // Access has an additional mover-access obligation. Calling generic
    // Dispose alone here would leave FM+4A incremented and prevent travel.
    if(p.access.scope.vt)reinterpret_cast<void*(__fastcall*)(Access*,uint32_t)>(base+n::kFieldAccessDtor.rva)(&p.access,0);
    p.access={};
    release(p.field);release(p.primary);release(p.host_actor);release(p.guest_user);release(p.host_user);
}
bool typed_pin(uintptr_t raw,Scope& out){
    const uint32_t wanted=key(raw);const uintptr_t manager=ptr(base+0x7283378);
    if(!raw || !wanted || !manager || ptr(manager)!=base+0x5F69100)return false;
    const bool user=(wanted>>28)==9;
    // FindActor excludes the native user descriptor (type zero). Its sibling
    // FindUser validates that type and returns ScopeAttacher<ServerUserActor>.
    const auto fn=user?n::kFindUser.rva:n::kFindActor.rva;
    const auto result=reinterpret_cast<Scope*(__fastcall*)(uintptr_t,Scope*,uint32_t)>(base+fn)(manager,&out,wanted);
    return result==&out && out.vt==base+(user?0x59C2580:0x5A54310) && out.object==raw && out.valid==1 && !out.flag && !out.holder && key(raw)==wanted;
}
uintptr_t mover(uintptr_t actor){return ptr(ptr(actor+0x68)+0x1A0)+0x508;}
bool field_access(uintptr_t actor,Access& out){
    const uintptr_t fm=mover(actor);
    if(fm==0x508 || ptr(ptr(fm)+8)!=actor)return false;
    const auto result=reinterpret_cast<Access*(__fastcall*)(uintptr_t,Access*)>(base+n::kGetField.rva)(fm,&out);
    return result==&out && out.scope.vt==base+0x5D45B68 && out.scope.valid==1 &&
        !out.scope.flag && !out.scope.holder && out.scope.object && ptr(out.actor_holder+8)==actor;
}
bool field_pin(uintptr_t field,Scope& out){
    if(!field || !reinterpret_cast<bool(__fastcall*)(uintptr_t)>(base+n::kTryAddRef.rva)(field))return false;
    out={base+0x5B40000,field,1,0,{},0};return true;
}
bool context_matches(const Context& c){Context now;return read_context && read_context(now) &&
    now.host==c.host && now.guest==c.guest && now.epoch==c.epoch && c.epoch;}
bool context(Context& c,bool* host_valid=nullptr){const bool valid=read_context && read_context(c) && c.host && c.epoch;
    if(host_valid)*host_valid=valid;const bool ok=valid && c.guest;
    if(ok && c.guest)known_guest.store(c.guest);return ok;}
bool rw_object(uintptr_t obj,uintptr_t vt){return obj && ptr(obj)==base+vt &&
    ptr(base+vt+8)==base+n::kRwLock.rva && ptr(base+vt+0x10)==base+n::kRwUnlock.rva;}
void rw(uintptr_t obj,bool unlock){reinterpret_cast<void(__fastcall*)(uintptr_t)>(base+(unlock?n::kRwUnlock.rva:n::kRwLock.rva))(obj);}
// No failure after the first lock can skip unlocking it. Pointer/type checks
// occur before acquisition; native pinned lifetime keeps these objects alive.
bool lock_field(uintptr_t field,uintptr_t& second){
    second=0;if(!rw_object(field+0x18,0x5B3FFB0))return false;
    rw(field+0x18,false);
    const auto candidate=ptr(field+8); // read second object UNDER the first lock, as native AddActor
    if(!rw_object(candidate,0x5BCEE40)){rw(field+0x18,true);return false;}
    second=candidate;rw(second,false);return true;
}
void unlock_field(uintptr_t field,uintptr_t second){rw(second,true);rw(field+0x18,true);}
bool rows(uintptr_t field,std::array<Row,64>& out,uintptr_t& data,uint32_t& count){
    uint32_t capacity=0;
    return read(field+0x2A8,data) && read(field+0x2B0,count) && read(field+0x2B4,capacity) &&
        count<=out.size() && count<=capacity && capacity<=4096 &&
        (!count || (data && game::safe_read(data,out.data(),count*sizeof(Row))));
}
// Native entries are observed even if their eventual operation fails. This
// deliberately refuses some safe rollbacks rather than miss an ABA mutation.
void mutation(safetyhook::Context&){clock.fetch_add(1,std::memory_order_seq_cst);}
bool native_refusal(uint32_t* result){
    uint32_t error=0;return read(base+0x71D0D3C,error) && error && store(reinterpret_cast<uintptr_t>(result),error);
}
int native_fault(Pins& pins,EXCEPTION_POINTERS* exception){
    sealed.store(true);busy.store(false);
    if(pins.host_user.valid || pins.guest_user.valid || pins.host_actor.valid || pins.primary.valid ||
       pins.field.valid || pins.access.scope.vt){quarantine_pins=pins;pins={};}
    if(native_fatal)native_fatal();
    // No unlock, rollback or further native cleanup after a native fault.
    return game::game_fault_continue(exception,"3663 join","native reference/insertion/EnterField transaction");
}

struct Transaction {
    Context c;uintptr_t fm{},field{},actor{},second{},vector{},initial_vector{};
    uint32_t field_key{},before_count{};uint64_t generation{};
    uint32_t pin_stage{};policy::Admission admission=policy::Admission::Incoherent;
    policy::FieldRoute route=policy::FieldRoute::Refuse;
    uint32_t* result{};uint32_t* returned{};
    Pins pins;std::array<policy::RosterEntry,1> roster{};
    std::array<Row,64> before{};
    policy::Ticket ticket;
    bool partial=false,called=false;
    bool pin(){
        pin_stage=1;
        if(!context_matches(c) || !typed_pin(c.host,pins.host_user) || !typed_pin(c.guest,pins.guest_user))return false;
        pin_stage=2;
        actor=ptr(ptr(fm)+8);
        const uintptr_t host_actor=ptr(c.host+0xD0);
        if(!typed_pin(host_actor,pins.host_actor) || !typed_pin(actor,pins.primary) ||
            !field_access(host_actor,pins.access) || !field_pin(field,pins.field))return false;
        pin_stage=3;
        if(pins.access.scope.object!=field){
            uint32_t owner=0,controller=0;
            const auto guest_key=key(c.guest);
            const bool guest_actor_verified=(key(actor)>>28)==0xA && mover(actor)==fm &&
                ptr(fm-0x508+8)==actor && ptr(actor+0xA0)==c.guest &&
                read(actor+0x90,controller) && controller==guest_key &&
                identity_ready && identity_ready(c.guest,actor,c.epoch);
            if(!read(field+0x180,owner) || !read(field+0x2D0,field_key) ||
                !field_key || !context_matches(c))return false;
            route=policy::enter_route(pins.access.scope.object,field,key(c.host),guest_key,owner,guest_actor_verified);
            return route==policy::FieldRoute::Native;
        }
        route=policy::FieldRoute::Share;
        generation=generations.fetch_add(1);
        return read(field+0x2D0,field_key) && context_matches(c);
    }
    policy::FieldRoute field_route()const{return route;}
    bool lock(){return lock_field(field,second);}
    void unlock(){unlock_field(field,second);second=0;}
    policy::Snapshot snapshot(){
        policy::Snapshot s;
        uint32_t nowkey=0,fmkey=0,count=0;uint16_t fm_busy=1;uint8_t open=0;
        uintptr_t native_roster=0,current_field=~uintptr_t{};uint16_t kind=0;uint32_t rosterkey=0,controller=0;
        s.coherent=context_matches(c) && read(field+0x2D0,nowkey) && nowkey==field_key &&
            ptr(ptr(fm)+8)==actor && mover(actor)==fm && ptr(fm-0x508+8)==actor && ptr(actor+0xA0)==c.guest &&
            read(actor+0x90,controller) && controller==key(c.guest) &&
            pins.access.scope.object==field && read(mover(pins.host_actor.object)+8,fmkey) && fmkey==field_key &&
            ptr(mover(pins.host_actor.object)+0x10)==field && read(field+0x4A,open) &&
            read(fm+0x10,current_field) && read(fm+0x4A,fm_busy) && read(fm+0x68,native_roster) && read(fm+0x70,count) && count==1 &&
            read(native_roster,kind) && read(native_roster+4,rosterkey) &&
            identity_ready && identity_ready(c.guest,actor,c.epoch);
        s.field_open=open==1;s.epoch=c.epoch;s.field_generation=generation;
        s.host_owner=key(c.host);s.guest_user=key(c.guest);s.primary_key=key(actor);
        s.primary_owner=key(ptr(actor+0xA0));s.primary_pinned=pins.primary.valid==1;s.owner_pinned=pins.guest_user.valid==1;
        s.mover_empty=current_field==0;s.mover_idle=fm_busy==0;
        // The native producer, separately traced, sets class A for the primary.
        if((s.primary_key>>28)!=0xA || !read(field+0x180,nowkey) || nowkey!=s.host_owner)s.coherent=false;
        roster[0]={kind,rosterkey,s.primary_owner};s.roster=roster;admission=policy::preflight(s);return s;
    }
    bool insert(){
        if(!rows(field,before,initial_vector,before_count) || before_count>=63)return false;
        for(size_t i=0;i<before_count;++i)if(before[i].user==c.guest)return false;
        const Row row{c.guest,0,0};
        partial=true; // native allocation/growth must never be assumed atomic
        reinterpret_cast<void(__fastcall*)(uintptr_t,const Row*)>(base+n::kVecPush.rva)(field+0x2A8,&row);
        std::array<Row,64> after{};uint32_t count=0;
        if(!rows(field,after,vector,count) || count!=before_count+1 || after[before_count]!=row)return false;
        for(size_t i=0;i<before_count;++i)if(after[i]!=before[i])return false;
        ticket={c.epoch,generation,clock.load(std::memory_order_seq_cst),key(c.guest),true};
        partial=false;++inserted;return true;
    }
    bool uncertain()const{return partial;}
    void enter_native(){
        called=true;
        returned=enter_hook.unsafe_call<uint32_t*>(reinterpret_cast<void*>(fm),result,reinterpret_cast<void*>(field));
    }
    bool enter(){
        enter_native();
        uint32_t code=~0u;
        const bool success=returned==result && read(reinterpret_cast<uintptr_t>(returned),code) && !code;
        if(success)++committed;return success;
    }
    bool rollback(){
        if(!context_matches(c))return false;
        std::array<Row,64> current{};uintptr_t data=0;uint32_t count=0,nowkey=0;uint8_t open=0;
        if(!read(field+0x2D0,nowkey) || nowkey!=field_key || !read(field+0x4A,open) || open!=1 ||
            !rows(field,current,data,count) || data!=vector || count!=before_count+1)return false;
        for(size_t i=0;i<before_count;++i)if(current[i]!=before[i])return false;
        const Row& last=current[before_count];
        const policy::Ticket now{c.epoch,generation,clock.load(std::memory_order_seq_cst),key(c.guest),last.user==c.guest};
        if(last.pad || policy::after_failure(ticket,now,last.actors)!=policy::Rollback::RemoveOwnUnusedEntry)return false;
        // Our insertion was the last row. No swap/compaction or allocator call
        // is needed; retaining spare capacity matches native RemoveActor.
        if(!store(field+0x2B0,before_count))return false;
        ++undone;return true;
    }
    void release(){field_share3663::release(pins);}
    void quarantine(){
        // busy excludes another adapter transaction; sealing precedes publish.
        sealed.store(true,std::memory_order_release);quarantine_pins=pins;pins={};
        log::error("3663 join quarantined: owning references retained; no more guest joins or rollback guesses");
    }
};

uint32_t* __fastcall enter_detour(void* mover_arg,uint32_t* result,void* field_arg){
    Context c;
    const uintptr_t fm=reinterpret_cast<uintptr_t>(mover_arg),field=reinterpret_cast<uintptr_t>(field_arg);
    const uintptr_t actor=ptr(ptr(fm)+8),owner=ptr(actor+0xA0);
    bool host_valid=false;const bool have_context=context(c,&host_valid);
    if(on.load() && host_valid && owner==c.host && actor==ptr(c.host+0xD0))
        return enter_hook.unsafe_call<uint32_t*>(mover_arg,result,field_arg);
    if(on.load() && have_context && actor==ptr(c.guest+0xD0) && owner!=c.guest){
        ++refused;(void)native_refusal(result);return result;
    }
    if(on.load() && !have_context && owner && owner==known_guest.load()){
        ++refused;(void)native_refusal(result);return result;
    }
    if(!on.load() || !have_context || !owner || owner!=c.guest)
        return enter_hook.unsafe_call<uint32_t*>(mover_arg,result,field_arg);
    if(sealed.load() || busy.exchange(true)){++refused;(void)native_refusal(result);return result;}
    Transaction t;t.c=c;t.fm=fm;t.field=field;t.result=result;
    auto operation=[&](){return policy::transact(t);};
    auto fault=[&](EXCEPTION_POINTERS* e){return native_fault(t.pins,e);};
    const auto outcome=game::native_call_scope(operation,fault,policy::Result::Quarantined);
    if(outcome==policy::Result::Native){
        uint32_t code=~0u;
        const bool read_code=t.returned==result && read(reinterpret_cast<uintptr_t>(result),code);
        if(read_code && !code)native_entry_committed.store(true);
        if(!native_enter_reported.exchange(true))try{
            log::info("3663 join Enter native guest-owned field: field key 0x{:X} result 0x{:X} result_read={}",
                t.field_key,code,read_code);
        }catch(...){}
    }
    if(outcome!=policy::Result::Native && diagnostic_lines.fetch_add(1)<32)try{
        uint32_t code=~0u;(void)read(reinterpret_cast<uintptr_t>(result),code);
        log::info("3663 join Enter: outcome {} pin stage {} admission {} original called {} result 0x{:X} field key 0x{:X}",
            static_cast<int>(outcome),t.pin_stage,static_cast<int>(t.admission),t.called,code,t.field_key);
    }catch(...){}
    busy.store(false);
    if(outcome==policy::Result::Refused){++refused;(void)native_refusal(result);return result;}
    if(!t.called){(void)native_refusal(result);return result;}
    return t.returned; // preserve the native return pointer, including a null one
}

struct AcquireTrace {
    const char *host_pin="skipped", *guest_pin="skipped", *primary_pin="skipped", *access="skipped";
    const char *hash_header="skipped", *hash_bucket="skipped", *destination_found="skipped";
    const char *entry_present="skipped", *destination_matches="skipped", *owner_matches="skipped";
    const char *field_open="skipped", *field_key="skipped", *scope_read="skipped";
    const char *scope_vtable="skipped", *scope_slot="skipped", *context="skipped", *field_pin="skipped";
    uint32_t requested_info_key{}, host_field_key{};
    uintptr_t observed_vtable{}, observed_slot{};
    bool requested_key_read{}, host_key_attempted{}, host_key_read{};
};
// Record the original outcome inline so a failed predicate still short-circuits.
bool checked(const char*& result,bool value){
    result=value?"pass":"fail";
    return value;
}
void report_acquire(const AcquireTrace& a,const char* stage,bool shared){
    try{
        log::info("3663 join Acquire predicates: stage={} shared={} host_pin={} guest_pin={} host_primary_pin={} "
            "field_access={} requested_info_key=0x{:X} requested_key_read={} host_field_key=0x{:X} host_key_read={} "
            "hash_header={} hash_bucket={} destination_found={} entry_present={} dest_eq_entry={} "
            "owner_eq_host={} field_open={} field_key={} scope_read={} scope_vtable={} observed_vtable=0x{:X} "
            "scope_slot={} observed_slot=0x{:X} context_match={} field_pin={}",
            stage,shared,a.host_pin,a.guest_pin,a.primary_pin,a.access,a.requested_info_key,a.requested_key_read,
            a.host_field_key,a.host_key_read,a.hash_header,a.hash_bucket,a.destination_found,a.entry_present,
            a.destination_matches,a.owner_matches,a.field_open,a.field_key,a.scope_read,a.scope_vtable,
            a.observed_vtable,a.scope_slot,a.observed_slot,a.context,a.field_pin);
    }catch(...){}
}

uint32_t* __fastcall acquire_detour(void* manager,uint32_t* result,uint8_t type,void* out_arg,void* traveller){
    Context c;const auto out=reinterpret_cast<uintptr_t>(out_arg);
    bool host_valid=false;const bool have_context=context(c,&host_valid);
    if(on.load() && host_valid && reinterpret_cast<uintptr_t>(traveller)==c.host)
        return acquire_hook.unsafe_call<uint32_t*>(manager,result,type,out_arg,traveller);
    if(on.load() && !have_context && traveller && reinterpret_cast<uintptr_t>(traveller)==known_guest.load()){
        if(acquire_diagnostic_lines.fetch_add(1)<32)report_acquire({},"missing_context",false);
        ++refused;(void)native_refusal(result);return result;
    }
    if(!on.load() || !have_context || reinterpret_cast<uintptr_t>(traveller)!=c.guest)
        return acquire_hook.unsafe_call<uint32_t*>(manager,result,type,out_arg,traveller);
    const bool report=acquire_diagnostic_lines.fetch_add(1)<32;
    if(sealed.load() || busy.exchange(true)){
        if(report)report_acquire({},"sealed_or_busy",false);
        ++refused;(void)native_refusal(result);return result;
    }
    AcquireTrace trace;
    Pins p;bool success=false,private_destination=false;
    uint32_t* returned=result;
    auto operation=[&](){
    if(!sealed.load() && checked(trace.host_pin,typed_pin(c.host,p.host_user)) &&
        checked(trace.guest_pin,typed_pin(c.guest,p.guest_user)) &&
        checked(trace.primary_pin,typed_pin(ptr(c.host+0xD0),p.host_actor)) &&
        checked(trace.access,field_access(p.host_actor.object,p.access))){
        const auto field=p.access.scope.object;
        uint32_t owner=0;uint8_t open=0;
        const auto entry=ptr(field+0x2D8);
        // Match the destination's native FieldInfoKey to its level entry using
        // the manager's proved 256-byte hash buckets, with strict bounds.
        uint32_t buckets=0,entries=0,nrows=0;uintptr_t dest=0;
        const auto mgr=reinterpret_cast<uintptr_t>(manager);
        if((trace.requested_key_read=read(out+0x24,trace.requested_info_key)) &&
            checked(trace.hash_header,read(mgr+0x48,buckets) && buckets && buckets<=0x100000 &&
                read(mgr+0x4C,entries) && entries && entries<=0x100000)){
            const uint32_t info=trace.requested_info_key;
            const auto bucket=ptr(reinterpret_cast<uintptr_t>(manager)+0x58)+(info%buckets)*0x100ull;
            if(checked(trace.hash_bucket,read(bucket,nrows) && nrows<=31)){
                for(uint32_t i=0;i<nrows;++i){
                    uint32_t k=0,index=0,hkey=0;
                    if(!read(bucket+8+i*8ull,k) || !read(bucket+12+i*8ull,index) || index>=entries)break;
                    const auto h=ptr(ptr(reinterpret_cast<uintptr_t>(manager)+0x60)+index*8ull);
                    if(k==info && read(h+4,hkey) && hkey==info){dest=ptr(h+8);break;}
                }
                checked(trace.destination_found,dest!=0);
            }
        }
        Scope old;
        const auto read_host_key=[&]{
            trace.host_key_attempted=true;
            return (trace.host_key_read=read(field+0x2D0,trace.host_field_key)) && trace.host_field_key;
        };
        const auto route=policy::acquire_route(entry,dest);
        if(checked(trace.entry_present,entry!=0))checked(trace.destination_matches,route==policy::FieldRoute::Share);
        if(route==policy::FieldRoute::Native)
            private_destination=checked(trace.context,context_matches(c));
        if(route==policy::FieldRoute::Share &&
            checked(trace.owner_matches,read(field+0x180,owner) && owner==key(c.host)) &&
            checked(trace.field_open,read(field+0x4A,open) && open==1) &&
            checked(trace.field_key,read_host_key()) &&
            checked(trace.scope_read,read(out,old)) &&
            checked(trace.scope_vtable,(trace.observed_vtable=old.vt)==base+0x597BFE0) &&
            checked(trace.scope_slot,(trace.observed_slot=ptr(old.vt+0x10))==base+0x1659370) &&
            checked(trace.context,context_matches(c)) && checked(trace.field_pin,field_pin(field,p.field))){
            // The caller already owns the existing output. Dispose its native
            // ownership ON THE ORIGINAL OBJECT, then transfer our retained ref.
            reinterpret_cast<Dispose>(base+n::kDispose.rva)(reinterpret_cast<Scope*>(out));
            const Scope replacement{base+0x597BFE0,field,1,0,{},0};const uint8_t not_new=0;const uint32_t ok=0;
            success=store(out,replacement) && store(out+0x20,not_new) && store(reinterpret_cast<uintptr_t>(result),ok);
            if(success)p.field={};
            else{sealed.store(true);quarantine_pins=p;p={};}
        }
        // Diagnostic snapshot only, using the still-live field access. Preserve
        // "skipped" for the guard when an earlier check short-circuited it.
        if(report && !trace.host_key_attempted && p.access.scope.valid)
            trace.host_key_read=read(field+0x2D0,trace.host_field_key);
    }
    release(p);
    if(private_destination){
        if(!native_acquire_reported.exchange(true))report_acquire(trace,"guest_native_destination",false);
        returned=acquire_hook.unsafe_call<uint32_t*>(manager,result,type,out_arg,traveller);
        return true; // preserve the native return and status, including errors
    }
    if(report)report_acquire(trace,"guest",success);
    return success;
    };
    auto fault=[&](EXCEPTION_POINTERS* e){return native_fault(p,e);};
    success=game::native_call_scope(operation,fault,false);
    busy.store(false);
    if(!success){++refused;(void)native_refusal(result);}
    return success?returned:result;
}
bool matches(const n::Site& site){std::array<uint8_t,16> bytes{};return game::safe_read(base+site.rva,bytes.data(),bytes.size()) && bytes==site.bytes;}
template<class Hook,class Fn> bool prepare(const n::Site& s,Fn fn,Hook& out){
    auto check=[s](const auto& bytes){return bytes.size()==5 && std::equal(bytes.begin(),bytes.end(),s.bytes.begin());};
    if constexpr(std::is_same_v<Hook,SafetyHookInline>){
        auto h=game::prepare_inline(base+s.rva,reinterpret_cast<void*>(fn),check,"3663 join","native entry");
        if(!h)return false;out=std::move(*h);
    }else{
        auto h=game::prepare_mid(base+s.rva,fn,check,"3663 join","native mutation entry");
        if(!h)return false;out=std::move(*h);
    }
    return true;
}
}
bool install(const game::build_guard::ImageIdentity& id,ReadContext context,IdentityReady identity,NativeFatal fatal){
    if(on.load())return true;
    if(!game::build_guard::select_3663_tiers(id,{true,true,true,true},
        {game::build_3663::kTierAComplete,game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete,game::build_3663::kTierDComplete}).tier_c_join || !context || !identity || !fatal)return false;
    for(const auto& s:{n::kAcquire,n::kEnter,n::kTryAddRef,n::kVecPush,n::kFindActor,n::kFindUser,n::kDispose,
        n::kGetField,n::kFieldAccessDtor,n::kRwLock,n::kRwUnlock,n::kAddActor,n::kRemoveActor})if(!matches(s))return false;
    read_context=context;identity_ready=identity;native_fatal=fatal;
    if(!prepare(n::kAddActor,&mutation,add_observer) || !prepare(n::kRemoveActor,&mutation,remove_observer) ||
        !prepare(n::kEnter,&enter_detour,enter_hook) || !prepare(n::kAcquire,&acquire_detour,acquire_hook))return false;
    // Until every hook is enabled, callbacks only run their originals.
    if(!add_observer.enable() || !remove_observer.enable() || !enter_hook.enable() || !acquire_hook.enable()){
        if(add_observer)(void)add_observer.disable();if(remove_observer)(void)remove_observer.disable();
        if(enter_hook)(void)enter_hook.disable();if(acquire_hook)(void)acquire_hook.disable();return false;
    }
    on.store(true,std::memory_order_release);return true;
}
bool active(){return on.load();}
bool quarantined(){return sealed.load();}
bool shared_entry_committed(){return committed.load()!=0;}
bool private_entry_committed(){return native_entry_committed.load();}
std::string describe(){return std::format("3663 join: inserted {} committed {} rolled back {} refused {} quarantine {}",
    inserted.load(),committed.load(),undone.load(),refused.load(),sealed.load());}
}
