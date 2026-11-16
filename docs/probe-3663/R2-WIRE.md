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
1013 (0x3F5). The unnamed old opcode 2679 remains unadopted until its actual
native message role is identified. The aggregate forbidden-request filter
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
constructs 1008. These are partial facts for C150: that aggregate remains
blocked until 1001/1003/1007 roles are checked and a target filter is routed.
The active 2976 filter has not been changed. Discovery records continue to
say unadopted; no target network path is enabled by this review.

Reproduce discovery with rtti_candidates.py, descriptor_candidates.py and
r2_request_candidates.py, review the initializer/handler exports, then run
r2_wire_review.py. The review script checks image hashes, source IDs and the
exact verified initializer operands before materializing the reviewed rows.
Raw discovery records retain `adopted=false`; adoption is only in the reviewed
catalog and eventual R2-MATCHES. No native wire or mod serialization is changed
by this evidence-only catalog, so protocol kVersion is unchanged.

Evidence: port-evidence/r2/request-*.json/txt, with original and canonical
text hashes in port-evidence/r2/text-hashes.json.
