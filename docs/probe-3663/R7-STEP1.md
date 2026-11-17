# R7 Step 1 — guest-only save-directory isolation

User-approved scope: `docs/GPT-3663-PORT-PLAN-R7.md`; baseline `9bbc3df`.
The plan was saved and pushed as `16d271d`. This is the offline Step 1
checkpoint, not the Step 5 completion package. Steps 2–5 remain pending.

## Problem, expected behavior and reproduction

The R6 guest failed with activation error 88500006 before native login.
Profile redirection causing additional activations is a hypothesis, not a
measured cause. R7 removes the production profile override without altering
activation data or the game executable. No R7 game launch is authorized before
**2026-11-18 10:00 AM Eastern / 15:00 UTC**.

An owned guest fixture opening its apparent real save path must read the
guest copy. Writes, mappings, creates, renames and deletes must change zero
bytes in the source/frozen backup and leave effects only in the guest copy.
USERPROFILE, LOCALAPPDATA and the native LocalAppData resolver must remain
unchanged. Ordinary owned data outside the save directory stays in its normal
location. Host startup must retain the existing no-sandbox route.

## Implementation

- `launch-2inst-3663.ps1` defaults to NT, refuses profile Prepare/Run and never
  creates a guest profile. Both roles inherit the normal profile variables.
  Historical Verify/Finish can still explicitly identify old profile sessions;
  old evidence is preserved. New sessions use only `guest-sandbox`.
- The production guest environment installer accepts NT only and requires the
  native CSIDL_LOCAL_APPDATA save root to equal the launcher's verified source.
  It logs unchanged profile variables and save-only redirection after install.
  Existing exact-build/guest-only gates and all NT file protections remain.
- The desktop runner rejects profile overrides, checks that its inherited
  profile matches the prepared context, and records NT/normal inheritance in
  the launch receipt. Host isolation stays `none`. Pure owned child fixtures
  prove profile/native resolver inheritance. Historical fake-profile fixture
  coverage remains separate from the rejected production route.
- The launcher refuses production Run before the deadline, ahead of file or
  executable access. The desktop runner independently checks the deadline
  before reading a plan and immediately before process creation. Its pure
  boundary test checks refusal just before, acceptance at/after the instant.
  No CLI/environment bypass is provided. Presence, expiry, source/binary seal,
  exact game hash, pinned identities and common watchdog checks remain.
- `r7_validation.py` produces separate version-3/round-7 NT arming receipts
  from passing full-suite and synthetic-smoke evidence. It refuses to overwrite
  evidence and never deploys or launches the game. Old receipts cannot arm R7.
  The R6 character-login fix, field adapter, native addresses, network policy
  and GPL/attribution are unchanged.

## Validation and evidence

Baseline `scripts/build.ps1`: **119/119**, 106.18 seconds.
The first changed suite passed 118 tests; the new launcher test stopped on
PowerShell 5's merged native stderr while capturing the expected time refusal.
The refusal worked. The test now captures that expected error under Continue
and explicitly checks the nonzero exit/message; its focused rerun passed.
The first suite/log are preserved as `build/r7/step1-first-*`.

Final suite, smoke, binary hashes and local evidence hashes are sealed in
`port-evidence/r7/step1-validation.json`. The full CTest log retains the owned
NT mutation tests, unchanged source/backup manifests, normal profile/native
resolver, unrelated-file location, Steam-style MAP-only named pagefile IPC,
protected-file mapping refusal, pipes and pipe roots, 128 KB thread stacks,
ordinary CRT hook teardown, and host save-delta/cleanup tests. No real SteamAPI
or activation is needed by those fixtures. Synthetic smoke runs only
`cdmp_host`/`cdmp_peer`, requires compiled tier metadata 0xF, and cleanly unloads.

Final `scripts/build.ps1`: **119/119**, **110.93 seconds**. Eight-second
synthetic smoke: **78 remote states, zero rejects, clean unload**. Final audit:
zero game processes and all six controlled installation files absent. The
stored R7 plan exactly matches the Downloads file by SHA-256. Seal generated
with `tools/port3663/r7_validation.py --build-dir <validated r2-build>`;
local receipt: `build/r7/step1-arming-receipt.json`.

These results establish game-free behavior only. They do not prove activation
recovery, native guest login/admission, runtime NT compatibility, L3 or L4.
No activation file was copied/deleted/moved/edited. No game executable was
patched. No game launch, attachment or controlled installation occurred in R7
Step 1; no fresh live attempt was consumed. Earlier raw evidence is untouched.

## Next live sitting — after the time gate

Preflight on **2026-11-17 10:48–10:49 Eastern**: user present, but the UTC
clock and Windows local clock both confirm the gate remains closed until
the following day. Reverified all 680 source-file seals, ASI/desktop-runner
hashes, and every indexed Step 1 evidence/arming hash. No game process or
controlled installation was present. No plan/timer, deployment or launch was
started; startup remains 0/5, join 0/3. Steps 2–5 stay pending. Computer Use is
authorized by the user if needed for the later live setup.

Use a fresh R7 session outside OneDrive, fresh verified save backups, the
sealed R7 receipt and visible desktop runner. Recheck game hash/source seal.
Ask once for presence and the following batched actions; do not arm a timer
until shortcuts are visible and the user is ready.

1. Guest-only Step 2: launch only on signal, reach title, choose Quit to Desktop
   without loading a character. Verify normal exit, NT isolation log and zero
   real-save manifest delta. If activation error returns, stop all live work
   and write the blocker Step 5 package; no retry.
2. If Step 2 passes, cleanup/fresh backups, then L3 host load/walk/turn. Launch
   guest, keep it at menu, verify both actual window/process identities and
   host-left/guest-right positions plus the user's visibility confirmation.
   Only then signal guest character selection once.
3. After native admission, user records both views (Win+Alt+R or phone), walks
   and turns in each, checks mutual actors and supplies recording name/duration.
   Guest Quit to Title, check removal, try reconnect once and record its result,
   then guest Quit to Desktop. Take save checkpoint. Host Quit to Title,
   reload its save, and Quit to Desktop normally before the fixed deadline.
4. A failed join records pin stage, admission and result; fix only the measured
   failure offline, validate, then retry within R7's caps. An attempt ending
   before native guest login counts as startup, not join. Fresh limits are
   startup 5 / join 3; the initial ledger is `port-evidence/r7/attempts.json`.
5. L4 requires passing L3, a second PC/account, normal profile and NT guest
   save redirect. Request that setup only once the prerequisite passes.

Keep startup sessions within ten minutes and L3 within the shared fourteen
minutes. Stop on crash/hang, preserve own-PID logs/receipts/hashes, verify
cleanup and frozen backups, and retain legitimate host save changes. Commit
and push after each remaining step. The final Step 5 package must supersede
this checkpoint and enumerate every attempt and commits since `9bbc3df`.
