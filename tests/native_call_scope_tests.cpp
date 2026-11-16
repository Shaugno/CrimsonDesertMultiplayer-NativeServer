#include "mod/game/native_call_scope.h"
#include <cstdio>
struct State {int calls{},filter{},continued{},caught{};bool sealed{};};
static void exercise(State& s){
    auto call=[&](){++s.calls;RaiseException(0xE0423663,0,0,nullptr);++s.continued;return 1;};
    auto filter=[&](EXCEPTION_POINTERS* e){if(e->ExceptionRecord->ExceptionCode==0xE0423663){++s.filter;s.sealed=true;}return EXCEPTION_CONTINUE_SEARCH;};
    (void)cdmp::game::native_call_scope(call,filter,-1);
}
static void outer(State& s){
    __try{exercise(s);}
    __except(EXCEPTION_EXECUTE_HANDLER){++s.caught;}
}
int main(){
    State s;outer(s);
    if(s.calls!=1 || s.filter!=1 || s.continued || s.caught!=1 || !s.sealed)return 1;
    auto call=[](){return 23;};auto filter=[](EXCEPTION_POINTERS*){return EXCEPTION_EXECUTE_HANDLER;};
    if(cdmp::game::native_call_scope(call,filter,-1)!=23)return 2;
    std::puts("Native call boundary: owned synthetic exception seals then propagates; no cleanup or resumed call; normal return preserved PASS");
}
