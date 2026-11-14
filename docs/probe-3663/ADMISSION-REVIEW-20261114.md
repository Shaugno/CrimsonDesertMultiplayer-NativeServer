# Codex review: post-ready admission and attempt session retention

## Final disposition after timing-test correction

Reviewed again 2026-11-14: no remaining findings in the admission, per-attempt
session retention, sealing fix, or revised timing assertion. The changes are
approved as a game-free source checkpoint. Live capture remains a separate
validation step; no new usable movement records have been collected here.

The test permits 50 ms of lower-bound timing slack and prints the measured
value. The production 20000 ms wait is unchanged. Assertions for timeout,
argument survival after delayed completion, a single invocation, target
survival, safe stop, and refused unload remain intact.

The controller target rebuilt successfully. The full MSVC x64 RelWithDebInfo
CTest suite passed 100/100 in 89.47 seconds; the controller test passed in
48.64 seconds. Game absence was checked before and after the run using only
the process list. No game was launched or attached to. Changes remain local;
this review did not commit or push them. This disposition supersedes the
earlier sealing finding and intermittent timing-test concern below.

## Re-review after session sealing fix

Re-reviewed 2026-11-14: the session-sealing finding below is resolved.
`AttemptSeal` now scopes ownership after successful log creation in both
production and fixture starts. Early returns and C++ exception unwinding
seal the attempt; only a Started result keeps it open. No new runtime-code
correctness findings in this fix.

The production-on-dummy regression now checks the immutable first refusal,
pending phase mark, and separately recorded second refusal. The original
independent reproduction also confirms the correction:

```text
first_start=1 mark_return=0 sealed_attempt_changed=0 second_start=1
file1_before=892 file1_after=892
```

Affected MSVC x64 RelWithDebInfo targets rebuilt successfully. Validation
was not a completely clean full-suite run: 99/100 passed in 89.75 seconds.
The sole failure was the pre-existing delayed-start test's strict
`wait_ms >= 20000 && wait_ms < 30000` assertion at
`tests/pkt3663_live_ctl_tests.cpp:1308`. Timeout, retained argument witness,
and other safety assertions passed. Its failing measured value was not
printed, so the exact boundary cause cannot be reconstructed from that log.

The controller-only rerun passed in 48.87 seconds. An isolated reviewer
wrapper printing the same delayed-start test's parsed wait value observed
20008 ms and passed. This supports an intermittent timing-test issue, not
a reproduced session-sealing failure. The controller floors QPC elapsed
time to integer milliseconds; exact threshold assertions merit a documented
timing tolerance and actual-value output, while retaining all timeout and
argument-lifetime checks. Do not claim a clean 100/100 full-suite result
from this review or change the production wait merely to satisfy the test.

The full-run failure log is preserved separately at sibling
`Review3663/session-seal-full-run.log`. No runtime source was modified by
the reviewer. The game stayed closed, and changes remain uncommitted and
unpushed. This section supersedes the original sealing finding below;
the timing-test concern remains recorded for follow-up.

## Original review

Date: 2026-11-14. Base `b89a733`. Disposition: one P2 finding remains.
This is a game-free source/test review. No game was launched, attached to,
stopped, or modified. Implementation source was left unchanged.

## P2: early production-start refusals leave an attempt open

Locations: `tools/pkt3663/live_runner.cpp:851-865` and `:868-874`.

Production `start` opens the exclusive attempt session, then returns early
when the mapped identity or module-address check fails. These branches write
a refusal block but never call `seal_attempt`. Only the path that actually
calls `arm` reaches the seal at lines 886-887. The state slot returns to Idle,
but `g_attempt_open` stays true.

As a result, a subsequent successful idle mark rewrites that failed attempt's
file rather than the pending file, and the next start fails `open_attempt_log`
because it thinks an earlier attempt is still open. This violates the new
contract that non-active attempts are sealed and later marks do not reopen them.

Reproduced in a separate owned non-game executable loading the production
DLL and calling only its normal exports. The production identity guard
correctly refused this host; no packet hook was installed:

```text
first_start=1 mark_return=0 sealed_attempt_changed=1 second_start=6
file1_before=892 file1_after=926
```

Refused is 1; BadLog is 6. The first attempt gained the later walk mark, and
the second start never obtained a separate attempt file.

Required fix: seal an opened attempt on every path that does not leave a
started capture active, including early identity/module refusal and exception
exits. Prefer a scoped owner so future early returns cannot omit this cleanup.
Preserve Retained ownership behavior and existing error results.

Add a production-on-dummy regression: refuse a start, snapshot session 1,
mark while idle, and start again. Session 1 must remain byte-identical; the
mark must go to pending, and the second rejected attempt must have its own
session file and retain Refused rather than fail due to a stale open flag.

## What looks correct in the primary changes

- Callbacks check atomic readiness before copying or enqueueing startup
  traffic. The writer defensively excludes rows below the cutoff, including
  the zero-cutoff case, without spending stored/omitted budget.
- Readiness is published only after the ready block and timing write succeed.
  Failed publication follows the existing BadLog/Retained cleanup policy.
- Stop closes admission before hook retirement and drains already accepted
  rows under the existing callback-lifetime rules.
- Successful/arm-path attempts use exclusive numbered session files and are
  sealed on completion. Later marks use pending/mirror files. Buffer bounds
  and explicit history-truncation reporting remain.
- New coverage checks pre-ready receive pressure, cutoff equality, a stale
  queued row, stop before publication, and byte-identical previous successful
  attempt logs across later marks, attempts, and ready-write failures.

## Validation

Affected capture, observer, runner-host, and controller targets rebuilt with
MSVC x64 RelWithDebInfo in the existing sibling `NativeServer-build`.
Full CTest passed 100/100 in 92.42 seconds. The capture and runner-host
regressions passed, as did controller delayed-start and timeout coverage.
The passing suite does not cover the reproduced early-refusal sealing path.
The refusal reproduction is isolated in sibling `Review3663/session_refusal.cpp`
and does not change the reviewed source or capture archives.

Keep changes local for review after the finding is fixed. The pre-ready
admission behavior still needs a later controlled 3663 capture; unit tests
are not proof that live stand/walk/turn evidence has been obtained.
