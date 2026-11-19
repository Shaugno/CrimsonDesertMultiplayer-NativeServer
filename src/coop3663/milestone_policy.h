#pragma once
// R10 Step 2 (docs/CLAUDE-3663-PLAN-R10.md, docs/probe-3663/port-evidence/r10/
// step1-preload-owner-path.md): the guest's own PlayAllPreLoadSequencerAck 2941
// when it enters the HOST's field. Pure logic, no game access, for the tests.
//
// Build 1.0.0.3732. MoveFieldCompleteCore 0x2ED78B0 sends 2941 only on its
// field-owner branch (jne 0x2ED87DE). A guest moving into the host-owned field
// receives MoveFieldCompleteAck 2268 (sent at 0x2ED751D in the handler, before
// the Core) and MoveFieldCompletePostAck 2792 (0x2ED83D0, inside the Core) but no
// 2941, so its loading stays at state 0x11 and it never sends LoadingCompleteReq
// 2789. This is the 2976 observer_milestone case (opcodes 2144/3066/2322 there,
// docs/research/pendientes/p8-2322-observador.md), ported with its guards.
//
// A direct send runs session 2's Send (the mod's take_frame) synchronously on the
// sending thread, so this trip's 2268 and 2792 have gone through take_frame on
// THIS thread before the hook at 0x2ED8852 runs, and the 2941 the hook asks the
// game to send goes through it during the call. The ledger binds the decision to
// those real pushes. One trip = one 2364 MoveFieldAck pushed to the live guest
// session. At most one decision per trip, final: emit once or refuse with a
// reason. No retry, no timer.

#include <cstdint>

