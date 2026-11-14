# Codex review: startup timing and delayed-start tests

## Final re-review after both fixes

Re-reviewed 2026-11-13: both findings below are resolved. No new correctness
findings in the revised logging failure path, explicit benchmark opt-in, or
fixture-only fault export. The implementation is approved as an offline
diagnostic checkpoint. The original live timeout's cause remains unproved.

Stage-write failure is now sticky and forces BadLog cleanup, or Retained if
cleanup cannot be proved. The new fixture regression covers a ready-stage
rewrite failing after the session is truncated and checks removed hooks and
a durable invalid-session marker. The independent original short-write
reproduction now reports:

```text
ready_write_fault=1 start_return=6 report_start=6 file_exists=1 session_bytes=1858
stop_return=5
```

The file-backed benchmark runs only with an explicit path argument and
reports unsupported identity without failing normal synthetic unit tests.
The default CTest identity run does not open the installed executable.

Affected targets rebuilt successfully in the normal MSVC x64 RelWithDebInfo
build. Full CTest: 100/100 passed in 116.95 seconds. The controller test,
including delayed start and report-timeout cases, passed in 48.55 seconds.
Crimson Desert was confirmed absent using elevated process-list-only checks;
no game was launched or attached to. `git diff --check` passed. No runtime
source was changed by this review and nothing was committed or pushed.

The earlier findings and validation limitation below are historical and are
superseded by this result. Before a new live session, retain the capture-window
planning and report corrections from `CAPTURE-REVIEW-20261113.md`.

## Earlier review

Date: 2026-11-13. Base `bed420f`; changes are local. Disposition:
changes requested. No game was launched, attached to, stopped, or modified
by this review. Implementation source was left unchanged.

## P1: the final timing write can destroy readiness evidence after success

Locations: `tools/pkt3663/live_runner.cpp:246-272` (`note_stage`) and
`:711-713` (ready timing after the successful ready block).

`note_stage` appends to the in-memory session text, calls `log_flush_file`,
and ignores failure. That function opens the existing session with `wb`
at line 193 and rewrites the entire file. The new `ready-durable` stage runs
after the evidence block successfully reached disk, but before Started is
returned. A short write or close failure during this extra rewrite can
therefore erase/corrupt the previously durable ready evidence while start
still returns Started and keeps capture active.

Reproduced with a reviewer-only executable compiling the unchanged runner
source, actual capture backend, and owned dummy functions. A stdio wrapper
returned a short write only for the buffer containing `stage=ready-durable`,
after the real `wb` open. No existing source or live process was patched.

```text
ready_write_fault=1 start_return=0 report_start=0 file_exists=1 session_bytes=0
stop_return=4
```

The session was empty immediately after start returned success. Stop was
performed afterward to clean up the owned dummy hooks.

Required fix: diagnostic timing must not invalidate committed session
evidence while preserving a success result. Either keep timing writes
non-destructive to that evidence or propagate every relevant stage-write
failure through the existing BadLog/Retained cleanup policy. Retain sticky
invalid-session evidence. Do not bypass actual I/O failure coverage merely
to preserve the old fixture failure counter: the new stage writes currently
use `log_flush_file` directly and are not exercised by that counter. Add
short-write/close-failure coverage specifically after the ready block.

## P2: the default identity unit test now depends on one local game install

Location: `tests/pkt3663_live_tests.cpp:467-516`, called unconditionally
from `main` at line 536.

The new supposedly optional benchmark automatically opens a hardcoded
installed executable when present, maps it, and asserts Match four times.
There is no opt-in parameter. A machine without that path skips it, while a
machine with another game version at that path fails the same otherwise
synthetic test suite. In this project Steam has already replaced several
builds, so presence at that path is not a stable test fixture. A correct
refusal of the next build must not become a synthetic-unit-test failure.

Required fix: separate the file-backed benchmark behind an explicit opt-in
path or command. Keep normal tests deterministic with owned fixtures. The
benchmark should report unsupported identity cleanly, without weakening
the production gate or assuming arbitrary installed mappings are readable.

## What the patch establishes

The change adds timing and a delayed-start double. It does not change the
20-second wait or optimize identity scanning. It is diagnostic preparation;
it does not establish that the live startup timeout is fixed.

The offline benchmark on this machine reported:

```text
synthetic_identify_us=28
installed_kind=match
installed_identify_us=4450930
installed_identify_x3_us=12409499
file_bytes=335440280
```

Those measurements used a read-only image mapping in the test process.
They are not measurements of loader delay, in-game memory reads, hook
installation, or the original timed-out export. They make repeated identity
verification a useful measurement target, not a proved root cause.

## Validation and current limitation

The usual build hit LNK1104 on the existing production diagnostic DLL.
Affected targets rebuilt successfully into a separate `StartupReview-bin`
using an MSBuild OutDir override, without replacing that loaded file.

Direct results from the fresh binaries:

- capture tests: passed, including fault children;
- live runner host tests: passed;
- live identity tests and the offline timing benchmark: passed;
- controller tests: refused before process-control integration tests because
  the installed Crimson Desert process was already running. The guards
  printed `Crimson Desert is running; controller tests will not attach`.

An elevated process-list-only check confirmed existing PID 17324 at the
installed game path. The initial sandbox process listing did not expose it.
No game was launched or attached to, and the process was left alone. The new
delayed-start integration regression was therefore NOT independently run
in this review. No full-suite pass is claimed. `git diff --check` passed.

## Next task for Grok

Fix the two findings and retain all existing identity and cleanup rules.
Close the existing game before validation. Run delayed-start, timeout,
repeated-start, report-loss, and new late-timing-I/O regressions, followed by
the full suite. Provide measured results and a precise explanation of what
is still unknown about the live timeout. Also finish the capture-window plan
and factual report corrections requested in `CAPTURE-REVIEW-20261113.md`.
Leave changes local for Codex review before publication or another live run.
