# Handoff: 3663 acknowledgement findings fixed; walk prototype blocked

User-requested transfer to Claude: [implementation handoff](../CLAUDE-HANDOFF.md).
Continue offline; preserve any newer local changes. The user requested this
progress checkpoint be committed and pushed to Shaugno before Claude takes over.
Earlier references to uncommitted work describe its pre-publication status.

## Latest checkpoint: concrete character-control component reader

See [COMPONENT-READERS-20261114.md](COMPONENT-READERS-20261114.md).
Resolved character-control and transform-sync class vtables through file RTTI.
Added an offline structural character-control decoder for its conditional
fields, variable bytes, resource IDs and variable entries. Nested reads are
accounted for; native object stride is not used as wire length. Focused tests
pass. Full offline suite passed 101/101 in 90.26 seconds, including the prior
controller fixture isolation fix. git diff --check passed. Native character creation is still
unimplemented; component inventory, transform structures, resource validity,
and ownership remain unresolved. No game use or deployment. Changes local.

Earlier checkpoints below retain their historical scope.

## Current Codex checkpoint: component initialization and cleanup

See [COMPONENT-LIFECYCLE-20261114.md](COMPONENT-LIFECYCLE-20261114.md).
New file evidence establishes the compact component list's initialization
order, shared-reader deserialization, reverse cleanup order, and a deferred
cleanup callback. The concrete factory and component schemas remain unknown;
these native list entries are not wire layouts. Failed-initialization rollback
and deferred completion ownership are still unresolved.

An offline decoder now checks the body relation tail only when its start is
independently known. It never guesses where opaque components end. Focused
tests passed. Full suite: 100/101 passed in 89.65 seconds. The controller
fixture hit a stale directory from a recycled dummy PID. Its test-only log
naming now includes the test process identity/creation time; the rebuilt
controller test passed in 49.03 seconds (CTest total 49.38). No failure was
suppressed and old evidence was preserved. The other 100 tests were not
rerun after this isolated fixture change. No game launch,
attachment, native calls, deployment, or push; changes remain local.

Next dependency: resolve the descriptor factory and each required component
reader, initialization phase and cleanup contract before native actor work.
The earlier checkpoint sections below retain their historical validation.

## Current Codex checkpoint: spawn/rotation/removal offline parsers

Latest evidence and implementation:
[SPAWN-HEADING-REMOVAL-20261114.md](SPAWN-HEADING-REMOVAL-20261114.md).
The bounded offline library now parses spawn record envelopes, the
file-traced four-component rotation, and the removal layout (including
its leading byte). A labeled synthetic test composes decoded fields through
fake actor create/move/remove and verifies late samples are refused.
Native actor creation/removal is still not implemented: component-specific
spawn bodies, native ownership/thread contracts, and live conventions remain
unresolved. Nothing was deployed, no game process was opened, and guards
are unchanged. Current implementation is local and uncommitted.

Validation: full suite 101/101 in 90.26 seconds; the final composed fixture
was added during that run and then separately rebuilt and passed. See the
new report for exact scope, evidence hash, and remaining dependencies.

The paragraphs below retain earlier checkpoints and their historical limits.

Latest offline implementation: `decode_move_ack_spatial_fields` now extracts
only the file-proven raw word and three finite spatial floats from the
239-byte full-parse MoveActorAck layout. It checks header, file-descriptor
opcode, exact frame length, and retained byte coverage. It does not provide
heading, coordinate-space classification, or live readiness and is not
connected to a game backend. New regression coverage and the full independent
suite passed: 101/101 in 98.97 seconds. Later tracer-only extensions rebuilt
and ran successfully. See the updated movement-consumer note for lifecycle
types and the remaining work. Nothing has been deployed or pushed.

Codex takeover update, 2026-11-14: the user assigned implementation and
investigation to Codex. New file-only results are in
[MOVEMENT-CONSUMERS-20261114.md](MOVEMENT-CONSUMERS-20261114.md) and
ack-trace-consumers-20261114.txt. The three floats at payload offsets
196/200/204 reach a conversion using 1000-unit spatial cells. Coordinate
space and live meaning remain unverified. The earlier blocker description
below is historical; independent investigation is continuing.

Date: 2026-11-14. Branch `grok/compat-guard`, HEAD `502577a`, local
diff uncommitted. This is a blocker handoff. Two installations have not
been shown walking or turning. No actor was spawned, no packet was
replayed, and no diagnostic was deployed. Production guards, protocol
`kVersion`, and the installed game file were not changed.

## Findings from ACK-HANDLER-REVIEW-20261114

