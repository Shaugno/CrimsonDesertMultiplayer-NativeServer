#pragma once
// Per-build evidence catalog. R3 approves the guarded HP-only Tier A reader.
// B/C/D native paths require their own complete adapters and gates below.
// Constants alone never authorize native calls or hooks. Preserve R2 row IDs.

#include <cstdint>

namespace cdmp::game::build_3663 {

// File identity. R10 (2026-11-19): Steam updated the game to 1.0.0.3732, and
// every address, byte check and struct offset in this namespace was moved
// from 1.0.0.3663 to 1.0.0.3732 by file comparison (tools/port3732,
// docs/probe-3663/port-evidence/r10). The namespace keeps its 3663 name until
// the cleanup plan. The old 3663 identity (timestamp 0x6AF5D198, SizeOfImage
// 0x144E9000) is no longer accepted.
inline constexpr uint32_t kPeTimestamp = 0x6AFDC0CF;
inline constexpr uintptr_t kImageBase = 0x140000000;
inline constexpr uint32_t kSizeOfImage = 0x13C7E000;
inline constexpr const char* kVersionText = "1.0.0.3732";
inline constexpr uint32_t kRetiredPeTimestamp3663 = 0x6AF5D198;
inline constexpr uint32_t kRetiredSizeOfImage3663 = 0x144E9000;

// R3 dependency gates: R2 catalog plus approved R3 scope/adoption records,
// per-build routing and validation. B currently exposes the guarded native
// host loopback diagnostic; native guest join and lifecycle require C/D.
// These are compiled evidence, never ini/environment overrides.
inline constexpr bool kTierAComplete = true;
inline constexpr bool kTierBComplete = true;
// R4: separate guarded v1 route, native three-argument backend, identity,
// file sandbox and stable disconnect. Explicit opt-in; runtime L3/L4 separate.
inline constexpr bool kTierCComplete = true;
inline constexpr bool kTierDComplete = true;

inline constexpr uintptr_t kSessionSend = 0x165DD70;       // 2976 0x1437740; N0981, send; R2 B023
inline constexpr uintptr_t kSyncSessionSend = 0x165E6E0;   // 2976 0x14380B0; N1030, send; R2 B063
inline constexpr uintptr_t kQueuePush = 0x3AB8D0;         // 2976 0x35DF50; N0985, queue; R2 B027
inline constexpr uintptr_t kAllocThread = 0x4B82660;      // 2976 0x47EF9B0; N0989, alloc; R2 B031
inline constexpr uintptr_t kAllocGlobal = 0x4B82728;      // 2976 0x47EFA78; N0987, alloc; R2 B029
inline constexpr uintptr_t kNwSessionCtor = 0x165D550;    // 2976 0x1436EE0; N0983, ctor; R2 B025
inline constexpr uintptr_t kTlsIndexSlot = 0x7162090;     // 2976 0x6C88C50; N0991, tls-index; R2 B033
inline constexpr uint32_t kTls_UseGlobalHeap = 0x1FE;     // 2976 0x1FD; N0992, tls-flag; R2 B034
inline constexpr uintptr_t kNwVirtualAsyncSessionVtable = 0x5B45590;  // 2976 0x57260C0; N0961; R2 B008
inline constexpr uintptr_t kNwAsyncSessionVtable = 0x5B45610;         // 2976 0x5726140; N0962; R2 B009
inline constexpr uint32_t kSessionSize = 0xD0;            // 2976 0xD0; N0965, size; R2 B012
inline constexpr uint32_t kSessionVtableEntries = 15;     // 2976 15; N0963, vtable-count; R2 B010
inline constexpr uint32_t kSessionVfSend = 9;             // 2976 9; N0964, async-table; R2 B011
inline constexpr uint32_t kSession_OutQueue = 0xC8;       // 2976 0xC8; N0973, out-queue; R2 B020
inline constexpr uint32_t kRq_Frame = 0x30;               // 2976 0x30; N1018, frame; R2 B051
inline constexpr uintptr_t kServerOnRecvPacket = 0x2A111F0; // 2976 0x2773C20; N0979, server-recv; R2 B021
inline constexpr uintptr_t kSaveMgrRefreshSlots = 0x1A19F90; // 2976 0x1870C20; N1056, save-refresh; R2 D018

// Passive external probe only. These RVA candidates are not production
// adoptions; their standard-correlator checks remain in progress. No ASI
// installer/reader obtains a table from this namespace.
namespace diagnostic_candidates {
inline constexpr uintptr_t kChunkOrigin = 0x7237470; // 2976 0x6D56500; R2 A001
inline constexpr uintptr_t kStaticPlayerPos = 0x725A040; // 2976 0x6D77858; R2 A002
inline constexpr uintptr_t kVtCharacterControl = 0x59C6748; // 2976 0x55B5460; R2 A005
inline constexpr uintptr_t kVtPhysicsObject = 0x614F3A8; // 2976 0x5D00F88; R2 A007
inline constexpr uintptr_t kHpDefinitionsSlot = 0x724B6E8; // 2976 0x6D69528; R2 A032
inline constexpr uintptr_t kHpDefinitionsAltSlot = 0x724B6F0; // 2976 0x6D69530; R2 A033
inline constexpr uintptr_t kWorldSystemSlot = 0x724B350; // 2976 0x6D69190; R2 A031
inline constexpr uintptr_t kVtClientActorManager = 0x59C7A08; // 2976 0x55B62C8; R2 A003
inline constexpr uintptr_t kVtControlledActor = 0x59961D0; // 2976 0x5585C40; R2 A004
inline constexpr uintptr_t kVtStatusComponent = 0x599DC40; // 2976 0x558D868; R2 A006
inline constexpr uint32_t kWorldSystem_ActorManager = 0x30; // 2976 0x30; R2 A008
inline constexpr uint32_t kActorManager_ControlledActor = 0x50; // 2976 0x50; R2 A009
} // namespace diagnostic_candidates

// R2 reviewed layouts BEGIN
// Layout proof catalog: docs/probe-3663/r2-reviewed.json. Permissions are gated above.
namespace player_layout {
inline constexpr uint32_t kActor_ComponentTable = 0x68; // 2976 0x68; R2 A010
inline constexpr uint32_t kComponents_CharacterControl = 0x40; // 2976 0x40; R2 A012
inline constexpr uint32_t kControl_Inner = 0x140; // 2976 0x140; R2 A013
inline constexpr uint32_t kControlInner_Physics = 0x2D0; // 2976 0x2C0; R2 A014
inline constexpr uint32_t kPhysics_Position = 0x90; // 2976 0x90; R2 A018
inline constexpr uint32_t kWorldSystem_ActorManager = 0x30; // 2976 0x30; R2 A008
inline constexpr uint32_t kActorManager_ControlledActor = 0x50; // 2976 0x50; R2 A009
inline constexpr uint32_t kPhysics_AxisX = 0x60; // 2976 0x60; R2 A015
inline constexpr uint32_t kPhysics_AxisY = 0x70; // 2976 0x70; R2 A016
inline constexpr uint32_t kPhysics_AxisZ = 0x80; // 2976 0x80; R2 A017
inline constexpr uint32_t kComponents_Status = 0x20; // 2976 0x20; R2 A011
inline constexpr uint32_t kStatus_StatContainer = 0x18; // 2976 0x18; R2 A019
inline constexpr uint32_t kStatContainer_Entries = 0x58; // 2976 0x58; R2 A020
inline constexpr uint32_t kStatContainer_Count = 0x60; // 2976 0x60; R2 A021
inline constexpr uint32_t kStatEntry_Size = 0x90; // 2976 0x90; R2 A022
inline constexpr uint32_t kStatEntry_Id = 0x00; // 2976 0x00; R2 A023
inline constexpr uint32_t kStatEntry_Current = 0x08; // 2976 0x08; R2 A024
inline constexpr uint32_t kStatEntry_Max = 0x18; // 2976 0x18; R2 A025
inline constexpr float kNativeStatUnits = 1000.0f; // 2976 1000.0f; R2 A029
inline constexpr uint32_t kStatIdReadWidth = 2; // 2976 8; R2 A030
inline constexpr uint32_t kHpDefinitionsRoot = 0x00; // 2976 0x00; R2 A034
inline constexpr uint32_t kHpDefinitionsTables = 0x08; // 2976 0x08; R2 A035
inline constexpr uint32_t kHpDefinitionsObject = 0x10; // 2976 0x10; R2 A036
inline constexpr uint32_t kHpDefinitionId = 0x78; // 2976 0xA0; R2 A037
} // namespace player_layout
namespace tunnel_layout {
inline constexpr uint32_t kServerNet_Threads = 0xF8; // 2976 0xF8; R2 B035
inline constexpr uint32_t kServerNet_ThreadCount = 0x100; // 2976 0x100; R2 B036
inline constexpr uint32_t kServerThreadEntry = 0x10; // 2976 0x10; R2 B037
inline constexpr uint32_t kArray_Count = 0x08; // 2976 0x08; R2 B038
inline constexpr uint32_t kArray_Capacity = 0x0C; // 2976 0x0C; R2 B039
inline constexpr uint32_t kServerNet_Session = 0x18; // 2976 0x18; R2 B003
inline constexpr uint32_t kSession_Name = 0x18; // 2976 0x18; R2 B013
inline constexpr uint32_t kSession_B0 = 0xB0; // 2976 0xB0; R2 B017
inline constexpr uint32_t kSession_B8 = 0xB8; // 2976 0xB8; R2 B018
inline constexpr uint32_t kSession_C0 = 0xC0; // 2976 0xC0; R2 B019
inline constexpr uint32_t kSession_IdPtr = 0x88; // 2976 0x88; R2 B014
inline constexpr uint32_t kSession_User = 0x90; // 2976 0x90; R2 B015
inline constexpr uint32_t kSession_Crypto = 0x98; // 2976 0x98; R2 B016
inline constexpr uint32_t kPbSize = 0x8030; // 2976 0x8030; R2 B040
inline constexpr uint32_t kPb_FramePtr = 0x8000; // 2976 0x8000; R2 B041
inline constexpr uint32_t kPb_Len1 = 0x8008; // 2976 0x8008; R2 B042
inline constexpr uint32_t kPb_Len2 = 0x800A; // 2976 0x800A; R2 B043
inline constexpr uint32_t kPb_Cap = 0x800C; // 2976 0x800C; R2 B044
inline constexpr uint32_t kPb_IsRequest = 0x800E; // 2976 0x800E; R2 B045
inline constexpr uint32_t kPb_Base = 0x8028; // 2976 0x8028; R2 B048
inline constexpr uint32_t kFrameOverhead = 0x1F; // 2976 0x1F; R2 B049
inline constexpr uint32_t kFrameHeader = 3; // 2976 3; R2 B050
inline constexpr uint64_t kNoEpoch = 0x7FFFFFFFFFFFFFFF; // 2976 0x7FFFFFFFFFFFFFFFull; R2 B072
inline constexpr uint32_t kHeaderSize = 5; // 2976 5; R2 B074
inline constexpr uint32_t kNativeCryptoEnabledOffset = 0x24; // 2976 0x24; R2 B076
inline constexpr uint32_t kNativePacketEpochOffset = 0x8020; // 2976 0x8020; R2 B077
inline constexpr uint32_t kSyncSession_Epoch = 0xB0; // 2976 0xB0; R2 B062
inline constexpr uint32_t kClientNet_Slot4 = 0x58; // 2976 0x58; R2 B061
inline constexpr uint32_t kTroc_Kind = 0x08; // 2976 0x08; R2 B054
inline constexpr uint32_t kTroc_Opcode = 0x0C; // 2976 0x0C; R2 B055
inline constexpr uint32_t kTroc_Channel = 0x14; // 2976 0x14; R2 B056
inline constexpr uint32_t kTroc_Flags = 0x16; // 2976 0x16; R2 B057
inline constexpr uint32_t kTrocKindServer = 4; // 2976 4; R2 B058
inline constexpr uint32_t kClientNet_InQueue = 0x18; // 2976 0x18; R2 B007
inline constexpr uint32_t kClientNet_Session = 0x10; // 2976 0x10; R2 B059
inline constexpr uint32_t kClientNet_FreeC2S = 0x28; // 2976 0x28; R2 B060
inline constexpr uint32_t kNetLoopPumpPatch = 7; // 2976 7; R2 B067
inline constexpr uint32_t kServerNet_InQueue = 0x20; // 2976 0x20; R2 B004
inline constexpr uint32_t kServerNet_FreeC2S = 0x78; // 2976 0x78; R2 B005
inline constexpr uint32_t kPb_Tag = 0x8010; // 2976 0x8010; R2 B046
inline constexpr uint32_t kPb_Magic = 0x8018; // 2976 0x8018; R2 B047
inline constexpr uint32_t kServerNet_FreeS2C = 0x80; // 2976 0x80; R2 B006
inline constexpr uint8_t kServerOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54}; // 2976 signature: src/mod/session2/offsets_s2.h:52; R2 B022
inline constexpr uint8_t kSessionSendHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30}; // 2976 signature: src/mod/session2/offsets_s2.h:55; R2 B024
inline constexpr uint8_t kNwSessionCtorHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}; // 2976 signature: src/mod/session2/offsets_s2.h:58; R2 B026
inline constexpr uint8_t kQueuePushHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}; // 2976 signature: src/mod/session2/offsets_s2.h:61; R2 B028
inline constexpr uint8_t kAllocGlobalHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA}; // 2976 signature: src/mod/session2/offsets_s2.h:72; R2 B030
inline constexpr uint8_t kAllocThreadBytes[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0x0E, 0xFA, 0x5D, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF}; // 2976 signature: src/mod/session2/offsets_s2.h:76; R2 B032
inline constexpr uint8_t kSyncSessionSendHead[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA}; // 2976 signature: src/mod/session2/offsets_s2.h:141; R2 B064
inline constexpr uint8_t kNetLoopPumpHead[] = {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7}; // 2976 signature: src/mod/session2/offsets_s2.h:144; R2 B066
inline constexpr uint8_t kNetLoopPumpAob[] = {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74}; // 2976 signature: src/mod/session2/offsets_s2.h:154; R2 B068
inline constexpr uint8_t kClientOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24}; // 2976 signature: src/mod/session2/offsets_s2.h:158; R2 B070
inline constexpr uint32_t kOpConnectCryptogramToServerReq = 2401; // 2976 2319; R2 B071
} // namespace tunnel_layout
namespace field_layout {
inline constexpr uint32_t kFinder_Actor = 0x10; // 2976 0x10; R2 C017
inline constexpr uint32_t kServerActor_Controller = 0x90; // 2976 0x90; R2 C007
inline constexpr uint32_t kUser_Session = 0x1F8; // 2976 0x1F8; R2 C006
inline constexpr uint32_t kControl_Layer0 = 0x88; // 2976 0x88; R2 C141
inline constexpr uint32_t kLayer_Playback = 0x18; // 2976 0x18; R2 C142
inline constexpr uint32_t kLayer_Chart = 0x118; // 2976 0x110; R2 C143
inline constexpr uint32_t kRefs = 0x10; // 2976 0x10; R2 C018
inline constexpr uint32_t kLock1 = 0x18; // 2976 0x18; R2 C019
inline constexpr uint32_t kLock2Ptr = 0x08; // 2976 0x08; R2 C020
inline constexpr uint32_t kOpen = 0x4A; // 2976 0x4A; R2 C021
inline constexpr uint32_t kOwner = 0x180; // 2976 0x180; R2 C022
inline constexpr uint32_t kUsers = 0x2A8; // 2976 0x2A0; R2 C023
inline constexpr uint32_t kUserCount = 0x2B0; // 2976 0x2A8; R2 C024
inline constexpr uint32_t kUserActors = 0x2B8; // 2976 0x2B0; R2 C025
inline constexpr uint32_t kLevelEntry = 0x2D8; // 2976 0x2D0; R2 C027
inline constexpr uint32_t kTsFieldMover = 0x508; // 2976 0x4F0; R2 C028
inline constexpr uint32_t kFmFieldKey = 0x08; // 2976 0x08; R2 C029
inline constexpr uint32_t kFmField = 0x10; // 2976 0x10; R2 C030
inline constexpr uint32_t kKey = 0x2D0; // 2976 0x2C8; R2 C026
inline constexpr uint32_t kFmBusy = 0x4A; // 2976 0x5A; R2 C031
inline constexpr uint32_t kOutField = 0x08; // 2976 0x08; R2 C032
inline constexpr uint32_t kOutValid = 0x10; // 2976 0x10; R2 C033
inline constexpr uint32_t kOutHolder = 0x18; // 2976 0x18; R2 C034
inline constexpr uint32_t kOutNew = 0x20; // 2976 0x20; R2 C035
inline constexpr uint32_t kOutInfoKey = 0x24; // 2976 0x24; R2 C036
inline constexpr uint32_t kMgrBucketCount = 0x48; // 2976 0x48; R2 C037
inline constexpr uint32_t kMgrEntries = 0x4C; // 2976 0x4C; R2 C038
inline constexpr uint32_t kMgrBuckets = 0x58; // 2976 0x58; R2 C039
inline constexpr uint32_t kMgrHolders = 0x60; // 2976 0x60; R2 C040
inline constexpr uint32_t kUserKey = 0x60; // 2976 0x60; R2 C050
inline constexpr uint32_t kActorUser = 0xA0; // 2976 0xA0; R2 C051
inline constexpr uint32_t kFieldLockAcquireVslot = 8; // 2976 8; R2 C145
inline constexpr uint32_t kFieldLockReleaseVslot = 0x10; // 2976 0x10; R2 C146
inline constexpr uint32_t kHostCharacterComponents = 0x68; // 2976 0x68; R2 C147
inline constexpr uint32_t kHostTransformSyncComponent = 0x1A0; // 2976 0x1A0; R2 C148
inline constexpr uint32_t kFieldUserEntrySize = 16; // 2976 16; R2 C149
inline constexpr uint32_t kFieldUserEntryUser = 0; // 2976 0; R2 C149
inline constexpr uint32_t kFieldUserEntryActors = 8; // 2976 8; R2 C149
inline constexpr uint32_t kOpNak = 1013; // 2976 1013; R2 C128
inline constexpr uint32_t kUserMainCharacter = 0xD0; // 2976 0xD0; R2 C049
inline constexpr uint32_t kSam_ActorCount = 0x9C; // 2976 0x9C; R2 C003
inline constexpr uint64_t kNative = 0x7FFFFFFFFFFFFFFF; // 2976 0x7FFFFFFFFFFFFFFF; R2 C052
inline constexpr uint32_t kLobbyOwner = 0x08; // 2976 0x08; R2 C053
inline constexpr uint32_t kLobbyList = 0x18; // 2976 0x18; R2 C054
inline constexpr uint32_t kListCount = 0x08; // 2976 0x08; R2 C055
inline constexpr uint32_t kEntryStride = 0xD0; // 2976 0xD0; R2 C056
inline constexpr uint32_t kEntryNumber = 0x00; // 2976 0x00; R2 C057
inline constexpr uint32_t kUserComponents = 0x68; // 2976 0x68; R2 C058
inline constexpr uint32_t kTableLobby = 0x10; // 2976 0x10; R2 C059
inline constexpr uint32_t kTableStatus = 0x20; // 2976 0x20; R2 C060
inline constexpr uint32_t kStatusActor = 0x08; // 2976 0x08; R2 C061
inline constexpr uint32_t kStatusNumber = 0x90; // 2976 0x70; R2 C062
inline constexpr uint32_t kLockObject = 0x08; // 2976 0x08; R2 C065
inline constexpr uint32_t kActorComponents = 0x68; // 2976 0x68; R2 C091
// kTableStatus: same native contract; 2976 0x20; R2 C092
inline constexpr uint32_t kStatusCharacterNo = 0x90; // 2976 0x70; R2 C093
inline constexpr uint32_t kComponentOwner = 0x08; // 2976 0x08; R2 C094
// kActorUser: same native contract; 2976 0xA0; R2 C095
inline constexpr uint32_t kKey_C096 = 0x60; // 2976 0x60; R2 C096
inline constexpr uint32_t kLoginMgrActors = 0xB8; // 2976 0xB8; R2 C087
inline constexpr uint32_t kLoginMgrCount = 0xC0; // 2976 0xC0; R2 C088
inline constexpr uint32_t kForbiddenReserveShutdownOp = 2447; // 2976 2447 (paired schema); R2 C150
inline constexpr uint32_t kForbiddenShutdownTimerOp = 2042; // 2976 2042 (paired schema); R2 C150
inline constexpr uint32_t kForbiddenPointerRequestOp = 2004; // 2976 2004 (paired schema); R2 C150
inline constexpr uint32_t kUser_Key = 0x60; // 2976 0x60; R2 C005
inline constexpr uint32_t kServerActor_Key = 0x60; // 2976 0x60; R2 C008
inline constexpr uint32_t kActorKey = 0x60; // 2976 0x60; R2 C063
// kActorUser: same native contract; 2976 0xA0; R2 C064
// kActorUser: same native contract; 2976 0xA0; R2 C105
// kUserKey: same native contract; 2976 0x60; R2 C106
inline constexpr uint32_t kActor_Key = 0x60; // 2976 0x60; R2 C136
inline constexpr uint32_t kSam_FreeKeys = 0xD0; // 2976 0xD0; R2 C002
inline constexpr uint32_t kTsActor = 0x08; // 2976 0x08; R2 C104
inline constexpr uint32_t kTsFieldTime = 0x450; // 2976 0x450; R2 C107
inline constexpr uint32_t kFieldTimeSize = 32; // 2976 32; R2 C108
inline constexpr uint8_t kFindByNoScanBytes[] = {0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x14, 0xC1}; // 2976 signature: src/mod/session2/identity_fix.cpp:238; R2 C084
inline constexpr uint8_t kFindByNoReadBytes[] = {0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20}; // 2976 signature: src/mod/session2/identity_fix.cpp:241; R2 C086
inline constexpr uint8_t kLoadingHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x60, 0x4D, 0x8B, 0xE9, 0x49, 0x8B, 0xF0, 0x4C, 0x8B, 0xF2, 0x48, 0x8B, 0xD9}; // 2976 signature: src/mod/session2/clock_gate.cpp:30; R2 C117
inline constexpr uint8_t kLoadingFlagCmpBytes[] = {0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00}; // 2976 signature: src/mod/session2/clock_gate.cpp:35; R2 C119
inline constexpr uint8_t kAcquireFieldHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70}; // 2976 signature: src/mod/session2/field_share.cpp:21; R2 C042
inline constexpr uint8_t kEnterFieldHead[] = {0x48, 0x89, 0x5C, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x70, 0xFF}; // 2976 signature: src/mod/session2/field_share.cpp:25; R2 C044
inline constexpr uint8_t kTryAddRefHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9}; // 2976 signature: src/mod/session2/field_share.cpp:29; R2 C046
inline constexpr uint8_t kVecPushHead[] = {0x40, 0x53, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, 0x4C, 0x8B, 0xFA, 0x8B, 0x49, 0x08, 0x8B, 0x43, 0x0C}; // 2976 signature: src/mod/session2/field_share.cpp:32; R2 C048
inline constexpr uint8_t kLobbyLoadHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x54, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0xF0, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x10, 0x03}; // 2976 signature: src/mod/session2/identity_fix.cpp:216; R2 C068
inline constexpr uint8_t kLoadHead[] = {0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x68, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00, 0x49, 0x8B, 0xD9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xF1}; // 2976 signature: src/mod/session2/identity_probe.cpp:32; R2 C098
inline constexpr uint8_t kGetterHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0xFA, 0x48, 0x8D, 0x54, 0x24, 0x70, 0x48, 0x8B, 0xD9, 0xFF, 0x50, 0x60}; // 2976 signature: src/mod/session2/clock_gate.cpp:37; R2 C121
inline constexpr uint8_t kActionLayerAdvanceHead[] = {0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0xC5, 0xF8, 0x29, 0x78, 0xD8}; // 2976 signature: src/mod/game/offsets.h:208; R2 C138
inline constexpr uint8_t kActionLayersAdvanceHead[] = {0x48, 0x8B, 0xC4, 0x41, 0x55, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x68, 0xC5, 0xF8, 0x29, 0x70, 0xC8}; // 2976 signature: src/mod/game/offsets.h:219; R2 C140
inline constexpr uint8_t kLobbyConstantBytes[] = {0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x48, 0x89, 0x45, 0xC0, 0xC5}; // 2976 signature: src/mod/session2/identity_fix.cpp:221; R2 C071
inline constexpr uint8_t kLobbyAddCallBytes[] = {0xE8, 0x1B, 0x3F, 0x4F, 0xFF}; // 2976 signature: src/mod/session2/identity_fix.cpp:224; R2 C073
inline constexpr uint32_t kOpLoginUser = 2554; // 2976 2490; R2 C089
inline constexpr uint32_t kOpLoginUserToFieldServerReq = 2554; // 2976 2490; R2 C124
// kOpLoginUserToFieldServerReq: same native contract; 2976 2490; R2 C134
inline constexpr uint32_t kOpLoginCharacter = 3081; // 2976 3093; R2 C090
inline constexpr uint32_t kOpLoginCharacterToFieldServerReq = 3081; // 2976 3093; R2 C123
// kOpLoginCharacterToFieldServerReq: same native contract; 2976 3093; R2 C133
inline constexpr uint32_t kOpMainPlayerDataStartAck = 2844; // 2976 2756; R2 C125
inline constexpr uint32_t kOpMainUserDataStartAck = 2420; // 2976 2646; R2 C126
inline constexpr uint32_t kOpMoveFieldAck = 2364; // 2976 2174; R2 C127
inline constexpr uint32_t kOpScopeSenderAck = 2046; // 2976 2042; R2 C129
inline constexpr uint32_t kOpActorAppear = 2005; // 2976 2005; R2 C130
inline constexpr uint32_t kOpDataAck = 2896; // 2976 2869; R2 C131
inline constexpr uint32_t kOpSetFieldTimeReq = 2150; // 2976 2125; R2 C135
inline constexpr uint32_t kOp2679 = 3116; // 2976 2679; R2 C132
} // namespace field_layout
namespace lifecycle_layout {
inline constexpr uint32_t kServerActorLookup_SaveManager = 0x48; // 2976 0x48; R2 D003
inline constexpr uint32_t kSaveMgr_Lock = 0x00; // 2976 0x00; R2 D004
inline constexpr uint32_t kSaveMgr_Slots = 0x40; // 2976 0x40; R2 D006
inline constexpr uint32_t kSaveMgr_Count = 0x48; // 2976 0x48; R2 D007
inline constexpr uint32_t kSaveMgr_Storage = 0x50; // 2976 0x50; R2 D009
inline constexpr uint32_t kSaveMgr_Busy = 0x60; // 2976 0x60; R2 D010
inline constexpr uint32_t kSaveMgr_EntrySize = 0x78; // 2976 0x68; R2 D011
inline constexpr uint32_t kSaveStorage_FolderCount = 0x40; // 2976 0x40; R2 D012
inline constexpr uint32_t kNativeSlotBaseMax = 3; // 2976 3 (paired schema); R2 D121
inline constexpr uint32_t kNativeSlotManualBegin = 100; // 2976 100 (paired schema); R2 D121
inline constexpr uint32_t kNativeSlotManualCount = 9; // 2976 9 (paired schema); R2 D121
inline constexpr uint32_t kNativeSlotSpecialBegin = 200; // 2976 200 (paired schema); R2 D121
inline constexpr uint32_t kNativeSlotSpecialCount = 8; // 2976 8 (paired schema); R2 D121
inline constexpr uint32_t kActorDescriptor = 0x88; // 2976 0x88; R2 D081
inline constexpr uint32_t kActorUser = 0xA0; // 2976 0xA0; R2 D082
inline constexpr uint32_t kActorKey = 0x60; // 2976 0x60; R2 D083
inline constexpr uint32_t kUserMainCharacter = 0xD0; // 2976 0xD0; R2 D035
// kActorKey: same native contract; 2976 0x60; R2 D034
inline constexpr uint32_t kSamMapBuckets = 0x98; // 2976 0x98; R2 D025
inline constexpr uint32_t kSamMapSize = 0x9C; // 2976 0x9C; R2 D026
inline constexpr uint32_t kSamMapBucketBase = 0xA8; // 2976 0xA8; R2 D027
inline constexpr uint32_t kSamMapHolders = 0xB0; // 2976 0xB0; R2 D028
inline constexpr uint32_t kMgrPendingSlot = 0x5C; // 2976 0x5C; R2 D084
inline constexpr uint32_t kSaveMgr_Capacity = 0x4C; // 2976 0x4C; R2 D008
inline constexpr uint32_t kEntrySize = 0x78; // 2976 0x68; R2 D127
inline constexpr uint32_t kTitleSlotNumberOffset = 0; // 2976 0; R2 D128
inline constexpr uint32_t kTitleSlotTimeOffset = 8; // 2976 8; R2 D129
inline constexpr uint32_t kSlotMemoryOnly = 0x699; // 2976 0x699; R2 D078
inline constexpr uint32_t kSlotInMemory = 0x69C; // 2976 0x69C; R2 D079
inline constexpr uint32_t kFileHeader = 0x98; // 2976 0x98; R2 D080
inline constexpr uint32_t kHeaderSize = 0x80; // 2976 0x80; R2 D085
inline constexpr uint32_t kMagic = 0x45564153; // 2976 0x45564153; R2 D086
inline constexpr uint32_t kSlotNameMap = 0x08; // 2976 0x08; R2 D087
inline constexpr uint32_t kMapBucketCount = 0x00; // 2976 0x00; R2 D088
inline constexpr uint32_t kMapBuckets = 0x10; // 2976 0x10; R2 D089
inline constexpr uint32_t kMapEntries = 0x18; // 2976 0x18; R2 D090
inline constexpr uint32_t kBucketSize = 0x100; // 2976 0x100; R2 D091
inline constexpr uint32_t kBucketPairs = 0x08; // 2976 0x08; R2 D092
inline constexpr uint32_t kMaxBucketPairs = 31; // 2976 (kBucketSize - kBucketPairs) / 8; R2 D093
inline constexpr uint32_t kEntryKey = 0x08; // 2976 0x08; R2 D094
inline constexpr uint32_t kEntryValue = 0x10; // 2976 0x10; R2 D095
inline constexpr uint32_t kKeyChars = 0x00; // 2976 0x00; R2 D096
inline constexpr uint32_t kKeyLength = 0x08; // 2976 0x08; R2 D097
inline constexpr uint32_t kSlotNames = 0x688; // 2976 0x688; R2 D098
inline constexpr uint32_t kSlotNameCount = 0x690; // 2976 0x690; R2 D099
inline constexpr uint32_t kNativeMemoryOnlyExact = 201; // 2976 201 (paired schema); R2 D122
inline constexpr uint32_t kNativeMemoryOnlyBegin = 205; // 2976 205 (paired schema); R2 D122
inline constexpr uint32_t kNativeMemoryOnlyCount = 3; // 2976 3 (paired schema); R2 D122
inline constexpr uint32_t kSaveHeaderMagicOffset = 0; // 2976 0 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderVersionOffset = 4; // 2976 4 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderSizeOffset = 6; // 2976 6 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderFlagsOffset = 12; // 2976 12 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderRawSizeOffset = 18; // 2976 18 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderDataSizeOffset = 22; // 2976 22 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderVersionMin = 1; // 2976 1 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderVersionMax = 2; // 2976 2 (paired schema); R2 D126
inline constexpr uint32_t kSaveHeaderAllowedFlags = 2; // 2976 2 (paired schema); R2 D126
inline constexpr uint32_t kWorldSystem_SaveManager = 0x88; // 2976 0x88; R2 D001
inline constexpr uint32_t kSamBucketBytes = 0x100; // 2976 0x100; R2 D029
inline constexpr uint32_t kSamKeyIndexMask = 0xFFFFF; // 2976 0xFFFFF; R2 D031
inline constexpr uint32_t kSamUserList = 0xB8; // 2976 0xB8; R2 D032
inline constexpr uint32_t kSamUserCount = 0xC0; // 2976 0xC0; R2 D033
inline constexpr uint32_t kActorComponents = 0x68; // 2976 0x68; R2 D042
inline constexpr uint32_t kComponentsTransformSync = 0x1A0; // 2976 0x1A0; R2 D043
inline constexpr uint32_t kUserComponents = 0x68; // 2976 0x68; R2 D036
inline constexpr uint32_t kComponentsSlots = 0x18; // 2976 0x18; R2 D037
inline constexpr uint32_t kSlotsArray = 0x18; // 2976 0x18; R2 D038
inline constexpr uint32_t kSlotsCount = 0x20; // 2976 0x20; R2 D039
inline constexpr uint32_t kSlotStride = 0x10; // 2976 0x10; R2 D040
inline constexpr uint32_t kActorState = 0x5E; // 2976 0x5E; R2 D041
inline constexpr uint32_t kSaveMgr_LockObj = 0x08; // 2976 0x08; R2 D005
inline constexpr uint8_t kWindowsLockVf1Head[] = {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18}; // 2976 signature: src/mod/session2/offsets_s2.h:182; R2 D016
inline constexpr uint8_t kWindowsLockVf2Head[] = {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20}; // 2976 signature: src/mod/session2/offsets_s2.h:183; R2 D017
inline constexpr uint8_t kSaveMgrRefreshSlotsHead[] = {0xE9, 0xDB, 0x0D, 0xC9, 0x09}; // 2976 signature: src/mod/session2/offsets_s2.h:186; R2 D019
inline constexpr uint8_t kLoadSlotHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x50, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xB0, 0x01, 0x00, 0x00, 0x4D, 0x8B, 0xF1}; // 2976 signature: src/mod/session2/guest_save_host.cpp:43; R2 D054
inline constexpr uint8_t kSaveHead[] = {0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x38, 0xFC, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC8, 0x04, 0x00, 0x00}; // 2976 signature: src/mod/session2/guest_save_host.cpp:48; R2 D056
inline constexpr uint8_t kFlushHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x40, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC0, 0x01, 0x00, 0x00, 0x45, 0x0F, 0xB7, 0xF9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2}; // 2976 signature: src/mod/session2/guest_save_host.cpp:54; R2 D058
inline constexpr uint8_t kGetOrCreateHead[] = {0xE9, 0x6B, 0xDB, 0xC3, 0x0A}; // 2976 signature: src/mod/session2/guest_save_host.cpp:60; R2 D060
inline constexpr uint8_t kAddFileHead[] = {0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEA, 0x4C, 0x8B, 0xF9}; // 2976 signature: src/mod/session2/guest_save_host.cpp:62; R2 D062
inline constexpr uint32_t kLoginCharacterReq = 3081; // 2976 3093; R2 D046
inline constexpr uint32_t kOpLoginCharacterReq = 3081; // 2976 3093; R2 D112
inline constexpr uint32_t kMoveFieldAck = 2364; // 2976 2174; R2 D050
inline constexpr uint32_t kGameLoadingStartReq = 2350; // 2976 2437; R2 D047
inline constexpr uint32_t kMoveFieldCompleteReq = 2723; // 2976 2549; R2 D048
inline constexpr uint32_t kLoadingCompleteReq = 2789; // 2976 2445; R2 D049
inline constexpr uint32_t kMoveFieldCompleteAck = 2268; // 2976 2144; R2 D051
inline constexpr uint32_t kLoadingCompleteAck = 2891; // 2976 2762; R2 D052
inline constexpr uint32_t kOpReserveShutDownServerReq = 2447; // 2976 2790; R2 D024
inline constexpr uint32_t kOpSaveAutoReq = 2234; // 2976 3037; R2 D107
inline constexpr uint32_t kOpSaveToFileCurrentPlayerReq = 3158; // 2976 2910; R2 D108
inline constexpr uint32_t kOpSaveToFileCurrentPlayerAck = 2813; // 2976 3016; R2 D109
inline constexpr uint32_t kOpCreateCharacterReq = 2533; // 2976 2711; R2 D110
inline constexpr uint32_t kOpNewGameStartReq = 2688; // 2976 2178; R2 D111
inline constexpr uint32_t kOpLogoutSessionReq = 2001; // 2976 2001; R2 D113
// kOpLogoutSessionReq: same native contract; 2976 2001; R2 D116
inline constexpr uint32_t kOpLogoutSessionAck = 2002; // 2976 2002; R2 D117
inline constexpr uint32_t kOpLogoutUserAck = 2003; // 2976 2003; R2 D118
inline constexpr uint32_t kSaveCurrentAckSize = 14; // 2976 14 (paired schema); R2 D123
inline constexpr uint32_t kSaveCurrentAckKey = 0; // 2976 0 (paired schema); R2 D123
inline constexpr uint32_t kSaveCurrentAckSlot = 4; // 2976 4 (paired schema); R2 D123
inline constexpr uint32_t kSaveCurrentAckOk = 8; // 2976 8 (paired schema); R2 D123
inline constexpr uint32_t kSaveCurrentAckFlag = 9; // 2976 9 (paired schema); R2 D123
inline constexpr uint32_t kSaveCurrentAckResult = 10; // 2976 10 (paired schema); R2 D123
inline constexpr uint32_t kSaveRequestSlot = 0; // 2976 0 (paired schema); R2 D124
inline constexpr uint32_t kSaveRequestNameLength = 4; // 2976 4 (paired schema); R2 D124
inline constexpr uint32_t kSaveRequestName = 8; // 2976 8 (paired schema); R2 D124
inline constexpr uint32_t kSaveRequestFixedSize = 10; // 2976 10 (paired schema); R2 D124
inline constexpr uint32_t kSaveRequestMaxName = 30; // 2976 30 (paired schema); R2 D124
inline constexpr uint32_t kLoginCharacterFlag = 0; // 2976 0 (paired schema); R2 D125
inline constexpr uint32_t kLoginCharacterNumber = 1; // 2976 1 (paired schema); R2 D125
inline constexpr uint32_t kLoginCharacterSlot = 9; // 2976 9 (paired schema); R2 D125
inline constexpr uint32_t kInternalHeaderSize = 3; // 2976 3; R2 D119
inline constexpr uint32_t kLogoutAckSize = 7; // 2976 7; R2 D120
} // namespace lifecycle_layout
namespace core_rvas {
// Standard-correlator adoptions only; these constants grant no startup permission.
inline constexpr uintptr_t A001_kChunkOrigin = 0x7237470; // 2976 0x6D56500; R2 A001, R2-MATCHES.json
inline constexpr uintptr_t A002_kStaticPlayerPos = 0x725a040; // 2976 0x6D77858; R2 A002, R2-MATCHES.json
inline constexpr uintptr_t A003_kVtClientActorManager = 0x59c7a08; // 2976 0x55B62C8; R2 A003, R2-MATCHES.json
inline constexpr uintptr_t A004_kVtControlledActor = 0x59961d0; // 2976 0x5585C40; R2 A004, R2-MATCHES.json
inline constexpr uintptr_t A005_kVtCharacterControl = 0x59c6748; // 2976 0x55B5460; R2 A005, R2-MATCHES.json
inline constexpr uintptr_t A006_kVtStatusComponent = 0x599dc40; // 2976 0x558D868; R2 A006, R2-MATCHES.json
inline constexpr uintptr_t A007_kVtPhysicsObject = 0x614f3a8; // 2976 0x5D00F88; R2 A007, R2-MATCHES.json
inline constexpr uintptr_t B001_kServerNetSlot = 0x724b848; // 2976 0x6D69680; R2 B001, R2-MATCHES.json
inline constexpr uintptr_t B002_kClientNetSlot = 0x724b330; // 2976 0x6D69170; R2 B002, R2-MATCHES.json
inline constexpr uintptr_t B008_kNwVirtualAsyncSessionVtable = 0x5B45590; // 2976 0x57260C0; R2 B008, R2-MATCHES.json
inline constexpr uintptr_t B009_kNwAsyncSessionVtable = 0x5B45610; // 2976 0x5726140; R2 B009, R2-MATCHES.json
inline constexpr uintptr_t B021_kServerOnRecvPacket = 0x2A111F0; // 2976 0x2773C20; R2 B021, R2-MATCHES.json
inline constexpr uintptr_t B023_kSessionSend = 0x165DD70; // 2976 0x1437740; R2 B023, R2-MATCHES.json
inline constexpr uintptr_t B025_kNwSessionCtor = 0x165D550; // 2976 0x1436EE0; R2 B025, R2-MATCHES.json
inline constexpr uintptr_t B027_kQueuePush = 0x3AB8D0; // 2976 0x35DF50; R2 B027, R2-MATCHES.json
inline constexpr uintptr_t B029_kAllocGlobal = 0x4B82728; // 2976 0x47EFA78; R2 B029, R2-MATCHES.json
inline constexpr uintptr_t B031_kAllocThread = 0x4B82660; // 2976 0x47EF9B0; R2 B031, R2-MATCHES.json
inline constexpr uintptr_t B033_kTlsIndexSlot = 0x7162090; // 2976 0x6C88C50; R2 B033, R2-MATCHES.json
inline constexpr uintptr_t B063_kSyncSessionSend = 0x165E6E0; // 2976 0x14380B0; R2 B063, R2-MATCHES.json
inline constexpr uintptr_t B069_kClientOnRecvPacket = 0x3aa100; // 2976 0x35CA00; R2 B069, R2-MATCHES.json
inline constexpr uintptr_t C004_kServerUserActorVtable = 0x5f6de90; // 2976 0x5B257A8; R2 C004, R2-MATCHES.json
inline constexpr uintptr_t D002_kServerActorLookupSlot = 0x724b860; // 2976 0x6D69698; R2 D002, R2-MATCHES.json
inline constexpr uintptr_t D013_kWindowsLock3Vtable = 0x598c5a8; // 2976 0x557C120; R2 D013, R2-MATCHES.json
inline constexpr uintptr_t D018_kSaveMgrRefreshSlots = 0x1A19F90; // 2976 0x1870C20; R2 D018, R2-MATCHES.json
inline constexpr uintptr_t C041_kAcquireField = 0x2e0cfb0; // 2976 0x2B07790; R2 C041, R2-MATCHES.json
inline constexpr uintptr_t C045_kTryAddRef = 0x165aa10; // 2976 0x14343A0; R2 C045, R2-MATCHES.json
inline constexpr uintptr_t C082_kLoginMgrSlot = 0x724b870; // 2976 0x6D696A8; R2 C082, R2-MATCHES.json
inline constexpr uintptr_t C083_kFindByNoScan = 0x2d7f1b6; // 2976 0x2A84066; R2 C083, R2-MATCHES.json
inline constexpr uintptr_t C085_kFindByNoRead = 0x2d7f1e0; // 2976 0x2A84090; R2 C085, R2-MATCHES.json
inline constexpr uintptr_t C097_kLoad = 0x2296dd0; // 2976 0x20688A0; R2 C097, R2-MATCHES.json
inline constexpr uintptr_t C109_kWorldTime = 0x6f37c98; // 2976 0x6A6D238; R2 C109, R2-MATCHES.json
inline constexpr uintptr_t C116_kLoading = 0x23de900; // 2976 0x219D7F0; R2 C116, R2-MATCHES.json
inline constexpr uintptr_t C118_kLoadingFlagCmp = 0x23de93b; // 2976 0x219D82B; R2 C118, R2-MATCHES.json
inline constexpr uintptr_t C120_kGetter = 0x231be00; // 2976 0x20E55A0; R2 C120, R2-MATCHES.json
inline constexpr uintptr_t B073_kServerFrameworkMode = 0x6f2fb70; // 2976 0x6A65564; R2 B073, R2-MATCHES.json
inline constexpr uintptr_t D053_kLoadSlot = 0x1a188b0; // 2976 0x186F570; R2 D053, R2-MATCHES.json
inline constexpr uintptr_t D055_kSave = 0x1a175a0; // 2976 0x186E450; R2 D055, R2-MATCHES.json
inline constexpr uintptr_t D057_kFlush = 0x1a1af40; // 2976 0x1871930; R2 D057, R2-MATCHES.json
inline constexpr uintptr_t D059_kGetOrCreate = 0x25bd900; // 2976 0x2355EC0; R2 D059, R2-MATCHES.json
inline constexpr uintptr_t D061_kAddFile = 0x25c2d90; // 2976 0x235B100; R2 D061, R2-MATCHES.json
inline constexpr uintptr_t D068_kCallSites_5__site = 0x2bc548a; // 2976 0x28F273A; R2 D068, R2-MATCHES.json
inline constexpr uintptr_t D069_kCallSites_6__site = 0xe584ac8; // 2976 0x108393C8; R2 D069, R2-MATCHES.json
inline constexpr uintptr_t D070_kErrBusy_site = 0x1a18a3e; // 2976 0x186F6DF; R2 D070, R2-MATCHES.json
inline constexpr uintptr_t D071_kErrBusy_global = 0x71d6e3c; // 2976 0x6CF7CA4; R2 D071, R2-MATCHES.json
inline constexpr uintptr_t D072_kErrInvalidSlot_site = 0x1a1898f; // 2976 0x186F62C; R2 D072, R2-MATCHES.json
inline constexpr uintptr_t D073_kErrInvalidSlot_global = 0x71d6a6c; // 2976 0x6CF7900; R2 D073, R2-MATCHES.json
inline constexpr uintptr_t D074_kErrAbsentSlot_site = 0x2297bef; // 2976 0x2069615; R2 D074, R2-MATCHES.json
inline constexpr uintptr_t D075_kErrAbsentSlot_global = 0x71d6988; // 2976 0x6CF781C; R2 D075, R2-MATCHES.json
inline constexpr uintptr_t D076_kErrSaveFail_site = 0x1a1789a; // 2976 0x186E780; R2 D076, R2-MATCHES.json
inline constexpr uintptr_t D077_kErrSaveFail_global = 0x71d6e38; // 2976 0x6CF7CA0; R2 D077, R2-MATCHES.json
inline constexpr uintptr_t D105_kRetFlushLogout = 0x2bc548f; // 2976 0x28F273F; R2 D105, R2-MATCHES.json
inline constexpr uintptr_t D106_kRetFlushTimer = 0xe584acd; // 2976 0x108393CD; R2 D106, R2-MATCHES.json
inline constexpr uintptr_t D114_kReporter = 0x15388a0; // 2976 0x1317D40; R2 D114, R2-MATCHES.json
inline constexpr uintptr_t C137_kActionLayerAdvance = 0x21bf2b0; // 2976 0x1F63F70; R2 C137, R2-MATCHES.json
inline constexpr uintptr_t C139_kActionLayersAdvance = 0x258ea30; // 2976 0x2329700; R2 C139, R2-MATCHES.json
inline constexpr uintptr_t A031_kWorldSystemSlot = 0x724b350; // 2976 0x6D69190; R2 A031, R2-MATCHES.json
inline constexpr uintptr_t A032_kHpDefinitionsSlot = 0x724B6E8; // 2976 0x6D69528; R2 A032, R2-MATCHES.json
inline constexpr uintptr_t A033_kHpDefinitionsAltSlot = 0x724B6F0; // 2976 0x6D69530; R2 A033, R2-MATCHES.json
} // namespace core_rvas
// R2 reviewed layouts END

