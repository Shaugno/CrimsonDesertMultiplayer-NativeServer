# GPT plan, round 3: switch on what is proven, then build the 3663 join

Written by Claude, 2026-11-16, after reviewing round 2 (branch `gpt/3663-port`,
final commit 69075db). GPT owns all work start to finish; Claude reviews once
at the end. This replaces GPT-3663-PORT-PLAN-R2.md for remaining work. All
rules in CLAUDE.md and AGENTS.md and the R2 live safety rules still apply.

## Where round 2 ended

- 329 of 395 core contracts adopted; whole-image AutoVT and BinDiff complete.
- Nothing armed. Blocked: A 3, B 5, C 37, D 21 (HANDOFF-CURRENT "Exact
  remaining rows").
- Three passive sessions passed safely. HP definition key 0 matched the
  visible 900/900; the loading trace followed title, park, placement,
  movement and a fast-travel jump; all saves unchanged.
- The 3663 EnterField ABI changed from four arguments to three with a new
  roster, so the legacy join detour cannot be redirected. This is the main
  remaining work.

## Rule changes for round 3 (Claude's decision; they override R2 where they differ)

1. **Tier A may arm as a read-only, HP-only reader.** Remove stamina from the
   3663 reachability (A027 retired by scope, not by proof). HP uses the
   existing definition-consensus helper (A026). Readiness (A028) is: not-ready
   on the park rule, on any chain change during a snapshot, or on HP
   definitions not ready. That is sufficient for a guarded read-only reader:
   its worst failure is a wrong or not-ready sample, never a write. L1
   decides whether it is good enough.
2. **Named RTTI beats a correlator conflict between sibling bodies.** When an
   exact RTTI-named vtable slot identifies a function, and an independent
   body-behaviour check agrees, adopt it even if BinDiff swapped two
   near-identical siblings (C009–C016, C012's conflict). Record the conflict.
3. **Passive traces may target a strong candidate entry before adoption.**
   A candidate that is a `.pdata` function start, the unique strong match,
   and consistent with its callers may get an entry/exit logging hook in a
   passive session, in order to settle adoption. Logging only: no calls, no
   writes, no argument changes. This removes the R2 circular dependency
   (trace needs adoption, adoption needs trace) for EnterField and similar.
4. **Evidence budget.** Commit scripts, summaries, result JSON and hashes.
   Do not commit new raw decompiler/correlator dumps or new game screenshots
   unless a reviewer could not check a claim without them; keep them local and
   hash them. Spend effort on making things work, not on re-proving adopted rows.

## Operating rules

Same as R2: branch `gpt/3663-port`, commit and push after each step, never
main, never force-push, no binaries/release. Do not stop for review; document
blockers and continue with independent work. Batch live sessions so Shaugn is
called as few times as possible. Re-hash the exe before every matching run and
live session (build drift rule). Live sessions keep the R2 safety rules:
save backup with hash manifest, deploy/remove, one written question, 15-minute
limit, crash = stop and fix offline.

## Step 1 — Arm Tier A and run L1

1. Publish the 3663 `PlayerLayout` from adopted rows, HP-only, with the native
   HP definition path. Set `kTierAComplete` true only for that layout. Keep
   every read guarded; keep the chain-change and park checks.
2. Guard tests: A armed on exact 3663 identity only; wrong size/timestamp/base
   and unknown builds still arm nothing; B/C/D stay false; 2976 unchanged.
3. Full suite + loopback smoke. Commit "R3 step 1".
4. **L1 (live):** load a save, walk, turn, fast travel, take a little damage.
   Accept when position/rotation track the character, HP changes with damage,
   and loading/fast travel give not-ready or coherent samples, with no
   fabricated pose. Record the log. If it fails, fix offline and rerun.

## Step 2 — Close Tier B and run L2

1. Resolve B052/B053 (Troc instance range), B065 (net loop pump mid-point),
   B075 (max packet size) and B078 (Troc scan skip) using the round-2
   correlator output plus code/xref checks, or a passive trace under rule 3.
2. Arm B (requires A). Tests, suite, loopback. Commit "R3 step 2".
3. **L2 (live, can share a sitting with L1):** host role against `cdmp_peer`
   over loopback. Accept on session handshake, movement stream in both
   directions in `cdmp.log`, clean shutdown.

## Step 3 — Build the 3663 join (Tier C)

1. Adopt the recipient-finder vtables and slot-1 functions (C009–C016) under
   rule 2.
2. Passive trace under rule 3 during one ordinary local load and one fast
   travel: entry/exit of AcquireField, candidate EnterField 0x2ECB130,
   the travel transition, and the lobby/status/login sites in the C list.
   Record arguments, list owners, results and call order.
3. Write `docs/probe-3663/R3-JOIN-ADAPTER.md`: how a guest is inserted into the
   host's field on 3663 with the three-argument EnterField and the new roster,
   including reference ownership and rollback on failure. Then implement a
   new 3663 join adapter (do not reuse the legacy four-argument detour).
   Unit-test its pure logic.
4. Resolve the remaining C rows (lobby, status, login manager, phases, travel,
   session ID, VecPush conflict, actor owner user) with the trace results.
5. Arm C (requires A and B). Commit "R3 step 3".

## Step 4 — Tier D, then two-player tests

1. Resolve D rows, prioritising guest save protection (title/save sites, slot
   refresh, call sites, return addresses, Windows lock vtable) before
   moving-state and reporter rows. Arm D (requires A–C).
2. **L3 (live):** two instances on one PC via `scripts/launch-2inst.ps1`, separate
   guest save, both backed up. Accept: each window shows the other character
   walking and turning; guest disconnect removes it on the host; reconnect
   works; host saves unchanged by manifest. Short screen capture kept local,
   hash committed.
3. **L4 (live):** two PCs over Steam per `docs/PRUEBA-DOS-PC.md`. Same acceptance
   plus one load screen or fast travel on each side.

If C or D cannot be completed, still deliver Steps 1–2 working and stop with
the exact blocker; do not arm a tier with unadopted rows.

## Step 5 — Final package for Claude's review

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: commits since 69075db,
which tiers are armed, test results, each live session (question, identity,
result, log hash), remaining blocked rows with the missing fact, and known
limitations. A loopback or simulated result is not two-player evidence.
Push, then tell Shaugn the branch is ready for review.

## Prompt to give GPT

> Read AGENTS.md, CLAUDE.md, and docs/GPT-3663-PORT-PLAN-R3.md. Execute it
> from Step 1 through Step 5 on branch gpt/3663-port without stopping for
> review. The rule changes in that plan are approved by me and override R2.
> Commit and push after each step. Ask me only to be present for live
> sessions (batched, with exact steps) and for the second PC. If something is
> blocked, document it and continue with independent work. Finish with the
> Step 5 package.
