# R2 Step 7 — blocker package for final review (2026-11-16)

Branch: `gpt/3663-port`. The native two-player goal was **not reached**.
Furthest tier armed: **none**. Furthest L1–L4 passed: **none**. Three passive
P1 observations are evidence for specific questions, not native-tier acceptance.

The exact final source commit, full commit list since `2c80c21`, archive file
hashes and source/evidence inventory are in the generated **R2-MANIFEST.json**.
It is generated from clean committed HEAD inside the local Step 7 archive.
R2-STATUS.json is the machine-readable status; R2-MATCHES.md/JSON contains
every row, including all blocked rows and their missing facts. The archive is
a source/evidence review delta, with governing files, attribution, GPL v3 and
SOURCE-DIFF.patch. No executable/mod binary, save, credential, downloaded tool,
analysis database, release or main-branch publication is included.

## Core and actual permissions

| Tier | Adopted | Blocked | Armed |
|---|---:|---:|---|
| A | 34 | 3 | no |
| B | 73 | 5 | no |
| C | 113 | 37 | no |
| D | 109 | 21 | no |

Total: **395 core contracts, 329 adopted, 66 blocked**; 17 R1 adoptions
retained. The compiled catalog contains 265 independently reviewed layout/
guard contracts and 58 adopted RVAs. These numbers overlap the row evidence;
they are not extra completed game features. All four completeness flags are
false. The 3663 PlayerLayout and complete B/C/D module adapters are absent.
2976 behavior is preserved; 3576 remains recognized and unarmed. Optional
modules remain unreachable on 3663, and the synthetic fallback stays unchanged.

## What was completed and why native work stays gated

Step 1 cut the reachable core below 500 rows. Step 2 completed whole-image
Ghidra AutoVT and default BinDiff using independently exported saved analyses
and exact image identities. AutoVT accepted 138,587 unique associations;
default BinDiff produced 207,217 function pairs. All raw output hashes,
algorithm counts and caveats are in port-evidence/r2/standard-completion.json.
The conservative run is supplemental. A redundant third stock subset was
stopped after required results completed; its exact process identity and
partial-log/config hashes are retained, no partial result counted.

AutoVT’s original markup warning is retained: the persisted audit found zero
error flags and zero failed markup items across 46 inspected core/anchor
associations, which does not prove no transient warning occurred. BinDiff’s
missing-basic-block diagnostics are retained. Standard similarity is not an
ABI guarantee. Native checks found further changed fields and two concrete
conflicts: VecPush and the control/self recipient-finder swap. Neither was
resolved by silently preferring the desired address.

Step 3 established the changed three-argument EnterField ABI and new roster,
but entry correspondence and complete ownership/rollback remain unproved.
AcquireField is adopted; the legacy four-argument detour is not installed on
the new entry. Travel clock publication is embedded in a larger transition,
so its old inline hook cannot simply be redirected.

Step 4 prepared/tested dependency guards and partial tables, then committed
each tier’s blocked decision. A remains blocked on real loading readiness
and the unpublished HP-only layout. B still lacks descriptor range/scan, pump
correspondence and an actual per-build native packet-bound path. C additionally
lacks full join/identity/containment adapters and ownership. D lacks complete
save callsites, moving-state contracts, title eligibility and native cleanup
barriers. Each exact row is listed below. Constants-only preparation is not
a completed port. No unsupported guard, Steam-friends restriction, executable
filter or explicit test-host route was bypassed; protocol remains version 8.

Step 5 completed the passive questions below, with L1–L4 gated by their unmet
prerequisites. No second PC was requested prematurely. Step 6’s comment fix
was completed in c5f7074; the fallback was not expanded. This Step 7 package
records the resulting blocker boundary without requesting intermediate review.

## Validation and binary identities

Latest full `scripts/build.ps1`, x64 MSVC RelWithDebInfo: **101/101 passed,
87.04 seconds**. Latest game-free host/peer loopback: **77 remote states,
zero rejected, clean Network stopped and Worker exiting**. Older runs had
78 states; the final status parses the actual log rather than assuming that
exact timing-dependent count. This is synthetic validation, not L2.

