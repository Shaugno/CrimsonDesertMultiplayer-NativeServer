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

Counts: A 30, B 75, C 150, D 125; total 380; R1 adopted 17.

| Row | Tier | Symbol | 2976 value | Source | Use | Evidence status |
|---|---|---|---|---|---|---|
| A001 | A | kChunkOrigin | `0x6D56500` | src/mod/game/offsets.h:16 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A002 | A | kStaticPlayerPos | `0x6D77858` | src/mod/game/offsets.h:17 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A003 | A | kVtClientActorManager | `0x55B62C8` | src/mod/game/offsets.h:20 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A004 | A | kVtControlledActor | `0x5585C40` | src/mod/game/offsets.h:21 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A005 | A | kVtCharacterControl | `0x55B5460` | src/mod/game/offsets.h:22 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A006 | A | kVtStatusComponent | `0x558D868` | src/mod/game/offsets.h:23 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A007 | A | kVtPhysicsObject | `0x5D00F88` | src/mod/game/offsets.h:24 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A008 | A | kWorldSystem_ActorManager | `0x30` | src/mod/game/offsets.h:27 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A009 | A | kActorManager_ControlledActor | `0x50` | src/mod/game/offsets.h:29 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A010 | A | kActor_ComponentTable | `0x68` | src/mod/game/offsets.h:31 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A011 | A | kComponents_Status | `0x20` | src/mod/game/offsets.h:33 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A012 | A | kComponents_CharacterControl | `0x40` | src/mod/game/offsets.h:34 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A013 | A | kControl_Inner | `0x140` | src/mod/game/offsets.h:36 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A014 | A | kControlInner_Physics | `0x2C0` | src/mod/game/offsets.h:37 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A015 | A | kPhysics_AxisX | `0x60` | src/mod/game/offsets.h:39 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A016 | A | kPhysics_AxisY | `0x70` | src/mod/game/offsets.h:40 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A017 | A | kPhysics_AxisZ | `0x80` | src/mod/game/offsets.h:41 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A018 | A | kPhysics_Position | `0x90` | src/mod/game/offsets.h:42 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A019 | A | kStatus_StatContainer | `0x18` | src/mod/game/offsets.h:47 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A020 | A | kStatContainer_Entries | `0x58` | src/mod/game/offsets.h:48 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A021 | A | kStatContainer_Count | `0x60` | src/mod/game/offsets.h:49 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A022 | A | kStatEntry_Size | `0x90` | src/mod/game/offsets.h:50 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A023 | A | kStatEntry_Id | `0x00` | src/mod/game/offsets.h:51 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A024 | A | kStatEntry_Current | `0x08` | src/mod/game/offsets.h:52 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A025 | A | kStatEntry_Max | `0x18` | src/mod/game/offsets.h:53 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A026 | A | kStatId_Health | `0` | src/mod/game/offsets.h:54 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A027 | A | kStatId_Stamina | `22` | src/mod/game/offsets.h:55 | PlayerReader::resolve_actor/read/read_stats: guarded local snapshot | needs target evidence |
| A028 | A | kLoadingParkHeight | `1000.0f` | src/mod/game/offsets.h:236 | PlayerReader::read loading sentinel | needs target evidence |
| B001 | B | kServerNetSlot | `0x6D69680` | src/mod/session2/offsets_s2.h:12 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B002 | B | kClientNetSlot | `0x6D69170` | src/mod/session2/offsets_s2.h:13 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B003 | B | kServerNet_Session | `0x18` | src/mod/session2/offsets_s2.h:17 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B004 | B | kServerNet_InQueue | `0x20` | src/mod/session2/offsets_s2.h:18 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B005 | B | kServerNet_FreeC2S | `0x78` | src/mod/session2/offsets_s2.h:19 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B006 | B | kServerNet_FreeS2C | `0x80` | src/mod/session2/offsets_s2.h:20 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B007 | B | kClientNet_InQueue | `0x18` | src/mod/session2/offsets_s2.h:22 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B008 | B | kNwVirtualAsyncSessionVtable | `0x57260C0` | src/mod/session2/offsets_s2.h:28 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B009 | B | kNwAsyncSessionVtable | `0x5726140` | src/mod/session2/offsets_s2.h:29 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B010 | B | kSessionVtableEntries | `15` | src/mod/session2/offsets_s2.h:30 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B011 | B | kSessionVfSend | `9` | src/mod/session2/offsets_s2.h:31 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B012 | B | kSessionSize | `0xD0` | src/mod/session2/offsets_s2.h:32 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B013 | B | kSession_Name | `0x18` | src/mod/session2/offsets_s2.h:33 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B014 | B | kSession_IdPtr | `0x88` | src/mod/session2/offsets_s2.h:34 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B015 | B | kSession_User | `0x90` | src/mod/session2/offsets_s2.h:35 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B016 | B | kSession_Crypto | `0x98` | src/mod/session2/offsets_s2.h:36 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B017 | B | kSession_B0 | `0xB0` | src/mod/session2/offsets_s2.h:37 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B018 | B | kSession_B8 | `0xB8` | src/mod/session2/offsets_s2.h:38 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B019 | B | kSession_C0 | `0xC0` | src/mod/session2/offsets_s2.h:39 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B020 | B | kSession_OutQueue | `0xC8` | src/mod/session2/offsets_s2.h:40 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B021 | B | kServerOnRecvPacket | `0x2773C20` | src/mod/session2/offsets_s2.h:51 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B022 | B | kServerOnRecvPacketHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54}` | src/mod/session2/offsets_s2.h:52 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B023 | B | kSessionSend | `0x1437740` | src/mod/session2/offsets_s2.h:54 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B024 | B | kSessionSendHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30}` | src/mod/session2/offsets_s2.h:55 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B025 | B | kNwSessionCtor | `0x1436EE0` | src/mod/session2/offsets_s2.h:57 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B026 | B | kNwSessionCtorHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x6C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56}` | src/mod/session2/offsets_s2.h:58 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B027 | B | kQueuePush | `0x35DF50` | src/mod/session2/offsets_s2.h:60 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B028 | B | kQueuePushHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}` | src/mod/session2/offsets_s2.h:61 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B029 | B | kAllocGlobal | `0x47EFA78` | src/mod/session2/offsets_s2.h:71 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B030 | B | kAllocGlobalHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA}` | src/mod/session2/offsets_s2.h:72 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B031 | B | kAllocThread | `0x47EF9B0` | src/mod/session2/offsets_s2.h:74 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B032 | B | kAllocThreadBytes | `{ 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0x7E, 0x92, 0x49, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF}` | src/mod/session2/offsets_s2.h:76 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B033 | B | kTlsIndexSlot | `0x6C88C50` | src/mod/session2/offsets_s2.h:83 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B034 | B | kTls_UseGlobalHeap | `0x1FD` | src/mod/session2/offsets_s2.h:84 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B035 | B | kServerNet_Threads | `0xF8` | src/mod/session2/offsets_s2.h:89 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B036 | B | kServerNet_ThreadCount | `0x100` | src/mod/session2/offsets_s2.h:90 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B037 | B | kServerThreadEntry | `0x10` | src/mod/session2/offsets_s2.h:91 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B038 | B | kArray_Count | `0x08` | src/mod/session2/offsets_s2.h:107 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B039 | B | kArray_Capacity | `0x0C` | src/mod/session2/offsets_s2.h:108 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B040 | B | kPbSize | `0x8030` | src/mod/session2/offsets_s2.h:111 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B041 | B | kPb_FramePtr | `0x8000` | src/mod/session2/offsets_s2.h:112 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B042 | B | kPb_Len1 | `0x8008` | src/mod/session2/offsets_s2.h:113 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B043 | B | kPb_Len2 | `0x800A` | src/mod/session2/offsets_s2.h:114 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B044 | B | kPb_Cap | `0x800C` | src/mod/session2/offsets_s2.h:115 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B045 | B | kPb_IsRequest | `0x800E` | src/mod/session2/offsets_s2.h:116 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B046 | B | kPb_Tag | `0x8010` | src/mod/session2/offsets_s2.h:117 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B047 | B | kPb_Magic | `0x8018` | src/mod/session2/offsets_s2.h:118 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B048 | B | kPb_Base | `0x8028` | src/mod/session2/offsets_s2.h:119 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B049 | B | kFrameOverhead | `0x1F` | src/mod/session2/offsets_s2.h:120 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B050 | B | kFrameHeader | `3` | src/mod/session2/offsets_s2.h:121 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B051 | B | kRq_Frame | `0x30` | src/mod/session2/offsets_s2.h:123 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B052 | B | kTrocInstancesBegin | `0x6A51AE0` | src/mod/session2/offsets_s2.h:127 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B053 | B | kTrocInstancesEnd | `0x6A82C80` | src/mod/session2/offsets_s2.h:128 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B054 | B | kTroc_Kind | `0x08` | src/mod/session2/offsets_s2.h:129 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B055 | B | kTroc_Opcode | `0x0C` | src/mod/session2/offsets_s2.h:130 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B056 | B | kTroc_Channel | `0x14` | src/mod/session2/offsets_s2.h:131 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B057 | B | kTroc_Flags | `0x16` | src/mod/session2/offsets_s2.h:132 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B058 | B | kTrocKindServer | `4` | src/mod/session2/offsets_s2.h:133 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B059 | B | kClientNet_Session | `0x10` | src/mod/session2/offsets_s2.h:136 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B060 | B | kClientNet_FreeC2S | `0x28` | src/mod/session2/offsets_s2.h:137 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B061 | B | kClientNet_Slot4 | `0x58` | src/mod/session2/offsets_s2.h:138 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B062 | B | kSyncSession_Epoch | `0xB0` | src/mod/session2/offsets_s2.h:139 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B063 | B | kSyncSessionSend | `0x14380B0` | src/mod/session2/offsets_s2.h:140 | server_install/guest_install: session, queue, buffer and descriptor access | R1 done |
| B064 | B | kSyncSessionSendHead | `{0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA}` | src/mod/session2/offsets_s2.h:141 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B065 | B | kNetLoopPump | `0x35BAD4` | src/mod/session2/offsets_s2.h:143 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B066 | B | kNetLoopPumpHead | `{0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7}` | src/mod/session2/offsets_s2.h:144 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B067 | B | kNetLoopPumpPatch | `7` | src/mod/session2/offsets_s2.h:153 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B068 | B | kNetLoopPumpAob | `{ 0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74}` | src/mod/session2/offsets_s2.h:154 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B069 | B | kClientOnRecvPacket | `0x35CA00` | src/mod/session2/offsets_s2.h:157 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| B070 | B | kClientOnRecvPacketHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24}` | src/mod/session2/offsets_s2.h:158 | server_install/guest_install: session, queue, buffer and descriptor access | needs target evidence |
| C001 | C | kSamSlot | `0x6DA67D8` | src/mod/session2/offsets_s2.h:14 | server users and native recipient selection | needs target evidence |
| C002 | C | kSam_FreeKeys | `0xD0` | src/mod/session2/offsets_s2.h:24 | server users and native recipient selection | needs target evidence |
| C003 | C | kSam_ActorCount | `0x9C` | src/mod/session2/offsets_s2.h:25 | server users and native recipient selection | needs target evidence |
| C004 | C | kServerUserActorVtable | `0x5B257A8` | src/mod/session2/offsets_s2.h:43 | server users and native recipient selection | needs target evidence |
| C005 | C | kUser_Key | `0x60` | src/mod/session2/offsets_s2.h:44 | server users and native recipient selection | needs target evidence |
| C006 | C | kUser_Session | `0x1F8` | src/mod/session2/offsets_s2.h:45 | server users and native recipient selection | needs target evidence |
| C007 | C | kServerActor_Controller | `0x90` | src/mod/session2/offsets_s2.h:47 | server users and native recipient selection | needs target evidence |
| C008 | C | kServerActor_Key | `0x60` | src/mod/session2/offsets_s2.h:48 | server users and native recipient selection | needs target evidence |
| C009 | C | kToNearExceptOneControlVtable | `0x5B25358` | src/mod/session2/offsets_s2.h:95 | server users and native recipient selection | needs target evidence |
| C010 | C | kToNearExceptOneSelfVtable | `0x5B25378` | src/mod/session2/offsets_s2.h:96 | server users and native recipient selection | needs target evidence |
| C011 | C | kToNearVtable | `0x5B25418` | src/mod/session2/offsets_s2.h:97 | server users and native recipient selection | needs target evidence |
| C012 | C | kExceptOneVf1 | `0x2A759B0` | src/mod/session2/offsets_s2.h:98 | server users and native recipient selection | needs target evidence |
| C013 | C | kExceptOneSelfVf1 | `0x2A75880` | src/mod/session2/offsets_s2.h:99 | server users and native recipient selection | needs target evidence |
| C014 | C | kToNearVf1 | `0x2A75860` | src/mod/session2/offsets_s2.h:100 | server users and native recipient selection | needs target evidence |
| C015 | C | kExceptOneVf1Head | `{0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00}` | src/mod/session2/offsets_s2.h:101 | server users and native recipient selection | needs target evidence |
| C016 | C | kToNearVf1Head | `{0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9}` | src/mod/session2/offsets_s2.h:103 | server users and native recipient selection | needs target evidence |
| C017 | C | kFinder_Actor | `0x10` | src/mod/session2/offsets_s2.h:105 | server users and native recipient selection | needs target evidence |
| D001 | D | kWorldSystem_SaveManager | `0x88` | src/mod/session2/offsets_s2.h:166 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D002 | D | kServerActorLookupSlot | `0x6D69698` | src/mod/session2/offsets_s2.h:167 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D003 | D | kServerActorLookup_SaveManager | `0x48` | src/mod/session2/offsets_s2.h:168 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D004 | D | kSaveMgr_Lock | `0x00` | src/mod/session2/offsets_s2.h:170 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D005 | D | kSaveMgr_LockObj | `0x08` | src/mod/session2/offsets_s2.h:171 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D006 | D | kSaveMgr_Slots | `0x40` | src/mod/session2/offsets_s2.h:172 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D007 | D | kSaveMgr_Count | `0x48` | src/mod/session2/offsets_s2.h:173 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D008 | D | kSaveMgr_Capacity | `0x4C` | src/mod/session2/offsets_s2.h:174 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D009 | D | kSaveMgr_Storage | `0x50` | src/mod/session2/offsets_s2.h:175 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D010 | D | kSaveMgr_Busy | `0x60` | src/mod/session2/offsets_s2.h:176 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D011 | D | kSaveMgr_EntrySize | `0x68` | src/mod/session2/offsets_s2.h:177 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D012 | D | kSaveStorage_FolderCount | `0x40` | src/mod/session2/offsets_s2.h:178 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D013 | D | kWindowsLock3Vtable | `0x557C120` | src/mod/session2/offsets_s2.h:179 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D014 | D | kWindowsLockVf1 | `0x1371340` | src/mod/session2/offsets_s2.h:180 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D015 | D | kWindowsLockVf2 | `0xCE4EF0` | src/mod/session2/offsets_s2.h:181 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D016 | D | kWindowsLockVf1Head | `{0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18}` | src/mod/session2/offsets_s2.h:182 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D017 | D | kWindowsLockVf2Head | `{0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20}` | src/mod/session2/offsets_s2.h:183 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D018 | D | kSaveMgrRefreshSlots | `0x1870C20` | src/mod/session2/offsets_s2.h:185 | guest refresh_title_slots and protected save-manager table | R1 done |
| D019 | D | kSaveMgrRefreshSlotsHead | `{0xE9, 0xAB, 0x86, 0x36, 0x0B}` | src/mod/session2/offsets_s2.h:186 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D020 | D | kSlotRefreshReqSequence | `0x2A22475` | src/mod/session2/offsets_s2.h:190 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D021 | D | kSlotRefreshReqSequenceBytes | `{ 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10}` | src/mod/session2/offsets_s2.h:191 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D022 | D | kTitlePlayReadsSaveMgr | `0x65DC3E` | src/mod/session2/offsets_s2.h:211 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| D023 | D | kTitlePlayReadsSaveMgrBytes | `{0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00}` | src/mod/session2/offsets_s2.h:212 | guest refresh_title_slots and protected save-manager table | needs target evidence |
| C018 | C | kRefs | `0x10` | src/mod/session2/field_share.h:41 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C019 | C | kLock1 | `0x18` | src/mod/session2/field_share.h:42 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C020 | C | kLock2Ptr | `0x08` | src/mod/session2/field_share.h:43 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C021 | C | kOpen | `0x4A` | src/mod/session2/field_share.h:44 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C022 | C | kOwner | `0x180` | src/mod/session2/field_share.h:45 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C023 | C | kUsers | `0x2A0` | src/mod/session2/field_share.h:46 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C024 | C | kUserCount | `0x2A8` | src/mod/session2/field_share.h:47 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C025 | C | kUserActors | `0x2B0` | src/mod/session2/field_share.h:48 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C026 | C | kKey | `0x2C8` | src/mod/session2/field_share.h:49 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C027 | C | kLevelEntry | `0x2D0` | src/mod/session2/field_share.h:50 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C028 | C | kTsFieldMover | `0x4F0` | src/mod/session2/field_share.h:52 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C029 | C | kFmFieldKey | `0x08` | src/mod/session2/field_share.h:53 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C030 | C | kFmField | `0x10` | src/mod/session2/field_share.h:54 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C031 | C | kFmBusy | `0x5A` | src/mod/session2/field_share.h:55 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C032 | C | kOutField | `0x08` | src/mod/session2/field_share.h:58 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C033 | C | kOutValid | `0x10` | src/mod/session2/field_share.h:59 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C034 | C | kOutHolder | `0x18` | src/mod/session2/field_share.h:60 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C035 | C | kOutNew | `0x20` | src/mod/session2/field_share.h:61 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C036 | C | kOutInfoKey | `0x24` | src/mod/session2/field_share.h:62 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C037 | C | kMgrBucketCount | `0x48` | src/mod/session2/field_share.h:66 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C038 | C | kMgrEntries | `0x4C` | src/mod/session2/field_share.h:67 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C039 | C | kMgrBuckets | `0x58` | src/mod/session2/field_share.h:68 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C040 | C | kMgrHolders | `0x60` | src/mod/session2/field_share.h:69 | share_candidate/share_give_field/enter_insert/enter_undo/field snapshot | needs target evidence |
| C041 | C | kAcquireField | `0x2B07790` | src/mod/session2/field_share.cpp:20 | field_share_install and acquire/enter detours | needs target evidence |
| C042 | C | kAcquireFieldHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70}` | src/mod/session2/field_share.cpp:21 | field_share_install and acquire/enter detours | needs target evidence |
| C043 | C | kEnterField | `0x2BC04F0` | src/mod/session2/field_share.cpp:24 | field_share_install and acquire/enter detours | needs target evidence |
| C044 | C | kEnterFieldHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC}` | src/mod/session2/field_share.cpp:25 | field_share_install and acquire/enter detours | needs target evidence |
| C045 | C | kTryAddRef | `0x14343A0` | src/mod/session2/field_share.cpp:28 | field_share_install and acquire/enter detours | needs target evidence |
| C046 | C | kTryAddRefHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9}` | src/mod/session2/field_share.cpp:29 | field_share_install and acquire/enter detours | needs target evidence |
| C047 | C | kVecPush | `0x40B5B0` | src/mod/session2/field_share.cpp:31 | field_share_install and acquire/enter detours | needs target evidence |
| C048 | C | kVecPushHead | `{0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, 0x4C, 0x8B, 0xF2, 0x8B, 0x49, 0x08, 0x8B, 0x43, 0x0C}` | src/mod/session2/field_share.cpp:32 | field_share_install and acquire/enter detours | needs target evidence |
| C049 | C | kUserMainCharacter | `0xD0` | src/mod/session2/field_share.cpp:35 | field_share_install and acquire/enter detours | needs target evidence |
| C050 | C | kUserKey | `0x60` | src/mod/session2/field_share.cpp:36 | field_share_install and acquire/enter detours | needs target evidence |
| C051 | C | kActorUser | `0xA0` | src/mod/session2/field_share.cpp:37 | field_share_install and acquire/enter detours | needs target evidence |
| C052 | C | kNative | `0x7FFFFFFFFFFFFFFF` | src/mod/session2/identity_fix.h:45 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C053 | C | kLobbyOwner | `0x08` | src/mod/session2/identity_fix.h:48 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C054 | C | kLobbyList | `0x18` | src/mod/session2/identity_fix.h:49 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C055 | C | kListCount | `0x08` | src/mod/session2/identity_fix.h:50 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C056 | C | kEntryStride | `0xD0` | src/mod/session2/identity_fix.h:51 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C057 | C | kEntryNumber | `0x00` | src/mod/session2/identity_fix.h:52 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C058 | C | kUserComponents | `0x68` | src/mod/session2/identity_fix.h:53 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C059 | C | kTableLobby | `0x10` | src/mod/session2/identity_fix.h:54 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C060 | C | kTableStatus | `0x20` | src/mod/session2/identity_fix.h:55 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C061 | C | kStatusActor | `0x08` | src/mod/session2/identity_fix.h:56 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C062 | C | kStatusNumber | `0x70` | src/mod/session2/identity_fix.h:57 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C063 | C | kActorKey | `0x60` | src/mod/session2/identity_fix.h:58 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C064 | C | kActorUser | `0xA0` | src/mod/session2/identity_fix.h:59 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C065 | C | kLockObject | `0x08` | src/mod/session2/identity_fix.h:60 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C066 | C | kCharacterClass | `0xA` | src/mod/session2/identity_fix.h:63 | identity_fix_install: unique user identity and status/lobby layout | needs target evidence |
| C067 | C | kLobbyLoad | `0x2B7F4D0` | src/mod/session2/identity_fix.cpp:215 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C068 | C | kLobbyLoadHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xF2, 0x4C, 0x8B, 0xF9, 0x49, 0x8B, 0x39}` | src/mod/session2/identity_fix.cpp:216 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C069 | C | kLobbyVtable | `0x5B2BE58` | src/mod/session2/identity_fix.cpp:219 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C070 | C | kLobbyConstant | `0x2B7F66C` | src/mod/session2/identity_fix.cpp:220 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C071 | C | kLobbyConstantBytes | `{0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x48, 0x89, 0x44, 0x24, 0x70}` | src/mod/session2/identity_fix.cpp:221 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C072 | C | kLobbyAddCall | `0x2B7F6DB` | src/mod/session2/identity_fix.cpp:223 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C073 | C | kLobbyAddCallBytes | `{0xE8, 0x90, 0x47, 0x5C, 0xFF}` | src/mod/session2/identity_fix.cpp:224 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C074 | C | kClearLobbySite | `0x2C3E0FF` | src/mod/session2/identity_fix.cpp:226 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C075 | C | kClearLobbySiteBytes | `{0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}` | src/mod/session2/identity_fix.cpp:227 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C076 | C | kClearListSite | `0x2C3E126` | src/mod/session2/identity_fix.cpp:228 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C077 | C | kClearListSiteBytes | `{0x48, 0x8B, 0x4B, 0x18}` | src/mod/session2/identity_fix.cpp:229 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C078 | C | kStatusVtable | `0x5B203F0` | src/mod/session2/identity_fix.cpp:230 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C079 | C | kStatusLoad | `0x293FE70` | src/mod/session2/identity_fix.cpp:231 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C080 | C | kLoginMgrSite | `0x2777A18` | src/mod/session2/identity_fix.cpp:234 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C081 | C | kLoginMgrSiteBytes | `{0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}` | src/mod/session2/identity_fix.cpp:235 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C082 | C | kLoginMgrSlot | `0x6D696A8` | src/mod/session2/identity_fix.cpp:236 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C083 | C | kFindByNoScan | `0x2A84066` | src/mod/session2/identity_fix.cpp:237 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C084 | C | kFindByNoScanBytes | `{0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x14, 0xC1}` | src/mod/session2/identity_fix.cpp:238 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C085 | C | kFindByNoRead | `0x2A84090` | src/mod/session2/identity_fix.cpp:240 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C086 | C | kFindByNoReadBytes | `{0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20}` | src/mod/session2/identity_fix.cpp:241 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C087 | C | kLoginMgrActors | `0xB8` | src/mod/session2/identity_fix.cpp:242 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C088 | C | kLoginMgrCount | `0xC0` | src/mod/session2/identity_fix.cpp:243 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C089 | C | kOpLoginUser | `2490` | src/mod/session2/identity_fix.cpp:244 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C090 | C | kOpLoginCharacter | `3093` | src/mod/session2/identity_fix.cpp:245 | identity_fix_install code validation, lobby/status and login-manager access | needs target evidence |
| C091 | C | kActorComponents | `0x68` | src/mod/session2/identity_probe.h:48 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C092 | C | kTableStatus | `0x20` | src/mod/session2/identity_probe.h:49 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C093 | C | kStatusCharacterNo | `0x70` | src/mod/session2/identity_probe.h:50 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C094 | C | kComponentOwner | `0x08` | src/mod/session2/identity_probe.h:51 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C095 | C | kActorUser | `0xA0` | src/mod/session2/identity_probe.h:52 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C096 | C | kKey | `0x60` | src/mod/session2/identity_probe.h:53 | shared identity helpers used by the fix and guest-save hooks | needs target evidence |
| C097 | C | kLoad | `0x20688A0` | src/mod/session2/identity_probe.cpp:31 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C098 | C | kLoadHead | `{0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x68, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00, 0x49, 0x8B, 0xD9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xF1}` | src/mod/session2/identity_probe.cpp:32 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C099 | C | kPhases | `0x2941020` | src/mod/session2/identity_probe.cpp:50 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C100 | C | kPhasesHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78}` | src/mod/session2/identity_probe.cpp:51 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C101 | C | kStatusVtable | `0x5B203F0` | src/mod/session2/identity_probe.cpp:83 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C102 | C | kStatusLoad | `0x293FE70` | src/mod/session2/identity_probe.cpp:84 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C103 | C | kLoadStub | `0x35EA90` | src/mod/session2/identity_probe.cpp:85 | identity_shared_hooks_install: shared load/phases entries and status validation | needs target evidence |
| C104 | C | kTsActor | `0x08` | src/mod/session2/clock_gate.h:41 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C105 | C | kActorUser | `0xA0` | src/mod/session2/clock_gate.h:42 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C106 | C | kUserKey | `0x60` | src/mod/session2/clock_gate.h:43 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C107 | C | kTsFieldTime | `0x450` | src/mod/session2/clock_gate.h:44 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C108 | C | kFieldTimeSize | `32` | src/mod/session2/clock_gate.h:45 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C109 | C | kWorldTime | `0x6A6D238` | src/mod/session2/clock_gate.h:46 | clock_gate_install: prevent the guest changing host field time | needs target evidence |
| C110 | C | kTravel | `0x2BC0A30` | src/mod/session2/clock_gate.cpp:21 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C111 | C | kTravelHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}` | src/mod/session2/clock_gate.cpp:22 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C112 | C | kTravelOwnerCmp | `0x2BC0A78` | src/mod/session2/clock_gate.cpp:25 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C113 | C | kTravelOwnerCmpBytes | `{0x39, 0x82, 0x80, 0x01, 0x00, 0x00}` | src/mod/session2/clock_gate.cpp:26 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C114 | C | kTravelGetterCall | `0x2BC0CC7` | src/mod/session2/clock_gate.cpp:27 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C115 | C | kTravelGetterCallBytes | `{0xE8, 0xD4, 0x48, 0x52, 0xFF}` | src/mod/session2/clock_gate.cpp:28 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C116 | C | kLoading | `0x219D7F0` | src/mod/session2/clock_gate.cpp:29 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C117 | C | kLoadingHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x60, 0x4D, 0x8B, 0xE9, 0x49, 0x8B, 0xF0, 0x4C, 0x8B, 0xF2, 0x48, 0x8B, 0xD9}` | src/mod/session2/clock_gate.cpp:30 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C118 | C | kLoadingFlagCmp | `0x219D82B` | src/mod/session2/clock_gate.cpp:34 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C119 | C | kLoadingFlagCmpBytes | `{0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00}` | src/mod/session2/clock_gate.cpp:35 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C120 | C | kGetter | `0x20E55A0` | src/mod/session2/clock_gate.cpp:36 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C121 | C | kGetterHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0xFA, 0x48, 0x8D, 0x54, 0x24, 0x70, 0x48, 0x8B, 0xD9, 0xFF, 0x50, 0x60}` | src/mod/session2/clock_gate.cpp:37 | clock_gate_install entry and call-neighbour checks | needs target evidence |
| C122 | C | kSessionId | `0` | src/mod/session2/server.cpp:76 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C123 | C | kOpLoginCharacterToFieldServerReq | `3093` | src/mod/session2/server.cpp:133 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C124 | C | kOpLoginUserToFieldServerReq | `2490` | src/mod/session2/server.cpp:134 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C125 | C | kOpMainPlayerDataStartAck | `2756` | src/mod/session2/server.cpp:138 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C126 | C | kOpMainUserDataStartAck | `2646` | src/mod/session2/server.cpp:139 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C127 | C | kOpMoveFieldAck | `2174` | src/mod/session2/server.cpp:140 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| C128 | C | kOpNak | `1013` | src/mod/session2/server.cpp:181 | server login filtering and native main-user/main-player acknowledgement tracking | needs target evidence |
| B071 | B | kOpConnectCryptogramToServerReq | `2319` | src/mod/session2/guest.cpp:71 | guest handshake and epoch ownership | needs target evidence |
| B072 | B | kNoEpoch | `0x7FFFFFFFFFFFFFFFull` | src/mod/session2/guest.cpp:72 | guest handshake and epoch ownership | needs target evidence |
| B073 | B | kServerFrameworkMode | `0x6A65564` | src/mod/session2/guest.cpp:84 | guest handshake and epoch ownership | needs target evidence |
| C129 | C | kOpScopeSenderAck | `2042` | src/mod/session2/guest.cpp:76 | guest native login and inbound packet classification | needs target evidence |
| C130 | C | kOpActorAppear | `2005` | src/mod/session2/guest.cpp:77 | guest native login and inbound packet classification | needs target evidence |
| C131 | C | kOpDataAck | `2869` | src/mod/session2/guest.cpp:78 | guest native login and inbound packet classification | needs target evidence |
| C132 | C | kOp2679 | `2679` | src/mod/session2/guest.cpp:79 | guest native login and inbound packet classification | needs target evidence |
| C133 | C | kOpLoginCharacterToFieldServerReq | `3093` | src/mod/session2/guest.cpp:80 | guest native login and inbound packet classification | needs target evidence |
| C134 | C | kOpLoginUserToFieldServerReq | `2490` | src/mod/session2/guest.cpp:214 | guest native login and inbound packet classification | needs target evidence |
| D024 | D | kOpReserveShutDownServerReq | `2790` | src/mod/session2/guest.cpp:81 | guest native shutdown filter | needs target evidence |
| D025 | D | kSamMapBuckets | `0x98` | src/mod/session2/guest_lifecycle.h:50 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D026 | D | kSamMapSize | `0x9C` | src/mod/session2/guest_lifecycle.h:51 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D027 | D | kSamMapBucketBase | `0xA8` | src/mod/session2/guest_lifecycle.h:52 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D028 | D | kSamMapHolders | `0xB0` | src/mod/session2/guest_lifecycle.h:53 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D029 | D | kSamBucketBytes | `0x100` | src/mod/session2/guest_lifecycle.h:54 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D030 | D | kSamBucketMaxEntries | `(kSamBucketBytes - 8) / 8` | src/mod/session2/guest_lifecycle.h:55 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D031 | D | kSamKeyIndexMask | `0xFFFFF` | src/mod/session2/guest_lifecycle.h:56 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D032 | D | kSamUserList | `0xB8` | src/mod/session2/guest_lifecycle.h:58 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D033 | D | kSamUserCount | `0xC0` | src/mod/session2/guest_lifecycle.h:59 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D034 | D | kActorKey | `0x60` | src/mod/session2/guest_lifecycle.h:64 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D035 | D | kUserMainCharacter | `0xD0` | src/mod/session2/guest_lifecycle.h:68 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D036 | D | kUserComponents | `0x68` | src/mod/session2/guest_lifecycle.h:69 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D037 | D | kComponentsSlots | `0x18` | src/mod/session2/guest_lifecycle.h:70 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D038 | D | kSlotsArray | `0x18` | src/mod/session2/guest_lifecycle.h:71 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D039 | D | kSlotsCount | `0x20` | src/mod/session2/guest_lifecycle.h:72 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D040 | D | kSlotStride | `0x10` | src/mod/session2/guest_lifecycle.h:73 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D041 | D | kActorState | `0x5E` | src/mod/session2/guest_lifecycle.h:196 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D042 | D | kActorComponents | `0x68` | src/mod/session2/guest_lifecycle.h:197 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D043 | D | kComponentsTransformSync | `0x1A0` | src/mod/session2/guest_lifecycle.h:198 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D044 | D | kTsMoving | `0x3DC` | src/mod/session2/guest_lifecycle.h:199 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D045 | D | kFmMoving | `0x58` | src/mod/session2/guest_lifecycle.h:200 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D046 | D | kLoginCharacterReq | `3093` | src/mod/session2/guest_lifecycle.h:215 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D047 | D | kGameLoadingStartReq | `2437` | src/mod/session2/guest_lifecycle.h:216 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D048 | D | kMoveFieldCompleteReq | `2549` | src/mod/session2/guest_lifecycle.h:217 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D049 | D | kLoadingCompleteReq | `2445` | src/mod/session2/guest_lifecycle.h:218 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D050 | D | kMoveFieldAck | `2174` | src/mod/session2/guest_lifecycle.h:219 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D051 | D | kMoveFieldCompleteAck | `2144` | src/mod/session2/guest_lifecycle.h:220 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D052 | D | kLoadingCompleteAck | `2762` | src/mod/session2/guest_lifecycle.h:221 | snapshot/remove/barrier: SAM map, user slots, actor and field-move state | needs target evidence |
| D053 | D | kLoadSlot | `0x186F570` | src/mod/session2/guest_save_host.cpp:42 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D054 | D | kLoadSlotHead | `{0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x50, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xB0, 0x01, 0x00, 0x00, 0x4D, 0x8B, 0xF9}` | src/mod/session2/guest_save_host.cpp:43 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D055 | D | kSave | `0x186E450` | src/mod/session2/guest_save_host.cpp:47 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D056 | D | kSaveHead | `{0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x78, 0xFE, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x88, 0x02, 0x00, 0x00}` | src/mod/session2/guest_save_host.cpp:48 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D057 | D | kFlush | `0x1871930` | src/mod/session2/guest_save_host.cpp:53 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D058 | D | kFlushHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x40, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC0, 0x01, 0x00, 0x00, 0x45, 0x0F, 0xB7, 0xF9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2}` | src/mod/session2/guest_save_host.cpp:54 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D059 | D | kGetOrCreate | `0x2355EC0` | src/mod/session2/guest_save_host.cpp:59 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D060 | D | kGetOrCreateHead | `{0xE9, 0xDB, 0xD8, 0xBF, 0x0C}` | src/mod/session2/guest_save_host.cpp:60 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D061 | D | kAddFile | `0x235B100` | src/mod/session2/guest_save_host.cpp:61 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D062 | D | kAddFileHead | `{0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEA, 0x4C, 0x8B, 0xF9}` | src/mod/session2/guest_save_host.cpp:62 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D063 | D | kCallSites[0].site | `0x2C17881` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D064 | D | kCallSites[1].site | `0x2C3E1CE` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D065 | D | kCallSites[2].site | `0x1042ABF3` | src/mod/session2/guest_save_host.cpp:70 | native call site to kLoadSlot | needs target evidence |
| D066 | D | kCallSites[3].site | `0xCBD86CE` | src/mod/session2/guest_save_host.cpp:70 | native call site to kSave | needs target evidence |
| D067 | D | kCallSites[4].site | `0x103BE99A` | src/mod/session2/guest_save_host.cpp:70 | native call site to kSave | needs target evidence |
| D068 | D | kCallSites[5].site | `0x28F273A` | src/mod/session2/guest_save_host.cpp:70 | native call site to kFlush | needs target evidence |
| D069 | D | kCallSites[6].site | `0x108393C8` | src/mod/session2/guest_save_host.cpp:70 | native call site to kFlush | needs target evidence |
| D070 | D | kErrBusy.site | `0x186F6DF` | src/mod/session2/guest_save_host.cpp:79 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D071 | D | kErrBusy.global | `0x6CF7CA4` | src/mod/session2/guest_save_host.cpp:79 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D072 | D | kErrInvalidSlot.site | `0x186F62C` | src/mod/session2/guest_save_host.cpp:80 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D073 | D | kErrInvalidSlot.global | `0x6CF7900` | src/mod/session2/guest_save_host.cpp:80 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D074 | D | kErrAbsentSlot.site | `0x2069615` | src/mod/session2/guest_save_host.cpp:81 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D075 | D | kErrAbsentSlot.global | `0x6CF781C` | src/mod/session2/guest_save_host.cpp:81 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D076 | D | kErrSaveFail.site | `0x186E780` | src/mod/session2/guest_save_host.cpp:82 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D077 | D | kErrSaveFail.global | `0x6CF7CA0` | src/mod/session2/guest_save_host.cpp:82 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D078 | D | kSlotMemoryOnly | `0x699` | src/mod/session2/guest_save_host.cpp:85 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D079 | D | kSlotInMemory | `0x69C` | src/mod/session2/guest_save_host.cpp:86 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D080 | D | kFileHeader | `0x98` | src/mod/session2/guest_save_host.cpp:87 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D081 | D | kActorDescriptor | `0x88` | src/mod/session2/guest_save_host.cpp:88 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D082 | D | kActorUser | `0xA0` | src/mod/session2/guest_save_host.cpp:89 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D083 | D | kActorKey | `0x60` | src/mod/session2/guest_save_host.cpp:90 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D084 | D | kMgrPendingSlot | `0x5C` | src/mod/session2/guest_save_host.cpp:91 | guest_save_install: native save/load/flush and protected slot access | needs target evidence |
| D085 | D | kHeaderSize | `0x80` | src/mod/session2/guest_save_logic.h:102 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D086 | D | kMagic | `0x45564153` | src/mod/session2/guest_save_logic.h:103 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D087 | D | kSlotNameMap | `0x08` | src/mod/session2/guest_save_logic.h:199 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D088 | D | kMapBucketCount | `0x00` | src/mod/session2/guest_save_logic.h:200 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D089 | D | kMapBuckets | `0x10` | src/mod/session2/guest_save_logic.h:201 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D090 | D | kMapEntries | `0x18` | src/mod/session2/guest_save_logic.h:202 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D091 | D | kBucketSize | `0x100` | src/mod/session2/guest_save_logic.h:203 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D092 | D | kBucketPairs | `0x08` | src/mod/session2/guest_save_logic.h:204 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D093 | D | kMaxBucketPairs | `(kBucketSize - kBucketPairs) / 8` | src/mod/session2/guest_save_logic.h:205 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D094 | D | kEntryKey | `0x08` | src/mod/session2/guest_save_logic.h:206 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D095 | D | kEntryValue | `0x10` | src/mod/session2/guest_save_logic.h:207 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D096 | D | kKeyChars | `0x00` | src/mod/session2/guest_save_logic.h:208 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D097 | D | kKeyLength | `0x08` | src/mod/session2/guest_save_logic.h:209 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D098 | D | kSlotNames | `0x688` | src/mod/session2/guest_save_logic.h:210 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D099 | D | kSlotNameCount | `0x690` | src/mod/session2/guest_save_logic.h:211 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D100 | D | kRetUserLogin | `0x2C17886` | src/mod/session2/guest_save_logic.h:462 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D101 | D | kRetLobby3093 | `0x2C3E1D3` | src/mod/session2/guest_save_logic.h:463 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D102 | D | kRetCharacter3093 | `0x1042ABF8` | src/mod/session2/guest_save_logic.h:464 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D103 | D | kRetAutosave | `0xCBD86D3` | src/mod/session2/guest_save_logic.h:483 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D104 | D | kRetManual2910 | `0x103BE99F` | src/mod/session2/guest_save_logic.h:484 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D105 | D | kRetFlushLogout | `0x28F273F` | src/mod/session2/guest_save_logic.h:501 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D106 | D | kRetFlushTimer | `0x108393CD` | src/mod/session2/guest_save_logic.h:502 | guest save native file header, slot-name map, and classified native return sites | needs target evidence |
| D107 | D | kOpSaveAutoReq | `3037` | src/mod/session2/guest_save_wire.h:67 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D108 | D | kOpSaveToFileCurrentPlayerReq | `2910` | src/mod/session2/guest_save_wire.h:68 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D109 | D | kOpSaveToFileCurrentPlayerAck | `3016` | src/mod/session2/guest_save_wire.h:69 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D110 | D | kOpCreateCharacterReq | `2711` | src/mod/session2/guest_save_wire.h:70 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D111 | D | kOpNewGameStartReq | `2178` | src/mod/session2/guest_save_wire.h:71 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D112 | D | kOpLoginCharacterReq | `3093` | src/mod/session2/guest_save_wire.h:72 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D113 | D | kOpLogoutSessionReq | `2001` | src/mod/session2/guest_save_wire.h:73 | guest save policy names native request/ack identifiers; mod control format is excluded | needs target evidence |
| D114 | D | kReporter | `0x1317D40` | src/mod/session2/report_guard.cpp:19 | required native report guard installed before save writes | needs target evidence |
| D115 | D | kReporterHead | `{0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}` | src/mod/session2/report_guard.cpp:20 | required native report guard installed before save writes | needs target evidence |
| B074 | B | kHeaderSize | `5` | src/common/troc_frame.h:30 | verify native packet framing; preserve the mod batch/control format | needs target evidence |
| B075 | B | kMaxPacketSize | `0x8000 - 0x20` | src/common/troc_frame.h:33 | verify native packet framing; preserve the mod batch/control format | needs target evidence |
| D116 | D | kOpLogoutSessionReq | `2001` | src/common/troc_frame.h:60 | verify native internal logout framing | needs target evidence |
| D117 | D | kOpLogoutSessionAck | `2002` | src/common/troc_frame.h:61 | verify native internal logout framing | needs target evidence |
| D118 | D | kOpLogoutUserAck | `2003` | src/common/troc_frame.h:62 | verify native internal logout framing | needs target evidence |
| D119 | D | kInternalHeaderSize | `3` | src/common/troc_frame.h:63 | verify native internal logout framing | needs target evidence |
| D120 | D | kLogoutAckSize | `7` | src/common/troc_frame.h:64 | verify native internal logout framing | needs target evidence |
| C135 | C | kOpSetFieldTimeReq | `2125` | src/mod/session2/session2.cpp:276 | required deny filter protects host native world time | needs target evidence |
| C136 | C | kActor_Key | `0x60` | src/mod/game/offsets.h:127 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C137 | C | kActionLayerAdvance | `0x1F63F70` | src/mod/game/offsets.h:207 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C138 | C | kActionLayerAdvanceHead | `{0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0xC5, 0xF8, 0x29, 0x78, 0xD8}` | src/mod/game/offsets.h:208 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C139 | C | kActionLayersAdvance | `0x2329700` | src/mod/game/offsets.h:218 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C140 | C | kActionLayersAdvanceHead | `{0x48, 0x8B, 0xC4, 0x41, 0x54, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x68, 0xC5, 0xF8, 0x29, 0x70, 0xC8}` | src/mod/game/offsets.h:219 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C141 | C | kControl_Layer0 | `0x88` | src/mod/game/offsets.h:221 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C142 | C | kLayer_Playback | `0x18` | src/mod/game/offsets.h:222 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C143 | C | kLayer_Chart | `0x110` | src/mod/game/offsets.h:223 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| C144 | C | kActor_OwnerUser | `0x90` | src/mod/game/offsets.h:230 | puppet_install_layer_guards: shared action-layer crash containment | needs target evidence |
| A029 | A | NativeStatUnits | `1000.0f` | src/mod/game/player_layout.h:42 | native int64 stat conversion scale (not a timeout) | needs target evidence |
| A030 | A | StatIdReadWidth | `8` | src/mod/game/player_layout.h:43 | legacy reader width; target native ID width must be confirmed separately from padding | needs target evidence |
| C145 | C | FieldLockAcquireVslot | `8` | src/mod/session2/field_share.cpp:65 | both field lock acquire slots | needs target evidence |
| C146 | C | FieldLockReleaseVslot | `0x10` | src/mod/session2/field_share.cpp:74 | both field lock release slots | needs target evidence |
| C147 | C | HostCharacterComponents | `0x68` | src/mod/session2/field_share.cpp:115 | field-share traversal to host components | needs target evidence |
| C148 | C | HostTransformSyncComponent | `0x1A0` | src/mod/session2/field_share.cpp:115 | field-share traversal to native transform sync | needs target evidence |
| C149 | C | FieldUserEntryLayout | `16 B; user +0, actors +8` | src/mod/session2/field_share.h:33 | native field user vector element, confirm both member offsets and stride | needs target evidence |
| D121 | D | NativeSlotNumberDomain | `0..3;100..108;200..207` | src/mod/session2/guest_save_logic.h:80 | verify supported native slots before choosing reserved mod slot204 | needs target evidence |
| D122 | D | NativeMemoryOnlySlots | `201;205..207` | src/mod/session2/guest_save_logic.h:83 | verify which slots cannot be persisted | needs target evidence |
| D123 | D | SaveCurrentAckLayout | `14 B; key0 slot4 ok8 flag9 result10` | src/mod/session2/guest_save_guest.cpp:777 | native 3016 result parser | needs target evidence |
| C150 | C | NativeForbiddenRequests | `2790,2038,2004,1001..1008` | src/mod/session2/session2.cpp:278 | native shutdown, pointer and connection messages must never reach remote server | needs target evidence |
| D124 | D | SaveReq2910Layout | `slot0:u32; length4:u32; UTF16+8; terminator; 10+2n bytes; n<=30` | src/mod/session2/guest_save_wire.h:383 | verify native 2910 writer and handler, including maximum native name length | needs target evidence |
| D125 | D | LoginCharacterSaveLayout | `flag0:u8; characterNo1:u64; slot9:u32` | src/mod/session2/guest_save_wire.h:72 | native 3093 payload used for selecting and holding the guest save | needs target evidence |