namespace cdmp::coop3663::milestone {

inline constexpr uint16_t kOpMoveFieldAck = 2364;
inline constexpr uint16_t kOpMoveFieldCompleteAck = 2268;
inline constexpr uint16_t kOpMoveFieldCompletePostAck = 2792;
inline constexpr uint16_t kOpMilestone = 2941;
inline constexpr uint16_t kOpLoadingCompleteAck = 2891;

constexpr bool followed(uint16_t op) {
    return op == kOpMoveFieldAck || op == kOpMoveFieldCompleteAck || op == kOpMoveFieldCompletePostAck ||
           op == kOpMilestone || op == kOpLoadingCompleteAck;
}

// [session2] core_milestone = 0 (off, default) | 1 (decide and log) | 2 (send).
enum class Mode : uint8_t { Off, DryRun, Send };
constexpr Mode mode_from(unsigned v) { return v >= 2 ? Mode::Send : v == 1 ? Mode::DryRun : Mode::Off; }
inline const char* mode_name(Mode m) {
    return m == Mode::Send ? "send" : m == Mode::DryRun ? "dry run (decide and log only)" : "off";
}

enum class Why : uint8_t {
    Emit = 0,
    Off,
    NotGuest,          // the traveller is not the guest's character (host entries): not ours
    Owner,             // the guest owns this field: the native owner branch sends its 2941
    Unreadable,        // the native frame or the mod's view could not be read
    Faulted,           // an earlier native call faulted: off for the rest of the run
    InTakeFrame,       // reached from inside take_frame
    NotShared,         // the shared Enter into the host's field is not committed
    NoTrip,            // no 2364 reached the live guest session
    AlreadyDecided,    // this trip already had its decision
    TripPoisoned,      // this trip's 2364, 2268 or 2792 was lost in the outgoing queue
    NativeAlready,     // a native 2941 already reached the guest in this trip
    No2268,            // no 2268 of this trip reached the guest
    OtherThread,       // the trip's 2268 went out on another thread
    No2792,            // no 2792 after that 2268 on this thread
    NotRealm,          // TLS +0x1EB is 0: not a server-realm thread
    NoSendContext,     // TLS +0x250 is null
    ScopeOpen,         // a native scope sender is open (+0x2AC mode with +0x2A0 senders)
    NotPrimary,        // the traveller's key is not a primary character key (0xA...)
    NotMainCharacter,  // [guest user +0xD0] is not the traveller
    SessionMismatch,   // the guest user's session (+0x1F8) is not the live guest session
    FieldMismatch,     // the traveller's field mover (TS +0x508 -> +0x10) is another field
    OwnerUnknown,      // the trip's 2364 had no readable owner
    OwnerMismatch,     // the field owner is not the 2364's or not the host user
    NotHostField,      // the field is not the host's current field
    Count
};

inline const char* why_name(Why w) {
    switch (w) {
        case Why::Emit: return "eligible";
        case Why::Off: return "off";
        case Why::NotGuest: return "not the guest";
        case Why::Owner: return "the guest owns this field: the native branch sends 2941";
        case Why::Unreadable: return "the native frame or the mod's view is unreadable";
        case Why::Faulted: return "an earlier call faulted: off for this run";
        case Why::InTakeFrame: return "reached from inside take_frame";
        case Why::NotShared: return "the shared Enter is not committed";
        case Why::NoTrip: return "no 2364 reached the live guest session";
        case Why::AlreadyDecided: return "this trip was already decided";
        case Why::TripPoisoned: return "a 2364/2268/2792 of this trip was lost in the outgoing queue";
        case Why::NativeAlready: return "a native 2941 already reached the guest in this trip";
        case Why::No2268: return "no 2268 of this trip reached the guest";
        case Why::OtherThread: return "the trip's 2268 went out on another thread";
        case Why::No2792: return "no 2792 after that 2268 on this thread";
        case Why::NotRealm: return "not a server-realm thread (TLS +0x1EB = 0)";
        case Why::NoSendContext: return "no send context (TLS +0x250 = 0)";
        case Why::ScopeOpen: return "a native scope sender is open on this thread";
        case Why::NotPrimary: return "the traveller is not a primary character";
        case Why::NotMainCharacter: return "the traveller is not the guest user's main character";
        case Why::SessionMismatch: return "the guest user's session is not the live guest session";
        case Why::FieldMismatch: return "the traveller's field mover points to another field";
        case Why::OwnerUnknown: return "the trip's 2364 had no readable owner";
        case Why::OwnerMismatch: return "the field owner is not the 2364's or not the host";
        case Why::NotHostField: return "the field is not the host's current field";
        default: return "?";
    }
}

// The 2364 MoveFieldAck payload: u8 discriminator, u32 actor, u32 move type, u32
// owner at +9 (the layout v1_3663_policy aliases). The SERVER's owner is read
// before the guest copy is rewritten.
struct MoveOwner { bool known = false; uint32_t owner = 0; };
inline MoveOwner move_owner(const uint8_t* payload, unsigned size) {
    if (!payload || size < 13) return {};
    return {true, static_cast<uint32_t>(payload[9]) | (static_cast<uint32_t>(payload[10]) << 8) |
                      (static_cast<uint32_t>(payload[11]) << 16) | (static_cast<uint32_t>(payload[12]) << 24)};
}

// What the hook read (milestone3663.cpp fills it with guarded reads; tests by hand).
struct Facts {
    uint32_t tid = 0;
    bool reads_ok = false;
    bool faulted = false;
    bool in_take_frame = false;
    bool shared_committed = false;
    uintptr_t field = 0, traveller = 0;          // r15, r14 at 0x2ED8852
    uintptr_t traveller_user = 0, guest_user = 0, main_character = 0;
    uintptr_t user_session = 0, live_session = 0;
    uintptr_t traveller_field = 0, host_field = 0;
    uint32_t traveller_key = 0, guest_user_key = 0, host_user_key = 0, field_owner = 0;
    bool realm = false;
    uintptr_t send_ctx = 0;
    uint8_t scope_mode = 0;
    uint32_t scope_count = 0;
};

struct Decision {
    Why why = Why::Off;
    bool emit = false;     // DryRun: logged as "would emit"; Send: the caller asks the game once
    bool tracked = false;  // the decision belongs to (and finalized) a trip
    uint32_t gen = 0;
};

class Ledger {
public:
    // A followed opcode left take_frame for `session` (the live guest session).
    // `ours` = the 2941 the hook itself asked for (same thread, during the call).
    void on_out(uint16_t op, uintptr_t session, bool pushed, uint32_t tid, bool ours, MoveOwner owner = {}) {
        if (!followed(op)) return;
        if (session != session_) {   // a new guest session: nothing carries over
            *this = Ledger{};
            session_ = session;
        }
        if (op == kOpMoveFieldAck) {
            trip_ = Trip{};
            trip_.active = true;
            trip_.gen = ++gen_;
            trip_.owner = owner;
            trip_.poisoned = !pushed;
            return;
        }
        if (!trip_.active) return;
        if (op == kOpMoveFieldCompleteAck) {
            if (!pushed) { trip_.poisoned = true; return; }
            trip_.have2268 = true;
            trip_.tid2268 = tid;
            trip_.have2792 = false;
        } else if (op == kOpMoveFieldCompletePostAck) {
            if (!pushed) { trip_.poisoned = true; return; }
            if (trip_.have2268 && tid == trip_.tid2268) trip_.have2792 = true;
        } else if (op == kOpMilestone) {
            if (ours) trip_.ours_pushed = pushed;
            else if (pushed) trip_.native2941 = true;
        }
    }

