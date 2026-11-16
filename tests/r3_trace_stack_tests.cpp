#include "mod/game/r3_trace_stack.h"
#include <cstdio>
int main() {
    using namespace cdmp::game::r3_join_probe;
    TraceFrames frames;
    Frame outer{{1,2,3,4,5},0x1000},inner{{6,7,8,9,10},0xF00};
    if(!frames.enter(0,outer)||!frames.enter(0,inner)||!frames.enter(1,inner))return 1;
    if(frames.leave(0,0xE00))return 2; // an unrelated exit cannot borrow a result pointer
    auto result=frames.leave(0,inner.entry_rsp);
    if(!result||result->args[1]!=7||!frames.leave(1,inner.entry_rsp))return 3;
    result=frames.leave(0,outer.entry_rsp);
    if(!result||result->args[1]!=2||frames.leave(0,outer.entry_rsp))return 4;
    if(!frames.enter(0,outer)||!frames.enter(0,inner))return 5;
    result=frames.leave(0,outer.entry_rsp); // native exception skipped inner epilogue
    if(!result||frames.unwound()!=1||frames.leave(0,inner.entry_rsp))return 6;
    if(!frames.enter(0,outer)||!frames.enter(0,outer)||frames.unwound()!=2)return 7;
    result=frames.leave(0,outer.entry_rsp);if(!result)return 8;
    for(uintptr_t i=0;i<16;++i)if(!frames.enter(2,Frame{{},0x1000-i*16}))return 9;
    if(frames.enter(2,Frame{{},0xE00})||frames.enter(4,outer)||frames.leave(4,0x1000))return 10;
    std::puts("Trace pairs recursion, roles, skipped epilogues and bounded depth without borrowed arguments: PASS");
    return 0;
}
