# R2 work in progress — 2026-11-16

Latest continuation:

- Step 2 now has two completed real whole-image standard outputs: stock
  AutoVT and an additional stock conservative BinDiff profile. Exact scope,
  algorithm counts, warnings and output hashes: standard-completion.json.
  Current join: 310 adopted / 85 blocked; A33/37, B68/78, C105/150, D104/130.
  R2-MATCHES lists every row and missing check. Catalog: 253 native contracts
  and 51 adopted RVAs materialized in build_3663.h; every tier flag is false.
- Final regenerated build passed 101/101 in 90.38 s; smoke logged 78 remote
  states, zero rejected and clean unload. Earlier 235-contract suite passed
  101/101 in 89.85 s. A first header materialization rejected B010 because
  the inventory incorrectly labelled a vtable entry count as an RVA. The
  classification now distinguishes numeric counts; IDs/values are unchanged.
  The command chain initially continued onto the old header after that error;
  its 88.16-s build is retained as pre-final evidence, not used to validate
  the new table. The fail-fast refresh and final build were then run.
- Entry-guard extraction initially rejected non-pdata Windows lock thunks.
  Their actual named RTTI slots and complete seven-byte dispatch bodies
  (including the REX prefix) are now checked explicitly. Guards never adopt
  an entry RVA, unchanged ABI or permission to install a legacy detour.

- Network/lock follow-up: 234 reviewed contracts plus 17 R1, 55 independent
  RVA candidates. Q0/return-queue dispatch and client free paths prove tag
  storage only for complete mod-owned zero-kind C-to-S buffers handed directly
  to Q0, never native/request buffers. Full scope is in R2-TUNNEL.md. The
  seven-byte client pump site and actual network roots have independent
  candidates; save-manager construction proves lock self-offset+8. Full
  build/CTest passed 101/101 in 89.13 s, and synthetic smoke unloaded cleanly.
  No tier or native network path is armed.

- Lifecycle contract batch: 101/101 tests in 89.64 s; after the full clock
  batch, 101/101 in 90.63 s. Final synthetic host/peer smoke: 78 remote
  states, zero rejected, clean network stop/worker unload. Evidence-join
  regressions passed, including conflicting standard results and manual-RVA
  bypass rejection. All production target permissions remain false.
  Self-review: this batch adds unused target contract data and evidence only;
  it installs no hook, calls no native function, changes no wire format or
  legacy table, and does not establish native multiplayer behavior.

- Independent lifecycle/clock review raises the native proof catalog to 222
  (plus 17 retained R1 values), with 49 RVA candidates awaiting standards.
  The stable core remains 395. Five changed loading/completion IDs and two
  existing lifecycle aliases are checked through paired named descriptor
  initializers and handlers. Server-user child cleanup and key recycling
  now have complete paired exports. The six-argument native loading-complete
  clock gate, 32-byte component+450 copy and distinct clock banks are reviewed.
  The apparent neighbouring travel-hook lead was rejected by its actual
  roster-position role; R2-CLOCK.md records this unresolved publisher.

- Loading P1 R2-P1-20261116-075900 captured guarded position/rotation changes,
  but no missing-chain, parked or fast-travel transition. It began after save
  load, so title-to-load was not sampled. This is not L1 and does not resolve
  A028. User later confirmed completed fast travel, walking afterward and game closure.
  The candidate trace does not reflect that travel; without synchronized event
  markers, stale-chain versus out-of-interval travel remains unresolved.
  The independent watchdog stopped the pinned test process at fourteen minutes
  from launch. Cleanup verified: all test deployment files absent, 23 save
  files unchanged, verified backup, both owned matchers resumed. Session hashes
  and limitations are recorded in its committed result.json.
- The valid watchdog receipt initially failed because PowerShell JSON converted
  its ISO timestamp to DateTime and Parse coerced it back through a locale
  string, losing precision and shifting timezone. Preserve DateTime directly;
  parse only strings using invariant round-trip rules. Regression tests cover
  Z/offset strings, UTC/local DateTime, native JSON, fractional precision and
  malformed input; all passed. No deadline was extended or guard bypassed.

