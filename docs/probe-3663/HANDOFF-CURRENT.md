# Phase 9 — blocker package for Claude's final review

**The native two-player 3663 goal was not reached.** This is the plan's blocker
handoff, with all independent completed work preserved and pushed.
No live two-player, native spawn/movement, disconnect/removal or save-isolation
result is claimed. The 3663 gameplay gate remains unarmed.

Branch: `gpt/3663-port` at
https://github.com/Shaugno/CrimsonDesertMultiplayer-NativeServer/tree/gpt/3663-port.
Base: `a8656e87a0c43db400bdf863193c1afb353d6de0` (`origin/grok/compat-guard`).
Final package commit: the Phase 9 commit containing this section. Its exact
full SHA and the full commit list including Phase 9 are written **after the
commit/push** into `PHASE9-MANIFEST.json` inside the local review ZIP in
`dist/phase9/`. `tools/port3663/package_phase9.py` reproduces the package directly
from committed Git blobs, not working-tree files. The final chat also names
the final SHA. No release or binary publication was performed.

## Completion boundary

| Phase | Actual result |
|---|---|
| 0 | Complete: all five review fixes, regressions and full suites pass |
| 1 | Complete required references: 2976 downloaded/verified; optional 3576 midpoint unavailable |
| 2 | Partial: reproducible conservative inventory; semantic/minimal-reachability audit remains |
| 3 | Partial: 17 corroborated values; complete core map unresolved |
| 4 | Partial: separate static header and strict recognition; native modules unarmed/unrouted |
| 5 | Complete game-free validation of this guarded checkpoint, not of a functional port |
| 6 | Blocked before any live session |
| 7 | Blocked before any two-instance session |
| 8 | Blocked before any two-PC Steam session or guest kit |
| 9 | Blocker review package |

Contiguous implementation completion ends at Phase 1. Independent work extends
through the Phase 5 validation. The Phase 6–8 commits record blocked status;
their existence is not evidence that live acceptance passed.

## Build, identities and hashes

x64 RelWithDebInfo, VS 18 Community, MSVC toolset 14.52.37025 / compiler
19.52.37029.0, Windows SDK 10.0.26100.0, C++20 and static CRT. The tested ASI
contains clean source build ID **3C3EC37B** (source commit
3c3ec37b713d7e676cd6bb898b21e32e92075699). Later commits are analysis/reporting
changes. Binary SHA-256 values and sizes: `port-evidence/phase5-binary-hashes.json`.
ASI hash: `5E2FC9BEF6D8802150936FC381F23AE89C061AB53A78F5E3DEEA9592D67C9FEE`.
Compiled binaries remain local in `build/bin/RelWithDebInfo` and `build/e2e`.
The Phase 9 review ZIP deliberately contains source/evidence only.

2976 reference (outside repo):
`C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\reference-bins\2976\CrimsonDesert.exe`,
SHA-256 `57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`.
The user completed the single Steam depot touchpoint. See PHASE1-REFERENCES.md.

Installed 3663 target: timestamp 0x6AF5D198, SizeOfImage 0x144E9000, preferred
base 0x140000000, SHA-256
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Re-hashed for Phase 3 and each Phase 6–8 preflight, with no drift. JSON file
facts are in port-evidence/. Neither game executable was executed. Installed
files and saves were not changed or read as a live process.

Evidence SHA-256 values for the committed Git-blob bytes are recorded in
PORT-EVIDENCE-SHA256.json; this avoids ambiguity from Windows checkout newline
conversion. The ZIP's PHASE9-MANIFEST.json hashes every included file and records
the exact final commit. Evidence text has trailing whitespace normalized.

## Inventory, matches and the next experiment

PORT-INVENTORY.md and its JSON/literal supplement are pinned to the 2976 source
baseline. They include 1292 named rows and 8261 inline scalar occurrences in
193 files. These are a conservative audit superset, including policy values
and disabled headers, not 9553 required game offsets.

PORT-MATCHES.md / PORT-MATCHES-LITERALS.md list every row and its missing check.
**17 named values adopted as strong static identities; 1275 named plus 8261
inline occurrences remain unadopted/unclassified.** Three PE identity fields
are separate file facts. Only the 17 reviewed values enter build_3663.h;
none enables native execution. Weak discoveries remain in evidence only.

