# Packet map for Crimson Desert 1.0.0.3576

Date: 2026-11-12. Offline read of the installed executable. No process was opened, and no capture code, offset table, or build guard was edited.

This is a focused map of nine co-op-critical messages plus the fifteen opcodes stored by the 2026-11-12 diagnostic capture. It does not adopt hook addresses, claim co-op, or establish actor identity, coordinates, or a complete payload layout.

## File identity

Installed image: `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`

| Field | Value |
| --- | --- |
| File size | 326,872,984 |
| SHA-256 | `C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F` |
| PE timestamp | `0x6AED2E85` |
| Preferred base | `0x140000000` |
| SizeOfImage | `0x13CC4000` |
| Magic | PE32+ `0x20B` |
| Exception directory | 248,753 functions |

The identity check matched before any RTTI or handler read. Build 1.0.0.2976 (`0x6AB28F00`) is a different image. `docs/research/data/troc_messages_2976.csv` was not modified and was not used to assign these names.

## How a name and an opcode were paired

Each class below has a `.text1` initializer of the form `lea rax, [derived vtable]` / `mov [instance], rax`. The instance's on-disk first qword is the base `pa::Troc` vtable `0x145AC5F10`. The initializer named here overwrites that slot with the derived vtable. Kind, opcode, and the min/max fields are already present in the instance bytes.

The derived vtable's RTTI was checked on disk: the pointer at `vtable-8` addresses a complete-object locator whose signature is 1 and whose self-RVA points back at that locator. The type-descriptor name is at locator type-descriptor `+0x10`. A second store in the same initializer image writes the base `pa::Troc` vtable; the class name comes only from the derived vtable.

Instance fields used below, read from the file:

| Offset | Meaning in this map |
| --- | --- |
| `+0x08` | `u32` kind |
| `+0x0C` | `u16` opcode |
| `+0x14` | `u32` flag (`0x100000` on the three Req classes, `0x2000000` on the six Ack classes) |
| `+0x18` / `+0x1A` | `u16` min / max, 5 and 32710 on all nine |
| `+0x21` | byte, 0 on all nine; the handler tests it |
| `+0x88` | `u32`, 2 on all nine |

The static min/max pair is not a measured payload size. A same-file scan of `.text1` initializer stores found 1,175 `TrocTr` instances and 1,175 distinct opcodes at `+0x0C`. This pass re-read the instances listed below and their derived-vtable names. Kind on that full set was 4 for 496 Req-suffixed classes and 1 for 554 Ack-suffixed and 21 Nak-suffixed classes. The suffix and the kind agree for these nine. Wire direction is stated only where a stored capture record exists.

Callable vtable length is three slots. Slot 0 is `exe+0xC61250` for every class below. Slot 1 is `exe+0xC611E0` except `ActorLoadDataAck`, whose slot 1 is `exe+0xDE9080` and has no pdata entry. Slot 2 is the per-message handler. The fourth pointer lands in `.arch` and is not code.

## Captured opcodes

Source of counts and lengths: `docs/probe-3576/LIVE-CAPTURE-REVIEW-20261112.txt`, from the standing and walking archives under `backups/NativeServer/20261112-215320-pkt3576-live-evidence`. All 70 stored rows had `payload_u16 + 5 ==` the raw stack `u16`. Stored send slots were 0 and stored receive slots were -1. Prefix length was `min(packet bytes, 16)`. The walking log kept 64 of 3,486 counted observations (`omitted=3422`). Those omitted rows were not saved. There are no per-record timestamps.

The class in this table is the 3576 instance whose `u16` at `+0x0C` equals the captured opcode, named from that instance's derived vtable.

