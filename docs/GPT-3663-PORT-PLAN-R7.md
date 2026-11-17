# GPT plan, round 7: stop re-triggering activation, then pass L3

Written by Claude, 2026-11-17, after reviewing round 6 (branch `gpt/3663-port`,
final commit 9bbc3df). GPT owns all work start to finish; Claude reviews once
at the end. All earlier rules still apply unless changed below.

## Where round 6 ended

- The diagnostic worked. The character-login refusal had two causes: the
  3663 request is 17 bytes, not 13, and the guest sends the native "my own
  character" number, which the 3663 route never translated. The fix (accept
  only the observed 17-byte form; translate the number in an owned copy,
  then rerun the strict check) is correct and matches the legacy route.
  Claude reviewed it. 119/119 tests pass.
- The post-fix L3 never reached login: the **guest** instance hit anti-tamper
  error **88500006** ("purchase revalidation currently unavailable; wait 24
  hours"). The host started normally.

## Likely cause (Claude's hypothesis, to be checked, not assumed)

Since R5 the guest starts with **USERPROFILE and LOCALAPPDATA redirected** to a
copied profile. Anti-tamper keeps its activation data under the user's
profile. If the guest cannot see the normal activation data, each guest launch
may count as a **new activation**. About five guest launches in roughly 36
hours (R5-S2, R5-L3-1, R5-L3-3, R6-D1b, R6-L3-1) would plausibly reach a
daily activation limit. The host, with the normal profile, was unaffected.

## Rule changes for round 7 (Claude's decision; they override earlier rounds)

1. **No game launch before the official wait ends:** not before
   **2026-11-18 10:00 AM Eastern**. Offline work may proceed now.
2. **The guest must not redirect USERPROFILE or LOCALAPPDATA.** Use guest-only
   isolation that redirects **only the save directory**, so everything else,
   including anti-tamper data, stays at its normal location. The existing,
   fixture-tested NT fallback (`CDMP_SAVE_ISOLATION` NT mode) is the starting
   point; it runs **in the guest only**, never in the host. The R4 host crash
   causes (Steam IPC mappings, hook teardown, small stacks) must stay covered
   by its tests. Do not copy, delete, move or edit any anti-tamper or
   activation file, and do not patch the executable.
3. **Minimise launches.** Plan every live sitting so each guest launch is used
   for a real test. No exploratory guest launches.
4. **Budget accounting:** fresh budgets of startup 5 and join 3. A launch that
   ends before native guest login (activation error, access denial, crash at
   startup) counts as **startup**, not join.

## Steps

### Step 1 — Guest-only save redirect (game closed, now)

Switch the 3663 guest to the NT save-directory redirect with no profile
override. Fixture tests: guest mutations under the real save path change zero
bytes there; reads and writes land in the guest copy; the profile and
LOCALAPPDATA seen by the guest are unchanged; Steam/pagefile IPC, pipes,
128 KB stacks and teardown still pass. Host has no sandbox. Full suite and
smoke. Commit "R7 step 1".

### Step 2 — Guest-only startup check (live, after the wait)

One guest launch alone to the title screen and Quit. Accept: no activation
error, isolation verified in the log, real save directory unchanged by
manifest. If the activation error returns, stop all live work and write the
blocker package (Step 5) rather than retry.

### Step 3 — L3 (live, same sitting if Step 2 passes)

As in R6 step 4: desktop runner, windowed games side by side, Shaugn records
with Win+Alt+R or a phone, host quits from the menu. Accept: guest character
login admitted; guest appears in the host's world; each window shows the other
walking and turning; guest disconnect removes it on the host; real saves
unchanged by the guest; host saves load afterward. Try reconnect and report
the result. A failed join logs pin stage, admission and result; fix offline
and retry within budget.

### Step 4 — L4 (second PC), only after L3 passes

As before. The second PC uses its own Steam account and normal profile, and
the guest there also uses the save-directory redirect.

### Step 5 — Final package for Claude's review

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: commits since 9bbc3df,
the isolation change and tests, whether the activation error recurred, every
attempt with outcome and log hash, L3/L4 results, limitations. Push, then
tell Shaugn it is ready.

## Prompt to give GPT

> Save the attached file as docs/GPT-3663-PORT-PLAN-R7.md on branch
> gpt/3663-port, commit and push it. Then read AGENTS.md, CLAUDE.md and that
> file and execute it from Step 1 through Step 5 without stopping for review.
> The rule changes are approved by me. Do Step 1 now; do not launch the game
> before 2026-11-18 10:00 AM Eastern. Commit and push after each step. Ask me
> only to be present for live sessions (batched, with exact steps), to record
> the screen during L3, and for the second PC. Finish with the Step 5 package.
