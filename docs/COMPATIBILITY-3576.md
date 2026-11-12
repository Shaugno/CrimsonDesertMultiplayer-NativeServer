# Compatibility assessment: Crimson Desert 1.0.0.3576

Date of this file pass: 2026-11-12.
Checkout: `CrimsonDesertMultiplayer-NativeServer` on `grok/compat-guard`.
Supported build of the mod remains **1.0.0.2976** (PE timestamp `0x6AB28F00`, mapped image base `0x140000000`).

This note records a read-only look at the installed executable and a proposed live session. It does not adopt addresses, replace offsets, open the compatibility guard, install hooks, or deploy the mod.

## Result

The installed game is 1.0.0.3576. The mod still rejects that identity. No address from this note has been copied into an offset header.

Labels used below:

| Label | Meaning |
|---|---|
| Verified file fact | Read from the installed executable on disk on 2026-11-12. |
| Verified non-match | The bytes at a 2976 RVA are not the object that RVA names on 2976. |
| File candidate | A vtable located in the file by the same Complete Object Locator rules as `cdmp_probe vtables`. No running process confirmed it. |
| Not observed | Needs a live process. No value was recorded. |

The load samples in `docs/probe-3576/CAPTURE.md` stay raw evidence. They are not a coordinate rule for detecting a load. Health stays unverified: the skill screen showed 600 and the status table's id 0 read 900/900. Chunk origin is still unread. No slot was adopted. A later process, pid 16920, resolved the same two title-screen slots and, after a load, all three. Its local position moved 10.502 on a walk. The image held no separate copy of that position. The local `+0x90` position tracked a round trip, and the rotation axes turned in place, in `docs/probe-3576/CAPTURE.md`. That position is local. Chunk origin and the independent world copy were not read. The in-world walk is in `docs/probe-3576/CAPTURE.md`. All three manager slots still agree: `+0x50` is `ClientChildOnlyInGameActor`, and `+0x68` then `+0x40` is `ClientCharacterControlActorComponent`. The `+0x2C0` pointer is readable and its `rtti` result is `(no RTTI)`, so the physics class is unverified. No permanent slot was chosen. The title-screen manager `findptr` is in `docs/probe-3576/CAPTURE.md`: one `ClientActorManager` at `0x3C9F8757800`, stored by three manager-rooted slots (`exe+0x71C09A8`, `exe+0x7233960`, `exe+0x7233AA0`). No single slot was adopted. The title-screen `info` and `vtables` capture is in `docs/probe-3576/CAPTURE.md`. That live pass verified the mapped base and the four vtable RVAs below. It did not adopt them.

## Installed executable

Path: `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`.

| Field | Value |
|---|---|
| File version | 1.0.0.3576 |
| Product version | 1.0.0.3576 |
| Size | 326,872,984 bytes |
| SHA-256 | `C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F` |
| PE | PE32+ (magic `0x20B`), machine `0x8664`, signature `0x4550`, 12 sections |
| `e_lfanew` | `0x80` |
| TimeDateStamp | `0x6AED2E85` |
| Preferred ImageBase | `0x140000000` |
| SizeOfImage | `0x13CC4000` |
| AddressOfEntryPoint | RVA `0x10288D60` (section `.tls$`) |
| File characteristics | `0x23` (`RELOCS_STRIPPED`, `EXECUTABLE_IMAGE`, `LARGE_ADDRESS_AWARE`) |
| DllCharacteristics | `0x8120` (`HIGH_ENTROPY_VA`, `NX_COMPAT`, `TERMINAL_SERVER_AWARE`) |

`DYNAMIC_BASE` is clear. The documented 2976 research copy is a different file: 384,521,624 bytes, SHA-256 `57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`, timestamp `0x6AB28F00` (`docs/research/ghidra/REVISION.md`). Older notes that give the Steam path above are describing that 2976 file. The file at that path now is the 3576 binary in the table.

The preferred base matches 2976, so a base-only check would accept this file. The guard also requires the timestamp. The mapped base of a running process was not read.

