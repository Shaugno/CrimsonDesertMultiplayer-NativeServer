# Preserving matching work across passive sessions

Large standard comparisons may be temporarily suspended for a live session
instead of discarded or run concurrently with the game. This is not matcher
completion evidence. Resume and finish both real outputs before joining them.

`tools/port3663/r2_matching_pause.ps1` accepts at most two explicit PIDs, one
per matcher role. It restricts executable paths and command-line context to
this checkout's R2NativeSavedTarget AutoVT and nativefix BinDiff jobs. A
receipt pins executable/hash, command hash and creation time. The acquired
process handle's creation time must match, protecting against PID reuse.
No game or unrelated application is accepted.

Pause requires the game closed and every observed analysis thread initially
unsuspended. Each successful suspension is verified by querying every thread's
suspend count. Failure resumes the successfully suspended owned jobs. Verify
is read-only and requires the same identities and a positive suspend count
for every current thread. P1's default still rejects active matching: only an
explicit `-MatchingPauseSession` with a verified receipt can permit the paused
jobs, and any additional unlisted matcher blocks deployment or collection.

Resume requires the game closed, revalidates identities, resumes each pending
entry once and verifies zero counts. A repeated completed resume is a no-op.
If another suspension remains, it raises an error instead of blindly issuing
additional resumes. A partially written receipt can be inspected/recovered
through the same identity-checked resume path; zero counts do not trigger a
native resume. Receipts stay local under build/r2/matching-pause.

The native API declarations and thread-information enum were checked against
the primary [System Informer phnt header](https://github.com/winsiderss/phnt/blob/master/ntpsapi.h).
Unsupported query/control operations fail closed. The helper never changes
analysis memory, matching algorithms, inputs, game memory or native game calls.

Rehearsal on the two actual owned jobs verified all-thread pause and resume.
The first run resumed both jobs but failed while adding a missing PowerShell
receipt property; Add-Member corrected that bookkeeping error. A new receipt
then passed Pause/Verify/Resume, and a duplicate resume did nothing. Negative
checks rejected an unrelated PowerShell PID before creating a receipt and a
changed creation identity before resuming. No game was running in these tests.

Live use: prepare the backup/question first; after the user confirms presence,
pause the remaining owned matchers, verify the receipt, deploy and collect
with that explicit pause session. Finish backup/install verification with the
game closed, then resume the matching jobs. Each live question retains its
own fifteen-minute maximum even when two profiles share one sitting.
