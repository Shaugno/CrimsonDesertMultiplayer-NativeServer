# R10 Step 4 — review of the R9 A1 guest startup crash

Offline review of session `CDMP-R9-L3-20261118-A1` (build 1.0.0.3663, mod
build `830923F5 + local changes`). No game was launched.

## What happened

- Guest PID 29220 started 19:20:06 Eastern; it exited `0xC0000005` at
  19:20:46.6, about 40 s later, at the title screen, before native login.
- Guest log (`evidence/cdmp.guest.log`, last line 19:20:41.9): sandbox
  verified, achievement and character guards installed, v1 guest active,
  tunnel connected, 35 control packets received, "no controlled actor (not
  in world)". No error or fault line.
- The crash capture was installed but not yet armed (R9 armed it at character
  login). `guest-crash-29220.log` holds only its header. No minidump.
- No Windows Application error event between 19:19 and 19:23.
- Host log: world ready, link connected, nothing unusual at 19:20:40–19:20:47.
  The host quit normally (code 0).

## Ruled out

- **Sandbox HID refusals.** From 19:20:19 the guest's input code opened HID
  devices (keyboards, mouse, Intel HID) about 230 times a second, and the
  default-deny sandbox refused them (5,337 refusals by 19:20:41). Every R8
  guest log shows the same 17 sampled HID names and higher totals (38,227–
  90,663 refusals), and those guests ran for minutes and reached login. Normal
  for this sandbox, not the cause.
- **R9 code.** Guest-reachable changes since the last R8 build
  (`6d47b99..3728807`):
  - `sample_loading()` runs on every pump but returns until the second
    MoveFieldAck, so it is inert at the title screen.
  - `crash3663` only adds a host packet ring.
  - The `v1_3663` additions are host-only branches.
  - The ServerNet null check in `server.cpp` is host-only.
  Nothing new ran in the guest before login.
- **Window report timing.** `window-guest.json` lists no windows, but every
  R8 guest report does too; the report is written at process exit.

## Conclusion

The cause is not established, and the evidence does not point at mod code.
R8's guest launches, with the same pre-login code, reached login.

## What changed since, and what the next launch will capture

- `33dae67` arms the bounded guest crash capture 25 s into startup, before
  login. A repeat at around 40 s would now record the fault site,
  module-relative stack and registers.
- On 3732, every guest hook and native site is byte-checked before install
  (P3). A mismatch refuses the v1 guest route and ends the guest with
  recognizable code `0xCD65` instead of hooking wrong code.
- R10 guest additions (remote-actor containment, controller notes) run only at
  the null-playback read inside the character update, never before login.

Recommendation: proceed with the next live sitting as planned. If the guest
crashes at startup again, report the captured fault site and stop, as the
R9 handoff says.