Methods: paired targeted Ghidra 12.1.4 analysis; Zydis whole-pdata normalized
fingerprints (242245/247296 decoded functions; 57160 unique candidates); named
RTTI/COL/vtable slots; existing short signatures; call-neighbour ranking;
strict pkt3663 entry verifier. This is not completed project-wide BSim/Version
Tracking. Ghidra/JDK projects/tools remain local; reproducible scripts and
commands are in tools/port3663/README.md. A concrete changed field is the heap
selector TLS byte: 2976 +0x1FD becomes 3663 +0x1FE, independently confirmed at
multiple native callers. Copying layouts unchanged is unsafe.

All blocking groups, with precise missing evidence:

1. **Field sharing:** old AcquireField 0x2B07790 and EnterField 0x2BC04F0 have
   no corroborated target after signature, normalized-function and neighbour
   matching. Confirm current function entries, the two locks, field user list,
   counts, reference ownership and rollback before installing H1/H2.
2. **Identity/status:** component-load and phase hooks have multiple candidates
   without independent role/register checks. Confirm current lobby/status
   number storage and post-load ordering before any identity write.
3. **Player/SAM and cleanup:** confirm singleton provenance, guarded player/HP
   chain, exact-key lookup, field user/actor accounting, native logout and
   deferred removal barriers. The 2976 maps cannot simply be reused.
4. **Wire completeness:** current descriptor IDs are discovery evidence only;
   confirm each handler/writer's exact consumption, conditional fields, sizes,
   channel/kind semantics and frame tag/tail usage. No old ID or layout adopted.
5. **Guest saves:** matching one refresh thunk does not establish early guest
   interception, save ownership or load ordering. These remain a live blocker.

**Smallest next experiment:** file-only trace of the named 3663
LoginCharacterToFieldServerReq handler at 0x2C6AE10 to its field-acquisition and
entry call paths; corroborate both targets and the actual list/lock writers
against the paired 2976 functions. This directly targets the first core blocker.
The existing coop3663 structural decoders/FakeActorBackend do not provide field
sharing, initialized native characters, identity fixups or deferred cleanup;
they are preserved as fallback research/tests, without expansion in this run.

## Exact validation and live-session record

| Check | Command / evidence | Result |
|---|---|---|
| Phase 0 baseline | `scripts/build.ps1`; phase0-baseline.txt | 101/101, 139.68 s |
| Phase 0 fixed | `scripts/build.ps1`; phase0-fixed.txt | 101/101, 93.77 s |
| Focused feed/decoder | build target cdmp_tests_coop3663, then executable | Passed |
| Phase 4 guard | build target cdmp_tests_build_guard, then executable; phase4-guard.txt | Passed, including wrong-size 3663 and zero gameplay callbacks |
| Phase 4 ASI | `cmake --build build --config RelWithDebInfo --target cdmp_mod` | Passed |
| Phase 5 full build/suite | `scripts/build.ps1`; phase5-suite.txt | 101/101, 111.81 s |
| Phase 5 synthetic loopback | `tools/port3663/smoke.ps1`; build/e2e host/peer, UDP 39763, 8 s | 78 remote states, sent 85/recv 83, rejected 0, clean shutdown/unload |
| Phase 6 | PHASE6-LIVE-STATUS.md; phase6-preflight-identity.json | NOT STARTED; blocked |
| Phase 7 | PHASE7-TWO-INSTANCE-STATUS.md; phase7-preflight-identity.json | NOT STARTED; blocked |
| Phase 8 | PHASE8-TWO-PC-STATUS.md; phase8-preflight-identity.json | NOT STARTED; blocked |

Failures/corrections: initial sandbox configure could not resolve github.com
for pinned dependencies; the authorized network-enabled build succeeded.
The first normal push stalled in the configured credential helper; the existing
authenticated GitHub CLI helper completed all phase pushes without new login.
A Python syntax-check invocation passed a literal wildcard; compileall corrected
the invocation and passed. No CTest failures or live crashes occurred in this
run. Full outputs are retained; the corrected commands, not suppressed results,
are the recorded validation. Self-review and scope are in PHASE5-VALIDATION.md.

No live session began, so no save backup, live log/capture or crash context was
generated. The Phase 6–8 files state the question, expected observation, time
limit and stop condition for future sessions. No deployment/removal or launcher
command ran. The combined Phase 6–7 presence and Phase 8 second-PC touchpoints
remain unused until their offline prerequisites are satisfied.

## What works and what remains disabled

