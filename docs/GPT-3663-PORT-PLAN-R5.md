# GPT plan, round 5: get L3 to actually start

Written by Claude, 2026-11-16, after reviewing round 4 (branch `gpt/3663-port`,
final commit 756fcd5). GPT owns all work start to finish; Claude reviews once
at the end. This replaces GPT-3663-PORT-PLAN-R4.md for remaining work. All
earlier rules still apply unless changed below.

## Why round 4 stopped

All A/B/C/D code exists behind the explicit `[session2] core_v1=1` route.
All three L3 launches failed **in the host, before title, before any guest
or join ran**, and every failure came from the file sandbox running inside
the host process:

1. Steam IPC mapping refused by the sandbox (`MapViewOfFile` error 5).
2. Crash in `NtProtectVirtualMemory` during sandbox hook removal.
3. Stack exhaustion in a sandbox callback on a 128 KB game IO thread.

GPT fixed each one offline, but the last fix was never tried live because
the round's attempt budget was used up. The join backend itself is untested.

The root cause is partly the R4 plan: it said "host saves unchanged by
manifest", which pushed the sandbox into the host. The host does not need it.

## Rule changes for round 5 (Claude's decision; they override earlier rounds)

1. **No file sandbox in the host process.** The host plays on its real saves,
   backed up before every session as today. Host saves may change through
   normal autosave. New acceptance: **the guest changes zero bytes in the
   real save directory (manifest), and the host's saves still load after
   the session.** Remove the host-side sandbox requirement from the v1 route;
   keep it mandatory for the guest.
2. **Prefer zero-hook guest isolation if it works.** Before relying on the
   ntdll hooks, test offline whether the game resolves its save path from an
   overridable environment (`LOCALAPPDATA`, `USERPROFILE`, or the known-folder
   API it actually calls; find out from code). If an override reliably points
   the guest at a sandbox copy, use it plus a startup check that the
   resolved path is the sandbox, and keep the hooks only as a default-deny
   backstop if they can be made safe. If no override works, keep the R4
   sandbox in the guest only, with its three fixes.
3. **Separate budgets.** Startup attempts (launches that fail before a join is
   attempted) and join attempts are counted separately. Startup: at most five
   in this round, and each retry needs a fix reproduced offline first. Join:
   at most three, as in R4. Previous rounds' counts reset.
4. **Single-process startup checks come before L3.** They are short live
   sessions (≤10 minutes) in one batched sitting where possible.

## Operating rules

Same as R4: branch `gpt/3663-port`, commit and push after each step, never
main, never force-push, no binaries/release, evidence budget. Do not stop for
review; document blockers and continue with independent work. Batch live
sessions. Re-hash the exe first. Live safety rules unchanged: verified
backups outside OneDrive, deploy/remove, one written question, time limit,
crash = stop and fix offline.

## Step 1 — Rework isolation (game closed)

Implement rules 1 and 2. Update `launch-2inst` (3663) so only the guest gets
isolation, and the guest refuses to start unless isolation is verified.
Fixture tests: the guest's create/write/rename/delete under the real save
path change zero bytes there; Steam IPC/pagefile mappings, named pipes and
small-stack threads still work under the guest's isolation. Full suite and
smoke. Commit "R5 step 1".

## Step 2 — Startup checks (live, Shaugn present, one sitting)

- **S1, host only:** v1 host route on, no sandbox. Reach title, load a save,
  stand in the world for a minute, quit normally. Accept: no crash, native
  descriptors ready, v1 host route active in the log, clean exit, saves load.
- **S2, guest only:** guest role with isolation. Reach title (it cannot join
  yet), quit. Accept: no crash, isolation verified in the log, real save
  directory unchanged by manifest.

Any failure: stop, reproduce offline, fix, rerun the suite, retry within the
startup budget. Commit results "R5 step 2".

## Step 3 — L3 (live)

Two instances on one PC. Accept: guest joins; each window shows the other
walking and turning; guest disconnect removes it on the host; the real save
directory is unchanged by the guest (manifest); host saves load afterward.
Try reconnect and report the result (R4 expects it unsupported). Keep a short
screen capture local and commit its hash. Join failures follow the R4 crash
rule and the join budget. Commit "R5 step 3".

## Step 4 — L4 (live, second PC)

Only after L3 passes. Two PCs over Steam per `docs/PRUEBA-DOS-PC.md`, guest
kit with isolation on Shaugn's second PC. Same acceptance; the guest does not
fast travel; the host may load or fast travel once. Commit "R5 step 4".

## Step 5 — Final package for Claude's review

New top section in `docs/probe-3663/HANDOFF-CURRENT.md`: commits since 756fcd5,
the isolation method chosen and its tests, every startup and join attempt
with outcome and log hash, L3/L4 results, remaining limitations. Push, then
tell Shaugn the branch is ready for review.

## Prompt to give GPT

> Save the attached file as docs/GPT-3663-PORT-PLAN-R5.md on branch
> gpt/3663-port, commit and push it. Then read AGENTS.md, CLAUDE.md and that
> file and execute it from Step 1 through Step 5 without stopping for review.
> The rule changes in that plan are approved by me and override earlier
> rounds, including the reset attempt budgets. Commit and push after each
> step. Ask me only to be present for live sessions (batched, with exact
> steps) and for the second PC. If something is blocked, document it and
> continue with independent work. Finish with the Step 5 package.