The guard matrix covers all 256 request/completeness combinations, every
lower-tier dependency, wrong size/timestamp/base, missing/unknown identity
and recognized 3576. Actual 3663 startup returns no legacy game reads, hooks,
overlay, synthetic feed or networking. The native HP helper refuses missing,
ambiguous or changing definitions and rechecks the source chains.

Failures/fixes/reruns are retained in port-evidence/r2/validation-deviations.md:
PID reuse in the 3576 fixture hit correctly protected existing archives; adding
process creation identity fixed the test path. A separate run was refused by
game-closed checks when an unmodified game launched during validation; the
user closed it and the full game-closed rerun passed. Earlier build/exporter
and matching failures remain in chronological evidence, including the
one-line defined-function exporter fix and graph-equivalence fixture.
Session hash records now distinguish canonical Git bytes from original raw
capture hashes. Package generation verifies every referenced live evidence
file and all raw standard-output identities before producing the archive.

Binaries are **local validation outputs only**, not included in the package:

| Output | Bytes | SHA-256 |
|---|---:|---|
| cdmp.asi | 15945216 | `8AA59BE1F8D70DE14736619B46D2785B6CDB498FC9E05B101C8DE05207818673` |
| cdmp_host.exe | 737280 | `4B3DA924D51443E1D7269373976E4AD8DDBDF74918B16F899C3C31E8DC4C2629` |
| cdmp_peer.exe | 2723328 | `2A2FB5B34B5D416D85588FDF504778111CA4BDC010EB6C441CD3DBA83999F1CE` |

Installed target SHA-256:
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Reference SHA-256:
`57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`.
No target drift was accepted; the downloaded reference was never executed.

## Passive sessions, cleanup and limitations

- **072000, HP definition:** both registries select key 0; unique native
  900000/900000 agrees with inventory 900/900. No HP variation was observed.
  The sitting exceeded fifteen minutes during delayed exit; that deviation
  is retained. Verified cleanup preserved normal save progress and led to
  the independent exact-process fourteen-minute watchdog.
- **075900, loading:** collection began after load because JSON DateTime
  conversion rejected a valid receipt. User-confirmed travel did not appear
  in the unsynchronized interval. The timestamp fix and precision regression
  preserve exact deadline checking. Watchdog termination is recorded as such,
  not a crash. Installation restored and backup verified.
- **100800, loading rerun:** collection verified active before Continue.
  438 samples show title not-ready, initial park, placement, movement/turning
  and a 182.35 m jump in the user-confirmed travel interval. No distinct
  not-ready interval appears during travel. Normal exit in about 4.5 minutes
  from launch; all 23 save files unchanged, backup verified, installation
  restored. No UI control or new screen capture used.

Each session’s question, exact executable identity, result, canonical log/
capture hashes and original capture hashes are in its result.json under
port-evidence/r2/live/. R2-LIVE-SESSIONS.md gives the ordered test disposition.
No native game crash or two-player success is claimed. The final cleanup
check found no game process and no test ASI/PDB/loader/config installed. The
optional matcher was identity-verified before stopping; other apps and the
original sibling checkout were left alone.

The actual new trace narrows, rather than closes, A028. Native client handlers
now identify candidate completion flags, but their full start/reset and
lifetime contract is unproved. The candidate owner predicate calls locking/
allocating helpers; neither it nor reference-acquisition vslots may be invoked
by an external passive reader. The existing pointer-only collector must not
be repeated as though it can answer this new question.

## Exact remaining rows

