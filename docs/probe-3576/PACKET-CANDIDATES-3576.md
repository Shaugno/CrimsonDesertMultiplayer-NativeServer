# 3576 client packet candidates

Date: 2026-11-12. Checkout: `CrimsonDesertMultiplayer-NativeServer` on `grok/compat-guard`.

These are file candidates for Codex review. No address is adopted. `src/mod/game/offsets.h`, `src/mod/session2/offsets_s2.h`, and `src/mod/game/build_guard.h` were not edited. The ASI guard stays closed. Nothing was installed, hooked, or deployed. The game was not launched.

An experimental game-free observer for the plan below lives in `tools/pkt3576`. It is not linked into the ASI and it was not attached to the game.

Raw scanner output: `docs/probe-3576/packet-scan-3576.txt`.
Reproduce with `powershell -NoProfile -File tools\probe-3576\run_packet_scan.ps1`.

## Identity

| Field | Value |
|---|---|
| Path | `D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe` |
| Size | 326,872,984 |
| SHA-256 | `C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F` |
| PE timestamp | `0x6AED2E85` |
| Preferred base | `0x140000000` |

The scanner refuses any other size or timestamp. This run matched. `cdmp.asi` and `winmm.dll` were absent from `bin64` after the run.

## Method

Search keys are the 2976 facts used by `src/mod/net/troc_sniffer.cpp` and `docs/research/opcion-d-p8.md`: the 14-byte prologues, alloc size `0x8030`, buffer displacements `0x8028` / `0x800A` / `0x8014` / `0x8000`, limit `0x7FD7`, and the 39-byte NetworkThread pump.

The scanner parses the `.shared` exception directory (248,753 `RUNTIME_FUNCTION` entries, sorted, no inversions). It length-decodes each range. A constant counts only when those bytes are an immediate or a displacement of an instruction in a range that decodes exactly onto its end. Coverage of that pass:

| Section | Ranges | Exact | Short | Overshoot |
|---|---|---|---|---|
| `.text1` | 223,488 | 221,677 (99.19%) | 1,811 | 0 |
| `.tls$` | 25,265 | 24,850 (98.36%) | 415 | 0 |

Both candidates below are in the exact set, and a second walk of each range was exact. Direct `E8` callers were checked by decoding from the caller's unwind entry to the `E8`. Every caller listed for the two candidates landed on an instruction boundary.

The log's mnemonic column is a decoder annotation. The instruction bytes are the evidence. The stack arithmetic in this note was checked from those bytes.

A 14-byte prologue match is not an identification. Each 2976 head occurs more than 16 times in executable bytes; the scanner stored 16 and stopped. Those extra hits stay non-candidates.

## 2976 addresses on this file

| 2976 name | RVA | Result |
|---|---|---|
| `ClientNet::PostPacket` | `0x35C670` | Miss. Not an unwind entry. Bytes `41 B9 FF FF 02 00 48 8D 15 5B 15 DA 05 ...` |
| `ClientNet::OnRecvPacket` | `0x35CA00` | Miss. Not an unwind entry. Bytes `00 00 00 48 8D 0D E6 7B E5 06 E9 ...` |

## Candidates

### Client send — `exe+0x3A6310`

| Field | Value |
|---|---|
| Role | File candidate for `ClientNet::PostPacket` (client to server) |
| Section | `.text1` |
| File offset | `0x3A5710` |
| Unwind | `0x3A6310`–`0x3A652F`, exact |
| Size | `0x21F` |
| Entry | `48 89 5C 24 10 4C 89 4C 24 20 55 56 57 41 54 41` |
| 2976 post head | The first 14 bytes match |
| 2976 recv head | No |
| Direct `E8` callers | 413, all boundary-verified: 299 in `.text1`, 114 in `.tls$` |
| Confidence | Strong file candidate. Not adopted |

