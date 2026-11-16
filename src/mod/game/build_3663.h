#pragma once
// Corroborated static identities only. This is NOT a complete address table.
// Field sharing, player reads, identity/status, wire layouts, save ordering and
// cleanup remain blocked in docs/probe-3663/PORT-MATCHES.md. No 3663 gameplay
// installer or reader may use these until that complete path is validated.
// Row numbers refer to the Phase 3 inventory at commit 878d14f.

#include <cstdint>

namespace cdmp::game::build_3663 {

// File identity, independently checked by PE parsing and pkt3663_scan.
inline constexpr uint32_t kPeTimestamp = 0x6AF5D198;
inline constexpr uintptr_t kImageBase = 0x140000000;
inline constexpr uint32_t kSizeOfImage = 0x144E9000;

// R2 completeness gates. True requires every row of the named tier in
// R2-CORE-LIST/R2-MATCHES plus its per-build module routing and validation.
// These are compiled evidence, never ini/environment overrides.
inline constexpr bool kTierAComplete = false;
inline constexpr bool kTierBComplete = false;
inline constexpr bool kTierCComplete = false;
inline constexpr bool kTierDComplete = false;

inline constexpr uintptr_t kSessionSend = 0x165C1F0;       // 2976 0x1437740; N0981, send; R2 B023
inline constexpr uintptr_t kSyncSessionSend = 0x165CB60;   // 2976 0x14380B0; N1030, send; R2 B063
inline constexpr uintptr_t kQueuePush = 0x3AB4C0;         // 2976 0x35DF50; N0985, queue; R2 B027
inline constexpr uintptr_t kAllocThread = 0x4B7E0C0;      // 2976 0x47EF9B0; N0989, alloc; R2 B031
inline constexpr uintptr_t kAllocGlobal = 0x4B7E188;      // 2976 0x47EFA78; N0987, alloc; R2 B029
inline constexpr uintptr_t kNwSessionCtor = 0x165B9D0;    // 2976 0x1436EE0; N0983, ctor; R2 B025
inline constexpr uintptr_t kTlsIndexSlot = 0x715BFD0;     // 2976 0x6C88C50; N0991, tls-index; R2 B033
inline constexpr uint32_t kTls_UseGlobalHeap = 0x1FE;     // 2976 0x1FD; N0992, tls-flag; R2 B034
inline constexpr uintptr_t kNwVirtualAsyncSessionVtable = 0x5B40310;  // 2976 0x57260C0; N0961; R2 B008
inline constexpr uintptr_t kNwAsyncSessionVtable = 0x5B403A8;         // 2976 0x5726140; N0962; R2 B009
inline constexpr uint32_t kSessionSize = 0xD0;            // 2976 0xD0; N0965, size; R2 B012
inline constexpr uint32_t kSessionVtableEntries = 15;     // 2976 15; N0963, vtable-count; R2 B010
inline constexpr uint32_t kSessionVfSend = 9;             // 2976 9; N0964, async-table; R2 B011
inline constexpr uint32_t kSession_OutQueue = 0xC8;       // 2976 0xC8; N0973, out-queue; R2 B020
inline constexpr uint32_t kRq_Frame = 0x30;               // 2976 0x30; N1018, frame; R2 B051
inline constexpr uintptr_t kServerOnRecvPacket = 0x2A0EA70; // 2976 0x2773C20; N0979, server-recv; R2 B021
inline constexpr uintptr_t kSaveMgrRefreshSlots = 0x1A18320; // 2976 0x1870C20; N1056, save-refresh; R2 D018

// Passive external probe only. These RVA candidates are not production
// adoptions; their standard-correlator checks remain in progress. No ASI
// installer/reader obtains a table from this namespace.
namespace diagnostic_candidates {
inline constexpr uintptr_t kChunkOrigin = 0x7231330; // 2976 0x6D56500; R2 A001
inline constexpr uintptr_t kStaticPlayerPos = 0x7253E60; // 2976 0x6D77858; R2 A002
inline constexpr uintptr_t kVtCharacterControl = 0x59C1660; // 2976 0x55B5460; R2 A005
inline constexpr uintptr_t kVtPhysicsObject = 0x614A050; // 2976 0x5D00F88; R2 A007
inline constexpr uintptr_t kHpDefinitionsSlot = 0x7245558; // 2976 0x6D69528; R2 A032
inline constexpr uintptr_t kHpDefinitionsAltSlot = 0x7245560; // 2976 0x6D69530; R2 A033
inline constexpr uintptr_t kWorldSystemSlot = 0x72451C0; // 2976 0x6D69190; R2 A031
inline constexpr uintptr_t kVtClientActorManager = 0x59C2760; // 2976 0x55B62C8; R2 A003
inline constexpr uintptr_t kVtControlledActor = 0x5990BF0; // 2976 0x5585C40; R2 A004
inline constexpr uintptr_t kVtStatusComponent = 0x5998B60; // 2976 0x558D868; R2 A006
inline constexpr uint32_t kWorldSystem_ActorManager = 0x30; // 2976 0x30; R2 A008
inline constexpr uint32_t kActorManager_ControlledActor = 0x50; // 2976 0x50; R2 A009
} // namespace diagnostic_candidates

// R2 reviewed layouts BEGIN
// Layout proof catalog: docs/probe-3663/r2-reviewed.json. No tier is armed.
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
inline constexpr uint32_t kSyncSession_Epoch = 0xB0; // 2976 0xB0; R2 B062
inline constexpr uint32_t kClientNet_Slot4 = 0x58; // 2976 0x58; R2 B061
inline constexpr uint8_t kServerOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54}; // 2976 signature: src/mod/session2/offsets_s2.h:52; R2 B022
inline constexpr uint8_t kSessionSendHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30}; // 2976 signature: src/mod/session2/offsets_s2.h:55; R2 B024
inline constexpr uint8_t kNwSessionCtorHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57}; // 2976 signature: src/mod/session2/offsets_s2.h:58; R2 B026
inline constexpr uint8_t kQueuePushHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9}; // 2976 signature: src/mod/session2/offsets_s2.h:61; R2 B028
inline constexpr uint8_t kAllocGlobalHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA}; // 2976 signature: src/mod/session2/offsets_s2.h:72; R2 B030
inline constexpr uint8_t kAllocThreadBytes[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0xEE, 0xDE, 0x5D, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF}; // 2976 signature: src/mod/session2/offsets_s2.h:76; R2 B032
inline constexpr uint8_t kSyncSessionSendHead[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA}; // 2976 signature: src/mod/session2/offsets_s2.h:141; R2 B064
inline constexpr uint32_t kOpConnectCryptogramToServerReq = 2401; // 2976 2319; R2 B071
} // namespace tunnel_layout
namespace field_layout {
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
inline constexpr uint32_t kOpLoginCharacterReq = 3081; // 2976 3093; R2 D112
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
// R2 reviewed layouts END

}  // namespace cdmp::game::build_3663
