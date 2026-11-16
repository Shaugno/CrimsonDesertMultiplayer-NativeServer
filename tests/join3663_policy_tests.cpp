#include "coop3663/join3663_policy.h"
#include <array>
#include <cstdio>
#include <type_traits>
int main() {
    using namespace cdmp::coop3663::join;
    static_assert(std::is_invocable_r_v<int*,EnterField3663,void*,int*,void*>);
    static_assert(!std::is_invocable_v<EnterField3663,void*,int*,void*,void*>);
    std::array<RosterEntry,2> roster{{{0,100,2},{1,101,2}}};
    Snapshot s{1,2,100,2,1,10,true,true,true,true,true,true,{roster.data(),1}};
    if(preflight(s)!=Admission::Candidate)return 1;
    s.roster=roster;if(preflight(s)!=Admission::RosterShape)return 2;
    s.roster={roster.data(),1};roster[0].owner_key=3;if(preflight(s)!=Admission::RosterShape)return 3;
    roster[0].owner_key=2;s.primary_owner=3;if(preflight(s)!=Admission::WrongOwner)return 4;
    s.primary_owner=2;s.owner_pinned=false;if(preflight(s)!=Admission::Lifetime)return 5;
    s.owner_pinned=true;s.coherent=false;if(preflight(s)!=Admission::Incoherent)return 6;
    Ticket ticket{1,10,55,2,true}, current=ticket;
    if(after_failure(ticket,current,0)!=Rollback::RemoveOwnUnusedEntry)return 7;
    if(after_failure(ticket,current,1)!=Rollback::Quarantine)return 8;
    ++current.insertion_serial;if(after_failure(ticket,current,0)!=Rollback::Quarantine)return 9;
    current=ticket;++current.field_generation;if(after_failure(ticket,current,0)!=Rollback::Quarantine)return 10;
    current=ticket;++current.epoch;if(after_failure(ticket,current,0)!=Rollback::Quarantine)return 11;
    ticket.inserted=false;if(after_failure(ticket,current,0)!=Rollback::None)return 12;
    std::puts("3663 three-argument ABI, roster/owner/lifetime refusal and insertion-specific rollback policy PASS; no native backend");
    return 0;
}
