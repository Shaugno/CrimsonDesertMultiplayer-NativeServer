# Codex re-review: feed fixes and actor investigation

Reviewed 2026-11-14. Base `502577a`, branch `grok/compat-guard`.
Read the current handoff, complete feed implementation and tests, CMake
addition, and the cited packet scan and entry-verifier reports.

## Disposition

The four original feed findings are resolved under the revised synthetic-only
contract. No additional blocking findings were found in the reviewed code.
Targeted validation passes. Required full-suite validation remains pending
while the game is running; this is not approval to deploy a native backend
or claim 3663 co-op compatibility.

The original review is preserved in OFFLINE-PACKAGE-REVIEW-20261114.md and
describes the superseded implementation.

## Fix verification

| Original finding | Revised behavior and location |
| --- | --- |
| Initial pose was lost | `feed.cpp:170-178` calls `create(id, pose)` and advances placement/clock state only on success. `feed.h:75-76` defines failed creation as leaving no actor. |
| Sample could switch back to a retired generation | `feed.cpp:139-155` explicitly admits a session; `feed.cpp:162-168` requires its generation and orders timestamps within it. Disconnect retires the active session. |
| Slots grew on every replacement/reconnect | `feed.h:129-130` uses fixed eight-slot and 32-entry arrays; `feed.cpp:102-107` reuses inactive slots. |
| Zero/invalid quaternion accepted | `feed.cpp:66-88` rejects non-finite and unusable rotations, computes a scaled norm, enforces its accepted interval, and normalizes valid input. |

The retired-session fence is explicitly bounded. After its 32-entry history
is overwritten, a generation can be explicitly admitted again. Future
transport integration must make begin_session a connection-admission action,
not an action triggered by arbitrary pose traffic. The current feed cannot
select another generation through publish alone. Keep that distinction and
the documented single-threaded use when integrating it.

## Independent validation

- Rebuilt `cdmp_tests_coop3663` in sibling `NativeServer-build`, x64
  RelWithDebInfo, MSVC 19.52.37029. Executed it successfully:
  `coop3663 tests ok`.
- Independent harness in sibling `Coop3663-review/fixed_review.cpp` exercised
  initial-pose delivery, retry after failed create, retry after failed move,
  failed removal during replacement/disconnect/disconnect-all, reset clocks,
  rejection of delayed retired samples, quaternion rejection/normalization,
  and 200 create/disconnect cycles. Result:
  `fixed feed independent checks failures=0`.
- `git diff --check` passed. The feed links only into its test; the diff does
  not change production ASI, protocol, offsets, or guards.
- A read-only process inventory confirmed CrimsonDesert.exe PID 51032 was
  already running. No game was launched, attached to, stopped, or modified.
  Full-suite/controller tests were not run. The canonical full build script
  was not invoked; the existing configured sibling build was used for the
  safe, isolated target above. No full-suite result is claimed.

## What the investigation establishes

HANDOFF-CURRENT.md:38 explicitly bases its descriptor/handler table on the
existing PACKET-SCAN-3663.md and ENTRY-VERIFIER-3663.md. The values and the
distinction between RemoveActorAck and ManagementRemoveActorAck agree with
those reports. Layouts remain unresolved, and the table does not promote
handler addresses to callable actor APIs. These are appropriate limits.

There is no new instruction-level handler investigation in this diff.
The code remains a synthetic lifecycle scaffold. A 3663 player reader,
transport adapter, live readiness rule, and native actor lifecycle remain
unimplemented/unverified. Do not describe the planned investigation as done.

## Next work package

Proceed with the handoff's file-only handler investigation, in one batch:

1. Recheck the executable file identity without opening a game process.
2. Trace ActorLoadDataAck, MoveActorAck, RotateActorAck, and RemoveActorAck
   descriptor handlers through their bounded readers and consumers. Compare
   MoveActorReq only where it helps establish the request/ack relationship.
   Receiving acknowledgements matter for the local visible remote actor;
   tracing only the movement request is insufficient for that path.
3. Record instruction addresses, lengths/branches, field widths, frame versus
   payload offsets, and the downstream use of candidate keys/transforms.
   Separate demonstrated reads from candidate semantics and live claims.
   Identify the concrete lifecycle dependency or unresolved branch.
4. Implement a bounded offline decoder only for fields justified by those
   traces, with malformed/truncated-input tests and labeled synthetic data.
   If insufficient evidence exists, report that precise blocker instead.
5. Update HANDOFF-CURRENT.md once with artifacts, commands, tests, conclusions,
   and the smallest further experiment. Run the required full suite after
   the user closes the game, before final publication approval.

Keep runtime changes undeployed. No packet replay, native actor call, new
live capture, or production guard change is part of this offline package.
