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



// R2 reviewed layouts BEGIN
// Layout proof catalog: docs/probe-3663/r2-reviewed.json. No tier is armed.
namespace player_layout {
inline constexpr uint32_t kActor_ComponentTable = 0x68; // 2976 0x68; R2 A010
inline constexpr uint32_t kComponents_CharacterControl = 0x40; // 2976 0x40; R2 A012
inline constexpr uint32_t kControl_Inner = 0x140; // 2976 0x140; R2 A013
inline constexpr uint32_t kControlInner_Physics = 0x2D0; // 2976 0x2C0; R2 A014
inline constexpr uint32_t kPhysics_Position = 0x90; // 2976 0x90; R2 A018
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
} // namespace player_layout
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
} // namespace field_layout
// R2 reviewed layouts END

}  // namespace cdmp::game::build_3663
