#include "coop3663/join3663_transaction.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace cdmp::coop3663::join;
static void check(bool v) { if (!v) std::abort(); }
struct Fake {
    std::array<RosterEntry,1> roster{{{0,0xA0000001,0x90000002}}};
    Snapshot s{0x90000001,0x90000002,0xA0000001,0x90000002,1,2,true,true,true,true,true,true,roster};
    Ticket ticket{1,2,3,0x90000002,true}, current=ticket;
    bool pin_ok=true,lock_ok=true,insert_ok=true,enter_ok=true,partial=false;
    bool locked=false,pinned=false,held=false,row=false,entered=false;
    int releases=0,locks=0,fail_lock=0; uint32_t actors=0;
    std::string order;
    bool pin(){order+='P';pinned=true;return pin_ok;}
    bool lock(){check(!locked);order+='L';++locks;locked=lock_ok && locks!=fail_lock;return locked;}
    void unlock(){check(locked);order+='U';locked=false;}
    Snapshot snapshot(){check(locked && pinned);order+='S';return s;}
    bool insert(){check(locked && pinned);order+='I';row=insert_ok || partial;return insert_ok;}
    bool uncertain(){return partial;}
    bool enter(){check(!locked && pinned && row);order+='E';entered=true;return enter_ok;}
    bool rollback(){check(locked && pinned && entered);order+='B';
        if(after_failure(ticket,current,actors)!=Rollback::RemoveOwnUnusedEntry)return false;
        row=false;return true;}
    void release(){check(!locked && !held);order+='R';pinned=false;++releases;}
    void quarantine(){check(!locked && pinned);order+='Q';held=true;}
};
int main(){
    {Fake f;check(transact(f)==Result::Committed);check(f.order=="PLSIUER" && f.row && f.releases==1);}
    {Fake f;f.pin_ok=false;check(transact(f)==Result::Refused);check(f.order=="PR");}
    {Fake f;f.lock_ok=false;check(transact(f)==Result::Refused);check(f.order=="PLR");}
    {Fake f;f.s.owner_pinned=false;check(transact(f)==Result::Refused);check(f.order=="PLSUR");}
    {Fake f;f.insert_ok=false;check(transact(f)==Result::Refused);check(f.order=="PLSIUR" && !f.row);}
    {Fake f;f.insert_ok=false;f.partial=true;check(transact(f)==Result::Quarantined);check(f.held && f.row && !f.entered);}
    {Fake f;f.enter_ok=false;check(transact(f)==Result::RolledBack);check(f.order=="PLSIUELBUR" && !f.row);}
    {Fake f;f.enter_ok=false;f.fail_lock=2;check(transact(f)==Result::Quarantined);check(f.held && f.releases==0);}
    for(int mismatch=0;mismatch<5;++mismatch){Fake f;f.enter_ok=false;
        if(mismatch==0)++f.current.epoch;
        if(mismatch==1)++f.current.field_generation;
        if(mismatch==2)++f.current.insertion_serial; // includes remove/reinsert ABA
        if(mismatch==3)++f.current.user;
        if(mismatch==4)f.actors=1;
        check(transact(f)==Result::Quarantined);check(f.row && f.held && !f.locked && f.releases==0);
    }
    {Fake f;f.s.roster={};check(transact(f)==Result::Refused);check(!f.entered && f.releases==1);}
    std::puts("3663 transaction: reference/lock ordering, successful admission, every refusal, exact rollback, ABA and partial-write quarantine PASS");
}
