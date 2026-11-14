# Codex review: offline prototype feed

Reviewed 2026-11-14. Base `502577a`, branch `grok/compat-guard`.
Changes reviewed: `src/coop3663/feed.h`, `feed.cpp`,
`tests/coop3663_tests.cpp`, `CMakeLists.txt`, and `HANDOFF-CURRENT.md`.

Disposition: changes need fixes before approval. This is a synthetic,
single-threaded feed scaffold, not a 3663 state reader, transport adapter,
or native actor backend. It does not resolve the compatibility blocker.
The handoff correctly states those limits and does not claim live success.

## Findings

### P2: first accepted pose never reaches the backend

`src/coop3663/feed.cpp:93-97` calls `create(id)` and stores the pose only
inside the feed. `ActorBackend::create` (`feed.h:57`) has no pose parameter,
and there is no move call on that path. A single accepted sample therefore
creates an actor without ever providing its position or orientation. This
also occurs after a generation replacement. The fake backend records a
Create event without a pose, and the supplied test never checks the first
pose's delivery. Make initial placement part of successful publication and
define cleanup/retry behavior when placement fails.

Independent reproduction: `initial_sample accepted=1 events=1 poses_delivered=0`.

### P2: generation changes are not fenced against retired generations

`src/coop3663/feed.cpp:85-91` first compares timestamps across generations,
then treats any generation inequality as a replacement. Publishing
generation 1/time 100, generation 2/time 200, then generation 1/time 300
removes generation 2 and recreates the retired generation. Conversely,
generation 2/time 1 after generation 1/time 100 is rejected; there is no
documented cross-session clock requirement that would justify that rule.
Specify an admitted session/generation contract, fence retired sessions,
and scope sample ordering to the correct session. Do not let arbitrary
received generation values switch the current actor. Cover delayed old
traffic and a legitimate session restart with a reset clock. A disconnect
also discards the only ordering information, so account for late traffic
after disconnect in that contract.

Independent reproductions:

```text
generation_rollback accepted_old=1 old_alive=1 current_alive=0
generation_clock_reset accepted_new=0 old_alive=1 current_alive=0
```

### P2: slot storage grows with every reconnect or replacement

`src/coop3663/feed.cpp:96` always appends a slot. Replacement marks the old
slot absent (line 90), disconnect marks it absent (line 112), and
disconnect-all does the same (line 121). No path reuses or erases those
entries, and there is no capacity. Even one peer repeatedly reconnecting
grows memory and lookup/disconnect scan cost indefinitely. Reuse bounded
active slots and keep any necessary session rejection history bounded too.
Add repeated reconnect/replacement coverage and a capacity contract. This
finding concerns Feed storage; the fake backend's event history has a
separate test-only purpose.

### P2: finite values alone do not establish a valid quaternion

`src/coop3663/feed.cpp:12-15,60` accepts `(0,0,0,0)` and arbitrarily scaled
finite quaternions when `has_heading` is true. Such values do not represent
a valid rotation for the proposed actor adapter. Check a documented norm
tolerance, or normalize appropriate inputs while rejecting a zero/near-zero
norm. Use a numerically safe norm calculation and test zero, tiny, extreme,
non-finite, and normal rotations. A presence flag does not make malformed
rotation values valid.

Independent reproduction: `zero_quaternion feedable=1`.

## Validation and isolation

Rebuilt `cdmp_tests_coop3663` in the existing sibling `NativeServer-build`
with MSVC 19.52.37029, x64, RelWithDebInfo. Its supplied tests pass:
`coop3663 tests ok`. The canonical build script was inspected; its full
build/test path was not run because a read-only elevated process inventory
confirmed CrimsonDesert.exe PID 51032 was already running. The new synthetic
test is safe to run independently and never opens that process.

Built an independent C++ reproduction against the unchanged feed source
in sibling `Coop3663-review`, output `Coop3663-review-build`. It exercises
only in-memory fake actors; results above confirm gaps not covered by the
supplied tests. No game launch, attachment, deployment, stop, or memory
operation was performed. No full-suite pass is claimed for these changes.

The CMake addition links the feed only into its new test. Production mod,
wire protocol, old-build offsets/guards, and Steam restrictions have no
changes in this diff. No new executable-file analysis, external reader,
memory-reader fixtures, or transport round-trip is included in the package.

## Next package for Grok

Handle these four defects and their regressions in one local implementation
and self-review batch. Preserve the synthetic-only boundary and do not add
more unconnected abstractions solely to claim progress.

Then advance the concrete dependency investigation: perform and document
file-only analysis of 3663 controlled-actor/state access and the native
actor create/move/remove path, tracing candidates to instructions and
types. Existing 2976/3576 implementations are research leads, not layouts.
State which route the evidence supports and the exact unresolved fact.
Implement a read-only reader and fixtures only where evidence supports its
design; retain explicit unknown fields. A simulated feed alone cannot
establish live validity or actor compatibility.

Update HANDOFF-CURRENT.md with fixes, new evidence, tests actually run,
and one consolidated next step. Run the required full suite once the user
has closed the game. Do not terminate it for testing. The handoff's default
return to the same 15-second capture is not evidence that the independent
file-analysis work is exhausted. Request another live experiment only with
a concrete hypothesis and a reason it resolves the selected route's blocker.
Keep changed runtime code undeployed and production guards intact.
