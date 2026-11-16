# GPT plan, round 2: finish the 3663 port in tiers

Written by Claude, 2026-11-15, after reviewing round 1 (branch `gpt/3663-port`,
final commit 2c80c21). GPT owns all work from start to finish; Claude reviews
once at the end. This replaces GPT-3663-PORT-PLAN.md for all remaining work.
All engineering rules in CLAUDE.md and AGENTS.md still apply.

## What round 1 established (keep it)

- Phase 0 fixes are correct and verified. The 3663 identity is recognized and
  arms nothing. The 2976 reference exe is downloaded and hash-verified.
- 17 values are adopted in `src/mod/game/build_3663.h`. One real layout change
  was found (TLS heap flag 0x1FD -> 0x1FE), so nothing may be copied unchanged.
- Blocker groups from HANDOFF-CURRENT.md: field sharing (AcquireField
  0x2B07790 / EnterField 0x2BC04F0 unmatched), identity/status hooks, player
  chain and cleanup, wire layouts, guest saves.

## Why round 1 stalled, and what changes

1. **The inventory was never cut to the core.** It listed 1292 named values
   plus 8261 bare numbers across every module. Round 2 works from a short,
   tiered list (Step 1) and ignores everything outside it.
2. **Whole-program matching was skipped.** Round 1 used targeted analysis and
   a custom fingerprint pass. Round 2 runs two standard whole-program
   correlators and uses agreement between them as the main evidence (Step 2).
3. **Nothing could be armed until everything was matched.** Round 2 arms each
   tier separately behind its own flag, so the read-only player reader can be
   live-tested as soon as its handful of values is matched (Step 4).
4. **Static analysis alone could not resolve some roles.** Round 2 allows
   short, passive, read-only live traces to settle a specific blocker, under
   the safety rules below (Step 5).

## Operating rules

- **Branch:** continue on `gpt/3663-port`. Commit at the end of every step
  and tier, push after each commit. Never push to main, never force-push,
  never publish binaries or a release.
- **Do not stop for review.** Self-review, test, fix, continue. A blocked row
  or tier is written down (exact missing fact, what was tried) and work moves
  to the next independent item. Stop entirely only when nothing independent
  remains, then write the final package (Step 7).
- **User touchpoints, batched.** Ask Shaugn only for: (a) being at the PC for a
  live session, requested once per session with the exact question, steps and
  expected duration; (b) the second PC for the two-PC test. Group live
  sessions so he is called as few times as possible.
- **Build drift:** re-hash the installed exe before every matching run and every
  live session. If Steam updated it, stop live work, record the new identity,
  and redo matching for the new build. Never broaden a guard.
- **Live sessions are authorized** for the tier tests and passive traces below,
  under the live safety rules. Everything else keeps the game closed.

### Live safety rules (every live session)

- Back up `%LOCALAPPDATA%\Pearl Abyss\CD\save` with a hash manifest first;
  verify the manifest after.
- Deploy with `scripts/deploy.ps1`, remove with `-Remove` after the session.
- One written question per session, an expected observation, a time limit
  (at most 15 minutes) and a stop condition, recorded before launch.
- Only hooks at verified 3663 function entries (or `safetyhook::create_mid` at
  a verified point), only guarded reads. Passive traces log; they never write
  game memory, call native functions or replay packets.
- On a crash: stop, collect `cdmp.log` and the crash context, fix offline, rerun
  the full suite, then try again. Never retry blindly. Crashes are reported to
  Pearl Abyss automatically.

## Step 1 — Build the tiered core list (game closed)

From round 1's inventory, keep only values reachable from the option-D core
path. Classify each as a **game value** (RVA, vtable, struct offset, signature,
Troc ID, payload size) or a **policy value** (timeouts, limits, protocol
constants). Policy values are not ported; they stay as they are.

Assign every game value to one tier. The starting point, by source file:

| Tier | Purpose | Main sources |
|---|---|---|
| A | Read-only local player: position, rotation, HP | `game/offsets.h` (player chain, camera optional), `game/actor_lookup.cpp`, `game/rtti.cpp` |
| B | Internal session tunnel: send/receive frames | `session2/offsets_s2.h` session/queue/Troc rows, `server.cpp` receive, `common/troc_frame.h` (verify, not port) |
| C | Guest joins host world: login, field sharing, identity | `session2/field_share.*`, `session2/identity_fix.*`, `server.cpp`, `guest.cpp`, related `offsets_s2.h` rows |
| D | Disconnect, reconnect, removal, guest save protection | `session2/guest_lifecycle.*`, `session2/save_hooks.cpp`, `guest_save_*`, `rejoin_logic.h` |

Exclude every module listed "disabled on 3663" in PHASE2-SCOPE.md. Confirm by
following calls from the session2 entry points; a value no core code path reads
is out of scope even if it sits in a core header.

Output `docs/probe-3663/R2-CORE-LIST.md`: one row per game value with tier,
symbol, 2976 value, file:line, use, and the 17 already adopted marked as done.
Expected size is a few hundred rows; if it is over 500, the cut is too loose.
Commit and push "R2 step 1".

## Step 2 — Whole-program matching (game closed)

Run two independent correlators over the 2976 and 3663 executables:

1. **Ghidra Version Tracking**, headless, with the automatic correlators
   (exact bytes/instructions/mnemonics, symbol/RTTI names, data references,
   combined function and data reference, implied matches). Script it so it is
   reproducible (`tools/port3663/`).