- P1 HP ran with the user present on exact 3663. Both definition registries
  selected key 0; the coherent controlled-actor container had 20 entries and
  exactly one matching HP entry, raw 900000/900000. Inventory visibly showed
  900/900 Health, confirming the native scale and selector for this full-HP
  state. Regeneration was not observed. This is passive evidence, not L1 or
  an armed tier. Full trace, startup log, original screenshots and hashes are
  in port-evidence/r2/live/R2-P1-20261116-072000/result.json.
- The collector stopped at 600.125 s. Game closure exceeded the fifteen-minute
  session cap while automated Escape did not open the quit menu and normal
  exit did not complete. The verified test PID was force-stopped; backup was
  verified, installed files restored, and normal changed saves retained. This
  deviation is recorded, not hidden or labelled an unqualified live pass.
  Before another launch, an independent watchdog was added: it pins the game
  process handle and enforces fourteen minutes from launch. P1 now requires
  its live identity/deadline receipt before collection. Owned hidden-child
  tests passed for deadline stop, normal exit and creation-identity rejection.

- Current core: 395 (A37/B78/C150/D130), with 190 additional native contracts
  plus 17 retained R1 values and 46 independent RVA checks awaiting standards.
  B078 records the native-instance scan skip that the cut previously missed.
  C150's known native protected IDs are proved; 1001..1008 is an unchanged
  conservative deny interval, not a claim about every individual handler.
  The new explicit control table denies everything when incomplete; actual
  production still uses legacy values and all 3663 tiers remain closed.
- Control-policy build initially passed 101/101 in 87.10 s. A subsequent build
  hit Proton Drive locks (MSB3491, LNK1168/LNK1201), with ample disk space.
  Optional BuildDir parameters preserve existing defaults while allowing an
  unsynced build. Reusing pinned dependency source trees, the external x64
  RelWithDebInfo build passed 101/101 in 131.07 s; smoke 78 remote states,
  zero rejected, clean network stop/worker exit. offsync-*.txt and the failed
  control-policy-final-build.txt preserve the actual failure/fix/rerun.
- P1 now permits only receipt-verified suspended owned matchers during live
  collection. Pause/Verify/Resume and duplicate-resume/foreign-PID/reused-PID
  rejection were rehearsed without the game. R2-MATCHING-PAUSE.md records the
  bookkeeping failure and fix. No matcher output is treated as finished by
  suspension. The fresh HP backup/question is R2-P1-20261116-072000 (23 files,
  verified); user presence has now been confirmed for the batched HP/loading
  sitting. No live result is claimed yet.

- Native character-number lookup is identified at 2D7A640 by its complete
  locked list/number/reference role. Member +B8/+C0 and two validation-window
  byte contracts are proved; their site RVAs still await standard support.
  Title consumer 758EA0 independently confirms the WorldSystem+88 save-manager
  hop, but exposes a new requirement-list eligibility gate absent from the
  legacy parser. D130 stays blocked on that complete rule. Catalog is 189
  plus R1 17; independent RVA candidates 46; core remains 394. Per-row critical
  missing facts/next experiments are in r2-blockers.json, consumed by the
  real evidence join without changing adoption decisions.
- Evidence joins now require a FUNCTION association for a VT-only native
  operand anchor. A DATA association cannot stand in for a function match;
  a dedicated synthetic rejection fixture passes. All previous conflict and
  manual-RVA bypass fixtures still pass.

- Tier A preparation now has an unpublished HP-only native reader selected
  by an explicit definition-layout option. It requires both complete registry
  chains to agree on a u16 key, one valid matching stat, stable definitions
  and source containers, and unchanged actor/control/physics pointers. No
  literal ID or single-registry fallback is used. Legacy defaults keep the
  existing health/stamina loop; the 3663 table remains absent and all tiers
  false. No A row is adopted/retired by this change. P1 still must validate
  usability and HP agreement, followed separately by real L1 readiness tests.
- HP-path validation: full build/CTest 101/101, 87.82 seconds; targeted
  consensus/changed-chain/ambiguity/value/bounds regressions passed. Synthetic
  host/peer smoke logged 78 states, zero rejected, clean unload. Canonical
  native-health-*.txt logs are retained. Self-review: all new memory accesses
  use guarded copies; no native call, hook, network/protocol or save change;
  no legacy layout or unknown-build fallback introduced.

- After 8e1b8ef, descriptor consumers add five member/value proofs: catalog
  184 plus 17 retained R1 values; 44 independent RVA checks await standard
  support. Incremental cdmp_mod/build_guard compilation and all guard tests
  passed (troc-fields-build.txt / troc-fields-guards.txt). The native framework
  mode operand initially holds 11h, distinct from named request kind 4.