## Guard

`src/mod/game/build_guard.h` and `PlayerReader::init` still require mapped base `0x140000000` and timestamp `0x6AB28F00`. Timestamp `0x6AED2E85` is an unsupported in-game identity: no game-address reads, no hooks, no player source, and no network run. This assessment does not change that code.

`cdmp_probe` (`tools/probe/main.cpp`) attaches with `PROCESS_VM_READ | PROCESS_QUERY_INFORMATION` only. `info` prints `UNKNOWN BUILD` when the timestamp is not `0x6AB28F00`, and it still attaches. The probe is not the mod, and it was not run for this note.

## Section map

| Section | VA | Virtual size | Executable | Writable | Code flag |
|---|---|---|---|---|---|
| `.text1` | `0x1000` | `0x05596000` | yes | no | yes |
| `.arch` | `0x05597000` | `0x0173B000` | no | no | no |
| `.rodata` | `0x06CD2000` | `0x00879000` | no | yes | no |
| `.sdata` | `0x0754B000` | `0x002D9000` | no | no | no |
| `.edata` | `0x07824000` | `0x3000` | no | no | no |
| `.tls` | `0x07827000` | `0x30000` | no | no | no |
| `.ecode` | `0x07857000` | `0x001ED000` | no | no | no |
| `.tls$` | `0x07A44000` | `0x0BDE1F34` | yes | yes | yes |
| `.rsrc` | `0x13826000` | `0x001BAEA8` | no | no | yes |
| `.reloc` | `0x139E1000` | `0x248` | no | yes | yes |
| `.xpdata` | `0x139E2000` | `0x8DDC` | no | no | no |
| `.shared` | `0x139EB000` | `0x002D8C4C` | no | no | no |

There is no section named `.pdata` or `.xtls`. `.rodata` is file-backed only through VA `0x0703EA00` (raw size `0x36CA00`). The 2976 globals checked below sit inside that file-backed span.

Data directories that matter for the 2976 section names:

| Directory | RVA | Size | Section |
|---|---|---|---|
| Export | `0x139E2040` | `0x8D1C` | `.xpdata` |
| Import | `0x13929C00` | `0x76C` | `.rsrc` |
| Resource | `0x7827000` | `0x2F388` | `.tls` |
| Exception | `0x139EB000` | `0x2D8C4C` | `.shared` (the whole section) |

`in_real_code` in `src/mod/session2/guest_actor_diag_logic.h` allows a patch only when the bytes sit entirely in a section named `.rsrc`. On this file that section is not executable, and the import directory is inside it. Unwind data is `.shared`, not a section named `.pdata`. The entry point is in writable executable `.tls$`. Those names were not changed, and no patch site is proposed. `.tls$` is not a proposed hook target.

The certificate directory slot is a file offset in the PE specification. It is omitted here so it is not read as an RVA.

## Signature non-matches

The 2976 prologue bytes were compared with the file at the recorded RVA. All ten sites are in executable `.text1`. All ten differ. No replacement site was searched. These RVAs are not new hook addresses.

| Name | 2976 RVA | Result |
|---|---|---|
| `ClientPostPacket` | `0x35C670` | MISS |
| `ClientOnRecvPacket` | `0x35CA00` | MISS |
| `PointClimbEnter` | `0x691400` | MISS |
| `ActionLayerAdvance` | `0x1F63F70` | MISS |
| `ServerOnRecvPacket` | `0x2773C20` | MISS |
| `SessionSend` | `0x1437740` | MISS |
| `NwSessionCtor` | `0x1436EE0` | MISS |
| `QueuePush` | `0x35DF50` | MISS |
| `SaveMgrRefreshSlots` | `0x1870C20` | MISS |
| `AllocGlobal` | `0x47EFA78` | MISS |

`SaveMgrRefreshSlots` was compared with the 2976 head `E9 AB 86 36 0B`. The other heads are the byte arrays in `src/mod/game/offsets.h` and `src/mod/session2/offsets_s2.h`.