| Tier | Row / symbol | Missing fact |
|---|---|---|
| A | A026 kStatId_Health | P1 HP confirmed both definition registries select key 0 and unique raw 900000/900000 agrees with visible 900/900. Publish the definition-derived selector only with adopted definition-slot RVAs; one full-HP state does not prove literal-ID stability. |
| A | A027 kStatId_Stamina | The legacy reader still consumes stamina key 22. Prove the target stamina key, or publish an actual HP-only 3663 layout and explicitly revise target reachability; the unpublished helper alone does not retire this row. |
| A | A028 kLoadingParkHeight | P1 synchronized from title captured chain_not_ready, initial park at (0,1000,0), placement, walking/turning and user-confirmed fast travel with a 182.35m world jump. During travel the actor chain stayed coherent with no distinct park/not-ready interval; the initial park rule alone does not prove readiness during travel or pointer lifetime. |
| B | B052 kTrocInstancesBegin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesBegin at 2976 0x6A51AE0 |
| B | B053 kTrocInstancesEnd | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTrocInstancesEnd at 2976 0x6A82C80 |
| B | B065 kNetLoopPump | Standard support for candidate 0x3a8d94 (or its documented native operand anchor) is absent; independent check: Decoded seven-byte MOV RAX,[R15+E0] at the pre-flush native NetworkThread point; R15 is ClientNet, incoming queue+18 is handled before this point, then synchronous session+10 is flushed and returned queue+28 freed. Both complete parent loops reviewed. This is a safetyhook mid-point, not an inline entry. |
| C | C001 kSamSlot | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSamSlot at 2976 0x6DA67D8 |
| C | C009 kToNearExceptOneControlVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearExceptOneControlVtable at 2976 0x5B25358 |
| C | C010 kToNearExceptOneSelfVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearExceptOneSelfVtable at 2976 0x5B25378 |
| C | C011 kToNearVtable | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kToNearVtable at 2976 0x5B25418 |
| C | C012 kExceptOneVf1 | Resolve contradictory direct standard evidence for candidate 0x2d6c360; its anchor cannot override the conflict. independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C013 kExceptOneSelfVf1 | Resolve contradictory direct standard evidence for candidate 0x2d6c230; its anchor cannot override the conflict. independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C014 kToNearVf1 | Standard support for candidate 0x2d6c210 (or its documented native operand anchor) is absent; independent check: Exact named RTTI primary-vtable slot 1 identifies control/self/near independently; paired native bodies follow finder+10 to actor+68/TS+1A0 and the field mover (4F0 -> 508). Except-one implementations preserve native reference acquisition/release without filling the output; near tail-dispatches its full recipient-list helper. Default BinDiff swaps the similar control/self bodies, which is a conflict, not corroboration. |
| C | C015 kExceptOneVf1Head | Confirm 3663 code consumes kExceptOneVf1Head for the same native role as src/mod/session2/offsets_s2.h:101 ({0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00}) |
| C | C016 kToNearVf1Head | Confirm 3663 code consumes kToNearVf1Head for the same native role as src/mod/session2/offsets_s2.h:103 ({0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9}) |
| D | D014 kWindowsLockVf1 | Standard support for candidate 0x1593240 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 1 has the paired native thunk that dispatches this through vslot+18. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D | D015 kWindowsLockVf2 | Standard support for candidate 0xe297f0 (or its documented native operand anchor) is absent; independent check: Named WindowsLock<3> vslot 2 has the paired native thunk that dispatches this through vslot+20. Exact body and named RTTI tables are reviewed; shared use by other WindowsLock classes does not create a different native role. |
| D | D020 kSlotRefreshReqSequence | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kSlotRefreshReqSequence at 2976 0x2A22475 |
| D | D021 kSlotRefreshReqSequenceBytes | Confirm 3663 code consumes kSlotRefreshReqSequenceBytes for the same native role as src/mod/session2/offsets_s2.h:191 ({ 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10}) |
| D | D022 kTitlePlayReadsSaveMgr | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTitlePlayReadsSaveMgr at 2976 0x65DC3E |
| D | D023 kTitlePlayReadsSaveMgrBytes | Confirm 3663 code consumes kTitlePlayReadsSaveMgrBytes for the same native role as src/mod/session2/offsets_s2.h:212 ({0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00}) |
| C | C043 kEnterField | Standard support for candidate 0x2ecb130 (or its documented native operand anchor) is absent; independent check: Native field admission and failure cleanup. ABI CHANGED: 3663 has three arguments and iterates its character list; matching this RVA does not authorize the legacy four-argument detour. |
| C | C047 kVecPush | Resolve contradictory direct standard evidence for candidate 0x4cae10; its anchor cannot override the conflict. independent check: Native AddActor callee; 16-byte user entries, vector pointer/count/capacity, growth and element copy verified. |
| C | C066 kCharacterClass | Confirm 3663 code consumes kCharacterClass for the same native role as src/mod/session2/identity_fix.h:63 (0xA) |
| C | C067 kLobbyLoad | Standard support for candidate 0x2e879a0 (or its documented native operand anchor) is absent; independent check: Paired ServerLobbyActorComponent native loads obtain the owner lock and INT64_MAX number, call paired lobby-entry writers with the same argument roles and propagate results. Exact RTTI vtable slot +50 independently identifies both entries; RTTI labels alone are not a second native-body check. |
| C | C069 kLobbyVtable | Standard support for candidate 0x5f6f2e8 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C070 kLobbyConstant | Standard support for candidate 0x2e87db0 (or its documented native operand anchor) is absent; independent check: Native lobby loader instruction MOV RAX,7FFFFFFFFFFFFFFF followed by a stack store; its address-taken qword becomes the paired entry writer character-number argument. Target stack slot changes to RBP-40; no old instruction patch bytes are reused. |
| C | C072 kLobbyAddCall | Standard support for candidate 0x2e87e10 (or its documented native operand anchor) is absent; independent check: Native E8 call from the lobby loader to paired entry writer 2143E70/237EA60, with component, result, address-of-number, slot/name data and extra native arguments. This callsite proof does not establish a safe detour ABI. |
| C | C074 kClearLobbySite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearLobbySite at 2976 0x2C3E0FF |
| C | C075 kClearLobbySiteBytes | Confirm 3663 code consumes kClearLobbySiteBytes for the same native role as src/mod/session2/identity_fix.cpp:227 ({0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}) |
| C | C076 kClearListSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kClearListSite at 2976 0x2C3E126 |
| C | C077 kClearListSiteBytes | Confirm 3663 code consumes kClearListSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:229 ({0x48, 0x8B, 0x4B, 0x18}) |
| C | C078 kStatusVtable | Standard support for candidate 0x5f63430 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C079 kStatusLoad | Standard support for candidate 0x2c160b0 (or its documented native operand anchor) is absent; independent check: Paired ServerStatusActorComponent loads call the common status loader, propagate its result and write INT64_MAX through the character-number pointer, changed +70 -> +90; target LoginCharacter lookup consumes +90 independently. Exact named RTTI slot +50 supplies entry identity. |
| C | C080 kLoginMgrSite | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoginMgrSite at 2976 0x2777A18 |
| C | C081 kLoginMgrSiteBytes | Confirm 3663 code consumes kLoginMgrSiteBytes for the same native role as src/mod/session2/identity_fix.cpp:235 ({0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}) |
| C | C099 kPhases | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kPhases at 2976 0x2941020 |
| C | C100 kPhasesHead | Confirm 3663 code consumes kPhasesHead for the same native role as src/mod/session2/identity_probe.cpp:51 ({0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78}) |
| C | C101 kStatusVtable | Standard support for candidate 0x5f63430 (or its documented native operand anchor) is absent; independent check: Exact MSVC RTTI class/COL identifies this component vtable; slot +50 points to the independently reviewed native loader with matching role and changed fields. Standard loader agreement can anchor the paired slot relationship; RTTI naming alone does not satisfy adoption. |
| C | C102 kStatusLoad | Standard support for candidate 0x2c160b0 (or its documented native operand anchor) is absent; independent check: Paired ServerStatusActorComponent loads call the common status loader, propagate its result and write INT64_MAX through the character-number pointer, changed +70 -> +90; target LoginCharacter lookup consumes +90 independently. Exact named RTTI slot +50 supplies entry identity. |
| C | C103 kLoadStub | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kLoadStub at 2976 0x35EA90 |
| C | C110 kTravel | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravel at 2976 0x2BC0A30 |
| C | C111 kTravelHead | Confirm 3663 code consumes kTravelHead for the same native role as src/mod/session2/clock_gate.cpp:22 ({0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}) |
| C | C112 kTravelOwnerCmp | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravelOwnerCmp at 2976 0x2BC0A78 |
| C | C113 kTravelOwnerCmpBytes | Confirm 3663 code consumes kTravelOwnerCmpBytes for the same native role as src/mod/session2/clock_gate.cpp:26 ({0x39, 0x82, 0x80, 0x01, 0x00, 0x00}) |
| C | C114 kTravelGetterCall | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kTravelGetterCall at 2976 0x2BC0CC7 |
| C | C115 kTravelGetterCallBytes | Confirm 3663 code consumes kTravelGetterCallBytes for the same native role as src/mod/session2/clock_gate.cpp:28 ({0xE8, 0xD4, 0x48, 0x52, 0xFF}) |
| C | C122 kSessionId | Confirm 3663 code consumes kSessionId for the same native role as src/mod/session2/server.cpp:76 (0) |
| D | D030 kSamBucketMaxEntries | Confirm 3663 code consumes kSamBucketMaxEntries for the same native role as src/mod/session2/guest_lifecycle.h:55 ((kSamBucketBytes - 8) / 8) |
| D | D044 kTsMoving | Confirm 3663 code consumes kTsMoving for the same native role as src/mod/session2/guest_lifecycle.h:199 (0x3DC) |
| D | D045 kFmMoving | Confirm 3663 code consumes kFmMoving for the same native role as src/mod/session2/guest_lifecycle.h:200 (0x58) |
| D | D063 kCallSites[0].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[0].site at 2976 0x2C17881 |
| D | D064 kCallSites[1].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[1].site at 2976 0x2C3E1CE |
| D | D065 kCallSites[2].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[2].site at 2976 0x1042ABF3 |
| D | D066 kCallSites[3].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[3].site at 2976 0xCBD86CE |
| D | D067 kCallSites[4].site | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kCallSites[4].site at 2976 0x103BE99A |
| D | D100 kRetUserLogin | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetUserLogin at 2976 0x2C17886 |
| D | D101 kRetLobby3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetLobby3093 at 2976 0x2C3E1D3 |
| D | D102 kRetCharacter3093 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetCharacter3093 at 2976 0x1042ABF8 |
| D | D103 kRetAutosave | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetAutosave at 2976 0xCBD86D3 |
| D | D104 kRetManual2910 | Resolve standard correlator agreement or add an independent code/RTTI/xref proof for kRetManual2910 at 2976 0x103BE99F |
| D | D115 kReporterHead | Confirm 3663 code consumes kReporterHead for the same native role as src/mod/session2/report_guard.cpp:20 ({0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}) |
| B | B075 kMaxPacketSize | The legacy shared packet limit 7FE0 differs from the target native PostPacket bound 7FDA and ordinary decoder payload bound 7FC1. A target-specific acceptance bound and complete framing check remain necessary. |
| C | C144 kActor_OwnerUser | Confirm 3663 code consumes kActor_OwnerUser for the same native role as src/mod/game/offsets.h:230 (0x90) |
| D | D130 NativeTitleSlotDomain | Target title eligibility adds per-entry u32-list membership at +68/count+70 against available .admeta IDs at global 71AEAA0/71AEAA8. Target producer 14AA720 reads those IDs in addition to names; reference producer 128D360 only builds names. The new global dependencies, list lifetime/coherence and complete target parser remain unadopted/unimplemented. The old domain-only playable count is insufficient. |
| B | B078 NativeTrocScanSkip | The native descriptor scan skips 80h bytes after a match, then advances eight more. Target minimum instance span/alignment for every scanned descriptor is not yet established; the skip must not hide a neighbouring target descriptor. |