| Opcode | Stored path | Count | Packet bytes | 3576 class | Kind | Instance |
| --- | --- | --- | --- | --- | --- | --- |
| 2272 | receive | 2 | 10 | `UpdateMissionUIStateListAck` | 1 | `exe+0x6EA0FD0` |
| 2360 | receive | 2 | 3532 | `UpdateMissionListAck` | 1 | `exe+0x6EA0C70` |
| 2366 | receive | 1 | 10 | `UpdateFieldGimmickSaveDataToResetAck` | 1 | `exe+0x6E97370` |
| 2779 | receive | 1 | 25 | `ServerTimeInfoAck` | 1 | `exe+0x6E9E990` |
| 3143 | receive | 14 | 123 | `UpdateElementalAck` | 1 | `exe+0x6E948B0` |
| 2155 | send | 3 | 251 | `GimmickExecuteFrameEventReq` | 4 | `exe+0x6EB73E0` |
| 2202 | send | 6 | 734 | `StartActionReq` | 4 | `exe+0x6EAF550` |
| 2276 | send | 1 | 166 | `GamePlayTriggerInOutReq` | 4 | `exe+0x6EB0510` |
| 2394 | send | 1 | 677 | `ChangeAIStateReq` | 4 | `exe+0x6EB0CF0` |
| 2410 | send | 6 | 9 | `ProcessAfterBeginFrameEventReq` | 4 | `exe+0x6EAF670` |
| 2606 | send | 15 | 125 | `StartAIFunctionReq` | 4 | `exe+0x6EB0E10` |
| 2787 | send | 1 | 249 | `UpdateGimmickReq` | 4 | `exe+0x6EB6ED0` |
| 2833 | send | 15 | 523 | `MoveActorReq` | 4 | `exe+0x6EB0240` |
| 2852 | send | 1 | 181 | `RotateActorReq` | 4 | `exe+0x6EB02D0` |
| 3061 | send | 1 | 60 | `ActivateSpawningPoolReq` | 4 | `exe+0x6EB34E0` |

`ServerTimeInfoAck` is the one captured class whose `+0x14` value is `0x2000002` rather than `0x2000000`. Opcode 2360 is `UpdateMissionListAck` on this image. The 2976 catalog's label for numeric 2360 is a different build's row. Opcode 3143 has a 3576 class; the 2976 catalog has no row with that number. Every stored kind-4 opcode was on the send path, and every stored kind-1 opcode was on the receive path. Frequency was not used to pick a movement name.

## Historical 2976 numbers

These rows are the preserved catalog only. Matching or nearby numbers are not evidence that a layout survived.

| Class | 2976 opcode in the catalog | 3576 opcode from the instance |
| --- | --- | --- |
| `MoveActorReq` | 2556 | 2833 |
| `MoveActorAck` | 2697 | 3094 |
| `RotateActorReq` | 2828 | 2852 |
| `RotateActorAck` | 2687 | 3078 |
| `StartActionReq` | 2223 | 2202 |
| `StartActionAck` | 2991 | 2552 |
| `ActorLoadDataAck` | 2005 | 2005 |
| `UpdateVisiblePlayerAck` | 2135 | 2555 |
| `RemoveActorAck` | 2516 | 3067 |

`ActorLoadDataAck` uses 2005 on both builds. The 3576 assignment is the RTTI name on instance `exe+0x6EA4EF0`, not the shared number. The 2976 research note `docs/research/mirror-ack-formats.md` records payload sizes for that older build (MoveActorReq 514 / Ack 235, StartActionReq and Ack 725, RotateActorReq 176 / Ack 177). Those sizes are not copied into this map. The stored 3576 totals for opcodes 2833, 2852, and 2202 are 523, 181, and 734 bytes, which include the five-byte frame.

## Deserializer used by the handlers

`DeserializerMemory` read is vtable slot `+0x28`, function `exe+0x1602F60`. Its on-disk body is in `.text1`:

```
mov r9, rcx
mov ecx, [rcx+0x2C]
lea eax, [rcx+r8]
cmp [r9+0x28], eax
jnb success
xor al, al
ret
success:
test r8d, r8d
jz done
copy one byte from [buffer_base + cursor] to the destination
inc dword [r9+0x2C]
dec r8
jnz copy
mov al, 1
ret
```

`r8` is the byte count. Object `+0x2C` is the cursor. Object `+0x28` is the limit. Object `+0x20` is the source base used by the copy. A failing bounds check returns false and copies nothing. Handlers compare this slot with `exe+0x3A37D0` and take a failure path when the slot points there. `exe+0x3A37D0` is not a read.