A later read-only search for 3576 client packet functions is in `docs/probe-3576/PACKET-CANDIDATES-3576.md`. No address from that note is adopted, and the compatibility guard stays closed. An experimental game-free observer for those candidates is in `tools/pkt3576`. It is not linked into the ASI, it is not installed in the game, and it does not adopt the addresses.

## Player-state chain

On 2976, `PlayerReader` walks a read-only chain (`src/mod/game/player_reader.cpp`, `src/mod/game/offsets.h`):

`[exe+0x6D69190] + 0x30` ClientActorManager, `+ 0x50` ClientChildOnlyInGameActor, `+ 0x68` component table, `+ 0x40` ClientCharacterControlActorComponent, `+ 0x140`, `+ 0x2C0` physics object. Local position is physics `+ 0x90`. World position adds the vec3 at `exe+0x6D56500`. Rotation is the axes at physics `+ 0x60 / + 0x70 / + 0x80`. A second world position is the vec3 at `exe+0x6D77858`. Health is component-table `+ 0x20`, then `+ 0x18`, `+ 0x58`, entry stride `0x90`, id `0`, int64 milli-units divided by 1000. While loading, a world position near `(0, 1000, 0)` (radius 5) is not broadcast.

### Verified non-matches at the 2976 RVAs

| 2976 site | RVA | What the file contains |
|---|---|---|
| WorldSystem slot | `0x6D69190` | Writable `.rodata`. First 32 bytes `18 D1 76 3D AA 82 11 BD DF 35 E8 BC EA 5C D1 3C 9B 20 6A BC B5 51 1D BC 00 00 80 3F D2 53 54 3F`. 0 of 4 image pointers. One `1.0f` (`00 00 80 3F`). |
| Chunk origin | `0x6D56500` | Writable `.rodata`. 0 of 4 image pointers. Contains `1.0f`. |
| Static player position | `0x6D77858` | Writable `.rodata`. 0 of 4 image pointers. Contains `1.0f`. |
| Controlled-actor vtable | `0x5585C40` | Executable `.text1`. Bytes start `40 53 48 83 EC 20`. Preceding qword is not an image pointer. |
| Status-component vtable | `0x558D868` | Executable `.text1`. Preceding qword is not an image pointer. |

Three other 2976 vtable RVAs sit in non-executable `.arch`, and the first four qwords are pointers into executable `.text1`. The qword immediately before each RVA is not a Complete Object Locator (signature is not 1, and the self-RVA does not match). They are not vtable starts, and they are not the RTTI vtables of the classes below.

| 2976 name | RVA | Section | Pointer-shaped | COL at RVA−8 |
|---|---|---|---|---|
| ClientActorManager vtable | `0x55B62C8` | `.arch` | yes | no |
| CharacterControl vtable | `0x55B5460` | `.arch` | yes | no |
| Physics-object vtable | `0x5D00F88` | `.arch` | yes | no |

### File candidates from RTTI

The file still contains MSVC RTTI. For each decorated name the pass required a trailing NUL, a TypeDescriptor at name − `0x10`, a Complete Object Locator with signature 1, the type-descriptor RVA at locator + `0x0C`, and the self-RVA at locator + `0x14` equal to the locator. It then found an absolute preferred-base pointer to that locator. The candidate vtable is that pointer plus 8, which is the rule in `cmd_vtables`. Each name occurred once. Each class had one locator, subobject offset 0, and one vtable candidate. Reading locator−8 back from the candidate returns signature 1, the same self-RVA, and the same class name. The first four slots are executable `.text1` pointers.