2. **BinDiff** on BinExport output from Ghidra for both executables.

Feed both with the RTTI names round 1 already extracted. Denuvo-mutated
`.xtls` functions will not correlate by bytes; match them through their
callers, callees and data references, and say so per row.

Adoption rule for each core row:
- **Function/global RVA:** both correlators agree, or one agrees plus an
  independent check (unique signature, RTTI vtable slot, string or data xref,
  matching call neighbours).
- **Struct offset:** confirmed from 3663 code that uses it the same way as the
  2976 user of that offset. Never assumed unchanged.
- **Troc ID / payload size:** confirmed from the 3663 handler or writer code.
- Anything weaker stays unadopted with its missing check written down.

Record results in `docs/probe-3663/R2-MATCHES.md` (tier, row, 3663 value, method,
second check, confidence). Keep large raw dumps out of the main docs: commit
the scripts, a summary, and SHA-256 hashes; store bulky correlator output under
`docs/probe-3663/port-evidence/r2/` only if it is under 5 MB per file,
otherwise keep it local and record its hash. Commit and push "R2 step 2".

## Step 3 — Field sharing first (game closed)

Field sharing is the first core blocker. Start from round 1's lead: trace
the 3663 `LoginCharacterToFieldServerReq` handler at 0x2C6AE10 to its field
acquisition and entry calls. Match those to 2976 AcquireField 0x2B07790 and
EnterField 0x2BC04F0 using the Step 2 correlator results, the two locks, the
field user list, counts, reference ownership and rollback. If static evidence
is still insufficient, prepare a Step 5 passive trace for exactly this question.

## Step 4 — Arm tier by tier

Add a per-tier permission to the startup decision for `Build3663`:
`tier_a_reads`, `tier_b_tunnel`, `tier_c_join`, `tier_d_lifecycle`. A tier may be
enabled only when **every** game value it uses is adopted. Each tier's flag
also requires all lower tiers. Disabled modules stay unreachable on 3663 by
both config default and guard. 2976 behavior stays unchanged; 3576 stays
recognized and unarmed.

Route the core modules through a per-build address table or accessor instead of
copying modules. Every 3663 value lives in `build_3663.h` with its 2976
counterpart and its R2-MATCHES row. Bump `kVersion` if the wire format changes;
keep the Steam-friends restriction and executable filter.

For each tier, before any live use: unit tests for the guard (each tier on and
off, wrong size, unknown build), the full CTest suite, the `cdmp_host` +
`cdmp_peer` loopback smoke test from `build\e2e\`, and a self-review against
AGENTS.md "Review priorities". Commit and push per tier.

## Step 5 — Live sessions (Shaugn present, batched)

Run these in order; each one only once its prerequisite is met. Group any
that are ready at the same time into one sitting.

- **L1, Tier A read-only:** load a save, walk, turn, fast travel. Accept when the
  3663 reader's position, rotation and HP track the character and loading
  produces an explicit not-ready state. No hooks.
- **Passive traces (as needed):** to settle a specific blocked role (for example
  which function adds a user to a field during login), hook verified 3663
  function entries, log arguments and call order, change nothing. One question
  per trace. Use the existing reviewed capture tooling where it fits.
- **L2, Tiers A–B against `cdmp_peer`:** session handshake and movement stream
  appear in `cdmp.log`, clean shutdown.
- **L3, two instances on one PC (Tiers A–D):** use `scripts/launch-2inst.ps1`,
  separate guest saves, back up both. Accept when each window shows the other
  character walking and turning, guest disconnect removes it on the host,
  reconnect works, and host saves are unchanged by manifest. Record a short
  screen capture.
- **L4, two PCs over Steam:** follow `docs/PRUEBA-DOS-PC.md` with the guest kit on
  Shaugn's second PC. Same acceptance as L3 plus one load screen or fast travel
  on each side.

Commit evidence (logs, hashes, capture; never saves or credentials) after each
session and push.

## Step 6 — Small cleanups

- `src/coop3663/feed.h`: the comment still says begin_session "fences" the old
  generation; update it to describe the per-peer highest-generation rule.
- Keep `src/coop3663` as a fallback only; do not expand it unless a tier cannot
  be ported and that decision is written down.

## Step 7 — Final package for Claude's review

Add a new top section to `docs/probe-3663/HANDOFF-CURRENT.md`:

- branch, final commit, commit list since 2c80c21; build config and binary hashes;
- R2-CORE-LIST and R2-MATCHES summary per tier: adopted, blocked, and every
  blocked row with its missing evidence;
- which tiers are armed, with their tests and exact results (failures, fixes,
  reruns included);
- each live session: question, identity, result, log and capture hashes;
- what works, what stays disabled on 3663, known limitations.

If the goal is not reached, write it as a blocker package: furthest tier armed,
furthest live session passed, the exact missing fact, and the smallest next
experiment. A loopback or simulated result is not two-player evidence. Push,
then tell Shaugn the branch is ready for review.

## Prompt to give GPT

> Read AGENTS.md, CLAUDE.md, and docs/GPT-3663-PORT-PLAN-R2.md. Execute that
> plan from Step 1 through Step 7 on branch gpt/3663-port without stopping for
> review. Commit and push after each step and tier. Ask me only to be present
> for live sessions (batched, with exact steps) and for the second PC. If
> something is blocked, document it and continue with independent work.
> Finish with the Step 7 package.