The handlers construct the object with vtable `0x145AC1770` (`IDeserializer`) and then `0x145AC17F8` (`DeserializerMemory`), set the source to `packet+5`, and store the payload `u16` and a zero cursor on the stack. A read in this map is a direct call to `exe+0x1602F60`, or an indirect call through that `+0x28` slot, with the count taken from `r8` when the count is an immediate.

A walk that treated every `r8` immediate before an arbitrary indirect call as a read produced the same three 2,049-byte sites inside unrelated handlers. Those sites do not use slot `+0x28`. They are not layout evidence. The constant `0x1FE` on the failure epilogue is a TLS index used before `exe+0x4B0248C`, not a byte count.

## Frame check

Eight handlers are in `.text1`. Each one contains all three of these sites before any deserializer read. `MoveActorReq` at `exe+0x2BED248` shows the shape: `rcx` is `this`, `rdx` is the result pointer, `r8` is the parameter block, and `[r8+0x18]` is the packet.

```
mov r14, [r8+0x18]
movzx ecx, word ptr [r14+3]
movzx eax, word ptr [r8+0x10]
sub rax, 5
cmp rax, rcx
jnz fail
cmp byte ptr [r8+0x38], 0
jnz skip_cap
mov eax, 0x7FC1
cmp ax, cx
jb fail
skip_cap:
cmp byte ptr [this+0x21], 0
jz body
```

`0x7FC1` is 32,705. It is a payload cap applied only when the byte at `params+0x38` is 0. It is a different number from the instance max 32,710. The failure path stores the dword at file VA `0x14712DFE4` (`.rodata` RVA `0x712DFE4`) into the result pointer. The success-path walk below follows the other side of that branch.

| Class | `sub rax, 5` | cap `0x7FC1` | compare `[this+0x21]` | cursor compare present in the function |
| --- | --- | --- | --- | --- |
| `MoveActorReq` | `exe+0x2BED266` | `exe+0x2BED27A` | `exe+0x2BED288` | `cmp [rbp-0x54], eax` at `exe+0x2BED500` |
| `MoveActorAck` | `exe+0xC70749` | `exe+0xC7075C` | `exe+0xC7076A` | `cmp [rbp-0x54], eax` at `exe+0xC709C3` |
| `RotateActorReq` | `exe+0x2BED806` | `exe+0x2BED81A` | `exe+0x2BED828` | `cmp [rbp-0x54], eax` at `exe+0x2BEDAD4` |
| `RotateActorAck` | `exe+0xC70C49` | `exe+0xC70C5C` | `exe+0xC70C6A` | `cmp [rbp-0x4C], eax` at `exe+0xC70FAC` |
| `StartActionReq` | `exe+0x2BE1565` | `exe+0x2BE1579` | `exe+0x2BE1589` | `cmp [rbp-0x34], eax` at `exe+0x2BE1752` |
| `StartActionAck` | `exe+0xC646A9` | `exe+0xC646BC` | `exe+0xC646CA` | `cmp [rbp-0x54], eax` at `exe+0xC648F2` |
| `UpdateVisiblePlayerAck` | `exe+0xD3484F` | `exe+0xD34862` | `exe+0xD34870` | none against the constructed cursor slot |
| `RemoveActorAck` | `exe+0xC71303` | `exe+0xC71316` | `exe+0xC71324` | none against the constructed cursor slot |

The cursor compare is a compare of the stack cursor against a register. The walk stopped on an earlier value-dependent branch, so these compares are sites in the function, not a proof that every success path consumes the payload exactly. `UpdateVisiblePlayerAck` and `RemoveActorAck` construct a cursor slot and do not compare that slot later in the decoded body.

## The nine messages

Direction for `MoveActorReq`, `RotateActorReq`, and `StartActionReq` is client-to-server: the stored records are on the send hook, and the instance kind is 4. The six Ack opcodes were not among the 70 stored rows. Their instance kind is 1, which is the kind of every stored receive opcode. That is registration evidence, not an observation of these six opcodes.

### MoveActorReq

Opcode 2833 (`0xB11`), kind 4, instance `exe+0x6EB0240`, derived vtable `exe+0x5EEE898`, handler `exe+0x2BED230`..`0x2BED7C5`, initializer lea `exe+0x2557D5`.

