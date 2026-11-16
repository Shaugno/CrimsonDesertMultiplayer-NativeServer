#pragma once
#include "coop3663/join3663_policy.h"

namespace cdmp::coop3663::join {
enum class Result { Refused, Committed, RolledBack, Quarantined };
// Backend owns all references and the exact insertion ticket. Its snapshot and
// rollback methods run under BOTH native field locks. Native Enter runs under
// neither. No registry mutex may be held across any native call.
template<class Backend> Result transact(Backend& b) {
    if (!b.pin()) { b.release(); return Result::Refused; }
    if (!b.lock()) { b.release(); return Result::Refused; }
    if (preflight(b.snapshot()) != Admission::Candidate) {
        b.unlock(); b.release(); return Result::Refused;
    }
    const bool inserted = b.insert();
    b.unlock();
    if (!inserted) {
        // insert() must distinguish a known no-op from uncertain partial
        // growth/write. An uncertain operation keeps its owning references.
        if (b.uncertain()) { b.quarantine(); return Result::Quarantined; }
        b.release(); return Result::Refused;
    }
    if (b.enter()) { b.release(); return Result::Committed; }
    if (!b.lock()) { b.quarantine(); return Result::Quarantined; }
    const bool undone = b.rollback();
    b.unlock();
    if (!undone) { b.quarantine(); return Result::Quarantined; }
    b.release(); return Result::RolledBack;
}
} // namespace cdmp::coop3663::join
