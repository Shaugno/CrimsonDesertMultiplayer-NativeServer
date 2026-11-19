# R10 Step 1 — the guest never gets its own 2941 (build 1.0.0.3732)

File-only analysis of the 1.0.0.3732 image (SHA-256 `DFCF5C50…`), compared
instruction by instruction with the 1.0.0.2976 reference (`57DA440D…`) that
the 2976 `observer_milestone` fix was built and live-tested against
([p8-2322-observador.md](../../../research/pendientes/p8-2322-observador.md)).
Tools: `opwriters.py`, `callers.py`, `refs.py`, `match.py`, `rtti.py`,
`fieldrefs.py`, `disasm.py` in [tools/port3732](../../../../tools/port3732/README.md).
No game was launched.

## Finding

In 3732, as in 2976, MoveFieldCompleteCore sends PlayAllPreLoadSequencerAck
(3732 opcode 2941, 2976 opcode 2322) **only when the traveller owns the field**.
A guest moving into the host's field takes the non-owner branch, receives
MoveFieldCompleteAck 2268 and MoveFieldCompletePostAck 2792 but no 2941, so its
loading state stays at `0x11` and it never sends LoadingCompleteReq 2789. This
matches all three R8 guest packet rings (2268 and 2792 delivered, no 2941, no
2789). R9's comparison used a single-player capture, where the client owns
the field and receives its 2941 unasked.

## The owner branch (3732 `0x2ED87D4..0x2ED8852`, 2976 `0x2BC8469..0x2BC84E2`)

```
3732                                              2976
0x2ED87D4 mov r8d,[r15+0x180]   ; field owner     0x2BC8469 mov r8d,[r13+0x180]
0x2ED87DB cmp eax,r8d           ; traveller key   0x2BC8470 cmp eax,r8d
0x2ED87DE jne 0x2ED8849         ; not owner: skip 0x2BC8473 jne 0x2BC84D9
0x2ED87EF call 0x2D7D6B0        ; owner's user    0x2BC8483 call 0x2A82590
          [user+0xD0]+0x68 -> +0xA8 -> +0x18      (same chain)
0x2ED8844 call 0x2B42100        ; owner only      0x2BC84D4 call 0x287EF60
0x2ED8849 mov rcx,[rdi+0x18]    ; paths rejoin    0x2BC84D9 mov rcx,[rsi+0x18]
0x2ED884D call 0x2AF1920                          0x2BC84DD call 0x2841D00
0x2ED8852 mov rax,[r15+0x2D8]   ; hook point      0x2BC84E2 mov rax,[r13+0x2D0]
```

The traveller key is `[[traveller+0xA0]+0x60]` in both builds. In 3732 the
field is in **r15** (2976: r13) and the traveller in **r14** (both).
`0x2B42100(rcx=owner object, rdx=field, r8=traveller, r9)` builds a ToTarget
finder for the traveller's session and calls the 2941 sender.

## Everything the 2976 observer milestone uses, in 3732

| Role | 2976 | 3732 | How confirmed |
| --- | --- | --- | --- |
| 2941/2322 sender (`mov word [r14],op`) | `0x2999D80` (writes at `0x2999E77`) | `0x2C7F430` (writes `0xB7D` at `0x2C7F527`) | only writer of the opcode; entry identical through the recipient-count check |
| Its only direct caller | `0x287F1D4` in `0x287EF60` | `0x2B42374` in `0x2B42100` | `callers.py`; same ToTarget recipe |
| Call from the Core, owner path | `0x2BC84D4` | `0x2ED8844` | `callers.py`; block above |
| Hook point (bytes) | `0x2BC84E2` `49 8B 85 D0 02 00 00` | `0x2ED8852` `49 8B 87 D8 02 00 00` | rejoin point; level entry `+0x2D8` = adopted kLevelEntry |
| Rejoin predecessor | `0x2BC84D9` | `0x2ED8849` | non-owner `jne` target |
| ToTarget constructor | `0x2A757A0` | `0x2D70BE0` | unique signature; code identical, loads the two vtables below |
| ToTarget vtable | `0x5B25438` | `0x5F6D9E0` | RTTI `.?AVToTargetSendTargetFinder@pa@@` |
| ISendTargetFinder base vtable | `0x5B253F8` | `0x5F6D9C0` | RTTI `.?AVISendTargetFinder@pa@@` |
| ToTarget slots 0/1/2 | `0x2A75740/0x2A757F0/0x2A75850` | `0x2D70B80/0x2D70C30/0x2D70C90` | read from the vtable |
| Base slot 0 | `0x2A756B0` | `0x2D70AE0` | read from the vtable |
| Scope sender lookup | `0x2961CA0` | `0x2C40870` | unique signature; sender calls it |
| Current scope flush | `0x2962220` | `0x2C40DF0` | unique signature |
| Send-context fields (TLS block 0 `+0x250` →) | `+0x298 +0x2A0 +0x2A8 +0x2AC +0x2B0 +0x2B8` | same | equal use counts in lookup, flush and sender |
| MoveFieldCompleteAck send | `0x2BC7433` (2144) | `0x2ED751D` (2268, serializer `0x2C7F830`) | only caller; in the handler before the Core call |
| MoveFieldCompletePostAck send | `0x2BC80F0` (3066) | `0x2ED83D0` (2792, serializer `0x2C7FCD0`) | caller inside the Core, before the hook point |
| Server-thread realm flag | TLS `+0x1EC` in `0x2773390` | TLS **`+0x1EB`** in `0x2A10920` | same function; all TLS byte flags shifted down by one (3 sites of `+0x1EC` in 2976 ↔ 3 of `+0x1EB` in 3663/3732) |

## Consequences for Step 2

- Port the 2976 host-side milestone with the 3732 addresses above; it calls
  the game's own constructor and sender, so no packet is built by hand.
- Use **r15** for the field and **r14** for the traveller in the hook
  context; check the realm flag at TLS **`+0x1EB`**.
- The guards stay as in 2976: live guest session, this trip's 2268 and 2792
  sent on this thread, verified guest primary key, host-owned current field,
  no native 2941 already queued, no open bundler, disable after any fault.
- Still to verify before live use: the server-user and travel-state offsets
  the 2976 decision reads (session `+0x1F8`, field mover, main character)
  against the adopted 3732 tables, and the 3732 trip events in the host tunnel.