- Step 3 ABI/ownership assessment is written in R2-FIELD-SHARING.md, with
  R2-FIELD-TRACE.md specifying the next observation and its unmet collector
  requirements. Field-sharing implementation remains blocked on standard
  entry support and roster ownership/lifetime/rollback; no hook is armed.

- At 05:49 EST both previously active matcher processes were absent and no
  final VT TSV or BinDiff database existed. Their last logs are preserved as
  vt-interrupted-0549.log and bindiff-interrupted-0549.log locally; the cause
  of interruption is not established. Neither run counts as completed evidence.
  BinDiff restarted from the unchanged saved exports; stock AutoVT restarted
  in fresh session R2AutoCoreRetry0553. An immediate retry of R2AutoCore was
  rejected because that interrupted session already exists. The driver now
  accepts an explicit session name, preserving interrupted databases.
- Export-session reopening also exposed a script transaction lock. The export
  script now ends its enclosing transaction before opening the VT session,
  following the stock AutoVersionTrackingScript lock sequence. Match algorithms
  and adoption thresholds are unchanged; the final game export still needs to
  run. The owned synthetic AutoVT/export fixture passed with 125 accepted
  functions. Its first attempt tried matching an unsaved import, which stock
  AutoVT refused as read-only; saving both imports before matching fixed that
  fixture setup. Both failure and successful logs plus output hashes are
  retained in port-evidence/r2/vt-export-fixture*. No fixture matches are
  included in game adoption evidence.
- Core is now 394 (A37/B77/C150/D130). Seven further native contracts bring
  the reviewed catalog to 179, plus 17 retained R1 values. Independent RVA
  checks now number 43; all await actual standard support. Save flush, title
  entry, native AES enablement and packet epoch facts are detailed in the
  canonical evidence. All prior core IDs/symbols/values are checked stable.
- Evidence joins now reject a contradictory direct global match even if a
  paired operand anchor agrees, and reject manual function/global RVA proofs
  that bypass standard support. Synthetic regression fixtures pass. These
  fixtures never populate the actual R2-MATCHES output.
- The pinned official loader is now available locally and hash-verified;
  port-evidence/r2/loader-identity.json records its identity. It has not been
  deployed. Passive HP/loading profiles remain prepared and unrun.
- Continuation validation: full scripts/build.ps1 passed 101/101 CTests in
  91.15 seconds. Synthetic host/peer smoke logged 77 remote states, zero
  rejected, clean network stop and worker exit; frozen local logs are
  build/r2/continuation-build.txt and continuation-host.txt. This does not
  establish native 3663 behavior. Evidence-join regression fixtures passed.

Earlier checkpoint details (counts and process IDs below are historical):

- Full whole-image exports completed and saved: reference 589385035 bytes,
  target 615418483 bytes, 18.69/19.52 million analyzed instructions. Hashes
  and local paths are in port-evidence/r2/whole-image-exports.json. BinDiff
  runs on those completed independent exports; its missing-basic-block
  warnings are retained for review, not hidden.
- Stock AutoVT initially failed when opening/closing the source project
  cleared AppInfo's active project. R2CopySource now restores the destination
  and validates a reused full-image source copy. A default retry then spent
  repeated samples in optional duplicate-function generation; it was stopped
  incomplete. R2VTOptions uses the official headless configuration seam:
  every Step 2 required standard pass remains enabled, optional Duplicate
  Function Correlator is disabled, thresholds unchanged. Current session is
  R2AutoCore, exec 68411; BinDiff remains under controller exec 60079 (that
  controller will also report its earlier failed VT attempt). No partial or
  stopped session is evidence. Both final correlators are still in progress.
- Catalog now 172 beyond retained R1 17; 36 independent RVA checks await
  standard support. Native save/header/map review is in R2-LIFECYCLE; native
  wire registration/lookup code corroborates descriptor ID use. Character
  logout Ack 2679→3116; shutdown timer 2038→2042 is a partial C150 fact, not
  a completed target filter. Native epoch and client session-slot fields
  are confirmed from the named connection acknowledgment.
- Core cut correction: append D126 for the inline native save-header schema;
  all older IDs/value pairs mechanically verified unchanged. Core is 388.
