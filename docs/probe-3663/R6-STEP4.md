# R6 Step 4 — L3 retry blocked at game activation

**L3 remains unpassed.** The post-fix guest displayed the game's anti-tamper
error **88500006** before reaching title or native login. It never sent the
cryptogram or a character-login request. The measured layout/identity fix
was therefore **not exercised in this runtime attempt**. No further launch
was made after this error.

The source checkpoint is **8ff1732**, with its 119/119 full suite and sealed
eight-second smoke. Host PID 35788 launched **09:41:46 Eastern** and loaded
its real save. Guest PID 51432 launched **09:45:22** with its verified native
guest profile and zero file hooks. Its mod worker/UDP transport started, but
native guest sent/delivered counts stayed zero, remote=false, and WorldSystem
was not created. Transport connection alone is not native login or L3 proof.
No canonicalization, identity admission, actor insertion or commit occurred.

The user supplied the error screenshot at **09:45:39**, selected Exit and
the guest exited at **09:45:50**, code 0xFFFFFFFF. The user was instructed to
quit the host normally immediately, without retrying the guest. Host normal
exit **0** was verified at **09:48:40**, well before the fixed **09:55:46**
watchdog deadline. Both games and their controllers are gone. All 23 real
saves and the frozen backups match their manifests; installation cleanup
verified. No real save was restored over.

The guest existed, so this is conservatively retained as R6 join experiment
2 under the stated receipt-based ledger procedure. It is a failed guest
startup, **not a second character-login test or successful join**. Current
usage is startup **1/5**, join **2/3**; remaining budget is not permission to
retry a known external activation hold. R6-D1b remains the only diagnostic
character-login request. The unused expired R6-D1 setup remains separate.

## Official support finding

The browser-readable shell of the linked CodeFusion page did not expose its
explanation. Read-only retrieval of its public application data established
the mapping `88500006 -> 24H`; the official English language resource advises
waiting **24 hours before trying again** because purchase revalidation is
currently unavailable. See [the exact error's official support page](https://support.codefusion.technology/anti-tamper/?e=88500006&l=en-US&p=s&a=Crimson%20Desert),
[public error mapping](https://support.codefusion.technology/js/app/v2.6.3/app.js?v=2)
and [official English text](https://support.codefusion.technology/anti-tamper/lang/english.json?v=3).
Retrieved 2026-11-17; local snapshots and hashes are in the evidence JSON.

This does not establish why revalidation failed, a device count, account
misuse, a sandbox cause or a guaranteed reset instant. Community claims are
not used as technical evidence. No Retry was automated, protection changed,
token copied/deleted, credential requested, game executable patched or
further activation attempted. The official wait puts the next possible
manual test no earlier than approximately **2026-11-18 09:46 Eastern**;
actual recovery is unverified. No automatic retry/reminder was scheduled.

## Remaining acceptance

Guest character admission and actual native join still need live validation.
Mutual walking/turning, user-recorded two-window video with filename/duration,
guest removal, reconnect outcome, guest real-save manifest acceptance and
post-session host reload remain unproved. No recording was requested after
this startup error; the screenshot documents only the error. The diagnostic
host quit was separately deadline-stopped; the user clarified they had not
selected Quit then, so no hang was established. This retry's host quit is
normal, but it does not pass the remaining L3 checks.

L4 remains gated on L3. Independent work completed the two measured fixes,
regression suite/smoke, exact-build/source checks, official support research,
immutable experiment evidence and cleanup. `port-evidence/r6/step4-results.json`
seals the role logs, receipts, screenshot and external-blocker provenance.
