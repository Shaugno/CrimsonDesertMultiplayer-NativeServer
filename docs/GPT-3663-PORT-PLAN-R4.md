# GPT plan, round 4: first two-player session on 3663

Written by Claude, 2026-11-16, after reviewing round 3 (branch `gpt/3663-port`,
final commit 0f11f6e). GPT owns all work start to finish; Claude reviews once
at the end. This replaces GPT-3663-PORT-PLAN-R3.md for remaining work. All
rules in CLAUDE.md and AGENTS.md, the R2 live safety rules and the R3 rule
changes still apply unless changed below.

## Where round 3 ended

- Tier A (HP-only reader) and Tier B (native host tunnel, loopback test
  route) are armed. L1 and L2 passed live. Default 3663 install runs only
  the reader; B needs explicit test configuration.
- Join trace captured AcquireField/EnterField across two fields.
  R3-JOIN-ADAPTER.md defines the three-argument transaction; its pure policy
  is tested; the native insertion/reference/rollback backend is absent.
- Remaining: C 20 rows, D 21 rows (R3-BLOCKERS.md). Guest role on 3663 is
  denied before the save redirect.

## Goal of this round

**L3:** two game instances on one PC. The guest joins the host's world,
each window shows the other character walking and turning, guest disconnect
removes the guest character on the host, and host saves are unchanged by
manifest. Then **L4** on two PCs over Steam. Reconnect is wanted but may be
reported as a known limitation if its rows stay blocked.

## Rule changes for round 4 (Claude's decision; they override earlier rounds)

1. **First-milestone scope.** For 3663 v1 the guest may walk, turn, run and
   stay in the host's field. **Guest fast travel and time-of-day publication
   are out of scope:** add the guest's travel/fast-travel requests to the 3663
   guest control filter so the host refuses them, document it, and retire
   C110–C115 by scope (as stamina was in R3). Mounts, combat sync and all
   other optional modules stay unreachable.
2. **Guest save protection v1 may be file-level.** If `save_hooks` (Windows
   file APIs, not game RVAs) can sandbox every guest-process write, rename,
   delete and create under the save directory into a guest-only directory,
   and point guest reads there, use that instead of per-callsite save
   classification. Then D020–D023, D063–D067, D100–D104 and D130 are
   retired by scope for v1. Requirements:
   - Default-deny: any guest file operation under the real save directory
     that is not redirected is refused, never passed through.
   - The guest sandbox starts as a copy of a backup, never a link.
   - Offline test with a fixture directory: a guest-role process performing
     create/write/rename/delete under the save path changes zero bytes in
     the real directory (hash manifest) and all effects land in the sandbox.
   - The guest role on 3663 is allowed only when this sandbox is active and
     verified at startup; otherwise the guest process exits before the game
     reaches the title screen, as today.
   If file-level protection cannot meet these requirements, say why and
   fall back to resolving the D save rows.
3. **Disconnect removal is the only lifecycle feature required for L3.**
   Prioritise the rows it needs (D014/D015 WindowsLock slots, D030, D044/D045
   moving flags) and the report guard D115 only if removal reaches it.
   Reconnect follows if those rows suffice; otherwise it is a documented
   limitation.
4. **Passive traces at call sites and mid-points are allowed** under the R3
   rule-3 conditions (verified function context, logging only), to settle
   identity/lobby/login sites (C070–C081, C001, C103, C122) and join
   backend facts (C047 VecPush, C066 character class, C144 owner user).
5. **Join attempts are live experiments with a crash budget.** A join attempt
   that crashes or hangs either game: stop, collect logs, fix offline, rerun
   the full suite, then retry. After three failed join attempts with
   different fixes, stop live work and write the blocker package.

## Operating rules

Same as R3: branch `gpt/3663-port`, commit and push after each step, never
main, never force-push, no binaries/release, evidence budget (scripts,
summaries, result JSON, hashes; no new raw dumps or game screenshots). Do not
stop for review; document blockers and continue with independent work. Batch
live sessions. Re-hash the exe before matching and live sessions. Live safety
rules unchanged: verified save backups outside OneDrive, deploy/remove, one
written question, 15-minute limit, crash = stop.

## Step 1 — Guest save sandbox (game closed)

Implement rule 2. Add the offline fixture test and a startup self-check that
logs the sandbox path and refuses guest startup if it is not active. Extend
`scripts/launch-2inst.ps1` (or a 3663 sibling) to create the guest sandbox
from a fresh verified backup and to verify host saves by manifest after exit.
Full suite. Commit "R4 step 1".

## Step 2 — Guest scope filter (game closed)

Implement rule 1: identify the guest travel/fast-travel and time requests on
3663 from the wire map and handler code, add them to the 3663 guest control
filter, unit-test refusal, and retire C110–C115 by scope with a written
reason. Commit "R4 step 2".

## Step 3 — Native join backend (passive traces allowed)

1. Settle C047, C066, C144 and the identity/lobby/login rows with code
   evidence and rule-4 traces in one batched passive sitting.
2. Implement the native backend for R3-JOIN-ADAPTER.md: three-argument
   EnterField insertion, reference acquisition/release, lock order and
   rollback on every failure path. Keep the pure policy as the gate.
3. Implement the identity fix for the guest login on 3663 using the settled
   sites; new per-build patches only, no legacy bytes reused.
4. Unit tests for every pure decision; full suite; synthetic smoke. Commit
   "R4 step 3".

## Step 4 — Disconnect removal and arming (game closed)

Implement rule 3. Arm C and the v1 D scope (requires A and B), with guest role
on 3663 allowed only under the Step 1 sandbox. Guard tests: exact identity
only; every unreachable optional module still unreachable; 2976 unchanged.
Commit "R4 step 4".

## Step 5 — L3 then L4 (live, Shaugn present)

- **L3:** `launch-2inst.ps1` with separate sandboxed guest saves, both backed up.
  Accept: guest joins; each window shows the other walking and turning;
  guest disconnect removes it on the host; host saves unchanged by manifest;
  the real save directory unchanged by the guest. Try reconnect and report
  the result either way. Keep a short screen capture local; commit its hash.
- **L4:** after L3 passes, two PCs over Steam per `docs/PRUEBA-DOS-PC.md`, guest
  kit with the sandbox on Shaugn's second PC. Same acceptance (no fast
  travel by the guest; the host may load or fast travel once).

## Step 6 — Final package for Claude's review

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: commits since 0f11f6e,
armed tiers, scope retirements with reasons, test results, each live session
(question, identity, result, log hash), crash/attempt record, remaining rows
and known limitations. A loopback or simulated result is not two-player
evidence. Push, then tell Shaugn the branch is ready for review.

## Prompt to give GPT

> Save the attached file as docs/GPT-3663-PORT-PLAN-R4.md on branch
> gpt/3663-port, commit and push it. Then read AGENTS.md, CLAUDE.md and that
> file and execute it from Step 1 through Step 6 without stopping for review.
> The rule changes in that plan are approved by me and override earlier
> rounds. Commit and push after each step. Ask me only to be present for live
> sessions (batched, with exact steps) and for the second PC. If something is
> blocked, document it and continue with independent work. Finish with the
> Step 6 package.
