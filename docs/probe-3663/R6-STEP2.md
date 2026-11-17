# R6 Step 2 — diagnostic captured (2026-11-17)

The one actual diagnostic L3 conversation reproduced the refusal. **Exactly
two admission predicates failed: size and number.** No actor insertion or
commit ran. This meets the diagnostic goal, not L3 acceptance.

At **09:21:57 Eastern**, host PID 45568 recorded:

```text
length=17 prefix13=04FFFFFFFFFFFFFF7F01000000
discriminator=0x04 number=0x7FFFFFFFFFFFFFFF slot=1
active=true broken=false context=true bound=true lobby_ready=true
size=false number=false slot=true generation=true
state_ok=true epoch_nonzero=true epoch_match=true guest_nonzero=true user_match=true
lobby_edits=1 status_edits=0 phase_edits=0
state_epoch=1 current_epoch=1 state_user_key=0x901002FA current_user_key=0x901002FA
```

`broken=false` is a passing health condition, not a failed admission check.
The native request is 17 bytes, matching the exact target reader's fixed
1+8+4+4 fields with an empty UTF-16 remainder. The extra u32 was not included
in the bounded first-13-byte diagnostic and its value is not invented.
The requested number is the single-player/native sentinel, while the verified
host guest lobby holds the assigned guest number. The log rules out missing
lobby readiness, stale epoch/user/generation and forbidden slot 3/206 as the
cause of this refusal. The old 19-byte reference is not the observed request.

Both copies used the sealed Step 1 diagnostic build, exact 3663 executable,
external verified backups, host real saves and zero-hook guest profile.
Host launched at **09:10:33**, guest PID 34412 at **09:13:54**. The user
confirmed host in-world and both usable views left/right before guest Play.
PID-bound reports were frozen before that signal. Native login/title refresh
passed; no native fault was logged. No automated capture was required or used;
this failed join did not produce a user movement recording.

The guest exited normally, code 0, at **09:23:13**. A host normal-quit signal
was given before the common **09:24:33** watchdog, but its pinned receipt
records deadline termination at **09:24:34**, exit 0x3663. Normal host exit
is not accepted. The user subsequently confirmed they had not selected host
Quit before the deadline; no exit hang is established. The next acceptance must reserve time for
normal quit. No timeout was extended.

Cleanup verified all 23 real saves unchanged, both frozen backups, and
restoration/removal of the six controlled installation files. No progress
was restored over. The earlier unused R6-D1 plan expired without launching
either game; official reuse was refused, cleanup ran, and a fresh session
was prepared without editing timestamps. That setup is conservatively counted
as startup. R6 usage is **startup 1/5, join 1/3**; the join count was promoted
only after the guest's real launch receipt existed. R5 records stay unchanged.

Next work is limited to the two measured mismatches: decode the observed
17-byte form from exact-3663 handler evidence and map the guest's native
single-player request number into its assigned remote conversation identity.
The host must continue to require the assigned guest number and all existing
lobby/binding/generation/slot checks. Do not admit arbitrary numbers, guess a
different slot, patch identity checks out, or fix unrelated paths without
evidence. Add regression coverage, run the full suite and smoke, then retry
L3 within the fresh budget. `port-evidence/r6/step2-results.json` seals the
actual diagnostic, own-PID logs, receipts, window confirmations and cleanup.
