# R2 work in progress — 2026-11-16

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
