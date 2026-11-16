# R2 native wire audit

No native target tunnel is armed. Native identifiers and payload layouts are
separate contracts. `r2-wire-reviewed.json` records 31 reviewed core rows;
`r2-reviewed.json` also records the native Nak ID. The numeric target values
and the reviewed payload member fields live in build_3663.h, still unused.

Both files independently identify each named RTTI class and its descriptor.
Paired native initializer disassembly verifies the instruction boundaries
that load its primary vtable and store it in the exact descriptor instance.
The target's vslot-2 handler is reviewed alongside that descriptor; the native
dispatcher reads descriptor opcode, kind and channel. This is stronger than
searching for an integer or treating a descriptor size as a payload schema.
Native registration CB4BE0 explicitly reads the named descriptor's u16 +C
opcode, passes its key and instance to insertion 165FF90, and stores the key
in registry entry+4. Lookup 165FAC0 compares that key and the descriptor's +8
kind mask; ServerOnRecvPacket then selects the descriptor's handler. These
target instructions corroborate the descriptor field's actual dispatch role.
No function/global RVA is adopted by this wire audit.

| Native message | 2976 | 3663 |
|---|---|---|
| ConnectCryptogramToServerReq | 2319 | 2401 |
| LoginUserToFieldServerReq | 2490 | 2554 |
| LoginCharacterToFieldServerReq | 3093 | 3081 |
| MainPlayerDataStartAck | 2756 | 2844 |
| MainUserDataStartAck | 2646 | 2420 |
| MoveFieldAck | 2174 | 2364 |
| ScopeSenderAck | 2042 | 2046 |
| ActorLoadDataAck (legacy ActorAppear alias) | 2005 | 2005 |
| CharacterLobbyDataAck (legacy DataAck alias) | 2869 | 2896 |
| LogoutCharacterToFieldServerAck (legacy opaque kOp2679) | 2679 | 3116 |
| SetFieldTimeReq | 2125 | 2150 |
| ReserveShutDownServerReq | 2790 | 2447 |
| SaveAutoReq | 3037 | 2234 |
| SaveToFileCurrentPlayerReq | 2910 | 3158 |
| SaveToFileCurrentPlayerAck | 3016 | 2813 |
| CreateCharacterReq | 2711 | 2533 |
| NewGameStartReq | 2178 | 2688 |
| LogoutSessionToServerReq | 2001 | 2001 |
| LogoutSessionToServerAck | 2002 | 2002 |
| LogoutUserToServerAck | 2003 | 2003 |

The native LogoutSession request failure path independently constructs Nak
1013 (0x3F5). The earlier unnamed opcode 2679 is now identified as the
LogoutCharacterToFieldServerAck mapping above. The aggregate forbidden-request filter
also remains blocked on every member, including shutdown 2038 and pointer
message 2004; the 2790 shutdown mapping alone does not complete that filter.

Separately reviewed payloads:

- SaveCurrentAck: old handler BE02B0 and target D42840 decode 4/4/1/1/4
  bytes and require complete consumption: key +0, slot +4, ok +8, flag +9,
  result +10, total 14 bytes. Its ID changes to 2813.
- Save request: old 2A21B80 and target 2D12AC0 consume a u32 slot at +0,
  u32 character count at +4 (strictly less than 31), UTF-16 at +8 with a
  checked terminator, then require exact exhaustion. Total 10+2n bytes,
  n≤30. Its ID changes to 3158.
- LoginCharacter slot-selection prefix: old 298BA10 and target 2C6AE10
  decode u8 flag, u64 character number, u32 slot, so the prefix offsets
  remain 0/1/9. This does not assert that the complete payload ends at byte13.
- Internal logout: request descriptor is exactly 3 bytes; ack descriptors
  are exactly 7. Native ack handlers consume the u32 at packet+3, confirming
  the separate three-byte internal header without an ordinary payload length.

Field-time decoding crosses Denuvo helpers and obfuscated scalar accesses;
its ID is established, but its complete 32-byte layout and target native
consumer still need proof. No unchanged structure is inferred from its ID.

Further core control review confirms the native shutdown timer changes
**2038→2042** (`ShutDownServerOnceTimer`), whereas pointer-bearing
`OnLogoutXxxToServerReq` remains 2004. Its target handler reads native process
pointers at packet+3 and +B; it must never cross the tunnel. Named connection
descriptors retain 1002/1004/1005/1006, and the target network pump explicitly
constructs 1008. An earlier investigation treated every integer in the
connection deny interval as an individual native-ID adoption obligation.
The target connect writer 3A9440 explicitly constructs opcode 1001 before
calling PostPacket 3A9960. Raw word searches for 1003/1007 instead found short
jumps and shifts; those false leads are rejected, not ID evidence. The
remaining 1003/1007 roles are unresolved; no native identity is claimed for them.

C150 now distinguishes the native IDs from unchanged conservative policy:
the filter rejects **every** value 1001..1008 regardless of handler existence.
The three actual protected native opcodes are proved as 2447/2042/2004;
field-time 2150 has its own row. control_filter.h accepts explicit per-build
native values and denies every request if that table is incomplete. Boundary
tests cover the whole closed interval, including its unclassified values,
and preserve legitimate logout 2001 with complete data. The production caller
still supplies only legacy values and remains unreachable on 3663; a target
route must supply the target catalog before any tunnel is enabled. C150's
native-value contract is proved; this is not a claim that Tier C is armed.
The active 2976 filter has not been changed. Discovery records continue to
say unadopted; no target network path is enabled by this review.

Reproduce discovery with rtti_candidates.py, descriptor_candidates.py and
r2_request_candidates.py, review the initializer/handler exports, then run
r2_wire_review.py. The review script checks image hashes, source IDs and the
exact verified initializer operands before materializing the reviewed rows.
Raw discovery records retain `adopted=false`; adoption is only in the reviewed
catalog and eventual R2-MATCHES. No native wire or mod serialization is changed
by this evidence-only catalog, so protocol kVersion is unchanged.

The paired client receive entries 35CA00→3A9CF0 now have independent native
code review: size-minus-three <=7FD7, optional crypto at session+98, TrocFind
kind mask 1, descriptor minimum/maximum lengths +18/+1A and flags +16, native
request construction, vslot+8 handler invocation, client queues +E8/+F0, and
owned-buffer cleanup. The target uses its own destructor chain and TLS+1FE;
this is not whole-body equality. A raw bound-immediate scan only found the
lead; actual instruction boundaries, call roles and both complete bodies
were checked. This adds an RVA candidate awaiting standard support. The
separately explored 3AAA60 is a constructor/setup routine, not this receiver.

Five descriptor member/value contracts are now independently confirmed:
kind+8, opcode+C, channel+14, flags+16 and named server-request kind 4.
Target receive compares +14 to the incoming channel and consumes flags+16;
native lookup masks kind+8; registration loads opcode+C and stores the same
descriptor pointer. Both files' named LoginUser/Character request descriptors
have kind 4. The separate framework-mode operand initially contains 11h in
both files (an exploratory assertion expecting 4 failed offline and was
corrected). Its target global candidate is 6F29B70, derived from the actual
receive operand, not from the descriptor-kind constant. Runtime mode values,
descriptor scan boundaries and scan skip remain separate contracts.

Evidence: port-evidence/r2/request-*.json/txt, with original and canonical
text hashes in port-evidence/r2/text-hashes.json.
