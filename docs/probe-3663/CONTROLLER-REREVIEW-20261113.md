# Codex controller re-review

## Final disposition after the repeated-start fix

Reviewed again on 2026-11-13: the remaining finding is resolved. No new
correctness findings in the revised result policy or regression. The
controller now preserves every invocation result, with conservative Retained
escalation. The real-fixture regression observed
`exit=5 start=1 note=refused attempts=1 stop=4`: the rejected duplicate
leaves the first capture active and it subsequently stops normally.

The controller target rebuilt successfully. The full game-free CTest suite
passed 100/100 in 85.50 seconds; the controller test took 26.30 seconds.
Its stop-flush, report-loss, timeout, and stale-report regressions also passed.
Crimson Desert remained closed. The offline implementation is approved for
a checkpoint; live behavior remains unverified. This review did not commit,
push, deploy, or perform a live capture. The earlier finding below is preserved
as review history and is superseded by this disposition.

## Earlier re-review

Date: 2026-11-13. Base checkpoint `0b03641`; implementation remains local.
Disposition: original two reproductions are fixed; one related P2 remains.
The game stayed closed. No implementation source was modified by this review.

## Resolved

- The actual fixture runner's stop-time flush failure now gives
  `ctl_stop=6`, `ctl_note=bad-log`, `ctl_exit=5`, and stays mapped. The
  session retains `session_write_failed=1`.
- A failing required report after Started or Idle now gives
  `ctl_note=report-unreadable` and `ctl_exit=5` instead of success.
- New controller regressions cover final flush failure, report error return,
  unreadable report storage, actual report timeout, and immediate Retained
  and BadLog returns with milder snapshots. Those tests passed independently.

## P2: Refused is still overwritten by an earlier Started snapshot

Locations: `tools/pkt3663/live_control.h:131-134` and
`tools/pkt3663/live_control.cpp:1118-1127`.

`command_result_after_report` preserves only Retained and BadLog. Every
other invocation result is replaced with the report's stored state.
The runner deliberately leaves the prior `g_start_code` unchanged when
`enter_start` refuses a new invocation while it is active
(`tools/pkt3663/live_runner.cpp:89-97`, `:823-825`). As a result, a second
start returning Refused is converted into the first start's Started result.

Reproduced through the actual fixture controller and runner, using only the
owned dummy host:

1. Start the fixture capture with an explicit dummy PID, fixture DLL path,
   and log directory. It succeeds.
2. Issue exactly the same start command again before stop.
3. The runner refuses the second invocation; no new capture is armed.
   The controller nevertheless prints:

```text
ctl_action=start
ctl_start=0
ctl_capture_attempts=1
ctl_note=started
ctl_exit=0
```

Expected: the second invocation reports Refused and a nonzero exit, while
leaving the original capture active and preserving its reported counters.
This is a command-result bug even though the existing capture stays intact.
The production path normally rejects another start after its hooks change
the identity anchors; two commands that pass pristine identity before either
arms can still reach the same shared result-handling code. That production
race was not exercised against the game.

Fix: preserve the invocation's result independently of the report snapshot
for every code, including Refused, RolledBack, and unknown/error results.
Keep conservative Retained escalation when either observation demands it.
Do not turn a failure into success by selecting older state. Add a repeated
start regression with the real fixture runner and disagreement tests for
the result-selection policy. Verify the first capture remains active, its
attempt count stays unchanged, and it can still be stopped afterward.

## Validation

The corrected controller target and its dependencies rebuilt successfully
with MSVC x64 RelWithDebInfo. The complete CTest suite passed 100/100 in
88.86 seconds. The new controller test passed in 25.75
seconds, including its report timeout cases. Independent reproductions used
the sibling `Review3663` harness and owned dummy processes only.

Do not publish this implementation or start a live capture until the remaining
finding is fixed and reviewed. Keep game-version gates, production offsets,
and fixture separation unchanged. Leave changes local for the next review.
