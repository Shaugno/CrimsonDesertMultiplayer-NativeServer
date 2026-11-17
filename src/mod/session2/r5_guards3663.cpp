// Port of Samuel Cardillo's field-leave and achievement guard rules from
// ByJunter PR #1 a3904af3e1a32056dcf7a63bc7a469eb4231084b (GPL v3).
// Credit: Samuel's original behavior/research; this continuation independently
// verifies 3663 vtables, native bodies, midpoint bytes and cleanup ownership.
// See docs/probe-3663/R5-UPSTREAM-REFERENCE.md. No upstream branch merge.
#include "mod/session2/r5_guards3663.h"
#include "coop3663/r5_guard_policy.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include <windows.h>
#include <safetyhook.hpp>
#include <array>
#include <atomic>
#include <format>

namespace cdmp::session2::r5_guards3663 {
namespace {
constexpr uintptr_t base=game::build_3663::kImageBase;
constexpr uintptr_t leave_entry=0x242D4E0,leave_point=0x242D4FE,own_call=0x242D50B;
constexpr uintptr_t sequencer_vt=0x5F70CB0,achievement_vt=0x5BF4138;
constexpr uintptr_t unlock=0x19E9070,achievement_point=0x19E90A3,cleanup=0x19E92CE;
constexpr std::array<uint8_t,31> leave_head{0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x48,0x8B,0x02,0x48,0x8B,0xF9,0x48,0x8B,0xCA,0x48,0x8B,0xDA,0xFF,0x50,0x48,0x48,0x85,0xC0,0x74,0x0D,0x4C};
constexpr std::array<uint8_t,6> leave_bytes{0x4C,0x8B,0x00,0x48,0x8B,0xD3};
constexpr std::array<uint8_t,10> own_bytes{0x48,0x8B,0x4F,0x18,0x48,0x8B,0xD3,0x48,0x8B,0x01};
// Check the entire frame construction, including R15/RSi preservation, rather
// than only the shared 27-byte prefix. The cleanup rel32 is target-specific.
constexpr std::array<uint8_t,51> unlock_head{0x44,0x89,0x4C,0x24,0x20,0x4C,0x89,0x44,0x24,0x18,0x48,0x89,0x54,0x24,0x10,0x55,0x53,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8D,0xAC,0x24,0x58,0xFF,0xFF,0xFF,0x48,0x81,0xEC,0xA8,0x01,0x00,0x00,0x49,0x8B,0xF0,0x4C,0x8B,0xFA,0x4C,0x8B,0xE9};
constexpr std::array<uint8_t,7> achievement_bytes{0x48,0x8B,0x49,0x10,0x49,0x8B,0x10};
constexpr std::array<uint8_t,19> cleanup_bytes{0x49,0x8B,0xCF,0xE8,0xAA,0x21,0x9F,0xFE,0x90,0x48,0x8B,0xCE,0xE8,0x71,0xDC,0x9F,0xFE,0xB0,0x01};
constexpr std::array<uint8_t,20> epilogue{0x48,0x81,0xC4,0xA8,0x01,0x00,0x00,0x41,0x5F,0x41,0x5E,0x41,0x5D,0x41,0x5C,0x5F,0x5E,0x5B,0x5D,0xC3};
SafetyHookMid leave_hook,achievement_hook;
std::atomic<bool> host_on{},guest_on{},remote_seen{};
std::atomic<uint64_t> preserved{},blocked{};
template<class T>T read(uintptr_t at){T v{};return at && game::safe_read(at,&v,sizeof v)?v:T{};}
template<size_t N>bool matches(uintptr_t rva,const std::array<uint8_t,N>& expected){
    std::array<uint8_t,N> actual{};return game::safe_read(base+rva,actual.data(),N) && actual==expected;
}
bool permission(const game::build_guard::ImageIdentity& id){
    return game::build_guard::select_3663_tiers(id,{true,true,true,true},
        {game::build_3663::kTierAComplete,game::build_3663::kTierBComplete,
         game::build_3663::kTierCComplete,game::build_3663::kTierDComplete}).tier_d_lifecycle;
}
void on_leave(safetyhook::Context& c) noexcept {
    if(!host_on.load(std::memory_order_acquire) || read<uintptr_t>(c.rdi)!=base+sequencer_vt)return;
    const auto actor=read<uintptr_t>(c.rdi+8),user=actor?read<uintptr_t>(actor+0xA0):0;
    const auto leaver=user?read<uint32_t>(user+0x60):0,owner=read<uint32_t>(c.rbx+0x180);
    // Restrict the upstream rule to verified server user characters.
    // Guarded reads only: no native calls, locks, allocations or logs here.
    // Do not demand user+primary/controller links during native destruction:
    // their clearing order at logout is not established by a static layout.
    if(!actor || !user || (read<uint32_t>(actor+0x60)>>28)!=0xA || (leaver>>28)!=9 || (owner>>28)!=9)return;
    if(coop3663::r5_guard::preserve_owner(c,read<uintptr_t>(c.rdi+0x18),leaver,owner,base+own_call))++preserved;
}
void on_achievement(safetyhook::Context& c) noexcept {
    if(!guest_on.load(std::memory_order_acquire) || read<uintptr_t>(c.r13)!=base+achievement_vt)return;
    if(coop3663::r5_guard::skip_achievement(c,remote_seen.load(std::memory_order_acquire),base+cleanup))++blocked;
}
}
bool host_install(const game::build_guard::ImageIdentity& id){
    if(host_on.load())return true;
    if(!permission(id) || read<uintptr_t>(base+sequencer_vt+34*8)!=base+leave_entry ||
        !matches(leave_entry,leave_head) || !matches(leave_point,leave_bytes) || !matches(own_call,own_bytes))return false;
    auto hook=game::prepare_mid(base+leave_point,&on_leave,[](const auto& b){return b.size()==leave_bytes.size() &&
        std::equal(b.begin(),b.end(),leave_bytes.begin());},"3663 R5","sequencer owner-unload guard");
    if(!hook)return false;leave_hook=std::move(*hook);
    host_on.store(true,std::memory_order_release);
    if(!leave_hook.enable()){host_on.store(false);return false;}
    log::info("3663 R5 field-leave guard active: Samuel Cardillo rule; verified native own-manager continuation");return true;
}
bool guest_install(const game::build_guard::ImageIdentity& id){
    if(guest_on.load())return true;
    if(!permission(id) || read<uintptr_t>(base+achievement_vt+2*8)!=base+unlock ||
        !matches(unlock,unlock_head) || !matches(achievement_point,achievement_bytes) ||
        !matches(cleanup,cleanup_bytes) || !matches(0x19E931D,epilogue))return false;
    auto hook=game::prepare_mid(base+achievement_point,&on_achievement,[](const auto& b){return b.size()==achievement_bytes.size() &&
        std::equal(b.begin(),b.end(),achievement_bytes.begin());},"3663 R5","Steam achievement native-cleanup guard");
    if(!hook)return false;achievement_hook=std::move(*hook);
    guest_on.store(true,std::memory_order_release);
    if(!achievement_hook.enable()){guest_on.store(false);return false;}
    log::info("3663 R5 achievement guard active: Samuel Cardillo rule; sticky after native remote cryptogram; original argument cleanup");return true;
}
void mark_remote() noexcept {remote_seen.store(true,std::memory_order_release);}
std::string describe(){return std::format("R5 guards host {} guest {} remote_seen {} owner unloads preserved {} achievements blocked {}",
    host_on.load(),guest_on.load(),remote_seen.load(),preserved.load(),blocked.load());}
}
