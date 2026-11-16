#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
namespace cdmp::game::r3_join_probe {
struct Frame { uintptr_t args[5]{}; uintptr_t entry_rsp{}; };
// One instance per native thread. A stack address pairs the invocation even
// when native exception unwinding bypassed an observed epilogue.
class TraceFrames {
public:
    bool enter(size_t role, const Frame& frame) {
        if (role>=frames_.size()) return false;
        auto& depth=depths_[role];
        while (depth && frames_[role][depth-1].entry_rsp<=frame.entry_rsp) { --depth; ++unwound_; }
        if (depth==frames_[role].size()) return false;
        frames_[role][depth++]=frame;
        return true;
    }
    std::optional<Frame> leave(size_t role, uintptr_t entry_rsp) {
        if (role>=frames_.size()) return std::nullopt;
        auto& depth=depths_[role];
        for(size_t at=depth;at>0;--at) if(frames_[role][at-1].entry_rsp==entry_rsp) {
            const auto frame=frames_[role][at-1];
            unwound_+=depth-at; depth=at-1; return frame;
        }
        return std::nullopt;
    }
    size_t unwound() const { return unwound_; }
private:
    std::array<std::array<Frame,16>,4> frames_{};
    std::array<size_t,4> depths_{};
    size_t unwound_=0;
};
}
