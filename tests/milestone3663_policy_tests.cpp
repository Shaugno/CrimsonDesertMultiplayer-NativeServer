// R10 Step 2: the guest's own 2941 when it enters the host's field (coop3663/milestone_policy.h).
#include "coop3663/milestone_policy.h"
#include <cstdio>
#include <cstdlib>

namespace m = cdmp::coop3663::milestone;

static int failed = 0;
static void check(bool ok, const char* what) {
    if (!ok) { ++failed; std::printf("FAIL %s\n", what); }
}

constexpr uintptr_t kSession = 0x5000, kField = 0x7000, kHostField = 0x7000, kGuest = 0x9100, kTraveller = 0xA100;
constexpr uint32_t kHostKey = 0x90000001, kGuestKey = 0x90000178, kTravellerKey = 0xA0100178, kTid = 42;

static m::MoveOwner host_owner() { return {true, kHostKey}; }

// Everything the hook reads for an eligible guest entry into the host's field.
static m::Facts eligible() {
    m::Facts f;
    f.tid = kTid;
    f.reads_ok = true;
    f.shared_committed = true;
    f.field = kField;
    f.traveller = kTraveller;
    f.traveller_user = kGuest;
    f.guest_user = kGuest;
    f.main_character = kTraveller;
    f.user_session = kSession;
    f.live_session = kSession;
    f.traveller_field = kField;
    f.host_field = kHostField;
    f.traveller_key = kTravellerKey;
    f.guest_user_key = kGuestKey;
    f.host_user_key = kHostKey;
    f.field_owner = kHostKey;
    f.realm = true;
    f.send_ctx = 0x1234;
    return f;
}

// One trip as the server sends it: 2364, then 2268 and 2792 on the Core's thread.
static m::Ledger trip(uint32_t tid2268 = kTid, uint32_t tid2792 = kTid) {
    m::Ledger l;
    l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
    l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, tid2268, false);
    l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, tid2792, false);
    return l;
}

static void test_emits_once_per_trip() {
    auto l = trip();
    const auto d = l.decide(m::Mode::Send, eligible());
    check(d.emit && d.why == m::Why::Emit && d.tracked && d.gen == 1, "eligible trip emits");
    const auto again = l.decide(m::Mode::Send, eligible());
    check(!again.emit && again.why == m::Why::AlreadyDecided, "second entry of the same trip refused");
    // The next trip is decided afresh.
    l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
    l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, kTid, false);
    l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, kTid, false);
    const auto next = l.decide(m::Mode::Send, eligible());
    check(next.emit && next.gen == 2, "next trip emits again");
}

static void test_off_and_dry_run() {
    auto l = trip();
    check(l.decide(m::Mode::Off, eligible()).why == m::Why::Off, "off decides nothing");
    check(l.trip_active(), "off leaves the trip untouched");
    const auto d = l.decide(m::Mode::DryRun, eligible());
    check(d.emit && d.why == m::Why::Emit, "dry run reports would-emit (the caller does not call)");
}

static void test_not_ours() {
    auto l = trip();
    auto f = eligible();
    f.traveller_user = 0x9200;  // the host's own entry
    check(l.decide(m::Mode::Send, f).why == m::Why::NotGuest, "host entry is not ours");
    f = eligible();
    f.field_owner = kGuestKey;  // the guest's own private field: native branch sends 2941
    check(l.decide(m::Mode::Send, f).why == m::Why::Owner, "guest-owned field left to the native branch");
    check(l.decide(m::Mode::Send, eligible()).emit, "those refusals did not consume the trip");
}

static void test_refusals_finalize_the_trip() {
    struct Case { const char* name; void (*edit)(m::Facts&); m::Why why; };
    const Case cases[] = {
        {"not realm", [](m::Facts& f) { f.realm = false; }, m::Why::NotRealm},
        {"no send context", [](m::Facts& f) { f.send_ctx = 0; }, m::Why::NoSendContext},
        {"scope open", [](m::Facts& f) { f.scope_mode = 1; f.scope_count = 1; }, m::Why::ScopeOpen},
        {"not primary", [](m::Facts& f) { f.traveller_key = 0xB01002C7; }, m::Why::NotPrimary},
        {"not main character", [](m::Facts& f) { f.main_character = 0xA200; }, m::Why::NotMainCharacter},
        {"session mismatch", [](m::Facts& f) { f.user_session = 0x5100; }, m::Why::SessionMismatch},
        {"no live session", [](m::Facts& f) { f.live_session = 0; f.user_session = 0; }, m::Why::SessionMismatch},
        {"field mismatch", [](m::Facts& f) { f.traveller_field = 0x7100; }, m::Why::FieldMismatch},
        {"owner not host", [](m::Facts& f) { f.field_owner = 0x90000099; }, m::Why::OwnerMismatch},
        {"not host field", [](m::Facts& f) { f.host_field = 0x7200; }, m::Why::NotHostField},
    };
    for (const auto& c : cases) {
        auto l = trip();
        auto f = eligible();
        c.edit(f);
        const auto d = l.decide(m::Mode::Send, f);
        check(!d.emit && d.why == c.why && d.tracked, c.name);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::AlreadyDecided, "a refusal is final for its trip");
    }
}

