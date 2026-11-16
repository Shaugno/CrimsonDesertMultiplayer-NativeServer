#pragma once
#include <windows.h>
#include <type_traits>
namespace cdmp::game {
// A first-chance observation boundary, not native exception recovery. The
// production filter records/seals ownership and returns CONTINUE_SEARCH.
// POD/reference callables avoid C++ unwinding obligations in this SEH frame.
template<class Result,class Call,class Filter>
Result native_call_scope(Call& call,Filter& filter,Result terminal){
    static_assert(std::is_trivially_destructible_v<Call> && std::is_trivially_destructible_v<Filter>);
    __try{return call();}
    __except(filter(GetExceptionInformation())){return terminal;}
}
}
