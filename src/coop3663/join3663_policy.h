#pragma once
// Pure guard/rollback policy for a future 3663 three-argument adapter.
// Not linked to the ASI. A successful policy check grants no native permission.
#include <cstdint>
#include <span>

namespace cdmp::coop3663::join {
using EnterField3663 = int*(__fastcall*)(void* mover, int* result, void* field);
struct RosterEntry { uint16_t kind{}; uint32_t actor_key{}, owner_key{}; };
struct Snapshot {
    uint32_t host_owner{}, guest_user{}, primary_key{}, primary_owner{};
    uint64_t epoch{}, field_generation{}; // caller-owned lifetime tokens; no guessed native member
    bool coherent{}, field_open{}, primary_pinned{}, owner_pinned{}, mover_empty{}, mover_idle{};
    std::span<const RosterEntry> roster;
};
enum class Admission { Candidate, Incoherent, Closed, Lifetime, Busy, WrongOwner, RosterShape };
inline Admission preflight(const Snapshot& s) {
    if (!s.coherent || !s.epoch || !s.field_generation) return Admission::Incoherent;
    if (!s.field_open) return Admission::Closed;
    if (!s.primary_pinned || !s.owner_pinned) return Admission::Lifetime;
    if (!s.mover_empty || !s.mover_idle) return Admission::Busy;
    if (!s.host_owner || !s.guest_user || s.host_owner==s.guest_user ||
        !s.primary_key || s.primary_owner!=s.guest_user) return Admission::WrongOwner;
    // Only the measured ordinary primary shape. Companions and factory paths
    // require independent ownership proof; never silently omit other entries.
    if (s.roster.size()!=1 || s.roster[0].kind!=0 || s.roster[0].actor_key!=s.primary_key ||
        s.roster[0].owner_key!=s.guest_user) return Admission::RosterShape;
    return Admission::Candidate;
}
struct Ticket { uint64_t epoch{}, field_generation{}, insertion_serial{}; uint32_t user{}; bool inserted{}; };
enum class Rollback { None, RemoveOwnUnusedEntry, Quarantine };
// Called only with BOTH field locks held by a future native backend. That
// backend must PROVE the ticket still identifies its exact insertion, including
// intervening native mutations. No native serial member is presumed to exist.
inline Rollback after_failure(const Ticket& ticket,const Ticket& current,uint32_t actor_count) {
    if (!ticket.inserted) return Rollback::None;
    if (!ticket.insertion_serial || ticket.epoch!=current.epoch ||
        ticket.field_generation!=current.field_generation || ticket.user!=current.user ||
        ticket.insertion_serial!=current.insertion_serial || !current.inserted || actor_count)
        return Rollback::Quarantine;
    return Rollback::RemoveOwnUnusedEntry;
}
}
