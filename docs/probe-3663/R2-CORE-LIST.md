# R2 core list — reviewed reachability cut

Scope baseline: 90c12f5. Reader refactoring adds an explicit native ID-width
contract; declaration locations follow the working branch. Reproduce with
`python tools/port3663/r2_core.py`. Stable R2 IDs refer to the generated JSON.
Each row is a game contract, including signatures and native wire layouts;
a compound native structure/signature is shown in full as one contract.
R1 done means the existing evidence is retained, not that its tier is armed.

Reachability: mod worker -> PlayerReader (A); session2::install -> server_install
and guest_install -> queues, send/receive, Troc descriptor scan (B); create_session
-> recipient finders, field_share, identity_shared_hooks and clock_gate (C);
logout/barrier/rejoin -> guest_lifecycle, save_hooks, guest_save_host/guest,
guest_save_tunnel, refresh_title_slots and report_guard (D). Shared contracts are
assigned at their first tier; consumers in higher tiers inherit that permission.

actor_lookup and camera are excluded: PlayerReader never calls them. RTTI parser
metadata offsets are MSVC x64 ABI invariants, not game offsets; RTTI names are
independent matching evidence. Save_hooks uses Windows file APIs, not game RVAs.
rejoin_logic and mod save/control formats are policy/protocol, verified by tests.

The call audit found a coupling the R1 scope obscured: share_field calls both
puppet_install_layer_guards and horse_diag_install(contain=true). Their shared
character safety contracts remain C rows, while mount diagnostics themselves
must be split off and stay unreachable. Deleting the safety guards is not a port.
All PHASE2-SCOPE optional modules stay disabled; optional probe-only identity
sites are excluded, but shared load/phases entries remain.

Policy values stay unchanged: timeouts, rate/log limits, maximum bounded scans,
queue/backlog caps, retry/generation counts, epoch budgets, reserved mod actor
IDs and slot204 choice, mod magic/tag contents, exit codes, packet batch/control
IDs, protocol versions and wire fields. Native buffer locations used for mod
tags remain game rows because their unused-space claim must be proved. Native
container capacities derived from stride are game contracts, not policy limits.
MaxStatEntries=64, loading radius=5, MaxEntries=64, MaxActors=0x4000 and C4
SamUserListMax=0x4000 are policy bounds. Stamina stays included because today's
reader reads it; no unimplemented exclusion is used to make A appear complete.

Counts: A 37, B 75, C 150, D 126; total 388; R1 adopted 17.

