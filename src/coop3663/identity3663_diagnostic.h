#pragma once
#include "coop3663/identity3663_policy.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <format>
#include <string>

namespace cdmp::coop3663::identity {
// Observation only. This decoder never supplies an admission decision and
// never reads beyond the owned packet span, even for a malformed prefix.
struct LoginDiagnostic {
    size_t length{},prefix_size{};
    std::array<uint8_t,13> prefix{};
    bool discriminator_available{},number_available{},slot_available{};
    uint8_t discriminator{};uint64_t number{};uint32_t slot{};
    bool active{},broken{},context{},bound{},lobby_ready{},size_ok{},number_ok{},slot_ok{},generation_ok{};
    bool state_ok{},epoch_nonzero{},epoch_match{},guest_nonzero{},user_match{};
    uint64_t state_epoch{},current_epoch{},lobby_edits{},status_edits{},phase_edits{};
    uint32_t state_user_key{},current_user_key{};
};
inline LoginDiagnostic diagnose_login(const State& s,uint64_t epoch,uintptr_t user,uint32_t user_key,
    std::span<const uint8_t> bytes,bool active,bool broken,bool context,
    uint64_t lobby_edits,uint64_t status_edits,uint64_t phase_edits){
    LoginDiagnostic d;d.length=bytes.size();d.prefix_size=std::min(bytes.size(),d.prefix.size());
    std::copy_n(bytes.begin(),d.prefix_size,d.prefix.begin());
    d.discriminator_available=bytes.size()>=1;d.number_available=bytes.size()>=9;d.slot_available=bytes.size()>=13;
    if(d.discriminator_available)d.discriminator=bytes[0];
    if(d.number_available)for(unsigned i=0;i<8;++i)d.number|=uint64_t(bytes[1+i])<<(8*i);
    if(d.slot_available)for(unsigned i=0;i<4;++i)d.slot|=uint32_t(bytes[9+i])<<(8*i);
    d.active=active;d.broken=broken;d.context=context;d.bound=identity::bound(s,epoch,user);
    d.lobby_ready=s.lobby_ready;d.size_ok=bytes.size()==kLoginPayloadSize;
    d.number_ok=d.number_available && d.number==kGuest;
    d.slot_ok=d.slot_available && d.slot!=3 && d.slot!=206;
    d.generation_ok=s.user_key==user_key;
    d.state_ok=!s.failed;d.epoch_nonzero=epoch!=0;d.epoch_match=s.epoch==epoch;
    d.guest_nonzero=user!=0;d.user_match=s.user==user;
    d.state_epoch=s.epoch;d.current_epoch=epoch;d.state_user_key=s.user_key;d.current_user_key=user_key;
    d.lobby_edits=lobby_edits;d.status_edits=status_edits;d.phase_edits=phase_edits;
    return d;
}
inline std::string format_login_diagnostic(const LoginDiagnostic& d){
    std::string prefix;for(size_t i=0;i<d.prefix_size;++i)prefix+=std::format("{:02X}",d.prefix[i]);
    return std::format("length={} prefix13={} discriminator={} number={} slot={} | "
        "active={} broken={} context={} bound={} lobby_ready={} size={} number={} slot={} generation={} | "
        "bound_parts state_ok={} epoch_nonzero={} epoch_match={} guest_nonzero={} user_match={} | "
        "lobby_edits={} status_edits={} phase_edits={} state_epoch={} current_epoch={} state_user_key=0x{:08X} current_user_key=0x{:08X}",
        d.length,prefix,d.discriminator_available?std::format("0x{:02X}",d.discriminator):"unavailable",
        d.number_available?std::format("0x{:016X}",d.number):"unavailable",
        d.slot_available?std::to_string(d.slot):"unavailable",
        d.active,d.broken,d.context,d.bound,d.lobby_ready,d.size_ok,d.number_ok,d.slot_ok,d.generation_ok,
        d.state_ok,d.epoch_nonzero,d.epoch_match,d.guest_nonzero,d.user_match,
        d.lobby_edits,d.status_edits,d.phase_edits,d.state_epoch,d.current_epoch,d.state_user_key,d.current_user_key);
}
class RefusalDiagnosticOnce {
    std::atomic_flag logged_=ATOMIC_FLAG_INIT;
public:
    bool claim(){return !logged_.test_and_set(std::memory_order_relaxed);}
};
}
