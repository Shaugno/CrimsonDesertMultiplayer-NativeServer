# Grok execution plan: from usable 3663 captures to a co-op prototype

Current direction, updated 2026-11-14:
[Co-op prototype plan](COOP-3663-PROTOTYPE-PLAN.md) supersedes the requirement
to repeat Packages A/B as the immediate next work. Preserve this plan as
the capture procedure when a focused experiment needs it. First finish the
independent state/transport/actor-path work in the new plan.

Updated 2026-11-14. Owner: Shaugno. Grok implements; Codex reviews completed
milestones. This plan batches work so the user does not shuttle every small
fix between assistants. It does not provide automatic monitoring or automatic
Codex reviews. The user starts the work by giving Grok the prompt below.

## Starting point

- Repository: `Shaugno/CrimsonDesertMultiplayer-NativeServer`.
- Approved published checkpoint: `3e950a7`, branch `grok/compat-guard`.
- Work in the recovered checkout opened in Cursor, not the incomplete old
  OneDrive folder. Inspect branch, HEAD, status, and remotes first. Preserve
  newer/unrelated work; never restore an old agent snapshot over it.
- Target identity: executable 1.0.0.3663, timestamp `0x6AF5D198`, base
  `0x140000000`, image size `0x144E9000`, plus all existing structural,
  signature, topology, and ownership checks. A public patch label alone
  is insufficient. Recheck the installed file before live work.
- Corrected post-ready admission, exclusive per-attempt session logs, and
  sealing are reviewed. The last independent full suite passed 100/100.
- The earlier live archives contain useful startup timings, but the Nov 14
  windows retained zero post-ready records. No 3663 movement layout or
  working co-op has been demonstrated.

Read `AGENTS.md`, `CLAUDE.md`, `docs/probe-3663/LIVE-CONTROLLER-3663.md`,
`LIVE-RUNNER-3663.md`, `CAPTURE-REVIEW-20261114.md`, and the final disposition
in `ADMISSION-REVIEW-20261114.md`. Preserve GPL and upstream attribution.
Some older status paragraphs describe historical checkpoints; use the
published commit and final review dispositions to resolve those differences.

## Working agreement: finish a package, then request review

Implement, self-review, add meaningful regressions, fix failures, and rerun
the affected tests without asking the user to relay each iteration. Run the
full suite at a completed code milestone. Never weaken a test simply to hide
the behavior it should detect. Record intermittent failures and measured
values honestly, including failures that pass on rerun.

Proceed through Package A and Package B below while their prerequisites hold.
Return one consolidated handoff when both are finished, or when a concrete
blocker prevents further independent work. Do not stop at every small edit or
ask the user to get an intermediate review after each test correction.

The user is needed for Steam launch and the physical stand/walk/turn actions,
credentials if unavailable, an unexpected game-version change, or an action
outside this package. Ask concise questions only when needed. Group related
information requests. Avoid progress claims such as "co-op works" until
the corresponding acceptance criterion has actually been observed.

Normal implementation changes and draft reports stay local for the milestone
review. Publishing to GitHub follows the user's explicit save/push requests.
Do not merge unrelated work or change repository visibility/access.

## Package A: obtain usable controlled captures

### Preparation with the game closed

1. Verify the approved source and binaries. Build if necessary, record compiler
   configuration and SHA-256 hashes of the controller/DLL, and confirm relevant
   game-free checks before live use. A known unchanged reviewed checkpoint
   does not need repeated full-suite runs without a reason.
2. Verify the currently installed executable with the approved verifier.
   Refuse an unsupported identity; do not update a timestamp or reuse an RVA
   to force a match. Record version, identity, verifier output, and file hash.
3. Back up saves while the game is closed. Record per-file relative path,
   size, and SHA-256; compare copied files to their sources and retain the
   verification manifest locally. Do not publish save data or credentials.
4. Confirm `cdmp.asi` and `winmm.dll` are absent from the game directory.
   Use a new capture directory, leaving every earlier archive intact.

### One guided live session using the reviewed diagnostic

Have the user launch a fresh process through Steam and choose a suitable
in-world place. Identify that explicit PID. Follow the controller contract
for status/load/mark/start/stop; never select another process automatically.

Collect separate stand, walk, and character-turn windows. Explain each action
before arming and have the user maintain it through the observed capture.
Distinguish character heading from camera movement. Plan around the finite
256-record budget; the first useful rows must actually overlap the action.
Keep the existing 4096-byte copy cap, empty allow-list, and raw labels.

For each window require:

- observed successful start, correct phase, matching non-fixture identity,
  successful durable readiness, no retained ownership, no session-write
  failure, and no history-truncation flag;
- a separately preserved numbered attempt session file, records, command
  output, readiness cutoff, binary/source identity, and SHA-256 manifest;
- observed stop with hooks Removed and the process alive;
- nonempty retained records whose QPC meets that attempt's readiness rule.
  Pre-ready traffic must not have consumed accepted storage. For outgoing
  movement analysis, obtain complete post-ready send records in the
  contrasting windows. A phase label by itself proves no field meaning.