| Class | TypeDescriptor RVA | COL RVA | Vtable candidate | 2976 vtable |
|---|---|---|---|---|
| `.?AVClientActorManager@pa@@` | `0x6F01FC8` | `0x61A0C38` | `0x5946C40` | `0x55B62C8` |
| `.?AVClientChildOnlyInGameActor@pa@@` | `0x6EED278` | `0x6185438` | `0x5914D58` | `0x5585C40` |
| `.?AVClientCharacterControlActorComponent@pa@@` | `0x6F010F0` | `0x619F8C0` | `0x59459F0` | `0x55B5460` |
| `.?AVClientStatusActorComponent@pa@@` | `0x6EF0468` | `0x618A040` | `0x591C7C0` | `0x558D868` |

`.?AVWorldSystem@pa@@` is absent. The 2976 live notes already say WorldSystem has no RTTI (`docs/research/hallazgos-en-vivo.md`). No replacement global slot was searched.

`offsets.h` does not record an RTTI name for the physics object, so this pass did not choose a physics vtable. The callee RVAs inside the candidate vtables are not hook addresses.

On the title screen, `cdmp_probe vtables` printed one subobject-0 vtable for each class, at the RVA in this table: `exe+0x5946C40`, `exe+0x5914D58`, `exe+0x59459F0`, and `exe+0x591C7C0`. Those four vtable addresses are verified. `classes WorldSystem` printed `0 shown`. The raw stdout is in `docs/probe-3576/CAPTURE.md`. A verified vtable does not show that a live object uses the 2976 field offsets, that position and rotation move with the character, or that health tracks the on-screen bar. Those items stay unverified, and the RVAs stay out of the offset headers.

### Not observed

| Item | 2976 hypothesis, unchecked on 3576 |
|---|---|
| Mapped image base | Preferred base is `0x140000000`. The running base was not read. |
| WorldSystem global | Old slot is a verified non-match. No new slot. |
| Field offsets | `0x30`, `0x50`, `0x68`, `0x40`, `0x20`, `0x140`, `0x2C0`, axes `0x60/0x70/0x80`, position `0x90`. |
| Chunk origin and static world position | Both old globals are verified non-matches. |
| Rotation | Orthonormal axes, facing along −basis Z. |
| Health | Id 0, stride `0x90`, milli-units / 1000, max 300 on the 2976 character. |
| Loading | Park near `(0, 1000, 0)` within 5, then an in-world position. Radius and height are `kLoadingParkRadius` / `kLoadingParkHeight`. |

## Other assumptions left closed

Opcode numbers, payload sizes, the Troc instance range, the camera chain, and the save-manager refresh RVA are 2976 values. `scripts/host_save_restore.ps1` remains pinned to `game_build=1.0.0.2976` and `pe_timestamp=0x6AB28F00` and refuses other generations. The Steam app id `3321460` in `scripts/launch-2inst.ps1` was not re-checked against this install. The peer protocol is unchanged.

## Proposed probe session

Codex review corrected two errors in the first draft of this procedure. The executable commands and the capture checklist are in `docs/probe-3576/COMMANDS.md` and `docs/probe-3576/CHECKLIST.md`. Raw stdout goes in `docs/probe-3576/CAPTURE.md` as the session is run. Nothing in that log is an adopted offset.

This session does not authorize loading `cdmp.asi`, editing `build_guard.h` or `offsets.h`, or treating a match as an adopted address. `cdmp.asi` is not installed in the game directory. Start the installed game from Steam. Do not use `scripts/launch-2inst.ps1`.

`cdmp_probe` accepts an address only as `exe+RVA` or as a full hex number (`0x...` or hex digits). Chain and watch offsets are hex (`0x50` and `50` are the same offset). `parse_addr` rejects `<physics>+90`, `object+0x90`, and any other `+` that is not the `exe+` prefix. `rtti` takes the object address: `rtti 0xOBJECT`. `rtti exe+SLOT` treats the slot's bytes as an object and is the wrong command.

`chain` with no offsets reads the pointer stored at that address and prints `-> object .?AV...` for it. Each extra offset reads a pointer, adds the offset, and continues (`follow_chain`). `watch` uses the same walk, re-resolves every tick, and prints `<chain broken>` when a hop cannot be read.

### Correction 1: two different roots