1. The 17-byte MoveActorAck decoder is removed. `exe+0x26A8CE0` consumes
   the same reader between the first 4-byte read and the later reads.
2. The flat ActorLoad u16 decoder is removed. Slot 2's start is a runtime
   subtraction. `exe+0x9D68E0` receives the reader and reads further bytes
   itself; the caller's u16 is not its integer argument.
3. The literal header bound matches `cmp ax, cx` / `jb` at
   `exe+0xCA8B01`. Declared length `0x7FC1` passes. Only a greater length
   fails when `bound_flag` is 0. ActorLoad slot 2 uses a different bound
   and is not accepted by this gate.

## What the success path consumes

File-only `SEC_IMAGE` trace. The tool printed
`identity=build-3663 timestamp=0x6AF5D198 adopted=0` and
`move-reader-vtable=0x5B3BE18 slot28=0x141643CD0`. Listings:
`ack-trace-20261114.txt`, `ack-trace-helpers-20261114.txt`,
`ack-trace-nested-20261114.txt`.

MoveActorAck builds its reader at `rbp-0x80` with vtable `exe+0x5B3BE18`,
base `bytes+5` at reader `+0x20`, declared length at reader `+0x28`, and
cursor 0 at reader `+0x2C`. Slot `+0x28` is `exe+0x1643CD0`. That function
copies exactly `r8d` bytes and advances the cursor by the same count. The
handler fails the later reads when the slot is `exe+0x3A6E20`.

`exe+0x16436F0` only grows the pointer array at reader `+0x10` and
increments reader `+0x18`. It does not call slot `+0x28`.

On the success path the cursor therefore advances by:

| Step | Bytes | Running cursor |
| --- | ---: | ---: |
| first read into `rbp+0x148` | 4 | 4 |
| `exe+0x14CB240` | 136 | 140 |
| `exe+0x1644720` into object `+0x88` | 40 | 180 |
| `exe+0x1644640` into object `+0xB0` | 16 | 196 |
| object `+0xC0`, `+0xCC`, `+0xD0`, `+0xD8`, `+0xDC`, `+0xDD` | 12+4+8+4+1+1 | 226 |
| one byte, then 8, then 4 | 1+8+4 | 239 |

`exe+0xCA8D63` requires that cursor to equal the declared length. A
successful return has consumed 239 payload bytes. The 136-byte prefix is
`exe+0x14CB150` twice (64 each) plus one 8-byte read. Each 64 is
`exe+0x1644720` (12 + 16 + 12) plus two 12-byte reads. This is the
handler's own success path, not a captured message.

For the object-kind branch `[type+1] == 1`, `exe+0x55EF60` copies:

| Source | Payload offset | Store |
| --- | ---: | --- |
| 16 bytes at object `+0xB0` | 180 | `[A+0x55C]` |
| 8 bytes at object `+0xC0` | 196 | `[A+0x550]` |
| 4 bytes at object `+0xC8` | 204 | `[A+0x558]` |
| 8 bytes at `rbp-0x40` | 227 | `[D+0x90]` |

With S=qword[r14], A=qword[qword[qword[S+8]+0x68]+0x1A0], whereas
D=qword[S+0xE8]. These are different pointer paths; aliasing is unproved.

Those are byte copies. The instructions do not show a world position or a
heading. No decoder maps them to movement.

`exe+0x9D68E0` reads 1 byte, then 4, masks that dword with `0x30000000`,
and compares it with `0x10000000`. The matching arm calls
`[result+0x130]` with the reader. Allocation/removal through downstream
callees has not been established or ruled out.

## Tests

`ack_header_ok` checks the span, the declared-length equality, and the
inclusive `0x7FC1` bound. The movement and actor-load field decoders are
gone. The regression covers `0x7FC0`, `0x7FC1`, and `0x7FC2` with flag 0,
`0x7FC2` with flag 1, a short span, and a length mismatch.

Game was closed. Sibling RelWithDebInfo build, MSVC 19.52, then CTest:
101/101 passed, 0 failed, 124.67 s. The tracer changes after that run do
not link into a tested binary. No game process was launched or stopped.

## Blocker

Two-player walking is not demonstrated. The missing fact is the meaning
of the four stores above and a 3663 create/remove operation. The 239-byte
cursor map does not identify a player position, a heading, or an actor
lifetime, so neither a position reader nor a packet relay is selected.

Smallest next experiment: file-only, find one later read of
`[inner+0x550]` or `[inner+0x90]` that uses the value in a demonstrated
world-coordinate operation, or show that those slots are not coordinates.
Do not deploy a decoder and do not repeat the empty post-ready capture
for this question. A second installation cannot show the other player
until that fact exists.