`imm32 0x8030` occurs once in the whole exact-decode set, at `exe+0x3A634D` inside this function. The three exact `disp32 0x8028` sites are this function (two) and the pump below (one).

Checked bytes:

```text
48 89 5C 24 10              mov [rsp+10h], rbx
4C 89 4C 24 20              mov [rsp+20h], r9
55 56 57                    push rbp / rsi / rdi
41 54 41 55 41 56 41 57    push r12 / r13 / r14 / r15
48 83 EC 20                 sub rsp, 20h
45 8B E0                    mov r12d, r8d
4C 8B F2                    mov r14, rdx
4C 8B E9                    mov r13, rcx
BA 10 00 00 00              mov edx, 10h
B9 30 80 00 00              mov ecx, 8030h
```

The byte test that follows picks one of `exe+0x4B01B48` or `exe+0x4B01A80`. Those two callees are not named. After seven pushes and `sub rsp, 20h`, the entry stack is `rsp+58h` below the instruction pointer:

| Instruction | Entry slot | 2976 name for that slot |
|---|---|---|
| `0F B7 9C 24 80 00 00 00` `movzx ebx, word [rsp+80h]` | `[rsp+28h]` | `u16` size |
| `0F B6 84 24 88 00 00 00` `movzx eax, byte [rsp+88h]` | `[rsp+30h]` | `u8` channel |
| `0F B7 84 24 90 00 00 00` `movzx eax, word [rsp+90h]` | `[rsp+38h]` | `u16` flags |
| `48 8B 54 24 78` `mov rdx, [rsp+78h]` | `[rsp+20h]`, the home of `r9` | packet pointer |

`r13` is the saved `rcx`. `r12d` is the saved `r8d`. Later stores from this function:

```text
48 8D 4F 03                 lea rcx, [rdi+3]
4C 8B C3                    mov r8, rbx          ; the u16 loaded above, before add 1Fh
48 8B 54 24 78              mov rdx, [rsp+78h]   ; saved r9
E8 .. .. .. ..              call exe+4BAAEEE
66 83 C3 1F                 add bx, 1Fh
66 89 9F 0A 80 00 00        mov [rdi+800Ah], bx
49 8B 8D E0 00 00 00        mov rcx, [r13+0E0h]
66 89 87 14 80 00 00        mov [rdi+8014h], ax  ; the entry [rsp+38h] word
48 89 B7 28 80 00 00        mov [rdi+8028h], rsi
```

`B8 DA 7F 00 00` / `66 3B C3` compares that entry `u16` with `0x7FDA`. The full instruction list is in the scan log under `POST-LIKE exe+3A6310`.

The `.tls$` callers are call sites inside unwind ranges. The candidate entry is in `.text1`. `.tls$` is not an observation point.

### Client receive — `exe+0x3A66A0`

| Field | Value |
|---|---|
| Role | File candidate for `ClientNet::OnRecvPacket` (server to client) |
| Section | `.text1` |
| File offset | `0x3A5AA0` |
| Unwind | `0x3A66A0`–`0x3A69EC`, exact |
| Size | `0x34C` |
| Entry | `48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 55` |
| 2976 recv head | The first 14 bytes match |
| 2976 post head | No |
| Direct `E8` callers | 4, all boundary-verified |
| Confidence | Strong file candidate. Not adopted |

`imm32 0x7FD7` occurs twice in the exact-decode set. This function is one. The other is `exe+0x29AC440`, described with the rejected neighbors.

Checked bytes at the entry, after `push rbp/r12/r13/r14/r15` and `lea rbp, [rsp-27h]`:

```text
4D 8B F9                    mov r15, r9
48 8B F2                    mov rsi, rdx
4C 8B E9                    mov r13, rcx
0F B7 7D 77                 movzx edi, word [rbp+77h]
8D 47 FD                    lea eax, [rdi-3]
B9 D7 7F 00 00              mov ecx, 7FD7h
66 3B C1                    cmp ax, cx
0F 87 E9 02 00 00           ja +2E9h
```