Works: Phase 0 synthetic lifecycle/decoder fixes; reproducible static reference
comparison; a limited corroborated value header; strict 3663 identity rejection
of all gameplay side effects; complete game-free suite and synthetic transport.

Disabled on 3663: all native modules, including the intended minimal path,
plus mounts/rope/shops/docked gimmicks, NPC/enemy/control/combat probes,
scene/schedule extras, map/minimap and overlay. Compiled legacy defaults do not
grant permission because the central gate remains closed. 2976 address tables,
legacy module implementations, wire version, Steam-friends restriction and
executable filter are preserved. 3576 stays recognized and unarmed.

Do not deploy this branch as a working 3663 co-op kit. Passing loopback is only
synthetic transport evidence. Native walk/turn, creation/removal, reconnect,
save safety and real Steam tunnel acceptance remain unverified.

## Commits since the requested base

All checkpoint commits below were pushed to origin/gpt/3663-port. The final
Phase 9 package commit and its full SHA are appended to this list in the ZIP
manifest generated after publication.

| Commit | Subject |
|---|---|
| 3cc9c28018de903fdc8a5bfcceb322033b42dace | Phase 0: fix offline feed generation fences and review findings |
| 5abdc221b524e6d12b674ff55f2f26dcba19d550 | Phase 1: verify and record static reference executables |
| 57097568652233fff6088d548e7b7c3f0101e16d | Phase 2: inventory option-D dependencies and record 3663 feature boundary |
| 878d14fe129b5569c606318677e9703f0a4ce972 | Phase 3: corroborate static matches and document unresolved core port rows |
| 3c3ec37b713d7e676cd6bb898b21e32e92075699 | Phase 4: recognize 3663 while keeping incomplete gameplay path unarmed |
| 8e454e945f744f03ce1c8e6a59dcc6f8b369afc4 | Phase 5: validate guarded build and synthetic loopback, pin inventory source |
| fc61b770a39bfb78a48d8b6f61a7afb983699b81 | Phase 6: record live prerequisites blocked and verified build identity |
| 5b3e54062c437ad4a56ecd3d8218e6e08a3fd583 | Phase 7: record blocked two-instance acceptance and preparation |
| 0d4490706fecce9060756dcd84331257f6162e92 | Phase 8: record blocked Steam two-PC acceptance and guest-kit prerequisites |

---

## Historical checkpoints (superseded by the Phase 9 status above)

# 3663 port run — Phases 0–1 complete, later phases in progress

Branch: gpt/3663-port, based on a8656e87a0c43db400bdf863193c1afb353d6de0.
The user's current request and docs/GPT-3663-PORT-PLAN.md supersede the historical
review/offline-only cadence below. Live work remains limited to Phases 6–8 and
the plan's prerequisites; no live work has started.

Phase 0: all five review findings fixed, focused regressions pass, baseline
101/101 (139.68 s), post-fix full suite 101/101 (93.77 s). See PHASE0-PORT.md
and port-evidence/phase0-{baseline,fixed}.txt for full commands/output.
Phase 1: the user completed the depot download; the copied 2976 reference
matches SHA-256 57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7.
Paths and target/reference identities are in PHASE1-REFERENCES.md and its
referenced JSON files. Both executables are file-analysis inputs only.
The production 3663 path is still blocked. No two-player result is claimed.

Phase 2 is a conservative inventory with semantic/reachability audit remaining.
Phase 3 performed paired Ghidra analysis, whole-pdata normalized matching,
RTTI/slot and signature checks, and call-neighbour ranking. Seventeen values
are corroborated static identities. The complete core table is not resolved.
PORT-MATCHES.md lists every named row, the literal supplement, and five precise
blocking groups, including the next file-only field-sharing experiment.
Raw discovery confidence is not automatically promoted. No weak candidate is
allowed into a runtime path. Phase 4 added the corroborated static-value header
and strict recognized-but-unarmed 3663 identity. Guard regressions and the ASI
build pass; the complete per-build module routing remains blocked. See
PHASE4-IMPLEMENTATION.md. Phases 5–9 independent work is continuing.

The following sections are preserved historical checkpoints.

# Handoff: 3663 acknowledgement findings fixed; walk prototype blocked

User-requested transfer to Claude: [implementation handoff](../CLAUDE-HANDOFF.md).
Continue offline; preserve any newer local changes. The user requested this
progress checkpoint be committed and pushed to Shaugno before Claude takes over.
Earlier references to uncommitted work describe its pre-publication status.

## Latest checkpoint: concrete character-control component reader

