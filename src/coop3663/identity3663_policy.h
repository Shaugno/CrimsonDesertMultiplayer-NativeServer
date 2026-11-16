#pragma once
#include <cstdint>
#include <span>

namespace cdmp::coop3663::identity {
inline constexpr uint64_t kNative=0x7FFFFFFFFFFFFFFFULL,kGuest=0x7FFFFFFFFFFFF002ULL;
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
};
inline bool affinity(uint32_t expected,uint32_t actual){return expected && expected==actual;}
inline bool bound(const State& s,uint64_t epoch,uintptr_t user){return !s.failed && epoch && s.epoch==epoch && user && s.user==user;}
inline bool ready(const State& s,uint64_t epoch,uintptr_t user,uintptr_t actor){
    return bound(s,epoch,user) && actor && s.actor==actor && s.lobby_ready && s.status_ready && s.phases_ready;
}
// LoginCharacter payload: u8 discriminator, u64 number, u32 save slot.
// No CrossSave (3) or native one-use slot (206), no trailing ambiguity here.
inline bool login_character(const State& s,uint64_t epoch,uintptr_t user,std::span<const uint8_t> bytes){
    if(!bound(s,epoch,user) || !s.lobby_ready || bytes.size()!=13)return false;
    uint64_t number=0;uint32_t slot=0;
    for(unsigned i=0;i<8;++i)number|=uint64_t(bytes[1+i])<<(8*i);
    for(unsigned i=0;i<4;++i)slot|=uint32_t(bytes[9+i])<<(8*i);
    return number==kGuest && slot!=3 && slot!=206;
}
}