The success-path walk reached one read, then entered a helper and stopped at the depth limit:

1. 4 bytes to `[rbp+0x280]` at `exe+0x2BED3BE`, direct call `exe+0x1602F60`.
2. Helper `exe+0x263A2D0` from `exe+0x2BED3E9`, which calls `exe+0x148C430`, which calls `exe+0x148C340`.

Further read sites exist in the handler and were not reached: 8 bytes to `[rbp-0x48]` at `exe+0x2BED44D`, 4 bytes to `[rbp-0x40]` at `exe+0x2BED477`, and helper `exe+0x26458A0` at `exe+0x2BED4C0`. No field names are assigned to the 8-byte or 4-byte destinations. The stored packet length is 523 bytes on 15 send records. That length is not a field layout.

### MoveActorAck

Opcode 3094 (`0xC16`), kind 1, instance `exe+0x6E920C0`, derived vtable `exe+0x599B970`, handler `exe+0xC70710`..`0xC70C0C`, initializer lea `exe+0x149855`. Not present in the stored sample.

The walk reached a 4-byte read to `[rbp+0x148]` at `exe+0xC70879`, then stopped at `jz exe+0xC708D5` (`exe+0xC708A4`). The fall-through masks the value just read with `0x30000000`, compares it with `0x10000000`, and can store the `.rodata` dword at `0x14712E044` into the result. The other side calls helper `exe+0x263A2D0`. Later unread sites in the function: 1 byte to `[rbp+0x140]` at `exe+0xC70924`, 8 bytes to `[rbp-0x40]` at `exe+0xC70972`, and 4 bytes to `[rbp-0x38]` at `exe+0xC709A3`.

### RotateActorReq

Opcode 2852 (`0xB24`), kind 4, instance `exe+0x6EB02D0`, derived vtable `exe+0x5EEE878`, handler `exe+0x2BED7D0`..`0x2BEDD94`, initializer lea `exe+0x255825`.

Reads reached, in order, before a branch inside a nested helper:

1. 4 bytes to `[rbp+0x100]` at `exe+0x2BED8BC`.
2. 4 bytes to `[rbp-0x38]` at `exe+0x2BED905`.
3. 4 bytes to `[rbp-0x34]` at `exe+0x2BED92F`.
4. 4 bytes to `[rbp-0x30]` at `exe+0x2BED959`.
5. 4 bytes to `[rbp-0x2C]` at `exe+0x2BED983`.
6. Helper `exe+0x148C340` from `exe+0x2BED9B6`, then `exe+0x16039B0`, which reads 12 bytes at `exe+0x16039CD` and then `jz exe+0x1603A06`. The fall-through of that `jz` sets up another 12-byte read through slot `+0x28`. The taken side returns.

Further handler sites not reached after that branch: 8 bytes at `exe+0x2BED9EA`, 8 bytes at `exe+0x2BEDA44`, 8 bytes at `exe+0x2BEDA87`, 4 bytes at `exe+0x2BEDAB1`, and a second call to `exe+0x148C340` at `exe+0x2BED9FD`. The stored packet length is 181 bytes on one send record.

### RotateActorAck

Opcode 3078 (`0xC06`), kind 1, instance `exe+0x6E92150`, derived vtable `exe+0x599B890`, handler `exe+0xC70C10`..`0xC712B4`, initializer lea `exe+0x1498A5`. Not present in the stored sample.

The walk reached a 4-byte read to `[rbp+0x118]` at `exe+0xC70D09`, then the same style of value test at `jz exe+0xC70D65` (`exe+0xC70D34`). Later sites in the function, not reached: 4-byte reads at `exe+0xC70D8B`, `exe+0xC70DBC`, `exe+0xC70DED`, and `exe+0xC70E1E`; 8-byte reads at `exe+0xC70E89`, `exe+0xC70EE7`, and `exe+0xC70F62`; a 1-byte read at `exe+0xC70F1B`; a 4-byte read at `exe+0xC70F8C`; helpers `exe+0x148C340` at `exe+0xC70E4E` and `exe+0xC70E9C`.