`+0x30` is a field of WorldSystem. On 2976 it leads to `ClientActorManager`. It is not a field of the manager. The manager-to-controlled-actor hop is `+0x50`.

`findptr` of the ClientActorManager vtable finds the manager object. A static slot that stores that object pointer is **manager-rooted**. Confirm with `chain exe+SLOT` and no offsets: the `=>` line names `ClientActorManager`. From that slot the next hop is `0x50`. Do not insert `0x30`.

A slot that stores a WorldSystem pointer is **WorldSystem-rooted**. `chain exe+SLOT` will not name `ClientActorManager` (WorldSystem has no RTTI name in the 2976 notes, and `.?AVWorldSystem@pa@@` is absent from this file). From that slot the first hop is `0x30`, and `chain exe+SLOT 0x30` must name `ClientActorManager` before `0x50` is used. The 2976 WorldSystem RVA `0x6D69190` is a verified non-match on this file. Do not pass it as the slot unless a live `chain` names the manager.

| Root, confirmed by `chain exe+SLOT` | Controlled actor | Position vec3 |
|---|---|---|
| Manager: `=>` names `ClientActorManager` | `chain exe+SLOT 0x50` | `chain exe+SLOT 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3` |
| WorldSystem: `chain exe+SLOT 0x30` names `ClientActorManager` | `chain exe+SLOT 0x30 0x50` | `chain exe+SLOT 0x30 0x50 0x68 0x40 0x140 0x2C0 0x90 --type vec3` |

Using the WorldSystem hops on a manager-rooted slot dereferences the manager at `+0x30` and is not evidence.

### Correction 2: position and rotation addresses

Position and the three axes are `chain` commands whose last offset is `0x90`, `0x60`, `0x70`, or `0x80`, with `--type vec3`. The hops in front of that last offset are the root's hops through `0x2C0` from the table above. An absolute read is `read 0x<already-added address> --type vec3` after the address is added by hand. There is no probe syntax for "object plus offset".

1. **Identity.** `cdmp_probe info`. Accept the session only when the timestamp is `0x6AED2E85` and the line says `UNKNOWN BUILD`. Record `Base` and `size`. Commands below use `exe+RVA`. Any other timestamp ends the session. The mapped base is unverified until this command's stdout is in the capture log.

2. **Vtables.** For each decorated name in the candidate table, `cdmp_probe vtables <name>`. A file candidate becomes a verified vtable address only when the probe prints that one subobject-0 vtable at the candidate RVA. A different RVA, or a second vtable, stays a candidate. Do not copy either into a header during the session.

3. **Manager, then controlled actor.** On the title screen, `findptr exe+<verified ClientActorManager vtable> --heap`, then `rtti 0xHIT` on each aligned hit (`findptr` reports the address of the stored vtable pointer, which is the object when the vtable is at offset 0). `findptr 0xOBJECT` with no `--heap` lists image slots that store that pointer. Classify each slot with the table above before any further hop. A second `chain exe+SLOT` must repeat the same class. Extra slots stay candidates. `classes WorldSystem` records whether a live class of that name exists.

   The controlled actor is verified only when the root's actor command names `ClientChildOnlyInGameActor`. On 2976 that pointer was null on the title screen. A null title-screen result does not verify the actor and does not fail the manager. Repeat the actor command after a character is in the world.

4. **Character control and physics, in the world.** One hop at a time, still on the classified root. Accept a 2976 field offset only when the `=>` line names the expected class:

   - manager root: `0x50 0x68 0x40` names `ClientCharacterControlActorComponent`
   - WorldSystem root: `0x30 0x50 0x68 0x40` names that same class
   - one more `0x140 0x2C0`: record the physics class from the `=>` line. Do not assume a Havok name.

   `dump 0xPHYSICS --size 0x100` uses the absolute physics address from that `=>` line, before trusting `0x60`, `0x70`, `0x80`, and `0x90`.

