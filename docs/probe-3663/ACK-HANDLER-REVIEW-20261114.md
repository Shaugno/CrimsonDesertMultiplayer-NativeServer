# Codex review: acknowledgement handler trace and decoders

Reviewed 2026-11-14 against local work on base `502577a`, branch
`grok/compat-guard`. Production guards, protocol, and ASI integration are
unchanged. No game process was launched, attached to, stopped, or modified.

## Disposition

Changes need fixes before approval. The disassembly is reproducible and is
useful new evidence. The decoders and parts of the handoff promote incomplete
reader traces to payload layouts, however. No movement or spawn layout is
approved by this review. The previously reviewed feed fixes remain separate.

## P2: movement decoder omits an intervening reader helper

`src/coop3663/ack_decode.cpp:35-41` requires a 17-byte payload and assigns
offsets 0, 4, 5, and 13 by summing four visible read sizes. The trace at
`ack-trace-20261114.txt:393-396` shows a call to `exe+0x26A8CE0` between the
initial four-byte read and the later one-byte read. The handler passes the
same reader (`rcx = &rbp[-0x80]`) and another destination object
(`rdx = &rbp[0x10]`) to this helper, then checks its result. Its consumption
has not been traced. Thus those four immediate sizes are not evidence of
four adjacent fields, a total length of 17, or the last three fixed offsets.
This is also the object later passed alongside the eight-byte buffer to
`exe+0x55EF60`. Tracing only that final consumer skips a crucial dependency.

Remove/quarantine the asserted layout until the helper and reader cursor
behavior are established. Correct HANDOFF-CURRENT.md:42-50,78-79. Do not
replace the length with an older-build constant. Synthetic tests that merely
fit 17 bytes do not validate this format.

## P2: actor-load decoder invents a flat u16 sequence and a fixed start

`src/coop3663/ack_decode.cpp:45-56` assumes the payload starts at bytes+5 and
consists entirely of consecutive u16 values, with odd lengths invalid.
The same handoff says slot 2's start is runtime-derived (lines 33-35).
The trace confirms this at `exe+0xA2F8D1C-0xA2F8D2D`.

After the two-byte read, the handler passes the reader itself to
`exe+0x9D68E0` (`exe+0xA2F8D8B-0xA2F8D92`), then computes its next count
from the saved count plus the extracted u16 (`exe+0xA2F8D97-0xA2F8DA0`).
That is not a demonstrated loop that simply advances by two bytes. The
callee may consume further data. The extracted u16 is kept in edi across
the call; the listing does not show it passed as the callee's ordinary
integer argument as the handoff claims. Also, slot 2's bound is formed by
an immediate plus a data load, unlike slot 1's literal bound.

Trace the start calculation, underlying reader, and nested dispatch before
defining a load decoder. Until then, do not assert flat pairs, odd-length
rejection, or a shared fixed-header policy for this handler. Fix
HANDOFF-CURRENT.md:64-70,80-82 and ack_decode.h:38-40 accordingly.

## P2: header bound rejects equality that the traced branch permits

`src/coop3663/ack_decode.cpp:20` requires declared < 0x7FC1 when bound_flag=0.
The MoveActorAck listing has `cmp ax, cx` followed by `jb` at
`exe+0xCA8B01-0xCA8B04`. Here ax=0x7FC1 and cx=declared, so that failure
branch is taken when declared > 0x7FC1. Equality passes this particular gate.
Rotate, Remove, and ActorLoad slot 1 have the same literal comparison shape.
This does not say that a boundary-size frame passes every later parser check.

Correct the gate to represent the demonstrated inclusive bound, update
ack_decode.h:23-24, and replace the incorrect boundary assertion in the
tests. Cover below/equal/above with sufficiently sized spans, both flag
states, short spans, and complete/declaration mismatches. Do not generalize
the gate to slot 2 until its computed bound is resolved.

Independent reproduction against the unchanged decoder:

```text
header_declared_0x7fc1 accepted=0 branch_allows_equality=1
unproved_17_byte_move accepted=1
flat_load accepted=1 values=2 assumes_start_plus_5=1
```

The last two lines demonstrate the decoder's current assumptions, not that
the synthetic buffers are messages accepted by the game.

## Validation

- Executable file SHA-256 independently matches
  `8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
- Rebuilt the new feed test and file tracer with MSVC 19.52.37029, x64
  RelWithDebInfo. Supplied test result: `coop3663 tests ok`.
- Ran the tracer only against the executable file as a read-only image.
  Its 1,967 output lines match the archived trace with zero differences.
  Mapping file bytes for disassembly does not execute the game image.
- Built and ran a separate C++ harness in sibling `Coop3663-review`, target
  `ack_decode_review`, producing the observations above. It uses synthetic
  byte buffers only. Repository source was not instrumented or modified.
- The sandboxed trace mapping returned `map failed`. The sandboxed trace
  target rebuild stalled and was canceled. A rebuild and file-only mapping
  outside the sandbox succeeded. No failed attempt is counted as a pass.
- Read-only process inventory found existing game PID 51032. Full-suite
  tests and the canonical full build script were not run while it was open.
  The existing sibling build was used for the independent targets.
  No current full-suite result is claimed. `git diff --check` passed.

## Next package for Grok

Resolve these three findings together. First trace `exe+0x26A8CE0`, the
reader implementation reached through slot +0x28 (including the recorded
`exe+0x1643CD0` target), and ActorLoad's `exe+0x9D68E0` and cursor-start
calculation. Then trace `exe+0x55EF60` where needed to establish how the
fully read movement object is used. Preserve branch conditions and nested
read consumption in the evidence table; do not sum only top-level calls.

Update the offline decoders only for demonstrated layouts, add regressions,
and deliver one consolidated handoff. If the dependency cannot be resolved,
mark the corresponding decoder unsupported and name the exact blocker.
Keep this file-only. No native actor invocation, packet replay, new live
capture, production offset change, or guard broadening is needed for this
package. Run the full suite after the user closes the game before final
publication approval. No push was performed by this review.