### StartActionReq

Opcode 2202 (`0x89A`), kind 4, instance `exe+0x6EAF550`, derived vtable `exe+0x5EEE318`, handler `exe+0x2BE1530`..`0x2BE1C08`, initializer lea `exe+0x2550A5`.

Reads reached:

1. 4 bytes to `[rbp+0x388]` at `exe+0x2BE169A`.
2. Helper `exe+0x2638560` from `exe+0x2BE16D5`, which reads 4 bytes at `exe+0x26385A0` and then `jz exe+0x263886D`. The fall-through sets up a 4-byte read and then a 2-byte read. The other side winds down a counter at `[rdx+0x18]`.

Two earlier backward edges (`exe+0x2BE15B6` and `exe+0x2BE1630`) are initialization loops with no read in the body. The walk left them by the exit. A 1-byte read at `exe+0x2BE1743` and helper `exe+0x2638B60` at `exe+0x2BE16F4` were not reached. The stored packet length is 734 bytes on six send records.

### StartActionAck

Opcode 2552 (`0x9F8`), kind 1, instance `exe+0x6E90D10`, derived vtable `exe+0x599BCB0`, handler `exe+0xC64670`..`0xC64B5C`, initializer lea `exe+0x148D65`. Not present in the stored sample.

The walk reached a 4-byte read to `[rbp+0x378]` at `exe+0xC647F8`, then `jz exe+0xC64854` (`exe+0xC64823`) on that value. Linear decoding of this function stopped at `exe+0xC64B40`, 28 bytes before the pdata end. Sites inside the decoded range and not reached: 1 byte at `exe+0xC648B2`, helper `exe+0x2638560` at `exe+0xC64875`, and helper `exe+0x2638B60` at `exe+0xC648C8`.

### ActorLoadDataAck

Opcode 2005 (`0x7D5`), kind 1, instance `exe+0x6EA4EF0`, derived vtable `exe+0x59D94D0`, initializer lea `exe+0x160FB5`. Slot 1 `exe+0xDE9080` and slot 2 `exe+0xDE9280` have no pdata entries. The first instruction of slot 2 is `jmp exe+0xA0B04E0`, and that target is in `.tls$`, which is writable and executable. The body was not disassembled. Registration stands. The serializer layout is unresolved. This opcode was not in the stored sample. The shared numeric value 2005 in the 2976 catalog is not a layout.

### UpdateVisiblePlayerAck

Opcode 2555 (`0x9FB`), kind 1, instance `exe+0x6EA1DE0`, derived vtable `exe+0x5997C70`, handler `exe+0xD34810`..`0xD34F65`, initializer lea `exe+0x1524F5`. Not present in the stored sample.

Reads reached:

1. 1 byte to `[rbp+0x10B0]` at `exe+0xD34902`.
2. 4 bytes to `[rbp-0x50]` at `exe+0xD34987`.

The walk then stopped at `jz exe+0xD34A04` (`exe+0xD34997`). One side masks the 4-byte value with `0x30000000` and can store `0x14712E044` into the result. The other side sets up an 8-byte read to `[rbp-0x48]` at `exe+0xD34A2A`. Also present and not reached: a 1-byte read at `exe+0xD34A5E` and a 4-byte read at `exe+0xD34B1A` whose destination is not a simple stack slot.

### RemoveActorAck

Opcode 3067 (`0xBFB`), kind 1, instance `exe+0x6E921E0`, derived vtable `exe+0x599B8B0`, handler `exe+0xC712C0`..`0xC71A52`, initializer lea `exe+0x1498F5`. Not present in the stored sample.

The first read, at `exe+0xC713B7` into `[rbp+0x10A0]`, calls `exe+0x1602F60` with a non-immediate `r8`. The count is unresolved. The walk then reached a 4-byte read to `[rbp-0x50]` at `exe+0xC71444` and stopped at `jz exe+0xC714D0` (`exe+0xC71454`), the same mask test as the other Ack handlers. The far side sets `r8d` from `ebx` and calls the read slot at `exe+0xC714EF` into `[rbp+0x10A8]`. A later 4-byte read at `exe+0xC715B3` uses destination `[rax+rcx*4]`. Neither of those counts is a proven constant.