- Passive trace preparation now has separate hp/loading profiles, each with
  one written question, ten-minute collection and full backup/restore cycle.
  Guarded loading collector tests passed; API check uses only its own process.
  Native vslot C0 was rejected as a loading alternative because it mutates
  references. Both actual passive profiles remain unrun; A/B/C/D stay false.
- Full build/CTest after passive modes: 101/101, 98.63 seconds; synthetic
  host/peer 78 remote states, zero rejected, clean network stop/worker exit.
  Later unused catalog additions receive an incremental compile/guard check;
  no native runtime result is inferred from these tests.

Previous checkpoint:

- Native identity review adds 18 contracts: catalog now 149 plus 17 retained
  R1 values; 28 RVA candidates have independent checks but still require
  standard support. Status character-number pointer changes 70→90. Incremental
  ASI/guard build passed; guard identity/tier matrices passed. See R2-IDENTITY.
- Original native exports were retired incomplete after repeated stacks in
  promoteUndefinedFunctions. R2-EXPORTER records the one-line defined-function
  skip, original revision, isolated jar, fixture failures/fixes and complete
  graph equivalence (759 instructions, 68 flow graphs, undefined function
  restored). This is transparently a patched exporter; matchers are stock.
- Current pipeline uses nativefix, projects R2NativeSavedSource/Target and raw
  analysis-tools/r2-nativefix. Analyses save before exports in separate
  processes. Then both standard matchers run. Controller exec session 60079;
  no actual matcher completion or new RVA adoption is claimed yet. Earlier
  PIDs and the unmodified-export controller below are historical, stopped.
- All tiers remain disabled; no live presence request/deployment/game access
  has occurred. P1 remains prepared, not run. This remains a Step 2 checkpoint,
  not the final Step 7 package; independent native save review continues.

Earlier continuation:

- Pushed 5df9866 (native evidence/passive preparation) and 06e8b72 (isolated
  Tier A route). Full routing build: 101/101, 88.70 s; synthetic smoke 78
  states, zero rejected, clean unload. Actual A/B/C/D remain false.
- Reviewed catalog now includes wire and lifecycle facts in addition to the
  earlier layouts/entry guards. R2-WIRE.md records 19 named ID mappings and
  separately proved save/login/logout payloads. R2-LIFECYCLE.md records the
  native save-entry stride change 68→78. Native actor-map member roles are
  verified independently of their still-unmatched entry RVAs. Reproduce the
  current exact contract count with r2_reviewed.py; do not use older counts.
- Corrected native full-image analysis completed: source 211152 functions /
  18688006 instructions (734 s), target 214216 / 19517708 (771 s). Official
  unmodified BinExport is still building the full graphs. The correlators
  themselves have NOT completed yet. A local controller waits for both
  completed exports and database closure, then runs standard VT and BinDiff
  in parallel with fresh file-identity checks.
- Projects: build/r2/projects/R2NativeCleanSource and R2NativeCleanTarget;
  raw output analysis-tools/r2-native. Active export Java PIDs at this
  checkpoint: 53872 source / 12868 target. Controller exec session 35252.
  The redundant direct jobs were stopped incomplete after the native analyses
  completed; no default/graph/direct result is counted as completed evidence.
- One native-profile preflight misspelled the analyzer "Address Tables";
  the actual key is "Create Address Tables". Its two jobs were stopped and
  logs preserved as failed-option-old/new.log. Options now use HeadlessScript
  ABORT until success, so a script exception cannot silently continue analysis.
  Existing-database preflight verified all four table/FID settings and RTTI on
  before the corrected full imports. Driver rejects REPORT SCRIPT ERROR.
- New target EnterField roster semantics are documented in R2-FIELD-SHARING.md.
  Ownership of all existing roster members/factory branches remains a real
  adapter prerequisite. An unrelated auxiliary offline query was excluded
  from proofs; its raw output is private under build/r2, and the canonical
  roster export was regenerated with only the two actually reached callees.
- P1 is still only prepared, not deployed/run. No user presence question has
  been sent. Matching/export jobs must close before the live sitting. The
  Downloads R2 plan and repository copy still have identical SHA-256
  A020CF4B70EEBFAABE6A3CA7C048F9AA598D094B0A9DD988249A23E1BAF2AF66.
- Step 7 packaging script is prepared but not run: it requires the final
  committed status and real matcher outputs. This continuation is not Step 7.