// R3 tunnel constants BEGIN
namespace r3_tunnel {
inline constexpr uintptr_t kTrocInstancesBegin = 0x6F1AE10; // R3 B052; 2976 0x6A51AE0
inline constexpr uintptr_t kTrocInstancesEnd = 0x6F4E7B0; // R3 B053; 2976 0x6A82C80
inline constexpr uintptr_t kNetLoopPump = 0x3A91A4; // R3 B065; 2976 0x35BAD4
inline constexpr uint32_t kPacketMax = 0x7FDA; // R3 B075 native PostPacket bound
inline constexpr uint32_t kOrdinaryPayloadMax = 0x7FC1; // R3 B075 native ordinary decoder bound
inline constexpr uint32_t kDescriptorSkip = 0; // R3 B078: target visits every aligned pointer
} // namespace r3_tunnel
// R3 tunnel constants END



// R3 join catalog BEGIN
namespace r3_join {
inline constexpr uintptr_t kLobbyVtable = 0x5f74678; // C069
inline constexpr uintptr_t kLobbyLoad = 0x2e8c100; // C067
inline constexpr uintptr_t kStatusVtable = 0x5f687b0; // C078
inline constexpr uintptr_t kStatusLoad = 0x2c1ab40; // C079
inline constexpr uintptr_t kEnterField = 0x2ECF680; // C043
inline constexpr uintptr_t kPhases = 0x2C1BEA0; // C099
inline constexpr uint8_t kPhasesHead[] = {0x44, 0x89, 0x4c, 0x24, 0x20, 0x4c, 0x89, 0x44, 0x24, 0x18, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xec, 0x80, 0x0, 0x0, 0x0, 0x4c, 0x8b, 0xfa, 0x4c, 0x8b, 0x71, 0x8, 0x49, 0x8b, 0x7e, 0x78, 0x41, 0x8b, 0xae, 0x80, 0x0, 0x0, 0x0, 0x48, 0xc1, 0xe5, 0x4}; // C100
} // namespace r3_join
// R3 join catalog END

}  // namespace cdmp::game::build_3663
