# Offline packet scan of Crimson Desert 1.0.0.3663

Date: 2026-11-13. The executable was read as a data file by `pkt3663_scan`.
No process was opened. Nothing in this note is adopted. The 3576 gate was not
changed.

File identity matched `docs/COMPATIBILITY-3663.md`: 335,440,280 bytes,
timestamp `0x6AF5D198`, base `0x140000000`, `SizeOfImage` `0x144E9000`.
The scanner printed `identity=build-3663`, `scanned=1`, `adopted=0`.

Section names in this image are not reliable. `.reloc` is executable. The
scanner classifies sections by flags.

## How a name was paired

The search key is the class text, not a 3576 RVA. A hit must sit in a type
descriptor name that starts with `.?A`. An exact name is
`.?AVTrocTr<Key>@pa@@` or `.?AV<Key>@@`. A longer name that only contains the
key is recorded and is not that message.

The complete-object locator has signature 1 and its self RVA points at the
locator. The vtable is the qword after a pointer to that locator. The instance
is the target of a following `lea` / `mov [rip]` pair in an executable section.
Instance `+0x08`, `+0x0C`, `+0x14`, `+0x18`, and `+0x1A` are the offsets read
on this file. They are not copied payload sizes. No field is called a
coordinate or a rotation.

Every exact instance below has the same on-disk first qword, `0x145B405E8`.
That value is not adopted. Kind is 4 on the three request names and 1 on the
acknowledgement names. The request flag at `+0x14` is `0x100000`. The
acknowledgement flag is `0x2000000`. Min and max are 5 and 32710. Those
agreements were measured here.

## Exact classes

The opcode column is the `u16` at instance `+0x0C` on this file. The 3576
column is the old file's value and is not a 3663 allow-list entry.

| Class | 3663 opcode | 3576 opcode, not reused | Kind | Instance | Handler slot 2 |
| --- | ---: | ---: | ---: | --- | --- |
| `TrocTrMoveActorReq` | 2790 | 2833 | 4 | `exe+0x6F34560` | `exe+0x2C52CD0` |
| `TrocTrMoveActorAck` | 3008 | 3094 | 1 | `exe+0x6F161C0` | `exe+0xCA8AB0` |
| `TrocTrRotateActorReq` | 2296 | 2852 | 4 | `exe+0x6F345F0` | `exe+0x2C53270` |
| `TrocTrRotateActorAck` | 2664 | 3078 | 1 | `exe+0x6F16250` | `exe+0xCA8FB0` |
| `TrocTrStartActionReq` | 2431 | 2202 | 4 | `exe+0x6F33870` | `exe+0x2C46FA0` |
| `TrocTrStartActionAck` | 2800 | 2552 | 1 | `exe+0x6F14E10` | `exe+0xC9CA10` |
| `TrocTrActorLoadDataAck` | 2005 | 2005 | 1 | `exe+0x6F29110` | `exe+0xE22330` |
| `TrocTrUpdateVisiblePlayerAck` | 2913 | 2555 | 1 | `exe+0x6F25F70` | `exe+0xD6D2C0` |
| `TrocTrRemoveActorAck` | 3037 | 3067 | 1 | `exe+0x6F162E0` | `exe+0xCA9660` |

`ActorLoadDataAck` uses 2005 on this file as well. The assignment is the RTTI
name on `exe+0x6F29110`, not the shared number. Slot 1 of that vtable is
`exe+0xE22130`. The other eight exact vtables use slot 1 `exe+0xC99580`. Slot 0
is `exe+0xC995F0` on all nine. Payload layout is unresolved. These handler
addresses are not hook targets.

## Name that is not RemoveActorAck

`RemoveActorAck` also occurs inside `.?AVTrocTrManagementRemoveActorAck@pa@@`.
That class is not exact. Its instance is `exe+0x6F1CD30`, kind 1, `u16` 2636,
flag `0x2000000`. Opcode 2636 is not `RemoveActorAck`.

## Entry bytes

`mov ecx, 8030h` occurs once, at `exe+0x3A999D`, inside the pdata function
`exe+0x3A9960`–`exe+0x3A9B7F`. Two `mov edx, 8030h` sites, `exe+0x9A9878B` and
`exe+0x11FCF835`, are not inside a pdata function. They are not entry
candidates.

The 16-byte sample from the 3576 send note occurs 387 times in executable
bytes. The 3576 receive sample occurs 968 times. The scanner stored eight send
hits and then stopped the list. Frequency does not identify an entry. One of
the send hits is the first 16 bytes of `exe+0x3A9960`, which is also the
function that contains `mov ecx, 8030h`. Inside that function, the byte
sequence `28 80 00 00` occurs at `exe+0x3A9A0B` and `exe+0x3A9A3D`. Those two
sites were not decoded as displacements.

`exe+0x3A9960` is a file candidate only. It is not adopted, and it is not
declared to be the 3576 send function. No receive function was isolated.