See [COMPONENT-READERS-20261114.md](COMPONENT-READERS-20261114.md).
Resolved character-control and transform-sync class vtables through file RTTI.
Added an offline structural character-control decoder for its conditional
fields, variable bytes, resource IDs and variable entries. Nested reads are
accounted for; native object stride is not used as wire length. Focused tests
pass. Full offline suite passed 101/101 in 90.26 seconds, including the prior
controller fixture isolation fix. git diff --check passed. Native character creation is still
unimplemented; component inventory, transform structures, resource validity,
and ownership remain unresolved. No game use or deployment. Changes local.

Earlier checkpoints below retain their historical scope.

## Current Codex checkpoint: component initialization and cleanup

See [COMPONENT-LIFECYCLE-20261114.md](COMPONENT-LIFECYCLE-20261114.md).
New file evidence establishes the compact component list's initialization
order, shared-reader deserialization, reverse cleanup order, and a deferred
cleanup callback. The concrete factory and component schemas remain unknown;
these native list entries are not wire layouts. Failed-initialization rollback
and deferred completion ownership are still unresolved.

An offline decoder now checks the body relation tail only when its start is
independently known. It never guesses where opaque components end. Focused
tests passed. Full suite: 100/101 passed in 89.65 seconds. The controller
fixture hit a stale directory from a recycled dummy PID. Its test-only log
naming now includes the test process identity/creation time; the rebuilt
controller test passed in 49.03 seconds (CTest total 49.38). No failure was
suppressed and old evidence was preserved. The other 100 tests were not
rerun after this isolated fixture change. No game launch,
attachment, native calls, deployment, or push; changes remain local.

Next dependency: resolve the descriptor factory and each required component
reader, initialization phase and cleanup contract before native actor work.
The earlier checkpoint sections below retain their historical validation.

## Current Codex checkpoint: spawn/rotation/removal offline parsers

Latest evidence and implementation:
[SPAWN-HEADING-REMOVAL-20261114.md](SPAWN-HEADING-REMOVAL-20261114.md).
The bounded offline library now parses spawn record envelopes, the
file-traced four-component rotation, and the removal layout (including
its leading byte). A labeled synthetic test composes decoded fields through
fake actor create/move/remove and verifies late samples are refused.
Native actor creation/removal is still not implemented: component-specific
spawn bodies, native ownership/thread contracts, and live conventions remain
unresolved. Nothing was deployed, no game process was opened, and guards
are unchanged. Current implementation is local and uncommitted.

Validation: full suite 101/101 in 90.26 seconds; the final composed fixture
was added during that run and then separately rebuilt and passed. See the
new report for exact scope, evidence hash, and remaining dependencies.

The paragraphs below retain earlier checkpoints and their historical limits.

Latest offline implementation: `decode_move_ack_spatial_fields` now extracts
only the file-proven raw word and three finite spatial floats from the
239-byte full-parse MoveActorAck layout. It checks header, file-descriptor
opcode, exact frame length, and retained byte coverage. It does not provide
heading, coordinate-space classification, or live readiness and is not
connected to a game backend. New regression coverage and the full independent
suite passed: 101/101 in 98.97 seconds. Later tracer-only extensions rebuilt
and ran successfully. See the updated movement-consumer note for lifecycle
types and the remaining work. Nothing has been deployed or pushed.

Codex takeover update, 2026-11-14: the user assigned implementation and
investigation to Codex. New file-only results are in
[MOVEMENT-CONSUMERS-20261114.md](MOVEMENT-CONSUMERS-20261114.md) and
ack-trace-consumers-20261114.txt. The three floats at payload offsets
196/200/204 reach a conversion using 1000-unit spatial cells. Coordinate
space and live meaning remain unverified. The earlier blocker description
below is historical; independent investigation is continuing.

Date: 2026-11-14. Branch `grok/compat-guard`, HEAD `502577a`, local
diff uncommitted. This is a blocker handoff. Two installations have not
been shown walking or turning. No actor was spawned, no packet was
replayed, and no diagnostic was deployed. Production guards, protocol
`kVersion`, and the installed game file were not changed.

## Findings from ACK-HANDLER-REVIEW-20261114

1. The 17-byte MoveActorAck decoder is removed. `exe+0x26A8CE0` consumes
   the same reader between the first 4-byte read and the later reads.