## Prior checkpoint before these continuations

Current checkpoint (supersedes the historical section below):

- Branch gpt/3663-port; Step 1 and Step 6 pushed. Step 2 remains in progress;
  this is not the final Step 7 package. No tier armed, game launched, game
  process opened, deployment made, or live presence requested.
- Core 387 game contracts: A37/B75/C150/D125. Every retained row now has an
  explicit game-contract classification; policy exclusions remain documented.
  Stable IDs A001–A030 were preserved when seven native HP/root rows were added.
- 17 R1 values retained; 80 additional reviewed native contracts (layouts and
  seven entry-byte guards), still unused by any target gameplay path. Fourteen
  independent RVA checks await standard-correlator support in r2-rva-checks.json.
  Their existence alone never adopts an RVA.
- Paired native getter 8B44E0/9CDC00 confirms manager+50 controlled actor;
  WorldSystem+30 role separately verified. Field key 2C8→2D0 and mover busy
  5A→4A are verified. R2-FIELD-SHARING.md records ownership, locks, rollback
  and the changed three-argument entry ABI; standard entry support is pending.
- R2-TUNNEL.md records native packet maximum 7FDA versus mod helper 7FE0,
  and the still-unproved safe reuse of native packet metadata at 8010/8018.
  These must be resolved in the per-build path before Tier B can be enabled.
- Full preflight build/CTest 101/101, 101.61 s. Synthetic smoke: 78 remote
  states, zero rejected, clean unload. Collector logic and own-process API
  checks pass. Evidence-join fixtures exercise agreement, single-plus-native,
  native operand anchors, conflicts, unaccepted VT and an unproved health ID;
  they write only build/r2/match-fixture and are never port evidence.
- P1 passive HP collector and Prepare/Deploy/Probe/Finish controller prepared.
  Session R2-P1-20261116-020700 has 23 save files backed up and hash-verified
  privately. Only Prepare ran; no deployment. Written question and stop rules
  precede launch. The collector has a read/query-only process handle, strict
  file/mapped identity, guarded bounded reads, coherent resampling and no
  production feed. Live presence must still be acknowledged explicitly.
- Default and graph runs were stopped incomplete after prolonged constant
  propagation/decompiler recovery. Their verified process identities and
  incomplete status are local in build/r2/stopped-default-graph.json. Direct
  runs remain active in AddressTableAnalyzer. A native profile keeps complete
  image import/disassembly, explicit references and RTTI, while additionally
  disabling Address Tables and Function ID (besides direct's speculative
  propagation/switch passes). Exact options are saved with each run. Its
  whole-program exports and the standard VT/BinDiff runs are not complete yet.
- One signature-extractor check initially assumed a terminal RET. Native
  allocator wrapper ends with a verified tail JMP; corrected to guard the
  full 92-byte entry wrapper and explicitly avoid claiming tail-callee equality.
  No game crash occurred.

Next: complete standard matching, generate real R2-MATCHES and push Step 2;
finish the Step 3 correspondence/adapter assessment; close and validate each
remaining tier contract; request the prepared passive sitting only after
offline jobs end. Continue independent work through the Step 7 package.

## Historical checkpoint at 5a72f3b

This is a checkpoint, not Step 7 or a completed Step 2. Branch gpt/3663-port.
Step 1 pushed as 29bc5df. Independent Step 6 pushed as c5f7074 while analysis ran.
The Downloads R2 plan is copied verbatim to docs/GPT-3663-PORT-PLAN-R2.md.

Core audit: 380 rows (A 30, B 75, C 150, D 125). The extra A row records the
native stat-ID width discovered during code review. The 17 R1 adoptions remain.
r2-reviewed.json records 46 further native layout contracts; these are compiled
as unused data in build_3663.h. Function/global candidates await the standard
correlators. No tier is armed, no live session has run, no game was deployed.

Native findings, with paired disassembly in port-evidence/r2:

- Login decoder 2C6AE10 -> implementation 2F4AE80. Reference implementation
  2C3E030 has the same save/load/login sequence. Standard agreement is pending.
- Shared FieldMover caller 2BBFA80 -> candidate 2ECA480 (seven mapped call
  neighbours). Its acquisition call is 2E08520; entry is 2ECB130. Acquisition
  preserves pool lookup, reference ownership, open/owner assignment and result
  ownership. Entry now iterates a character list and has three native arguments;
  the reference has four. The legacy detour must not be installed there.