## Unresolved

- No message has a proven total payload size. Nested helpers (`exe+0x263A2D0`, `exe+0x2638560`, `exe+0x2638B60`, `exe+0x148C340`, `exe+0x16039B0`) contain further reads and branches, including a 12-byte read that is followed by another conditional 12-byte read.
- The six Ack opcodes were not in the stored records. Their receive direction is inferred from kind 1 and from the fifteen stored rows, not from observing these opcodes.
- `ActorLoadDataAck`'s handler leaves `.text1` for `.tls$`. No layout is taken from that section.
- `RemoveActorAck`'s first read has a computed count. `UpdateVisiblePlayerAck` and `RemoveActorAck` have no compare of the constructed cursor slot.
- Field meaning is unresolved. An 8-byte read and a 4-byte read are sizes and stack destinations only.
- The omitted 3,422 walking observations, the 16-byte prefixes, and the lack of timestamps block actor identity, coordinates, and action timing.
- Hook candidates `exe+0x3A6310` and `exe+0x3A66A0` stay unadopted. Production offsets and the ASI guard stay on the 2976 timestamp.

## Proposed capture change

Implemented in the experimental `tools/pkt3576` collector and awaiting review. This is not approval for another live request or for packet replay. Prefix-only archives cannot be replayed as complete packets. A truncated 4096-byte `ActorLoadDataAck` sample is not a spawn packet; its maximum stays unknown (`kActorLoadDataAckMax` is 0).

The change is limited to the experimental `tools/pkt3576` observer. It does not open the ASI guard, does not call `sniffer_install`, and does not write `offsets.h` or `offsets_s2.h`.

1. Allow-list, checked in `Observer::observe` before the payload copy. Read the leading `u16` of the buffer at `r9` only when the claimed length is at least 2. Keep the record when that `u16` is one of: 2833, 2852, 2202, 3094, 3078, 2552, 2005, 2555, 3067. Any other opcode increments a skip counter and returns. The callback stays nonblocking. The other twelve captured opcodes stay out of this list until a later review.

2. Complete-copy bound. `candidates.h` `kPrefixCap` is 16, `Record::prefix` is `uint8_t[16]`, and `prefix_len` is a `uint8_t`. A full copy needs a separate buffer and a `uint16_t` length, plus a truncated flag. Copy `min(claimed stack u16 at [rsp+0x28], 4096)`. 4096 covers the largest stored packet among the three observed requests (734) and leaves room for the six Acks, whose sizes were not stored. A larger claim is truncated, counted, and not treated as a complete packet. Do not copy the registration max of 32,710 bytes on the callback thread.

3. Queue and output stay inside the existing limits: queue 2..256 and records 1..256 (`live_runner.cpp`). For this capture, set both production values to 256. The live run used 64 and omitted 3,422 rows. A full queue still drops and counts. `publish_record` still omits and counts once `output_records` is reached. `capture.cpp` `write_line` formats into a 384-byte stack buffer; a 4,096-byte payload needs a bounded writer buffer sized from the copy cap. No file I/O on the callback thread.

4. Per-record time. Read `QueryPerformanceCounter` in `observe` and store it on the record. The existing writer thread prints it. Do not take a lock in the callback.

5. Phase marks come from the controller, as explicit `stand`, `walk`, and `turn` lines between operator actions, or as three separate arm/stop windows. The hook does not infer a phase from opcode frequency.

6. Lifetime stays as it is. Callbacks return immediately when the gate is closed or the queue is full. Removal still disables hooks, waits for a zero callback count, and joins the writer before freeing the queue. `Retained` leaves the DLL mapped. The controller does not force-unload and does not terminate the target. The production controller still rejects `--fixture`. The version gate still requires timestamp `0x6AED2E85`, base `0x140000000`, and `SizeOfImage` `0x13CC4000`, and still rejects `0x6AB28F00`.

7. Game-free test of the allow-list, the 4,096-byte cap, truncation, the 256 record cap, the timestamp field, and a controller mark line before any new live request. Existing prefix-only archives are not replay input. This proposal does not authorize packet injection or replay.
