# Struct offsets checked by hand (3663 -> 3732)

Rows not covered by `offset-rows-3663-to-3732.tsv` (file-only, capstone, jumps followed):

| Offset | Use in the mod | 3663 evidence | 3732 result |
| --- | --- | --- | --- |
| Session size `0xD0` (B012) | allocation before NwSession ctor | `mov ecx,0xD0` at 0xE6F8CB7 before the ctor call 0xE6F8D00 | `mov ecx,0xD0` at 0xDF5F747 before 0xDF5F790 |
| Session out queue `+0xC8` (B020), request frame `+0x30` (B051) | tunnel send | Send 0x165C1F0 uses each once; `kSessionSendHead` bytes | Send 0x165DD70 uses each once; head bytes unchanged |
| TLS global-heap flag `+0x1FE` (B034) | allocator | NwSession ctor 0x165B9D0 uses it once | ctor 0x165D550 uses it once |
| Field users `+0x2A8/+0x2B0/+0x2B8`, `+0x4A`, `+0x2D8` (C021-C027) | native join row insert | AddActor 0xEB68740 (protected, jump-linked) 4/5/2/3/1 uses | AddActor 0xE3F1900: 4/5/2/3/1 |
| Same, removal | guest removal | RemoveActor 0xEB6C670 1/2/1/1 | RemoveActor 0xE3F2210 1/2/1/1 |
| Field mover `+0x508` (C028), owner `+0x180`, busy `+0x4A` | join | EnterField 0x2ECB130, AcquireField 0x2E08520 | 0x2ECF680, 0x2E0CFB0: equal counts |
| Loading state `controller+0x15C` | R9 sampler | writer 0x96D04A0 `mov word [rdi+0x15C]` | 0x9443080: one equal write |
| Sequencer marker `+0x259` (word `+0x258`) | R9 sampler | ctor 0x9C85530 `mov qword [rcx+0x258]` | ctor 0x99AF790: one equal write |

Rows reported DIFFERS (D004, C019, C057) differ only inside one protected-section
function each (0xBB1A850, 0xEB6F3D0, 0x9A21D40), whose obfuscation changes per
build; every non-protected evidence function for those rows matches exactly.

The layout changes 3732 does contain (for example `+0x250 -> +0x150` in the
character update 0x9729C0, `+0x270 -> +0x170` in its helper 0x9770C0,
`+0x154 -> +0x164` in 0x2F4AE80) are in fields the mod does not read.

## Packet opcodes

The packet descriptor table (3663 `0x6F14E10..0x6F487B0`, 3732 `0x6F1AE10..0x6F4E7B0`,
26,420 qwords, instances `+0 vtable, +8 kind, +0xC opcode`) is identical between builds
except for its 1,231 pointer slots (`tablecmp.py`). Every pointer shifts by `+0x5018..+0x6140`,
the local .rdata/.data deltas of P2, so no descriptor moved or swapped (`ptrdelta.py`).
Opcodes and kinds are unchanged in 3732; the mod's opcode constants stay as they are.
