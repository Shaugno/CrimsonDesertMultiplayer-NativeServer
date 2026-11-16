# R2 work in progress — 2026-11-16

Latest continuation:

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