2. The flat ActorLoad u16 decoder is removed. Slot 2's start is a runtime
   subtraction. `exe+0x9D68E0` receives the reader and reads further bytes
   itself; the caller's u16 is not its integer argument.
3. The literal header bound matches `cmp ax, cx` / `jb` at
   `exe+0xCA8B01`. Declared length `0x7FC1` passes. Only a greater length
   fails when `bound_flag` is 0. ActorLoad slot 2 uses a different bound
   and is not accepted by this gate.

## What the success path consumes

File-only `SEC_IMAGE` trace. The tool printed
`identity=build-3663 timestamp=0x6AF5D198 adopted=0` and
`move-reader-vtable=0x5B3BE18 slot28=0x141643CD0`. Listings:
`ack-trace-20261114.txt`, `ack-trace-helpers-20261114.txt`,
`ack-trace-nested-20261114.txt`.

MoveActorAck builds its reader at `rbp-0x80` with vtable `exe+0x5B3BE18`,
base `bytes+5` at reader `+0x20`, declared length at reader `+0x28`, and
cursor 0 at reader `+0x2C`. Slot `+0x28` is `exe+0x1643CD0`. That function
copies exactly `r8d` bytes and advances the cursor by the same count. The
handler fails the later reads when the slot is `exe+0x3A6E20`.

`exe+0x16436F0` only grows the pointer array at reader `+0x10` and
increments reader `+0x18`. It does not call slot `+0x28`.

On the success path the cursor therefore advances by:

| Step | Bytes | Running cursor |
| --- | ---: | ---: |
| first read into `rbp+0x148` | 4 | 4 |
| `exe+0x14CB240` | 136 | 140 |
| `exe+0x1644720` into object `+0x88` | 40 | 180 |
| `exe+0x1644640` into object `+0xB0` | 16 | 196 |
| object `+0xC0`, `+0xCC`, `+0xD0`, `+0xD8`, `+0xDC`, `+0xDD` | 12+4+8+4+1+1 | 226 |
| one byte, then 8, then 4 | 1+8+4 | 239 |

`exe+0xCA8D63` requires that cursor to equal the declared length. A
successful return has consumed 239 payload bytes. The 136-byte prefix is
`exe+0x14CB150` twice (64 each) plus one 8-byte read. Each 64 is
`exe+0x1644720` (12 + 16 + 12) plus two 12-byte reads. This is the
handler's own success path, not a captured message.

For the object-kind branch `[type+1] == 1`, `exe+0x55EF60` copies:

| Source | Payload offset | Store |
| --- | ---: | --- |
| 16 bytes at object `+0xB0` | 180 | `[A+0x55C]` |
| 8 bytes at object `+0xC0` | 196 | `[A+0x550]` |
| 4 bytes at object `+0xC8` | 204 | `[A+0x558]` |
| 8 bytes at `rbp-0x40` | 227 | `[D+0x90]` |

With S=qword[r14], A=qword[qword[qword[S+8]+0x68]+0x1A0], whereas
D=qword[S+0xE8]. These are different pointer paths; aliasing is unproved.

Those are byte copies. The instructions do not show a world position or a
heading. No decoder maps them to movement.

`exe+0x9D68E0` reads 1 byte, then 4, masks that dword with `0x30000000`,
and compares it with `0x10000000`. The matching arm calls
`[result+0x130]` with the reader. Allocation/removal through downstream
callees has not been established or ruled out.

## Tests

`ack_header_ok` checks the span, the declared-length equality, and the
inclusive `0x7FC1` bound. The movement and actor-load field decoders are
gone. The regression covers `0x7FC0`, `0x7FC1`, and `0x7FC2` with flag 0,
`0x7FC2` with flag 1, a short span, and a length mismatch.

Game was closed. Sibling RelWithDebInfo build, MSVC 19.52, then CTest:
101/101 passed, 0 failed, 124.67 s. The tracer changes after that run do
not link into a tested binary. No game process was launched or stopped.

## Blocker

Two-player walking is not demonstrated. The missing fact is the meaning
of the four stores above and a 3663 create/remove operation. The 239-byte
cursor map does not identify a player position, a heading, or an actor
lifetime, so neither a position reader nor a packet relay is selected.

Smallest next experiment: file-only, find one later read of
`[inner+0x550]` or `[inner+0x90]` that uses the value in a demonstrated
world-coordinate operation, or show that those slots are not coordinates.
Do not deploy a decoder and do not repeat the empty post-ready capture
for this question. A second installation cannot show the other player
until that fact exists.
