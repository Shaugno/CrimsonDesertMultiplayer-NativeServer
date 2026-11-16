# Current Step 4 Tier A decision — blocked and unarmed

34/37 contracts are adopted. A026/A027 require the actual target HP-only
layout using adopted native definition roots; the guarded implementation is
prepared but unpublished. A028 now has positive initial-park evidence and
a concrete remaining fast-travel readiness gap. Read R2-LIVE-SESSIONS.md and
the 100800 session result before interpreting earlier pending statements.
No 3663 PlayerLayout is returned and the completeness flag stays false.
The existing 256 guard combinations, wrong-size/unknown-build checks, full
game-closed CTest 101/101 (87.03 s), and loopback 78 states/zero rejected/clean
stop validate this denial state. They do not establish an armed reader or L1.
Self-review leaves lifetime and travel readiness as explicit prerequisites.
No code was changed for this documentation-only decision.

The following sections are chronological implementation and investigation history.

# R2 Tier A — isolated route prepared, arming blocked

Tier A remains **off**. The core contains 37 A contracts: 24 reviewed member,
width and unit contracts; ten candidate global/vtable RVAs await standard
support, and three legacy semantic values remain unproved on 3663. No target
PlayerLayout is published. Its selector returns null and a compile-time
assertion prevents simply flipping kTierAComplete to bypass the missing table.

The worker now routes recognized 3663 directly to a dedicated read-only path
before every legacy optional configuration/installer. A closed permission
logs the identity and exits. Once A is actually complete, that route will
construct the per-build PlayerReader, poll every 50 ms, log state transitions
and a snapshot at most every two seconds, and stop on the worker event. It
does not enter late-init, hooks, overlay, the synthetic source or networking.
Config cannot enable the compiled permission. B–D still need their own
per-build module routing before they can be armed.

The ordinary 2976 path and early guest-role save protections remain in place.
The executable filter and explicit test hosts are preserved. 3576 remains
recognized and unarmed. Shutdown of the unstarted NPC/background helpers
checks their owned thread handles and does not access gameplay addresses.

Validation after the routing change:

- scripts/build.ps1, x64 RelWithDebInfo: **101/101**, **88.70 seconds**.
- Guard test: all 256 request/completeness masks verify both lower-tier
  dependencies and that the isolated A callback never enables legacy reads,
  hooks, overlay, synthetic state or network callbacks. Wrong-size, unknown,
  wrong-base, missing identity facts and recognized 3576 remain denied.
  Legacy 2976 never enters the target-only callback.
- Synthetic host/peer from build/e2e: **78 remote states**, **0 rejected**,
  clean network stop and worker exit. No game process used; this is not L2.
- Self-review: identity precedes every target read; PlayerReader copies remain
  guarded; no new offsets or hook/native-call sites introduced; no asynchronous
  reference to the reader escapes the worker; stop polling is bounded; no wire
  or Steam policy change. No CTest failure or game crash occurred.

Exact remaining A facts:

| Row | Missing fact |
|---|---|
| A001 chunk origin | Standard support for the paired native operand selecting candidate 7231330 |
| A002 static world position | Standard support for native writer 941E20→A61070 selecting 7253E60 |
| A003 manager vtable | Standard data support for candidate 59C2760, with native construction check |
| A004 controlled actor vtable | Standard data support for candidate 5990BF0, with native construction check |
| A005 control vtable | Standard data support for candidate 59C1660, with native construction check |
| A006 status vtable | Standard data support for candidate 5998B60, with native construction check |
| A007 physics vtable | Standard support for constructor 1408ECE0→11B11940 selecting 614A050 |
| A026 health ID | Native HP key is runtime definition data; the old literal zero is not adopted. P1 must compare definition-selected current/max with the visible character HP, then a guarded dynamic selector must replace the target literal |
| A027 stamina ID | Native target identity of stamina key 22 is unproved; today's reader still consumes it, so it stays in scope |
| A028 loading sentinel | No native or live proof of target parked world position (0,1000,0); no silent unchanged assumption. A verified native readiness alternative would require code/proof and the actual L1 loading test |
| A031 WorldSystem root | Standard support for the paired native consumers selecting 72451C0 |
| A032 normal HP definitions | Standard support for HP updater 62FA00→71C4E0 selecting 7245558 |
| A033 alternate HP definitions | Same native updater correspondence, selecting 7245560; active-registry role must remain explicit |

The prepared P1 session only settles the HP-definition question, with all
production tiers closed. It is not L1. L1 remains gated on a complete A table
and then must demonstrate position, rotation, HP and explicit loading not-ready
through load, walk, turn and fast travel, with the user present.

An unpublished reader path now supports a guarded definition-derived HP key.
It requires both complete native registry chains to agree, reads only the
unique matching u16 stat, checks signed current/max bounds and finite units,
then verifies both definition chains and the source container again. Changed,
missing or ambiguous data produces explicit not-ready rather than a literal
zero-ID fallback. It also rechecks the actor/control/physics pointer chain.
This path reads HP only. The legacy layout keeps its literal health/stamina
reader unchanged; no 3663 layout is published and all tier gates remain false.
The conservative requirement for two agreeing registries may deny valid game
states; P1 must establish whether it is usable before a target table selects
it. Missing alternate definitions never silently select the normal registry.
No core row is declared adopted or retired by this preparatory implementation.

The session driver now also supports a separately written `-Profile loading`
question. Its external read-only collector records the candidate position,
chunk origin, rotation axes, static position and full-chain coherence, from
title through load, walking/turning and one fast travel. It labels the old park
position as a hypothesis; it never calls that a production readiness result.
Each profile needs its own pre-launch backup, written question, explicit user
presence, ten-minute collection limit, removal and installation restoration.
The presence request can batch both profiles in one sitting. Neither has run.

Pure tests cover missing actor, wrong physics vtable, nonfinite position,
identity changing during a sample, chunk-origin addition and the park
hypothesis. Windows API self-test uses only the tool's own process. These are
tool checks, not live evidence or permission to arm A.

An investigated readiness alternative was rejected: the controlled-actor
getter's vslot C0 is a reference-acquisition wrapper, not a read-only loading
predicate. Reference `4F8FF0` tail-jumps `1436A90`; target `5D93C0` tail-jumps
`165B590`. They take locks and increment strong references, with changed TLS
state (+1D2→+1D0). Tier A must not call them. Native +4A/+5E eligibility alone
has not been shown to distinguish loading. Paired exports and the file-backed
vtable-slot pointers are retained as negative evidence.

Evidence: port-evidence/r2/tier-a-route-build.txt,
tier-a-route-loopback.txt and tier-a-route-host.txt; hashes are in
port-evidence/r2/text-hashes.json. Read R2-PROGRESS.md for ongoing Step 2 work.

Completed standard-result update (2026-11-16): 34/37 Tier A rows are
adopted. The older pending-address table above is chronological history.
A002 now uses standard-correlated stage update 9418D0/A60C50, decoded
calls at 941A77/A60DE1 to the independently reviewed writers, and the exact
static-position store operands. This is a parent/callee/operand proof, not a
claim that the writer itself matched. Remaining rows are dynamic HP selector
A026, actual target HP-only reachability/stamina retirement A027, and loading
readiness A028. Both definition roots and the other reader addresses now
have standard support. No target PlayerLayout or permission is published.
P1 HP and loading sessions ran; see their actual result.json files. The first
loading capture began after the save loaded, and reported travel timing was
not synchronized, so it cannot establish a loading predicate. A second
user-present passive loading session is being prepared after the DateTime
receipt fix, with collection verified at title before Continue.