5. **Position.** While the character stands still, run the position `chain` from the table. Ask for a short walk and run the same command again. The local vec3 is evidence only if it changes with the walk. World position on 2976 was local plus a chunk-origin vec3, checked against a second world-space vec3. Both old globals are verified non-matches, so those two reads are not `exe+0x6D56500` or `exe+0x6D77858`. A second copy, if one is found, is `read 0xABSOLUTE --type vec3`. Position is verified only when the walked samples move and, if both copies are found, local plus origin agrees with the second copy across the walk, allowing the one-frame lag described for 2976. Until then the cross-check is unverified.

6. **Rotation.** The same chain, with the last offset `0x60`, then `0x70`, then `0x80`, each `--type vec3`, before and after a turn. The axes are verified only when they stay orthonormal across the turn and the facing axis changes with the turn. Which axis is forward is recorded from that turn. It stays unverified if the character does not turn during the capture.

7. **Health.** From the classified root, the status hop is `0x68 0x20` after the actor hop (`0x50`, or `0x30 0x50`). The `=>` line must name `ClientStatusActorComponent` before that slot is accepted. The 2976 container layout is a hypothesis, read with chain offsets rather than `object+displacement`:

   - count: append `0x18 0x60 --type u32` (reads the container pointer, adds `0x60`, prints the u32)
   - entries: append `0x18 0x58` and copy the absolute pointer from the `=>` line
   - one entry: `dump 0xENTRIES --size <count * 0x90>` and read id at `+0x00`, current at `+0x08`, max at `+0x18` as int64 milli-units

   `read` of a field inside an entry uses `0x<entries + i*0x90 + field>` after the sum is computed by hand. Health is verified only when one stable id changes as the on-screen health changes, and current/max divided by 1000 match the number the operator reads off the bar. Record that displayed number beside the probe line. Do not assume id 0. Stamina is outside this milestone unless the same kind of sample is taken.

8. **Loading.** Start `watch` with the classified root's position chain and `--seconds 90`, then load a save during that window. `watch` reports the vec3 or `<chain broken>`, not which hop failed. While it runs, run the verbose position `chain` two or three times and keep those hops in the log so a broken pointer is distinguished from a parked coordinate. A parked coordinate is verified only when it holds across those samples and is then replaced by the in-world position from step 5. Compare the log with `(0, 1000, 0)` within 5; that park is a 2976 hypothesis until the log shows it.

After the log is reviewed, a later patch can propose a 3576 player table. That patch is not this note. It still must not install hooks, and it must not mark 3576 supported until the review accepts the table. `PlayerReader` and `build_guard` continue to require 1.0.0.2976 until then.

## Out of this milestone

Packet hooks, session send, save-manager refresh, the camera chain, and any change to `in_real_code` wait until the player-state log exists. Each of those needs its own signature, unwind entry, and section rule. `host_save_restore.ps1` stays pinned to 2976 generations until a 3576 export format exists.

## Local-to-world candidates

Not adopted. The guard stays closed. The 3576 file still has the 2976 subtract-into-`[rcx+0x90]` shape and the static XY/Z store, at new operands `exe+0x71AC2C0` and `exe+0x71CF140`. `exe+0x6D56500` and `exe+0x6D77858` are not those operands. Two stationary bursts in pid 16920, recorded in `docs/probe-3576/CAPTURE.md` section 17, share residual `(0.0000, -0.0503, 0.0000, 0.0000)`. Section 18, after a short walk of about 6.0237, records residual `(0.0000, -0.0506, 0.0000, 0.0000)`. Section 19 records an origin change at 11:16:47 from `(-8000, 0, -1000)` to `(-10000, 0, -4000)`. The rebase sample kept the world point. The next sample jumped. Section 20, after the character stopped, records residual `(0.0000, -0.0500, 0.0000, 0.0000)`. Y was not adjusted. Evidence and the corrected burst script are in `docs/probe-3576/LOCAL-TO-WORLD.md`. A separate read-only diagnostic, `cdmp_diag3576`, is described in `docs/probe-3576/DIAG3576.md`. It does not change `offsets.h` or open the guard.
