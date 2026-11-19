# R10 L3 runbook — first live test on 1.0.0.3732

Prepared 2026-11-19 by Claude. **Nothing in this file has been run.** Every
step that touches the game happens only with Shaugn present and after his
explicit go-ahead in chat.

## What this test asks

1. Does the 3732 port start cleanly in both copies? (A startup failure ends
   the sitting; report the captured fault site and stop.)
2. After the shared Enter, does the host's milestone send the guest its own
   2941, and does the guest then send 2789 and receive 2891, reaching control?
3. If both players reach control: mutual walking and turning, guest removal on
   exit, one reconnect attempt, host save reload (the R7 L3 criteria).

## Build and gates (sealed, checked)

| Item | Value |
| --- | --- |
| Game | 1.0.0.3732, SHA-256 `DFCF5C50…A9D5` |
| Branch | `gpt/3663-port` (sources sealed at commit with `r10_arm.py`) |
| Arming receipt | `build/r10/r10-l3-arming-receipt.json` (ASI `B507B79D…`, runner `A2F205AC…`) |
| Validation | `docs/probe-3663/port-evidence/r10/r10-l3-validation.json`: 121/121 tests, v1 smoke 78 states / 0 rejects |
| Host | `core_milestone=2` (asks the game for the guest's own 2941) |
| Both | `remote_actor_guard=1` (exact null-playback skip for other-player actors) |
| Guest | save-folder-only NT redirect, default deny; host has no sandbox |

The receipt is invalidated by any change to `src/`, `tests/`, `scripts/`,
`tools/` or `CMakeLists.txt`. Docs may change. Re-seal with a new prefix if
code changes.

**Budgets.** R9 actually used startup 2/5 and join 1/3 (A3 was a join, recorded
2026-11-19). Unless Shaugn approves fresh R10 budgets, this sitting has
**startup 3, join 2** left. A launch that ends before guest native login
counts as startup; after it, as join.

## Before the sitting (Claude, with Shaugn's go-ahead)

1. Confirm no `CrimsonDesert.exe` is running, Steam is open and signed in, and
   the installed executable is still 3732:

   ```powershell
   (Get-FileHash 'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe').Hash
   ```
2. Re-run the receipt check (all `True`) and the full suite if anything was
   rebuilt.
3. Prepare a fresh session. This takes and verifies the hashed save backups
   outside OneDrive and freezes the guest sandbox. It launches nothing.

   ```powershell
   .\scripts\launch-2inst-3663.ps1 -Stage Prepare -SessionRoot "$env:LOCALAPPDATA\CDMP-R10-L3-<yyyyMMdd>-A1" -SessionKind L3
   ```

## The sitting (Shaugn present)

4. **Host plan** (expires after two minutes, so open the runner right after):

   ```powershell
   .\scripts\launch-2inst-3663.ps1 -Stage Run -SessionRoot "$env:LOCALAPPDATA\CDMP-R10-L3-<yyyyMMdd>-A1" -ArmingReceipt .\build\r10\r10-l3-arming-receipt.json -LaunchRole host -SessionKind L3 -Present -DesktopLaunchPlan
   ```
   Open `r5_host_desktop.exe` from the session folder on the desktop. It
   re-checks hashes and starts the host. One shared 14-minute watchdog starts
   here.
5. **Shaugn: press Load on the host immediately**, pick an identifiable slot,
   and stand still. Claude waits for `3663 v1 host world ready` in
   `cdmp.host.log` and checks the host log for `3663 milestone active (send)`.
6. **Guest plan**, only with more than six minutes left on the watchdog:

   ```powershell
   .\scripts\launch-2inst-3663.ps1 -Stage Run -SessionRoot "$env:LOCALAPPDATA\CDMP-R10-L3-<yyyyMMdd>-A1" -ArmingReceipt .\build\r10\r10-l3-arming-receipt.json -LaunchRole guest -SessionKind L3 -Present -DesktopLaunchPlan
   ```
   Open `r5_guest_desktop.exe` from the session folder.
7. Arrange HOST left, GUEST right. Claude takes the window report; Shaugn
   confirms both windows are usable. **Shaugn starts recording (Win+Alt+R).**
8. Shaugn picks the **same slot** once in the guest.

## What to watch (Claude reads the logs live)

| Where | Expected line | Meaning |
| --- | --- | --- |
| host log | `3663 join Enter native guest-owned field … result 0x0` | staging Enter |
| host log | `3663 join Enter: outcome 1 … result 0x0 field key …` | shared Enter |
| host log | `3663 MoveFieldAck 2364 guest copy: owner_alias=aliased` | owner alias |
| host log | `3663 milestone trip 1: eligible \| returned` | **the fix fired** |
| host log | `3663 milestone trip 1: <reason>` | the fix refused; the reason names the check |
| guest ring | 2268, 2792, **2941**, then **2789** out, **2891** in | loading completes |
| guest log | `Local: pos …` that changes when Shaugn moves | guest in control |
| crash logs | `null_playback_controller … remote_controlled=1` | guard evidence |

Shaugn reports what each window shows: loading screen or world, the other
player visible or not, any freeze.

If both reach control: walk and turn in each window and confirm the other
sees it; quit the guest from its menu and confirm it disappears on the host;
try one reconnect; then reload a save on the host.

## Stop rules

Stop the trial and do not relaunch on: any crash or freeze, an activation
error, a wrong executable hash, failed save verification, a missing window
pair, the milestone refusing with a fault, or less than six minutes of
watchdog reserve for the guest. Close the surviving copy from its menu.

## After the sitting (always, even after a crash)

9. Verify and finish. Finish collects both logs, packet rings and crash files
   into `evidence`, verifies the real saves and backups, and restores the game
   folder:

   ```powershell
   .\scripts\launch-2inst-3663.ps1 -Stage Verify -SessionRoot "$env:LOCALAPPDATA\CDMP-R10-L3-<yyyyMMdd>-A1" -SessionKind L3
   ```
   ```powershell
   .\scripts\launch-2inst-3663.ps1 -Stage Finish -SessionRoot "$env:LOCALAPPDATA\CDMP-R10-L3-<yyyyMMdd>-A1" -SessionKind L3
   ```
10. Confirm the game folder has no `cdmp.asi`, `winmm.dll`, inis or lock, and
    re-check saves with `tools/port3732/savecheck.ps1` against the session
    manifest.
11. Record the attempt in a new `port-evidence/r10/attempts.json`, decode the
    guest ring with `tools/port3663/r8_guest_trace.py`, write the result
    section in `HANDOFF-CURRENT.md`, and push. (R9 A3 was lost because this
    step was skipped.)
