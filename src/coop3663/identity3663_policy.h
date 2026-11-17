#pragma once
#include <cstdint>
#include <span>

namespace cdmp::coop3663::identity {
inline constexpr uint64_t kNative=0x7FFFFFFFFFFFFFFFULL,kGuest=0x7FFFFFFFFFFFF002ULL;
// R6-D1b measured exactly 17 bytes. Exact 3663 vf2 2C6AE10 consumes
// u8, u64, u32 slot, u32 opaque value, then an optional UTF-16 remainder.
// This minimal route adopts only the observed empty-remainder form.
inline constexpr size_t kLoginPayloadSize=17;
enum class Edit { Refuse, Replace, Already };
struct Plan { Edit edit=Edit::Refuse; uint32_t index{}; };
inline Plan lobby(std::span<const uint64_t> numbers){
    uint32_t native=0,assigned=0,index=0;
    if(numbers.empty() || numbers.size()>64)return {};
    for(uint32_t i=0;i<numbers.size();++i){
        if(numbers[i]==kNative){++native;index=i;}
        if(numbers[i]==kGuest){++assigned;if(!native)index=i;}
    }
    if(native==1 && !assigned)return {Edit::Replace,index};
    if(!native && assigned==1)return {Edit::Already,index};
    return {};
}
inline Edit status(uint64_t before){return before==kNative?Edit::Replace:before==kGuest?Edit::Already:Edit::Refuse;}
struct State {
    uint64_t epoch{};uintptr_t user{},actor{};uint32_t thread{};
    bool lobby_ready{},status_ready{},phases_ready{},failed{};
    uint32_t user_key{},actor_key{}; // generation-bearing native keys, not pointer identity alone
};
inline bool affinity(uint32_t expected,uint32_t actual){return expected && expected==actual;}
inline bool bound(const State& s,uint64_t epoch,uintptr_t user){return !s.failed && epoch && s.epoch==epoch && user && s.user==user;}
inline bool ready(const State& s,uint64_t epoch,uintptr_t user,uintptr_t actor,uint32_t user_key,uint32_t actor_key){
    return bound(s,epoch,user) && actor && s.actor==actor && user_key && actor_key &&
        s.user_key==user_key && s.actor_key==actor_key && s.lobby_ready && s.status_ready && s.phases_ready;
}
// LoginCharacter payload: u8 discriminator, u64 number, u32 save slot,
// u32 opaque native field (forwarded unchanged); no text remainder in v1.
// No CrossSave (3) or native one-use slot (206), no trailing ambiguity here.
inline bool login_character(const State& s,uint64_t epoch,uintptr_t user,std::span<const uint8_t> bytes){
    if(!bound(s,epoch,user) || !s.lobby_ready || bytes.size()!=kLoginPayloadSize)return false;
    uint64_t number=0;uint32_t slot=0;
    for(unsigned i=0;i<8;++i)number|=uint64_t(bytes[1+i])<<(8*i);
    for(unsigned i=0;i<4;++i)slot|=uint32_t(bytes[9+i])<<(8*i);
    return number==kGuest && slot!=3 && slot!=206;
}
// The native Play sentinel means this conversation's own character. The
// legacy tunnel already makes this translation (server.cpp); the separate
// 3663 core route previously skipped it. Change only an owned packet copy,
// after validated guest binding, then run the strict admission check again.
inline bool canonicalize_native_login(const State& s,uint64_t epoch,uintptr_t user,uint32_t user_key,
    std::span<uint8_t> bytes){
    if(!bound(s,epoch,user) || !s.lobby_ready || s.user_key!=user_key ||
        bytes.size()!=kLoginPayloadSize || bytes[0]!=4)return false;
    uint64_t number=0;uint32_t slot=0;
    for(unsigned i=0;i<8;++i)number|=uint64_t(bytes[1+i])<<(8*i);
    for(unsigned i=0;i<4;++i)slot|=uint32_t(bytes[9+i])<<(8*i);
    if(number!=kNative || slot==3 || slot==206)return false;
    for(unsigned i=0;i<8;++i)bytes[1+i]=static_cast<uint8_t>(kGuest>>(8*i));
    return true;
}
}