`rbp = entry_rsp - 4Fh`, so `[rbp+77h]` is the entry `[rsp+28h]`. The compare is `(u16 at entry [rsp+28h]) - 3` against `0x7FD7`.

The same prologue keeps `rdx` and then does:

```text
48 8B 9A 98 00 00 00        mov rbx, [rdx+98h]
FF 50 10                    call qword [rax+10h]
FF 50 30                    call qword [rax+30h]   ; r9 reloaded from the saved packet
41 BC 01 00 00 00           mov r12d, 1
45 8B C4                    mov r8d, r12d
41 0F B7 17                 movzx edx, word [r15]  ; first u16 of the r9 buffer
E8 .. .. .. ..              call exe+161EDB0
```

Direct callers:

| Call site | Unwind range | Where |
|---|---|---|
| `exe+0x3A57C3` | `0x3A5470`–`0x3A5B15` | The pump below |
| `exe+0x3A5D7D` | `0x3A5B20`–`0x3A5DE2` | `.text1` neighbor the pump also calls |
| `exe+0x3A65F1` | `0x3A6559`–`0x3A668C` | `.text1`. This range has no direct `E8` callers of its own |
| `exe+0x87FEFDF` | `0x87FEFB1`–`0x87FF006` | `.tls$`. Not an observation point |

### Pump neighbor — `exe+0x3A5470`

The 2976 39-byte pump sequence occurs once, at `exe+0x3A5744`, inside this exact unwind range (`0x3A5470`–`0x3A5B15`, `.text1`). Decoding from the range start reaches that address, and decoding from that address reaches the range end. This function is how the receive candidate was found. It is not itself an observation point, and it is not in `.tls$`.

At `exe+0x3A57C3` it calls the receive candidate:

```text
41 0F B7 86 0A 80 00 00     movzx eax, word [r14+800Ah]
66 83 E8 1F                 sub ax, 1Fh
4D 8D 4E 03                 lea r9, [r14+3]
66 89 44 24 20              mov [rsp+20h], ax     ; fifth argument; callee sees [rsp+28h]
49 8B 57 10                 mov rdx, [r15+10h]
49 8B CF                    mov rcx, r15
E8 D8 0E 00 00              call exe+3A66A0
```

`r8` is not written in this sequence. `r15` is the object whose `+0xE0` the pump loads (`49 8B 87 E0 00 00 00`). The same range also calls the send candidate at `exe+0x3A5A22`.

The send function writes `size+1Fh` to `[buffer+800Ah]` and copies the `r9` buffer to `[buffer+3]`. The pump reads `[buffer+800Ah]`, subtracts `1Fh`, and passes `[buffer+3]` as `r9`. That agreement is between these two 3576 functions.

## Rejected neighbors

| RVA | Why it is not the client receive entry |
|---|---|
| `exe+0x29AC440`–`0x29ACC6B` | The other exact `imm32 0x7FD7`. Size `0x82B`. One direct caller (`exe+0x29ABDE5`). Prologue does not match. The pump does not call it. `ServerNet::OnRecvPacket` stays the 2976 miss at `0x2773C20` and is not retargeted. |
| `exe+0x29AA300`–`0x29AA468` | Exact `imm32 0x7FDA`, 541 direct callers, size `0x168`. A shared helper shape, not the four-caller funnel. |
| `exe+0x29FB7A0`–`0x29FD01A` | Exact `imm32 0x7FDA`. Its only direct caller is in `.tls$`. |
| `exe+0xBDB130`–`0xBDC1B3` | Exact `imm32 0x7FDA` and one `.text1` caller. Not called from the pump. |

`disp32 0x800A` is common (551 exact hits) and was not used by itself. `disp32 0xE0` is common (17,717 exact hits). The pump hit is the 39-byte sequence, not the displacement alone.

The bounded name sample in the log is `TrocTr*` log text. It does not identify either function. No class name was required for the candidates above.

