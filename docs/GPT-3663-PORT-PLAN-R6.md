# GPT plan, round 6: find out why character login is refused, fix it, pass L3

Written by Claude, 2026-11-17, after reviewing round 5 (branch `gpt/3663-port`,
final commit a2418f9). GPT owns all work start to finish; Claude reviews once
at the end. All earlier rules still apply unless changed below.

## Where round 5 ended

Both real game windows were visible side by side. The guest's native login
and title refresh passed. When the guest pressed Play, **our own identity
check refused the character-login request (opcode 3081)** before any join
insertion. EnterField was never reached, and no native fault occurred. The
refused payload and the failing predicate were **not logged**, so the cause
is unknown. The join budget was spent (3/3), so the round stopped.

Code reviewed: `identity3663.cpp` refuses 3081 unless all hold:
`bound(state, epoch, guest)`, `state.lobby_ready`, payload size exactly 13,
number == `kGuest` (0x7FFFFFFFFFFFF002), slot != 3 (CrossSave) and != 206.
Any one of these could be the cause, for example:
- the lobby edit never ran or never set `lobby_ready`, so the guest picked the
  native character number;
- the 3663 payload is not 13 bytes (layout changed since 2976/3576);
- the guest's Play/Continue chose slot 3 or 206 from its copied profile.

## Rule changes for round 6 (Claude's decision; they override earlier rounds)

1. **Diagnose before fixing.** Every refusal of 3081 must log, once per
   attempt: payload length, the 13 decoded bytes' fields (discriminator,
   number, slot), each predicate's true/false (`bound`, `lobby_ready`, size,
   number, slot), the lobby/status/phase edit counters and the state epoch.
   This is diagnostic logging only. **Do not relax any check** to make the
   login pass.
2. **Fresh budgets:** startup 5, join 3. A launch that fails before the guest
   process exists (for example an app-access denial) counts as startup, not
   join.
3. **Visual evidence by Shaugn.** The automated capture produced empty clips
   twice. For L3, Shaugn records the screen himself with Xbox Game Bar
   (Win+Alt+R) or a phone, and GPT asks him for the file's name and duration.
   GPT's own capture may still run, but it is not required for acceptance.
4. **Host quits normally.** Ask Shaugn to quit the host from the game menu
   before the watchdog deadline, unless the host is frozen.

## Steps

### Step 1 — Logging and offline analysis (game closed)

Add rule-1 logging with a unit test. Offline, check the 3663 handler/writer
for opcode 3081 to confirm the payload length and field order, and compare
with the 2976 `identity_fix` and Samuel's 3576 handling (reference only).
Check where `lobby_ready` is set and whether the guest's lobby edit ran in
round 5's guest log. Full suite and smoke. Commit "R6 step 1".

### Step 2 — One diagnostic L3 (live, Shaugn present)

Same setup as R5-L3-3 (desktop runner, windowed games side by side, guest
profile isolation, fresh backups). Goal: reproduce the refusal and capture
the rule-1 log line. If the join passes instead, continue straight to the
Step 4 acceptance in the same sitting. Commit "R6 step 2".

### Step 3 — Fix the actual cause (game closed)

Fix only what the Step 2 log points to, with a regression test. If it shows a
real payload layout change, update the decoder from 3663 handler evidence. If
it shows the lobby edit not running, fix that path. If it shows slot 3/206,
make the guest profile copy present an ordinary slot, or document why a
different slot rule is safe. Full suite and smoke. Commit "R6 step 3".

### Step 4 — L3 (live, Shaugn present)

Accept: the guest's character login is admitted; the guest appears in the
host's world; each window shows the other walking and turning; guest
disconnect removes it on the host; the real save directory is unchanged by
the guest (manifest); host saves load afterward; Shaugn's recording shows
both windows. Try reconnect and report the result. If the join itself fails
after login is admitted, log pin stage, admission and result, fix offline,
retry within budget. Commit "R6 step 4".

### Step 5 — L4 (live, second PC), only after L3 passes

As in R5 step 4. Commit "R6 step 5".

### Step 6 — Final package for Claude's review

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: commits since a2418f9,
the logged refusal cause, the fix, every attempt with outcome and log hash,
L3/L4 results, remaining limitations. Push, then tell Shaugn it is ready.

## Prompt to give GPT

> Save the attached file as docs/GPT-3663-PORT-PLAN-R6.md on branch
> gpt/3663-port, commit and push it. Then read AGENTS.md, CLAUDE.md and that
> file and execute it from Step 1 through Step 6 without stopping for review.
> The rule changes in that plan are approved by me, including the fresh
> budgets. Commit and push after each step. Ask me only to be present for
> live sessions (batched, with exact steps), to record the screen during L3,
> and for the second PC. If something is blocked, document it and continue
> with independent work. Finish with the Step 6 package.
