# Codex review: corrected gate and nested reader trace

Reviewed 2026-11-14. Branch `grok/compat-guard`, published base `502577a`
plus the current local changes. This reviews the checkpoint requested by
the user, not completion of the two-player prototype.

## Disposition

The three prior decoder findings are resolved. Both unsupported field
decoders are removed, and the remaining literal header gate permits equality
at 0x7FC1. No new blocking source-code defect was found in these fixes.
One evidence-table correction is needed before its pointer paths are used
for further research. Working co-op remains unproved.

## P2: distinguish the two destination pointer chains

HANDOFF-CURRENT.md:57-62 labels all four stores as offsets from `inner`.
The listing does not establish that they share an object base. At
`exe+0x55F001-0x55F009`, the first three destinations are resolved through:

```text
S = qword[r14]
C = qword[S + 0x08]
T = qword[C + 0x68]
A = qword[T + 0x1A0]
stores at A + 0x550, A + 0x558, A + 0x55C
```

At `exe+0x55F0F4-0x55F101`, the eight-byte store instead resolves:

```text
S = qword[r14]
D = qword[S + 0xE8]
store at D + 0x90
```

These are different access paths; runtime equality or aliasing is unproved.
Correct the table and next-step wording to name A and D separately. Search
consumers using the complete object provenance, not a generic +0x90 offset.
The raw evidence is ack-trace-nested-20261114.txt:3288-3296 and 3347-3350.
This is a report/provenance defect, not a request to change gameplay memory.

## Evidence that checks out

- The nested reader trace supports the 239-byte consumption total for the
  fully parsed MoveActorAck path with this demonstrated reader. Scope that
  statement to the full parse path; the handler also has an early skip-body
  success branch. This is not a live packet observation or field semantics.
- The 17-byte movement decoder and flat u16 ActorLoad decoder are absent
  from the current source/tests. Unknown fields have not been adopted.
- ack_decode.cpp:13-25 checks span coverage, minimum header size, declared
  length equality, and the demonstrated inclusive literal bound. Tests cover
  below/equal/above, the flag exception, a short span, and a mismatch.
- The expanded trace identifies the direct reader and nested helpers that
  were previously omitted. The source-pointer offsets and byte-copy stores
  are useful leads, while coordinate space, heading meaning, actor identity,
  and lifecycle semantics remain unproved.
- An indirect call such as ActorLoad's `[result+0x130]` leaves transitive
  behavior unknown. Phrase the handoff's statement that the dispatcher
  does not allocate/remove an actor as "allocation/removal not established
  by this trace" until its target and downstream operations are resolved.

## Independent validation

The game was confirmed closed using a read-only process inventory. Built
all targets in the existing sibling NativeServer-build using MSVC
19.52.37029, x64 RelWithDebInfo. The canonical build script was not invoked;
the configured sibling build preserves the established source/toolchain and
avoids creating another dependency tree. Independent CTest result:
**101/101 passed, 0 failures, 93.40 seconds**. The new coop3663 test and
pkt3663_live_ctl both passed in this run.

Ran the rebuilt file tracer against the executable file only, never a game
process. All 3,359 lines exactly match ack-trace-nested-20261114.txt.
The executable SHA-256 still matches
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8.
No game launch, attachment, deployment, or gameplay memory operation was
performed. Production offsets, guards, wire protocol, and ASI integration
are unchanged. git diff --check passed.

Reviewer logs are in the sibling Coop3663-review directory:
latest-full-build.log, latest-full-suite.log, latest-ack-nested.txt.

## Continue in Cursor without another routine review stop

The current blocker is unfinished research, with independent file-only
work still possible. Apply the provenance correction while continuing the
end-to-end assignment in docs/CURSOR-END-TO-END-HANDOFF.md:

1. Trace consumers of A+0x550/+0x558/+0x55C and D+0x90 separately until
   their semantics are demonstrated or an exact unresolved dependency is
   identified. Position-like data is not world position without its space
   conversion evidence; an eight-byte value is not inherently a vector.
2. Resolve ActorLoad's relevant indirect dispatch target and follow the
   create path, and trace RemoveActorAck through its removal consumers.
   Do not stop after one callee or infer actor lifetime from a message name.
3. Select the route supported by that evidence and complete the independent
   implementation/tests. Maintain version isolation, bounded state,
   reproducible artifacts, and explicit unknown fields.
4. Continue self-review and test/fix loops. Ask the user only for actual game
   actions or a concrete external blocker. Bring one final prototype review
   package when demonstrated, or a precise blocker handoff if independent
   work truly cannot progress. Do not deploy speculative layouts or repeat
   the empty capture unchanged.

No commit or push was performed by this review.