Automatically audit the files after each window. Check byte counts, copied
versus claimed length, raw first-word consistency, phase, QPC cutoff, and
truncation/dropped/omitted accounting. Incomplete records may be described
but cannot establish fields beyond their retained bytes. Record coverage
loss instead of claiming every call was captured. Never manufacture a
missing direction or missing movement sample.

On timeout, Retained, failed readiness/logging, or identity mismatch, stop
the experiment, preserve evidence, and use only the reviewed cleanup policy.
A timeout is not cancellation. Do not force-unload uncertain code, terminate
remote threads, or automatically retry the failed process. Diagnose safely
offline and prepare a focused handoff. If code changes are needed in the
diagnostic, batch implementation/tests locally and obtain a review before
loading the changed diagnostic into a new live process.

**Package A passes** only when all three contrasting windows have usable
post-ready evidence and complete per-attempt histories. If evidence is
insufficient but no failure occurred, specify the exact missing observation
before requesting another planned window; avoid open-ended blind captures.

## Package B: analyze movement evidence offline

Once Package A passes, continue without another routine user check:

1. Build a bounded offline parser/analyzer for copies of the captures. It
   must not open a live process or invoke a game function. Validate headers,
   versions, hex, lengths, phase and cutoff; reject invalid/truncated history.
   Add tests for malformed input, pre-ready rows, incomplete frames, and
   mixed identities. Keep originals unchanged.
2. Compare standing, translation, and turning. Show recurring frame shapes,
   byte changes, candidate fields, and corroborating static-file evidence.
   Distinguish a candidate numerical correlation from a verified message ID,
   coordinate, rotation, timestamp, or actor identity.
3. Write a field/evidence table: frame byte offset versus payload offset,
   width, representation/endianness, observed values, supporting rows/windows,
   confidence, and counterexamples. Do not import 2976/3576 layouts as facts.
   Position may be local or world-space; determine that from evidence.
4. Verify candidate fields on a separate observation or controlled contrast
   rather than just fitting the rows used to propose them. If more live
   evidence is necessary, specify the exact experiment and use the same
   reviewed diagnostic only. Do not infer fields from missing records.
5. Provide a machine-readable offline summary and a human report. If a
   field is unresolved, leave it unresolved and explain what would prove it.

**Package B passes** when verified or explicitly unresolved movement-field
claims can be traced to raw rows and independently checked. Working co-op
is not a Package B acceptance criterion.

## One review handoff for Packages A and B

Keep `docs/probe-3663/HANDOFF-CURRENT.md` as the single current handoff.
Include source/base commit, changed files, exact commands and test results,
capture directories/hashes, accepted/rejected windows and why, measured
startup times, proved fields, remaining hypotheses, reproduction steps,
and the smallest proposed next implementation. Include first-run failures
and reruns. Do not claim tests or observations that were not performed.

Self-review input bounds, concurrency, shutdown/retention, evidence
preservation, version isolation, and production/fixture export separation.
Resolve findings that you can reproduce within the package and keep the
game closed during code changes/tests. When the package is ready, tell the
user once: "Ready for Codex milestone review: HANDOFF-CURRENT.md."

## Later milestones: roadmap, not permission to skip review

| Milestone | Acceptance evidence |
| --- | --- |
| Verified state and transport | A version-isolated diagnostic derives proved movement state, suppresses invalid/loading state, and exchanges bounded state with an owned peer in game-free tests. Unknown loading or identity rules stay explicit. |
| First visible remote actor | A separately reviewed 3663 actor-lifecycle design produces one correctly identified visible remote/proxy actor, moves it from verified data, and removes it cleanly. Native spawn/control calls require evidence first. |
| First two-player walk-around | Two legitimate game installations can see each other moving, with documented host/guest identity, loss/disconnect behavior, and clean cleanup. A local proxy demonstration alone does not meet this criterion. |
| Playable co-op | Animations, interactions/combat as scoped, loads, reconnects, and save handling pass repeatable runtime tests. Record unsupported features before a release. |

Prepare the next milestone's design and game-free work as a batch. Review is
required before a new gameplay-memory-writing, actor-spawning, packet-replay,
or gameplay-deployment experiment. This plan does not authorize those actions,
changing production offsets/guards, broadening old-build acceptance, bypassing
access controls, or launching multiple game instances with old-build scripts.

## Prompt to begin

> Execute docs/GROK-3663-EXECUTION-PLAN.md from approved checkpoint 3e950a7.
> Complete Packages A and B as one work package when the evidence allows.
> Handle your own implementation, self-review, regression fixes, and validation
> loops; ask me only for required in-game actions or concrete blockers.
> Use the reviewed diagnostic for live collection. Keep changed runtime code
> local for review before deployment. Finish with HANDOFF-CURRENT.md for one
> Codex milestone review. Do not move into actor spawning or gameplay writes.
