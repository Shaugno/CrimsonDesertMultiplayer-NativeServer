# Codex takeover: movement consumer investigation

2026-11-14, local work on base 502577a. The user assigned Codex the
implementation/investigation. Existing Grok work was preserved. No game
process was opened, launched, or modified; the tool reads the file only.

Extended pkt3663_ack_trace to follow movement consumers and removal
transition/lookup thunks. Raw evidence: ack-trace-consumers-20261114.txt.
Targets are research locations, not callable APIs or production offsets.

## New spatial evidence

The MoveActorAck full-parse map puts object+0xC0 at payload offset 196.
For the already traced type-1 branch, 0x55EF60 passes that address to
0x993BA0. Conditional branches in that function pass it to 0x5E90A0,
which jumps to 0x8E69280. The latter treats the three components as float32.

At 0x8E69288 it loads a float at image RVA 0x61BF1C8. The file value is
1000. It divides the first/third components by that value, converts the
results to integers, keeps signed 16-bit cell indices, and subtracts
cell*1000 from those components. The middle component is carried through
a float addition with zero. This supports spatial semantics; it does not
yet identify world space, axis names, or live actor readiness.

At 0xB39E40, the next consumer reconstructs first/third values from signed
cell indices and local components, then follows conditional validation and
stores the 16-byte cell/local representation at destination+0x5AC.
Its parent flag paths and virtual notification remain dependencies. The
numeric-validation callee at 0x4C2764E has not been characterized here.

Payload 196/200/204 are therefore stronger spatial candidates than arbitrary
three-float guesses. The 16 bytes at payload 180 remain unproved heading;
the eight bytes at payload 227 remain unclassified and go to another object.
No movement decoder or live feed was enabled by this finding.

## Lifecycle leads

The extended trace now follows the allocation and removal paths beyond
the first thunk. File RTTI corroborates the allocated user vtable as
ClientUserActor (0x59C1D90), and the two body allocation branches as
ClientNormalInGameActor (0x5991108) and ClientChildOnlyInGameActor
(0x5990BF0). The adjacent table 0x5990B68 is WindowsRWLock, not an actor;
the body constructors install it at a separate object offset.

The user factory at 0x5344D0 allocates 0x190 bytes, initializes the user
object through slot +0x40, and inserts it into registry storage. Its reader
slot +0x130 reaches 0xCC93A00 and then the separate continuation at
0xCC93A38, reading a four-byte field and component-specific bodies.
The initialize path also allocates 0x1C8 bytes for component slots.

The other ActorLoad branch calls 0x9C7660, with body allocation callees
0x5346C0 -> 0x8BEE740 and 0x5349D0 -> 0x8BF50A0. Both allocate 0x150
bytes and install the body types above. Its payload reader follows
0x198E820 -> 0xB846A40 and includes component virtual reads; a complete
spawn template is not established by these allocations alone.

Removal follows 0x2294C50 -> 0xCC82570 into actor state transition and
slot +0x110. On the demonstrated user vtable that resolves through
0x9BE570 -> 0x973AF70, calls detach 0x9CA890, and registry removal
0x534CC0. The latter checks object membership, updates key-index storage,
and removes array entries. These observations establish lifecycle
dependencies, not a complete safe native invocation contract. Body-specific
virtual targets and component cleanup still need resolution.

The ActorLoad dispatch lookup at 0x9C7C70 is a jump to 0x9740660.
RemoveActorAck's transition callee at 0x2294C50 jumps to 0xCC82570.
Their full bounded listings are now retained. Indirect dispatch and object
identity still need tracing; their names do not establish create/remove APIs.
The first raw-window attempt showed undecodable bytes after these jumps;
the final trace follows the thunks instead. Keep that limitation explicit.

## Verification and next work

Implemented `decode_move_ack_spatial_fields` in src/coop3663/ack_decode.*.
This offline-only function checks the header, complete length 244, and
file descriptor opcode 3008 before extracting the raw word at payload 0
and finite float32 components at 196/200/204. It decodes little-endian
words explicitly. It returns no heading, readiness, or coordinate-space
claim and cannot feed native actors by itself. Tests use labeled synthetic
frames and check distinct component values, truncation, wrong descriptor,
non-finite components, and incorrect length.

Independent full RelWithDebInfo build and CTest after that implementation:
**101/101 passed, 98.97 seconds**. Logs: sibling
Coop3663-review/takeover-build.log and takeover-suite.log. The game was
checked closed before the run and never launched or attached to. Subsequent
tracer-only additions were rebuilt and exercised against the file; another
full test run was not necessary for those listing changes.

Rebuilt the offline tracer successfully with MSVC 19.52, x64 RelWithDebInfo
and ran it against the installed executable file. The output reports the
strict timestamp/base/size match and adopted=0. No runtime code changed.
The independent 101/101 suite pass in 93.40 seconds predates these tracer-only
extensions; it is not a new run for them. Compilation and file execution
validate this research-tool change. No redundant full suite was run.

Continue tracing the destination's spatial application and the lookup/removal
targets, resolve actor ownership/indirect dispatch, and corroborate heading.
Select the gameplay route only when those facts are established. Keep raw
archives, old-build paths, production guards, and saves unchanged.