    // Called once per Core entry at the rejoin point; finalizes the trip.
    Decision decide(Mode mode, const Facts& f) {
        Decision d;
        if (mode == Mode::Off) return d;
        if (!f.guest_user || f.traveller_user != f.guest_user) { d.why = Why::NotGuest; return d; }
        if (f.guest_user_key && f.field_owner == f.guest_user_key) { d.why = Why::Owner; return d; }
        if (!f.reads_ok) { d.why = Why::Unreadable; return d; }
        if (f.faulted) { d.why = Why::Faulted; return d; }
        if (f.in_take_frame) { d.why = Why::InTakeFrame; return d; }
        if (!f.shared_committed) { d.why = Why::NotShared; return d; }
        if (!trip_.active) { d.why = Why::NoTrip; return d; }
        d.tracked = true;
        d.gen = trip_.gen;
        if (trip_.decided) { d.why = Why::AlreadyDecided; return d; }
        trip_.decided = true;
        d.why = check(f);
        d.emit = d.why == Why::Emit;
        return d;
    }

    bool trip_active() const { return trip_.active; }
    uint32_t trip_gen() const { return trip_.gen; }
    bool ours_pushed() const { return trip_.ours_pushed; }
    bool native_seen() const { return trip_.native2941; }

private:
    struct Trip {
        bool active = false, poisoned = false, decided = false;
        bool have2268 = false, have2792 = false, native2941 = false, ours_pushed = false;
        uint32_t gen = 0, tid2268 = 0;
        MoveOwner owner;
    };

    Why check(const Facts& f) const {
        if (trip_.poisoned) return Why::TripPoisoned;
        if (trip_.native2941) return Why::NativeAlready;
        if (!trip_.have2268) return Why::No2268;
        if (trip_.tid2268 != f.tid) return Why::OtherThread;
        if (!trip_.have2792) return Why::No2792;
        if (!f.realm) return Why::NotRealm;
        if (!f.send_ctx) return Why::NoSendContext;
        if (f.scope_mode && f.scope_count) return Why::ScopeOpen;
        if ((f.traveller_key >> 28) != 0xA) return Why::NotPrimary;
        if (f.main_character != f.traveller) return Why::NotMainCharacter;
        if (!f.live_session || f.user_session != f.live_session) return Why::SessionMismatch;
        if (!f.field || f.traveller_field != f.field) return Why::FieldMismatch;
        if (!trip_.owner.known) return Why::OwnerUnknown;
        if (!f.host_user_key || trip_.owner.owner != f.field_owner || f.field_owner != f.host_user_key)
            return Why::OwnerMismatch;
        if (!f.host_field || f.host_field != f.field) return Why::NotHostField;
        return Why::Emit;
    }

    uintptr_t session_ = 0;
    uint32_t gen_ = 0;
    Trip trip_;
};

}  // namespace cdmp::coop3663::milestone
