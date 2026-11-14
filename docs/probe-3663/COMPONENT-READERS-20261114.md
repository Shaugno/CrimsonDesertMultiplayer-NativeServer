# 3663 character component reader: offline checkpoint

No game launch or attachment, native actor calls, deployment, or guard changes.
The new decoder is linked only into the offline test library. It interprets
structure, not resource validity or successful character creation.

## Evidence and class resolution

Raw file trace: `component-readers-trace-20261114.txt`, SHA-256
`B7F620147F8C48FF31FD3BE02AC343770CC6E61C4B08B0645FEFEC25F5EFF524`.
The tracer checks the 3663 timestamp/base/image size before analysis.
It inventories file RTTI and resolves primary complete-object locators by
signature 1, object offset zero, exact type-descriptor RVA and self RVA.
It then finds the file pointer to each locator immediately before a vtable.
This is static class evidence, not a live actor/component inventory.

| Class | Name RVA | Locator | Vtable | Reader slot +0x58 |
|---|---|---|---|---|
| ClientCharacterControlActorComponent | 0x6F858B0 | 0x621C170 | 0x59C1660 | 0x2582580 |
| ClientTransformSyncActorComponent | 0x6F8B890 | 0x6226BE0 | 0x59CBA60 | 0x23DCEA0 |

The character-control initialization slot +0x18 resolves to
0x971AA0 -> 0x96E04C0; cleanup +0x38 resolves to
0x970F50 -> 0x96D56F0. Those listings are retained, but their complete
dependencies and lifecycle are not yet modeled. RTTI alone does not prove
either class is selected by every actor factory.

## Character-control wire consumption

`decode_character_control_fields` requires an independently established
component start. It returns borrowed spans and the consumed length; later
components remain unread. There is no automatic search for this layout in
opaque data and no integration into the native actor backend.

The full successful read branch at 0x2582580 consumes these fields in order:

| Order | Wire bytes | Evidence |
|---|---|---|
| Initial raw word | 4 | 0x25825B2 |
| Two slot blocks | flag byte; if ZERO, 2+4+8 additional bytes | 0x25825E0-0x2582676 |
| Optional byte | flag byte; if NONZERO, one value byte | 0x2582692-0x25826D4 |
| Two resource identifiers | 8 each | two calls to 0x2593A00 |
| Raw byte | 1 | 0x2582718 |
| String-like bytes | u32 count, then count bytes | 0x1643F30 |
| Raw suffix | 8+1+1+4 | 0x258274F, 0x258276B, 0x25827A3, 0x2582842 |
| Fixed entries | four conditional entries | 0x2582860-0x2582887 |
| Extra entries | u32 count, then that many conditional entries | 0x258289D-0x2582958 |

Each conditional entry is a u32 tag. Zero consumes only those four bytes;
nonzero consumes another 8+8+2+4+4+1 = 27 bytes. Helper
0x19D2DB0 -> 0xB9F63C0 computes the first destination as object+0x18
(file constant XOR verified by the tracer), then tests that field before
the remaining reads. The entry's native stride is 48 bytes, while its wire
size is 4 or 31 bytes. Those sizes must not be interchanged.

The scalar helper 0x2593A00 -> 0xD7A06B0 consumes EIGHT bytes, then resolves
them through a registry and stores a four-byte result. A four-byte destination
does not imply a four-byte wire ID. The offline decoder preserves the raw
16 identifier bytes and performs no registry lookup.

The string helper reads a four-byte length; both its small-stack branch and
large-buffer branch consume that many bytes. Encoding and converted native
string semantics are not claimed. Embedded zero bytes are preserved offline.
The scope helper 0x16436F0 does not consume wire bytes (previous trace).

The minimal structural component is 62 bytes. This is NOT a valid spawn
template: skipped slot blocks rely on native initialized state, identifiers
need resolution, and the complete actor component order is still unknown.
The parser caps string bytes at 4096 and extra entries at 256 as offline
resource policies, not native game limits. Nonzero flags are not restricted
to value 1. No guessed position, model name, health, or ownership labels are
assigned to the raw values.

## Transform component status

Its +0x58 reader is identified, but a full decoder is not implemented.
The trace includes nested helper 0x14D0870 -> 0xAD9DC20 and retains additional
variable readers 0x1797250, 0x1796F80 and 0x179D530 as unresolved dependencies.
Do not sum its top-level reads into a fixed spawn layout.

## Offline verification

The target rebuilt and printed `coop3663 tests ok`. Fixtures cover all eight
slot/optional combinations, noncanonical nonzero flags, every truncated
prefix of each fixture, borrowed-span locations, embedded zero bytes,
conditional entry sizes, extra-entry limits, and hostile length values.
A sentinel relation tail verifies that consumed length does not include
following data. This composition is synthetic; it does not prove the actual
factory puts the relation tail immediately after this component.

Full offline suite passed 101/101 in 90.26 seconds; the controller fixture
isolation fix also passed within that full run. git diff --check passed.
The game was confirmed closed during final verification. The existing configured
sibling MSVC build is reused instead of scripts/build.ps1's separate build
tree. Log: sibling `Coop3663-review/component-readers-suite.log`.

## Next work

Resolve the actual actor descriptor factory and ordered component inventory.
Finish the transform reader's nested variable structures. Identify resource
lookup requirements and constructor defaults for omitted character fields.
Only then compose a whole spawn record and model creation/rollback/cleanup.
Live native creation cannot be established by these fixtures.