## Smallest next work and manual validation

1. Keep the game closed; follow the recorded client loading start/completion
   path to establish which non-mutating fields reset and become ready for the
   controlled actor, with lifetime/coherence rules. No native helper calls.
2. Extend the passive collector only after that field contract is reviewed;
   test missing, changing and incoherent data offline. Then request one exact
   user-present travel session with fresh backups and the existing watchdog.
3. Once all A values are adopted, publish the real HP-only per-build layout,
   explicitly retire unreachable legacy stamina selection for that target,
   rerun guards/full CTest/loopback and perform real L1 before progressing.
4. Resolve the independent B/C/D contract and adapter gaps recorded above.
   L2, L3 and the second-PC L4 remain ordered by the plan; do not substitute
   loopback or simulated actors for native two-player acceptance.

Reproduction tools: r2_refresh.ps1 regenerates the native catalogs and join
from the exact completed outputs; r2_match_tests.py verifies the join/conflict
rules; scripts/build.ps1 -BuildDir <local r2-build> builds/tests; smoke.ps1
uses only game-free test hosts; r2_status.py materializes the status;
package_r2.py packages a clean committed HEAD and verifies its manifests.
Source/runtime changes, evidence and commands are inspectable through the
commits below, the generated manifest, and SOURCE-DIFF.patch.