## Calling convention and layout

Verified on the file, from the bytes above:

| Item | What the file shows |
|---|---|
| Send `rcx` / `rdx` / `r8d` / `r9` | Saved as `r13`, `r14`, `r12d`, and the `[rsp+20h]` home slot |
| Send fifth/sixth/seventh arguments | Entry `[rsp+28h]` `u16`, `[rsp+30h]` `u8`, `[rsp+38h]` `u16` |
| Receive `rcx` / `rdx` / `r9` | Saved as `r13`, `rsi`, `r15` |
| Receive fifth argument | Entry `[rsp+28h]` `u16`, read through `[rbp+77h]` |
| Receive `r8` at the pump call | Not written by that call sequence |
| Alloc immediate | `ecx = 0x8030`, `edx = 0x10`, then one of two calls |
| Buffer fields written by the send function | Direct fields of the `0x8030` block at `+0x8000`, `+0x8008`, `+0x800A`, `+0x800C`, `+0x800E`, `+0x8010`, `+0x8014`, `+0x8018`, `+0x8020`, `+0x8028`. The copy destination is `[block+3]`. A `u16` is stored through `[block+8000h]`, and the entry `u8` is stored at `[[block+8000h]+2]` |
| Object field used as the outbound queue | `[saved rcx+0xE0]`, then one more dereference, then `call exe+161E620` |
| Pump to receive | `r9 = [r14+3]`, fifth argument = `[r14+800Ah] - 1Fh`, `rdx = [r15+10h]`, `rcx = r15` |

Still assumptions. A passive capture is what would check them. None of these is established by the file:

| Assumption | Why it is still open |
|---|---|
| `r8d == 4` means the local server session | The file saves `r8d`. It does not show the value `4`. |
| Entry `[rsp+28h]` is the Troc packet length in every call | The send function compares it with `0x7FDA` and copies that many bytes. Callers can still pass other values. |
| Entry `[rsp+30h]` / `[rsp+38h]` are channel and flags | The send function stores the `u8` through `[[rdi+8000h]+2]` and the `u16` at `[rdi+8014h]`. Their numeric meaning is from 2976. |
| First `u16` of the `r9` buffer is the opcode | The receive function loads that `u16` and calls `exe+161EDB0` with `r8d = 1`. The callee is unnamed. |
| Five-byte header `u16 opcode \| i8 slot \| u16 payload_len` | Not re-derived on 3576. |
| 2976 opcode numbers (2005, 2516, 2556, and the rest in `offsets.h`) | 2976 only. |
| `[session+98h]` is the crypto object and byte `+24h == 0` means crypto is off | The receive function calls through `[rdx+98h]` at `+10h` and `+30h`. The 2976 names for those calls are not re-proven. |
| `TLS[0x1FD]` selects the allocator | The send function tests a byte and picks one of two calls. The TLS index was not identified. |
| A `ClientNet` global such as 2976 `exe+0x6D69170` | Not searched as a global, and not found by this pass. The pump holds the object in `r15`. |
| The four receive callers are the 2976 virtual loop, fabricated 1002, socket mode, and fabricated 1004 | The pump caller matches the 2976 pump bytes. The other three roles are not re-labeled. |

## Passive capture plan

The experimental observer in `tools/pkt3576` follows this plan. Hook installation in that library is behind a test-supplied sink. This note's candidates are not adopted, and this work does not install a hook in the game.

Observe only. Record direction and raw values. Do not inject, rewrite, or call either function. Do not assign an opcode, channel, flags, or session-id meaning to a raw value.

### What the observer records

One record per entry, from the entry instant:

- Direction: send entry `exe+0x3A6310` or receive entry `exe+0x3A66A0`.
- `rcx`, `rdx`, `r8`, and `r9` as raw register values. The file does not show that `r8` is a session id, so the record does not name it as one.
- Send only: the raw `u16` at entry `[rsp+28h]`, the raw `u8` at entry `[rsp+30h]`, and the raw `u16` at entry `[rsp+38h]`. Those two extra slots are evidenced on the send candidate. They are stored without channel or flags names.
- Receive, among stack arguments: the raw `u16` at entry `[rsp+28h]` only. The receive candidate does not evidence `[rsp+30h]` or `[rsp+38h]`. The observer does not read those slots on receive.
- A prefix of the bytes at `r9`. When the `[rsp+28h]` read succeeds, the copied length is `min(that raw u16, 16)`. Sixteen is a copy bound, not a packet field. A claimed length of 0 copies nothing. A claimed length of 1 copies one byte and does not read a `u16` from `r9`. The prefix stays raw bytes. The first word at `r9` is not labeled an opcode.

An unreadable `[rsp+28h]`, an unreadable send-only stack slot, or an unreadable `r9` does not crash the calling thread. That attempt increments an unread count and does not publish a record. A full queue increments a drop count and returns without waiting.

No full-packet archive. No write to the game buffer. No call to `exe+0x3A6310` or `exe+0x3A66A0`.

### Initialization

1. Keep this outside `build_guard`. Require mapped base `0x140000000` and timestamp `0x6AED2E85`, both checked before any hook. The timestamp is read from the image. Refuse the supported 2976 timestamp `0x6AB28F00`. Do not mark 3576 as `Supported`.
2. Re-read the 16 entry bytes above at both addresses. Any mismatch or unreadable entry fails closed. Install neither observer, and do not allocate the ring.
3. Allocate the capture ring only after both byte checks pass. The ring is fixed-capacity, at least two slots, and safe for concurrent producers. One slot is refused: that size cannot tell a full ring from an empty one. The callback path does not allocate the ring.
4. Install the receive observer, then the send observer. If the second install fails, remove the first. When that removal succeeds, free the ring and allow the callback code to be released. When that removal fails, keep the ring, the callback code, and the observer that is still installed.
5. Do not start a writer from the callback. A consumer may pop on some other thread. The callback must not wait, allocate, perform file I/O, modify game state, or call either packet function.
6. Do not call `sniffer_install`. That function requires timestamp `0x6AB28F00`, hooks the 2976 RVAs, and the current `on_post_packet` / `on_recv_packet` paths call the game functions to inject. `troc_sniffer.h` has no removal function. The 2976 sniffer is not a 3576 observer.
7. Do not use `in_real_code`. On this file that helper only accepts a section named `.rsrc`, which is not executable. Do not patch `.tls$`. Do not copy the candidates into an offset header. Do not deploy `cdmp.asi`.

### While it is installed

- Copy through the injected reader. A process-side reader is `guarded_read` (SEH). A fault or a failed read counts as unread and leaves the calling thread alive.
- A full ring drops the record. The calling thread does not wait on the consumer.
- The callback returns without changing registers, stack arguments, or packet bytes.
- Several producers may call the send and receive callbacks together.

### Shutdown

1. Remove the receive observer, then the send observer. A callback may run while removal is in progress. The queue stays allocated through that call, and the callback does not wait.
2. When both removals succeed, close admission with one atomic word that holds the closed bit and the active count. Callbacks and the consumer either publish themselves in that word or return without touching the ring. Wait outside the callback until the count is zero, free the ring, and then allow the callback code to be released.
3. When either removal fails, do not free the ring and do not release the callback code. The observer object stays alive with its reader and callback state. Releasing the caller's ownership does not delete it. Keep each observer whose removal failed. A removal that succeeded stays removed. The shared ring stays because the surviving observer can still fire.

## What this note does not do

It does not adopt `exe+0x3A6310` or `exe+0x3A66A0`. It does not open the 3576 guard. It does not change the protocol. It does not install the experimental observer into the game. It does not choose a manager slot, a health rule, or a loading detector. Those remain where `docs/probe-3576/DIAG3576-LIVE.md` left them.
