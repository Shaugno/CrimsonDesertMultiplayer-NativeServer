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

inline constexpr uintptr_t kSessionSend = 0x165C1F0;       // 2976 0x1437740; N0981, send
inline constexpr uintptr_t kSyncSessionSend = 0x165CB60;   // 2976 0x14380B0; N1030, send
inline constexpr uintptr_t kQueuePush = 0x3AB4C0;         // 2976 0x35DF50; N0985, queue
inline constexpr uintptr_t kAllocThread = 0x4B7E0C0;      // 2976 0x47EF9B0; N0989, alloc
inline constexpr uintptr_t kAllocGlobal = 0x4B7E188;      // 2976 0x47EFA78; N0987, alloc
inline constexpr uintptr_t kNwSessionCtor = 0x165B9D0;    // 2976 0x1436EE0; N0983, ctor
inline constexpr uintptr_t kTlsIndexSlot = 0x715BFD0;     // 2976 0x6C88C50; N0991, tls-index
inline constexpr uint32_t kTls_UseGlobalHeap = 0x1FE;     // 2976 0x1FD; N0992, tls-flag
inline constexpr uintptr_t kNwVirtualAsyncSessionVtable = 0x5B40310;  // 2976 0x57260C0; N0961
inline constexpr uintptr_t kNwAsyncSessionVtable = 0x5B403A8;         // 2976 0x5726140; N0962
inline constexpr uint32_t kSessionSize = 0xD0;            // 2976 0xD0; N0965, size
inline constexpr uint32_t kSessionVtableEntries = 15;     // 2976 15; N0963, vtable-count
inline constexpr uint32_t kSessionVfSend = 9;             // 2976 9; N0964, async-table
inline constexpr uint32_t kSession_OutQueue = 0xC8;       // 2976 0xC8; N0973, out-queue
inline constexpr uint32_t kRq_Frame = 0x30;               // 2976 0x30; N1018, frame
inline constexpr uintptr_t kServerOnRecvPacket = 0x2A0EA70; // 2976 0x2773C20; N0979, server-recv
inline constexpr uintptr_t kSaveMgrRefreshSlots = 0x1A18320; // 2976 0x1870C20; N1056, save-refresh

}  // namespace cdmp::game::build_3663