static void test_preconditions_before_a_trip() {
    {
        m::Ledger l;
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::NoTrip, "no 2364: no trip");
    }
    {
        auto l = trip();
        auto f = eligible();
        f.reads_ok = false;
        check(l.decide(m::Mode::Send, f).why == m::Why::Unreadable, "unreadable frame");
        f = eligible();
        f.faulted = true;
        check(l.decide(m::Mode::Send, f).why == m::Why::Faulted, "after a fault");
        f = eligible();
        f.in_take_frame = true;
        check(l.decide(m::Mode::Send, f).why == m::Why::InTakeFrame, "inside take_frame");
        f = eligible();
        f.shared_committed = false;
        check(l.decide(m::Mode::Send, f).why == m::Why::NotShared, "private staging entry is not ours");
        check(l.decide(m::Mode::Send, eligible()).emit, "precondition refusals do not consume the trip");
    }
}

static void test_ordering_and_queue() {
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::No2268, "no 2268");
    }
    {
        auto l = trip(kTid + 1, kTid + 1);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::OtherThread, "2268 on another thread");
    }
    {
        auto l = trip(kTid, kTid + 1);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::No2792, "2792 on another thread");
    }
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
        l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, kTid, false);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::No2792, "no 2792 yet");
    }
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
        l.on_out(m::kOpMoveFieldCompleteAck, kSession, false, kTid, false);
        l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, kTid, false);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::TripPoisoned, "lost 2268 poisons the trip");
    }
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, false, 7, false, host_owner());
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::TripPoisoned, "lost 2364 poisons the trip");
    }
    {
        // 2792 before 2268 (a previous Core) does not count.
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, host_owner());
        l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, kTid, false);
        l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, kTid, false);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::No2792, "2792 must follow 2268");
    }
}

static void test_native_and_ours() {
    {
        auto l = trip();
        l.on_out(m::kOpMilestone, kSession, true, kTid, false);
        check(l.native_seen(), "native 2941 recorded");
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::NativeAlready, "native 2941 already sent");
    }
    {
        auto l = trip();
        check(l.decide(m::Mode::Send, eligible()).emit, "emit");
        l.on_out(m::kOpMilestone, kSession, true, kTid, true);
        check(l.ours_pushed() && !l.native_seen(), "our 2941 is not counted as native");
    }
}

static void test_owner_from_2364() {
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, {});
        l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, kTid, false);
        l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, kTid, false);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::OwnerUnknown, "unreadable 2364 owner");
    }
    {
        m::Ledger l;
        l.on_out(m::kOpMoveFieldAck, kSession, true, 7, false, {true, 0x90000002});
        l.on_out(m::kOpMoveFieldCompleteAck, kSession, true, kTid, false);
        l.on_out(m::kOpMoveFieldCompletePostAck, kSession, true, kTid, false);
        check(l.decide(m::Mode::Send, eligible()).why == m::Why::OwnerMismatch, "2364 owner differs from field");
    }
    const uint8_t payload[13] = {1, 0x78, 0x01, 0x10, 0xA0, 1, 0, 0, 0, 0x01, 0x00, 0x00, 0x90};
    const auto o = m::move_owner(payload, sizeof payload);
    check(o.known && o.owner == 0x90000001, "owner read at +9");
    check(!m::move_owner(payload, 12).known && !m::move_owner(nullptr, 13).known, "short payload has no owner");
}

static void test_new_session_resets() {
    auto l = trip();
    l.on_out(m::kOpMoveFieldCompleteAck, 0x6000, true, kTid, false);  // a different guest session
    check(!l.trip_active(), "a new session drops the old trip");
    check(l.decide(m::Mode::Send, eligible()).why == m::Why::NoTrip, "no carried-over trip");
}

static void test_names() {
    for (unsigned w = 0; w < static_cast<unsigned>(m::Why::Count); ++w)
        check(m::why_name(static_cast<m::Why>(w))[0] != '?', "every reason has a name");
    check(m::mode_from(0) == m::Mode::Off && m::mode_from(1) == m::Mode::DryRun && m::mode_from(2) == m::Mode::Send &&
              m::mode_from(9) == m::Mode::Send, "mode keys");
    check(m::followed(2364) && m::followed(2268) && m::followed(2792) && m::followed(2941) && m::followed(2891) &&
              !m::followed(2005) && !m::followed(2789), "followed opcodes");
}

int main() {
    test_emits_once_per_trip();
    test_off_and_dry_run();
    test_not_ours();
    test_refusals_finalize_the_trip();
    test_preconditions_before_a_trip();
    test_ordering_and_queue();
    test_native_and_ours();
    test_owner_from_2364();
    test_new_session_resets();
    test_names();
    if (failed) { std::printf("%d failure(s)\n", failed); return 1; }
    std::printf("milestone3663 policy: all checks passed\n");
    return 0;
}