## Commits since round 1

- `90c12f5` R2 preflight: record passing baseline and missing plan blocker
- `29bc5df` R2 step 1: cut option-D game contracts into four core tiers
- `c5f7074` R2 step 6: clarify per-peer highest-generation admission
- `5a72f3b` R2 step 2 checkpoint: preserve native proofs and closed reader guards
- `5df9866` R2 step 2 checkpoint: verify native layouts and prepare passive HP trace
- `06e8b72` R2 tier A: isolate disabled reader route and validate startup effects
- `2daa8a3` R2 step 2 checkpoint: verify native wire and lifecycle contracts
- `30d105e` R2 step 2 checkpoint: verify identity layout and full exporter equivalence
- `7edb209` R2 step 2 checkpoint: verify save contracts and prepare loading trace
- `8e1b8ef` R2 step 2 checkpoint: verify save and receive contracts; harden evidence joins
- `5bf8fe4` R2 step 2 checkpoint: verify native descriptor field roles
- `2208404` R2 step 3: record field-sharing ABI assessment and ownership blocker
- `718ced6` R2 tier A preparation: guard definition-derived native HP reads
- `18635b3` R2 preflight: verify identity/save contracts and prepare guarded passive sessions
- `1fa910d` R2 step 5 P1 HP: record UI match and enforce hard session deadline
- `8d51290` R2 step 5: preserve loading trace and validate deadline receipts
- `90e92bb` R2 step 2: verify lifecycle and clock publication contracts
- `9c63f13` R2 step 2: join whole-image standard matches and native checks
- `dba9721` R2 step 2: complete default matching and native conflict audit
- `baebf12` R2 step 3: record field-sharing ABI and ownership blockers
- `582c2e9` R2 step 4 tier B: record closed-gate decision and validation
- `99d5329` R2 step 4 tier C: record closed-gate decision and validation
- `7992fa5` R2 step 4 tier D: record closed-gate decision and validation
- `185e63a` R2 step 4 tier A: retain closed guard after loading trace
- `6c3817e` R2 step 5: preserve synchronized loading trace and cleanup
- `521b4fd` R2 step 5: include bounded passive session logs
- `180da75` R2 tier C-D: verify native session and save-lock contracts

The generated manifest appends the Step 7 source commit and records the exact final commit.

---

# R2 preflight — 2026-11-15: execution plan missing

The user requested Steps 1–7 and per-step/tier commits on gpt/3663-port.
AGENTS.md and CLAUDE.md were read. docs/GPT-3663-PORT-PLAN-R2.md is absent
locally, from every fetched remote branch, and from fetched Git history;
searches of Downloads and prior workspaces did not recover it.

Required independent baseline passed: scripts/build.ps1, x64 RelWithDebInfo,
101/101 CTest tests in 109.20 seconds. The installed executable and stored
2976 reference retain their previously verified hashes; the game was closed.
No production source, guard, offsets, deployment or saves were changed.
No R2 step/tier or Step 7 package is claimed complete without its instructions.

See R2-PREFLIGHT-BLOCKER-20261115.md and r2-evidence/ for the exact search
scope, build output and current file identity. Required next input is the R2
plan's contents at the requested path or a retrievable file/ref. Existing
execution and push authorization remains in effect when that input is available.
The Phase 9 section below describes the historical R1 package at 2c80c21.

---

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
