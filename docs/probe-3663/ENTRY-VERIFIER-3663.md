# 3663 packet-entry verifier

Date: 2026-11-13. This is an offline, file-only result. The game remained
closed. No process was opened, no hook was installed, and no game file or save
was changed.

## Result

The 3663 client packet entries are now strong static file candidates for a
separate passive capture tool:

| Role | Entry | Exact pdata range | Static evidence |
| --- | --- | --- | --- |
| Client request post/enqueue | `exe+0x3A9960` | `[0x3A9960, 0x3A9B7F)` | Allocates `0x8030`, copies the `r9` frame to `block+3`, records the claimed length and raw stack values, then enqueues through `[[object+0xE0]]` |
| Client receive/dispatch | `exe+0x3A9CF0` | `[0x3A9CF0, 0x3AA03C)` | Bounds the complete frame length, optionally transforms it, looks up the first `u16` with kind mask 1, validates descriptor min/max, and copies the accepted frame |
| Client pump | `exe+0x3A8AC0` | `[0x3A8AC0, 0x3A9165)` | Reconstructs `r9=block+3` and the length from `[block+0x800A]-0x1F`, then calls the receive entry; it also calls the send entry |

The executable section is named `.reloc` in this file, but its PE flags are
`0x60000020` (readable and executable code). The verifier uses flags, not the
misleading name.

## Independent send evidence

The send function decodes exactly to its pdata end. Its 34-byte whole-
instruction entry signature occurs once in executable file bytes; the old
16-byte prefix occurs 387 times and is not accepted. The `0x8030` allocation
immediate and `0x7FDA` length limit each occur at their expected instruction
inside this function.

An exhaustive direct-call pass found 414 boundary-verified callers in 414
pdata functions. Current request descriptors are constructed before calls for
`StartActionReq` 2431, `MoveActorReq` 2790, and `RotateActorReq` 2296. The
client pump calls the function at `exe+0x3A9072` after writing the three raw
stack values used by the entry.

Static ABI evidence at the entry:

- `r9`: complete frame pointer;
- entry `[rsp+0x28]`: claimed `u16` length;
- entry `[rsp+0x30]`: raw `u8`;
- entry `[rsp+0x38]`: raw `u16`;
- `r8d`: raw slot/index value;
- `rdx`: result pointer.

The last three meanings remain deliberately raw. The file does not name them
as channel, flags, or session identity.

## Independent receive evidence

The receive function also decodes exactly to its pdata end. Its 66-byte entry
signature occurs once in executable bytes; the old 16-byte prefix occurs 968
times and is not accepted.

Both a decoded pass and a raw `E8 rel32` pass found exactly four direct calls:

| Call | Caller | Evidence |
| --- | --- | --- |
| `exe+0x3A8E13` | client pump `[0x3A8AC0, 0x3A9165)` | `r9=block+3`; fifth argument is `[block+0x800A]-0x1F` |
| `exe+0x3A93CD` | `[0x3A9170, 0x3A9432)` | constructs an internal 11-byte frame |
| `exe+0x3A9C41` | `[0x3A9BA9, 0x3A9CDC)` | framed-stream parser with `block+3` and a derived length |
| `exe+0x8974B9F` | executable `.data1` function | constructs another internal 11-byte frame |

The candidate reads entry `[rsp+0x28]` as a `u16` complete length. It accepts
the initial range 3 through `0x7FDA`, uses the first `u16` at `r9` for a
kind-1 descriptor lookup, and applies that descriptor's measured min/max
lengths. The exact target Ack descriptors are all kind 1. A MoveActorAck
handler then verifies `complete_length - 5 == u16[frame+3]` and initializes a
bounded reader at `frame+5`. This proves a five-byte frame header and real
deserialization. It does not assign meanings to the payload fields.

`exe+0x2A0EA70` is the other function containing the `0x7FD7` comparison. It
has a separate one-caller pump and kind mask `0x11`; the client pump does not
call it. It is rejected as this client receive entry.

The receive function can call an optional object at `[rdx+0x98]` before it
reads the first word. Therefore entry-time bytes may still be pre-transform
when that object is active. A live passive capture must keep that uncertainty.

## Fail-closed verifier

`tools/pkt3663/entry_verify.cpp` is a pure file verifier. It requires:

- timestamp `0x6AF5D198`, base `0x140000000`, and `SizeOfImage` `0x144E9000`;
- non-overlapping, file-backed PE sections and executable flags for all three
  functions;
- exact pdata boundaries;
- the unique 34-byte send entry, unique 66-byte receive entry, and unique
  39-byte pump anchor;
- the packet-copy, length, queue, lookup, and pump setup anchors;
- `E8 rel32` at `exe+0x3A8E13` decoding to the receive entry and at
  `exe+0x3A9072` decoding to the send entry.

A 2976 or 3576 timestamp is rejected before any candidate is considered.
Missing, mutated, duplicated, non-executable, overlapping, truncated, or
misdirected evidence leaves `passive_capture_ready=false`. Passing prints
`verified-file-candidates`, `capture_ready=1`, and `adopted=0`.

The verifier is in its own static target. It does not link the ASI,
SafetyHook, or `tools/pkt3576`.

Validation on 2026-11-13:

- final CTest run: 95/95 game-free tests passed in 59.49 seconds;
- MSVC AddressSanitizer synthetic verifier tests: exit 0;
- MSVC AddressSanitizer real-file scan: exit 0 and
  `verified-file-candidates`;
- real-file result: every range, anchor, and decoded role call matched,
  `capture_ready=1`, `adopted=0`.

The known MSVC linker warning that `/fsanitize=address` is not a linker option
was printed; compiler instrumentation was enabled by the existing ASan build.

## Boundary

This result is sufficient to build a separate, version-checked 3663 passive
capture runner and its dummy-host tests. It is not a live capture, not a
production offset adoption, and not proof that packets reached the wire.
The nine on-disk message IDs are not a live allow-list yet. Position,
rotation, actor identity, and every complete payload layout remain unproved.
The production ASI guard and both production offset headers are unchanged.