- Native AddActor 2AFF1E0 -> 2DFD3A0, both Denuvo thunks. Target body EB68740
  preserves both field locks, user admission and rollback. Users/count/actors
  move 2A0/2A8/2B0 -> 2A8/2B0/2B8; level entry 2D0 -> 2D8. Its obfuscated
  rollback scalar is one: file value 1F38148 XOR 1F38149.
- TransformSync FieldMover moves 4F0 -> 508. Field key offset and busy state
  still need exact checks; a whole object is never presumed unchanged.
- Native control consumer 859A00 -> 96C900 confirms actor+68 -> control+40 ->
  inner+140 -> physics, where the last pointer moves 2C0 -> 2D0. Physics position
  remains +90; the native constructor and quaternion/matrix helpers confirm
  basis rows +60/+70/+80.
- Native stat lookup 194CD30 uses u16 IDs and 90-byte entries. The HP handler's
  updater 71C4E0 reads the HP ID from game-data definitions, not a literal zero.
  Reference 62FA00 uses definition slot 6D69528/6D69530, TLS mode byte 1EC and
  HP-key field A0; target uses 7245558/7245560, TLS mode 1EB and HP-key field 78.
  Therefore the old hardcoded health/stamina IDs are not yet adopted.

Guarded reader work is prepared independently: four startup tier permissions,
all 256 request/completeness combinations plus corrupt identity cases tested;
no ini/environment evidence override. A per-build player layout selector returns
only the legacy table. A compile-time assertion prevents arming A until its
actual target table is published. PlayerReader PE copies are now guarded.
A width-aware stat reader preserves the legacy u64 path and supports native
u16 IDs without comparing padding. None of this publishes a 3663 player feed.

Validation so far:

- Preflight: 101/101, 109.20 s (prior evidence).
- Tier framework: 101/101, 98.66 s; loopback 78 remote states, clean unload.
- Layout selector: 101/101, 92.96 s; loopback passed, clean unload.
- Stat width/padding regression: 101/101, 105.23 s.
- Checkpoint compilation and guard test passed after using target cdmp_mod
  (the first command incorrectly requested the nonexistent target cdmp).
- First sandboxed MSBuild stalled spawning nodes; its verified process tree
  was stopped and the elevated rerun passed. No game crash or blind live retry.
- A helper query initially used an extra leading 4 in two matrix-helper RVAs;
  it returned BLOCKED. Corrected queries 44AF690/44AD6C0 supplied the proof.

Standard tooling: Ghidra 12.1.4, Temurin 21.0.13+9, official Google BinExport
source 9648cacb21a71aa6c21b3fca504f146c39164a53 (built unmodified using Gradle
8.14.3), official BinDiff 8. No tools or game binaries are committed.

Raw default analysis runs remain active. A separate whole-image graph profile
is running without a timeout. The earlier correlation profile was stopped
while in constant propagation and is not completion evidence. Both profiles turn
off Scalar Operand References, Stack, Decompiler Parameter ID, and speculative
Reference.Subroutine References; explicit references, RTTI and switch analysis
remain enabled. The graph profile also disables x86 Constant Reference Analyzer;
this preserves decoded direct call/data references while omitting speculative
register-value recovery. Full defaults remain a separate comparison. Exact
options are recorded beside the raw logs. RTTI seed TSVs are committed; they name
data only, never assumed function pairs. BinExport must
precede VT markup so BinDiff receives independently analyzed inputs.

Local analysis root:
`C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\analysis-tools`.
Ghidra projects are in build/r2/projects (Ghidra rejects project paths containing
a dot-prefixed component). Target/ref hashes are checked before each driver run.
Small committed text exports have whitespace/line endings normalized; original
local hashes and canonical committed hashes are in port-evidence/r2/text-hashes.json.
Do not treat the custom .pdata-range fingerprints as whole-function equality:
an identical short unwind fragment can precede a changed field in the body.

Next: finish standard VT/BinDiff, write R2-MATCHES, then commit/push Step 2;
finish the field-sharing ABI assessment and Step 3; close each tier's remaining
contracts, validate and push each tier. Prepare a concrete passive HP-definition
session if static evidence needs runtime data. Ask for presence only when the
collector, backup/restore checks and exact session question are ready. Continue
independent work through Step 7; this checkpoint is not a stopping point.
