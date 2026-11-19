// R10 Step 2 (docs/probe-3663/port-evidence/r10/step1-preload-owner-path.md).
// Port of the 2976 observer_milestone (P8 S2, docs/research/pendientes/
// p8-2322-observador.md) to build 1.0.0.3732. The game's own ToTarget
// constructor and 2941 sender are called once per guest trip at the rejoin point
// of MoveFieldCompleteCore; no packet is built by hand. Samuel Cardillo's 3576
// guest preloader (ByJunter PR #1, a3904af, GPL v3) addresses the same wait from
// the guest side.
#include "mod/session2/milestone3663.h"
#include "mod/game/hook_prepare.h"
#include "mod/game/safe_memory.h"
#include "mod/log.h"
#include "mod/session2/field_share3663.h"
#include "mod/session2/server.h"
#include "mod/session2/v1_3663.h"

#include <windows.h>
#include <intrin.h>
#include <safetyhook.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <format>
#include <mutex>

namespace cdmp::session2::milestone3663 {
namespace {
namespace m = coop3663::milestone;
constexpr uintptr_t base = game::build_3663::kImageBase;

// 1.0.0.3732 (step1-preload-owner-path.md).
constexpr uintptr_t kOwnerCheck = 0x2ED87D4;   // mov r8d,[r15+0x180]; cmp eax,r8d; jne 0x2ED8849
constexpr uintptr_t kOwnerCall = 0x2ED8844;    // call 0x2B42100 (owner only); rejoin 0x2ED8849; hook 0x2ED8852
constexpr uintptr_t kHook = 0x2ED8852;         // mov rax,[r15+0x2D8]  (field r15, traveller r14)
constexpr uintptr_t kCtor = 0x2D70BE0;         // ToTargetSendTargetFinder(this, actor)
constexpr uintptr_t kEmit = 0x2C7F430;         // PlayAllPreLoadSequencerAck 2941 sender (actor, finder)
constexpr uintptr_t kVtToTarget = 0x5F6D9E0, kVtFinderBase = 0x5F6D9C0;
constexpr uintptr_t kToTargetSlot0 = 0x2D70B80, kToTargetSlot1 = 0x2D70C30, kToTargetSlot2 = 0x2D70C90;
constexpr uintptr_t kBaseSlot0 = 0x2D70AE0;
constexpr uint32_t kTlsRealm = 0x1EB, kTlsSendCtx = 0x250, kCtxScopeCount = 0x2A0, kCtxScopeMode = 0x2AC;
constexpr uint32_t kActorUser = 0xA0, kKey = 0x60, kUserMain = 0xD0, kUserSession = 0x1F8;
constexpr uint32_t kActorComponents = 0x68, kTransformSync = 0x1A0, kTsFieldMover = 0x508, kFmField = 0x10;
constexpr uint32_t kFieldOwner = 0x180;

constexpr std::array<uint8_t, 12> kOwnerBytes{0x45,0x8B,0x87,0x80,0x01,0x00,0x00,0x41,0x3B,0xC0,0x75,0x69};
constexpr std::array<uint8_t, 21> kRejoinBytes{0xE8,0xB7,0x98,0xC6,0xFF,0x48,0x8B,0x4F,0x18,0xE8,0xCE,0x90,0xC1,0xFF,
                                               0x49,0x8B,0x87,0xD8,0x02,0x00,0x00};
constexpr std::array<uint8_t, 7> kHookBytes{0x49,0x8B,0x87,0xD8,0x02,0x00,0x00};
constexpr std::array<uint8_t, 16> kCtorBytes{0x48,0x89,0x4C,0x24,0x08,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B,0xD9,0x48,0x8D,0x05};
constexpr std::array<uint8_t, 32> kEmitBytes{0x48,0x89,0x5C,0x24,0x08,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,
                                             0x48,0x8D,0x6C,0x24,0xD9,0x48,0x81,0xEC,0xB0,0x00,0x00,0x00,0x48,0x8B,0xFA,0x48};
static_assert(kOwnerCall + 14 == kHook);

SafetyHookMid hook;
std::atomic<m::Mode> mode{m::Mode::Off};
std::atomic<bool> installed{}, faulted{};
std::mutex ledger_mutex;  // leaf lock: no native call and no logging while held
m::Ledger ledger;
std::atomic<uint64_t> entries{}, emitted{}, refused{}, not_ours{};
thread_local int t_take_frame = 0;

// Fixed-size notes written on the game's thread, formatted on the mod's
// (drain() from the host bridge's tick): no logging or allocation in the hook.
struct Note {
    uint32_t gen;
    m::Why why;
    uint8_t call;  // 0xFF = not called
    bool dry;
    uintptr_t traveller, field;
    uint32_t key, owner;
    bool realm, ctx;
    uint8_t scope_mode;
    uint32_t scope_count;
};
constexpr size_t kNotes = 32;
std::mutex note_mutex;  // leaf lock
std::array<Note, kNotes> notes{};
size_t note_count = 0, note_drained = 0;
std::atomic<uint64_t> notes_dropped{};
thread_local bool t_emitting = false;

template<class T> T rd(uintptr_t at) noexcept {
    T v{};
    return at && game::safe_read(at, &v, sizeof v) ? v : T{};
}
template<size_t N> bool matches(uintptr_t rva, const std::array<uint8_t, N>& want) {
    std::array<uint8_t, N> got{};
    return game::safe_read(base + rva, got.data(), N) && got == want;
}
uintptr_t field_of(uintptr_t actor) noexcept {
    const auto ts = rd<uintptr_t>(rd<uintptr_t>(actor + kActorComponents) + kTransformSync);
    return ts ? rd<uintptr_t>(ts + kTsFieldMover + kFmField) : 0;
}

m::Facts read_facts(const safetyhook::Context& c) noexcept {
    m::Facts f;
    f.tid = GetCurrentThreadId();
    f.in_take_frame = t_take_frame > 0;
    f.faulted = faulted.load(std::memory_order_acquire) || v1_3663::stopped();
    f.shared_committed = field_share3663::shared_entry_committed();
    f.field = c.r15;
    f.traveller = c.r14;
    uintptr_t guest = 0, host = 0;
    (void)server_user2(guest);
    (void)server_host_user(host);
    f.guest_user = guest;
    f.traveller_user = rd<uintptr_t>(f.traveller + kActorUser);
    f.main_character = rd<uintptr_t>(guest + kUserMain);
    f.user_session = rd<uintptr_t>(guest + kUserSession);
    f.live_session = server_live_session2();
    f.traveller_key = rd<uint32_t>(f.traveller + kKey);
    f.guest_user_key = rd<uint32_t>(guest + kKey);
    f.host_user_key = rd<uint32_t>(host + kKey);
    f.field_owner = rd<uint32_t>(f.field + kFieldOwner);
    f.traveller_field = field_of(f.traveller);
    f.host_field = field_of(rd<uintptr_t>(host + kUserMain));
    const auto tls = rd<uintptr_t>(static_cast<uintptr_t>(__readgsqword(0x58)));
    f.realm = tls && rd<uint8_t>(tls + kTlsRealm) != 0;
    f.send_ctx = tls ? rd<uintptr_t>(tls + kTlsSendCtx) : 0;
    f.scope_mode = f.send_ctx ? rd<uint8_t>(f.send_ctx + kCtxScopeMode) : 0;
    f.scope_count = f.send_ctx ? rd<uint32_t>(f.send_ctx + kCtxScopeCount) : 0;
    f.reads_ok = f.field && f.traveller && guest && host && f.traveller_user && f.field_owner;
    return f;
}

// SEH-only helpers (no C++ objects with destructors in these frames).
bool call_ctor(uintptr_t* finder, uintptr_t actor) noexcept {
    __try {
        reinterpret_cast<void(__fastcall*)(uintptr_t*, uintptr_t)>(base + kCtor)(finder, actor);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
bool call_emit(uintptr_t actor, uintptr_t* finder) noexcept {
    __try {
        reinterpret_cast<void(__fastcall*)(uintptr_t, uintptr_t*)>(base + kEmit)(actor, finder);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

enum class Call : uint8_t { Returned, WrongRecipient, BadFinder, FaultCtor, FaultEmit };
const char* call_name(Call c) {
    switch (c) {
        case Call::Returned: return "returned";
        case Call::WrongRecipient: return "NOT CALLED: the ToTarget session is not the live guest session";
        case Call::BadFinder: return "NOT CALLED: the ToTarget object is not as built natively";
        case Call::FaultCtor: return "FAULT in the ToTarget constructor";
        case Call::FaultEmit: return "FAULT in the 2941 sender";
        default: return "?";
    }
}

Call emit(uintptr_t traveller, uintptr_t live_session) noexcept {
    alignas(16) uintptr_t finder[4]{};
    if (!call_ctor(finder, traveller)) return Call::FaultCtor;
    Call result = Call::Returned;
    if (finder[0] != base + kVtToTarget || static_cast<uint8_t>(finder[1]) != 0) result = Call::BadFinder;
    else if (!live_session || finder[2] != live_session) result = Call::WrongRecipient;
    else {
        t_emitting = true;
        const bool ok = call_emit(traveller, finder);
        t_emitting = false;
        if (!ok) result = Call::FaultEmit;
    }
    finder[0] = base + kVtFinderBase;  // as the native recipe does after the send
    return result;
}

void on_entry(safetyhook::Context& c) noexcept {
    const auto current = mode.load(std::memory_order_acquire);
    if (current == m::Mode::Off) return;
    entries.fetch_add(1, std::memory_order_relaxed);
    const auto facts = read_facts(c);
    m::Decision d;
    {
        std::lock_guard lock(ledger_mutex);
        d = ledger.decide(current, facts);
    }
    if (d.why == m::Why::NotGuest || d.why == m::Why::Owner) {
        not_ours.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    uint8_t call = 0xFF;
    if (d.emit && current == m::Mode::Send) {
        const auto r = emit(facts.traveller, facts.live_session);
        if (r == Call::FaultCtor || r == Call::FaultEmit) faulted.store(true, std::memory_order_release);
        if (r == Call::Returned) emitted.fetch_add(1, std::memory_order_relaxed);
        else refused.fetch_add(1, std::memory_order_relaxed);
        call = static_cast<uint8_t>(r);
    } else if (!d.emit) {
        refused.fetch_add(1, std::memory_order_relaxed);
    }
    std::lock_guard lock(note_mutex);
    if (note_count - note_drained >= kNotes) {
        notes_dropped.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    notes[note_count++ % kNotes] = Note{d.gen, d.why, call, d.emit && current == m::Mode::DryRun, facts.traveller,
                                        facts.field, facts.traveller_key, facts.field_owner, facts.realm,
                                        facts.send_ctx != 0, facts.scope_mode, facts.scope_count};
}
}  // namespace

TakeFrameScope::TakeFrameScope() noexcept { ++t_take_frame; }
TakeFrameScope::~TakeFrameScope() { --t_take_frame; }

void on_out(uint16_t opcode, uintptr_t session, bool pushed, m::MoveOwner owner) noexcept {
    if (mode.load(std::memory_order_relaxed) == m::Mode::Off || !m::followed(opcode)) return;
    std::lock_guard lock(ledger_mutex);
    ledger.on_out(opcode, session, pushed, GetCurrentThreadId(), opcode == m::kOpMilestone && t_emitting, owner);
}

bool install(const game::build_guard::ImageIdentity& identity, m::Mode requested) {
    if (requested == m::Mode::Off) {
        log::info("3663 milestone off (core_milestone=0): the guest gets no 2941 in the host's field");
        return true;
    }
    if (installed.load()) return true;
    if (!game::build_guard::select_3663_tiers(identity, {true, true, true, true},
            {game::build_3663::kTierAComplete, game::build_3663::kTierBComplete, game::build_3663::kTierCComplete,
             game::build_3663::kTierDComplete}).tier_d_lifecycle) {
        log::error("3663 milestone refused: build or tier gate");
        return false;
    }
    const bool code = matches(kOwnerCheck, kOwnerBytes) && matches(kOwnerCall, kRejoinBytes) &&
                      matches(kCtor, kCtorBytes) && matches(kEmit, kEmitBytes);
    const bool vtables = rd<uintptr_t>(base + kVtToTarget) == base + kToTargetSlot0 &&
                         rd<uintptr_t>(base + kVtToTarget + 8) == base + kToTargetSlot1 &&
                         rd<uintptr_t>(base + kVtToTarget + 16) == base + kToTargetSlot2 &&
                         rd<uintptr_t>(base + kVtFinderBase) == base + kBaseSlot0;
    if (!code || !vtables) {
        log::error("3663 milestone refused: code {} vtables {} do not match build 1.0.0.3732", code, vtables);
        return false;
    }
    auto prepared = game::prepare_mid(base + kHook, &on_entry, [](const auto& bytes) {
        return bytes.size() == kHookBytes.size() && std::equal(bytes.begin(), bytes.end(), kHookBytes.begin());
    }, "3663 milestone", "MoveFieldCompleteCore rejoin point");
    if (!prepared) {
        log::error("3663 milestone refused: the hook at 0x2ED8852 could not be prepared");
        return false;
    }
    hook = std::move(*prepared);
    mode.store(requested, std::memory_order_release);
    if (!hook.enable()) {
        mode.store(m::Mode::Off, std::memory_order_release);
        log::error("3663 milestone refused: the hook could not be enabled");
        return false;
    }
    installed.store(true, std::memory_order_release);
    log::info("3663 milestone active ({}): the guest's own 2941 once per trip into the host's field, at 0x2ED8852 "
              "after this trip's 2268 and 2792 on the same thread; game constructor 0x2D70BE0 and sender 0x2C7F430",
              m::mode_name(requested));
    return true;
}

void drain() {
    std::array<Note, kNotes> copy{};
    size_t n = 0;
    {
        std::lock_guard lock(note_mutex);
        while (note_drained < note_count && n < kNotes) copy[n++] = notes[note_drained++ % kNotes];
    }
    bool ours = false, native = false;
    if (n) {
        std::lock_guard lock(ledger_mutex);
        ours = ledger.ours_pushed();
        native = ledger.native_seen();
    }
    for (size_t i = 0; i < n; ++i) {
        const auto& e = copy[i];
        const char* outcome = e.call != 0xFF ? call_name(static_cast<Call>(e.call))
                              : e.dry       ? "dry run: would ask the game for the guest's 2941"
                                            : "";
        try {
            log::info("3663 milestone trip {}: {}{}{} | traveller 0x{:X} key 0x{:08X} field 0x{:X} owner 0x{:08X} | "
                      "realm {} ctx {} scope {}/{} | current trip: our 2941 queued {} native 2941 {}",
                      e.gen, m::why_name(e.why), *outcome ? " | " : "", outcome, e.traveller, e.key, e.field, e.owner,
                      e.realm ? 1 : 0, e.ctx ? 1 : 0, e.scope_mode, e.scope_count, ours ? 1 : 0, native ? 1 : 0);
        } catch (...) {}
    }
    if (const auto lost = notes_dropped.exchange(0)) {
        try { log::info("3663 milestone: {} decision note(s) dropped (buffer full)", lost); } catch (...) {}
    }
}

std::string describe() {
    return std::format("milestone {} entries {} emitted {} refused {} not-ours {} faulted {}",
                       m::mode_name(mode.load()), entries.load(), emitted.load(), refused.load(), not_ours.load(),
                       faulted.load());
}
}  // namespace cdmp::session2::milestone3663