| Row | Tier | Game contract | Symbol | 2976 value | Source | Use | Evidence status |
|---|---|---|---|---|---|---|---|
| A001 | A | function/global RVA | kChunkOrigin | `0x6D56500` | src/mod/game/offsets.h:16 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A002 | A | function/global RVA | kStaticPlayerPos | `0x6D77858` | src/mod/game/offsets.h:17 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A003 | A | vtable RVA | kVtClientActorManager | `0x55B62C8` | src/mod/game/offsets.h:20 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A004 | A | vtable RVA | kVtControlledActor | `0x5585C40` | src/mod/game/offsets.h:21 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A005 | A | vtable RVA | kVtCharacterControl | `0x55B5460` | src/mod/game/offsets.h:22 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A006 | A | vtable RVA | kVtStatusComponent | `0x558D868` | src/mod/game/offsets.h:23 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A007 | A | vtable RVA | kVtPhysicsObject | `0x5D00F88` | src/mod/game/offsets.h:24 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A008 | A | native member/value contract | kWorldSystem_ActorManager | `0x30` | src/mod/game/offsets.h:27 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A009 | A | native member/value contract | kActorManager_ControlledActor | `0x50` | src/mod/game/offsets.h:29 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A010 | A | native member/value contract | kActor_ComponentTable | `0x68` | src/mod/game/offsets.h:31 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A011 | A | native member/value contract | kComponents_Status | `0x20` | src/mod/game/offsets.h:33 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A012 | A | native member/value contract | kComponents_CharacterControl | `0x40` | src/mod/game/offsets.h:34 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A013 | A | native member/value contract | kControl_Inner | `0x140` | src/mod/game/offsets.h:36 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A014 | A | native member/value contract | kControlInner_Physics | `0x2C0` | src/mod/game/offsets.h:37 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A015 | A | native member/value contract | kPhysics_AxisX | `0x60` | src/mod/game/offsets.h:39 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A016 | A | native member/value contract | kPhysics_AxisY | `0x70` | src/mod/game/offsets.h:40 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A017 | A | native member/value contract | kPhysics_AxisZ | `0x80` | src/mod/game/offsets.h:41 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A018 | A | native member/value contract | kPhysics_Position | `0x90` | src/mod/game/offsets.h:42 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A019 | A | native member/value contract | kStatus_StatContainer | `0x18` | src/mod/game/offsets.h:47 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A020 | A | native member/size contract | kStatContainer_Entries | `0x58` | src/mod/game/offsets.h:48 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A021 | A | native member/size contract | kStatContainer_Count | `0x60` | src/mod/game/offsets.h:49 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A022 | A | native member/size contract | kStatEntry_Size | `0x90` | src/mod/game/offsets.h:50 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A023 | A | native member/value contract | kStatEntry_Id | `0x00` | src/mod/game/offsets.h:51 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A024 | A | native member/value contract | kStatEntry_Current | `0x08` | src/mod/game/offsets.h:52 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A025 | A | native member/value contract | kStatEntry_Max | `0x18` | src/mod/game/offsets.h:53 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A026 | A | native identifier | kStatId_Health | `0` | src/mod/game/offsets.h:54 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A027 | A | native identifier | kStatId_Stamina | `22` | src/mod/game/offsets.h:55 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A028 | A | native loading sentinel | kLoadingParkHeight | `1000.0f` | src/mod/game/offsets.h:236 | PlayerReader::read loading sentinel | needs target evidence |
| B001 | B | function/global RVA | kServerNetSlot | `0x6D69680` | src/mod/session2/offsets_s2.h:12 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B002 | B | function/global RVA | kClientNetSlot | `0x6D69170` | src/mod/session2/offsets_s2.h:13 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B003 | B | native member/value contract | kServerNet_Session | `0x18` | src/mod/session2/offsets_s2.h:17 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B004 | B | native member/value contract | kServerNet_InQueue | `0x20` | src/mod/session2/offsets_s2.h:18 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B005 | B | native member/value contract | kServerNet_FreeC2S | `0x78` | src/mod/session2/offsets_s2.h:19 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B006 | B | native member/value contract | kServerNet_FreeS2C | `0x80` | src/mod/session2/offsets_s2.h:20 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B007 | B | native member/value contract | kClientNet_InQueue | `0x18` | src/mod/session2/offsets_s2.h:22 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B008 | B | vtable RVA | kNwVirtualAsyncSessionVtable | `0x57260C0` | src/mod/session2/offsets_s2.h:28 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B009 | B | vtable RVA | kNwAsyncSessionVtable | `0x5726140` | src/mod/session2/offsets_s2.h:29 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B010 | B | vtable RVA | kSessionVtableEntries | `15` | src/mod/session2/offsets_s2.h:30 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B011 | B | native member/value contract | kSessionVfSend | `9` | src/mod/session2/offsets_s2.h:31 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B012 | B | native member/size contract | kSessionSize | `0xD0` | src/mod/session2/offsets_s2.h:32 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B013 | B | native member/value contract | kSession_Name | `0x18` | src/mod/session2/offsets_s2.h:33 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B014 | B | native member/value contract | kSession_IdPtr | `0x88` | src/mod/session2/offsets_s2.h:34 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B015 | B | native member/value contract | kSession_User | `0x90` | src/mod/session2/offsets_s2.h:35 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B016 | B | native member/value contract | kSession_Crypto | `0x98` | src/mod/session2/offsets_s2.h:36 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B017 | B | native member/value contract | kSession_B0 | `0xB0` | src/mod/session2/offsets_s2.h:37 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B018 | B | native member/value contract | kSession_B8 | `0xB8` | src/mod/session2/offsets_s2.h:38 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B019 | B | native member/value contract | kSession_C0 | `0xC0` | src/mod/session2/offsets_s2.h:39 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B020 | B | native member/value contract | kSession_OutQueue | `0xC8` | src/mod/session2/offsets_s2.h:40 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B021 | B | function/global RVA | kServerOnRecvPacket | `0x2773C20` | src/mod/session2/offsets_s2.h:51 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B022 | B | native signature | kServerOnRecvPacketHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54}` | src/mod/session2/offsets_s2.h:52 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B023 | B | function/global RVA | kSessionSend | `0x1437740` | src/mod/session2/offsets_s2.h:54 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B024 | B | native signature | kSessionSendHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30}` | src/mod/session2/offsets_s2.h:55 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B025 | B | function/global RVA | kNwSessionCtor | `0x1436EE0` | src/mod/session2/offsets_s2.h:57 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B026 | B | native signature | kNwSessionCtorHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x6C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56}` | src/mod/session2/offsets_s2.h:58 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B027 | B | function/global RVA | kQueuePush | `0x35DF50` | src/mod/session2/offsets_s2.h:60 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B028 | B | native signature | kQueuePushHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}` | src/mod/session2/offsets_s2.h:61 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B029 | B | function/global RVA | kAllocGlobal | `0x47EFA78` | src/mod/session2/offsets_s2.h:71 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B030 | B | native signature | kAllocGlobalHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA}` | src/mod/session2/offsets_s2.h:72 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B031 | B | function/global RVA | kAllocThread | `0x47EF9B0` | src/mod/session2/offsets_s2.h:74 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B032 | B | native signature | kAllocThreadBytes | `{ 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0x7E, 0x92, 0x49, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF}` | src/mod/session2/offsets_s2.h:76 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B033 | B | function/global RVA | kTlsIndexSlot | `0x6C88C50` | src/mod/session2/offsets_s2.h:83 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B034 | B | native member/value contract | kTls_UseGlobalHeap | `0x1FD` | src/mod/session2/offsets_s2.h:84 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B035 | B | native member/value contract | kServerNet_Threads | `0xF8` | src/mod/session2/offsets_s2.h:89 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B036 | B | native member/size contract | kServerNet_ThreadCount | `0x100` | src/mod/session2/offsets_s2.h:90 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B037 | B | native member/value contract | kServerThreadEntry | `0x10` | src/mod/session2/offsets_s2.h:91 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B038 | B | native member/size contract | kArray_Count | `0x08` | src/mod/session2/offsets_s2.h:107 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B039 | B | native member/size contract | kArray_Capacity | `0x0C` | src/mod/session2/offsets_s2.h:108 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B040 | B | native member/size contract | kPbSize | `0x8030` | src/mod/session2/offsets_s2.h:111 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B041 | B | native member/value contract | kPb_FramePtr | `0x8000` | src/mod/session2/offsets_s2.h:112 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B042 | B | native member/value contract | kPb_Len1 | `0x8008` | src/mod/session2/offsets_s2.h:113 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B043 | B | native member/value contract | kPb_Len2 | `0x800A` | src/mod/session2/offsets_s2.h:114 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B044 | B | native member/value contract | kPb_Cap | `0x800C` | src/mod/session2/offsets_s2.h:115 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B045 | B | native member/value contract | kPb_IsRequest | `0x800E` | src/mod/session2/offsets_s2.h:116 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B046 | B | native member/value contract | kPb_Tag | `0x8010` | src/mod/session2/offsets_s2.h:117 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B047 | B | native member/value contract | kPb_Magic | `0x8018` | src/mod/session2/offsets_s2.h:118 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B048 | B | native member/value contract | kPb_Base | `0x8028` | src/mod/session2/offsets_s2.h:119 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B049 | B | native member/size contract | kFrameOverhead | `0x1F` | src/mod/session2/offsets_s2.h:120 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B050 | B | native member/size contract | kFrameHeader | `3` | src/mod/session2/offsets_s2.h:121 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B051 | B | native member/value contract | kRq_Frame | `0x30` | src/mod/session2/offsets_s2.h:123 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B052 | B | function/global RVA | kTrocInstancesBegin | `0x6A51AE0` | src/mod/session2/offsets_s2.h:127 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B053 | B | function/global RVA | kTrocInstancesEnd | `0x6A82C80` | src/mod/session2/offsets_s2.h:128 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B054 | B | native member/value contract | kTroc_Kind | `0x08` | src/mod/session2/offsets_s2.h:129 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B055 | B | native member/value contract | kTroc_Opcode | `0x0C` | src/mod/session2/offsets_s2.h:130 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B056 | B | native member/value contract | kTroc_Channel | `0x14` | src/mod/session2/offsets_s2.h:131 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B057 | B | native member/value contract | kTroc_Flags | `0x16` | src/mod/session2/offsets_s2.h:132 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B058 | B | native member/value contract | kTrocKindServer | `4` | src/mod/session2/offsets_s2.h:133 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B059 | B | native member/value contract | kClientNet_Session | `0x10` | src/mod/session2/offsets_s2.h:136 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B060 | B | native member/value contract | kClientNet_FreeC2S | `0x28` | src/mod/session2/offsets_s2.h:137 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B061 | B | native member/value contract | kClientNet_Slot4 | `0x58` | src/mod/session2/offsets_s2.h:138 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B062 | B | native member/value contract | kSyncSession_Epoch | `0xB0` | src/mod/session2/offsets_s2.h:139 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B063 | B | function/global RVA | kSyncSessionSend | `0x14380B0` | src/mod/session2/offsets_s2.h:140 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B064 | B | native signature | kSyncSessionSendHead | `{0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA}` | src/mod/session2/offsets_s2.h:141 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B065 | B | function/global RVA | kNetLoopPump | `0x35BAD4` | src/mod/session2/offsets_s2.h:143 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B066 | B | native signature | kNetLoopPumpHead | `{0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7}` | src/mod/session2/offsets_s2.h:144 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B067 | B | native member/value contract | kNetLoopPumpPatch | `7` | src/mod/session2/offsets_s2.h:153 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B068 | B | native signature | kNetLoopPumpAob | `{ 0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74}` | src/mod/session2/offsets_s2.h:154 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B069 | B | function/global RVA | kClientOnRecvPacket | `0x35CA00` | src/mod/session2/offsets_s2.h:157 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B070 | B | native signature | kClientOnRecvPacketHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24}` | src/mod/session2/offsets_s2.h:158 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| C001 | C | function/global RVA | kSamSlot | `0x6DA67D8` | src/mod/session2/offsets_s2.h:14 | server users and native recipient selection | needs target evidence |
| C002 | C | native member/value contract | kSam_FreeKeys | `0xD0` | src/mod/session2/offsets_s2.h:24 | server users and native recipient selection | needs target evidence |
| C003 | C | native member/size contract | kSam_ActorCount | `0x9C` | src/mod/session2/offsets_s2.h:25 | server users and native recipient selection | needs target evidence |
| C004 | C | vtable RVA | kServerUserActorVtable | `0x5B257A8` | src/mod/session2/offsets_s2.h:43 | server users and native recipient selection | needs target evidence |
| C005 | C | native member/value contract | kUser_Key | `0x60` | src/mod/session2/offsets_s2.h:44 | server users and native recipient selection | needs target evidence |
| C006 | C | native member/value contract | kUser_Session | `0x1F8` | src/mod/session2/offsets_s2.h:45 | server users and native recipient selection | needs target evidence |
| C007 | C | native member/value contract | kServerActor_Controller | `0x90` | src/mod/session2/offsets_s2.h:47 | server users and native recipient selection | needs target evidence |
| C008 | C | native member/value contract | kServerActor_Key | `0x60` | src/mod/session2/offsets_s2.h:48 | server users and native recipient selection | needs target evidence |
| C009 | C | vtable RVA | kToNearExceptOneControlVtable | `0x5B25358` | src/mod/session2/offsets_s2.h:95 | server users and native recipient selection | needs target evidence |
| C010 | C | vtable RVA | kToNearExceptOneSelfVtable | `0x5B25378` | src/mod/session2/offsets_s2.h:96 | server users and native recipient selection | needs target evidence |
| C011 | C | vtable RVA | kToNearVtable | `0x5B25418` | src/mod/session2/offsets_s2.h:97 | server users and native recipient selection | needs target evidence |
| C012 | C | function/global RVA | kExceptOneVf1 | `0x2A759B0` | src/mod/session2/offsets_s2.h:98 | server users and native recipient selection | needs target evidence |
| C013 | C | function/global RVA | kExceptOneSelfVf1 | `0x2A75880` | src/mod/session2/offsets_s2.h:99 | server users and native recipient selection | needs target evidence |
| C014 | C | function/global RVA | kToNearVf1 | `0x2A75860` | src/mod/session2/offsets_s2.h:100 | server users and native recipient selection | needs target evidence |
| C015 | C | native signature | kExceptOneVf1Head | `{0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00}` | src/mod/session2/offsets_s2.h:101 | server users and native recipient selection | needs target evidence |
| C016 | C | native signature | kToNearVf1Head | `{0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9}` | src/mod/session2/offsets_s2.h:103 | server users and native recipient selection | needs target evidence |
| C017 | C | native member/value contract | kFinder_Actor | `0x10` | src/mod/session2/offsets_s2.h:105 | server users and native recipient selection | needs target evidence |
| D001 | D | native member/value contract | kWorldSystem_SaveManager | `0x88` | src/mod/session2/offsets_s2.h:166 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D002 | D | function/global RVA | kServerActorLookupSlot | `0x6D69698` | src/mod/session2/offsets_s2.h:167 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D003 | D | native member/value contract | kServerActorLookup_SaveManager | `0x48` | src/mod/session2/offsets_s2.h:168 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D004 | D | native member/value contract | kSaveMgr_Lock | `0x00` | src/mod/session2/offsets_s2.h:170 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D005 | D | native member/value contract | kSaveMgr_LockObj | `0x08` | src/mod/session2/offsets_s2.h:171 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D006 | D | native member/value contract | kSaveMgr_Slots | `0x40` | src/mod/session2/offsets_s2.h:172 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D007 | D | native member/size contract | kSaveMgr_Count | `0x48` | src/mod/session2/offsets_s2.h:173 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D008 | D | native member/size contract | kSaveMgr_Capacity | `0x4C` | src/mod/session2/offsets_s2.h:174 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D009 | D | native member/value contract | kSaveMgr_Storage | `0x50` | src/mod/session2/offsets_s2.h:175 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D010 | D | native member/value contract | kSaveMgr_Busy | `0x60` | src/mod/session2/offsets_s2.h:176 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D011 | D | native member/size contract | kSaveMgr_EntrySize | `0x68` | src/mod/session2/offsets_s2.h:177 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D012 | D | native member/size contract | kSaveStorage_FolderCount | `0x40` | src/mod/session2/offsets_s2.h:178 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D013 | D | vtable RVA | kWindowsLock3Vtable | `0x557C120` | src/mod/session2/offsets_s2.h:179 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D014 | D | function/global RVA | kWindowsLockVf1 | `0x1371340` | src/mod/session2/offsets_s2.h:180 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D015 | D | function/global RVA | kWindowsLockVf2 | `0xCE4EF0` | src/mod/session2/offsets_s2.h:181 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D016 | D | native signature | kWindowsLockVf1Head | `{0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18}` | src/mod/session2/offsets_s2.h:182 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D017 | D | native signature | kWindowsLockVf2Head | `{0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20}` | src/mod/session2/offsets_s2.h:183 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D018 | D | function/global RVA | kSaveMgrRefreshSlots | `0x1870C20` | src/mod/session2/offsets_s2.h:185 | guest refresh_title_slots and protected save-manager table | R1 done |
| D019 | D | native signature | kSaveMgrRefreshSlotsHead | `{0xE9, 0xAB, 0x86, 0x36, 0x0B}` | src/mod/session2/offsets_s2.h:186 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D020 | D | function/global RVA | kSlotRefreshReqSequence | `0x2A22475` | src/mod/session2/offsets_s2.h:190 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D021 | D | native signature | kSlotRefreshReqSequenceBytes | `{ 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10}` | src/mod/session2/offsets_s2.h:191 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D022 | D | function/global RVA | kTitlePlayReadsSaveMgr | `0x65DC3E` | src/mod/session2/offsets_s2.h:211 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D023 | D | native signature | kTitlePlayReadsSaveMgrBytes | `{0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00}` | src/mod/session2/offsets_s2.h:212 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| C018 | C | native member/value contract | kRefs | `0x10` | src/mod/session2/field_share.h:41 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C019 | C | native member/value contract | kLock1 | `0x18` | src/mod/session2/field_share.h:42 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C020 | C | native member/value contract | kLock2Ptr | `0x08` | src/mod/session2/field_share.h:43 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C021 | C | native identifier | kOpen | `0x4A` | src/mod/session2/field_share.h:44 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C022 | C | native member/value contract | kOwner | `0x180` | src/mod/session2/field_share.h:45 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C023 | C | native member/value contract | kUsers | `0x2A0` | src/mod/session2/field_share.h:46 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C024 | C | native member/size contract | kUserCount | `0x2A8` | src/mod/session2/field_share.h:47 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C025 | C | native member/value contract | kUserActors | `0x2B0` | src/mod/session2/field_share.h:48 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C026 | C | native member/value contract | kKey | `0x2C8` | src/mod/session2/field_share.h:49 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C027 | C | native member/value contract | kLevelEntry | `0x2D0` | src/mod/session2/field_share.h:50 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C028 | C | native member/value contract | kTsFieldMover | `0x4F0` | src/mod/session2/field_share.h:52 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C029 | C | native member/value contract | kFmFieldKey | `0x08` | src/mod/session2/field_share.h:53 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C030 | C | native member/value contract | kFmField | `0x10` | src/mod/session2/field_share.h:54 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C031 | C | native member/value contract | kFmBusy | `0x5A` | src/mod/session2/field_share.h:55 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C032 | C | native member/value contract | kOutField | `0x08` | src/mod/session2/field_share.h:58 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C033 | C | native member/value contract | kOutValid | `0x10` | src/mod/session2/field_share.h:59 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C034 | C | native member/value contract | kOutHolder | `0x18` | src/mod/session2/field_share.h:60 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C035 | C | native member/value contract | kOutNew | `0x20` | src/mod/session2/field_share.h:61 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C036 | C | native member/value contract | kOutInfoKey | `0x24` | src/mod/session2/field_share.h:62 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C037 | C | native member/size contract | kMgrBucketCount | `0x48` | src/mod/session2/field_share.h:66 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C038 | C | native member/size contract | kMgrEntries | `0x4C` | src/mod/session2/field_share.h:67 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C039 | C | native member/value contract | kMgrBuckets | `0x58` | src/mod/session2/field_share.h:68 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C040 | C | native member/value contract | kMgrHolders | `0x60` | src/mod/session2/field_share.h:69 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C041 | C | function/global RVA | kAcquireField | `0x2B07790` | src/mod/session2/field_share.cpp:20 | field_share_install and acquire/enter detours | needs target evidence |
| C042 | C | native signature | kAcquireFieldHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70}` | src/mod/session2/field_share.cpp:21 | field_share_install and acquire/enter detours | needs target evidence |
| C043 | C | function/global RVA | kEnterField | `0x2BC04F0` | src/mod/session2/field_share.cpp:24 | field_share_install and acquire/enter detours | needs target evidence |
| C044 | C | native signature | kEnterFieldHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC}` | src/mod/session2/field_share.cpp:25 | field_share_install and acquire/enter detours | needs target evidence |
| C045 | C | function/global RVA | kTryAddRef | `0x14343A0` | src/mod/session2/field_share.cpp:28 | field_share_install and acquire/enter detours | needs target evidence |
| C046 | C | native signature | kTryAddRefHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9}` | src/mod/session2/field_share.cpp:29 | field_share_install and acquire/enter detours | needs target evidence |
| C047 | C | function/global RVA | kVecPush | `0x40B5B0` | src/mod/session2/field_share.cpp:31 | field_share_install and acquire/enter detours | needs target evidence |
| C048 | C | native signature | kVecPushHead | `{0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, 0x4C, 0x8B, 0xF2, 0x8B, 0x49, 0x08, 0x8B, 0x43, 0x0C}` | src/mod/session2/field_share.cpp:32 | field_share_install and acquire/enter detours | needs target evidence |
| C049 | C | native member/value contract | kUserMainCharacter | `0xD0` | src/mod/session2/field_share.cpp:35 | field_share_install and acquire/enter detours | needs target evidence |
| C050 | C | native member/value contract | kUserKey | `0x60` | src/mod/session2/field_share.cpp:36 | field_share_install and acquire/enter detours | needs target evidence |
| C051 | C | native member/value contract | kActorUser | `0xA0` | src/mod/session2/field_share.cpp:37 | field_share_install and acquire/enter detours | needs target evidence |
| C052 | C | native member/value contract | kNative | `0x7FFFFFFFFFFFFFFF` | src/mod/session2/identity_fix.h:45 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C053 | C | native member/value contract | kLobbyOwner | `0x08` | src/mod/session2/identity_fix.h:48 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C054 | C | native member/value contract | kLobbyList | `0x18` | src/mod/session2/identity_fix.h:49 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C055 | C | native member/size contract | kListCount | `0x08` | src/mod/session2/identity_fix.h:50 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C056 | C | native member/size contract | kEntryStride | `0xD0` | src/mod/session2/identity_fix.h:51 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C057 | C | native member/value contract | kEntryNumber | `0x00` | src/mod/session2/identity_fix.h:52 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C058 | C | native member/value contract | kUserComponents | `0x68` | src/mod/session2/identity_fix.h:53 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C059 | C | native member/value contract | kTableLobby | `0x10` | src/mod/session2/identity_fix.h:54 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C060 | C | native member/value contract | kTableStatus | `0x20` | src/mod/session2/identity_fix.h:55 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C061 | C | native member/value contract | kStatusActor | `0x08` | src/mod/session2/identity_fix.h:56 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C062 | C | native member/value contract | kStatusNumber | `0x70` | src/mod/session2/identity_fix.h:57 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C063 | C | native member/value contract | kActorKey | `0x60` | src/mod/session2/identity_fix.h:58 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C064 | C | native member/value contract | kActorUser | `0xA0` | src/mod/session2/identity_fix.h:59 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C065 | C | native member/value contract | kLockObject | `0x08` | src/mod/session2/identity_fix.h:60 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C066 | C | native member/value contract | kCharacterClass | `0xA` | src/mod/session2/identity_fix.h:63 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C067 | C | function/global RVA | kLobbyLoad | `0x2B7F4D0` | src/mod/session2/identity_fix.cpp:215 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C068 | C | native signature | kLobbyLoadHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xF2, 0x4C, 0x8B, 0xF9, 0x49, 0x8B, 0x39}` | src/mod/session2/identity_fix.cpp:216 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C069 | C | vtable RVA | kLobbyVtable | `0x5B2BE58` | src/mod/session2/identity_fix.cpp:219 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C070 | C | function/global RVA | kLobbyConstant | `0x2B7F66C` | src/mod/session2/identity_fix.cpp:220 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C071 | C | native signature | kLobbyConstantBytes | `{0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x48, 0x89, 0x44, 0x24, 0x70}` | src/mod/session2/identity_fix.cpp:221 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C072 | C | function/global RVA | kLobbyAddCall | `0x2B7F6DB` | src/mod/session2/identity_fix.cpp:223 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C073 | C | native signature | kLobbyAddCallBytes | `{0xE8, 0x90, 0x47, 0x5C, 0xFF}` | src/mod/session2/identity_fix.cpp:224 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C074 | C | function/global RVA | kClearLobbySite | `0x2C3E0FF` | src/mod/session2/identity_fix.cpp:226 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C075 | C | native signature | kClearLobbySiteBytes | `{0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}` | src/mod/session2/identity_fix.cpp:227 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C076 | C | function/global RVA | kClearListSite | `0x2C3E126` | src/mod/session2/identity_fix.cpp:228 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C077 | C | native signature | kClearListSiteBytes | `{0x48, 0x8B, 0x4B, 0x18}` | src/mod/session2/identity_fix.cpp:229 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C078 | C | vtable RVA | kStatusVtable | `0x5B203F0` | src/mod/session2/identity_fix.cpp:230 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C079 | C | function/global RVA | kStatusLoad | `0x293FE70` | src/mod/session2/identity_fix.cpp:231 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C080 | C | function/global RVA | kLoginMgrSite | `0x2777A18` | src/mod/session2/identity_fix.cpp:234 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C081 | C | native signature | kLoginMgrSiteBytes | `{0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}` | src/mod/session2/identity_fix.cpp:235 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C082 | C | function/global RVA | kLoginMgrSlot | `0x6D696A8` | src/mod/session2/identity_fix.cpp:236 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C083 | C | function/global RVA | kFindByNoScan | `0x2A84066` | src/mod/session2/identity_fix.cpp:237 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C084 | C | native signature | kFindByNoScanBytes | `{0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x14, 0xC1}` | src/mod/session2/identity_fix.cpp:238 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C085 | C | function/global RVA | kFindByNoRead | `0x2A84090` | src/mod/session2/identity_fix.cpp:240 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C086 | C | native signature | kFindByNoReadBytes | `{0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20}` | src/mod/session2/identity_fix.cpp:241 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C087 | C | native member/value contract | kLoginMgrActors | `0xB8` | src/mod/session2/identity_fix.cpp:242 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C088 | C | native member/size contract | kLoginMgrCount | `0xC0` | src/mod/session2/identity_fix.cpp:243 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C089 | C | native identifier | kOpLoginUser | `2490` | src/mod/session2/identity_fix.cpp:244 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C090 | C | native identifier | kOpLoginCharacter | `3093` | src/mod/session2/identity_fix.cpp:245 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C091 | C | native member/value contract | kActorComponents | `0x68` | src/mod/session2/identity_probe.h:48 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C092 | C | native member/value contract | kTableStatus | `0x20` | src/mod/session2/identity_probe.h:49 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C093 | C | native member/value contract | kStatusCharacterNo | `0x70` | src/mod/session2/identity_probe.h:50 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C094 | C | native member/value contract | kComponentOwner | `0x08` | src/mod/session2/identity_probe.h:51 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C095 | C | native member/value contract | kActorUser | `0xA0` | src/mod/session2/identity_probe.h:52 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C096 | C | native member/value contract | kKey | `0x60` | src/mod/session2/identity_probe.h:53 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C097 | C | function/global RVA | kLoad | `0x20688A0` | src/mod/session2/identity_probe.cpp:31 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C098 | C | native signature | kLoadHead | `{0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x68, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00, 0x49, 0x8B, 0xD9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xF1}` | src/mod/session2/identity_probe.cpp:32 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C099 | C | function/global RVA | kPhases | `0x2941020` | src/mod/session2/identity_probe.cpp:50 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C100 | C | native signature | kPhasesHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78}` | src/mod/session2/identity_probe.cpp:51 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C101 | C | vtable RVA | kStatusVtable | `0x5B203F0` | src/mod/session2/identity_probe.cpp:83 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C102 | C | function/global RVA | kStatusLoad | `0x293FE70` | src/mod/session2/identity_probe.cpp:84 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C103 | C | function/global RVA | kLoadStub | `0x35EA90` | src/mod/session2/identity_probe.cpp:85 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C104 | C | native member/value contract | kTsActor | `0x08` | src/mod/session2/clock_gate.h:41 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C105 | C | native member/value contract | kActorUser | `0xA0` | src/mod/session2/clock_gate.h:42 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C106 | C | native member/value contract | kUserKey | `0x60` | src/mod/session2/clock_gate.h:43 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C107 | C | native member/value contract | kTsFieldTime | `0x450` | src/mod/session2/clock_gate.h:44 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C108 | C | native member/size contract | kFieldTimeSize | `32` | src/mod/session2/clock_gate.h:45 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C109 | C | function/global RVA | kWorldTime | `0x6A6D238` | src/mod/session2/clock_gate.h:46 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C110 | C | function/global RVA | kTravel | `0x2BC0A30` | src/mod/session2/clock_gate.cpp:21 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C111 | C | native signature | kTravelHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}` | src/mod/session2/clock_gate.cpp:22 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C112 | C | function/global RVA | kTravelOwnerCmp | `0x2BC0A78` | src/mod/session2/clock_gate.cpp:25 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C113 | C | native signature | kTravelOwnerCmpBytes | `{0x39, 0x82, 0x80, 0x01, 0x00, 0x00}` | src/mod/session2/clock_gate.cpp:26 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C114 | C | function/global RVA | kTravelGetterCall | `0x2BC0CC7` | src/mod/session2/clock_gate.cpp:27 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C115 | C | native signature | kTravelGetterCallBytes | `{0xE8, 0xD4, 0x48, 0x52, 0xFF}` | src/mod/session2/clock_gate.cpp:28 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C116 | C | function/global RVA | kLoading | `0x219D7F0` | src/mod/session2/clock_gate.cpp:29 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C117 | C | native signature | kLoadingHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x60, 0x4D, 0x8B, 0xE9, 0x49, 0x8B, 0xF0, 0x4C, 0x8B, 0xF2, 0x48, 0x8B, 0xD9}` | src/mod/session2/clock_gate.cpp:30 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C118 | C | function/global RVA | kLoadingFlagCmp | `0x219D82B` | src/mod/session2/clock_gate.cpp:34 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C119 | C | native signature | kLoadingFlagCmpBytes | `{0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00}` | src/mod/session2/clock_gate.cpp:35 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C120 | C | function/global RVA | kGetter | `0x20E55A0` | src/mod/session2/clock_gate.cpp:36 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C121 | C | native signature | kGetterHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0xFA, 0x48, 0x8D, 0x54, 0x24, 0x70, 0x48, 0x8B, 0xD9, 0xFF, 0x50, 0x60}` | src/mod/session2/clock_gate.cpp:37 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C122 | C | native member/value contract | kSessionId | `0` | src/mod/session2/server.cpp:76 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C123 | C | native identifier | kOpLoginCharacterToFieldServerReq | `3093` | src/mod/session2/server.cpp:133 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C124 | C | native identifier | kOpLoginUserToFieldServerReq | `2490` | src/mod/session2/server.cpp:134 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C125 | C | native identifier | kOpMainPlayerDataStartAck | `2756` | src/mod/session2/server.cpp:138 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C126 | C | native identifier | kOpMainUserDataStartAck | `2646` | src/mod/session2/server.cpp:139 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C127 | C | native identifier | kOpMoveFieldAck | `2174` | src/mod/session2/server.cpp:140 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C128 | C | native identifier | kOpNak | `1013` | src/mod/session2/server.cpp:181 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| B071 | B | native identifier | kOpConnectCryptogramToServerReq | `2319` | src/mod/session2/guest.cpp:71 | guest handshake and epoch ownership | needs target evidence |
| B072 | B | native epoch sentinel | kNoEpoch | `0x7FFFFFFFFFFFFFFFull` | src/mod/session2/guest.cpp:72 | guest handshake and epoch ownership | needs target evidence |
| B073 | B | function/global RVA | kServerFrameworkMode | `0x6A65564` | src/mod/session2/guest.cpp:84 | guest handshake and epoch ownership | needs target evidence |
| C129 | C | native identifier | kOpScopeSenderAck | `2042` | src/mod/session2/guest.cpp:76 | guest native login and inbound packet classification | needs target evidence |
| C130 | C | native identifier | kOpActorAppear | `2005` | src/mod/session2/guest.cpp:77 | guest native login and inbound packet classification | needs target evidence |
| C131 | C | native identifier | kOpDataAck | `2869` | src/mod/session2/guest.cpp:78 | guest native login and inbound packet classification | needs target evidence |
| C132 | C | native identifier | kOp2679 | `2679` | src/mod/session2/guest.cpp:79 | guest native login and inbound packet classification | needs target evidence |
| C133 | C | native identifier | kOpLoginCharacterToFieldServerReq | `3093` | src/mod/session2/guest.cpp:80 | guest native login and inbound packet classification | needs target evidence |
| C134 | C | native identifier | kOpLoginUserToFieldServerReq | `2490` | src/mod/session2/guest.cpp:214 | guest native login and inbound packet classification | needs target evidence |
| D024 | D | native identifier | kOpReserveShutDownServerReq | `2790` | src/mod/session2/guest.cpp:81 | guest native shutdown filter | needs target evidence |
| D025 | D | native member/value contract | kSamMapBuckets | `0x98` | src/mod/session2/guest_lifecycle.h:50 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D026 | D | native member/size contract | kSamMapSize | `0x9C` | src/mod/session2/guest_lifecycle.h:51 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D027 | D | native member/value contract | kSamMapBucketBase | `0xA8` | src/mod/session2/guest_lifecycle.h:52 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D028 | D | native member/value contract | kSamMapHolders | `0xB0` | src/mod/session2/guest_lifecycle.h:53 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D029 | D | native member/value contract | kSamBucketBytes | `0x100` | src/mod/session2/guest_lifecycle.h:54 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D030 | D | native member/size contract | kSamBucketMaxEntries | `(kSamBucketBytes - 8) / 8` | src/mod/session2/guest_lifecycle.h:55 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D031 | D | function/global RVA | kSamKeyIndexMask | `0xFFFFF` | src/mod/session2/guest_lifecycle.h:56 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D032 | D | native member/value contract | kSamUserList | `0xB8` | src/mod/session2/guest_lifecycle.h:58 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D033 | D | native member/size contract | kSamUserCount | `0xC0` | src/mod/session2/guest_lifecycle.h:59 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D034 | D | native member/value contract | kActorKey | `0x60` | src/mod/session2/guest_lifecycle.h:64 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D035 | D | native member/value contract | kUserMainCharacter | `0xD0` | src/mod/session2/guest_lifecycle.h:68 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D036 | D | native member/value contract | kUserComponents | `0x68` | src/mod/session2/guest_lifecycle.h:69 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D037 | D | native member/value contract | kComponentsSlots | `0x18` | src/mod/session2/guest_lifecycle.h:70 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D038 | D | native member/value contract | kSlotsArray | `0x18` | src/mod/session2/guest_lifecycle.h:71 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D039 | D | native member/size contract | kSlotsCount | `0x20` | src/mod/session2/guest_lifecycle.h:72 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D040 | D | native member/size contract | kSlotStride | `0x10` | src/mod/session2/guest_lifecycle.h:73 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D041 | D | native member/value contract | kActorState | `0x5E` | src/mod/session2/guest_lifecycle.h:196 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D042 | D | native member/value contract | kActorComponents | `0x68` | src/mod/session2/guest_lifecycle.h:197 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D043 | D | native member/value contract | kComponentsTransformSync | `0x1A0` | src/mod/session2/guest_lifecycle.h:198 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D044 | D | native member/value contract | kTsMoving | `0x3DC` | src/mod/session2/guest_lifecycle.h:199 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D045 | D | native member/value contract | kFmMoving | `0x58` | src/mod/session2/guest_lifecycle.h:200 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D046 | D | native member/value contract | kLoginCharacterReq | `3093` | src/mod/session2/guest_lifecycle.h:215 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D047 | D | native member/value contract | kGameLoadingStartReq | `2437` | src/mod/session2/guest_lifecycle.h:216 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D048 | D | native member/value contract | kMoveFieldCompleteReq | `2549` | src/mod/session2/guest_lifecycle.h:217 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D049 | D | native member/value contract | kLoadingCompleteReq | `2445` | src/mod/session2/guest_lifecycle.h:218 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D050 | D | native member/value contract | kMoveFieldAck | `2174` | src/mod/session2/guest_lifecycle.h:219 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D051 | D | native member/value contract | kMoveFieldCompleteAck | `2144` | src/mod/session2/guest_lifecycle.h:220 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D052 | D | native member/value contract | kLoadingCompleteAck | `2762` | src/mod/session2/guest_lifecycle.h:221 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D053 | D | function/global RVA | kLoadSlot | `0x186F570` | src/mod/session2/guest_save_host.cpp:42 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D054 | D | native signature | kLoadSlotHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x50, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xB0, 0x01, 0x00, 0x00, 0x4D, 0x8B, 0xF9}` | src/mod/session2/guest_save_host.cpp:43 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D055 | D | function/global RVA | kSave | `0x186E450` | src/mod/session2/guest_save_host.cpp:47 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D056 | D | native signature | kSaveHead | `{0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x78, 0xFE, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x88, 0x02, 0x00, 0x00}` | src/mod/session2/guest_save_host.cpp:48 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D057 | D | function/global RVA | kFlush | `0x1871930` | src/mod/session2/guest_save_host.cpp:53 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D058 | D | native signature | kFlushHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x40, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC0, 0x01, 0x00, 0x00, 0x45, 0x0F, 0xB7, 0xF9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2}` | src/mod/session2/guest_save_host.cpp:54 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D059 | D | function/global RVA | kGetOrCreate | `0x2355EC0` | src/mod/session2/guest_save_host.cpp:59 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D060 | D | native signature | kGetOrCreateHead | `{0xE9, 0xDB, 0xD8, 0xBF, 0x0C}` | src/mod/session2/guest_save_host.cpp:60 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D061 | D | function/global RVA | kAddFile | `0x235B100` | src/mod/session2/guest_save_host.cpp:61 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D062 | D | native signature | kAddFileHead | `{0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEA, 0x4C, 0x8B, 0xF9}` | src/mod/session2/guest_save_host.cpp:62 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D063 | D | function/global RVA | kCallSites[0].site | `0x2C17881` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D064 | D | function/global RVA | kCallSites[1].site | `0x2C3E1CE` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D065 | D | function/global RVA | kCallSites[2].site | `0x1042ABF3` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D066 | D | function/global RVA | kCallSites[3].site | `0xCBD86CE` | src/mod/session2/guest_save_host.cpp:70 | native call site to kSave | needs target evidence |
| D067 | D | function/global RVA | kCallSites[4].site | `0x103BE99A` | src/mod/session2/guest_save_host.cpp:70 | native call site to kSave | needs target evidence |
| D068 | D | function/global RVA | kCallSites[5].site | `0x28F273A` | src/mod/session2/guest_save_host.cpp:70 | native call site to kFlush | needs target evidence |
| D069 | D | function/global RVA | kCallSites[6].site | `0x108393C8` | src/mod/session2/guest_save_host.cpp:70 | native call site to kFlush | needs target evidence |
| D070 | D | function/global RVA | kErrBusy.site | `0x186F6DF` | src/mod/session2/guest_save_host.cpp:79 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D071 | D | function/global RVA | kErrBusy.global | `0x6CF7CA4` | src/mod/session2/guest_save_host.cpp:79 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D072 | D | function/global RVA | kErrInvalidSlot.site | `0x186F62C` | src/mod/session2/guest_save_host.cpp:80 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D073 | D | function/global RVA | kErrInvalidSlot.global | `0x6CF7900` | src/mod/session2/guest_save_host.cpp:80 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D074 | D | function/global RVA | kErrAbsentSlot.site | `0x2069615` | src/mod/session2/guest_save_host.cpp:81 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D075 | D | function/global RVA | kErrAbsentSlot.global | `0x6CF781C` | src/mod/session2/guest_save_host.cpp:81 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D076 | D | function/global RVA | kErrSaveFail.site | `0x186E780` | src/mod/session2/guest_save_host.cpp:82 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D077 | D | function/global RVA | kErrSaveFail.global | `0x6CF7CA0` | src/mod/session2/guest_save_host.cpp:82 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D078 | D | native member/value contract | kSlotMemoryOnly | `0x699` | src/mod/session2/guest_save_host.cpp:85 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D079 | D | native member/value contract | kSlotInMemory | `0x69C` | src/mod/session2/guest_save_host.cpp:86 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D080 | D | native member/size contract | kFileHeader | `0x98` | src/mod/session2/guest_save_host.cpp:87 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D081 | D | native member/value contract | kActorDescriptor | `0x88` | src/mod/session2/guest_save_host.cpp:88 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D082 | D | native member/value contract | kActorUser | `0xA0` | src/mod/session2/guest_save_host.cpp:89 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D083 | D | native member/value contract | kActorKey | `0x60` | src/mod/session2/guest_save_host.cpp:90 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D084 | D | native member/value contract | kMgrPendingSlot | `0x5C` | src/mod/session2/guest_save_host.cpp:91 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D085 | D | native member/size contract | kHeaderSize | `0x80` | src/mod/session2/guest_save_logic.h:102 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D086 | D | native member/value contract | kMagic | `0x45564153` | src/mod/session2/guest_save_logic.h:103 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D087 | D | native member/value contract | kSlotNameMap | `0x08` | src/mod/session2/guest_save_logic.h:199 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D088 | D | native member/size contract | kMapBucketCount | `0x00` | src/mod/session2/guest_save_logic.h:200 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D089 | D | native member/value contract | kMapBuckets | `0x10` | src/mod/session2/guest_save_logic.h:201 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D090 | D | native member/size contract | kMapEntries | `0x18` | src/mod/session2/guest_save_logic.h:202 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D091 | D | native member/size contract | kBucketSize | `0x100` | src/mod/session2/guest_save_logic.h:203 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D092 | D | native member/value contract | kBucketPairs | `0x08` | src/mod/session2/guest_save_logic.h:204 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D093 | D | native member/value contract | kMaxBucketPairs | `(kBucketSize - kBucketPairs) / 8` | src/mod/session2/guest_save_logic.h:205 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D094 | D | native member/value contract | kEntryKey | `0x08` | src/mod/session2/guest_save_logic.h:206 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D095 | D | native member/value contract | kEntryValue | `0x10` | src/mod/session2/guest_save_logic.h:207 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D096 | D | native member/value contract | kKeyChars | `0x00` | src/mod/session2/guest_save_logic.h:208 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D097 | D | native member/value contract | kKeyLength | `0x08` | src/mod/session2/guest_save_logic.h:209 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D098 | D | native member/value contract | kSlotNames | `0x688` | src/mod/session2/guest_save_logic.h:210 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D099 | D | native member/size contract | kSlotNameCount | `0x690` | src/mod/session2/guest_save_logic.h:211 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D100 | D | function/global RVA | kRetUserLogin | `0x2C17886` | src/mod/session2/guest_save_logic.h:462 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D101 | D | function/global RVA | kRetLobby3093 | `0x2C3E1D3` | src/mod/session2/guest_save_logic.h:463 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D102 | D | function/global RVA | kRetCharacter3093 | `0x1042ABF8` | src/mod/session2/guest_save_logic.h:464 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D103 | D | function/global RVA | kRetAutosave | `0xCBD86D3` | src/mod/session2/guest_save_logic.h:483 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D104 | D | function/global RVA | kRetManual2910 | `0x103BE99F` | src/mod/session2/guest_save_logic.h:484 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D105 | D | function/global RVA | kRetFlushLogout | `0x28F273F` | src/mod/session2/guest_save_logic.h:501 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D106 | D | function/global RVA | kRetFlushTimer | `0x108393CD` | src/mod/session2/guest_save_logic.h:502 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D107 | D | native identifier | kOpSaveAutoReq | `3037` | src/mod/session2/guest_save_wire.h:67 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D108 | D | native identifier | kOpSaveToFileCurrentPlayerReq | `2910` | src/mod/session2/guest_save_wire.h:68 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D109 | D | native identifier | kOpSaveToFileCurrentPlayerAck | `3016` | src/mod/session2/guest_save_wire.h:69 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D110 | D | native identifier | kOpCreateCharacterReq | `2711` | src/mod/session2/guest_save_wire.h:70 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D111 | D | native identifier | kOpNewGameStartReq | `2178` | src/mod/session2/guest_save_wire.h:71 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D112 | D | native identifier | kOpLoginCharacterReq | `3093` | src/mod/session2/guest_save_wire.h:72 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D113 | D | native identifier | kOpLogoutSessionReq | `2001` | src/mod/session2/guest_save_wire.h:73 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D114 | D | function/global RVA | kReporter | `0x1317D40` | src/mod/session2/report_guard.cpp:19 | required native report guard installed before save writes | needs target evidence |
| D115 | D | native signature | kReporterHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}` | src/mod/session2/report_guard.cpp:20 | required native report guard installed before save writes | needs target evidence |
| B074 | B | native member/size contract | kHeaderSize | `5` | src/common/troc_frame.h:30 | verify native packet framing; preserve the mod batch/control format | needs target evidence |
| B075 | B | native member/size contract | kMaxPacketSize | `0x8000 - 0x20` | src/common/troc_frame.h:33 | verify native packet framing; preserve the mod batch/control format | needs target evidence |
| D116 | D | native identifier | kOpLogoutSessionReq | `2001` | src/common/troc_frame.h:60 | verify native internal logout framing | needs target evidence |
| D117 | D | native identifier | kOpLogoutSessionAck | `2002` | src/common/troc_frame.h:61 | verify native internal logout framing | needs target evidence |
| D118 | D | native identifier | kOpLogoutUserAck | `2003` | src/common/troc_frame.h:62 | verify native internal logout framing | needs target evidence |
| D119 | D | native member/size contract | kInternalHeaderSize | `3` | src/common/troc_frame.h:63 | verify native internal logout framing | needs target evidence |
| D120 | D | native member/size contract | kLogoutAckSize | `7` | src/common/troc_frame.h:64 | verify native internal logout framing | needs target evidence |
| C135 | C | native identifier | kOpSetFieldTimeReq | `2125` | src/mod/session2/session2.cpp:276 | required deny filter protects host native world time | needs target evidence |
| C136 | C | native member/value contract | kActor_Key | `0x60` | src/mod/game/offsets.h:127 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C137 | C | function/global RVA | kActionLayerAdvance | `0x1F63F70` | src/mod/game/offsets.h:207 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C138 | C | native signature | kActionLayerAdvanceHead | `{0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0xC5, 0xF8, 0x29, 0x78, 0xD8}` | src/mod/game/offsets.h:208 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C139 | C | function/global RVA | kActionLayersAdvance | `0x2329700` | src/mod/game/offsets.h:218 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C140 | C | native signature | kActionLayersAdvanceHead | `{0x48, 0x8B, 0xC4, 0x41, 0x54, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x68, 0xC5, 0xF8, 0x29, 0x70, 0xC8}` | src/mod/game/offsets.h:219 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C141 | C | native member/value contract | kControl_Layer0 | `0x88` | src/mod/game/offsets.h:221 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C142 | C | native member/value contract | kLayer_Playback | `0x18` | src/mod/game/offsets.h:222 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C143 | C | native member/value contract | kLayer_Chart | `0x110` | src/mod/game/offsets.h:223 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C144 | C | native member/value contract | kActor_OwnerUser | `0x90` | src/mod/game/offsets.h:230 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| A029 | A | native member/size contract | NativeStatUnits | `1000.0f` | src/mod/game/player_layout.h:42 | native int64 stat conversion scale (not a timeout) | needs target evidence |
| A030 | A | native member/size contract | StatIdReadWidth | `8` | src/mod/game/player_layout.h:43 | legacy reader width; target native ID width must be confirmed separately from padding | needs target evidence |
| A031 | A | function/global RVA | kWorldSystemSlot | `0x6D69190` | src/mod/game/offsets.h:15 | PlayerReader root and inherited save-manager root | needs target evidence |
| A032 | A | function/global RVA | kHpDefinitionsSlot | `0x6D69528` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:23 | native HP updater selects game-data definition registry | needs target evidence |
| A033 | A | function/global RVA | kHpDefinitionsAltSlot | `0x6D69530` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:25 | alternate native HP definition registry; no assumed selector ID | needs target evidence |
| A034 | A | native member/value contract | kHpDefinitionsRoot | `0x00` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:26 | first native definition pointer hop | needs target evidence |
| A035 | A | native member/value contract | kHpDefinitionsTables | `0x08` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:27 | second native definition pointer hop | needs target evidence |
| A036 | A | native member/value contract | kHpDefinitionsObject | `0x10` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:28 | third native definition pointer hop | needs target evidence |
| A037 | A | native member/value contract | kHpDefinitionId | `0xA0` | docs/probe-3663/port-evidence/r2/player-health-update-reference.txt:29 | native HP key field; target field is different | needs target evidence |
| C145 | C | native member/value contract | FieldLockAcquireVslot | `8` | src/mod/session2/field_share.cpp:65 | both field lock acquire slots | needs target evidence |
| C146 | C | native member/value contract | FieldLockReleaseVslot | `0x10` | src/mod/session2/field_share.cpp:74 | both field lock release slots | needs target evidence |
| C147 | C | native member/value contract | HostCharacterComponents | `0x68` | src/mod/session2/field_share.cpp:115 | field-share traversal to host components | needs target evidence |
| C148 | C | native member/value contract | HostTransformSyncComponent | `0x1A0` | src/mod/session2/field_share.cpp:115 | field-share traversal to native transform sync | needs target evidence |
| C149 | C | native layout contract | FieldUserEntryLayout | `16 B; user +0, actors +8` | src/mod/session2/field_share.h:33 | native field user vector element, confirm both member offsets and stride | needs target evidence |
| D121 | D | native member/value contract | NativeSlotNumberDomain | `0..3;100..108;200..207` | src/mod/session2/guest_save_logic.h:80 | verify supported native slots before choosing reserved mod slot204 | needs target evidence |
| D122 | D | native member/value contract | NativeMemoryOnlySlots | `201;205..207` | src/mod/session2/guest_save_logic.h:83 | verify which slots cannot be persisted | needs target evidence |
| D123 | D | native layout contract | SaveCurrentAckLayout | `14 B; key0 slot4 ok8 flag9 result10` | src/mod/session2/guest_save_guest.cpp:777 | native 3016 result parser | needs target evidence |
| C150 | C | native identifier | NativeForbiddenRequests | `2790,2038,2004,1001..1008` | src/mod/session2/session2.cpp:278 | native shutdown, pointer and connection messages must never reach remote server | needs target evidence |
| D124 | D | native layout contract | SaveReq2910Layout | `slot0:u32; length4:u32; UTF16+8; terminator; 10+2n bytes; n<=30` | src/mod/session2/guest_save_wire.h:383 | verify native 2910 writer and handler, including maximum native name length | needs target evidence |
| D125 | D | native layout contract | LoginCharacterSaveLayout | `flag0:u8; characterNo1:u64; slot9:u32` | src/mod/session2/guest_save_wire.h:72 | native 3093 payload used for selecting and holding the guest save | needs target evidence |
| D126 | D | native layout contract | NativeSaveHeaderLayout | `magic0:u32; version4:u16 in1..2; size6:u16=80h; flagsCh:u32 mask2; raw12h:u32>0; data16h:u32 in1..raw` | src/mod/session2/guest_save_logic.h:152 | native save validator schema, including inline field offsets and domain checks; file/raw size caps remain policy | needs target evidence |
