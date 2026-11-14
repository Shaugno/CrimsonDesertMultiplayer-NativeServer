# Offline spawn framing, rotation, and removal

Codex implementation, 2026-11-14. Base 502577a plus preserved local work.
No game process was launched, attached to, stopped, or modified. No native
actor function was invoked, and no packet was replayed. The ASI, production
guards, and protocol version are unchanged.

## Evidence

Raw listing: spawn-heading-removal-trace-20261114.txt.
SHA-256: 9E723E30913475971BF685F48AB0C1CD31F9ECC232F7DD152ABAAED79310A02B.
The strict file identity is 3663; all candidate addresses remain adopted=0.

### Spawn record envelope

ActorLoad slot 2 calculates its reader start using
0x41314825 - dword[exe+0x12345B60]. File arithmetic resolves to 5. Its
computed upper bound resolves to 32705 (0x7FC1) on the recorded file.
These file results do not establish values in an initialized process.

At 0xA2F8D63-0xA2F8DA0 the handler reads a u16, saves the cursor after
that read, calls the nested reader, and sets the cursor to the saved value
plus the u16. Thus the prefix bounds a body after the two-byte length;
it is not a flat list of u16 data. The nested dispatcher initially reads
one byte and a dword. Component schemas and the full spawn contract remain
unresolved. Structural record splitting cannot validate those schemas.

### Rotation

Payload 180 supplies the four floats passed through 0x993D30 to
0xB3A240 on the traced conditional branches. The continuation
0xB3A278-0xB3A319 sums all four squares, takes a square root and applies
the reciprocal norm; the file normalization numerator at 0x61BEB48 is 1.
0xB3A370 stores the four components at object+0x5C0.
0x3E5100 also forms bilinear terms from the four inputs and tail-calls a
math routine. This establishes rotation-like four-component processing;
axis convention, handedness, angle convention, and live body correspondence
have not been tested. A raw-window listing includes adjacent code after
returns; it is not one continuous function, and only the reachable sequence
through the identified returns/branches is used as evidence.

### Removal

The full parse starts at 0xCA973F with a **one-byte read**. Next comes the
four-byte read at 0xCA97D6, then another one-byte read at 0xCA9881.
The remaining bytes are read as dwords until the cursor equals the payload
length (0xCA98A5-0xCA9974). This gives 1+4+1+4*N, not 4+1+4*N.
The final array entries are used in actor lookup at 0xCA9AC0-0xCA9ACA.
The leading dword and byte meanings remain raw.

Both normal and child-only body vtables resolve removal slot +0x110 to
0x5DA7B0 -> 0x8E24710. This validates state, conditionally visits linked
actors (including child removal transitions), handles a parent relation,
then calls detach 0x9CA890 and registry removal 0x534CC0. The dependency
chain is now recorded; calling it directly would still require evidenced
ownership, thread context, component state, and valid spawn bodies.

## Implementation and test scope

src/coop3663/ack_decode.* now contains:

- `decode_move_ack_transform_fields`: retains the spatial observation and
  reads four finite rotation components, rejects unusable norms and normalizes.
  The 0.5..2 norm interval is a diagnostic policy, not a native limit.
- `split_actor_load_file_records`: validates opcode/header and bounded
  length-prefixed bodies, exposing raw kind/key and borrowed opaque body spans.
  Callers must keep the source storage alive. It does not validate a spawn.
- `decode_remove_actor_fields`: validates the 1+4+1+4*N structural layout
  and retains the raw fields and key array. It performs no removal.

Record counts are capped at 256 as an offline resource policy; that is not
the game's actor limit. All input lengths and opcodes are checked before
field reads. No parser implicitly sets player readiness or turns a wire key
into a cross-PC identity.

Tests cover malformed prefixes and body lengths, every truncated prefix of
a multi-record fixture, empty envelopes, the count limit, mismatched opcode,
removal alignment and byte order, zero/tiny/extreme/non-finite rotation,
normalization, and invalid spatial components. An explicit synthetic mapping
also exercises decoded data through fake create, move, disconnect, and late
sample refusal. It is a fake lifecycle test, not native runtime evidence.

Validation: the full sibling RelWithDebInfo build and CTest passed 101/101
in 90.26 seconds after the parser implementations and focused regressions.
The composed lifecycle fixture and a guarded test assertion were added while
that run was in progress; they were then rebuilt and run independently,
with `coop3663 tests ok`. Thus the full-suite run predates that final test-only
addition. Later tracer metadata additions were compiled and run against the
file. The game was confirmed closed before the suite and after final tracing.
No failure was suppressed. git diff --check passed.

Logs: sibling Coop3663-review/spawn-heading-build.log and
spawn-heading-suite.log. The existing sibling build was used rather than
creating another build/dependency tree with scripts/build.ps1. This preserves
the configured x64 MSVC toolchain and runs its full CTest selection.

## Remaining work

Full native spawn bodies still contain component-specific virtual readers.
Resolve those schemas and initialization dependencies before constructing a
native actor backend. Establish the orientation/space conventions and native
thread/ownership contracts. Runtime success cannot be inferred from fixtures.
Keep live work deferred under the user's current instruction.
