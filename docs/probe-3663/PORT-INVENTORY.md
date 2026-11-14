# 2976 core-path port inventory

Generated with `python tools/port3663/inventory.py` from the explicitly listed
core implementation seeds and their recursive project-header includes.
The validated walk-around uses option D (`session2`); `remote_puppet` supplies
shared helpers. Sources: README "Upstream co-op behavior", ROADMAP,
OPCION-D-EMULADOR, `session2::install`, `server_install`, and `guest_install`.
`kit/cdmp.ini` is a capture-only preset, not the validated multiplayer preset.

Minimal target: identity/startup, guarded player reads, UDP/Steam transport,
bounded handshake/tunnel, host session allocation/accept, guest remote login,
native movement relay, character lifecycle, reconnect, and save protection.
Preserve the Steam-friends restriction and the process executable filter.

This is a conservative inventory, not evidence that every row is needed or
verified. All named values in the scanned files are retained; raw inline
literals (including ordinary policy bounds) are in PORT-INVENTORY-LITERALS.md.
The JSON records exact source locations and the complete file list. Separate
semantic/reachability review is still required before any row is adopted.

2976 verification: the address headers cite `hallazgos-en-vivo.md`,
`opcion-d-d0.md`, `segunda-sesion.md`, and `troc-dispatch.md`; local implementation
comments cite further research. Those are historical author claims. They have
not been individually revalidated in this run. The 2976 reference is now
available with its required hash; per-row semantic audit remains. Each row
below therefore has **historical source only; revalidation pending**, and its
use column quotes its source statement rather than inventing a meaning.

Disabled on 3663: mounts/horse/ride, rope/tree, shops, docked gimmicks, NPC/enemy
relay/control, combat/damage probes, observer scene/schedule extras, map/minimap,
overlay, and every other optional session2 feature. Current production startup
denies ALL 3663 gameplay; config cannot reach these installers. Parent switches
`coop_enabled` and `s2_enabled` default false. Individual legacy suboptions may
default true on 2976, so they must not be reused as a 3663 preset.

No existing 2976 path or config defaults are changed by this inventory.

1292 rows; 193 scanned files.

| Row | File:line | Symbol/context | Kind | 2976 value | Source use |
|---|---|---|---|---|---|
| N0001 | src/common/capture_file.h:21 | kFileHeaderSize | RVA / layout / policy (audit) | 12 | inline constexpr size_t kFileHeaderSize = 12; |
| N0002 | src/common/capture_file.h:22 | kRecordHeaderSize | RVA / layout / policy (audit) | 24 | inline constexpr size_t kRecordHeaderSize = 24; |
| N0003 | src/common/capture_file.h:44 | kGapSession | RVA / layout / policy (audit) | 0xFFFFFFFF | inline constexpr uint32_t kGapSession = 0xFFFFFFFF; |
| N0004 | src/common/client_session.h:39 | kHelloRetryMs | RVA / layout / policy (audit) | 1000 | static constexpr uint32_t kHelloRetryMs = 1000; |
| N0005 | src/common/client_session.h:40 | kPingIntervalMs | RVA / layout / policy (audit) | 1000 | static constexpr uint32_t kPingIntervalMs = 1000; |
| N0006 | src/common/client_session.h:41 | kTimeoutMs | RVA / layout / policy (audit) | 5000 | static constexpr uint32_t kTimeoutMs = 5000; |
| N0007 | src/common/clock_mapper.h:24 | kWindowMs | RVA / layout / policy (audit) | 4000 | static constexpr uint32_t kWindowMs = 4000; |
| N0008 | src/common/clock_mapper.h:25 | kSlewPerSample | RVA / layout / policy (audit) | 1 | static constexpr int64_t kSlewPerSample = 1;  // ~40 ms/s at the Move rate |
| N0009 | src/common/clock_mapper.h:26 | kJumpThreshold | RVA / layout / policy (audit) | 250 | static constexpr int64_t kJumpThreshold = 250; |
| N0010 | src/common/clock_mapper.h:27 | kLateRunToReset | RVA / layout / policy (audit) | 20 | static constexpr int kLateRunToReset = 20;  // ~0.5 s of Moves |
| N0011 | src/common/loop_health.h:33 | kPhases | RVA / layout / policy (audit) | static_cast<size_t>(Phase::Count) | static constexpr size_t kPhases = static_cast<size_t>(Phase::Count); |
| N0012 | src/common/lz_block.h:29 | kMinMatch | RVA / layout / policy (audit) | 4 | inline constexpr size_t kMinMatch = 4; |
| N0013 | src/common/lz_block.h:30 | kMaxOffset | RVA / layout / policy (audit) | 65535 | inline constexpr size_t kMaxOffset = 65535; |
| N0014 | src/common/protocol.h:24 | kMagic | RVA / layout / policy (audit) | 0x504D4443 | inline constexpr uint32_t kMagic = 0x504D4443;  // bytes "CDMP" |
| N0015 | src/common/protocol.h:31 | kVersion | RVA / layout / policy (audit) | 8 | inline constexpr uint16_t kVersion = 8; |
| N0016 | src/common/protocol.h:35 | kHeaderSize | RVA / layout / policy (audit) | 16 | inline constexpr size_t kHeaderSize = 16; |
| N0017 | src/common/protocol.h:36 | kMaxPacketSize | RVA / layout / policy (audit) | 1200 | inline constexpr size_t kMaxPacketSize = 1200;  // stays under typical MTU |
| N0018 | src/common/protocol.h:37 | kMaxNameLen | RVA / layout / policy (audit) | 32 | inline constexpr size_t kMaxNameLen = 32; |
| N0019 | src/common/protocol.h:74 | kMaxEventKind | RVA / layout / policy (audit) | 12 | inline constexpr uint8_t kMaxEventKind = 12;  // newer kinds decode; the session drops them |
| N0020 | src/common/protocol.h:77 | kEnemyPosEventSize | RVA / layout / policy (audit) | 40 | inline constexpr size_t kEnemyPosEventSize = 40; |
| N0021 | src/common/protocol.h:82 | kDamageEventSize | RVA / layout / policy (audit) | 25 | inline constexpr size_t kDamageEventSize = 25; |
| N0022 | src/common/protocol.h:83 | kMaxEventPayload | RVA / layout / policy (audit) | 1024 | inline constexpr size_t kMaxEventPayload = 1024; |
| N0023 | src/common/protocol.h:96 | kMaxRelKind | RVA / layout / policy (audit) | 8 | inline constexpr uint8_t kMaxRelKind = 8;  // newer kinds are acknowledged and dropped |
| N0024 | src/common/protocol.h:101 | kSelfMarker | RVA / layout / policy (audit) | 0xA010F00F | inline constexpr uint32_t kSelfMarker = 0xA010F00F; |
| N0025 | src/common/protocol.h:102 | kRelChunk | RVA / layout / policy (audit) | 1100 | inline constexpr size_t kRelChunk = 1100;  // payload bytes per fragment |
| N0026 | src/common/protocol.h:103 | kMaxRelMessage | RVA / layout / policy (audit) | 256 * 1024 | inline constexpr size_t kMaxRelMessage = 256 * 1024; |
| N0027 | src/common/reliable.h:38 | kResendMs | RVA / layout / policy (audit) | 250 | static constexpr uint32_t kResendMs = 250;     // shortest resend timeout |
| N0028 | src/common/reliable.h:39 | kMaxResendMs | RVA / layout / policy (audit) | 2000 | static constexpr uint32_t kMaxResendMs = 2000; // longest, after backoff |
| N0029 | src/common/reliable.h:40 | kMaxResendPerTick | RVA / layout / policy (audit) | 64 | static constexpr size_t kMaxResendPerTick = 64; |
| N0030 | src/common/reliable.h:41 | kWindow | RVA / layout / policy (audit) | 128 | static constexpr size_t kWindow = 128;        // fragments in flight (unacknowledged), by default |
| N0031 | src/common/reliable.h:42 | kMaxWindow | RVA / layout / policy (audit) | 1024 | static constexpr size_t kMaxWindow = 1024; |
| N0032 | src/common/reliable.h:43 | kMaxQueued | RVA / layout / policy (audit) | 2048 | static constexpr size_t kMaxQueued = 2048;    // fragments waiting for the window (~2.2 MB) |
| N0033 | src/common/reliable.h:44 | kMaxPartials | RVA / layout / policy (audit) | 64 | static constexpr size_t kMaxPartials = 64;    // incomplete messages held by the receiver |
| N0034 | src/common/reliable.h:45 | kMaxAhead | RVA / layout / policy (audit) | 4096 | static constexpr uint32_t kMaxAhead = 4096;   // messages a fragment may be ahead of the next one due |
| N0035 | src/common/save_guard.cpp:10 | kGenericAll | RVA / layout / policy (audit) | 0x10000000 | constexpr uint32_t kGenericAll = 0x10000000; |
| N0036 | src/common/save_guard.cpp:11 | kGenericWrite | RVA / layout / policy (audit) | 0x40000000 | constexpr uint32_t kGenericWrite = 0x40000000; |
| N0037 | src/common/save_guard.cpp:12 | kMaximumAllowed | RVA / layout / policy (audit) | 0x02000000 | constexpr uint32_t kMaximumAllowed = 0x02000000; |
| N0038 | src/common/save_guard.cpp:13 | kDelete | RVA / layout / policy (audit) | 0x00010000 | constexpr uint32_t kDelete = 0x00010000; |
| N0039 | src/common/save_guard.cpp:14 | kWriteDac | RVA / layout / policy (audit) | 0x00040000 | constexpr uint32_t kWriteDac = 0x00040000; |
| N0040 | src/common/save_guard.cpp:15 | kWriteOwner | RVA / layout / policy (audit) | 0x00080000 | constexpr uint32_t kWriteOwner = 0x00080000; |
| N0041 | src/common/save_guard.cpp:16 | kFileWriteData | RVA / layout / policy (audit) | 0x0002 | constexpr uint32_t kFileWriteData = 0x0002; |
| N0042 | src/common/save_guard.cpp:17 | kFileAppendData | RVA / layout / policy (audit) | 0x0004 | constexpr uint32_t kFileAppendData = 0x0004; |
| N0043 | src/common/save_guard.cpp:18 | kFileWriteEa | RVA / layout / policy (audit) | 0x0010 | constexpr uint32_t kFileWriteEa = 0x0010; |
| N0044 | src/common/save_guard.cpp:19 | kFileWriteAttributes | RVA / layout / policy (audit) | 0x0100 | constexpr uint32_t kFileWriteAttributes = 0x0100; |
| N0045 | src/common/save_guard.cpp:20 | kWriteMask | RVA / layout / policy (audit) | kGenericAll &#124; kGenericWrite &#124; kMaximumAllowed &#124; kDelete &#124; kWriteDac &#124; kWriteOwner &#124; kFileWriteData &#124; kFileAppendData &#124; kFileWriteEa &#124; kFileWriteAttributes | constexpr uint32_t kWriteMask = kGenericAll &#124; kGenericWrite &#124; kMaximumAllowed &#124; kDelete &#124; kWriteDac &#124; |
| N0046 | src/common/save_guard.cpp:23 | kCreateNew | RVA / layout / policy (audit) | 1 | constexpr uint32_t kCreateNew = 1, kCreateAlways = 2, kOpenAlways = 4, kTruncateExisting = 5; |
| N0047 | src/common/save_guard.cpp:23 | kCreateAlways | RVA / layout / policy (audit) | 2 | constexpr uint32_t kCreateNew = 1, kCreateAlways = 2, kOpenAlways = 4, kTruncateExisting = 5; |
| N0048 | src/common/save_guard.cpp:23 | kOpenAlways | Troc ID (audit) | 4 | constexpr uint32_t kCreateNew = 1, kCreateAlways = 2, kOpenAlways = 4, kTruncateExisting = 5; |
| N0049 | src/common/save_guard.cpp:23 | kTruncateExisting | RVA / layout / policy (audit) | 5 | constexpr uint32_t kCreateNew = 1, kCreateAlways = 2, kOpenAlways = 4, kTruncateExisting = 5; |
| N0050 | src/common/save_guard.cpp:24 | kFlagDeleteOnClose | RVA / layout / policy (audit) | 0x04000000 | constexpr uint32_t kFlagDeleteOnClose = 0x04000000; |
| N0051 | src/common/save_guard.cpp:38 | kAccessBits | signature | { {0x80000000, }, {0x40000000, }, {0x20000000, }, {0x10000000, }, {0x02000000, }, {0x00100000, }, {0x00080000, }, {0x00040000, }, {0x00020000, }, {0x00010000, }, {0x0100, }, {0x0080, }, {0x0040, }, {0x0020, }, {0x0010, }, {0x0008, }, {0x0004, }, {0x0002, }, {0x0001, }, } | constexpr BitName kAccessBits[] = { |
| N0052 | src/common/save_guard.cpp:47 | kShareBits | signature | { {0x1, }, {0x2, }, {0x4, }, } | constexpr BitName kShareBits[] = { |
| N0053 | src/common/save_guard.cpp:51 | kFileFlagBits | signature | { {0x80000000, }, {0x40000000, }, {0x20000000, }, {0x10000000, }, {0x08000000, }, {0x04000000, }, {0x02000000, }, {0x01000000, }, {0x00200000, }, {0x0100, }, {0x0080, }, {0x0020, }, {0x0010, }, {0x0004, }, {0x0002, }, {0x0001, }, } | constexpr BitName kFileFlagBits[] = { |
| N0054 | src/common/save_guard.cpp:61 | kMoveBits | signature | { {0x01, }, {0x02, }, {0x04, }, {0x08, }, {0x10, }, {0x20, }, } | constexpr BitName kMoveBits[] = { |
| N0055 | src/common/save_guard.cpp:128 | kNames | signature | { , , , , } | static constexpr const char* kNames[] = {"CREATE_NEW", "CREATE_ALWAYS", "OPEN_EXISTING", "OPEN_ALWAYS", |
| N0056 | src/common/save_guard.h:84 | kNoMatch | RVA / layout / policy (audit) | static_cast<size_t>(-1) | static constexpr size_t kNoMatch = static_cast<size_t>(-1); |
| N0057 | src/common/sha256.h:129 | kDigits | RVA / layout / policy (audit) |  | static constexpr char kDigits[] = "0123456789abcdef"; |
| N0058 | src/common/steam_config.h:20 | kSendBufferSize | RVA / layout / policy (audit) | 9 | constexpr int32_t kSendBufferSize = 9; |
| N0059 | src/common/steam_config.h:21 | kSendRateMin | RVA / layout / policy (audit) | 10 | constexpr int32_t kSendRateMin = 10; |
| N0060 | src/common/steam_config.h:22 | kSendRateMax | RVA / layout / policy (audit) | 11 | constexpr int32_t kSendRateMax = 11; |
| N0061 | src/common/steam_config.h:26 | kRecvBufferSize | RVA / layout / policy (audit) | 47 | constexpr int32_t kRecvBufferSize = 47; |
| N0062 | src/common/steam_config.h:27 | kRecvBufferMessages | RVA / layout / policy (audit) | 48 | constexpr int32_t kRecvBufferMessages = 48; |
| N0063 | src/common/steam_config.h:28 | kMaxScannedId | RVA / layout / policy (audit) | 512 | constexpr int32_t kMaxScannedId = 512; |
| N0064 | src/common/tick_profile.h:23 | kMaxParts | RVA / layout / policy (audit) | 16 | static constexpr size_t kMaxParts = 16; |
| N0065 | src/common/troc_frame.h:30 | kHeaderSize | RVA / layout / policy (audit) | 5 | inline constexpr size_t kHeaderSize = 5; |
| N0066 | src/common/troc_frame.h:33 | kMaxPacketSize | RVA / layout / policy (audit) | 0x8000 - 0x20 | inline constexpr size_t kMaxPacketSize = 0x8000 - 0x20; |
| N0067 | src/common/troc_frame.h:60 | kOpLogoutSessionReq | Troc ID (audit) | 2001 | inline constexpr uint16_t kOpLogoutSessionReq = 2001; |
| N0068 | src/common/troc_frame.h:61 | kOpLogoutSessionAck | Troc ID (audit) | 2002 | inline constexpr uint16_t kOpLogoutSessionAck = 2002; |
| N0069 | src/common/troc_frame.h:62 | kOpLogoutUserAck | Troc ID (audit) | 2003 | inline constexpr uint16_t kOpLogoutUserAck = 2003; |
| N0070 | src/common/troc_frame.h:63 | kInternalHeaderSize | RVA / layout / policy (audit) | 3 | inline constexpr size_t kInternalHeaderSize = 3; |
| N0071 | src/common/troc_frame.h:64 | kLogoutAckSize | RVA / layout / policy (audit) | 7 | inline constexpr size_t kLogoutAckSize = 7; |
| N0072 | src/common/troc_frame.h:78 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1; |
| N0073 | src/common/troc_frame.h:79 | kControlHostReady | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kControlHostReady = 1;  // the host's session 2 exists and its tunnel end is up |
| N0074 | src/common/troc_frame.h:80 | kControlHostDead | RVA / layout / policy (audit) | 2 | inline constexpr uint8_t kControlHostDead = 2;   // the host stopped its end (the conversation is lost) |
| N0075 | src/common/troc_frame.h:83 | kControlHostCharacter | RVA / layout / policy (audit) | 4 | inline constexpr uint8_t kControlHostCharacter = 4; |
| N0076 | src/common/troc_frame.h:87 | kControlRejoin | RVA / layout / policy (audit) | 13 | inline constexpr uint8_t kControlRejoin = 13; |
| N0077 | src/common/troc_frame.h:88 | kControlEpoch | RVA / layout / policy (audit) | 14 | inline constexpr uint8_t kControlEpoch = 14; |
| N0078 | src/common/troc_frame.h:106 | kCapsOpcode | RVA / layout / policy (audit) | 2 | inline constexpr uint16_t kCapsOpcode = 2; |
| N0079 | src/common/troc_frame.h:107 | kCapsCompressed | RVA / layout / policy (audit) | 0x01 | inline constexpr uint8_t kCapsCompressed = 0x01;  // decodes compressed batches (below) |
| N0080 | src/common/troc_frame.h:108 | kCapsGuestSave | RVA / layout / policy (audit) | 0x02 | inline constexpr uint8_t kCapsGuestSave = 0x02;   // speaks control states 6-11 (the guest's own save, phase B) |
| N0081 | src/common/troc_frame.h:110 | kCapsRejoin | RVA / layout / policy (audit) | 0x08 | inline constexpr uint8_t kCapsRejoin = 0x08;      // speaks control states 13-14 (the re-entry, session2/rejoin_logic.h) |
| N0082 | src/common/troc_frame.h:116 | kBatchHeader | RVA / layout / policy (audit) | 3 | inline constexpr size_t kBatchHeader = 3; |
| N0083 | src/common/troc_frame.h:117 | kMaxBatchPackets | RVA / layout / policy (audit) | 0xFFFF | inline constexpr size_t kMaxBatchPackets = 0xFFFF; |
| N0084 | src/common/troc_frame.h:130 | kCompressedFlag | RVA / layout / policy (audit) | 0x80 | inline constexpr uint8_t kCompressedFlag = 0x80; |
| N0085 | src/common/troc_frame.h:131 | kCompressedHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kCompressedHeader = 5; |
| N0086 | src/common/troc_frame.h:132 | kMaxRawBatchBytes | RVA / layout / policy (audit) | 256 * 1024 | inline constexpr size_t kMaxRawBatchBytes = 256 * 1024;  // the decoder's bound |
| N0087 | src/common/troc_frame.h:185 | kMaxBatchBytes | RVA / layout / policy (audit) | 64 * 1024 | static constexpr size_t kMaxBatchBytes = 64 * 1024; |
| N0088 | src/common/troc_frame.h:187 | kMaxBacklogBytes | RVA / layout / policy (audit) | 16 * 1024 * 1024 | static constexpr size_t kMaxBacklogBytes = 16 * 1024 * 1024; |
| N0089 | src/common/troc_frame.h:194 | kMaxCompressPerFlush | RVA / layout / policy (audit) | 1024 * 1024 | static constexpr size_t kMaxCompressPerFlush = 1024 * 1024; |
| N0090 | src/common/troc_frame.h:196 | kMinCompressBytes | RVA / layout / policy (audit) | 256 | static constexpr size_t kMinCompressBytes = 256; |
| N0091 | src/common/troc_link.h:33 | kMaxPendingFragments | RVA / layout / policy (audit) | 1024 | static constexpr size_t kMaxPendingFragments = 1024; |
| N0092 | src/mod/bg_speed_logic.h:84 | kMaxFailures | RVA / layout / policy (audit) | 8 | static constexpr uint32_t kMaxFailures = 8;  // then it stops trying (said once by the caller) |
| N0093 | src/mod/config.cpp:12 | kDefaultIni | RVA / layout / policy (audit) | L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L | constexpr wchar_t kDefaultIni[] = |
| N0094 | src/mod/game/actor_lookup.cpp:15 | kManager_Table | RVA / layout / policy (audit) | 0x08 | constexpr uint32_t kManager_Table = 0x08; |
| N0095 | src/mod/game/actor_lookup.cpp:16 | kTable_BucketCount | RVA / layout / policy (audit) | 0x88 | constexpr uint32_t kTable_BucketCount = 0x88; |
| N0096 | src/mod/game/actor_lookup.cpp:17 | kTable_Buckets | RVA / layout / policy (audit) | 0x98 | constexpr uint32_t kTable_Buckets = 0x98; |
| N0097 | src/mod/game/actor_lookup.cpp:18 | kTable_Nodes | RVA / layout / policy (audit) | 0xA0 | constexpr uint32_t kTable_Nodes = 0xA0; |
| N0098 | src/mod/game/actor_lookup.cpp:19 | kBucketStride | RVA / layout / policy (audit) | 0x100 | constexpr uint32_t kBucketStride = 0x100; |
| N0099 | src/mod/game/actor_lookup.cpp:20 | kBucketMaxEntries | RVA / layout / policy (audit) | (kBucketStride - 8) / 8 | constexpr uint32_t kBucketMaxEntries = (kBucketStride - 8) / 8; |
| N0100 | src/mod/game/actor_lookup.cpp:21 | kNode_Key | RVA / layout / policy (audit) | 0x04 | constexpr uint32_t kNode_Key = 0x04; |
| N0101 | src/mod/game/actor_lookup.cpp:22 | kNode_Actor | RVA / layout / policy (audit) | 0x08 | constexpr uint32_t kNode_Actor = 0x08; |
| N0102 | src/mod/game/actor_lookup.cpp:24 | kComponents_TransformSync | RVA / layout / policy (audit) | 0x1A0 | constexpr uint32_t kComponents_TransformSync = 0x1A0; |
| N0103 | src/mod/game/actor_lookup.cpp:25 | kTransformSync_Position | RVA / layout / policy (audit) | 0xB4 | constexpr uint32_t kTransformSync_Position = 0xB4;   // vec3, local to the sector |
| N0104 | src/mod/game/actor_lookup.cpp:26 | kTransformSync_Sector | RVA / layout / policy (audit) | 0xC0 | constexpr uint32_t kTransformSync_Sector = 0xC0;     // i16 x, i16 z (1000 m sectors) |
| N0105 | src/mod/game/actor_lookup.cpp:27 | kTransformSync_Rotation | RVA / layout / policy (audit) | 0xA4 | constexpr uint32_t kTransformSync_Rotation = 0xA4;   // quaternion (caballo-coop.md §5) |
| N0106 | src/mod/game/actor_lookup.cpp:28 | kSectorSize | RVA / layout / policy (audit) | 1000.0f | constexpr float kSectorSize = 1000.0f; |
| N0107 | src/mod/game/build_3576.h:13 | kPeTimestamp | RVA / layout / policy (audit) | 0x6AED2E85 | inline constexpr uint32_t kPeTimestamp = 0x6AED2E85; |
| N0108 | src/mod/game/build_3576.h:14 | kImageBase | RVA / layout / policy (audit) | 0x140000000 | inline constexpr uintptr_t kImageBase = 0x140000000; |
| N0109 | src/mod/game/build_3576.h:15 | kSizeOfImage | RVA / layout / policy (audit) | 0x13CC4000 | inline constexpr uint32_t kSizeOfImage = 0x13CC4000; |
| N0110 | src/mod/game/build_guard.h:24 | kMaxELfanew | RVA / layout / policy (audit) | 1024 | inline constexpr uint32_t kMaxELfanew = 1024; |
| N0111 | src/mod/game/build_guard.h:25 | kDosMagic | RVA / layout / policy (audit) | 0x5A4D | inline constexpr uint16_t kDosMagic = 0x5A4D; |
| N0112 | src/mod/game/build_guard.h:26 | kPeSignature | RVA / layout / policy (audit) | 0x00004550 | inline constexpr uint32_t kPeSignature = 0x00004550; |
| N0113 | src/mod/game/build_guard.h:27 | kPe32PlusMagic | RVA / layout / policy (audit) | 0x020B | inline constexpr uint16_t kPe32PlusMagic = 0x020B; |
| N0114 | src/mod/game/game_errors.cpp:25 | kErrorLogger | RVA / layout / policy (audit) | 0x35D8D0 | constexpr uintptr_t kErrorLogger = 0x35D8D0; |
| N0115 | src/mod/game/game_errors.cpp:26 | kErrorLoggerHead | signature | {0xE9} | constexpr uint8_t kErrorLoggerHead[] = {0xE9};  // starts with a jmp into .xtls |
| N0116 | src/mod/game/game_errors.cpp:27 | kArgCode | RVA / layout / policy (audit) | 0x50 | constexpr uint32_t kArgCode = 0x50, kArgSeverity = 0x58, kArgText = 0x60, kArgValue = 0x68; |
| N0117 | src/mod/game/game_errors.cpp:27 | kArgSeverity | RVA / layout / policy (audit) | 0x58 | constexpr uint32_t kArgCode = 0x50, kArgSeverity = 0x58, kArgText = 0x60, kArgValue = 0x68; |
| N0118 | src/mod/game/game_errors.cpp:27 | kArgText | RVA / layout / policy (audit) | 0x60 | constexpr uint32_t kArgCode = 0x50, kArgSeverity = 0x58, kArgText = 0x60, kArgValue = 0x68; |
| N0119 | src/mod/game/game_errors.cpp:27 | kArgValue | RVA / layout / policy (audit) | 0x68 | constexpr uint32_t kArgCode = 0x50, kArgSeverity = 0x58, kArgText = 0x60, kArgValue = 0x68; |
| N0120 | src/mod/game/game_errors.cpp:28 | kExtraValues | RVA / layout / policy (audit) | 3 | constexpr int kExtraValues = 3;  // arguments 14-16 (more format values, if any) |
| N0121 | src/mod/game/game_errors.cpp:69 | kActorCreateFailedFrom | RVA / layout / policy (audit) | 0x8BDF7C | constexpr uintptr_t kActorCreateFailedFrom = 0x8BDF7C; |
| N0122 | src/mod/game/game_fault.cpp:12 | kImageSpan | RVA / layout / policy (audit) | 0x20000000 | constexpr uint64_t kImageSpan = 0x20000000;  // the exe's mapped size is below this (build 2976) |
| N0123 | src/mod/game/hook_prepare.cpp:16 | kTwoGb | RVA / layout / policy (audit) | 0x7FFFFFFFull | constexpr uint64_t kTwoGb = 0x7FFFFFFFull;  // safetyhook's default max_distance |
| N0124 | src/mod/game/hook_prepare.cpp:17 | kGranule | RVA / layout / policy (audit) | 0x10000 | constexpr uint64_t kGranule = 0x10000; |
| N0125 | src/mod/game/hook_prepare.cpp:18 | kNearWords | RVA / layout / policy (audit) | 6 + 48 * 5 | constexpr size_t kNearWords = 6 + 48 * 5; |
| N0126 | src/mod/game/hook_prepare.cpp:183 | kWhat | signature | { , , } | static constexpr const char* kWhat[] = {"not free", "free, VirtualAlloc refused", "free, no 64 KB-aligned room"}; |
| N0127 | src/mod/game/hook_prepare.h:35 | kHookTries | RVA / layout / policy (audit) | 3 | inline constexpr int kHookTries = 3; |
| N0128 | src/mod/game/hook_prepare.h:36 | kHookRetryMs | RVA / layout / policy (audit) | 250 | inline constexpr uint32_t kHookRetryMs = 250; |
| N0129 | src/mod/game/hook_prepare.h:37 | kNeighbourhoodLines | RVA / layout / policy (audit) | 4 | inline constexpr int kNeighbourhoodLines = 4; |
| N0130 | src/mod/game/offsets.h:11 | kPeTimestamp | RVA / layout / policy (audit) | 0x6AB28F00 | inline constexpr uint32_t kPeTimestamp = 0x6AB28F00; |
| N0131 | src/mod/game/offsets.h:12 | kImageBase | RVA / layout / policy (audit) | 0x140000000 | inline constexpr uintptr_t kImageBase = 0x140000000;  // fixed, no ASLR |
| N0132 | src/mod/game/offsets.h:15 | kWorldSystemSlot | RVA / layout / policy (audit) | 0x6D69190 | inline constexpr uintptr_t kWorldSystemSlot = 0x6D69190;  // -> WorldSystem* |
| N0133 | src/mod/game/offsets.h:16 | kChunkOrigin | RVA / layout / policy (audit) | 0x6D56500 | inline constexpr uintptr_t kChunkOrigin = 0x6D56500;      // vec3: world = physics local + this |
| N0134 | src/mod/game/offsets.h:17 | kStaticPlayerPos | RVA / layout / policy (audit) | 0x6D77858 | inline constexpr uintptr_t kStaticPlayerPos = 0x6D77858;  // vec3 world pos (cross-check) |
| N0135 | src/mod/game/offsets.h:20 | kVtClientActorManager | vtable | 0x55B62C8 | inline constexpr uintptr_t kVtClientActorManager = 0x55B62C8; |
| N0136 | src/mod/game/offsets.h:21 | kVtControlledActor | vtable | 0x5585C40 | inline constexpr uintptr_t kVtControlledActor = 0x5585C40;  // ClientChildOnlyInGameActor |
| N0137 | src/mod/game/offsets.h:22 | kVtCharacterControl | vtable | 0x55B5460 | inline constexpr uintptr_t kVtCharacterControl = 0x55B5460; |
| N0138 | src/mod/game/offsets.h:23 | kVtStatusComponent | vtable | 0x558D868 | inline constexpr uintptr_t kVtStatusComponent = 0x558D868; |
| N0139 | src/mod/game/offsets.h:24 | kVtPhysicsObject | vtable | 0x5D00F88 | inline constexpr uintptr_t kVtPhysicsObject = 0x5D00F88; |
| N0140 | src/mod/game/offsets.h:27 | kWorldSystem_ActorManager | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kWorldSystem_ActorManager = 0x30; |
| N0141 | src/mod/game/offsets.h:29 | kActorManager_ControlledActor | RVA / layout / policy (audit) | 0x50 | inline constexpr uint32_t kActorManager_ControlledActor = 0x50; |
| N0142 | src/mod/game/offsets.h:31 | kActor_ComponentTable | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActor_ComponentTable = 0x68; |
| N0143 | src/mod/game/offsets.h:33 | kComponents_Status | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kComponents_Status = 0x20; |
| N0144 | src/mod/game/offsets.h:34 | kComponents_CharacterControl | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kComponents_CharacterControl = 0x40; |
| N0145 | src/mod/game/offsets.h:36 | kControl_Inner | RVA / layout / policy (audit) | 0x140 | inline constexpr uint32_t kControl_Inner = 0x140; |
| N0146 | src/mod/game/offsets.h:37 | kControlInner_Physics | RVA / layout / policy (audit) | 0x2C0 | inline constexpr uint32_t kControlInner_Physics = 0x2C0; |
| N0147 | src/mod/game/offsets.h:39 | kPhysics_AxisX | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kPhysics_AxisX = 0x60; |
| N0148 | src/mod/game/offsets.h:40 | kPhysics_AxisY | RVA / layout / policy (audit) | 0x70 | inline constexpr uint32_t kPhysics_AxisY = 0x70; |
| N0149 | src/mod/game/offsets.h:41 | kPhysics_AxisZ | RVA / layout / policy (audit) | 0x80 | inline constexpr uint32_t kPhysics_AxisZ = 0x80; |
| N0150 | src/mod/game/offsets.h:42 | kPhysics_Position | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kPhysics_Position = 0x90; |
| N0151 | src/mod/game/offsets.h:47 | kStatus_StatContainer | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kStatus_StatContainer = 0x18; |
| N0152 | src/mod/game/offsets.h:48 | kStatContainer_Entries | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kStatContainer_Entries = 0x58; |
| N0153 | src/mod/game/offsets.h:49 | kStatContainer_Count | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kStatContainer_Count = 0x60; |
| N0154 | src/mod/game/offsets.h:50 | kStatEntry_Size | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kStatEntry_Size = 0x90; |
| N0155 | src/mod/game/offsets.h:51 | kStatEntry_Id | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kStatEntry_Id = 0x00; |
| N0156 | src/mod/game/offsets.h:52 | kStatEntry_Current | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kStatEntry_Current = 0x08; |
| N0157 | src/mod/game/offsets.h:53 | kStatEntry_Max | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kStatEntry_Max = 0x18; |
| N0158 | src/mod/game/offsets.h:54 | kStatId_Health | RVA / layout / policy (audit) | 0 | inline constexpr uint64_t kStatId_Health = 0;    // verified: regenerates, max 300 |
| N0159 | src/mod/game/offsets.h:55 | kStatId_Stamina | RVA / layout / policy (audit) | 22 | inline constexpr uint64_t kStatId_Stamina = 22;  // verified: drains while sprinting, max 120 |
| N0160 | src/mod/game/offsets.h:56 | kMaxStatEntries | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kMaxStatEntries = 64;  // sanity bound |
| N0161 | src/mod/game/offsets.h:59 | kCameraSlot | RVA / layout / policy (audit) | 0x6C8CF30 | inline constexpr uintptr_t kCameraSlot = 0x6C8CF30; |
| N0162 | src/mod/game/offsets.h:60 | kVtCamera | vtable | 0x5D20718 | inline constexpr uintptr_t kVtCamera = 0x5D20718; |
| N0163 | src/mod/game/offsets.h:61 | kCamera_Block | RVA / layout / policy (audit) | 0x428 | inline constexpr uint32_t kCamera_Block = 0x428; |
| N0164 | src/mod/game/offsets.h:62 | kCameraBlock_Position | RVA / layout / policy (audit) | 0x80 | inline constexpr uint32_t kCameraBlock_Position = 0x80; |
| N0165 | src/mod/game/offsets.h:63 | kCameraBlock_Forward | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kCameraBlock_Forward = 0x90; |
| N0166 | src/mod/game/offsets.h:64 | kCameraBlock_View | RVA / layout / policy (audit) | 0x3E0 | inline constexpr uint32_t kCameraBlock_View = 0x3E0; |
| N0167 | src/mod/game/offsets.h:65 | kCameraBlock_Projection | RVA / layout / policy (audit) | 0x4E0 | inline constexpr uint32_t kCameraBlock_Projection = 0x4E0; |
| N0168 | src/mod/game/offsets.h:66 | kCameraBlock_Sentinel | RVA / layout / policy (audit) | 0xAC0 | inline constexpr uint32_t kCameraBlock_Sentinel = 0xAC0; |
| N0169 | src/mod/game/offsets.h:67 | kCameraBlockSentinelValue | RVA / layout / policy (audit) | 6360000.0f | inline constexpr float kCameraBlockSentinelValue = 6360000.0f; |
| N0170 | src/mod/game/offsets.h:71 | kClientPostPacket | RVA / layout / policy (audit) | 0x35C670 | inline constexpr uintptr_t kClientPostPacket = 0x35C670;    // client -> server bytes |
| N0171 | src/mod/game/offsets.h:72 | kClientOnRecvPacket | RVA / layout / policy (audit) | 0x35CA00 | inline constexpr uintptr_t kClientOnRecvPacket = 0x35CA00;  // server -> client bytes |
| N0172 | src/mod/game/offsets.h:74 | kClientPostPacketHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x55, 0x56, 0x57, 0x41} | inline constexpr uint8_t kClientPostPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, |
| N0173 | src/mod/game/offsets.h:76 | kClientOnRecvPacketHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24} | inline constexpr uint8_t kClientOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, |
| N0174 | src/mod/game/offsets.h:79 | kHookArgSizeStackOffset | RVA / layout / policy (audit) | 0x28 | inline constexpr uint32_t kHookArgSizeStackOffset = 0x28; |
| N0175 | src/mod/game/offsets.h:81 | kTrocInstancesBegin | RVA / layout / policy (audit) | 0x6A51AE0 | inline constexpr uintptr_t kTrocInstancesBegin = 0x6A51AE0; |
| N0176 | src/mod/game/offsets.h:82 | kTrocInstancesEnd | RVA / layout / policy (audit) | 0x6A82C80 | inline constexpr uintptr_t kTrocInstancesEnd = 0x6A82C80; |
| N0177 | src/mod/game/offsets.h:83 | kTroc_Kind | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kTroc_Kind = 0x08;    // u32: 4 Req, 1 Ack/Nak, 2 auth, 0x7FFFFFFE internal |
| N0178 | src/mod/game/offsets.h:84 | kTroc_Opcode | RVA / layout / policy (audit) | 0x0C | inline constexpr uint32_t kTroc_Opcode = 0x0C;  // u16 |
| N0179 | src/mod/game/offsets.h:86 | kOpActorLoadDataAck | Troc ID (audit) | 2005 | inline constexpr uint16_t kOpActorLoadDataAck = 2005; |
| N0180 | src/mod/game/offsets.h:87 | kOpRemoveActorAck | Troc ID (audit) | 2516 | inline constexpr uint16_t kOpRemoveActorAck = 2516; |
| N0181 | src/mod/game/offsets.h:88 | kOpMoveActorReq | Troc ID (audit) | 2556 | inline constexpr uint16_t kOpMoveActorReq = 2556; |
| N0182 | src/mod/game/offsets.h:89 | kOpMoveActorAck | Troc ID (audit) | 2697 | inline constexpr uint16_t kOpMoveActorAck = 2697; |
| N0183 | src/mod/game/offsets.h:90 | kOpStartActionReq | Troc ID (audit) | 2223 | inline constexpr uint16_t kOpStartActionReq = 2223; |
| N0184 | src/mod/game/offsets.h:91 | kOpStartActionAck | Troc ID (audit) | 2991 | inline constexpr uint16_t kOpStartActionAck = 2991; |
| N0185 | src/mod/game/offsets.h:92 | kOpRotateActorReq | Troc ID (audit) | 2828 | inline constexpr uint16_t kOpRotateActorReq = 2828; |
| N0186 | src/mod/game/offsets.h:93 | kOpRotateActorAck | Troc ID (audit) | 2687 | inline constexpr uint16_t kOpRotateActorAck = 2687; |
| N0187 | src/mod/game/offsets.h:94 | kOpUpdateWallUpDataAck | Troc ID (audit) | 2893 | inline constexpr uint16_t kOpUpdateWallUpDataAck = 2893; |
| N0188 | src/mod/game/offsets.h:95 | kOpUnequipItemAck | Troc ID (audit) | 2280 | inline constexpr uint16_t kOpUnequipItemAck = 2280; |
| N0189 | src/mod/game/offsets.h:96 | kOpOtherEquipItemAck | Troc ID (audit) | 2757 | inline constexpr uint16_t kOpOtherEquipItemAck = 2757; |
| N0190 | src/mod/game/offsets.h:97 | kOpOtherUnequipItemAck | Troc ID (audit) | 3073 | inline constexpr uint16_t kOpOtherUnequipItemAck = 3073;  // u32 key &#124; u16 slot &#124; u32 serial |
| N0191 | src/mod/game/offsets.h:110 | kOpCreateOrChangePinMarkerAck | Troc ID (audit) | 2824 | inline constexpr uint16_t kOpCreateOrChangePinMarkerAck = 2824; |
| N0192 | src/mod/game/offsets.h:111 | kOpRemovePinMarkerAck | Troc ID (audit) | 2992 | inline constexpr uint16_t kOpRemovePinMarkerAck = 2992; |
| N0193 | src/mod/game/offsets.h:112 | kCoopPinId | RVA / layout / policy (audit) | 0x7FDD00000000F001ull | inline constexpr uint64_t kCoopPinId = 0x7FDD00000000F001ull; |
| N0194 | src/mod/game/offsets.h:113 | kPinGroupNormal | RVA / layout / policy (audit) | 0 | inline constexpr uint8_t kPinGroupNormal = 0; |
| N0195 | src/mod/game/offsets.h:114 | kPinShapeCount | RVA / layout / policy (audit) | 14 | inline constexpr uint8_t kPinShapeCount = 14, kPinColourCount = 15; |
| N0196 | src/mod/game/offsets.h:114 | kPinColourCount | RVA / layout / policy (audit) | 15 | inline constexpr uint8_t kPinShapeCount = 14, kPinColourCount = 15; |
| N0197 | src/mod/game/offsets.h:119 | kMinimapIconDecide | RVA / layout / policy (audit) | 0x9E89C0 | inline constexpr uintptr_t kMinimapIconDecide = 0x9E89C0; |
| N0198 | src/mod/game/offsets.h:120 | kMinimapIconDecideHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x18, 0x55} | inline constexpr uint8_t kMinimapIconDecideHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, |
| N0199 | src/mod/game/offsets.h:122 | kMinimapIconRoot | RVA / layout / policy (audit) | 0x6D69528 | inline constexpr uintptr_t kMinimapIconRoot = 0x6D69528; |
| N0200 | src/mod/game/offsets.h:123 | kMinimapIconRootAlt | RVA / layout / policy (audit) | 0x6D69530 | inline constexpr uintptr_t kMinimapIconRootAlt = 0x6D69530;  // used under a TLS flag |
| N0201 | src/mod/game/offsets.h:124 | kMinimapIconPlayerArrow | RVA / layout / policy (audit) | 0x126 | inline constexpr int kMinimapIconPlayerArrow = 0x126;       // the local player's arrow |
| N0202 | src/mod/game/offsets.h:125 | kMinimapIconCompanion | RVA / layout / policy (audit) | 0x100 | inline constexpr int kMinimapIconCompanion = 0x100;         // a mercenary in our clan |
| N0203 | src/mod/game/offsets.h:126 | kMinimapIconOutlinedArrow | RVA / layout / policy (audit) | 0x124 | inline constexpr int kMinimapIconOutlinedArrow = 0x124;     // key 23, the co-op default |
| N0204 | src/mod/game/offsets.h:127 | kActor_Key | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActor_Key = 0x60;                 // u32 actor key |
| N0205 | src/mod/game/offsets.h:135 | kServerOtherEquipWriter | RVA / layout / policy (audit) | 0x29ED1F0 | inline constexpr uintptr_t kServerOtherEquipWriter = 0x29ED1F0; |
| N0206 | src/mod/game/offsets.h:136 | kServerOtherEquipWriterHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55} | inline constexpr uint8_t kServerOtherEquipWriterHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x4C, 0x89, 0x4C, |
| N0207 | src/mod/game/offsets.h:138 | kServerActor_Key | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kServerActor_Key = 0x60; |
| N0208 | src/mod/game/offsets.h:145 | kOpChangeDockingSocketByFrameEventAck | Troc ID (audit) | 2747 | inline constexpr uint16_t kOpChangeDockingSocketByFrameEventAck = 2747; |
| N0209 | src/mod/game/offsets.h:159 | kRelayedAcks | signature | { {2147, false, 92, {0x54}}, {2586, false, 90, {0x54}}, {2434, false, 41, {}}, {2506, true, 77, {0x18, 0x1C, 0x21}}, {3007, true, 77, {0x18, 0x1C, 0x21}}, {2252, false, 42, {}}, {2642, false, 118, {0x28}}, {2530, true, 19, {0x04}}, {2899, true, 0, {}}, {3053, true, 0, {}}, {2475, true, 0, {}}, {2622, true, 279, {}}, {2763, true, 97, {}}, } | inline constexpr RelayedAck kRelayedAcks[] = { |
| N0210 | src/mod/game/offsets.h:176 | kServerDockingWriter | RVA / layout / policy (audit) | 0x2969BC0 | inline constexpr uintptr_t kServerDockingWriter = 0x2969BC0; |
| N0211 | src/mod/game/offsets.h:177 | kServerDockingWriterHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57} | inline constexpr uint8_t kServerDockingWriterHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x55, 0x56, 0x57, |
| N0212 | src/mod/game/offsets.h:180 | kLoadRotationFromEnd | RVA / layout / policy (audit) | 142 | inline constexpr size_t kLoadRotationFromEnd = 142; |
| N0213 | src/mod/game/offsets.h:184 | kControl_MoveController | RVA / layout / policy (audit) | 0xC8 | inline constexpr uint32_t kControl_MoveController = 0xC8;  // X = [CC+0xC8] |
| N0214 | src/mod/game/offsets.h:185 | kMoveController_Method | RVA / layout / policy (audit) | 0xF0 | inline constexpr uint32_t kMoveController_Method = 0xF0;   // u8 active movement method |
| N0215 | src/mod/game/offsets.h:186 | kMoveController_WallUp | RVA / layout / policy (audit) | 0x120 | inline constexpr uint32_t kMoveController_WallUp = 0x120;  // WallUpMovementMethod* |
| N0216 | src/mod/game/offsets.h:187 | kVtWallUpMovementMethod | vtable | 0x55968B8 | inline constexpr uintptr_t kVtWallUpMovementMethod = 0x55968B8; |
| N0217 | src/mod/game/offsets.h:188 | kMoveMethodWallUp | RVA / layout / policy (audit) | 5 | inline constexpr uint8_t kMoveMethodWallUp = 5; |
| N0218 | src/mod/game/offsets.h:189 | kControl_WallData | RVA / layout / policy (audit) | 0xD8 | inline constexpr uint32_t kControl_WallData = 0xD8;        // D = [CC+0xD8] |
| N0219 | src/mod/game/offsets.h:194 | kPointClimbEnter | RVA / layout / policy (audit) | 0x691400 | inline constexpr uintptr_t kPointClimbEnter = 0x691400; |
| N0220 | src/mod/game/offsets.h:195 | kPointClimbEnterHead | signature | {0x4C, 0x8B, 0xDC, 0x49, 0x89, 0x5B, 0x10, 0x49, 0x89, 0x6B, 0x18, 0x49, 0x89, 0x73, 0x20, 0x57} | inline constexpr uint8_t kPointClimbEnterHead[] = {0x4C, 0x8B, 0xDC, 0x49, 0x89, 0x5B, 0x10, 0x49, |
| N0221 | src/mod/game/offsets.h:207 | kActionLayerAdvance | RVA / layout / policy (audit) | 0x1F63F70 | inline constexpr uintptr_t kActionLayerAdvance = 0x1F63F70; |
| N0222 | src/mod/game/offsets.h:208 | kActionLayerAdvanceHead | signature | {0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0xC5, 0xF8, 0x29, 0x78, 0xD8} | inline constexpr uint8_t kActionLayerAdvanceHead[] = {0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, |
| N0223 | src/mod/game/offsets.h:218 | kActionLayersAdvance | RVA / layout / policy (audit) | 0x2329700 | inline constexpr uintptr_t kActionLayersAdvance = 0x2329700; |
| N0224 | src/mod/game/offsets.h:219 | kActionLayersAdvanceHead | signature | {0x48, 0x8B, 0xC4, 0x41, 0x54, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x68, 0xC5, 0xF8, 0x29, 0x70, 0xC8} | inline constexpr uint8_t kActionLayersAdvanceHead[] = {0x48, 0x8B, 0xC4, 0x41, 0x54, 0x41, 0x56, 0x48, |
| N0225 | src/mod/game/offsets.h:221 | kControl_Layer0 | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kControl_Layer0 = 0x88; |
| N0226 | src/mod/game/offsets.h:222 | kLayer_Playback | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kLayer_Playback = 0x18; |
| N0227 | src/mod/game/offsets.h:223 | kLayer_Chart | RVA / layout / policy (audit) | 0x110 | inline constexpr uint32_t kLayer_Chart = 0x110; |
| N0228 | src/mod/game/offsets.h:224 | kRideOnEnter | RVA / layout / policy (audit) | 0x681C80 | inline constexpr uintptr_t kRideOnEnter = 0x681C80; |
| N0229 | src/mod/game/offsets.h:225 | kRideOffEnter | RVA / layout / policy (audit) | 0x682C20 | inline constexpr uintptr_t kRideOffEnter = 0x682C20; |
| N0230 | src/mod/game/offsets.h:226 | kRideEnterHead | signature | {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x18, 0x55, 0x57, 0x41, 0x54, 0x41} | inline constexpr uint8_t kRideEnterHead[] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, |
| N0231 | src/mod/game/offsets.h:228 | kLoadKindMount | RVA / layout / policy (audit) | 5 | inline constexpr uint8_t kLoadKindMount = 5;  // ActorLoadDataAck +0x0D: 1 player, 5 mount |
| N0232 | src/mod/game/offsets.h:229 | kMethod_Actor | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kMethod_Actor = 0x20;       // movement method -> its actor |
| N0233 | src/mod/game/offsets.h:230 | kActor_OwnerUser | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kActor_OwnerUser = 0x90;    // u32 owning user key (0 = none) |
| N0234 | src/mod/game/offsets.h:233 | kLoadPositionFromEnd | RVA / layout / policy (audit) | 154 | inline constexpr size_t kLoadPositionFromEnd = 154; |
| N0235 | src/mod/game/offsets.h:236 | kLoadingParkHeight | RVA / layout / policy (audit) | 1000.0f | inline constexpr float kLoadingParkHeight = 1000.0f; |
| N0236 | src/mod/game/offsets.h:237 | kLoadingParkRadius | RVA / layout / policy (audit) | 5.0f | inline constexpr float kLoadingParkRadius = 5.0f; |
| N0237 | src/mod/game/safe_memory.cpp:11 | kMinAddress | RVA / layout / policy (audit) | 0x10000 | constexpr uintptr_t kMinAddress = 0x10000; |
| N0238 | src/mod/game/safe_memory.cpp:12 | kMaxAddress | RVA / layout / policy (audit) | 0x00007FFFFFFFFFFFull | constexpr uintptr_t kMaxAddress = 0x00007FFFFFFFFFFFull; |
| N0239 | src/mod/log.h:50 | kEmergencyFlushMs | RVA / layout / policy (audit) | 50 | inline constexpr uint32_t kEmergencyFlushMs = 50; |
| N0240 | src/mod/log.h:52 | kCrashFlushMs | RVA / layout / policy (audit) | 400 | inline constexpr uint32_t kCrashFlushMs = 400; |
| N0241 | src/mod/log.h:54 | kTerminateFlushMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint32_t kTerminateFlushMs = 1000; |
| N0242 | src/mod/log.h:60 | kStackLine | RVA / layout / policy (audit) | 1024 | inline constexpr size_t kStackLine = 1024; |
| N0243 | src/mod/log_ring.h:39 | kCapacity | RVA / layout / policy (audit) | Capacity | static constexpr size_t kCapacity = Capacity; |
| N0244 | src/mod/log_ring.h:41 | kMaxLine | RVA / layout / policy (audit) | Capacity / 8 < 16384 ? Capacity / 8 : 16384 | static constexpr size_t kMaxLine = Capacity / 8 < 16384 ? Capacity / 8 : 16384; |
| N0245 | src/mod/mod.cpp:79 | kModBuild | RVA / layout / policy (audit) | CDMP_BUILD_ID | constexpr uint32_t kModBuild = CDMP_BUILD_ID; |
| N0246 | src/mod/mod.cpp:395 | kSlowLines | RVA / layout / policy (audit) | 200 | constexpr uint64_t kSlowLines = 200; |
| N0247 | src/mod/net/coop.h:30 | kEchoOffset | RVA / layout / policy (audit) | 2.5f | static constexpr float kEchoOffset = 2.5f; |
| N0248 | src/mod/net/enemy_relay.h:20 | kEnemyCopyBase | RVA / layout / policy (audit) | 0xB01F0000 | inline constexpr uint32_t kEnemyCopyBase = 0xB01F0000;  // + net id (1..0xEFFF) |
| N0249 | src/mod/net/keys.h:15 | kClassUser | RVA / layout / policy (audit) | 1 | inline constexpr uint32_t kClassUser = 1; |
| N0250 | src/mod/net/keys.h:16 | kClassPlayer | RVA / layout / policy (audit) | 2 | inline constexpr uint32_t kClassPlayer = 2; |
| N0251 | src/mod/net/keys.h:17 | kClassWorld | RVA / layout / policy (audit) | 3 | inline constexpr uint32_t kClassWorld = 3; |
| N0252 | src/mod/net/keys.h:19 | kFirstModIndex | RVA / layout / policy (audit) | 0x8000 | inline constexpr uint32_t kFirstModIndex = 0x8000; |
| N0253 | src/mod/net/keys.h:23 | kMaxGeneration | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kMaxGeneration = 64; |
| N0254 | src/mod/net/remote_puppet.cpp:38 | kPuppetUser | RVA / layout / policy (audit) | 0x9010F001 | constexpr uint32_t kPuppetUser = 0x9010F001; |
| N0255 | src/mod/net/remote_puppet.cpp:39 | kPuppetPlayer | RVA / layout / policy (audit) | 0xA010F001 | constexpr uint32_t kPuppetPlayer = 0xA010F001; |
| N0256 | src/mod/net/remote_puppet.cpp:40 | kPuppetBody | RVA / layout / policy (audit) | 0xB010F001 | constexpr uint32_t kPuppetBody = 0xB010F001; |
| N0257 | src/mod/net/remote_puppet.cpp:43 | kPuppetHorse | RVA / layout / policy (audit) | 0xB010F002 | constexpr uint32_t kPuppetHorse = 0xB010F002; |
| N0258 | src/mod/net/remote_puppet.cpp:44 | kPuppetHorseParty | RVA / layout / policy (audit) | 0xB010F003 | constexpr uint32_t kPuppetHorseParty = 0xB010F003; |
| N0259 | src/mod/net/remote_puppet.cpp:51 | kSenderCharacterNo | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFFFFF | constexpr uint64_t kSenderCharacterNo = 0x7FFFFFFFFFFFFFFF; |
| N0260 | src/mod/net/remote_puppet.cpp:52 | kPuppetCharacterNo | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFF001 | constexpr uint64_t kPuppetCharacterNo = 0x7FFFFFFFFFFFF001; |
| N0261 | src/mod/net/remote_puppet.cpp:54 | kCharacterNoAt | signature | {0x17, 0x43} | constexpr size_t kCharacterNoAt[] = {0x17, 0x43}; |
| N0262 | src/mod/net/remote_puppet.cpp:192 | kStaleMovementMs | RVA / layout / policy (audit) | 2000 | constexpr uint64_t kStaleMovementMs = 2000; |
| N0263 | src/mod/net/remote_puppet.cpp:359 | kMoveSilenceMs | RVA / layout / policy (audit) | 150 | constexpr uint64_t kMoveSilenceMs = 150;  // real MoveActorReqs come every ~25 ms |
| N0264 | src/mod/net/remote_puppet.cpp:368 | kGapMinMs | RVA / layout / policy (audit) | 150 | constexpr uint64_t kGapMinMs = 150; |
| N0265 | src/mod/net/remote_puppet.cpp:369 | kGapMaxMs | RVA / layout / policy (audit) | 3000 | constexpr uint64_t kGapMaxMs = 3000; |
| N0266 | src/mod/net/remote_puppet.cpp:370 | kGapMinJump | RVA / layout / policy (audit) | 0.5f | constexpr float kGapMinJump = 0.5f;   // metres |
| N0267 | src/mod/net/remote_puppet.cpp:371 | kGapMaxJump | RVA / layout / policy (audit) | 15.0f | constexpr float kGapMaxJump = 15.0f;  // beyond this it's a real teleport (fast travel) |
| N0268 | src/mod/net/remote_puppet.cpp:372 | kGapStepMs | RVA / layout / policy (audit) | 50 | constexpr uint64_t kGapStepMs = 50; |
| N0269 | src/mod/net/remote_puppet.cpp:562 | kHeldTooLongMs | RVA / layout / policy (audit) | 3000 | constexpr uint64_t kHeldTooLongMs = 3000;  // companeros.md §C |
| N0270 | src/mod/net/remote_puppet.cpp:595 | kLayer_CurrentAction | RVA / layout / policy (audit) | 0x10 | constexpr uint32_t kLayer_CurrentAction = 0x10;  // horse_diag's cur(+10) |
| N0271 | src/mod/net/remote_puppet.cpp:1411 | kLead | RVA / layout / policy (audit) | 40 | constexpr uint64_t kLead = 40; |
| N0272 | src/mod/net/steam_transport.cpp:77 | kIdentitySteamID | RVA / layout / policy (audit) | 16 | constexpr int32_t kIdentitySteamID = 16; |
| N0273 | src/mod/net/steam_transport.cpp:78 | kSendUnreliableNoNagle | RVA / layout / policy (audit) | 1 | constexpr int kSendUnreliableNoNagle = 1; |
| N0274 | src/mod/net/steam_transport.cpp:79 | kSendAutoRestartBrokenSession | RVA / layout / policy (audit) | 32 | constexpr int kSendAutoRestartBrokenSession = 32; |
| N0275 | src/mod/net/steam_transport.cpp:80 | kFriendFlagImmediate | RVA / layout / policy (audit) | 0x04 | constexpr int kFriendFlagImmediate = 0x04; |
| N0276 | src/mod/net/steam_transport.cpp:81 | kResultOK | RVA / layout / policy (audit) | 1 | constexpr int kResultOK = 1; |
| N0277 | src/mod/net/steam_transport.cpp:82 | kResultLimitExceeded | RVA / layout / policy (audit) | 25 | constexpr int kResultLimitExceeded = 25; |
| N0278 | src/mod/net/steam_transport.cpp:83 | kSessionConnected | RVA / layout / policy (audit) | 3 | constexpr int kSessionConnected = 3; |
| N0279 | src/mod/net/steam_transport.cpp:89 | kConfigScopeGlobal | RVA / layout / policy (audit) | 1 | constexpr int32_t kConfigScopeGlobal = 1; |
| N0280 | src/mod/net/steam_transport.cpp:90 | kConfigTypeInt32 | RVA / layout / policy (audit) | 1 | constexpr int32_t kConfigTypeInt32 = 1; |
| N0281 | src/mod/net/steam_transport.cpp:91 | kGetConfigOK | RVA / layout / policy (audit) | 1 | constexpr int32_t kGetConfigOK = 1, kGetConfigOKInherited = 2; |
| N0282 | src/mod/net/steam_transport.cpp:91 | kGetConfigOKInherited | RVA / layout / policy (audit) | 2 | constexpr int32_t kGetConfigOK = 1, kGetConfigOKInherited = 2; |
| N0283 | src/mod/net/steam_transport.cpp:92 | kAppId | RVA / layout / policy (audit) | 3321460 | constexpr uint32_t kAppId = 3321460;  // Crimson Desert |
| N0284 | src/mod/net/steam_transport.cpp:93 | kPollEveryMs | RVA / layout / policy (audit) | 500 | constexpr uint32_t kPollEveryMs = 500; |
| N0285 | src/mod/net/steam_transport.cpp:94 | kReadyTimeoutMs | RVA / layout / policy (audit) | 120000 | constexpr uint32_t kReadyTimeoutMs = 120000; |
| N0286 | src/mod/net/steam_transport.cpp:568 | kBatch | RVA / layout / policy (audit) | 64 | constexpr int kBatch = 64; |
| N0287 | src/mod/net/steam_transport.h:78 | kSampleEveryMs | RVA / layout / policy (audit) | 20 | static constexpr uint32_t kSampleEveryMs = 20;     // Steam queue readings for the gate |
| N0288 | src/mod/net/steam_transport.h:79 | kHeldRetryMs | RVA / layout / policy (audit) | 10 | static constexpr uint32_t kHeldRetryMs = 10; |
| N0289 | src/mod/net/steam_transport.h:80 | kMaxHeldBytes | RVA / layout / policy (audit) | 1024 * 1024 | static constexpr size_t kMaxHeldBytes = 1024 * 1024; |
| N0290 | src/mod/net/troc_sniffer.cpp:40 | kRingBytes | RVA / layout / policy (audit) | 64ull * 1024 * 1024 | constexpr size_t kRingBytes = 64ull * 1024 * 1024; |
| N0291 | src/mod/net/troc_sniffer.cpp:41 | kMaxCopy | RVA / layout / policy (audit) | 32768 | constexpr uint16_t kMaxCopy = 32768;  // the game's packet buffers are 0x8000 bytes |
| N0292 | src/mod/net/troc_sniffer.cpp:42 | kPreviewBytes | RVA / layout / policy (audit) | 48 | constexpr size_t kPreviewBytes = 48; |
| N0293 | src/mod/net/troc_sniffer.cpp:60 | kRecModExtra | RVA / layout / policy (audit) | 1 | constexpr uint8_t kRecModExtra = 1;  // also the .bin record's pad8 (bit 0); readers that ignore it see the old format |
| N0294 | src/mod/net/troc_sniffer.cpp:77 | kExtraCap | RVA / layout / policy (audit) | 512 | constexpr size_t kExtraCap = 512;             // the largest extra packet (a 2697 is 240 bytes) |
| N0295 | src/mod/net/troc_sniffer.cpp:156 | kMaxC2SWaitMs | RVA / layout / policy (audit) | 3000 | constexpr uint64_t kMaxC2SWaitMs = 3000; |
| N0296 | src/mod/net/troc_sniffer.cpp:397 | kDiskTextBytes | RVA / layout / policy (audit) | 1u << 20 | constexpr size_t kDiskTextBytes = 1u << 20, kDiskBinBytes = 4u << 20; |
| N0297 | src/mod/net/troc_sniffer.cpp:397 | kDiskBinBytes | RVA / layout / policy (audit) | 4u << 20 | constexpr size_t kDiskTextBytes = 1u << 20, kDiskBinBytes = 4u << 20; |
| N0298 | src/mod/net/troc_sniffer.cpp:399 | kDiskReserve | RVA / layout / policy (audit) | 4096 | constexpr size_t kDiskReserve = 4096; |
| N0299 | src/mod/net/troc_sniffer.cpp:400 | kDiskPassBytes | RVA / layout / policy (audit) | 1u << 20 | constexpr size_t kDiskPassBytes = 1u << 20;  // one pass of the disk thread writes at most this per file |
| N0300 | src/mod/role_logic.h:24 | kEnvVar | RVA / layout / policy (audit) | L | inline constexpr wchar_t kEnvVar[] = L"CDMP_ROLE"; |
| N0301 | src/mod/role_logic.h:27 | kGuestSaveFolder | RVA / layout / policy (audit) | L | inline constexpr wchar_t kGuestSaveFolder[] = L"save_cdmp_guest"; |
| N0302 | src/mod/session2/child_hold.h:170 | kActorKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActorKey = 0x60;          // u32 (0x2AF9062) |
| N0303 | src/mod/session2/child_hold.h:171 | kActorComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActorComponents = 0x68;   // -> components |
| N0304 | src/mod/session2/child_hold.h:172 | kComponentsCc | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kComponentsCc = 0x40;      // -> CharacterControl (0x296CEFE) |
| N0305 | src/mod/session2/child_hold.h:173 | kComponentsTs | RVA / layout / policy (audit) | 0x1A0 | inline constexpr uint32_t kComponentsTs = 0x1A0;     // -> TransformSync (0x2BC5233) |
| N0306 | src/mod/session2/child_hold.h:174 | kActorInfo | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kActorInfo = 0x88;         // -> static info; u8 record type at +1 (0x296CE83) |
| N0307 | src/mod/session2/child_hold.h:175 | kInfoType | RVA / layout / policy (audit) | 0x01 | inline constexpr uint32_t kInfoType = 0x01; |
| N0308 | src/mod/session2/child_hold.h:176 | kActorOwner | RVA / layout / policy (audit) | 0xA0 | inline constexpr uint32_t kActorOwner = 0xA0;        // ServerUserActor* (vf160 = 0x50E6B0) |
| N0309 | src/mod/session2/child_hold.h:177 | kCcActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kCcActor = 0x08;           // CC -> its actor (0x2A78CCB) |
| N0310 | src/mod/session2/child_hold.h:178 | kCcRestore | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kCcRestore = 0x40;         // CC -> restore state (0x2A78DAF / 0x2328890) |
| N0311 | src/mod/session2/child_hold.h:179 | kRestoreByte | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kRestoreByte = 0x30;       // u8: 1 once the first action was accepted (0x2328894) |
| N0312 | src/mod/session2/child_hold.h:180 | kTsActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kTsActor = 0x08;           // TS -> its actor |
| N0313 | src/mod/session2/child_hold.h:181 | kTsSector | RVA / layout / policy (audit) | 0x540 | inline constexpr uint32_t kTsSector = 0x540;         // TS -> current sector (.xtls 0x10648425, read only) |
| N0314 | src/mod/session2/child_hold.h:182 | kSectorObservers | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kSectorObservers = 0x68;   // {ServerUserActor*, refs}[n] (0x2AF2F24) |
| N0315 | src/mod/session2/child_hold.h:183 | kSectorObserverCount | RVA / layout / policy (audit) | 0x70 | inline constexpr uint32_t kSectorObserverCount = 0x70; |
| N0316 | src/mod/session2/child_hold.h:184 | kObserverStride | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kObserverStride = 0x10; |
| N0317 | src/mod/session2/child_hold.h:185 | kMaxObservers | RVA / layout / policy (audit) | 256 | inline constexpr uint32_t kMaxObservers = 256; |
| N0318 | src/mod/session2/child_hold.h:186 | kUserKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kUserKey = 0x60; |
| N0319 | src/mod/session2/child_hold.h:187 | kChildKeyClass | RVA / layout / policy (audit) | 0xB | inline constexpr uint32_t kChildKeyClass = 0xB;      // key >> 28 of NPCs and children (users 0x9, characters 0xA) |
| N0320 | src/mod/session2/child_hold.h:188 | kTableCapacity | RVA / layout / policy (audit) | 64 | inline constexpr size_t kTableCapacity = 64; |
| N0321 | src/mod/session2/child_hold.h:189 | kTimeoutMs | RVA / layout / policy (audit) | 3000 | inline constexpr uint64_t kTimeoutMs = 3000; |
| N0322 | src/mod/session2/child_hold.h:191 | kActorController | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kActorController = 0x90;   // u32 controlling user key (0x2A78CF8 mov ecx,[rbx+0x90]) |
| N0323 | src/mod/session2/child_hold.h:200 | kNpcTableCapacity | RVA / layout / policy (audit) | 1024 | inline constexpr size_t kNpcTableCapacity = 1024; |
| N0324 | src/mod/session2/child_hold.h:203 | kNpcTimeoutMs | RVA / layout / policy (audit) | 10000 | inline constexpr uint64_t kNpcTimeoutMs = 10000; |
| N0325 | src/mod/session2/child_hold.h:207 | kLoadDeferMaxMs | RVA / layout / policy (audit) | 60000 | inline constexpr uint64_t kLoadDeferMaxMs = 60000; |
| N0326 | src/mod/session2/child_hold.h:209 | kLoadDeferLogged | RVA / layout / policy (audit) | 16 | inline constexpr uint64_t kLoadDeferLogged = 16; |
| N0327 | src/mod/session2/child_hold.h:212 | kNpcPassMs | RVA / layout / policy (audit) | 100 | inline constexpr uint64_t kNpcPassMs = 100; |
| N0328 | src/mod/session2/child_hold.h:213 | kNpcPassBudget | RVA / layout / policy (audit) | 256 | inline constexpr size_t kNpcPassBudget = 256; |
| N0329 | src/mod/session2/child_hold.h:215 | kNpcFallbackMs | RVA / layout / policy (audit) | 250 | inline constexpr uint64_t kNpcFallbackMs = 250; |
| N0330 | src/mod/session2/child_hold.h:220 | kReqWrapperSize | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kReqWrapperSize = 0x10; |
| N0331 | src/mod/session2/child_hold.h:221 | kReqWrapperRaw | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kReqWrapperRaw = 0x18; |
| N0332 | src/mod/session2/child_hold.h:222 | kReqRawLen | RVA / layout / policy (audit) | 0x03 | inline constexpr uint32_t kReqRawLen = 0x03; |
| N0333 | src/mod/session2/child_hold.h:223 | kReqRawPayload | RVA / layout / policy (audit) | 0x05 | inline constexpr uint32_t kReqRawPayload = 0x05; |
| N0334 | src/mod/session2/child_hold.h:225 | kCharKeyClass | RVA / layout / policy (audit) | 0xA | inline constexpr uint32_t kCharKeyClass = 0xA;       // key >> 28 of characters (users' Kliffs 0xA01...) |
| N0335 | src/mod/session2/child_hold.h:230 | kUserCharacter | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kUserCharacter = 0xD0; |
| N0336 | src/mod/session2/child_hold.h:232 | kCharTableCapacity | RVA / layout / policy (audit) | 16 | inline constexpr size_t kCharTableCapacity = 16; |
| N0337 | src/mod/session2/child_hold.h:236 | kCharTimeoutMs | RVA / layout / policy (audit) | 15000 | inline constexpr uint64_t kCharTimeoutMs = 15000; |
| N0338 | src/mod/session2/child_hold.h:237 | kCharPassMs | RVA / layout / policy (audit) | 50 | inline constexpr uint64_t kCharPassMs = 50;         // the passes at most this often (tail / fallback) |
| N0339 | src/mod/session2/child_hold.h:239 | kSettleTableCapacity | RVA / layout / policy (audit) | 32 | inline constexpr size_t kSettleTableCapacity = 32; |
| N0340 | src/mod/session2/child_hold.h:240 | kAnnounceLogSlots | RVA / layout / policy (audit) | 64 | inline constexpr size_t kAnnounceLogSlots = 64;     // (0xA key, session) -> last announcement |
| N0341 | src/mod/session2/child_hold.h:247 | kComponentsHierarchy | RVA / layout / policy (audit) | 0x1B8 | inline constexpr uint32_t kComponentsHierarchy = 0x1B8; |
| N0342 | src/mod/session2/child_hold.h:248 | kHierActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kHierActor = 0x08; |
| N0343 | src/mod/session2/child_hold.h:249 | kHierParent | RVA / layout / policy (audit) | 0x1C | inline constexpr uint32_t kHierParent = 0x1C; |
| N0344 | src/mod/session2/child_hold.h:253 | kFamilyTableCapacity | RVA / layout / policy (audit) | 1024 | inline constexpr size_t kFamilyTableCapacity = 1024; |
| N0345 | src/mod/session2/child_hold.h:256 | kFamilyMaxHoldMs | RVA / layout / policy (audit) | kLoadDeferMaxMs + 3 * kNpcTimeoutMs | inline constexpr uint64_t kFamilyMaxHoldMs = kLoadDeferMaxMs + 3 * kNpcTimeoutMs; |
| N0346 | src/mod/session2/child_hold.h:259 | kFamilyOrphanMs | RVA / layout / policy (audit) | kNpcTimeoutMs | inline constexpr uint64_t kFamilyOrphanMs = kNpcTimeoutMs; |
| N0347 | src/mod/session2/child_hold.h:261 | kFamilyKnownSlots | RVA / layout / policy (audit) | 16384 | inline constexpr size_t kFamilyKnownSlots = 16384; |
| N0348 | src/mod/session2/child_hold.h:266 | kActorParent | RVA / layout / policy (audit) | 0xAC | inline constexpr uint32_t kActorParent = 0xAC; |
| N0349 | src/mod/session2/child_hold.h:268 | kNativeThreads | RVA / layout / policy (audit) | 8 | inline constexpr size_t kNativeThreads = 8; |
| N0350 | src/mod/session2/child_hold.h:270 | kDeferWarnMs | RVA / layout / policy (audit) | 2000 | inline constexpr uint64_t kDeferWarnMs = 2000; |
| N0351 | src/mod/session2/clock_gate.cpp:21 | kTravel | RVA / layout / policy (audit) | 0x2BC0A30 | constexpr uintptr_t kTravel = 0x2BC0A30; |
| N0352 | src/mod/session2/clock_gate.cpp:22 | kTravelHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9} | constexpr uint8_t kTravelHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, |
| N0353 | src/mod/session2/clock_gate.cpp:25 | kTravelOwnerCmp | RVA / layout / policy (audit) | 0x2BC0A78 | constexpr uintptr_t kTravelOwnerCmp = 0x2BC0A78;  // cmp [rdx+0x180], eax |
| N0354 | src/mod/session2/clock_gate.cpp:26 | kTravelOwnerCmpBytes | signature | {0x39, 0x82, 0x80, 0x01, 0x00, 0x00} | constexpr uint8_t kTravelOwnerCmpBytes[] = {0x39, 0x82, 0x80, 0x01, 0x00, 0x00}; |
| N0355 | src/mod/session2/clock_gate.cpp:27 | kTravelGetterCall | RVA / layout / policy (audit) | 0x2BC0CC7 | constexpr uintptr_t kTravelGetterCall = 0x2BC0CC7;  // call 0x20E55A0 (non-owner branch) |
| N0356 | src/mod/session2/clock_gate.cpp:28 | kTravelGetterCallBytes | signature | {0xE8, 0xD4, 0x48, 0x52, 0xFF} | constexpr uint8_t kTravelGetterCallBytes[] = {0xE8, 0xD4, 0x48, 0x52, 0xFF}; |
| N0357 | src/mod/session2/clock_gate.cpp:29 | kLoading | RVA / layout / policy (audit) | 0x219D7F0 | constexpr uintptr_t kLoading = 0x219D7F0; |
| N0358 | src/mod/session2/clock_gate.cpp:30 | kLoadingHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x60, 0x4D, 0x8B, 0xE9, 0x49, 0x8B, 0xF0, 0x4C, 0x8B, 0xF2, 0x48, 0x8B, 0xD9} | constexpr uint8_t kLoadingHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, |
| N0359 | src/mod/session2/clock_gate.cpp:34 | kLoadingFlagCmp | RVA / layout / policy (audit) | 0x219D82B | constexpr uintptr_t kLoadingFlagCmp = 0x219D82B;  // cmp [rbp+0x50], r12b ; je 0x219D941 |
| N0360 | src/mod/session2/clock_gate.cpp:35 | kLoadingFlagCmpBytes | signature | {0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00} | constexpr uint8_t kLoadingFlagCmpBytes[] = {0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00}; |
| N0361 | src/mod/session2/clock_gate.cpp:36 | kGetter | RVA / layout / policy (audit) | 0x20E55A0 | constexpr uintptr_t kGetter = 0x20E55A0; |
| N0362 | src/mod/session2/clock_gate.cpp:37 | kGetterHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0xFA, 0x48, 0x8D, 0x54, 0x24, 0x70, 0x48, 0x8B, 0xD9, 0xFF, 0x50, 0x60} | constexpr uint8_t kGetterHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, |
| N0363 | src/mod/session2/clock_gate.cpp:39 | kMaxLogged | RVA / layout / policy (audit) | 24 | constexpr uint64_t kMaxLogged = 24; |
| N0364 | src/mod/session2/clock_gate.cpp:48 | kMaxWatchLogged | RVA / layout / policy (audit) | 60 | constexpr uint64_t kMaxWatchLogged = 60; |
| N0365 | src/mod/session2/clock_gate.h:41 | kTsActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kTsActor = 0x08;      // TS -> character |
| N0366 | src/mod/session2/clock_gate.h:42 | kActorUser | RVA / layout / policy (audit) | 0xA0 | inline constexpr uint32_t kActorUser = 0xA0;    // character -> ServerUserActor |
| N0367 | src/mod/session2/clock_gate.h:43 | kUserKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kUserKey = 0x60;      // ServerUserActor -> u32 user key (0x2BC0A70) |
| N0368 | src/mod/session2/clock_gate.h:44 | kTsFieldTime | RVA / layout / policy (audit) | 0x450 | inline constexpr uint32_t kTsFieldTime = 0x450; // 32-byte FieldTime of the character (setter 0xE52E914) |
| N0369 | src/mod/session2/clock_gate.h:45 | kFieldTimeSize | RVA / layout / policy (audit) | 32 | inline constexpr uint32_t kFieldTimeSize = 32; |
| N0370 | src/mod/session2/clock_gate.h:46 | kWorldTime | RVA / layout / policy (audit) | 0x6A6D238 | inline constexpr uintptr_t kWorldTime = 0x6A6D238;  // server FieldTime (exe+) |
| N0371 | src/mod/session2/clock_push_logic.h:27 | kOpLoadingCompleteAck | Troc ID (audit) | 2762 | inline constexpr uint16_t kOpLoadingCompleteAck = 2762; |
| N0372 | src/mod/session2/clock_push_logic.h:28 | kOpSetFieldTimeAck | Troc ID (audit) | 2816 | inline constexpr uint16_t kOpSetFieldTimeAck = 2816; |
| N0373 | src/mod/session2/clock_push_logic.h:29 | kOpSyncWeatherStateReq | Troc ID (audit) | 2560 | inline constexpr uint16_t kOpSyncWeatherStateReq = 2560; |
| N0374 | src/mod/session2/clock_push_logic.h:30 | kHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kHeader = 5;          // u16 opcode &#124; i8 slot (-1) &#124; u16 payload length |
| N0375 | src/mod/session2/clock_push_logic.h:31 | kFieldTimeMem | RVA / layout / policy (audit) | 32 | inline constexpr size_t kFieldTimeMem = 32; |
| N0376 | src/mod/session2/clock_push_logic.h:32 | kFieldTimeWire | RVA / layout / policy (audit) | 31 | inline constexpr size_t kFieldTimeWire = 31; |
| N0377 | src/mod/session2/clock_push_logic.h:33 | kAckPayload | RVA / layout / policy (audit) | 4 + kFieldTimeWire | inline constexpr size_t kAckPayload = 4 + kFieldTimeWire;   // 35 (0x23) |
| N0378 | src/mod/session2/clock_push_logic.h:34 | kAckPacket | RVA / layout / policy (audit) | kHeader + kAckPayload | inline constexpr size_t kAckPacket = kHeader + kAckPayload;  // 40 |
| N0379 | src/mod/session2/clock_push_logic.h:35 | kLoadingAckPayload | RVA / layout / policy (audit) | 13 | inline constexpr size_t kLoadingAckPayload = 13; |
| N0380 | src/mod/session2/clock_push_logic.h:38 | kServerClock | RVA / layout / policy (audit) | 0x6A6D238 | inline constexpr uintptr_t kServerClock = 0x6A6D238;   // server FieldTime (the host's world clock) |
| N0381 | src/mod/session2/clock_push_logic.h:39 | kClientClock | RVA / layout / policy (audit) | 0x6A6D218 | inline constexpr uintptr_t kClientClock = 0x6A6D218;   // client FieldTime (what this client's sky follows) |
| N0382 | src/mod/session2/clock_push_logic.h:40 | kWorldSystem | RVA / layout / policy (audit) | 0x6D69190 | inline constexpr uintptr_t kWorldSystem = 0x6D69190;   // [exe+] -> world system; +0x28 -> the time manager |
| N0383 | src/mod/session2/clock_push_logic.h:41 | kWorldTimeMgr | RVA / layout / policy (audit) | 0x28 | inline constexpr uint32_t kWorldTimeMgr = 0x28; |
| N0384 | src/mod/session2/clock_push_logic.h:42 | kMgrFlags | RVA / layout / policy (audit) | 0x28 | inline constexpr uint32_t kMgrFlags = 0x28;            // +0x28 active (set by the 2762), +0x29 private calendar, |
| N0385 | src/mod/session2/clock_push_logic.h:44 | kMgrPrivateTime | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kMgrPrivateTime = 0x30;      // FieldTime of the private calendar (+0x30..+0x4F) |
| N0386 | src/mod/session2/clock_push_logic.h:45 | kMgrDumpSize | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kMgrDumpSize = 0x30;         // +0x28 .. +0x57 |
| N0387 | src/mod/session2/clock_push_logic.h:46 | kClockBias | RVA / layout / policy (audit) | 0x6D69438 | inline constexpr uintptr_t kClockBias = 0x6D69438;     // i64: server tick - this client's FrameClock (2188, 0xBDF310) |
| N0388 | src/mod/session2/companion_probe.h:48 | kActorKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActorKey = 0x60;        // client actor -> u32 key |
| N0389 | src/mod/session2/companion_probe.h:49 | kActorInfo | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kActorInfo = 0x88;       // client actor -> static info; byte +1 == 1: main (0x206B3FA) |
| N0390 | src/mod/session2/companion_probe.h:50 | kActorOwnerUser | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kActorOwnerUser = 0x90;  // client actor -> u32 owning user key (offsets.h) |
| N0391 | src/mod/session2/companion_probe.h:51 | kActorState | RVA / layout / policy (audit) | 0x5E | inline constexpr uint32_t kActorState = 0x5E;      // u16 state flags (4 / 0x10 tested by 0x206B345 / 0x206B539) |
| N0392 | src/mod/session2/companion_probe.h:52 | kActorReady | RVA / layout / policy (audit) | 0x4A | inline constexpr uint32_t kActorReady = 0x4A;      // u8, tested next to it |
| N0393 | src/mod/session2/companion_probe.h:53 | kUserMain | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kUserMain = 0xD0;        // user actor -> its main character (0x206B413) |
| N0394 | src/mod/session2/companion_probe.h:54 | kCcActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kCcActor = 0x08;         // control component -> actor |
| N0395 | src/mod/session2/companion_probe.h:55 | kCcRestore | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kCcRestore = 0x40;       // CC -> restore data; its byte +0x30 (0x864743) |
| N0396 | src/mod/session2/companion_probe.h:56 | kRestoreByte | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kRestoreByte = 0x30; |
| N0397 | src/mod/session2/companion_probe.h:57 | kCcLayer0 | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kCcLayer0 = 0x88;        // CC -> layer 0, +0x90 layer 1 (0x232D51E) |
| N0398 | src/mod/session2/companion_probe.h:58 | kCcPending | RVA / layout / policy (audit) | 0xA8 | inline constexpr uint32_t kCcPending = 0xA8;       // u8 pending actions flag (horse_diag) |
| N0399 | src/mod/session2/companion_probe.h:59 | kLayerCurrent | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kLayerCurrent = 0x10;    // what 0x232D5C0 reads as [r8+0x10] |
| N0400 | src/mod/session2/companion_probe.h:60 | kLayerPlayback | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kLayerPlayback = 0x18; |
| N0401 | src/mod/session2/companion_probe.h:61 | kLayerChart | RVA / layout / policy (audit) | 0x110 | inline constexpr uint32_t kLayerChart = 0x110; |
| N0402 | src/mod/session2/companion_probe.h:68 | kRestoreSite | RVA / layout / policy (audit) | 0x8646F2 | inline constexpr uintptr_t kRestoreSite = 0x8646F2; |
| N0403 | src/mod/session2/companion_probe.h:69 | kRestoreSiteBytes | signature | {0xC5, 0xF8, 0x29, 0x70, 0xB8, 0xC5, 0xF8, 0x29, 0x78, 0xA8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xE9} | inline constexpr uint8_t kRestoreSiteBytes[] = {0xC5, 0xF8, 0x29, 0x70, 0xB8, 0xC5, 0xF8, 0x29, |
| N0404 | src/mod/session2/companion_probe.h:71 | kRestoreHook | RVA / layout / policy (audit) | kRestoreSite + 10 | inline constexpr uintptr_t kRestoreHook = kRestoreSite + 10;  // 0x8646FC |
| N0405 | src/mod/session2/companion_probe.h:73 | kRestoreLock | RVA / layout / policy (audit) | 0x86473F | inline constexpr uintptr_t kRestoreLock = 0x86473F; |
| N0406 | src/mod/session2/companion_probe.h:74 | kRestoreLockBytes | signature | {0x49, 0x8B, 0x45, 0x40, 0x44, 0x38, 0x70, 0x30, 0x75, 0x59} | inline constexpr uint8_t kRestoreLockBytes[] = {0x49, 0x8B, 0x45, 0x40, 0x44, 0x38, 0x70, 0x30, 0x75, 0x59}; |
| N0407 | src/mod/session2/companion_probe.h:78 | kRestoreExit | RVA / layout / policy (audit) | 0x864B1A | inline constexpr uintptr_t kRestoreExit = 0x864B1A; |
| N0408 | src/mod/session2/companion_probe.h:79 | kRestoreExitBytes | signature | {0x44, 0x89, 0x36, 0x48, 0x8B, 0xC6, 0x4C, 0x8D, 0x9C, 0x24, 0x60, 0x01, 0x00, 0x00} | inline constexpr uint8_t kRestoreExitBytes[] = {0x44, 0x89, 0x36, 0x48, 0x8B, 0xC6, 0x4C, |
| N0409 | src/mod/session2/companion_probe.h:81 | kRestoreExitHook | RVA / layout / policy (audit) | kRestoreExit + 3 | inline constexpr uintptr_t kRestoreExitHook = kRestoreExit + 3;  // 0x864B1D |
| N0410 | src/mod/session2/companion_probe.h:85 | kRestartCall | RVA / layout / policy (audit) | 0x864AEA | inline constexpr uintptr_t kRestartCall = 0x864AEA; |
| N0411 | src/mod/session2/companion_probe.h:86 | kRestartCallBytes | signature | {0x4C, 0x8D, 0x44, 0x24, 0x50, 0x48, 0x8D, 0x95, 0xA0, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xCD, 0xE8, 0x82, 0xED, 0xFF, 0xFF, 0x8B, 0x00, 0x85, 0xC0, 0x74, 0x04, 0x89, 0x06} | inline constexpr uint8_t kRestartCallBytes[] = {0x4C, 0x8D, 0x44, 0x24, 0x50, 0x48, 0x8D, 0x95, 0xA0, 0x00, |
| N0412 | src/mod/session2/companion_probe.h:92 | kLayerRestoreHash | RVA / layout / policy (audit) | 0x3C | inline constexpr uint32_t kLayerRestoreHash = 0x3C; |
| N0413 | src/mod/session2/companion_probe.h:93 | kCcDefaultChart0 | RVA / layout / policy (audit) | 0x70 | inline constexpr uint32_t kCcDefaultChart0 = 0x70;  // +0x78 for layer 1 |
| N0414 | src/mod/session2/fast_travel_logic.h:72 | kOpReloadStageStartReq | Troc ID (audit) | 2283 | inline constexpr uint16_t kOpReloadStageStartReq = 2283; |
| N0415 | src/mod/session2/fast_travel_logic.h:73 | kReloadPayload | RVA / layout / policy (audit) | 53 | inline constexpr size_t kReloadPayload = 53; |
| N0416 | src/mod/session2/fast_travel_logic.h:168 | kActorComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActorComponents = 0x68; |
| N0417 | src/mod/session2/fast_travel_logic.h:169 | kComponentsSequencer | RVA / layout / policy (audit) | 0xA8 | inline constexpr uint32_t kComponentsSequencer = 0xA8; |
| N0418 | src/mod/session2/fast_travel_logic.h:170 | kSeqComponentManager | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kSeqComponentManager = 0x18; |
| N0419 | src/mod/session2/fast_travel_logic.h:171 | kManagerActor | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kManagerActor = 0x18; |
| N0420 | src/mod/session2/fast_travel_logic.h:174 | kManagerLockObject | RVA / layout / policy (audit) | 0x8 | inline constexpr uint32_t kManagerLockObject = 0x8; |
| N0421 | src/mod/session2/fast_travel_logic.h:211 | kReloadVf48Ret | RVA / layout / policy (audit) | 0x2A5F7FC | inline constexpr uint32_t kReloadVf48Ret = 0x2A5F7FC; |
| N0422 | src/mod/session2/fast_travel_logic.h:218 | kApplyVf48Ret | RVA / layout / policy (audit) | 0x2BC4C53 | inline constexpr uint32_t kApplyVf48Ret = 0x2BC4C53; |
| N0423 | src/mod/session2/fast_travel_logic.h:223 | kSubstituteBlock | RVA / layout / policy (audit) | 0x40 | inline constexpr size_t kSubstituteBlock = 0x40; |
| N0424 | src/mod/session2/fast_travel_logic.h:289 | kOpReviveLoadingReq | Troc ID (audit) | 2362 | inline constexpr uint16_t kOpReviveLoadingReq = 2362; |
| N0425 | src/mod/session2/field_share.cpp:20 | kAcquireField | RVA / layout / policy (audit) | 0x2B07790 | constexpr uintptr_t kAcquireField = 0x2B07790; |
| N0426 | src/mod/session2/field_share.cpp:21 | kAcquireFieldHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70} | constexpr uint8_t kAcquireFieldHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, |
| N0427 | src/mod/session2/field_share.cpp:24 | kEnterField | RVA / layout / policy (audit) | 0x2BC04F0 | constexpr uintptr_t kEnterField = 0x2BC04F0; |
| N0428 | src/mod/session2/field_share.cpp:25 | kEnterFieldHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC} | constexpr uint8_t kEnterFieldHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, |
| N0429 | src/mod/session2/field_share.cpp:28 | kTryAddRef | RVA / layout / policy (audit) | 0x14343A0 | constexpr uintptr_t kTryAddRef = 0x14343A0; |
| N0430 | src/mod/session2/field_share.cpp:29 | kTryAddRefHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9} | constexpr uint8_t kTryAddRefHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, |
| N0431 | src/mod/session2/field_share.cpp:31 | kVecPush | RVA / layout / policy (audit) | 0x40B5B0 | constexpr uintptr_t kVecPush = 0x40B5B0; |
| N0432 | src/mod/session2/field_share.cpp:32 | kVecPushHead | signature | {0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, 0x4C, 0x8B, 0xF2, 0x8B, 0x49, 0x08, 0x8B, 0x43, 0x0C} | constexpr uint8_t kVecPushHead[] = {0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, |
| N0433 | src/mod/session2/field_share.cpp:35 | kUserMainCharacter | RVA / layout / policy (audit) | 0xD0 | constexpr uint32_t kUserMainCharacter = 0xD0; |
| N0434 | src/mod/session2/field_share.cpp:36 | kUserKey | RVA / layout / policy (audit) | 0x60 | constexpr uint32_t kUserKey = 0x60; |
| N0435 | src/mod/session2/field_share.cpp:37 | kActorUser | RVA / layout / policy (audit) | 0xA0 | constexpr uint32_t kActorUser = 0xA0;  // actor -> ServerUserActor (AddActor 0x1064C78F) |
| N0436 | src/mod/session2/field_share.cpp:38 | kMaxLogged | RVA / layout / policy (audit) | 24 | constexpr uint64_t kMaxLogged = 24; |
| N0437 | src/mod/session2/field_share.h:41 | kRefs | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kRefs = 0x10;          // u32 strong references |
| N0438 | src/mod/session2/field_share.h:42 | kLock1 | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kLock1 = 0x18;         // lock object (vtable: +8 lock, +0x10 unlock) |
| N0439 | src/mod/session2/field_share.h:43 | kLock2Ptr | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kLock2Ptr = 0x08;      // -> lock object |
| N0440 | src/mod/session2/field_share.h:44 | kOpen | Troc ID (audit) | 0x4A | inline constexpr uint32_t kOpen = 0x4A;          // u8 open |
| N0441 | src/mod/session2/field_share.h:45 | kOwner | RVA / layout / policy (audit) | 0x180 | inline constexpr uint32_t kOwner = 0x180;        // u32 owner user key |
| N0442 | src/mod/session2/field_share.h:46 | kUsers | RVA / layout / policy (audit) | 0x2A0 | inline constexpr uint32_t kUsers = 0x2A0;        // FieldUserEntry* |
| N0443 | src/mod/session2/field_share.h:47 | kUserCount | RVA / layout / policy (audit) | 0x2A8 | inline constexpr uint32_t kUserCount = 0x2A8;    // u32 |
| N0444 | src/mod/session2/field_share.h:48 | kUserActors | RVA / layout / policy (audit) | 0x2B0 | inline constexpr uint32_t kUserActors = 0x2B0;   // u32 |
| N0445 | src/mod/session2/field_share.h:49 | kKey | RVA / layout / policy (audit) | 0x2C8 | inline constexpr uint32_t kKey = 0x2C8;          // u32 field key (index, pool, generation) |
| N0446 | src/mod/session2/field_share.h:50 | kLevelEntry | RVA / layout / policy (audit) | 0x2D0 | inline constexpr uint32_t kLevelEntry = 0x2D0;   // -> level entry; u16 static level index at +4 |
| N0447 | src/mod/session2/field_share.h:52 | kTsFieldMover | RVA / layout / policy (audit) | 0x4F0 | inline constexpr uint32_t kTsFieldMover = 0x4F0; |
| N0448 | src/mod/session2/field_share.h:53 | kFmFieldKey | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kFmFieldKey = 0x08;    // u32 copy of field+0x2C8 |
| N0449 | src/mod/session2/field_share.h:54 | kFmField | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kFmField = 0x10;       // ServerField* |
| N0450 | src/mod/session2/field_share.h:55 | kFmBusy | RVA / layout / policy (audit) | 0x5A | inline constexpr uint32_t kFmBusy = 0x5A;        // u16 accesses in progress |
| N0451 | src/mod/session2/field_share.h:58 | kOutField | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kOutField = 0x08; |
| N0452 | src/mod/session2/field_share.h:59 | kOutValid | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kOutValid = 0x10;      // u8 |
| N0453 | src/mod/session2/field_share.h:60 | kOutHolder | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kOutHolder = 0x18; |
| N0454 | src/mod/session2/field_share.h:61 | kOutNew | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kOutNew = 0x20;        // u8 "newly created": 1 closes the field on failure (0x2BC006A) |
| N0455 | src/mod/session2/field_share.h:62 | kOutInfoKey | RVA / layout / policy (audit) | 0x24 | inline constexpr uint32_t kOutInfoKey = 0x24;    // u32 FieldInfoKey of the destination (0x2B077C2) |
| N0456 | src/mod/session2/field_share.h:66 | kMgrBucketCount | RVA / layout / policy (audit) | 0x48 | inline constexpr uint32_t kMgrBucketCount = 0x48; |
| N0457 | src/mod/session2/field_share.h:67 | kMgrEntries | RVA / layout / policy (audit) | 0x4C | inline constexpr uint32_t kMgrEntries = 0x4C; |
| N0458 | src/mod/session2/field_share.h:68 | kMgrBuckets | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kMgrBuckets = 0x58; |
| N0459 | src/mod/session2/field_share.h:69 | kMgrHolders | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kMgrHolders = 0x60; |
| N0460 | src/mod/session2/funcnpc_trade_policy.h:19 | kResources | signature | {{ { , 0xA7132561u}, { , 0x978345CAu}, }} | inline constexpr std::array<Resource, 2> kResources = {{ |
| N0461 | src/mod/session2/funcnpc_trade_policy.h:46 | kDictSeed | RVA / layout / policy (audit) | 0x000C5EDEu | inline constexpr uint32_t kDictSeed = 0x000C5EDEu; |
| N0462 | src/mod/session2/guest.cpp:71 | kOpConnectCryptogramToServerReq | Troc ID (audit) | 2319 | constexpr uint16_t kOpConnectCryptogramToServerReq = 2319; |
| N0463 | src/mod/session2/guest.cpp:72 | kNoEpoch | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFFFFFull | constexpr uint64_t kNoEpoch = 0x7FFFFFFFFFFFFFFFull;  // what PostPacket writes without a session |
| N0464 | src/mod/session2/guest.cpp:73 | kHostReadyFreshMs | RVA / layout / policy (audit) | 3000 | constexpr uint32_t kHostReadyFreshMs = 3000; |
| N0465 | src/mod/session2/guest.cpp:76 | kOpScopeSenderAck | Troc ID (audit) | 2042 | constexpr uint16_t kOpScopeSenderAck = 2042; |
| N0466 | src/mod/session2/guest.cpp:77 | kOpActorAppear | Troc ID (audit) | 2005 | constexpr uint16_t kOpActorAppear = 2005; |
| N0467 | src/mod/session2/guest.cpp:78 | kOpDataAck | Troc ID (audit) | 2869 | constexpr uint16_t kOpDataAck = 2869; |
| N0468 | src/mod/session2/guest.cpp:79 | kOp2679 | Troc ID (audit) | 2679 | constexpr uint16_t kOp2679 = 2679; |
| N0469 | src/mod/session2/guest.cpp:80 | kOpLoginCharacterToFieldServerReq | Troc ID (audit) | 3093 | constexpr uint16_t kOpLoginCharacterToFieldServerReq = 3093;  // u8 &#124; u64 characterNo @1 &#124; u32 slot @9 |
| N0470 | src/mod/session2/guest.cpp:81 | kOpReserveShutDownServerReq | Troc ID (audit) | 2790 | constexpr uint16_t kOpReserveShutDownServerReq = 2790; |
| N0471 | src/mod/session2/guest.cpp:84 | kServerFrameworkMode | RVA / layout / policy (audit) | 0x6A65564 | constexpr uintptr_t kServerFrameworkMode = 0x6A65564; |
| N0472 | src/mod/session2/guest.cpp:85 | kMaxDecisionLogged | RVA / layout / policy (audit) | 32 | constexpr uint64_t kMaxDecisionLogged = 32, kMaxInternalLogged = 16, kMaxLoginLogged = 8; |
| N0473 | src/mod/session2/guest.cpp:85 | kMaxInternalLogged | RVA / layout / policy (audit) | 16 | constexpr uint64_t kMaxDecisionLogged = 32, kMaxInternalLogged = 16, kMaxLoginLogged = 8; |
| N0474 | src/mod/session2/guest.cpp:85 | kMaxLoginLogged | RVA / layout / policy (audit) | 8 | constexpr uint64_t kMaxDecisionLogged = 32, kMaxInternalLogged = 16, kMaxLoginLogged = 8; |
| N0475 | src/mod/session2/guest.cpp:125 | kRjLines | RVA / layout / policy (audit) | 40 | constexpr uint64_t kRjLines = 40; |
| N0476 | src/mod/session2/guest.cpp:195 | kGoodbyeExitCode | RVA / layout / policy (audit) | 0x2001 | constexpr UINT kGoodbyeExitCode = 0x2001;    // the process's exit code when it ends here |
| N0477 | src/mod/session2/guest.cpp:205 | kLocalAfterRemoteExitCode | RVA / layout / policy (audit) | 0x2003 | constexpr UINT kLocalAfterRemoteExitCode = 0x2003; |
| N0478 | src/mod/session2/guest.cpp:214 | kOpLoginUserToFieldServerReq | Troc ID (audit) | 2490 | constexpr uint16_t kOpLoginUserToFieldServerReq = 2490; |
| N0479 | src/mod/session2/guest.cpp:215 | kSlotTestKey | RVA / layout / policy (audit) | ~0ull | constexpr uint64_t kSlotTestKey = ~0ull;  // the diagnostic's own key (never an epoch PostPacket writes) |
| N0480 | src/mod/session2/guest.cpp:1812 | kReanLines | RVA / layout / policy (audit) | 32 | constexpr uint64_t kReanLines = 32; |
| N0481 | src/mod/session2/guest_actor_diag_logic.h:281 | kUsed | RVA / layout / policy (audit) | 0x8C | static constexpr uint32_t kUsed = 0x8C, kBuckets = 0x88, kBucketArray = 0x98, kEntries = 0xA0; |
| N0482 | src/mod/session2/guest_actor_diag_logic.h:281 | kBuckets | RVA / layout / policy (audit) | 0x88 | static constexpr uint32_t kUsed = 0x8C, kBuckets = 0x88, kBucketArray = 0x98, kEntries = 0xA0; |
| N0483 | src/mod/session2/guest_actor_diag_logic.h:281 | kBucketArray | RVA / layout / policy (audit) | 0x98 | static constexpr uint32_t kUsed = 0x8C, kBuckets = 0x88, kBucketArray = 0x98, kEntries = 0xA0; |
| N0484 | src/mod/session2/guest_actor_diag_logic.h:281 | kEntries | RVA / layout / policy (audit) | 0xA0 | static constexpr uint32_t kUsed = 0x8C, kBuckets = 0x88, kBucketArray = 0x98, kEntries = 0xA0; |
| N0485 | src/mod/session2/guest_actor_diag_logic.h:282 | kBucketSize | RVA / layout / policy (audit) | 0x100 | static constexpr uint32_t kBucketSize = 0x100, kBucketMax = 31; |
| N0486 | src/mod/session2/guest_actor_diag_logic.h:282 | kBucketMax | RVA / layout / policy (audit) | 31 | static constexpr uint32_t kBucketSize = 0x100, kBucketMax = 31; |
| N0487 | src/mod/session2/guest_actor_diag_logic.h:283 | kEntryKey | RVA / layout / policy (audit) | 4 | static constexpr uint32_t kEntryKey = 4, kEntryActor = 8; |
| N0488 | src/mod/session2/guest_actor_diag_logic.h:283 | kEntryActor | RVA / layout / policy (audit) | 8 | static constexpr uint32_t kEntryKey = 4, kEntryActor = 8; |
| N0489 | src/mod/session2/guest_goodbye_logic.h:32 | kAnswerMs | RVA / layout / policy (audit) | 5000 | constexpr uint64_t kAnswerMs = 5000;    // the host answered in 0.16-0.41 s live |
| N0490 | src/mod/session2/guest_goodbye_logic.h:33 | kSilentMs | RVA / layout / policy (audit) | 5000 | constexpr uint64_t kSilentMs = 5000;    // with a field: nothing at all from the host for this long |
| N0491 | src/mod/session2/guest_goodbye_logic.h:34 | kFieldCapMs | RVA / layout / policy (audit) | 30000 | constexpr uint64_t kFieldCapMs = 30000; // with a field: no 2002 this long after the goodbye, whatever came |
| N0492 | src/mod/session2/guest_inbound_logic.h:54 | kMaxPerTurn | RVA / layout / policy (audit) | 4096 | inline constexpr size_t kMaxPerTurn = 4096; |
| N0493 | src/mod/session2/guest_lifecycle.cpp:16 | kMinPtr | RVA / layout / policy (audit) | 0x10000 | constexpr uintptr_t kMinPtr = 0x10000; |
| N0494 | src/mod/session2/guest_lifecycle.cpp:17 | kMaxPtr | RVA / layout / policy (audit) | 0x00007FFFFFFFFFFFull | constexpr uintptr_t kMaxPtr = 0x00007FFFFFFFFFFFull; |
| N0495 | src/mod/session2/guest_lifecycle.cpp:473 | kTagHigh | RVA / layout / policy (audit) | 0xD0D0 | constexpr uint32_t kTagHigh = 0xD0D0; |
| N0496 | src/mod/session2/guest_lifecycle.h:50 | kSamMapBuckets | RVA / layout / policy (audit) | 0x98 | inline constexpr uint32_t kSamMapBuckets = 0x98;   // u32 bucket count |
| N0497 | src/mod/session2/guest_lifecycle.h:51 | kSamMapSize | RVA / layout / policy (audit) | 0x9C | inline constexpr uint32_t kSamMapSize = 0x9C;      // u32 entries (offsets_s2.h kSam_ActorCount) |
| N0498 | src/mod/session2/guest_lifecycle.h:52 | kSamMapBucketBase | RVA / layout / policy (audit) | 0xA8 | inline constexpr uint32_t kSamMapBucketBase = 0xA8; // -> buckets of 0x100 B: u32 n, then n x {u32 index, u32 slot} |
| N0499 | src/mod/session2/guest_lifecycle.h:53 | kSamMapHolders | RVA / layout / policy (audit) | 0xB0 | inline constexpr uint32_t kSamMapHolders = 0xB0;   // -> holder*[slot]; holder: u32 index +4, actor* +8 |
| N0500 | src/mod/session2/guest_lifecycle.h:54 | kSamBucketBytes | RVA / layout / policy (audit) | 0x100 | inline constexpr uint32_t kSamBucketBytes = 0x100; |
| N0501 | src/mod/session2/guest_lifecycle.h:55 | kSamBucketMaxEntries | RVA / layout / policy (audit) | (kSamBucketBytes - 8) / 8 | inline constexpr uint32_t kSamBucketMaxEntries = (kSamBucketBytes - 8) / 8;  // 31 |
| N0502 | src/mod/session2/guest_lifecycle.h:56 | kSamKeyIndexMask | RVA / layout / policy (audit) | 0xFFFFF | inline constexpr uint32_t kSamKeyIndexMask = 0xFFFFF; |
| N0503 | src/mod/session2/guest_lifecycle.h:58 | kSamUserList | RVA / layout / policy (audit) | 0xB8 | inline constexpr uint32_t kSamUserList = 0xB8; |
| N0504 | src/mod/session2/guest_lifecycle.h:59 | kSamUserCount | RVA / layout / policy (audit) | 0xC0 | inline constexpr uint32_t kSamUserCount = 0xC0; |
| N0505 | src/mod/session2/guest_lifecycle.h:62 | kSamUserListMax | RVA / layout / policy (audit) | 0x4000 | inline constexpr uint32_t kSamUserListMax = 0x4000; |
| N0506 | src/mod/session2/guest_lifecycle.h:64 | kActorKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActorKey = 0x60; |
| N0507 | src/mod/session2/guest_lifecycle.h:68 | kUserMainCharacter | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kUserMainCharacter = 0xD0; |
| N0508 | src/mod/session2/guest_lifecycle.h:69 | kUserComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kUserComponents = 0x68; |
| N0509 | src/mod/session2/guest_lifecycle.h:70 | kComponentsSlots | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kComponentsSlots = 0x18; |
| N0510 | src/mod/session2/guest_lifecycle.h:71 | kSlotsArray | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kSlotsArray = 0x18; |
| N0511 | src/mod/session2/guest_lifecycle.h:72 | kSlotsCount | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kSlotsCount = 0x20; |
| N0512 | src/mod/session2/guest_lifecycle.h:73 | kSlotStride | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kSlotStride = 0x10; |
| N0513 | src/mod/session2/guest_lifecycle.h:185 | kMaxDepartedKeys | RVA / layout / policy (audit) | 160 | inline constexpr size_t kMaxDepartedKeys = 160; |
| N0514 | src/mod/session2/guest_lifecycle.h:196 | kActorState | RVA / layout / policy (audit) | 0x5E | inline constexpr uint32_t kActorState = 0x5E;           // u16: 0x20 logging out, 0x40 released (0xE0D9AF0) |
| N0515 | src/mod/session2/guest_lifecycle.h:197 | kActorComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActorComponents = 0x68;      // [pj+0x68] |
| N0516 | src/mod/session2/guest_lifecycle.h:198 | kComponentsTransformSync | RVA / layout / policy (audit) | 0x1A0 | inline constexpr uint32_t kComponentsTransformSync = 0x1A0;  // TS = [[pj+0x68]+0x1A0] |
| N0517 | src/mod/session2/guest_lifecycle.h:199 | kTsMoving | RVA / layout / policy (audit) | 0x3DC | inline constexpr uint32_t kTsMoving = 0x3DC;            // u8: 1 from MoveFieldOnceTimer to MoveFieldComplete |
| N0518 | src/mod/session2/guest_lifecycle.h:200 | kFmMoving | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kFmMoving = 0x58;             // u8 at TS+0x4F0+0x58: the move is open (GetField refuses) |
| N0519 | src/mod/session2/guest_lifecycle.h:215 | kLoginCharacterReq | RVA / layout / policy (audit) | 3093 | inline constexpr uint16_t kLoginCharacterReq = 3093;      // LoginCharacterToFieldServerReq |
| N0520 | src/mod/session2/guest_lifecycle.h:216 | kGameLoadingStartReq | RVA / layout / policy (audit) | 2437 | inline constexpr uint16_t kGameLoadingStartReq = 2437; |
| N0521 | src/mod/session2/guest_lifecycle.h:217 | kMoveFieldCompleteReq | RVA / layout / policy (audit) | 2549 | inline constexpr uint16_t kMoveFieldCompleteReq = 2549; |
| N0522 | src/mod/session2/guest_lifecycle.h:218 | kLoadingCompleteReq | RVA / layout / policy (audit) | 2445 | inline constexpr uint16_t kLoadingCompleteReq = 2445; |
| N0523 | src/mod/session2/guest_lifecycle.h:219 | kMoveFieldAck | RVA / layout / policy (audit) | 2174 | inline constexpr uint16_t kMoveFieldAck = 2174; |
| N0524 | src/mod/session2/guest_lifecycle.h:220 | kMoveFieldCompleteAck | RVA / layout / policy (audit) | 2144 | inline constexpr uint16_t kMoveFieldCompleteAck = 2144; |
| N0525 | src/mod/session2/guest_lifecycle.h:221 | kLoadingCompleteAck | RVA / layout / policy (audit) | 2762 | inline constexpr uint16_t kLoadingCompleteAck = 2762; |
| N0526 | src/mod/session2/guest_reload_wait.cpp:35 | kWaitHead | RVA / layout / policy (audit) | 0x779E8B | constexpr uintptr_t kWaitHead = 0x779E8B; |
| N0527 | src/mod/session2/guest_reload_wait.cpp:36 | kWaitHeadBytes | signature | {0x4C, 0x8B, 0xB7, 0xB8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x5E, 0x10, 0x48, 0x89, 0x5D, 0x40, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x49, 0x8B, 0x76, 0x08, 0x8B, 0x86, 0x80, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x1E} | constexpr uint8_t kWaitHeadBytes[] = {0x4C, 0x8B, 0xB7, 0xB8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x5E, 0x10, 0x48, 0x89, |
| N0528 | src/mod/session2/guest_reload_wait.cpp:40 | kWaitLoop | RVA / layout / policy (audit) | 0x779ED0 | constexpr uintptr_t kWaitLoop = 0x779ED0; |
| N0529 | src/mod/session2/guest_reload_wait.cpp:41 | kWaitLoopBytes | signature | {0x48, 0x8B, 0x56, 0x78, 0x8B, 0x12, 0x49, 0x8B, 0x4E, 0x08, 0xE8, 0xB1, 0xA3, 0xCE, 0xFF, 0x48, 0x85, 0xC0, 0x74, 0x13, 0x48, 0x8B, 0x08, 0x48, 0x8B, 0x91, 0xD8, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xC8, 0xFF, 0xD2, 0x84, 0xC0, 0x74, 0x1D} | constexpr uint8_t kWaitLoopBytes[] = {0x48, 0x8B, 0x56, 0x78, 0x8B, 0x12, 0x49, 0x8B, 0x4E, 0x08, 0xE8, 0xB1, 0xA3, |
| N0530 | src/mod/session2/guest_reload_wait.cpp:45 | kWaitRemove | RVA / layout / policy (audit) | 0x779EF7 | constexpr uintptr_t kWaitRemove = 0x779EF7; |
| N0531 | src/mod/session2/guest_reload_wait.cpp:46 | kWaitRemoveBytes | signature | {0x8B, 0x86, 0x80, 0x00, 0x00, 0x00, 0x83, 0xE8, 0x01, 0x74, 0x09, 0x48, 0x8B, 0x56, 0x78, 0x8B, 0x0C, 0x82, 0x89, 0x0A, 0x83, 0x86, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x75, 0xBC, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10} | constexpr uint8_t kWaitRemoveBytes[] = {0x8B, 0x86, 0x80, 0x00, 0x00, 0x00, 0x83, 0xE8, 0x01, 0x74, 0x09, 0x48, 0x8B, |
| N0532 | src/mod/session2/guest_reload_wait.cpp:50 | kReady | RVA / layout / policy (audit) | 0x4FE240 | constexpr uintptr_t kReady = 0x4FE240; |
| N0533 | src/mod/session2/guest_reload_wait.cpp:51 | kReadyBytes | signature | {0x80, 0xB9, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x75, 0x03, 0x32, 0xC0, 0xC3, 0x0F, 0xB6, 0x81, 0xCF, 0x00, 0x00, 0x00, 0xC3} | constexpr uint8_t kReadyBytes[] = {0x80, 0xB9, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x75, 0x03, 0x32, |
| N0534 | src/mod/session2/guest_reload_wait.cpp:54 | kFind | RVA / layout / policy (audit) | 0x464290 | constexpr uintptr_t kFind = 0x464290; |
| N0535 | src/mod/session2/guest_reload_wait.cpp:55 | kFindBytes | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x83, 0xB9, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x44, 0x8B, 0xC2, 0x4C, 0x8B, 0xD1, 0x74, 0x4F, 0x8B, 0x89, 0x88, 0x00, 0x00, 0x00} | constexpr uint8_t kFindBytes[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x83, 0xB9, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x44, |
| N0536 | src/mod/session2/guest_reload_wait.cpp:58 | kP_State | RVA / layout / policy (audit) | 0x15C | constexpr uint32_t kP_State = 0x15C, kP_Flags = 0x160, kP_290 = 0x290, kP_Manager = 0xB8; |
| N0537 | src/mod/session2/guest_reload_wait.cpp:58 | kP_Flags | RVA / layout / policy (audit) | 0x160 | constexpr uint32_t kP_State = 0x15C, kP_Flags = 0x160, kP_290 = 0x290, kP_Manager = 0xB8; |
| N0538 | src/mod/session2/guest_reload_wait.cpp:58 | kP_290 | RVA / layout / policy (audit) | 0x290 | constexpr uint32_t kP_State = 0x15C, kP_Flags = 0x160, kP_290 = 0x290, kP_Manager = 0xB8; |
| N0539 | src/mod/session2/guest_reload_wait.cpp:58 | kP_Manager | RVA / layout / policy (audit) | 0xB8 | constexpr uint32_t kP_State = 0x15C, kP_Flags = 0x160, kP_290 = 0x290, kP_Manager = 0xB8; |
| N0540 | src/mod/session2/guest_reload_wait.cpp:59 | kM_Lock | RVA / layout / policy (audit) | 0x10 | constexpr uint32_t kM_Lock = 0x10, kM_Container = 0x08; |
| N0541 | src/mod/session2/guest_reload_wait.cpp:59 | kM_Container | RVA / layout / policy (audit) | 0x08 | constexpr uint32_t kM_Lock = 0x10, kM_Container = 0x08; |
| N0542 | src/mod/session2/guest_reload_wait.cpp:60 | kLock_Acquire | RVA / layout / policy (audit) | 0x08 | constexpr uint32_t kLock_Acquire = 0x08, kLock_Release = 0x10;  // the lock's vf1 / vf2 (0x779EA0 / 0x779F1A) |
| N0543 | src/mod/session2/guest_reload_wait.cpp:60 | kLock_Release | RVA / layout / policy (audit) | 0x10 | constexpr uint32_t kLock_Acquire = 0x08, kLock_Release = 0x10;  // the lock's vf1 / vf2 (0x779EA0 / 0x779F1A) |
| N0544 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_State | RVA / layout / policy (audit) | 0x5E | constexpr uint32_t kActor_State = 0x5E, kActor_C2 = 0xC2, kActor_C3 = 0xC3, kActor_CF = 0xCF; |
| N0545 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_C2 | RVA / layout / policy (audit) | 0xC2 | constexpr uint32_t kActor_State = 0x5E, kActor_C2 = 0xC2, kActor_C3 = 0xC3, kActor_CF = 0xCF; |
| N0546 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_C3 | RVA / layout / policy (audit) | 0xC3 | constexpr uint32_t kActor_State = 0x5E, kActor_C2 = 0xC2, kActor_C3 = 0xC3, kActor_CF = 0xCF; |
| N0547 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_CF | RVA / layout / policy (audit) | 0xCF | constexpr uint32_t kActor_State = 0x5E, kActor_C2 = 0xC2, kActor_C3 = 0xC3, kActor_CF = 0xCF; |
| N0548 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_Parent | RVA / layout / policy (audit) | 0xAC | constexpr uint32_t kActor_Parent = 0xAC, kActor_Children = 0xB0, kActor_ChildCount = 0xB8; |
| N0549 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_Children | RVA / layout / policy (audit) | 0xB0 | constexpr uint32_t kActor_Parent = 0xAC, kActor_Children = 0xB0, kActor_ChildCount = 0xB8; |
| N0550 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_ChildCount | RVA / layout / policy (audit) | 0xB8 | constexpr uint32_t kActor_Parent = 0xAC, kActor_Children = 0xB0, kActor_ChildCount = 0xB8; |
| N0551 | src/mod/session2/guest_reload_wait.cpp:63 | kVf_Ready | RVA / layout / policy (audit) | 0x1D8 | constexpr uint32_t kVf_Ready = 0x1D8; |
| N0552 | src/mod/session2/guest_reload_wait.cpp:65 | kActor_Comps | RVA / layout / policy (audit) | 0x68 | constexpr uint32_t kActor_Comps = 0x68, kComps_Seq = 0xA8, kSeqComp_Mgr = 0x18, kMgr_Ready = 0x231; |
| N0553 | src/mod/session2/guest_reload_wait.cpp:65 | kComps_Seq | RVA / layout / policy (audit) | 0xA8 | constexpr uint32_t kActor_Comps = 0x68, kComps_Seq = 0xA8, kSeqComp_Mgr = 0x18, kMgr_Ready = 0x231; |
| N0554 | src/mod/session2/guest_reload_wait.cpp:65 | kSeqComp_Mgr | RVA / layout / policy (audit) | 0x18 | constexpr uint32_t kActor_Comps = 0x68, kComps_Seq = 0xA8, kSeqComp_Mgr = 0x18, kMgr_Ready = 0x231; |
| N0555 | src/mod/session2/guest_reload_wait.cpp:65 | kMgr_Ready | RVA / layout / policy (audit) | 0x231 | constexpr uint32_t kActor_Comps = 0x68, kComps_Seq = 0xA8, kSeqComp_Mgr = 0x18, kMgr_Ready = 0x231; |
| N0556 | src/mod/session2/guest_reload_wait.cpp:66 | kVtStageManager | vtable | 0x55C9720 | constexpr uintptr_t kVtStageManager = 0x55C9720; |
| N0557 | src/mod/session2/guest_reload_wait.cpp:68 | kSnapMax | RVA / layout / policy (audit) | 1024 | constexpr uint32_t kSnapMax = 1024; |
| N0558 | src/mod/session2/guest_reload_wait.cpp:71 | kDropCap | RVA / layout / policy (audit) | 128 | constexpr uint32_t kDropCap = 128, kWarnDrops = 16; |
| N0559 | src/mod/session2/guest_reload_wait.cpp:71 | kWarnDrops | RVA / layout / policy (audit) | 16 | constexpr uint32_t kDropCap = 128, kWarnDrops = 16; |
| N0560 | src/mod/session2/guest_reload_wait.cpp:72 | kGraceUs | RVA / layout / policy (audit) | 2'000'000 | constexpr uint64_t kGraceUs = 2'000'000; |
| N0561 | src/mod/session2/guest_reload_wait.cpp:73 | kStillUs | RVA / layout / policy (audit) | 5'000'000 | constexpr uint64_t kStillUs = 5'000'000; |
| N0562 | src/mod/session2/guest_reload_wait.cpp:74 | kTraceLines | RVA / layout / policy (audit) | 800 | constexpr uint64_t kTraceLines = 800, kBlockLines = 240, kDropLines = 400, kSnapLines = 48; |
| N0563 | src/mod/session2/guest_reload_wait.cpp:74 | kBlockLines | RVA / layout / policy (audit) | 240 | constexpr uint64_t kTraceLines = 800, kBlockLines = 240, kDropLines = 400, kSnapLines = 48; |
| N0564 | src/mod/session2/guest_reload_wait.cpp:74 | kDropLines | RVA / layout / policy (audit) | 400 | constexpr uint64_t kTraceLines = 800, kBlockLines = 240, kDropLines = 400, kSnapLines = 48; |
| N0565 | src/mod/session2/guest_reload_wait.cpp:74 | kSnapLines | RVA / layout / policy (audit) | 48 | constexpr uint64_t kTraceLines = 800, kBlockLines = 240, kDropLines = 400, kSnapLines = 48; |
| N0566 | src/mod/session2/guest_reload_wait.cpp:75 | kShownChildren | RVA / layout / policy (audit) | 4 | constexpr uint32_t kShownChildren = 4; |
| N0567 | src/mod/session2/guest_reload_wait_logic.h:27 | kStateIdle | RVA / layout / policy (audit) | 0x18 | inline constexpr int kStateIdle = 0x18;   // in the field, no load running |
| N0568 | src/mod/session2/guest_reload_wait_logic.h:28 | kStateWait | RVA / layout / policy (audit) | 0x12 | inline constexpr int kStateWait = 0x12;   // the wait list (0x779E8B..0x779F25) |
| N0569 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpMoveFieldAck | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpReloadPos = 2643, kOpPostAck = 3066; |
| N0570 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpReloadPos | Troc ID (audit) | 2643 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpReloadPos = 2643, kOpPostAck = 3066; |
| N0571 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpPostAck | Troc ID (audit) | 3066 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpReloadPos = 2643, kOpPostAck = 3066; |
| N0572 | src/mod/session2/guest_reload_wait_logic.h:31 | kOpTravelReq | Troc ID (audit) | 2283 | inline constexpr uint16_t kOpTravelReq = 2283;  // the client's fast-travel / reload request (C>S) |
| N0573 | src/mod/session2/guest_reload_wait_logic.h:33 | kStaleBeginUs | RVA / layout / policy (audit) | 10'000'000 | inline constexpr uint64_t kStaleBeginUs = 10'000'000; |
| N0574 | src/mod/session2/guest_reload_wait_logic.h:115 | kStateOwn2322 | RVA / layout / policy (audit) | 0x11 | inline constexpr int kStateOwn2322 = 0x11; |
| N0575 | src/mod/session2/guest_reload_wait_logic.h:116 | kOwn2322WarnUs | RVA / layout / policy (audit) | 10'000'000 | inline constexpr uint64_t kOwn2322WarnUs = 10'000'000; |
| N0576 | src/mod/session2/guest_reload_wait_logic.h:277 | kKeys | RVA / layout / policy (audit) | 0x78 | static constexpr uint32_t kKeys = 0x78, kCount = 0x80; |
| N0577 | src/mod/session2/guest_reload_wait_logic.h:277 | kCount | RVA / layout / policy (audit) | 0x80 | static constexpr uint32_t kKeys = 0x78, kCount = 0x80; |
| N0578 | src/mod/session2/guest_save_guest.cpp:29 | kKeepIncoming | RVA / layout / policy (audit) | 10 | constexpr size_t kKeepIncoming = 10;        // generations kept in incoming\ (the newest) |
| N0579 | src/mod/session2/guest_save_guest.cpp:30 | kAccountWaitMs | RVA / layout / policy (audit) | 60000 | constexpr uint64_t kAccountWaitMs = 60000;  // several account folders: wait this long for the own SteamID64 |
| N0580 | src/mod/session2/guest_save_host.cpp:42 | kLoadSlot | RVA / layout / policy (audit) | 0x186F570 | constexpr uintptr_t kLoadSlot = 0x186F570;  // LoadSlot(mgr, int* res, u32 slot, actor) -> res |
| N0581 | src/mod/session2/guest_save_host.cpp:43 | kLoadSlotHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x50, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xB0, 0x01, 0x00, 0x00, 0x4D, 0x8B, 0xF9} | constexpr uint8_t kLoadSlotHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, |
| N0582 | src/mod/session2/guest_save_host.cpp:47 | kSave | RVA / layout / policy (audit) | 0x186E450 | constexpr uintptr_t kSave = 0x186E450;  // SavePlayerToSlot(mgr, int* res, u32 slot, character, opts*) -> res |
| N0583 | src/mod/session2/guest_save_host.cpp:48 | kSaveHead | signature | {0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x78, 0xFE, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x88, 0x02, 0x00, 0x00} | constexpr uint8_t kSaveHead[] = {0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, |
| N0584 | src/mod/session2/guest_save_host.cpp:53 | kFlush | RVA / layout / policy (audit) | 0x1871930 | constexpr uintptr_t kFlush = 0x1871930; |
| N0585 | src/mod/session2/guest_save_host.cpp:54 | kFlushHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x40, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC0, 0x01, 0x00, 0x00, 0x45, 0x0F, 0xB7, 0xF9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2} | constexpr uint8_t kFlushHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, |
| N0586 | src/mod/session2/guest_save_host.cpp:59 | kGetOrCreate | RVA / layout / policy (audit) | 0x2355EC0 | constexpr uintptr_t kGetOrCreate = 0x2355EC0;  // (S, const char* name, u8 in_memory) -> slot |
| N0587 | src/mod/session2/guest_save_host.cpp:60 | kGetOrCreateHead | signature | {0xE9, 0xDB, 0xD8, 0xBF, 0x0C} | constexpr uint8_t kGetOrCreateHead[] = {0xE9, 0xDB, 0xD8, 0xBF, 0x0C}; |
| N0588 | src/mod/session2/guest_save_host.cpp:61 | kAddFile | RVA / layout / policy (audit) | 0x235B100 | constexpr uintptr_t kAddFile = 0x235B100;  // (slot, const char* name) -> ...; makes the 0x140-byte file object |
| N0589 | src/mod/session2/guest_save_host.cpp:62 | kAddFileHead | signature | {0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEA, 0x4C, 0x8B, 0xF9} | constexpr uint8_t kAddFileHead[] = {0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, |
| N0590 | src/mod/session2/guest_save_host.cpp:70 | kCallSites | signature | { {0x2C17881, kLoadSlot}, {0x2C3E1CE, kLoadSlot}, {0x1042ABF3, kLoadSlot}, {0xCBD86CE, kSave}, {0x103BE99A, kSave}, {0x28F273A, kFlush}, {0x108393C8, kFlush}, } | constexpr CallSite kCallSites[] = { |
| N0591 | src/mod/session2/guest_save_host.cpp:79 | kErrBusy | signature | {0x186F6DF, 0x6CF7CA4, } | constexpr ErrorSite kErrBusy = {0x186F6DF, 0x6CF7CA4, "LoadSlot's busy error"}; |
| N0592 | src/mod/session2/guest_save_host.cpp:80 | kErrInvalidSlot | signature | {0x186F62C, 0x6CF7900, } | constexpr ErrorSite kErrInvalidSlot = {0x186F62C, 0x6CF7900, "LoadSlot's invalid-slot error"}; |
| N0593 | src/mod/session2/guest_save_host.cpp:81 | kErrAbsentSlot | signature | {0x2069615, 0x6CF781C, } | constexpr ErrorSite kErrAbsentSlot = {0x2069615, 0x6CF781C, "LoadComponentsFromFile's absent-slot error"}; |
| N0594 | src/mod/session2/guest_save_host.cpp:82 | kErrSaveFail | signature | {0x186E780, 0x6CF7CA0, } | constexpr ErrorSite kErrSaveFail = {0x186E780, 0x6CF7CA0, "SavePlayerToSlot's own failure"}; |
| N0595 | src/mod/session2/guest_save_host.cpp:85 | kSlotMemoryOnly | RVA / layout / policy (audit) | 0x699 | constexpr uint32_t kSlotMemoryOnly = 0x699;  // u8: GetOrCreateSlot .xtls 0xEF538D7, CommitSlot .xtls 0xEF5186D |
| N0596 | src/mod/session2/guest_save_host.cpp:86 | kSlotInMemory | RVA / layout / policy (audit) | 0x69C | constexpr uint32_t kSlotInMemory = 0x69C;    // u8: CreateSlot 0x2355BB0's last argument |
| N0597 | src/mod/session2/guest_save_host.cpp:87 | kFileHeader | RVA / layout / policy (audit) | 0x98 | constexpr uint32_t kFileHeader = 0x98;       // the file's 0x80-byte header (0x2357DA0 copies it at each read) |
| N0598 | src/mod/session2/guest_save_host.cpp:88 | kActorDescriptor | RVA / layout / policy (audit) | 0x88 | constexpr uint32_t kActorDescriptor = 0x88;  // actor -> descriptor; byte +1 == 0: "lobby" (0x20688A0) |
| N0599 | src/mod/session2/guest_save_host.cpp:89 | kActorUser | RVA / layout / policy (audit) | 0xA0 | constexpr uint32_t kActorUser = 0xA0;        // character -> ServerUserActor (0x186E5DD; .xtls 0x1042AB16) |
| N0600 | src/mod/session2/guest_save_host.cpp:90 | kActorKey | RVA / layout / policy (audit) | 0x60 | constexpr uint32_t kActorKey = 0x60;         // u32 actor key (identity_fix.h idf::kActorKey) |
| N0601 | src/mod/session2/guest_save_host.cpp:91 | kMgrPendingSlot | RVA / layout / policy (audit) | 0x5C | constexpr uint32_t kMgrPendingSlot = 0x5C;   // i32: the slot a deferred autosave left (FlushPendingSaveData) |
| N0602 | src/mod/session2/guest_save_logic.h:77 | kGuestSlot | RVA / layout / policy (audit) | 204 | inline constexpr uint32_t kGuestSlot = 204; |
| N0603 | src/mod/session2/guest_save_logic.h:102 | kHeaderSize | RVA / layout / policy (audit) | 0x80 | inline constexpr size_t kHeaderSize = 0x80; |
| N0604 | src/mod/session2/guest_save_logic.h:103 | kMagic | RVA / layout / policy (audit) | 0x45564153 | inline constexpr uint32_t kMagic = 0x45564153; |
| N0605 | src/mod/session2/guest_save_logic.h:104 | kMaxFileSize | RVA / layout / policy (audit) | 16ull * 1024 * 1024 | inline constexpr uint64_t kMaxFileSize = 16ull * 1024 * 1024; |
| N0606 | src/mod/session2/guest_save_logic.h:109 | kMaxRawSize | RVA / layout / policy (audit) | 64ull * 1024 * 1024 | inline constexpr uint64_t kMaxRawSize = 64ull * 1024 * 1024; |
| N0607 | src/mod/session2/guest_save_logic.h:110 | kMaxRawRatio | RVA / layout / policy (audit) | 255 | inline constexpr uint64_t kMaxRawRatio = 255; |
| N0608 | src/mod/session2/guest_save_logic.h:199 | kSlotNameMap | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kSlotNameMap = 0x08; |
| N0609 | src/mod/session2/guest_save_logic.h:200 | kMapBucketCount | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kMapBucketCount = 0x00; |
| N0610 | src/mod/session2/guest_save_logic.h:201 | kMapBuckets | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kMapBuckets = 0x10; |
| N0611 | src/mod/session2/guest_save_logic.h:202 | kMapEntries | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kMapEntries = 0x18; |
| N0612 | src/mod/session2/guest_save_logic.h:203 | kBucketSize | RVA / layout / policy (audit) | 0x100 | inline constexpr uint32_t kBucketSize = 0x100; |
| N0613 | src/mod/session2/guest_save_logic.h:204 | kBucketPairs | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kBucketPairs = 0x08; |
| N0614 | src/mod/session2/guest_save_logic.h:205 | kMaxBucketPairs | RVA / layout / policy (audit) | (kBucketSize - kBucketPairs) / 8 | inline constexpr uint32_t kMaxBucketPairs = (kBucketSize - kBucketPairs) / 8;  // 31 |
| N0615 | src/mod/session2/guest_save_logic.h:206 | kEntryKey | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kEntryKey = 0x08; |
| N0616 | src/mod/session2/guest_save_logic.h:207 | kEntryValue | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kEntryValue = 0x10; |
| N0617 | src/mod/session2/guest_save_logic.h:208 | kKeyChars | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kKeyChars = 0x00; |
| N0618 | src/mod/session2/guest_save_logic.h:209 | kKeyLength | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kKeyLength = 0x08; |
| N0619 | src/mod/session2/guest_save_logic.h:210 | kSlotNames | RVA / layout / policy (audit) | 0x688 | inline constexpr uint32_t kSlotNames = 0x688; |
| N0620 | src/mod/session2/guest_save_logic.h:211 | kSlotNameCount | RVA / layout / policy (audit) | 0x690 | inline constexpr uint32_t kSlotNameCount = 0x690; |
| N0621 | src/mod/session2/guest_save_logic.h:212 | kMaxBuckets | RVA / layout / policy (audit) | 4096 | inline constexpr uint32_t kMaxBuckets = 4096; |
| N0622 | src/mod/session2/guest_save_logic.h:213 | kMaxEntryIndex | RVA / layout / policy (audit) | 1u << 16 | inline constexpr uint32_t kMaxEntryIndex = 1u << 16; |
| N0623 | src/mod/session2/guest_save_logic.h:214 | kMaxSlotFiles | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kMaxSlotFiles = 64; |
| N0624 | src/mod/session2/guest_save_logic.h:462 | kRetUserLogin | RVA / layout / policy (audit) | 0x2C17886 | inline constexpr uint64_t kRetUserLogin = 0x2C17886;      // UserLogin 2490 0x2C17520, call at 0x2C17881 |
| N0625 | src/mod/session2/guest_save_logic.h:463 | kRetLobby3093 | RVA / layout / policy (audit) | 0x2C3E1D3 | inline constexpr uint64_t kRetLobby3093 = 0x2C3E1D3;      // 3093 0x2C3E030, the lobby reload, call at 0x2C3E1CE |
| N0626 | src/mod/session2/guest_save_logic.h:464 | kRetCharacter3093 | RVA / layout / policy (audit) | 0x1042ABF8 | inline constexpr uint64_t kRetCharacter3093 = 0x1042ABF8; // 3093, .xtls 0x1042AAF0 (SetupCharacter), call at 0x1042ABF3 |
| N0627 | src/mod/session2/guest_save_logic.h:483 | kRetAutosave | RVA / layout / policy (audit) | 0xCBD86D3 | inline constexpr uint64_t kRetAutosave = 0xCBD86D3;   // .xtls 0xCBD84D0 (via 0x186E1A0, the 2077 timer), call at 0xCBD86CE |
| N0628 | src/mod/session2/guest_save_logic.h:484 | kRetManual2910 | RVA / layout / policy (audit) | 0x103BE99F | inline constexpr uint64_t kRetManual2910 = 0x103BE99F; // .xtls 0x103BE8E0 (SaveToFileCurrentPlayerReq 2910), call at 0x103BE99A |
| N0629 | src/mod/session2/guest_save_logic.h:501 | kRetFlushLogout | RVA / layout / policy (audit) | 0x28F273F | inline constexpr uint64_t kRetFlushLogout = 0x28F273F;  // the character's logout phase 0x28F26C0, call at 0x28F273A |
| N0630 | src/mod/session2/guest_save_logic.h:502 | kRetFlushTimer | RVA / layout / policy (audit) | 0x108393CD | inline constexpr uint64_t kRetFlushTimer = 0x108393CD;  // .xtls 0x10839370, call at 0x108393C8 |
| N0631 | src/mod/session2/guest_save_logic.h:624 | kKeepGenerations | RVA / layout / policy (audit) | 5 | inline constexpr size_t kKeepGenerations = 5; |
| N0632 | src/mod/session2/guest_save_logic.h:720 | kDigits | RVA / layout / policy (audit) |  | static constexpr char kDigits[] = "0123456789abcdef"; |
| N0633 | src/mod/session2/guest_save_wire.h:52 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1;  // troc_frame.h kControlOpcode |
| N0634 | src/mod/session2/guest_save_wire.h:53 | kStateOffer | RVA / layout / policy (audit) | 6 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0635 | src/mod/session2/guest_save_wire.h:53 | kStateChunk | RVA / layout / policy (audit) | 7 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0636 | src/mod/session2/guest_save_wire.h:53 | kStateAck | RVA / layout / policy (audit) | 8 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0637 | src/mod/session2/guest_save_wire.h:53 | kStateResult | RVA / layout / policy (audit) | 9 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0638 | src/mod/session2/guest_save_wire.h:53 | kStateStatus | RVA / layout / policy (audit) | 10 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0639 | src/mod/session2/guest_save_wire.h:53 | kStateResultAck | RVA / layout / policy (audit) | 11 | inline constexpr uint8_t kStateOffer = 6, kStateChunk = 7, kStateAck = 8, kStateResult = 9, kStateStatus = 10, |
| N0640 | src/mod/session2/guest_save_wire.h:55 | kWireVersion | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kWireVersion = 1; |
| N0641 | src/mod/session2/guest_save_wire.h:56 | kResultVersion | RVA / layout / policy (audit) | 2 | inline constexpr uint8_t kResultVersion = 2;  // SAVE_RESULT only: ver 2 adds the line root (session 25) |
| N0642 | src/mod/session2/guest_save_wire.h:57 | kCapsGuestSave | RVA / layout / policy (audit) | 0x02 | inline constexpr uint8_t kCapsGuestSave = 0x02;  // troc_frame.h kCapsGuestSave |
| N0643 | src/mod/session2/guest_save_wire.h:59 | kMaxChunk | RVA / layout / policy (audit) | 28672 | inline constexpr size_t kMaxChunk = 28672; |
| N0644 | src/mod/session2/guest_save_wire.h:60 | kTrocHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kTrocHeader = 5; |
| N0645 | src/mod/session2/guest_save_wire.h:61 | kOfferSize | RVA / layout / policy (audit) | 97 | inline constexpr size_t kOfferSize = 97, kChunkHeader = 12, kAckSize = 10, kResultSize = 162, kStatusSize = 23; |
| N0646 | src/mod/session2/guest_save_wire.h:61 | kChunkHeader | RVA / layout / policy (audit) | 12 | inline constexpr size_t kOfferSize = 97, kChunkHeader = 12, kAckSize = 10, kResultSize = 162, kStatusSize = 23; |
| N0647 | src/mod/session2/guest_save_wire.h:61 | kAckSize | RVA / layout / policy (audit) | 10 | inline constexpr size_t kOfferSize = 97, kChunkHeader = 12, kAckSize = 10, kResultSize = 162, kStatusSize = 23; |
| N0648 | src/mod/session2/guest_save_wire.h:61 | kResultSize | RVA / layout / policy (audit) | 162 | inline constexpr size_t kOfferSize = 97, kChunkHeader = 12, kAckSize = 10, kResultSize = 162, kStatusSize = 23; |
| N0649 | src/mod/session2/guest_save_wire.h:61 | kStatusSize | RVA / layout / policy (audit) | 23 | inline constexpr size_t kOfferSize = 97, kChunkHeader = 12, kAckSize = 10, kResultSize = 162, kStatusSize = 23; |
| N0650 | src/mod/session2/guest_save_wire.h:64 | kLoginHoldMs | RVA / layout / policy (audit) | 30000 | inline constexpr uint64_t kLoginHoldMs = 30000;  // a 3093 waits at most this for the host's SAVE_ACK |
| N0651 | src/mod/session2/guest_save_wire.h:65 | kFinalHoldMs | RVA / layout / policy (audit) | 15000 | inline constexpr uint64_t kFinalHoldMs = 15000;  // a goodbye waits at most this for the final save's SAVE_RESULT |
| N0652 | src/mod/session2/guest_save_wire.h:66 | kOfferRetries | RVA / layout / policy (audit) | 1 | inline constexpr uint32_t kOfferRetries = 1;     // a refused upload is offered once more |
| N0653 | src/mod/session2/guest_save_wire.h:67 | kOpSaveAutoReq | Troc ID (audit) | 3037 | inline constexpr uint16_t kOpSaveAutoReq = 3037; |
| N0654 | src/mod/session2/guest_save_wire.h:68 | kOpSaveToFileCurrentPlayerReq | Troc ID (audit) | 2910 | inline constexpr uint16_t kOpSaveToFileCurrentPlayerReq = 2910; |
| N0655 | src/mod/session2/guest_save_wire.h:69 | kOpSaveToFileCurrentPlayerAck | Troc ID (audit) | 3016 | inline constexpr uint16_t kOpSaveToFileCurrentPlayerAck = 3016; |
| N0656 | src/mod/session2/guest_save_wire.h:70 | kOpCreateCharacterReq | Troc ID (audit) | 2711 | inline constexpr uint16_t kOpCreateCharacterReq = 2711; |
| N0657 | src/mod/session2/guest_save_wire.h:71 | kOpNewGameStartReq | Troc ID (audit) | 2178 | inline constexpr uint16_t kOpNewGameStartReq = 2178; |
| N0658 | src/mod/session2/guest_save_wire.h:72 | kOpLoginCharacterReq | Troc ID (audit) | 3093 | inline constexpr uint16_t kOpLoginCharacterReq = 3093;  // u8 &#124; u64 characterNo @1 &#124; u32 slot @9 |
| N0659 | src/mod/session2/guest_save_wire.h:73 | kOpLogoutSessionReq | Troc ID (audit) | 2001 | inline constexpr uint16_t kOpLogoutSessionReq = 2001;   // D1 07 FF (internal form, 3 bytes) |
| N0660 | src/mod/session2/guest_save_wire.h:77 | kFiles | RVA / layout / policy (audit) | 2 | inline constexpr size_t kFiles = 2; |
| N0661 | src/mod/session2/guest_save_wire.h:374 | kFinalSaveSlot | RVA / layout / policy (audit) | 100 | inline constexpr uint32_t kFinalSaveSlot = 100; |
| N0662 | src/mod/session2/guest_save_wire.h:375 | kFinalSaveName | RVA / layout / policy (audit) | u | inline constexpr std::u16string_view kFinalSaveName = u"Co-op"; |
| N0663 | src/mod/session2/guest_save_wire.h:376 | kMaxSaveName | RVA / layout / policy (audit) | 30 | inline constexpr uint32_t kMaxSaveName = 30; |
| N0664 | src/mod/session2/guest_save_wire.h:685 | kDownloadBacklog | RVA / layout / policy (audit) | 256 * 1024 | inline constexpr size_t kDownloadBacklog = 256 * 1024; |
| N0665 | src/mod/session2/guest_save_wire.h:686 | kResultAckTimeoutMs | RVA / layout / policy (audit) | 60000 | inline constexpr uint64_t kResultAckTimeoutMs = 60000;  // after the last chunk |
| N0666 | src/mod/session2/guest_save_wire.h:687 | kMaxResultAttempts | RVA / layout / policy (audit) | 3 | inline constexpr uint32_t kMaxResultAttempts = 3;       // per generation and connection |
| N0667 | src/mod/session2/guest_save_wire.h:853 | kRecentReqMs | RVA / layout / policy (audit) | 20000 | inline constexpr uint64_t kRecentReqMs = 20000; |
| N0668 | src/mod/session2/guest_seq_route_logic.h:39 | kPoints | RVA / layout / policy (audit) | static_cast<size_t>(Point::Count) | inline constexpr size_t kPoints = static_cast<size_t>(Point::Count); |
| N0669 | src/mod/session2/guest_seq_route_logic.h:40 | kPointOp | signature | {2661, 2981, 2810, 2322, 2524} | inline constexpr std::array<uint16_t, kPoints> kPointOp = {2661, 2981, 2810, 2322, 2524}; |
| N0670 | src/mod/session2/guest_seq_route_logic.h:73 | kCharClass | RVA / layout / policy (audit) | 0xA | inline constexpr uint32_t kCharClass = 0xA; |
| N0671 | src/mod/session2/guest_seq_route_logic.h:142 | kMaxBranchTable | RVA / layout / policy (audit) | 4096 | inline constexpr uint32_t kMaxBranchTable = 4096; |
| N0672 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchActorAt | RVA / layout / policy (audit) | 1 | inline constexpr size_t kBranchActorAt = 1, kBranchSeqAt = 5, kBranchStateAt = 13, kBranchMinPayload = 17; |
| N0673 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchSeqAt | RVA / layout / policy (audit) | 5 | inline constexpr size_t kBranchActorAt = 1, kBranchSeqAt = 5, kBranchStateAt = 13, kBranchMinPayload = 17; |
| N0674 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchStateAt | RVA / layout / policy (audit) | 13 | inline constexpr size_t kBranchActorAt = 1, kBranchSeqAt = 5, kBranchStateAt = 13, kBranchMinPayload = 17; |
| N0675 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchMinPayload | RVA / layout / policy (audit) | 17 | inline constexpr size_t kBranchActorAt = 1, kBranchSeqAt = 5, kBranchStateAt = 13, kBranchMinPayload = 17; |
| N0676 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareActorAt | RVA / layout / policy (audit) | 1 | inline constexpr size_t kPrepareActorAt = 1, kPrepareSeqAt = 9, kPrepareStageAt = 19, kPrepareResourceAt = 27, |
| N0677 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareSeqAt | RVA / layout / policy (audit) | 9 | inline constexpr size_t kPrepareActorAt = 1, kPrepareSeqAt = 9, kPrepareStageAt = 19, kPrepareResourceAt = 27, |
| N0678 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareStageAt | RVA / layout / policy (audit) | 19 | inline constexpr size_t kPrepareActorAt = 1, kPrepareSeqAt = 9, kPrepareStageAt = 19, kPrepareResourceAt = 27, |
| N0679 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareResourceAt | RVA / layout / policy (audit) | 27 | inline constexpr size_t kPrepareActorAt = 1, kPrepareSeqAt = 9, kPrepareStageAt = 19, kPrepareResourceAt = 27, |
| N0680 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareMinPayload | RVA / layout / policy (audit) | 31 | inline constexpr size_t kPrepareActorAt = 1, kPrepareSeqAt = 9, kPrepareStageAt = 19, kPrepareResourceAt = 27, |
| N0681 | src/mod/session2/guest_seq_route_logic.h:255 | kSeqBinds | RVA / layout / policy (audit) | 8 | inline constexpr size_t kSeqBinds = 8; |
| N0682 | src/mod/session2/guest_seq_route_logic.h:475 | kSchedPoints | RVA / layout / policy (audit) | static_cast<size_t>(SchedPoint::Count) | inline constexpr size_t kSchedPoints = static_cast<size_t>(SchedPoint::Count); |
| N0683 | src/mod/session2/guest_seq_route_logic.h:476 | kSchedOp | signature | {2335, 2126, 2958, 2909} | inline constexpr std::array<uint16_t, kSchedPoints> kSchedOp = {2335, 2126, 2958, 2909}; |
| N0684 | src/mod/session2/guest_seq_route_logic.h:655 | kSchedLinesPerKey | RVA / layout / policy (audit) | 4 | inline constexpr uint8_t kSchedLinesPerKey = 4; |
| N0685 | src/mod/session2/guest_seq_route_logic.h:668 | kMaxCensusStages | RVA / layout / policy (audit) | 1024 | inline constexpr uint32_t kMaxCensusStages = 1024, kMaxCensusNodes = 256; |
| N0686 | src/mod/session2/guest_seq_route_logic.h:668 | kMaxCensusNodes | RVA / layout / policy (audit) | 256 | inline constexpr uint32_t kMaxCensusStages = 1024, kMaxCensusNodes = 256; |
| N0687 | src/mod/session2/guest_seq_route_logic.h:775 | kBindCap | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kBindCap = 64;  // native calls per 2335 at most (the rest counted) |
| N0688 | src/mod/session2/guest_seq_route_logic.h:782 | kBucketCount | RVA / layout / policy (audit) | 0x88 | static constexpr uint32_t kBucketCount = 0x88, kElements = 0x8C, kBuckets = 0x98, kNodes = 0xA0; |
| N0689 | src/mod/session2/guest_seq_route_logic.h:782 | kElements | RVA / layout / policy (audit) | 0x8C | static constexpr uint32_t kBucketCount = 0x88, kElements = 0x8C, kBuckets = 0x98, kNodes = 0xA0; |
| N0690 | src/mod/session2/guest_seq_route_logic.h:782 | kBuckets | RVA / layout / policy (audit) | 0x98 | static constexpr uint32_t kBucketCount = 0x88, kElements = 0x8C, kBuckets = 0x98, kNodes = 0xA0; |
| N0691 | src/mod/session2/guest_seq_route_logic.h:782 | kNodes | RVA / layout / policy (audit) | 0xA0 | static constexpr uint32_t kBucketCount = 0x88, kElements = 0x8C, kBuckets = 0x98, kNodes = 0xA0; |
| N0692 | src/mod/session2/guest_seq_route_logic.h:783 | kBucketStride | RVA / layout / policy (audit) | 0x100 | static constexpr uint32_t kBucketStride = 0x100, kBucketMax = (0x100 - 8) / 8, kNodeKey = 0x04, kNodeActor = 0x08; |
| N0693 | src/mod/session2/guest_seq_route_logic.h:783 | kBucketMax | RVA / layout / policy (audit) | (0x100 - 8) / 8 | static constexpr uint32_t kBucketStride = 0x100, kBucketMax = (0x100 - 8) / 8, kNodeKey = 0x04, kNodeActor = 0x08; |
| N0694 | src/mod/session2/guest_seq_route_logic.h:783 | kNodeKey | RVA / layout / policy (audit) | 0x04 | static constexpr uint32_t kBucketStride = 0x100, kBucketMax = (0x100 - 8) / 8, kNodeKey = 0x04, kNodeActor = 0x08; |
| N0695 | src/mod/session2/guest_seq_route_logic.h:783 | kNodeActor | RVA / layout / policy (audit) | 0x08 | static constexpr uint32_t kBucketStride = 0x100, kBucketMax = (0x100 - 8) / 8, kNodeKey = 0x04, kNodeActor = 0x08; |
| N0696 | src/mod/session2/guest_seq_route_logic.h:889 | kRetryTimeoutUs | RVA / layout / policy (audit) | 120'000'000 | inline constexpr uint64_t kRetryTimeoutUs = 120'000'000;  // 2 min after it was first queued |
| N0697 | src/mod/session2/guest_seq_route_logic.h:890 | kRetryPeriodUs | RVA / layout / policy (audit) | 250'000 | inline constexpr uint64_t kRetryPeriodUs = 250'000;       // one pass every 250 ms at most |
| N0698 | src/mod/session2/guest_seq_route_logic.h:891 | kRetryCallsPerPass | RVA / layout / policy (audit) | 32 | inline constexpr uint32_t kRetryCallsPerPass = 32;        // native calls per pass at most (the rest next pass) |
| N0699 | src/mod/session2/guest_seq_route_logic.h:892 | kRetryCap | RVA / layout / policy (audit) | 512 | inline constexpr uint32_t kRetryCap = 512;                // queued NPCs at most (196 in session 12) |
| N0700 | src/mod/session2/guest_seq_route_logic.h:895 | kRetryStillZeroMax | RVA / layout / policy (audit) | 3 | inline constexpr uint32_t kRetryStillZeroMax = 3; |
| N0701 | src/mod/session2/guest_seq_route_logic.h:896 | kRetryRefusedMax | RVA / layout / policy (audit) | 3 | inline constexpr uint32_t kRetryRefusedMax = 3; |
| N0702 | src/mod/session2/guest_seq_route_logic.h:1078 | kActorTypeNpc | RVA / layout / policy (audit) | 3 | inline constexpr uint8_t kActorTypeNpc = 3;  // [[actor+0x88]+1] (guest_sched_bind's "actor type 3", session 12) |
| N0703 | src/mod/session2/guest_seq_route_logic.h:1134 | kPrefix | RVA / layout / policy (audit) |  | constexpr std::string_view kPrefix = "cd_seq_funcnpc"; |
| N0704 | src/mod/session2/guest_seq_route_logic.h:1160 | kMgr_MapCount | RVA / layout / policy (audit) | 0x10C | inline constexpr uint32_t kMgr_MapCount = 0x10C, kMgr_MapNodes = 0x120, kMapNode_Seq = 0x08, kMapNode_Inst = 0x10, |
| N0705 | src/mod/session2/guest_seq_route_logic.h:1160 | kMgr_MapNodes | RVA / layout / policy (audit) | 0x120 | inline constexpr uint32_t kMgr_MapCount = 0x10C, kMgr_MapNodes = 0x120, kMapNode_Seq = 0x08, kMapNode_Inst = 0x10, |
| N0706 | src/mod/session2/guest_seq_route_logic.h:1160 | kMapNode_Seq | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kMgr_MapCount = 0x10C, kMgr_MapNodes = 0x120, kMapNode_Seq = 0x08, kMapNode_Inst = 0x10, |
| N0707 | src/mod/session2/guest_seq_route_logic.h:1160 | kMapNode_Inst | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kMgr_MapCount = 0x10C, kMgr_MapNodes = 0x120, kMapNode_Seq = 0x08, kMapNode_Inst = 0x10, |
| N0708 | src/mod/session2/guest_seq_route_logic.h:1160 | kInst_Dead | RVA / layout / policy (audit) | 0x15 | inline constexpr uint32_t kMgr_MapCount = 0x10C, kMgr_MapNodes = 0x120, kMapNode_Seq = 0x08, kMapNode_Inst = 0x10, |
| N0709 | src/mod/session2/guest_seq_route_logic.h:1162 | kMaxMapNodes | RVA / layout / policy (audit) | 4096 | inline constexpr uint32_t kMaxMapNodes = 4096; |
| N0710 | src/mod/session2/guest_seq_route_logic.h:1167 | kStage_KeyOff | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kStage_KeyOff = 0x30, kStage_InfoOff = 0x48, kInfo_NameOff = 0x1B8, kInst_StageOff = 0x58; |
| N0711 | src/mod/session2/guest_seq_route_logic.h:1167 | kStage_InfoOff | RVA / layout / policy (audit) | 0x48 | inline constexpr uint32_t kStage_KeyOff = 0x30, kStage_InfoOff = 0x48, kInfo_NameOff = 0x1B8, kInst_StageOff = 0x58; |
| N0712 | src/mod/session2/guest_seq_route_logic.h:1167 | kInfo_NameOff | RVA / layout / policy (audit) | 0x1B8 | inline constexpr uint32_t kStage_KeyOff = 0x30, kStage_InfoOff = 0x48, kInfo_NameOff = 0x1B8, kInst_StageOff = 0x58; |
| N0713 | src/mod/session2/guest_seq_route_logic.h:1167 | kInst_StageOff | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kStage_KeyOff = 0x30, kStage_InfoOff = 0x48, kInfo_NameOff = 0x1B8, kInst_StageOff = 0x58; |
| N0714 | src/mod/session2/guest_x_veto_logic.h:41 | kCharClass | RVA / layout / policy (audit) | 0xA | constexpr uint32_t kCharClass = 0xA;  // key >> 28 of a character (0xA0100001, 0xA0100377) |
| N0715 | src/mod/session2/guest_x_veto_logic.h:119 | kContextMaxAgeUs | RVA / layout / policy (audit) | 500'000 | constexpr uint64_t kContextMaxAgeUs = 500'000;  // a routed 2981 or one tick of X lasts far less (one frame) |
| N0716 | src/mod/session2/guest_x_veto_logic.h:204 | kFns | RVA / layout / policy (audit) | static_cast<size_t>(Fn::Count) | constexpr size_t kFns = static_cast<size_t>(Fn::Count); |
| N0717 | src/mod/session2/guest_x_veto_logic.h:293 | kDefaultAllow | RVA / layout / policy (audit) |  | constexpr std::string_view kDefaultAllow = "SetInteraction"; |
| N0718 | src/mod/session2/guest_x_veto_logic.h:397 | kOpLoginCharacterToFieldServerReq | Troc ID (audit) | 3093 | constexpr uint16_t kOpLoginCharacterToFieldServerReq = 3093;  // the title's Play / Load: a save is loaded |
| N0719 | src/mod/session2/guest_x_veto_logic.h:398 | kOpMoveFieldCompleteReq | Troc ID (audit) | 2549 | constexpr uint16_t kOpMoveFieldCompleteReq = 2549;            // the client finished its field load |
| N0720 | src/mod/session2/guest_x_veto_logic.h:399 | kStateInWorld | RVA / layout / policy (audit) | 0x18 | constexpr uint8_t kStateInWorld = 0x18;                       // [P+0x15C] in the world |
| N0721 | src/mod/session2/guest_x_veto_logic.h:404 | kStuckMs | RVA / layout / policy (audit) | 45'000 | static constexpr uint64_t kStuckMs = 45'000;  // no 2549 this long after the 3093 (3x a normal load)... |
| N0722 | src/mod/session2/guest_x_veto_logic.h:405 | kStillMs | RVA / layout / policy (audit) | 20'000 | static constexpr uint64_t kStillMs = 20'000;  // ...and the same loading state this long |
| N0723 | src/mod/session2/hold_scene_order_logic.h:53 | kOpPrepare | Troc ID (audit) | 2661 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0724 | src/mod/session2/hold_scene_order_logic.h:53 | kOpBranch | Troc ID (audit) | 2981 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0725 | src/mod/session2/hold_scene_order_logic.h:53 | kOpOnBranch | Troc ID (audit) | 2524 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0726 | src/mod/session2/hold_scene_order_logic.h:53 | kOpDelete | Troc ID (audit) | 2810 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0727 | src/mod/session2/hold_scene_order_logic.h:53 | kOpAnnounce | Troc ID (audit) | 2005 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0728 | src/mod/session2/hold_scene_order_logic.h:53 | kOpRemove | Troc ID (audit) | 2516 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0729 | src/mod/session2/hold_scene_order_logic.h:53 | kOpMoveField | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0730 | src/mod/session2/hold_scene_order_logic.h:53 | kOpLoadingField | Troc ID (audit) | 2643 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0731 | src/mod/session2/hold_scene_order_logic.h:53 | kOpBoundary2756 | Troc ID (audit) | 2756 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0732 | src/mod/session2/hold_scene_order_logic.h:53 | kOpChartEvent | Troc ID (audit) | 2111 | inline constexpr uint16_t kOpPrepare = 2661, kOpBranch = 2981, kOpOnBranch = 2524, kOpDelete = 2810, |
| N0733 | src/mod/session2/hold_scene_order_logic.h:74 | kPreparePrefix | RVA / layout / policy (audit) | 91 | inline constexpr size_t kPreparePrefix = 91, kBindingWire = 11; |
| N0734 | src/mod/session2/hold_scene_order_logic.h:74 | kBindingWire | RVA / layout / policy (audit) | 11 | inline constexpr size_t kPreparePrefix = 91, kBindingWire = 11; |
| N0735 | src/mod/session2/host_save_export.cpp:39 | kMaxManifestBytes | RVA / layout / policy (audit) | 16ull * 1024 * 1024 | constexpr uint64_t kMaxManifestBytes = 16ull * 1024 * 1024; |
| N0736 | src/mod/session2/host_save_export.cpp:40 | kMaxJournalBytes | RVA / layout / policy (audit) | 16ull * 1024 * 1024 | constexpr uint64_t kMaxJournalBytes = 16ull * 1024 * 1024; |
| N0737 | src/mod/session2/host_save_export.cpp:41 | kKeepJournals | RVA / layout / policy (audit) | 200 | constexpr size_t kKeepJournals = 200; |
| N0738 | src/mod/session2/host_save_export.cpp:42 | kMaxDepth | RVA / layout / policy (audit) | 16 | constexpr int kMaxDepth = 16; |
| N0739 | src/mod/session2/host_save_logic.h:68 | kDefaultKeep | RVA / layout / policy (audit) | 20 | inline constexpr uint32_t kDefaultKeep = 20; |
| N0740 | src/mod/session2/host_save_logic.h:69 | kMaxKeep | RVA / layout / policy (audit) | 500 | inline constexpr uint32_t kMaxKeep = 500; |
| N0741 | src/mod/session2/host_save_logic.h:171 | kPartialSuffix | RVA / layout / policy (audit) | L | inline constexpr std::wstring_view kPartialSuffix = L".partial"; |
| N0742 | src/mod/session2/host_save_logic.h:218 | kAsideInfix | RVA / layout / policy (audit) | L | inline constexpr std::wstring_view kAsideInfix = L"_unexported_"; |
| N0743 | src/mod/session2/host_save_logic.h:341 | kMaxAside | RVA / layout / policy (audit) | 3 | inline constexpr size_t kMaxAside = 3; |
| N0744 | src/mod/session2/host_save_logic.h:510 | kManifestHeader | RVA / layout / policy (audit) |  | inline constexpr std::string_view kManifestHeader = "cdmp host save generation (P8 S35 phase E)"; |
| N0745 | src/mod/session2/host_save_logic.h:511 | kManifestFile | RVA / layout / policy (audit) |  | inline constexpr std::string_view kManifestFile = "manifest.txt"; |
| N0746 | src/mod/session2/host_save_logic.h:698 | kJournalHeader | RVA / layout / policy (audit) |  | inline constexpr std::string_view kJournalHeader = "cdmp host save run journal (P8 S35 phase E)"; |
| N0747 | src/mod/session2/host_save_logic.h:699 | kJournalTruncated | RVA / layout / policy (audit) |  | inline constexpr std::string_view kJournalTruncated = "truncated"; |
| N0748 | src/mod/session2/host_save_logic.h:700 | kMaxJournalEvents | RVA / layout / policy (audit) | 4000 | inline constexpr uint32_t kMaxJournalEvents = 4000; |
| N0749 | src/mod/session2/host_save_logic.h:727 | kNoSlot | RVA / layout / policy (audit) | 0xFFFFFFFFu | inline constexpr uint32_t kNoSlot = 0xFFFFFFFFu; |
| N0750 | src/mod/session2/host_save_logic.h:952 | kPairGapMs | RVA / layout / policy (audit) | 10000 | inline constexpr uint64_t kPairGapMs = 10000; |
| N0751 | src/mod/session2/identity_fix.cpp:215 | kLobbyLoad | RVA / layout / policy (audit) | 0x2B7F4D0 | constexpr uintptr_t kLobbyLoad = 0x2B7F4D0;  // (comp, int* res, -, data**) -> res; r8 unused, no stack args |
| N0752 | src/mod/session2/identity_fix.cpp:216 | kLobbyLoadHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xF2, 0x4C, 0x8B, 0xF9, 0x49, 0x8B, 0x39} | constexpr uint8_t kLobbyLoadHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x56, 0x57, |
| N0753 | src/mod/session2/identity_fix.cpp:219 | kLobbyVtable | vtable | 0x5B2BE58 | constexpr uintptr_t kLobbyVtable = 0x5B2BE58;     // ServerLobbyActorComponent; +0x50 = 0x2B7F4D0 |
| N0754 | src/mod/session2/identity_fix.cpp:220 | kLobbyConstant | RVA / layout / policy (audit) | 0x2B7F66C | constexpr uintptr_t kLobbyConstant = 0x2B7F66C;   // movabs rax,7FFF.. ; mov [rsp+0x70],rax |
| N0755 | src/mod/session2/identity_fix.cpp:221 | kLobbyConstantBytes | signature | {0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x48, 0x89, 0x44, 0x24, 0x70} | constexpr uint8_t kLobbyConstantBytes[] = {0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, |
| N0756 | src/mod/session2/identity_fix.cpp:223 | kLobbyAddCall | RVA / layout / policy (audit) | 0x2B7F6DB | constexpr uintptr_t kLobbyAddCall = 0x2B7F6DB;    // call 0x2143E70 (adds the entry, R8 = &number) |
| N0757 | src/mod/session2/identity_fix.cpp:224 | kLobbyAddCallBytes | signature | {0xE8, 0x90, 0x47, 0x5C, 0xFF} | constexpr uint8_t kLobbyAddCallBytes[] = {0xE8, 0x90, 0x47, 0x5C, 0xFF}; |
| N0758 | src/mod/session2/identity_fix.cpp:226 | kClearLobbySite | RVA / layout / policy (audit) | 0x2C3E0FF | constexpr uintptr_t kClearLobbySite = 0x2C3E0FF; |
| N0759 | src/mod/session2/identity_fix.cpp:227 | kClearLobbySiteBytes | signature | {0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08} | constexpr uint8_t kClearLobbySiteBytes[] = {0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08}; |
| N0760 | src/mod/session2/identity_fix.cpp:228 | kClearListSite | RVA / layout / policy (audit) | 0x2C3E126 | constexpr uintptr_t kClearListSite = 0x2C3E126; |
| N0761 | src/mod/session2/identity_fix.cpp:229 | kClearListSiteBytes | signature | {0x48, 0x8B, 0x4B, 0x18} | constexpr uint8_t kClearListSiteBytes[] = {0x48, 0x8B, 0x4B, 0x18}; |
| N0762 | src/mod/session2/identity_fix.cpp:230 | kStatusVtable | vtable | 0x5B203F0 | constexpr uintptr_t kStatusVtable = 0x5B203F0;    // ServerStatusActorComponent; +0x50 = 0x293FE70 |
| N0763 | src/mod/session2/identity_fix.cpp:231 | kStatusLoad | RVA / layout / policy (audit) | 0x293FE70 | constexpr uintptr_t kStatusLoad = 0x293FE70; |
| N0764 | src/mod/session2/identity_fix.cpp:234 | kLoginMgrSite | RVA / layout / policy (audit) | 0x2777A18 | constexpr uintptr_t kLoginMgrSite = 0x2777A18; |
| N0765 | src/mod/session2/identity_fix.cpp:235 | kLoginMgrSiteBytes | signature | {0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09} | constexpr uint8_t kLoginMgrSiteBytes[] = {0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09}; |
| N0766 | src/mod/session2/identity_fix.cpp:236 | kLoginMgrSlot | RVA / layout / policy (audit) | 0x6D696A8 | constexpr uintptr_t kLoginMgrSlot = 0x6D696A8; |
| N0767 | src/mod/session2/identity_fix.cpp:237 | kFindByNoScan | RVA / layout / policy (audit) | 0x2A84066 | constexpr uintptr_t kFindByNoScan = 0x2A84066; |
| N0768 | src/mod/session2/identity_fix.cpp:238 | kFindByNoScanBytes | signature | {0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x14, 0xC1} | constexpr uint8_t kFindByNoScanBytes[] = {0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, |
| N0769 | src/mod/session2/identity_fix.cpp:240 | kFindByNoRead | RVA / layout / policy (audit) | 0x2A84090 | constexpr uintptr_t kFindByNoRead = 0x2A84090; |
| N0770 | src/mod/session2/identity_fix.cpp:241 | kFindByNoReadBytes | signature | {0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20} | constexpr uint8_t kFindByNoReadBytes[] = {0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20}; |
| N0771 | src/mod/session2/identity_fix.cpp:242 | kLoginMgrActors | RVA / layout / policy (audit) | 0xB8 | constexpr uint32_t kLoginMgrActors = 0xB8; |
| N0772 | src/mod/session2/identity_fix.cpp:243 | kLoginMgrCount | RVA / layout / policy (audit) | 0xC0 | constexpr uint32_t kLoginMgrCount = 0xC0; |
| N0773 | src/mod/session2/identity_fix.cpp:244 | kOpLoginUser | Troc ID (audit) | 2490 | constexpr uint16_t kOpLoginUser = 2490; |
| N0774 | src/mod/session2/identity_fix.cpp:245 | kOpLoginCharacter | Troc ID (audit) | 3093 | constexpr uint16_t kOpLoginCharacter = 3093; |
| N0775 | src/mod/session2/identity_fix.cpp:246 | kMaxLogged | RVA / layout / policy (audit) | 48 | constexpr uint64_t kMaxLogged = 48, kMaxRefusalsLogged = 8; |
| N0776 | src/mod/session2/identity_fix.cpp:246 | kMaxRefusalsLogged | RVA / layout / policy (audit) | 8 | constexpr uint64_t kMaxLogged = 48, kMaxRefusalsLogged = 8; |
| N0777 | src/mod/session2/identity_fix.h:45 | kNative | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFFFFF | inline constexpr uint64_t kNative = 0x7FFFFFFFFFFFFFFF;   // the constant (0x2B7F66C, .xtls 0x1042755C) |
| N0778 | src/mod/session2/identity_fix.h:46 | kUser2 | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFF002 | inline constexpr uint64_t kUser2 = 0x7FFFFFFFFFFFF002;    // identidad.md §5.1 (a mod value, not an official id) |
| N0779 | src/mod/session2/identity_fix.h:47 | kOldPuppet | RVA / layout / policy (audit) | 0x7FFFFFFFFFFFF001 | inline constexpr uint64_t kOldPuppet = 0x7FFFFFFFFFFFF001; |
| N0780 | src/mod/session2/identity_fix.h:48 | kLobbyOwner | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kLobbyOwner = 0x08;     // comp -> its ServerUserActor (0x2B7F649, 0x2C3E107) |
| N0781 | src/mod/session2/identity_fix.h:49 | kLobbyList | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kLobbyList = 0x18;      // comp -> list (0x2C3E126, .xtls 0x1042AB45) |
| N0782 | src/mod/session2/identity_fix.h:50 | kListCount | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kListCount = 0x08;      // u32 (.xtls 0x1042AB49, 0x103C31FB) |
| N0783 | src/mod/session2/identity_fix.h:51 | kEntryStride | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kEntryStride = 0xD0;    // (.xtls 0x1042AB62, 0x103C31B2) |
| N0784 | src/mod/session2/identity_fix.h:52 | kEntryNumber | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kEntryNumber = 0x00;    // u64 characterNo (.xtls 0x1042AB6C, 0xE4A214D) |
| N0785 | src/mod/session2/identity_fix.h:53 | kUserComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kUserComponents = 0x68; // user/actor -> component table |
| N0786 | src/mod/session2/identity_fix.h:54 | kTableLobby | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kTableLobby = 0x10;     // table -> lobby component (0x2C3E103, .xtls 0x1042AB21) |
| N0787 | src/mod/session2/identity_fix.h:55 | kTableStatus | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kTableStatus = 0x20;    // table -> Status (0x206A894) |
| N0788 | src/mod/session2/identity_fix.h:56 | kStatusActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kStatusActor = 0x08;    // Status -> its actor |
| N0789 | src/mod/session2/identity_fix.h:57 | kStatusNumber | RVA / layout / policy (audit) | 0x70 | inline constexpr uint32_t kStatusNumber = 0x70;   // Status -> u64* characterNo (0x206A8A5) |
| N0790 | src/mod/session2/identity_fix.h:58 | kActorKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActorKey = 0x60;       // u32 |
| N0791 | src/mod/session2/identity_fix.h:59 | kActorUser | RVA / layout / policy (audit) | 0xA0 | inline constexpr uint32_t kActorUser = 0xA0;      // character -> ServerUserActor (.xtls 0x1042AB16) |
| N0792 | src/mod/session2/identity_fix.h:60 | kLockObject | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kLockObject = 0x08;     // owner -> its lock object (0x2B7F64D); u32 count at +8 |
| N0793 | src/mod/session2/identity_fix.h:61 | kMaxEntries | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kMaxEntries = 64;       // a lobby has a handful of characters |
| N0794 | src/mod/session2/identity_fix.h:62 | kMaxActors | RVA / layout / policy (audit) | 0x4000 | inline constexpr uint32_t kMaxActors = 0x4000;    // logged-in list scanned for uniqueness |
| N0795 | src/mod/session2/identity_fix.h:63 | kCharacterClass | RVA / layout / policy (audit) | 0xA | inline constexpr uint32_t kCharacterClass = 0xA;  // key >> 28 of characters (users 0x9, NPCs/children 0xB) |
| N0796 | src/mod/session2/identity_probe.cpp:31 | kLoad | RVA / layout / policy (audit) | 0x20688A0 | constexpr uintptr_t kLoad = 0x20688A0;  // LoadComponentsFromFile(actor, res*, store, name, u32 slot) -> res |
| N0797 | src/mod/session2/identity_probe.cpp:32 | kLoadHead | signature | {0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x68, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00, 0x49, 0x8B, 0xD9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xF1} | constexpr uint8_t kLoadHead[] = {0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, |
| N0798 | src/mod/session2/identity_probe.cpp:39 | kComponentSite | RVA / layout / policy (audit) | 0x20693CD | constexpr uintptr_t kComponentSite = 0x20693CD; |
| N0799 | src/mod/session2/identity_probe.cpp:40 | kComponentSiteBytes | signature | {0x49, 0x8B, 0x07, 0x8B, 0x8D, 0x00, 0x03, 0x00, 0x00, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x8D, 0x4D, 0xC0, 0x44, 0x8B, 0xC6, 0x48, 0x8D, 0x55, 0x30, 0x49, 0x8B, 0xCF, 0xFF, 0x50, 0x50, 0x8B, 0x18, 0x85, 0xDB, 0x0F, 0x85, 0x8D, 0x00, 0x00, 0x00} | constexpr uint8_t kComponentSiteBytes[] = {0x49, 0x8B, 0x07, 0x8B, 0x8D, 0x00, 0x03, 0x00, 0x00, 0x89, |
| N0800 | src/mod/session2/identity_probe.cpp:44 | kComponentHook | RVA / layout / policy (audit) | kComponentSite + 30 | constexpr uintptr_t kComponentHook = kComponentSite + 30;  // 0x20693EB |
| N0801 | src/mod/session2/identity_probe.cpp:45 | kOwner | RVA / layout / policy (audit) | 0x2AE1010 | constexpr uintptr_t kOwner = 0x2AE1010;  // (component, u16* type, u64* no, u64* no2), void |
| N0802 | src/mod/session2/identity_probe.cpp:46 | kOwnerHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x59, 0x08, 0x49, 0x8B, 0xF9, 0x4D, 0x8B, 0xF0, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0x43, 0x68, 0x48, 0x8B, 0xA8, 0x18, 0x01, 0x00, 0x00} | constexpr uint8_t kOwnerHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, |
| N0803 | src/mod/session2/identity_probe.cpp:50 | kPhases | RVA / layout / policy (audit) | 0x2941020 | constexpr uintptr_t kPhases = 0x2941020;  // (obj, res*, ?, u32) -> res; actor = [obj+8] (0x2941049) |
| N0804 | src/mod/session2/identity_probe.cpp:51 | kPhasesHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78} | constexpr uint8_t kPhasesHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, |
| N0805 | src/mod/session2/identity_probe.cpp:55 | kAck | RVA / layout / policy (audit) | 0x28F31F0 | constexpr uintptr_t kAck = 0x28F31F0;  // (actor), void: the login path that writes CharacterStatAck 2358 |
| N0806 | src/mod/session2/identity_probe.cpp:56 | kAckHead | signature | {0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0xC8, 0x75, 0xFF, 0xFF, 0xB8, 0x38, 0x8B, 0x00, 0x00, 0xE8, 0xAD, 0x30, 0xFA, 0x01, 0x48, 0x2B, 0xE0, 0xC5, 0xF8, 0x29, 0xB4, 0x24, 0x20, 0x8B, 0x00, 0x00, 0x4C, 0x8B, 0xF1} | constexpr uint8_t kAckHead[] = {0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, |
| N0807 | src/mod/session2/identity_probe.cpp:60 | kPacker | RVA / layout / policy (audit) | 0x17908C0 | constexpr uintptr_t kPacker = 0x17908C0;  // (Status, archive), void: writes **(Status+0x70) |
| N0808 | src/mod/session2/identity_probe.cpp:61 | kPackerHead | signature | {0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x28, 0x4C, 0x8B, 0xF9, 0x48, 0x8B, 0xDA, 0x48, 0x83, 0xC1, 0x30, 0xE8, 0x8D, 0x8C, 0xBF, 0xFE, 0x41, 0xB8, 0x04, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xCB} | constexpr uint8_t kPackerHead[] = {0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, |
| N0809 | src/mod/session2/identity_probe.cpp:67 | kVersionThis | RVA / layout / policy (audit) | 0x179199C | constexpr uintptr_t kVersionThis = 0x179199C; |
| N0810 | src/mod/session2/identity_probe.cpp:68 | kVersionThisBytes | signature | {0x49, 0x8B, 0xD9, 0x4C, 0x8B, 0xE2, 0x48, 0x8B, 0xE9, 0x48, 0x8B, 0x41, 0x08} | constexpr uint8_t kVersionThisBytes[] = {0x49, 0x8B, 0xD9, 0x4C, 0x8B, 0xE2, 0x48, 0x8B, 0xE9, 0x48, 0x8B, 0x41, 0x08}; |
| N0811 | src/mod/session2/identity_probe.cpp:69 | kVersionSite | RVA / layout / policy (audit) | 0x1791B94 | constexpr uintptr_t kVersionSite = 0x1791B94; |
| N0812 | src/mod/session2/identity_probe.cpp:70 | kVersionSiteBytes | signature | {0x41, 0x8B, 0x43, 0x60, 0x89, 0x05, 0xFA, 0xB1, 0x4F, 0x05, 0x4C, 0x8B, 0x1D, 0xA3, 0x7E, 0x5D, 0x05} | constexpr uint8_t kVersionSiteBytes[] = {0x41, 0x8B, 0x43, 0x60, 0x89, 0x05, 0xFA, 0xB1, 0x4F, |
| N0813 | src/mod/session2/identity_probe.cpp:72 | kVersionHook | RVA / layout / policy (audit) | kVersionSite + 4 | constexpr uintptr_t kVersionHook = kVersionSite + 4;  // 0x1791B98 |
| N0814 | src/mod/session2/identity_probe.cpp:75 | kClientStatus | RVA / layout / policy (audit) | 0xB742B0 | constexpr uintptr_t kClientStatus = 0xB742B0; |
| N0815 | src/mod/session2/identity_probe.cpp:76 | kClientStatusBytes | signature | {0x48, 0x8B, 0x4B, 0x70, 0x48, 0x8B, 0x45, 0xC0, 0x48, 0x89, 0x01} | constexpr uint8_t kClientStatusBytes[] = {0x48, 0x8B, 0x4B, 0x70, 0x48, 0x8B, 0x45, 0xC0, 0x48, 0x89, 0x01}; |
| N0816 | src/mod/session2/identity_probe.cpp:77 | kClientVersionSite | RVA / layout / policy (audit) | 0xB742F0 | constexpr uintptr_t kClientVersionSite = 0xB742F0; |
| N0817 | src/mod/session2/identity_probe.cpp:78 | kClientVersionSiteBytes | signature | {0x48, 0x8B, 0x4B, 0x18, 0xE8, 0xB7, 0x69, 0xC3, 0x00, 0x48, 0x8B, 0x4B, 0x18, 0xE8, 0xCE, 0x75, 0xC3, 0x00, 0x44, 0x89, 0x35, 0x8F, 0x8A, 0x11, 0x06} | constexpr uint8_t kClientVersionSiteBytes[] = {0x48, 0x8B, 0x4B, 0x18, 0xE8, 0xB7, 0x69, 0xC3, 0x00, |
| N0818 | src/mod/session2/identity_probe.cpp:81 | kClientVersionHook | RVA / layout / policy (audit) | kClientVersionSite + 18 | constexpr uintptr_t kClientVersionHook = kClientVersionSite + 18;  // 0xB74302 |
| N0819 | src/mod/session2/identity_probe.cpp:82 | kRestoreVersion | RVA / layout / policy (audit) | 0x6C8CD98 | constexpr uintptr_t kRestoreVersion = 0x6C8CD98;  // u32 _currentRestoreCodeVersionXXX (0x187C490) |
| N0820 | src/mod/session2/identity_probe.cpp:83 | kStatusVtable | vtable | 0x5B203F0 | constexpr uintptr_t kStatusVtable = 0x5B203F0;    // ServerStatusActorComponent (RTTI at its COL) |
| N0821 | src/mod/session2/identity_probe.cpp:84 | kStatusLoad | RVA / layout / policy (audit) | 0x293FE70 | constexpr uintptr_t kStatusLoad = 0x293FE70;      // its +0x50: the 5-byte thunk to .xtls 0x10427500 |
| N0822 | src/mod/session2/identity_probe.cpp:85 | kLoadStub | RVA / layout / policy (audit) | 0x35EA90 | constexpr uintptr_t kLoadStub = 0x35EA90;         // the shared vf50 stub (jmp .xtls 0x86F0570) |
| N0823 | src/mod/session2/identity_probe.cpp:94 | kMaxHostLines | RVA / layout / policy (audit) | 2 | constexpr uint64_t kMaxHostLines = 2; |
| N0824 | src/mod/session2/identity_probe.cpp:95 | kMaxLogged | RVA / layout / policy (audit) | 160 | constexpr uint64_t kMaxLogged = 160; |
| N0825 | src/mod/session2/identity_probe.cpp:98 | kWindowMs | RVA / layout / policy (audit) | 60000 | constexpr uint64_t kWindowMs = 60000; |
| N0826 | src/mod/session2/identity_probe.cpp:136 | kMaxDepth | RVA / layout / policy (audit) | 4 | constexpr int kMaxDepth = 4; |
| N0827 | src/mod/session2/identity_probe.h:48 | kActorComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActorComponents = 0x68;   // actor -> component table (0x206A890, 0x2AE1037) |
| N0828 | src/mod/session2/identity_probe.h:49 | kTableStatus | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kTableStatus = 0x20;       // table -> Status component (0x206A894) |
| N0829 | src/mod/session2/identity_probe.h:50 | kStatusCharacterNo | RVA / layout / policy (audit) | 0x70 | inline constexpr uint32_t kStatusCharacterNo = 0x70; // Status -> u64* characterNo (0x206A8A5) |
| N0830 | src/mod/session2/identity_probe.h:51 | kComponentOwner | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kComponentOwner = 0x08;    // component -> actor (0x2AE102A, 0x17919A5) |
| N0831 | src/mod/session2/identity_probe.h:52 | kActorUser | RVA / layout / policy (audit) | 0xA0 | inline constexpr uint32_t kActorUser = 0xA0;         // character -> ServerUserActor (clock gate, P9 §2.2) |
| N0832 | src/mod/session2/identity_probe.h:53 | kKey | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kKey = 0x60;               // actor/user -> u32 key (offsets_s2.h) |
| N0833 | src/mod/session2/load_trip.cpp:22 | kMaxLines | RVA / layout / policy (audit) | 3000 | constexpr uint64_t kMaxLines = 3000; |
| N0834 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldAck | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0835 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldCompleteAck | Troc ID (audit) | 2144 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0836 | src/mod/session2/load_trip_logic.h:32 | kOpPostAck | Troc ID (audit) | 3066 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0837 | src/mod/session2/load_trip_logic.h:32 | kOpMilestone | Troc ID (audit) | 2322 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0838 | src/mod/session2/load_trip_logic.h:32 | kOpLoadingCompleteAck | Troc ID (audit) | 2762 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0839 | src/mod/session2/load_trip_logic.h:32 | kOpBundle | Troc ID (audit) | 2042 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0840 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldCompleteReq | Troc ID (audit) | 2549 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0841 | src/mod/session2/load_trip_logic.h:32 | kOpLoadingCompleteReq | Troc ID (audit) | 2445 | inline constexpr uint16_t kOpMoveFieldAck = 2174, kOpMoveFieldCompleteAck = 2144, kOpPostAck = 3066, |
| N0842 | src/mod/session2/load_trip_logic.h:46 | kSettleUs | RVA / layout / policy (audit) | 6'000'000 | inline constexpr uint64_t kSettleUs = 6'000'000; |
| N0843 | src/mod/session2/load_trip_logic.h:48 | kMaxTrips | RVA / layout / policy (audit) | 8 | inline constexpr size_t kMaxTrips = 8; |
| N0844 | src/mod/session2/mount_sync_logic.h:45 | kMaxSeats | RVA / layout / policy (audit) | 16 | inline constexpr uint8_t kMaxSeats = 16; |
| N0845 | src/mod/session2/mount_sync_logic.h:49 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1;     // troc_frame.h kControlOpcode |
| N0846 | src/mod/session2/mount_sync_logic.h:50 | kControlMountSync | RVA / layout / policy (audit) | 5 | inline constexpr uint8_t kControlMountSync = 5;   // troc_frame.h: 1 ready, 2 dead, 3 reannounce, 4 host character |
| N0847 | src/mod/session2/mount_sync_logic.h:51 | kVersion | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kVersion = 1; |
| N0848 | src/mod/session2/mount_sync_logic.h:52 | kHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kHeader = 5;              // u16 opcode &#124; i8 slot &#124; u16 len |
| N0849 | src/mod/session2/mount_sync_logic.h:53 | kPayloadSize | RVA / layout / policy (audit) | 44 | inline constexpr size_t kPayloadSize = 44; |
| N0850 | src/mod/session2/mount_sync_logic.h:69 | kFlagOk | RVA / layout / policy (audit) | 0x01 | inline constexpr uint8_t kFlagOk = 0x01;            // Commit/Detach: *result == 0 and the controller's postconditions held |
| N0851 | src/mod/session2/mount_sync_logic.h:70 | kFlagRiderWasLinked | RVA / layout / policy (audit) | 0x02 | inline constexpr uint8_t kFlagRiderWasLinked = 0x02;  // Begin: the rider's Link+0x10 was not 0 |
| N0852 | src/mod/session2/mount_sync_logic.h:71 | kFlagSeatWasTaken | RVA / layout / policy (audit) | 0x04 | inline constexpr uint8_t kFlagSeatWasTaken = 0x04;    // Begin: the seat's occupant was not 0 |
| N0853 | src/mod/session2/mount_sync_logic.h:76 | kFlagObserved | RVA / layout / policy (audit) | 0x08 | inline constexpr uint8_t kFlagObserved = 0x08; |
| N0854 | src/mod/session2/mount_sync_logic.h:80 | kClientDetachFlag | RVA / layout / policy (audit) | 12 | inline constexpr uint8_t kClientDetachFlag = 12; |
| N0855 | src/mod/session2/mount_sync_logic.h:83 | kFlagNoPublish | RVA / layout / policy (audit) | 0x10 | inline constexpr uint8_t kFlagNoPublish = 0x10; |
| N0856 | src/mod/session2/mount_sync_logic.h:168 | kStatusEveryMs | RVA / layout / policy (audit) | 15000 | inline constexpr uint64_t kStatusEveryMs = 15000; |
| N0857 | src/mod/session2/mount_sync_logic.h:169 | kStatusShowMs | RVA / layout / policy (audit) | 45000 | inline constexpr uint64_t kStatusShowMs = 45000; |
| N0858 | src/mod/session2/mount_sync_logic.h:224 | kDepRiderComps | RVA / layout / policy (audit) | 0x001 | inline constexpr uint32_t kDepRiderComps = 0x001;   // rider actor+0x68 |
| N0859 | src/mod/session2/mount_sync_logic.h:225 | kDepRiderCC | RVA / layout / policy (audit) | 0x002 | inline constexpr uint32_t kDepRiderCC = 0x002;      // rider comps+0x40 (CC; +0xA8 read, line 261) |
| N0860 | src/mod/session2/mount_sync_logic.h:226 | kDepRiderCCSlot | RVA / layout / policy (audit) | 0x004 | inline constexpr uint32_t kDepRiderCCSlot = 0x004;  // CC+0xA8 == 0: [CC+0x30]+0x1B and the pointer at CC+0x88+8*it |
| N0861 | src/mod/session2/mount_sync_logic.h:227 | kDepRiderComps50 | RVA / layout / policy (audit) | 0x008 | inline constexpr uint32_t kDepRiderComps50 = 0x008; // rider comps+0x50 (writes +0x11C, line 282) |
| N0862 | src/mod/session2/mount_sync_logic.h:228 | kDepHorseComps | RVA / layout / policy (audit) | 0x010 | inline constexpr uint32_t kDepHorseComps = 0x010;   // horse actor+0x68 |
| N0863 | src/mod/session2/mount_sync_logic.h:229 | kDepHorseComps1A0 | RVA / layout / policy (audit) | 0x020 | inline constexpr uint32_t kDepHorseComps1A0 = 0x020;// horse comps+0x1A0 (+0x98, line 213) |
| N0864 | src/mod/session2/mount_sync_logic.h:230 | kDepHorseComps90 | RVA / layout / policy (audit) | 0x040 | inline constexpr uint32_t kDepHorseComps90 = 0x040; // horse comps+0x90 (0x221C200, seat 0) |
| N0865 | src/mod/session2/mount_sync_logic.h:231 | kDepHorse88 | RVA / layout / policy (audit) | 0x080 | inline constexpr uint32_t kDepHorse88 = 0x080;      // horse actor+0x88 (+1, the type switches) |
| N0866 | src/mod/session2/mount_sync_logic.h:232 | kDepHorseCC | RVA / layout / policy (audit) | 0x100 | inline constexpr uint32_t kDepHorseCC = 0x100;      // horse comps+0x40 (CC->vf260) |
| N0867 | src/mod/session2/mount_sync_logic.h:233 | kDepHorseCompsB0 | RVA / layout / policy (audit) | 0x200 | inline constexpr uint32_t kDepHorseCompsB0 = 0x200; // horse comps+0xB0 (+0x288 / +0x300, conditional) |
| N0868 | src/mod/session2/mount_sync_logic.h:276 | kMethodRideOn | RVA / layout / policy (audit) | 6 | inline constexpr uint8_t kMethodRideOn = 6, kMethodRideOff = 7; |
| N0869 | src/mod/session2/mount_sync_logic.h:276 | kMethodRideOff | RVA / layout / policy (audit) | 7 | inline constexpr uint8_t kMethodRideOn = 6, kMethodRideOff = 7; |
| N0870 | src/mod/session2/mount_sync_logic.h:381 | kCtlGoneMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint64_t kCtlGoneMs = 1000; |
| N0871 | src/mod/session2/mount_sync_logic.h:410 | kGeoSpeeds | RVA / layout / policy (audit) | 5 | inline constexpr size_t kGeoSpeeds = 5;  // horse speed m/s: <1, <3, <6, <10, >=10 |
| N0872 | src/mod/session2/mount_sync_logic.h:411 | kGeoErrs | RVA / layout / policy (audit) | 6 | inline constexpr size_t kGeoErrs = 6;    // &#124;offset - seat&#124; m: <=0.25, <=0.5, <=1, <=2, <=5, >5 |
| N0873 | src/mod/session2/mount_sync_logic.h:412 | kGeoEveryMs | RVA / layout / policy (audit) | 30 | inline constexpr uint64_t kGeoEveryMs = 30; |
| N0874 | src/mod/session2/mount_sync_logic.h:434 | kGeoNotLinked | RVA / layout / policy (audit) | 0 | inline constexpr size_t kGeoNotLinked = 0, kGeoLinked = 1, kGeoSeated = 2, kGeoLinks = 3; |
| N0875 | src/mod/session2/mount_sync_logic.h:434 | kGeoLinked | RVA / layout / policy (audit) | 1 | inline constexpr size_t kGeoNotLinked = 0, kGeoLinked = 1, kGeoSeated = 2, kGeoLinks = 3; |
| N0876 | src/mod/session2/mount_sync_logic.h:434 | kGeoSeated | RVA / layout / policy (audit) | 2 | inline constexpr size_t kGeoNotLinked = 0, kGeoLinked = 1, kGeoSeated = 2, kGeoLinks = 3; |
| N0877 | src/mod/session2/mount_sync_logic.h:434 | kGeoLinks | RVA / layout / policy (audit) | 3 | inline constexpr size_t kGeoNotLinked = 0, kGeoLinked = 1, kGeoSeated = 2, kGeoLinks = 3; |
| N0878 | src/mod/session2/mount_sync_logic.h:438 | kAttachNone | RVA / layout / policy (audit) | 9 | inline constexpr uint8_t kAttachNone = 9, kAttachSeat = 4; |
| N0879 | src/mod/session2/mount_sync_logic.h:438 | kAttachSeat | RVA / layout / policy (audit) | 4 | inline constexpr uint8_t kAttachNone = 9, kAttachSeat = 4; |
| N0880 | src/mod/session2/mount_sync_logic.h:444 | kFrozenRiderM | RVA / layout / policy (audit) | 0.01f | inline constexpr float kFrozenRiderM = 0.01f, kFrozenHorseM = 0.5f; |
| N0881 | src/mod/session2/mount_sync_logic.h:444 | kFrozenHorseM | RVA / layout / policy (audit) | 0.5f | inline constexpr float kFrozenRiderM = 0.01f, kFrozenHorseM = 0.5f; |
| N0882 | src/mod/session2/mount_sync_logic.h:492 | kCommitWaitMs | RVA / layout / policy (audit) | 500 | inline constexpr uint64_t kCommitWaitMs = 500;     // a Begin holds the observed stream at most this long |
| N0883 | src/mod/session2/mount_sync_logic.h:493 | kApplyWaitMs | RVA / layout / policy (audit) | 3000 | inline constexpr uint64_t kApplyWaitMs = 3000;     // a Commit not applied by then is cancelled (fallback) |
| N0884 | src/mod/session2/mount_sync_logic.h:494 | kDetachForceMs | RVA / layout / policy (audit) | 3000 | inline constexpr uint64_t kDetachForceMs = 3000;   // a Detach without the observer's RideOff::Enter: applied anyway |
| N0885 | src/mod/session2/mount_sync_logic.h:498 | kDetachObservedForceMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint64_t kDetachObservedForceMs = 1000; |
| N0886 | src/mod/session2/mount_sync_logic.h:502 | kDetachNoEventMs | RVA / layout / policy (audit) | 6000 | inline constexpr uint64_t kDetachNoEventMs = 6000; |
| N0887 | src/mod/session2/mount_sync_logic.h:505 | kClaimStuckMs | RVA / layout / policy (audit) | 2000 | inline constexpr uint64_t kClaimStuckMs = 2000; |
| N0888 | src/mod/session2/mount_sync_logic.h:508 | kMaxAttempts | RVA / layout / policy (audit) | 20 | inline constexpr uint32_t kMaxAttempts = 20; |
| N0889 | src/mod/session2/mount_sync_logic.h:509 | kHoldMaxPackets | RVA / layout / policy (audit) | 2048 | inline constexpr size_t kHoldMaxPackets = 2048;    // the guest's held stream |
| N0890 | src/mod/session2/mount_sync_logic.h:510 | kHoldMaxBytes | RVA / layout / policy (audit) | 4u << 20 | inline constexpr size_t kHoldMaxBytes = 4u << 20; |
| N0891 | src/mod/session2/mount_sync_logic.h:511 | kHoldMaxReq | RVA / layout / policy (audit) | 256 | inline constexpr size_t kHoldMaxReq = 256;         // the host's held user 2 Req (its own bound is 512) |
| N0892 | src/mod/session2/mount_sync_logic.h:628 | kNativeFallbackMs | RVA / layout / policy (audit) | 2000 | inline constexpr uint64_t kNativeFallbackMs = 2000; |
| N0893 | src/mod/session2/mount_sync_logic.h:630 | kNativeFallbackCapMs | RVA / layout / policy (audit) | 30000 | inline constexpr uint64_t kNativeFallbackCapMs = 30000; |
| N0894 | src/mod/session2/mount_sync_logic.h:634 | kNativeUnreadableMs | RVA / layout / policy (audit) | 15000 | inline constexpr uint64_t kNativeUnreadableMs = 15000; |
| N0895 | src/mod/session2/mount_sync_logic.h:635 | kNativeDetachFlag | RVA / layout / policy (audit) | 0 | inline constexpr uint8_t kNativeDetachFlag = 0; |
| N0896 | src/mod/session2/mount_sync_logic.h:682 | kSlots | RVA / layout / policy (audit) | 4 | static constexpr size_t kSlots = 4; |
| N0897 | src/mod/session2/mount_sync_logic.h:1098 | kGateEntrySize | RVA / layout / policy (audit) | 0x78 | inline constexpr uint32_t kGateEntrySize = 0x78; |
| N0898 | src/mod/session2/mount_sync_logic.h:1099 | kGateEntries | RVA / layout / policy (audit) | 2 | inline constexpr uint32_t kGateEntries = 2; |
| N0899 | src/mod/session2/mount_sync_logic.h:1100 | kGateBytes | RVA / layout / policy (audit) | kGateEntrySize * kGateEntries | inline constexpr uint32_t kGateBytes = kGateEntrySize * kGateEntries; |
| N0900 | src/mod/session2/mount_sync_logic.h:1101 | kEntryHash | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kEntryHash = 0x00, kEntryDef = 0x18, kEntryNet = 0x50, kEntryForce = 0x51, kEntryLocal = 0x52; |
| N0901 | src/mod/session2/mount_sync_logic.h:1101 | kEntryDef | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kEntryHash = 0x00, kEntryDef = 0x18, kEntryNet = 0x50, kEntryForce = 0x51, kEntryLocal = 0x52; |
| N0902 | src/mod/session2/mount_sync_logic.h:1101 | kEntryNet | RVA / layout / policy (audit) | 0x50 | inline constexpr uint32_t kEntryHash = 0x00, kEntryDef = 0x18, kEntryNet = 0x50, kEntryForce = 0x51, kEntryLocal = 0x52; |
| N0903 | src/mod/session2/mount_sync_logic.h:1101 | kEntryForce | RVA / layout / policy (audit) | 0x51 | inline constexpr uint32_t kEntryHash = 0x00, kEntryDef = 0x18, kEntryNet = 0x50, kEntryForce = 0x51, kEntryLocal = 0x52; |
| N0904 | src/mod/session2/mount_sync_logic.h:1101 | kEntryLocal | RVA / layout / policy (audit) | 0x52 | inline constexpr uint32_t kEntryHash = 0x00, kEntryDef = 0x18, kEntryNet = 0x50, kEntryForce = 0x51, kEntryLocal = 0x52; |
| N0905 | src/mod/session2/mount_sync_logic.h:1102 | kEntryNetBit | RVA / layout / policy (audit) | 0x02 | inline constexpr uint8_t kEntryNetBit = 0x02, kEntryForceBit = 0x08, kEntryLocalBit = 0x01; |
| N0906 | src/mod/session2/mount_sync_logic.h:1102 | kEntryForceBit | RVA / layout / policy (audit) | 0x08 | inline constexpr uint8_t kEntryNetBit = 0x02, kEntryForceBit = 0x08, kEntryLocalBit = 0x01; |
| N0907 | src/mod/session2/mount_sync_logic.h:1102 | kEntryLocalBit | RVA / layout / policy (audit) | 0x01 | inline constexpr uint8_t kEntryNetBit = 0x02, kEntryForceBit = 0x08, kEntryLocalBit = 0x01; |
| N0908 | src/mod/session2/mount_sync_logic.h:1103 | kDefDismount | RVA / layout / policy (audit) | 0x40 | inline constexpr uint8_t kDefDismount = 0x40;  // def+0xA: "get off the vehicle" |
| N0909 | src/mod/session2/mount_sync_logic.h:1104 | kDefFlags | RVA / layout / policy (audit) | 0x0A | inline constexpr uint32_t kDefFlags = 0x0A; |
| N0910 | src/mod/session2/mount_sync_logic.h:1187 | kGateDegradeAfter | RVA / layout / policy (audit) | 8 | inline constexpr uint32_t kGateDegradeAfter = 8; |
| N0911 | src/mod/session2/mount_sync_logic.h:1243 | kActorPlayer | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kActorPlayer = 1, kActorWorld = 7; |
| N0912 | src/mod/session2/mount_sync_logic.h:1243 | kActorWorld | RVA / layout / policy (audit) | 7 | inline constexpr uint8_t kActorPlayer = 1, kActorWorld = 7; |
| N0913 | src/mod/session2/mount_sync_logic.h:1270 | kVf178Actor | RVA / layout / policy (audit) | 0x4FE260 | inline constexpr uint32_t kVf178Actor = 0x4FE260, kVf178User = 0x8A67D0; |
| N0914 | src/mod/session2/mount_sync_logic.h:1270 | kVf178User | RVA / layout / policy (audit) | 0x8A67D0 | inline constexpr uint32_t kVf178Actor = 0x4FE260, kVf178User = 0x8A67D0; |
| N0915 | src/mod/session2/mount_sync_logic.h:1382 | kSeatReleaseMs | RVA / layout / policy (audit) | 1500 | inline constexpr uint64_t kSeatReleaseMs = 1500; |
| N0916 | src/mod/session2/mount_sync_logic.h:1383 | kSeatRideCapMs | RVA / layout / policy (audit) | 6000 | inline constexpr uint64_t kSeatRideCapMs = 6000; |
| N0917 | src/mod/session2/mount_sync_logic.h:1384 | kSeatLostMs | RVA / layout / policy (audit) | 3000 | inline constexpr uint64_t kSeatLostMs = 3000;  // the rider unreadable this long: forgotten (its actor is gone) |
| N0918 | src/mod/session2/mount_sync_logic.h:1499 | kMcEmptyFlag | RVA / layout / policy (audit) | 0x3D0 | inline constexpr uint32_t kMcEmptyFlag = 0x3D0; |
| N0919 | src/mod/session2/mount_sync_logic.h:1529 | kOpStartActionReq | Troc ID (audit) | 2223 | inline constexpr uint16_t kOpStartActionReq = 2223; |
| N0920 | src/mod/session2/observer_hold_acks_logic.h:41 | kOpAnnounce | Troc ID (audit) | 2005 | inline constexpr uint16_t kOpAnnounce = 2005;     // ActorLoadDataAck |
| N0921 | src/mod/session2/observer_hold_acks_logic.h:42 | kOpScopeSender | Troc ID (audit) | 2042 | inline constexpr uint16_t kOpScopeSender = 2042;  // ScopeSenderAck: a native bundle (contents not parsed) |
| N0922 | src/mod/session2/observer_hold_acks_logic.h:43 | kNpcKeyClass | RVA / layout / policy (audit) | 0xB | inline constexpr uint32_t kNpcKeyClass = 0xB;     // key >> 28 of NPCs and children |
| N0923 | src/mod/session2/observer_hold_acks_logic.h:46 | kHeldOps | signature | { 2723, 2404, 2586, 2991, 2697, 2642, 2506, 2402, 2645, 2434, 2356, } | inline constexpr std::array<uint16_t, 11> kHeldOps = { |
| N0924 | src/mod/session2/observer_hold_acks_logic.h:59 | kOpCount | Troc ID (audit) | kHeldOps.size() | inline constexpr size_t kOpCount = kHeldOps.size(); |
| N0925 | src/mod/session2/observer_hold_acks_logic.h:86 | kCharKeyClass | RVA / layout / policy (audit) | 0xA | inline constexpr uint32_t kCharKeyClass = 0xA; |
| N0926 | src/mod/session2/observer_milestone_logic.h:46 | kOpMoveFieldAck | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpMoveFieldAck = 2174; |
| N0927 | src/mod/session2/observer_milestone_logic.h:47 | kOpMoveFieldCompleteAck | Troc ID (audit) | 2144 | inline constexpr uint16_t kOpMoveFieldCompleteAck = 2144; |
| N0928 | src/mod/session2/observer_milestone_logic.h:48 | kOpMoveFieldCompletePostAck | Troc ID (audit) | 3066 | inline constexpr uint16_t kOpMoveFieldCompletePostAck = 3066; |
| N0929 | src/mod/session2/observer_milestone_logic.h:49 | kOpMilestone | Troc ID (audit) | 2322 | inline constexpr uint16_t kOpMilestone = 2322; |
| N0930 | src/mod/session2/observer_milestone_logic.h:50 | kOpLoadingCompleteAck | Troc ID (audit) | 2762 | inline constexpr uint16_t kOpLoadingCompleteAck = 2762; |
| N0931 | src/mod/session2/observer_milestone_logic.h:51 | kOpScopeSenderAck | Troc ID (audit) | 2042 | inline constexpr uint16_t kOpScopeSenderAck = 2042;   // a native bundle (scope sender): contents not parsed |
| N0932 | src/mod/session2/observer_milestone_logic.h:52 | kOpMoveFieldCompleteReq | Troc ID (audit) | 2549 | inline constexpr uint16_t kOpMoveFieldCompleteReq = 2549; |
| N0933 | src/mod/session2/observer_milestone_logic.h:53 | kOpLoadingCompleteReq | Troc ID (audit) | 2445 | inline constexpr uint16_t kOpLoadingCompleteReq = 2445; |
| N0934 | src/mod/session2/observer_milestone_reload_logic.h:57 | kOpPreLoadSequencerReq | Troc ID (audit) | 2873 | inline constexpr uint16_t kOpPreLoadSequencerReq = 2873; |
| N0935 | src/mod/session2/observer_milestone_reload_logic.h:58 | kOpReloadStageStartReq | Troc ID (audit) | 2283 | inline constexpr uint16_t kOpReloadStageStartReq = 2283; |
| N0936 | src/mod/session2/observer_milestone_reload_logic.h:59 | kOpGameLoadingStartReq | Troc ID (audit) | 2437 | inline constexpr uint16_t kOpGameLoadingStartReq = 2437; |
| N0937 | src/mod/session2/observer_milestone_reload_logic.h:60 | kOpMoveFieldAck | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpMoveFieldAck = 2174; |
| N0938 | src/mod/session2/observer_milestone_reload_logic.h:61 | kOpLoadingFieldDataAck | Troc ID (audit) | 2643 | inline constexpr uint16_t kOpLoadingFieldDataAck = 2643; |
| N0939 | src/mod/session2/observer_milestone_reload_logic.h:62 | kOpMoveFieldCompletePostAck | Troc ID (audit) | 3066 | inline constexpr uint16_t kOpMoveFieldCompletePostAck = 3066; |
| N0940 | src/mod/session2/observer_milestone_reload_logic.h:63 | kOpMilestone | Troc ID (audit) | 2322 | inline constexpr uint16_t kOpMilestone = 2322; |
| N0941 | src/mod/session2/observer_milestone_reload_logic.h:64 | kOpLoadingCompleteAck | Troc ID (audit) | 2762 | inline constexpr uint16_t kOpLoadingCompleteAck = 2762; |
| N0942 | src/mod/session2/observer_milestone_reload_logic.h:65 | kOpLogoutCharacterAck | Troc ID (audit) | 2679 | inline constexpr uint16_t kOpLogoutCharacterAck = 2679; |
| N0943 | src/mod/session2/observer_milestone_reload_logic.h:66 | kOpReviveLoadingReq | Troc ID (audit) | 2362 | inline constexpr uint16_t kOpReviveLoadingReq = 2362;  // P8 S27: the revive's load request (C>S) |
| N0944 | src/mod/session2/observer_milestone_reload_logic.h:68 | kMinDelayUs | RVA / layout / policy (audit) | 300'000 | inline constexpr uint64_t kMinDelayUs = 300'000;     // native: the 2322 leaves 460-501 ms after the 2873 |
| N0945 | src/mod/session2/observer_milestone_reload_logic.h:69 | kTimeoutUs | RVA / layout / policy (audit) | 15'000'000 | inline constexpr uint64_t kTimeoutUs = 15'000'000;   // no fitting Req of user 2 within 15 s: given up (logged) |
| N0946 | src/mod/session2/observer_milestone_reload_logic.h:70 | kAttemptGapUs | RVA / layout / policy (audit) | 50'000 | inline constexpr uint64_t kAttemptGapUs = 50'000;    // transient refusals count at most once per 50 ms |
| N0947 | src/mod/session2/observer_milestone_reload_logic.h:71 | kMaxAttempts | RVA / layout / policy (audit) | 400 | inline constexpr uint32_t kMaxAttempts = 400;        // > kTimeoutUs / kAttemptGapUs: the timeout governs |
| N0948 | src/mod/session2/observer_milestone_reload_logic.h:72 | kMaxAgeUs | RVA / layout / policy (audit) | 45'000'000 | inline constexpr uint64_t kMaxAgeUs = 45'000'000;    // a 2873 later than this after its trip's 2437 passes natively |
| N0949 | src/mod/session2/observer_milestone_reload_logic.h:73 | kNativeInReloadLines | RVA / layout / policy (audit) | 16 | inline constexpr uint64_t kNativeInReloadLines = 16; // P8 S27: warnings for a 2873 handed during an unfinished reload |
| N0950 | src/mod/session2/observer_milestone_reload_logic.h:293 | kNoteBase | RVA / layout / policy (audit) | 64 | inline constexpr uint16_t kNoteBase = 64; |
| N0951 | src/mod/session2/offsets_s2.h:12 | kServerNetSlot | RVA / layout / policy (audit) | 0x6D69680 | inline constexpr uintptr_t kServerNetSlot = 0x6D69680;  // [exe+..] = ServerNet |
| N0952 | src/mod/session2/offsets_s2.h:13 | kClientNetSlot | RVA / layout / policy (audit) | 0x6D69170 | inline constexpr uintptr_t kClientNetSlot = 0x6D69170;  // [exe+..] = ClientNet |
| N0953 | src/mod/session2/offsets_s2.h:14 | kSamSlot | RVA / layout / policy (audit) | 0x6DA67D8 | inline constexpr uintptr_t kSamSlot = 0x6DA67D8;        // [exe+..] = ServerActorManager |
| N0954 | src/mod/session2/offsets_s2.h:17 | kServerNet_Session | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kServerNet_Session = 0x18;  // the only NwVirtualAsyncSession |
| N0955 | src/mod/session2/offsets_s2.h:18 | kServerNet_InQueue | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kServerNet_InQueue = 0x20;  // Q0: C->S buffers, popped by ServerRunThread |
| N0956 | src/mod/session2/offsets_s2.h:19 | kServerNet_FreeC2S | RVA / layout / policy (audit) | 0x78 | inline constexpr uint32_t kServerNet_FreeC2S = 0x78;  // Q3: C->S buffers back to the client |
| N0957 | src/mod/session2/offsets_s2.h:20 | kServerNet_FreeS2C | RVA / layout / policy (audit) | 0x80 | inline constexpr uint32_t kServerNet_FreeS2C = 0x80;  // Q2: S->C buffers back to the server |
| N0958 | src/mod/session2/offsets_s2.h:22 | kClientNet_InQueue | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kClientNet_InQueue = 0x18;  // Q1 (== session+0xC8) |
| N0959 | src/mod/session2/offsets_s2.h:24 | kSam_FreeKeys | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kSam_FreeKeys = 0xD0;     // u32: keys left in the common queue (15501 at most) |
| N0960 | src/mod/session2/offsets_s2.h:25 | kSam_ActorCount | RVA / layout / policy (audit) | 0x9C | inline constexpr uint32_t kSam_ActorCount = 0x9C;   // u32 |
| N0961 | src/mod/session2/offsets_s2.h:28 | kNwVirtualAsyncSessionVtable | vtable | 0x57260C0 | inline constexpr uintptr_t kNwVirtualAsyncSessionVtable = 0x57260C0;  // 15 entries, COL at [-1] |
| N0962 | src/mod/session2/offsets_s2.h:29 | kNwAsyncSessionVtable | vtable | 0x5726140 | inline constexpr uintptr_t kNwAsyncSessionVtable = 0x5726140; |
| N0963 | src/mod/session2/offsets_s2.h:30 | kSessionVtableEntries | vtable | 15 | inline constexpr uint32_t kSessionVtableEntries = 15; |
| N0964 | src/mod/session2/offsets_s2.h:31 | kSessionVfSend | RVA / layout / policy (audit) | 9 | inline constexpr uint32_t kSessionVfSend = 9; |
| N0965 | src/mod/session2/offsets_s2.h:32 | kSessionSize | RVA / layout / policy (audit) | 0xD0 | inline constexpr uint32_t kSessionSize = 0xD0; |
| N0966 | src/mod/session2/offsets_s2.h:33 | kSession_Name | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kSession_Name = 0x18;     // u16 short string: "1" for the host |
| N0967 | src/mod/session2/offsets_s2.h:34 | kSession_IdPtr | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kSession_IdPtr = 0x88;    // u32*: key of the deferred queue until a user exists |
| N0968 | src/mod/session2/offsets_s2.h:35 | kSession_User | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kSession_User = 0x90;     // ServerUserActor* |
| N0969 | src/mod/session2/offsets_s2.h:36 | kSession_Crypto | RVA / layout / policy (audit) | 0x98 | inline constexpr uint32_t kSession_Crypto = 0x98;   // CryptogramAesGcm*; byte +0x24 = 0 (off) |
| N0970 | src/mod/session2/offsets_s2.h:37 | kSession_B0 | RVA / layout / policy (audit) | 0xB0 | inline constexpr uint32_t kSession_B0 = 0xB0; |
| N0971 | src/mod/session2/offsets_s2.h:38 | kSession_B8 | RVA / layout / policy (audit) | 0xB8 | inline constexpr uint32_t kSession_B8 = 0xB8; |
| N0972 | src/mod/session2/offsets_s2.h:39 | kSession_C0 | RVA / layout / policy (audit) | 0xC0 | inline constexpr uint32_t kSession_C0 = 0xC0;       // u16 |
| N0973 | src/mod/session2/offsets_s2.h:40 | kSession_OutQueue | RVA / layout / policy (audit) | 0xC8 | inline constexpr uint32_t kSession_OutQueue = 0xC8; |
| N0974 | src/mod/session2/offsets_s2.h:43 | kServerUserActorVtable | vtable | 0x5B257A8 | inline constexpr uintptr_t kServerUserActorVtable = 0x5B257A8; |
| N0975 | src/mod/session2/offsets_s2.h:44 | kUser_Key | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kUser_Key = 0x60; |
| N0976 | src/mod/session2/offsets_s2.h:45 | kUser_Session | RVA / layout / policy (audit) | 0x1F8 | inline constexpr uint32_t kUser_Session = 0x1F8; |
| N0977 | src/mod/session2/offsets_s2.h:47 | kServerActor_Controller | RVA / layout / policy (audit) | 0x90 | inline constexpr uint32_t kServerActor_Controller = 0x90; |
| N0978 | src/mod/session2/offsets_s2.h:48 | kServerActor_Key | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kServerActor_Key = 0x60; |
| N0979 | src/mod/session2/offsets_s2.h:51 | kServerOnRecvPacket | RVA / layout / policy (audit) | 0x2773C20 | inline constexpr uintptr_t kServerOnRecvPacket = 0x2773C20;  // (ServerNet*, int*, NwSession*, u8* pkt, u16 size, u8 channel) |
| N0980 | src/mod/session2/offsets_s2.h:52 | kServerOnRecvPacketHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54} | inline constexpr uint8_t kServerOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, |
| N0981 | src/mod/session2/offsets_s2.h:54 | kSessionSend | RVA / layout / policy (audit) | 0x1437740 | inline constexpr uintptr_t kSessionSend = 0x1437740;  // vf9 (this, int* res, AsyncRequestSend* rq) -> res |
| N0982 | src/mod/session2/offsets_s2.h:55 | kSessionSendHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30} | inline constexpr uint8_t kSessionSendHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, |
| N0983 | src/mod/session2/offsets_s2.h:57 | kNwSessionCtor | RVA / layout / policy (audit) | 0x1436EE0 | inline constexpr uintptr_t kNwSessionCtor = 0x1436EE0;  // (this, bool) -> this |
| N0984 | src/mod/session2/offsets_s2.h:58 | kNwSessionCtorHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x6C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56} | inline constexpr uint8_t kNwSessionCtorHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x6C, 0x24, 0x20, |
| N0985 | src/mod/session2/offsets_s2.h:60 | kQueuePush | RVA / layout / policy (audit) | 0x35DF50 | inline constexpr uintptr_t kQueuePush = 0x35DF50;  // (Queue*, void** item) |
| N0986 | src/mod/session2/offsets_s2.h:61 | kQueuePushHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9} | inline constexpr uint8_t kQueuePushHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, |
| N0987 | src/mod/session2/offsets_s2.h:71 | kAllocGlobal | RVA / layout / policy (audit) | 0x47EFA78 | inline constexpr uintptr_t kAllocGlobal = 0x47EFA78;  // (size, align) -> void*; global heap with a lock |
| N0988 | src/mod/session2/offsets_s2.h:72 | kAllocGlobalHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA} | inline constexpr uint8_t kAllocGlobalHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, |
| N0989 | src/mod/session2/offsets_s2.h:74 | kAllocThread | RVA / layout / policy (audit) | 0x47EF9B0 | inline constexpr uintptr_t kAllocThread = 0x47EF9B0;  // (size, align) -> void*; the thread's own heap |
| N0990 | src/mod/session2/offsets_s2.h:76 | kAllocThreadBytes | signature | { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0x7E, 0x92, 0x49, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF} | inline constexpr uint8_t kAllocThreadBytes[] = { |
| N0991 | src/mod/session2/offsets_s2.h:83 | kTlsIndexSlot | RVA / layout / policy (audit) | 0x6C88C50 | inline constexpr uintptr_t kTlsIndexSlot = 0x6C88C50;  // u32 _tls_index (0 in the dumps) |
| N0992 | src/mod/session2/offsets_s2.h:84 | kTls_UseGlobalHeap | RVA / layout / policy (audit) | 0x1FD | inline constexpr uint32_t kTls_UseGlobalHeap = 0x1FD;  // u8: the allocator test above |
| N0993 | src/mod/session2/offsets_s2.h:89 | kServerNet_Threads | RVA / layout / policy (audit) | 0xF8 | inline constexpr uint32_t kServerNet_Threads = 0xF8; |
| N0994 | src/mod/session2/offsets_s2.h:90 | kServerNet_ThreadCount | RVA / layout / policy (audit) | 0x100 | inline constexpr uint32_t kServerNet_ThreadCount = 0x100; |
| N0995 | src/mod/session2/offsets_s2.h:91 | kServerThreadEntry | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kServerThreadEntry = 0x10; |
| N0996 | src/mod/session2/offsets_s2.h:95 | kToNearExceptOneControlVtable | vtable | 0x5B25358 | inline constexpr uintptr_t kToNearExceptOneControlVtable = 0x5B25358; |
| N0997 | src/mod/session2/offsets_s2.h:96 | kToNearExceptOneSelfVtable | vtable | 0x5B25378 | inline constexpr uintptr_t kToNearExceptOneSelfVtable = 0x5B25378; |
| N0998 | src/mod/session2/offsets_s2.h:97 | kToNearVtable | vtable | 0x5B25418 | inline constexpr uintptr_t kToNearVtable = 0x5B25418; |
| N0999 | src/mod/session2/offsets_s2.h:98 | kExceptOneVf1 | RVA / layout / policy (audit) | 0x2A759B0 | inline constexpr uintptr_t kExceptOneVf1 = 0x2A759B0;      // control |
| N1000 | src/mod/session2/offsets_s2.h:99 | kExceptOneSelfVf1 | RVA / layout / policy (audit) | 0x2A75880 | inline constexpr uintptr_t kExceptOneSelfVf1 = 0x2A75880;  // self |
| N1001 | src/mod/session2/offsets_s2.h:100 | kToNearVf1 | RVA / layout / policy (audit) | 0x2A75860 | inline constexpr uintptr_t kToNearVf1 = 0x2A75860; |
| N1002 | src/mod/session2/offsets_s2.h:101 | kExceptOneVf1Head | signature | {0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00} | inline constexpr uint8_t kExceptOneVf1Head[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, |
| N1003 | src/mod/session2/offsets_s2.h:103 | kToNearVf1Head | signature | {0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9} | inline constexpr uint8_t kToNearVf1Head[] = {0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, |
| N1004 | src/mod/session2/offsets_s2.h:105 | kFinder_Actor | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kFinder_Actor = 0x10; |
| N1005 | src/mod/session2/offsets_s2.h:107 | kArray_Count | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kArray_Count = 0x08; |
| N1006 | src/mod/session2/offsets_s2.h:108 | kArray_Capacity | RVA / layout / policy (audit) | 0x0C | inline constexpr uint32_t kArray_Capacity = 0x0C; |
| N1007 | src/mod/session2/offsets_s2.h:111 | kPbSize | RVA / layout / policy (audit) | 0x8030 | inline constexpr uint32_t kPbSize = 0x8030; |
| N1008 | src/mod/session2/offsets_s2.h:112 | kPb_FramePtr | RVA / layout / policy (audit) | 0x8000 | inline constexpr uint32_t kPb_FramePtr = 0x8000;  // u8*: the frame (= pb) |
| N1009 | src/mod/session2/offsets_s2.h:113 | kPb_Len1 | RVA / layout / policy (audit) | 0x8008 | inline constexpr uint32_t kPb_Len1 = 0x8008;      // u16 frame length |
| N1010 | src/mod/session2/offsets_s2.h:114 | kPb_Len2 | RVA / layout / policy (audit) | 0x800A | inline constexpr uint32_t kPb_Len2 = 0x800A;      // u16 frame length (the server reads this one) |
| N1011 | src/mod/session2/offsets_s2.h:115 | kPb_Cap | RVA / layout / policy (audit) | 0x800C | inline constexpr uint32_t kPb_Cap = 0x800C;       // u16 |
| N1012 | src/mod/session2/offsets_s2.h:116 | kPb_IsRequest | RVA / layout / policy (audit) | 0x800E | inline constexpr uint32_t kPb_IsRequest = 0x800E; // u8: 0 = packet |
| N1013 | src/mod/session2/offsets_s2.h:117 | kPb_Tag | RVA / layout / policy (audit) | 0x8010 | inline constexpr uint32_t kPb_Tag = 0x8010;       // u32, not read for packets: our tag |
| N1014 | src/mod/session2/offsets_s2.h:118 | kPb_Magic | RVA / layout / policy (audit) | 0x8018 | inline constexpr uint32_t kPb_Magic = 0x8018;     // u64, not read by the server: our magic |
| N1015 | src/mod/session2/offsets_s2.h:119 | kPb_Base | RVA / layout / policy (audit) | 0x8028 | inline constexpr uint32_t kPb_Base = 0x8028;      // void*: what the client frees |
| N1016 | src/mod/session2/offsets_s2.h:120 | kFrameOverhead | RVA / layout / policy (audit) | 0x1F | inline constexpr uint32_t kFrameOverhead = 0x1F;  // frame = u16 len &#124; u8 channel &#124; packet &#124; 12 IV + 16 tag |
| N1017 | src/mod/session2/offsets_s2.h:121 | kFrameHeader | RVA / layout / policy (audit) | 3 | inline constexpr uint32_t kFrameHeader = 3; |
| N1018 | src/mod/session2/offsets_s2.h:123 | kRq_Frame | RVA / layout / policy (audit) | 0x30 | inline constexpr uint32_t kRq_Frame = 0x30; |
| N1019 | src/mod/session2/offsets_s2.h:127 | kTrocInstancesBegin | RVA / layout / policy (audit) | 0x6A51AE0 | inline constexpr uintptr_t kTrocInstancesBegin = 0x6A51AE0; |
| N1020 | src/mod/session2/offsets_s2.h:128 | kTrocInstancesEnd | RVA / layout / policy (audit) | 0x6A82C80 | inline constexpr uintptr_t kTrocInstancesEnd = 0x6A82C80; |
| N1021 | src/mod/session2/offsets_s2.h:129 | kTroc_Kind | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kTroc_Kind = 0x08; |
| N1022 | src/mod/session2/offsets_s2.h:130 | kTroc_Opcode | RVA / layout / policy (audit) | 0x0C | inline constexpr uint32_t kTroc_Opcode = 0x0C; |
| N1023 | src/mod/session2/offsets_s2.h:131 | kTroc_Channel | RVA / layout / policy (audit) | 0x14 | inline constexpr uint32_t kTroc_Channel = 0x14; |
| N1024 | src/mod/session2/offsets_s2.h:132 | kTroc_Flags | RVA / layout / policy (audit) | 0x16 | inline constexpr uint32_t kTroc_Flags = 0x16; |
| N1025 | src/mod/session2/offsets_s2.h:133 | kTrocKindServer | RVA / layout / policy (audit) | 4 | inline constexpr uint32_t kTrocKindServer = 4; |
| N1026 | src/mod/session2/offsets_s2.h:136 | kClientNet_Session | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kClientNet_Session = 0x10;   // NwVirtualSyncSession (persistent) |
| N1027 | src/mod/session2/offsets_s2.h:137 | kClientNet_FreeC2S | RVA / layout / policy (audit) | 0x28 | inline constexpr uint32_t kClientNet_FreeC2S = 0x28;   // Q3: C->S buffers the NetworkThread frees |
| N1028 | src/mod/session2/offsets_s2.h:138 | kClientNet_Slot4 | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kClientNet_Slot4 = 0x58;     // != 0 once the client connected (1001) |
| N1029 | src/mod/session2/offsets_s2.h:139 | kSyncSession_Epoch | RVA / layout / policy (audit) | 0xB0 | inline constexpr uint32_t kSyncSession_Epoch = 0xB0;   // new on every connection (OnConnect 0x8CAFF0) |
| N1030 | src/mod/session2/offsets_s2.h:140 | kSyncSessionSend | RVA / layout / policy (audit) | 0x14380B0 | inline constexpr uintptr_t kSyncSessionSend = 0x14380B0;  // vf6 (this, int* res, PacketBuffer* pb) -> res |
| N1031 | src/mod/session2/offsets_s2.h:141 | kSyncSessionSendHead | signature | {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA} | inline constexpr uint8_t kSyncSessionSendHead[] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, |
| N1032 | src/mod/session2/offsets_s2.h:143 | kNetLoopPump | RVA / layout / policy (audit) | 0x35BAD4 | inline constexpr uintptr_t kNetLoopPump = 0x35BAD4;  // NetworkThread loop, each turn (<= 16 ms); r15 = ClientNet |
| N1033 | src/mod/session2/offsets_s2.h:144 | kNetLoopPumpHead | signature | {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7} | inline constexpr uint8_t kNetLoopPumpHead[] = {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, |
| N1034 | src/mod/session2/offsets_s2.h:153 | kNetLoopPumpPatch | RVA / layout / policy (audit) | 7 | inline constexpr uint32_t kNetLoopPumpPatch = 7; |
| N1035 | src/mod/session2/offsets_s2.h:154 | kNetLoopPumpAob | signature | { 0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74} | inline constexpr uint8_t kNetLoopPumpAob[] = { |
| N1036 | src/mod/session2/offsets_s2.h:157 | kClientOnRecvPacket | RVA / layout / policy (audit) | 0x35CA00 | inline constexpr uintptr_t kClientOnRecvPacket = 0x35CA00;  // (ClientNet*, NwSession*, -, u8* pkt, u16 size) |
| N1037 | src/mod/session2/offsets_s2.h:158 | kClientOnRecvPacketHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24} | inline constexpr uint8_t kClientOnRecvPacketHead[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, |
| N1038 | src/mod/session2/offsets_s2.h:165 | kWorldSystemSlot | RVA / layout / policy (audit) | 0x6D69190 | inline constexpr uintptr_t kWorldSystemSlot = 0x6D69190;      // [exe+..] = WorldSystem |
| N1039 | src/mod/session2/offsets_s2.h:166 | kWorldSystem_SaveManager | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kWorldSystem_SaveManager = 0x88;     // the save manager the title reads |
| N1040 | src/mod/session2/offsets_s2.h:167 | kServerActorLookupSlot | RVA / layout / policy (audit) | 0x6D69698 | inline constexpr uintptr_t kServerActorLookupSlot = 0x6D69698;  // [exe+..]; +0x48 = the server's save manager |
| N1041 | src/mod/session2/offsets_s2.h:168 | kServerActorLookup_SaveManager | RVA / layout / policy (audit) | 0x48 | inline constexpr uint32_t kServerActorLookup_SaveManager = 0x48; |
| N1042 | src/mod/session2/offsets_s2.h:170 | kSaveMgr_Lock | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kSaveMgr_Lock = 0x00;     // pa::WindowsLock<3>* (= this + 8) |
| N1043 | src/mod/session2/offsets_s2.h:171 | kSaveMgr_LockObj | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kSaveMgr_LockObj = 0x08;  // the lock itself (CRITICAL_SECTION inside) |
| N1044 | src/mod/session2/offsets_s2.h:172 | kSaveMgr_Slots | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kSaveMgr_Slots = 0x40;    // entries of 0x68 bytes: u32 slot @0, i64 time @8 |
| N1045 | src/mod/session2/offsets_s2.h:173 | kSaveMgr_Count | RVA / layout / policy (audit) | 0x48 | inline constexpr uint32_t kSaveMgr_Count = 0x48;    // u32 |
| N1046 | src/mod/session2/offsets_s2.h:174 | kSaveMgr_Capacity | RVA / layout / policy (audit) | 0x4C | inline constexpr uint32_t kSaveMgr_Capacity = 0x4C; // u32 |
| N1047 | src/mod/session2/offsets_s2.h:175 | kSaveMgr_Storage | RVA / layout / policy (audit) | 0x50 | inline constexpr uint32_t kSaveMgr_Storage = 0x50;  // SaveStorage*; +0x38 slot folders, +0x40 their count |
| N1048 | src/mod/session2/offsets_s2.h:176 | kSaveMgr_Busy | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kSaveMgr_Busy = 0x60;     // u8: a refresh/login holds the table |
| N1049 | src/mod/session2/offsets_s2.h:177 | kSaveMgr_EntrySize | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kSaveMgr_EntrySize = 0x68; |
| N1050 | src/mod/session2/offsets_s2.h:178 | kSaveStorage_FolderCount | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kSaveStorage_FolderCount = 0x40; |
| N1051 | src/mod/session2/offsets_s2.h:179 | kWindowsLock3Vtable | vtable | 0x557C120 | inline constexpr uintptr_t kWindowsLock3Vtable = 0x557C120;  // vf1 -> EnterCriticalSection, vf2 -> Leave |
| N1052 | src/mod/session2/offsets_s2.h:180 | kWindowsLockVf1 | RVA / layout / policy (audit) | 0x1371340 | inline constexpr uintptr_t kWindowsLockVf1 = 0x1371340;      // mov rax,[rcx]; jmp [rax+0x18] |
| N1053 | src/mod/session2/offsets_s2.h:181 | kWindowsLockVf2 | RVA / layout / policy (audit) | 0xCE4EF0 | inline constexpr uintptr_t kWindowsLockVf2 = 0xCE4EF0;       // mov rax,[rcx]; jmp [rax+0x20] |
| N1054 | src/mod/session2/offsets_s2.h:182 | kWindowsLockVf1Head | signature | {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18} | inline constexpr uint8_t kWindowsLockVf1Head[] = {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18}; |
| N1055 | src/mod/session2/offsets_s2.h:183 | kWindowsLockVf2Head | signature | {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20} | inline constexpr uint8_t kWindowsLockVf2Head[] = {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20}; |
| N1056 | src/mod/session2/offsets_s2.h:185 | kSaveMgrRefreshSlots | RVA / layout / policy (audit) | 0x1870C20 | inline constexpr uintptr_t kSaveMgrRefreshSlots = 0x1870C20; |
| N1057 | src/mod/session2/offsets_s2.h:186 | kSaveMgrRefreshSlotsHead | signature | {0xE9, 0xAB, 0x86, 0x36, 0x0B} | inline constexpr uint8_t kSaveMgrRefreshSlotsHead[] = {0xE9, 0xAB, 0x86, 0x36, 0x0B}; |
| N1058 | src/mod/session2/offsets_s2.h:190 | kSlotRefreshReqSequence | RVA / layout / policy (audit) | 0x2A22475 | inline constexpr uintptr_t kSlotRefreshReqSequence = 0x2A22475; |
| N1059 | src/mod/session2/offsets_s2.h:191 | kSlotRefreshReqSequenceBytes | signature | { 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10} | inline constexpr uint8_t kSlotRefreshReqSequenceBytes[] = { |
| N1060 | src/mod/session2/offsets_s2.h:211 | kTitlePlayReadsSaveMgr | RVA / layout / policy (audit) | 0x65DC3E | inline constexpr uintptr_t kTitlePlayReadsSaveMgr = 0x65DC3E; |
| N1061 | src/mod/session2/offsets_s2.h:212 | kTitlePlayReadsSaveMgrBytes | signature | {0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00} | inline constexpr uint8_t kTitlePlayReadsSaveMgrBytes[] = {0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, |
| N1062 | src/mod/session2/offsets_s2.h:215 | kImageBase | RVA / layout / policy (audit) | 0x140000000 | inline constexpr uintptr_t kImageBase = 0x140000000; |
| N1063 | src/mod/session2/offsets_s2.h:216 | kPeTimestamp | RVA / layout / policy (audit) | 0x6AB28F00 | inline constexpr uint32_t kPeTimestamp = 0x6AB28F00; |
| N1064 | src/mod/session2/p8_probe_logic.h:150 | kMaxProbe | RVA / layout / policy (audit) | 32 | static constexpr size_t kMaxProbe = 32; |
| N1065 | src/mod/session2/p8_probe_logic.h:151 | kProbe | RVA / layout / policy (audit) | N < kMaxProbe ? N : kMaxProbe | static constexpr size_t kProbe = N < kMaxProbe ? N : kMaxProbe; |
| N1066 | src/mod/session2/p8_probe_logic.h:152 | kMaxKeys | RVA / layout / policy (audit) | N / 2 | static constexpr size_t kMaxKeys = N / 2; |
| N1067 | src/mod/session2/p8_probe_logic.h:245 | kTable | RVA / layout / policy (audit) | 0x18 | static constexpr uint32_t kTable = 0x18, kCount = 0x20, kTripleSize = 0x18, kHashInThird = 0x18; |
| N1068 | src/mod/session2/p8_probe_logic.h:245 | kCount | RVA / layout / policy (audit) | 0x20 | static constexpr uint32_t kTable = 0x18, kCount = 0x20, kTripleSize = 0x18, kHashInThird = 0x18; |
| N1069 | src/mod/session2/p8_probe_logic.h:245 | kTripleSize | RVA / layout / policy (audit) | 0x18 | static constexpr uint32_t kTable = 0x18, kCount = 0x20, kTripleSize = 0x18, kHashInThird = 0x18; |
| N1070 | src/mod/session2/p8_probe_logic.h:245 | kHashInThird | RVA / layout / policy (audit) | 0x18 | static constexpr uint32_t kTable = 0x18, kCount = 0x20, kTripleSize = 0x18, kHashInThird = 0x18; |
| N1071 | src/mod/session2/p8_probe_logic.h:355 | kHashSeqBodyAnim | RVA / layout / policy (audit) | 0x837487D4 | inline constexpr uint32_t kHashSeqBodyAnim = 0x837487D4; |
| N1072 | src/mod/session2/p8_probe_logic.h:356 | kHashSeqBodyAnimLoop | RVA / layout / policy (audit) | 0x35DD392F | inline constexpr uint32_t kHashSeqBodyAnimLoop = 0x35DD392F; |
| N1073 | src/mod/session2/reannounce_logic.h:40 | kControlReannounce | RVA / layout / policy (audit) | 3 | inline constexpr uint8_t kControlReannounce = 3;  // troc_frame.h: 1 host ready, 2 host dead (ToGuestClient) |
| N1074 | src/mod/session2/reannounce_logic.h:41 | kPayloadSize | RVA / layout / policy (audit) | 10 | inline constexpr size_t kPayloadSize = 10; |
| N1075 | src/mod/session2/reannounce_logic.h:42 | kMaxAttempts | RVA / layout / policy (audit) | 3 | inline constexpr uint8_t kMaxAttempts = 3; |
| N1076 | src/mod/session2/reannounce_logic.h:43 | kMinGapMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint64_t kMinGapMs = 1000; |
| N1077 | src/mod/session2/reannounce_logic.h:76 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1;  // troc_frame.h kControlOpcode |
| N1078 | src/mod/session2/reannounce_logic.h:77 | kFrameSize | RVA / layout / policy (audit) | 5 + kPayloadSize | inline constexpr size_t kFrameSize = 5 + kPayloadSize; |
| N1079 | src/mod/session2/reannounce_logic.h:110 | kVerdicts | RVA / layout / policy (audit) | 6 | inline constexpr size_t kVerdicts = 6; |
| N1080 | src/mod/session2/reannounce_logic.h:125 | kMaxPerEpoch | RVA / layout / policy (audit) | 16 | inline constexpr uint32_t kMaxPerEpoch = 16; |
| N1081 | src/mod/session2/rejoin_logic.h:47 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1;    // troc_frame.h kControlOpcode |
| N1082 | src/mod/session2/rejoin_logic.h:48 | kControlRejoin | RVA / layout / policy (audit) | 13 | inline constexpr uint8_t kControlRejoin = 13;    // troc_frame.h kControlRejoin |
| N1083 | src/mod/session2/rejoin_logic.h:49 | kControlEpoch | RVA / layout / policy (audit) | 14 | inline constexpr uint8_t kControlEpoch = 14;     // troc_frame.h kControlEpoch |
| N1084 | src/mod/session2/rejoin_logic.h:50 | kCapsRejoin | RVA / layout / policy (audit) | 0x08 | inline constexpr uint8_t kCapsRejoin = 0x08;     // troc_frame.h kCapsRejoin |
| N1085 | src/mod/session2/rejoin_logic.h:51 | kWireVersion | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kWireVersion = 1; |
| N1086 | src/mod/session2/rejoin_logic.h:52 | kRejoinPayload | RVA / layout / policy (audit) | 11 | inline constexpr size_t kRejoinPayload = 11, kEpochPayload = 12; |
| N1087 | src/mod/session2/rejoin_logic.h:52 | kEpochPayload | RVA / layout / policy (audit) | 12 | inline constexpr size_t kRejoinPayload = 11, kEpochPayload = 12; |
| N1088 | src/mod/session2/rejoin_logic.h:53 | kRetryMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint64_t kRetryMs = 1000; |
| N1089 | src/mod/session2/rejoin_logic.h:54 | kMaxAsks | RVA / layout / policy (audit) | 30 | inline constexpr uint32_t kMaxAsks = 30; |
| N1090 | src/mod/session2/rejoin_logic.h:223 | kGrantUnusedMs | RVA / layout / policy (audit) | 60000 | inline constexpr uint64_t kGrantUnusedMs = 60000; |
| N1091 | src/mod/session2/report_guard.cpp:19 | kReporter | RVA / layout / policy (audit) | 0x1317D40 | constexpr uintptr_t kReporter = 0x1317D40; |
| N1092 | src/mod/session2/report_guard.cpp:20 | kReporterHead | signature | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57} | constexpr uint8_t kReporterHead[] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, |
| N1093 | src/mod/session2/report_guard.cpp:22 | kMaxPaths | RVA / layout / policy (audit) | 8 | constexpr uint32_t kMaxPaths = 8; |
| N1094 | src/mod/session2/report_guard.cpp:23 | kMaxText | RVA / layout / policy (audit) | 260 | constexpr size_t kMaxText = 260; |
| N1095 | src/mod/session2/report_guard.cpp:24 | kMaxLogged | RVA / layout / policy (audit) | 16 | constexpr uint64_t kMaxLogged = 16;  // a broken loop must not flood the log |
| N1096 | src/mod/session2/ride_filter_logic.h:50 | kOpStartActionAck | Troc ID (audit) | 2991 | inline constexpr uint16_t kOpStartActionAck = 2991; |
| N1097 | src/mod/session2/ride_filter_logic.h:51 | kHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kHeader = 5; |
| N1098 | src/mod/session2/ride_filter_logic.h:52 | kAckSize | RVA / layout / policy (audit) | 725 | inline constexpr size_t kAckSize = 725; |
| N1099 | src/mod/session2/ride_filter_logic.h:53 | kA0 | RVA / layout / policy (audit) | 0x004 | inline constexpr size_t kA0 = 0x004; |
| N1100 | src/mod/session2/ride_filter_logic.h:54 | kA1 | RVA / layout / policy (audit) | 0x049 | inline constexpr size_t kA1 = 0x049; |
| N1101 | src/mod/session2/ride_filter_logic.h:55 | kBlock | RVA / layout / policy (audit) | 69 | inline constexpr size_t kBlock = 69; |
| N1102 | src/mod/session2/ride_filter_logic.h:56 | kBlockHash | RVA / layout / policy (audit) | 0x04 | inline constexpr size_t kBlockHash = 0x04; |
| N1103 | src/mod/session2/ride_filter_logic.h:57 | kParent | RVA / layout / policy (audit) | 0x117 | inline constexpr size_t kParent = 0x117;  // u64, all ones = none |
| N1104 | src/mod/session2/ride_filter_logic.h:58 | kTarget | RVA / layout / policy (audit) | 0x16D | inline constexpr size_t kTarget = 0x16D;  // u32 |
| N1105 | src/mod/session2/ride_filter_logic.h:60 | kMountA0 | signature | {0x91605616u, 0x872A6750u} | inline constexpr uint32_t kMountA0[] = {0x91605616u, 0x872A6750u};  // the guest's / the host's (session 9) |
| N1106 | src/mod/session2/ride_filter_logic.h:61 | kDismountA1 | RVA / layout / policy (audit) | 0x7B6AFF57u | inline constexpr uint32_t kDismountA1 = 0x7B6AFF57u;                // with A0 0x667E3B35 / 0xC9356CEB / 0x412C2C2B |
| N1107 | src/mod/session2/ride_filter_logic.h:62 | kFootHash | RVA / layout / policy (audit) | 0x26107D3Cu | inline constexpr uint32_t kFootHash = 0x26107D3Cu; |
| N1108 | src/mod/session2/ride_filter_logic.h:63 | kRidingHash | RVA / layout / policy (audit) | 0xF79C7424u | inline constexpr uint32_t kRidingHash = 0xF79C7424u; |
| N1109 | src/mod/session2/ride_filter_logic.h:67 | kBlockFoot | signature | { 0x00, 0x00, 0x00, 0x00, 0x3C, 0x7D, 0x10, 0x26, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x89, 0x88, 0x08, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD7, 0xA3, 0x70, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | inline constexpr std::array<uint8_t, kBlock> kBlockFoot = { |
| N1110 | src/mod/session2/ride_filter_logic.h:72 | kBlockRiding | signature | { 0x00, 0x00, 0x00, 0x00, 0x24, 0x74, 0x9C, 0xF7, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x89, 0x88, 0x08, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD7, 0xA3, 0x70, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | inline constexpr std::array<uint8_t, kBlock> kBlockRiding = { |
| N1111 | src/mod/session2/ride_filter_logic.h:77 | kBlockEmpty | signature | { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | inline constexpr std::array<uint8_t, kBlock> kBlockEmpty = { |
| N1112 | src/mod/session2/ride_filter_logic.h:121 | kKinds | RVA / layout / policy (audit) | 3 | inline constexpr size_t kKinds = 3; |
| N1113 | src/mod/session2/ride_filter_logic.h:181 | kWindowMs | RVA / layout / policy (audit) | 3000 | inline constexpr uint64_t kWindowMs = 3000; |
| N1114 | src/mod/session2/ride_filter_logic.h:222 | kSlots | RVA / layout / policy (audit) | 64 | static constexpr size_t kSlots = 64; |
| N1115 | src/mod/session2/ride_filter_logic.h:302 | kGateDismountA0 | signature | {0xC9356CEBu, 0x412C2C2Bu} | inline constexpr uint32_t kGateDismountA0[] = {0xC9356CEBu, 0x412C2C2Bu}; |
| N1116 | src/mod/session2/ride_filter_logic.h:307 | kSlots | RVA / layout / policy (audit) | 8 | static constexpr size_t kSlots = 8; |
| N1117 | src/mod/session2/save_hooks.cpp:142 | kTraceLines | RVA / layout / policy (audit) | 5000 | constexpr uint32_t kTraceLines = 5000; |
| N1118 | src/mod/session2/save_hooks.cpp:143 | kTraceNonWriteLines | RVA / layout / policy (audit) | 2000 | constexpr uint32_t kTraceNonWriteLines = 2000; |
| N1119 | src/mod/session2/save_hooks.cpp:259 | kName | RVA / layout / policy (audit) |  | static constexpr char kName[] = "pearl abyss"; |
| N1120 | src/mod/session2/save_hooks.cpp:281 | kRedirectLines | RVA / layout / policy (audit) | 200 | constexpr uint64_t kRedirectLines = 200;  // "Session2 save redirect #N" lines, writes only |
| N1121 | src/mod/session2/save_hooks.cpp:282 | kRedirectErrorLines | RVA / layout / policy (audit) | 20 | constexpr uint64_t kRedirectErrorLines = 20; |
| N1122 | src/mod/session2/save_hooks.cpp:444 | kWrite | RVA / layout / policy (audit) | [](const auto& | constexpr auto kWrite = [](const auto&, auto, DWORD) { return Kind::Write; }; |
| N1123 | src/mod/session2/save_hooks.cpp:445 | kQuery | RVA / layout / policy (audit) | [](const auto& | constexpr auto kQuery = [](const auto&, auto, DWORD) { return Kind::Query; }; |
| N1124 | src/mod/session2/save_hooks.cpp:447 | kWriteIfSecond | RVA / layout / policy (audit) | [](const auto& m | constexpr auto kWriteIfSecond = [](const auto& m, auto, DWORD) { return m[1].hit ? Kind::Write : Kind::Read; }; |
| N1125 | src/mod/session2/save_hooks.cpp:449 | kBoolText | RVA / layout / policy (audit) | [](BOOL ok | constexpr auto kBoolText = [](BOOL ok, DWORD error) { return bool_result(ok, error); }; |
| N1126 | src/mod/session2/save_hooks.cpp:450 | kHandleText | RVA / layout / policy (audit) | [](HANDLE h | constexpr auto kHandleText = [](HANDLE h, DWORD error) { return handle_result(h, error); }; |
| N1127 | src/mod/session2/save_hooks.cpp:451 | kNoText | RVA / layout / policy (audit) | [](auto | constexpr auto kNoText = [](auto, DWORD) { return std::string(); };  // queries get no line |
| N1128 | src/mod/session2/server.cpp:68 | kMagic | RVA / layout / policy (audit) | 0x32534553'44504F43ull | constexpr uint64_t kMagic = 0x32534553'44504F43ull;  // "COPDSES2" |
| N1129 | src/mod/session2/server.cpp:76 | kSessionId | RVA / layout / policy (audit) | 0 | constexpr uint32_t kSessionId = 0; |
| N1130 | src/mod/session2/server.cpp:77 | kMaxOutgoingBytes | RVA / layout / policy (audit) | 32u << 20 | constexpr size_t kMaxOutgoingBytes = 32u << 20;      // a login burst is ~42,000 Acks |
| N1131 | src/mod/session2/server.cpp:114 | kEpochSlots | RVA / layout / policy (audit) | kMaxGuestEpochs + 1 | constexpr size_t kEpochSlots = kMaxGuestEpochs + 1; |
| N1132 | src/mod/session2/server.cpp:123 | kNotReq | RVA / layout / policy (audit) | 0xFF | constexpr uint8_t kNotReq = 0xFF, kBlocked = 0xFE; |
| N1133 | src/mod/session2/server.cpp:123 | kBlocked | RVA / layout / policy (audit) | 0xFE | constexpr uint8_t kNotReq = 0xFF, kBlocked = 0xFE; |
| N1134 | src/mod/session2/server.cpp:133 | kOpLoginCharacterToFieldServerReq | Troc ID (audit) | 3093 | constexpr uint16_t kOpLoginCharacterToFieldServerReq = 3093; |
| N1135 | src/mod/session2/server.cpp:134 | kOpLoginUserToFieldServerReq | Troc ID (audit) | 2490 | constexpr uint16_t kOpLoginUserToFieldServerReq = 2490; |
| N1136 | src/mod/session2/server.cpp:137 | kOpMoveActorReq | Troc ID (audit) | 2556 | constexpr uint16_t kOpMoveActorReq = 2556, kOpStartActionReq = 2223, kOpRotateActorReq = 2828; |
| N1137 | src/mod/session2/server.cpp:137 | kOpStartActionReq | Troc ID (audit) | 2223 | constexpr uint16_t kOpMoveActorReq = 2556, kOpStartActionReq = 2223, kOpRotateActorReq = 2828; |
| N1138 | src/mod/session2/server.cpp:137 | kOpRotateActorReq | Troc ID (audit) | 2828 | constexpr uint16_t kOpMoveActorReq = 2556, kOpStartActionReq = 2223, kOpRotateActorReq = 2828; |
| N1139 | src/mod/session2/server.cpp:138 | kOpMainPlayerDataStartAck | Troc ID (audit) | 2756 | constexpr uint16_t kOpMainPlayerDataStartAck = 2756; |
| N1140 | src/mod/session2/server.cpp:139 | kOpMainUserDataStartAck | Troc ID (audit) | 2646 | constexpr uint16_t kOpMainUserDataStartAck = 2646;  // u32 user key at +0 |
| N1141 | src/mod/session2/server.cpp:140 | kOpMoveFieldAck | Troc ID (audit) | 2174 | constexpr uint16_t kOpMoveFieldAck = 2174;          // u8 &#124; u32 actor &#124; u32 &#124; u32 field owner @9 &#124; 0x30 * n |
| N1142 | src/mod/session2/server.cpp:159 | kMaxMirrored | RVA / layout / policy (audit) | 4096 | constexpr size_t kMaxMirrored = 4096; |
| N1143 | src/mod/session2/server.cpp:168 | kLogoutPendingMs | RVA / layout / policy (audit) | 60000 | constexpr uint64_t kLogoutPendingMs = 60000;    // give up handing it after this |
| N1144 | src/mod/session2/server.cpp:181 | kOpNak | Troc ID (audit) | 1013 | constexpr uint16_t kOpNak = 1013; |
| N1145 | src/mod/session2/server.cpp:182 | kOpRemoveActorAck2516 | Troc ID (audit) | 2516 | constexpr uint16_t kOpRemoveActorAck2516 = 2516;  // RemoveActorAck (world_anchor's probe) |
| N1146 | src/mod/session2/server.cpp:207 | kMaxExceptionNotes | RVA / layout / policy (audit) | 8 | constexpr uint32_t kMaxExceptionNotes = 8; |
| N1147 | src/mod/session2/server.h:152 | kMaxGuestEpochs | RVA / layout / policy (audit) | 32 | inline constexpr uint32_t kMaxGuestEpochs = 32; |
| N1148 | src/mod/session2/session2.cpp:80 | kMaxEarly | RVA / layout / policy (audit) | 512 | constexpr size_t kMaxEarly = 512; |
| N1149 | src/mod/session2/session2.cpp:82 | kHoldRetryMs | RVA / layout / policy (audit) | 20 | constexpr uint64_t kHoldRetryMs = 20; |
| N1150 | src/mod/session2/session2.cpp:93 | kLinkLostMs | RVA / layout / policy (audit) | 15000 | constexpr uint32_t kLinkLostMs = 15000;  // longer than the session's own 5 s peer timeout |
| N1151 | src/mod/session2/session2.cpp:109 | kSampleMs | RVA / layout / policy (audit) | 1000 | constexpr uint32_t kSampleMs = 1000, kBarrierMs = 250; |
| N1152 | src/mod/session2/session2.cpp:109 | kBarrierMs | RVA / layout / policy (audit) | 250 | constexpr uint32_t kSampleMs = 1000, kBarrierMs = 250; |
| N1153 | src/mod/session2/session2.cpp:120 | kRejoinLines | RVA / layout / policy (audit) | 64 | constexpr uint64_t kRejoinLines = 64; |
| N1154 | src/mod/session2/session2.cpp:161 | kSlowTickUs | RVA / layout / policy (audit) | 50'000 | constexpr uint64_t kSlowTickUs = 50'000;  // a tick above this is logged with its parts |
| N1155 | src/mod/session2/session2.cpp:162 | kSlowTickLines | RVA / layout / policy (audit) | 200 | constexpr uint64_t kSlowTickLines = 200; |
| N1156 | src/mod/session2/session2.cpp:276 | kOpSetFieldTimeReq | Troc ID (audit) | 2125 | constexpr uint16_t kOpSetFieldTimeReq = 2125; |
| N1157 | src/mod/session2/session2.cpp:1471 | kReanLines | RVA / layout / policy (audit) | 32 | constexpr uint64_t kReanLines = 32; |
| N1158 | src/mod/session2/slot_refresh.h:75 | kEntrySize | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kEntrySize = 0x68; |
| N1159 | src/mod/session2/slot_refresh.h:76 | kMaxEntriesRead | RVA / layout / policy (audit) | 64 | inline constexpr uint32_t kMaxEntriesRead = 64; |
| N1160 | src/mod/session2/travel_probe.h:55 | kTravelContext | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kTravelContext = 1, kTravelPrepared = 2, kTravelConsume = 3, kTravelRevive = 4; |
| N1161 | src/mod/session2/travel_probe.h:55 | kTravelPrepared | RVA / layout / policy (audit) | 2 | inline constexpr uint16_t kTravelContext = 1, kTravelPrepared = 2, kTravelConsume = 3, kTravelRevive = 4; |
| N1162 | src/mod/session2/travel_probe.h:55 | kTravelConsume | RVA / layout / policy (audit) | 3 | inline constexpr uint16_t kTravelContext = 1, kTravelPrepared = 2, kTravelConsume = 3, kTravelRevive = 4; |
| N1163 | src/mod/session2/travel_probe.h:55 | kTravelRevive | RVA / layout / policy (audit) | 4 | inline constexpr uint16_t kTravelContext = 1, kTravelPrepared = 2, kTravelConsume = 3, kTravelRevive = 4; |
| N1164 | src/mod/session2/travel_probe.h:59 | kTravelHead | RVA / layout / policy (audit) | 24 | inline constexpr size_t kTravelHead = 24; |
| N1165 | src/mod/session2/travel_probe_logic.h:26 | kOpReloadStageStartReq | Troc ID (audit) | 2283 | inline constexpr uint16_t kOpReloadStageStartReq = 2283; |
| N1166 | src/mod/session2/travel_probe_logic.h:27 | kOpGameLoadingStartReq | Troc ID (audit) | 2437 | inline constexpr uint16_t kOpGameLoadingStartReq = 2437; |
| N1167 | src/mod/session2/travel_probe_logic.h:28 | kOpMoveFieldAck | Troc ID (audit) | 2174 | inline constexpr uint16_t kOpMoveFieldAck = 2174; |
| N1168 | src/mod/session2/travel_probe_logic.h:29 | kOpLoadingFieldDataAck | Troc ID (audit) | 2643 | inline constexpr uint16_t kOpLoadingFieldDataAck = 2643; |
| N1169 | src/mod/session2/travel_probe_logic.h:30 | kOpMoveFieldCompletePostAck | Troc ID (audit) | 3066 | inline constexpr uint16_t kOpMoveFieldCompletePostAck = 3066; |
| N1170 | src/mod/session2/travel_probe_logic.h:31 | kOpMoveFieldCompleteAck | Troc ID (audit) | 2144 | inline constexpr uint16_t kOpMoveFieldCompleteAck = 2144; |
| N1171 | src/mod/session2/travel_probe_logic.h:32 | kOpPlayAllPreLoadSequencerAck | Troc ID (audit) | 2322 | inline constexpr uint16_t kOpPlayAllPreLoadSequencerAck = 2322; |
| N1172 | src/mod/session2/travel_probe_logic.h:33 | kOpLoadingCompleteAck | Troc ID (audit) | 2762 | inline constexpr uint16_t kOpLoadingCompleteAck = 2762; |
| N1173 | src/mod/session2/travel_probe_logic.h:34 | kOpLogoutCharacterAck | Troc ID (audit) | 2679 | inline constexpr uint16_t kOpLogoutCharacterAck = 2679; |
| N1174 | src/mod/session2/travel_probe_logic.h:35 | kOpNak | Troc ID (audit) | 1013 | inline constexpr uint16_t kOpNak = 1013; |
| N1175 | src/mod/session2/travel_probe_logic.h:38 | kOpReviveLoadingReq | Troc ID (audit) | 2362 | inline constexpr uint16_t kOpReviveLoadingReq = 2362; |
| N1176 | src/mod/session2/travel_probe_logic.h:40 | kOwnerAt | RVA / layout / policy (audit) | 9 | inline constexpr size_t kOwnerAt = 9;           // u32 owner, both 2174 and 2643 (payload offset, no Troc header) |
| N1177 | src/mod/session2/travel_probe_logic.h:41 | kMoveFieldHead | RVA / layout / policy (audit) | 17 | inline constexpr size_t kMoveFieldHead = 17;    // 2174: 17 + 48n |
| N1178 | src/mod/session2/travel_probe_logic.h:42 | kLoadingFieldHead | RVA / layout / policy (audit) | 20 | inline constexpr size_t kLoadingFieldHead = 20; // 2643: 20 + 48n |
| N1179 | src/mod/session2/travel_probe_logic.h:43 | kRecord | RVA / layout / policy (audit) | 0x30 | inline constexpr size_t kRecord = 0x30; |
| N1180 | src/mod/session2/travel_probe_logic.h:157 | kErrInvalidFieldInfoKey | RVA / layout / policy (audit) | 0x8E19630A | inline constexpr uint32_t kErrInvalidFieldInfoKey = 0x8E19630A;  // hashlittle("eErrNoInvalidFieldInfoKey") |
| N1181 | src/mod/session2/travel_probe_logic.h:287 | kMaxAcks | RVA / layout / policy (audit) | 24 | inline constexpr uint32_t kMaxAcks = 24; |
| N1182 | src/mod/session2/travel_probe_logic.h:288 | kWindowMs | RVA / layout / policy (audit) | 20000 | inline constexpr uint64_t kWindowMs = 20000; |
| N1183 | src/mod/session2/travel_probe_logic.h:289 | kAlwaysLines | RVA / layout / policy (audit) | 8 | inline constexpr uint32_t kAlwaysLines = 8; |
| N1184 | src/mod/session2/travel_probe_logic.h:322 | kOpPreLoadSequencerReq | Troc ID (audit) | 2873 | inline constexpr uint16_t kOpPreLoadSequencerReq = 2873; |
| N1185 | src/mod/session2/travel_probe_logic.h:323 | kOpPrepareSequencerAck | Troc ID (audit) | 2661 | inline constexpr uint16_t kOpPrepareSequencerAck = 2661; |
| N1186 | src/mod/session2/travel_probe_logic.h:324 | kOpStageBranchStateAck | Troc ID (audit) | 2981 | inline constexpr uint16_t kOpStageBranchStateAck = 2981; |
| N1187 | src/mod/session2/travel_probe_logic.h:325 | kHostBurstUs | RVA / layout / policy (audit) | 3'000'000 | inline constexpr uint64_t kHostBurstUs = 3'000'000; |
| N1188 | src/mod/session2/travel_probe_logic.h:326 | kHostOwnBeforeUs | RVA / layout / policy (audit) | 1'000'000 | inline constexpr uint64_t kHostOwnBeforeUs = 1'000'000; |
| N1189 | src/mod/session2/tree_bend_logic.h:45 | kVf2 | RVA / layout / policy (audit) | 0x117F7E0 | inline constexpr uintptr_t kVf2 = 0x117F7E0; |
| N1190 | src/mod/session2/tree_bend_logic.h:48 | kVf2Patch | RVA / layout / policy (audit) | 5 | inline constexpr uint32_t kVf2Patch = 5;  // mov [rsp+8],rbx |
| N1191 | src/mod/session2/tree_bend_logic.h:49 | kVf2Slot | RVA / layout / policy (audit) | 0x56FFB58 | inline constexpr uintptr_t kVf2Slot = 0x56FFB58; |
| N1192 | src/mod/session2/tree_bend_logic.h:51 | kApply | RVA / layout / policy (audit) | 0x1190FD0 | inline constexpr uintptr_t kApply = 0x1190FD0; |
| N1193 | src/mod/session2/tree_bend_logic.h:53 | kApplyPatch | RVA / layout / policy (audit) | 7 | inline constexpr uint32_t kApplyPatch = 7;  // mov rax,rsp &#124; mov [rax+0x10],rbx |
| N1194 | src/mod/session2/tree_bend_logic.h:54 | kApplyCall | RVA / layout / policy (audit) | 0x117F911 | inline constexpr uintptr_t kApplyCall = 0x117F911;  // in 0x117F7E0: call 0x1190FD0 |
| N1195 | src/mod/session2/tree_bend_logic.h:57 | kSetBend | RVA / layout / policy (audit) | 0x349EA10 | inline constexpr uintptr_t kSetBend = 0x349EA10; |
| N1196 | src/mod/session2/tree_bend_logic.h:59 | kSetBendPatch | RVA / layout / policy (audit) | 5 | inline constexpr uint32_t kSetBendPatch = 5;  // mov [rsp+8],rbx |
| N1197 | src/mod/session2/tree_bend_logic.h:60 | kSetBendCall | RVA / layout / policy (audit) | 0x1191532 | inline constexpr uintptr_t kSetBendCall = 0x1191532;  // in 0x1190FD0: call SetBend (rdx = state+0x380, xmm2 = state+0x378) |
| N1198 | src/mod/session2/tree_bend_logic.h:65 | kTick | RVA / layout / policy (audit) | 0x59C0D0 | inline constexpr uintptr_t kTick = 0x59C0D0; |
| N1199 | src/mod/session2/tree_bend_logic.h:67 | kTickBoundaries | signature | {2, 3, 4, 5, 7, 9} | inline constexpr uint32_t kTickBoundaries[] = {2, 3, 4, 5, 7, 9}; |
| N1200 | src/mod/session2/tree_bend_logic.h:68 | kTickPatch | signature | {5} | inline constexpr uint32_t kTickPatch[] = {5};  // push rbp &#124; push rbx &#124; push rsi &#124; push rdi |
| N1201 | src/mod/session2/tree_bend_logic.h:69 | kTickJump | RVA / layout / policy (audit) | 0x8A5DA0 | inline constexpr uintptr_t kTickJump = 0x8A5DA0; |
| N1202 | src/mod/session2/tree_bend_logic.h:74 | kGetTarget | RVA / layout / policy (audit) | 0x5B1600 | inline constexpr uintptr_t kGetTarget = 0x5B1600; |
| N1203 | src/mod/session2/tree_bend_logic.h:76 | kFindTree | RVA / layout / policy (audit) | 0x491AC0 | inline constexpr uintptr_t kFindTree = 0x491AC0; |
| N1204 | src/mod/session2/tree_bend_logic.h:81 | kActor_Key | RVA / layout / policy (audit) | 0x60 | inline constexpr uint32_t kActor_Key = 0x60; |
| N1205 | src/mod/session2/tree_bend_logic.h:82 | kActor_Control | RVA / layout / policy (audit) | 0xC8 | inline constexpr uint32_t kActor_Control = 0xC8;  // u8, != 0: controlled by this client |
| N1206 | src/mod/session2/tree_bend_logic.h:83 | kActor_Comps | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActor_Comps = 0x68; |
| N1207 | src/mod/session2/tree_bend_logic.h:84 | kActor_LockObj | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kActor_LockObj = 0x08; |
| N1208 | src/mod/session2/tree_bend_logic.h:85 | kComps_RemoteCatch | RVA / layout / policy (audit) | 0x78 | inline constexpr uint32_t kComps_RemoteCatch = 0x78; |
| N1209 | src/mod/session2/tree_bend_logic.h:86 | kComps_Control | RVA / layout / policy (audit) | 0x40 | inline constexpr uint32_t kComps_Control = 0x40;  // f6 = [[[[actor+0x68]+0x40]+0x88]+0x10]+0xF6 (0x117F850..0x117F873) |
| N1210 | src/mod/session2/tree_bend_logic.h:87 | kControl_F6a | RVA / layout / policy (audit) | 0x88 | inline constexpr uint32_t kControl_F6a = 0x88; |
| N1211 | src/mod/session2/tree_bend_logic.h:88 | kControl_F6b | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kControl_F6b = 0x10; |
| N1212 | src/mod/session2/tree_bend_logic.h:89 | kControl_F6 | RVA / layout / policy (audit) | 0xF6 | inline constexpr uint32_t kControl_F6 = 0xF6; |
| N1213 | src/mod/session2/tree_bend_logic.h:90 | kComp_Owner | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kComp_Owner = 0x08; |
| N1214 | src/mod/session2/tree_bend_logic.h:91 | kClientRemoteCatchVtbl | vtable | 0x55890D8 | inline constexpr uintptr_t kClientRemoteCatchVtbl = 0x55890D8;  // pa::ClientRemoteCatchActorComponent |
| N1215 | src/mod/session2/tree_bend_logic.h:92 | kLockVf | signature | {0x18, 0x20, 0x30, 0x40} | inline constexpr uint32_t kLockVf[] = {0x18, 0x20, 0x30, 0x40}; |
| N1216 | src/mod/session2/tree_bend_logic.h:93 | kSlotBase | RVA / layout / policy (audit) | 0x20 | inline constexpr uint32_t kSlotBase = 0x20; |
| N1217 | src/mod/session2/tree_bend_logic.h:94 | kSlotStride | RVA / layout / policy (audit) | 0x58 | inline constexpr uint32_t kSlotStride = 0x58; |
| N1218 | src/mod/session2/tree_bend_logic.h:95 | kSlot_Kind | RVA / layout / policy (audit) | 0x00 | inline constexpr uint32_t kSlot_Kind = 0x00;   // u8, 6 = empty |
| N1219 | src/mod/session2/tree_bend_logic.h:96 | kSlot_Scene | RVA / layout / policy (audit) | 0x10 | inline constexpr uint32_t kSlot_Scene = 0x10;  // 16 bytes (u64 object id, u64 level id) |
| N1220 | src/mod/session2/tree_bend_logic.h:97 | kSlotEmpty | RVA / layout / policy (audit) | 6 | inline constexpr uint8_t kSlotEmpty = 6; |
| N1221 | src/mod/session2/tree_bend_logic.h:98 | kStateBase | RVA / layout / policy (audit) | 0x9BC | inline constexpr uint32_t kStateBase = 0x9BC;  // u8 per slot: 2 flight, 3 attached, 7 idle ... |
| N1222 | src/mod/session2/tree_bend_logic.h:99 | kStateStride | RVA / layout / policy (audit) | 0x1A0 | inline constexpr uint32_t kStateStride = 0x1A0; |
| N1223 | src/mod/session2/tree_bend_logic.h:100 | kStateAttached | RVA / layout / policy (audit) | 3 | inline constexpr uint8_t kStateAttached = 3; |
| N1224 | src/mod/session2/tree_bend_logic.h:101 | kSlots | RVA / layout / policy (audit) | 2 | inline constexpr int kSlots = 2; |
| N1225 | src/mod/session2/tree_bend_logic.h:105 | kState_Amount | RVA / layout / policy (audit) | 0x378 | inline constexpr uint32_t kState_Amount = 0x378; |
| N1226 | src/mod/session2/tree_bend_logic.h:106 | kState_Dir | RVA / layout / policy (audit) | 0x380 | inline constexpr uint32_t kState_Dir = 0x380; |
| N1227 | src/mod/session2/tree_bend_logic.h:108 | kTree_Amount | RVA / layout / policy (audit) | 0x2A4 | inline constexpr uint32_t kTree_Amount = 0x2A4; |
| N1228 | src/mod/session2/tree_bend_logic.h:109 | kTree_Set | RVA / layout / policy (audit) | 0x2B4 | inline constexpr uint32_t kTree_Set = 0x2B4; |
| N1229 | src/mod/session2/tree_bend_logic.h:110 | kTree_Flags | RVA / layout / policy (audit) | 0x59 | inline constexpr uint32_t kTree_Flags = 0x59; |
| N1230 | src/mod/session2/tree_bend_logic.h:111 | kMainThreadId | RVA / layout / policy (audit) | 0x6A652D4 | inline constexpr uintptr_t kMainThreadId = 0x6A652D4;  // dword: the client's main thread |
| N1231 | src/mod/session2/tree_bend_logic.h:122 | kControlOpcode | RVA / layout / policy (audit) | 1 | inline constexpr uint16_t kControlOpcode = 1;  // troc_frame.h kControlOpcode |
| N1232 | src/mod/session2/tree_bend_logic.h:123 | kControlTreeBend | RVA / layout / policy (audit) | 15 | inline constexpr uint8_t kControlTreeBend = 15;  // troc_frame.h: 1-14 taken (5 mount_sync, 12 weather, 13-14 rejoin) |
| N1233 | src/mod/session2/tree_bend_logic.h:124 | kVersion | RVA / layout / policy (audit) | 1 | inline constexpr uint8_t kVersion = 1; |
| N1234 | src/mod/session2/tree_bend_logic.h:125 | kHeader | RVA / layout / policy (audit) | 5 | inline constexpr size_t kHeader = 5;  // u16 opcode &#124; i8 slot &#124; u16 len |
| N1235 | src/mod/session2/tree_bend_logic.h:126 | kPayloadSize | RVA / layout / policy (audit) | 52 | inline constexpr size_t kPayloadSize = 52; |
| N1236 | src/mod/session2/tree_bend_logic.h:127 | kFrameSize | RVA / layout / policy (audit) | kHeader + kPayloadSize | inline constexpr size_t kFrameSize = kHeader + kPayloadSize; |
| N1237 | src/mod/session2/tree_bend_logic.h:130 | kFlagApplyFlag | RVA / layout / policy (audit) | 0x01 | inline constexpr uint8_t kFlagApplyFlag = 0x01;  // 0x1190FD0's r8b (f6 == 1) on that call |
| N1238 | src/mod/session2/tree_bend_logic.h:230 | kMinGapMs | RVA / layout / policy (audit) | 25 | inline constexpr uint32_t kMinGapMs = 25;      // never more than 40 Hz per channel |
| N1239 | src/mod/session2/tree_bend_logic.h:231 | kPeriodMs | RVA / layout / policy (audit) | 50 | inline constexpr uint32_t kPeriodMs = 50;      // 20 Hz while it changes |
| N1240 | src/mod/session2/tree_bend_logic.h:232 | kJump | RVA / layout / policy (audit) | 0.15f | inline constexpr float kJump = 0.15f;          // a change this big goes at once (after kMinGapMs) |
| N1241 | src/mod/session2/tree_bend_logic.h:233 | kStill | RVA / layout / policy (audit) | 0.01f | inline constexpr float kStill = 0.01f;         // below this it counts as still |
| N1242 | src/mod/session2/tree_bend_logic.h:234 | kKeepAliveMs | RVA / layout / policy (audit) | 250 | inline constexpr uint32_t kKeepAliveMs = 250;  // 4 Hz while still |
| N1243 | src/mod/session2/tree_bend_logic.h:235 | kEndIdleMs | RVA / layout / policy (audit) | 150 | inline constexpr uint32_t kEndIdleMs = 150;    // no SetBend this long: the run ended (End sent) |
| N1244 | src/mod/session2/tree_bend_logic.h:252 | kChannels | RVA / layout / policy (audit) | 4 | static constexpr size_t kChannels = 4; |
| N1245 | src/mod/session2/tree_bend_logic.h:409 | kMax | RVA / layout / policy (audit) | 8 | static constexpr size_t kMax = 8; |
| N1246 | src/mod/session2/tree_bend_logic.h:440 | kMaxAgeMs | RVA / layout / policy (audit) | 500 | inline constexpr uint32_t kMaxAgeMs = 500;     // an older sample is not applied (a lost End) |
| N1247 | src/mod/session2/tree_bend_logic.h:441 | kRestartMs | RVA / layout / policy (audit) | 2000 | inline constexpr uint32_t kRestartMs = 2000;   // an older seq after this long of silence: the sender restarted |
| N1248 | src/mod/session2/tree_bend_logic.h:442 | kDelayMs | RVA / layout / policy (audit) | 100 | inline constexpr uint32_t kDelayMs = 100;      // the observer renders this far behind the newest sample |
| N1249 | src/mod/session2/tree_bend_logic.h:443 | kMaxSpanMs | RVA / layout / policy (audit) | 1000 | inline constexpr uint32_t kMaxSpanMs = 1000;   // two samples further apart than this are not interpolated |
| N1250 | src/mod/session2/tree_bend_logic.h:469 | kEntries | RVA / layout / policy (audit) | 4 | static constexpr size_t kEntries = 4; |
| N1251 | src/mod/session2/tree_bend_logic.h:672 | kRunGapUs | RVA / layout / policy (audit) | 200000 | inline constexpr uint64_t kRunGapUs = 200000;  // M1: calls further apart than this start a new run |
| N1252 | src/mod/session2/tree_bend_logic.h:821 | kOwnBendUs | RVA / layout / policy (audit) | kEndIdleMs * 1000ull | inline constexpr uint64_t kOwnBendUs = kEndIdleMs * 1000ull;  // this PC's own bend run counts as open this long |
| N1253 | src/mod/session2/tree_bend_logic.h:848 | kReadTries | RVA / layout / policy (audit) | 4 | static constexpr int kReadTries = 4; |
| N1254 | src/mod/session2/tree_bend_logic.h:887 | kDuringApplyUs | RVA / layout / policy (audit) | 200000 | inline constexpr uint64_t kDuringApplyUs = 200000; |
| N1255 | src/mod/session2/tree_bend_logic.h:892 | kApplyLineUs | RVA / layout / policy (audit) | 1000000 | inline constexpr uint64_t kApplyLineUs = 1000000; |
| N1256 | src/mod/session2/ui_pause_logic.h:107 | kPauseBelow | RVA / layout / policy (audit) | 0.05f | inline constexpr float kPauseBelow = 0.05f; |
| N1257 | src/mod/session2/ui_pause_logic.h:186 | kWorldList | RVA / layout / policy (audit) | 0x38 | static constexpr uint32_t kWorldList = 0x38;   // world -> {entries*, u32 count} |
| N1258 | src/mod/session2/ui_pause_logic.h:187 | kListCount | RVA / layout / policy (audit) | 0x08 | static constexpr uint32_t kListCount = 0x08; |
| N1259 | src/mod/session2/ui_pause_logic.h:188 | kEntryHolder | RVA / layout / policy (audit) | 0xA0 | static constexpr uint32_t kEntryHolder = 0xA0; // entry -> holder (0 = none) |
| N1260 | src/mod/session2/ui_pause_logic.h:189 | kHolderView | RVA / layout / policy (audit) | 0x10 | static constexpr uint32_t kHolderView = 0x10;  // holder -> view V |
| N1261 | src/mod/session2/ui_pause_logic.h:190 | kViewState | RVA / layout / policy (audit) | 0x26F | static constexpr uint32_t kViewState = 0x26F;  // u8: 0/1 open, 2 hiding, 3 unloaded |
| N1262 | src/mod/session2/ui_pause_logic.h:191 | kViewRoot | RVA / layout / policy (audit) | 0x118 | static constexpr uint32_t kViewRoot = 0x118;   // V -> UI root (UIGamePlayControl*) |
| N1263 | src/mod/session2/ui_pause_logic.h:192 | kViewScale | RVA / layout / policy (audit) | 0x23C | static constexpr uint32_t kViewScale = 0x23C;  // f32: the layout's timeScale |
| N1264 | src/mod/session2/ui_pause_logic.h:193 | kRootScaleSlot | RVA / layout / policy (audit) | 0x168 | static constexpr uint32_t kRootScaleSlot = 0x168;  // root vtable slot: 0x63DA70 `return 1.0f` in all of them |
| N1265 | src/mod/session2/ui_pause_logic.h:194 | kMaxEntries | RVA / layout / policy (audit) | 4096 | static constexpr uint32_t kMaxEntries = 4096;  // more = garbage, the replay stops |
| N1266 | src/mod/session2/ui_pause_logic.h:197 | kMaxHits | RVA / layout / policy (audit) | 3 | inline constexpr size_t kMaxHits = 3;  // what one ring event carries |
| N1267 | src/mod/session2/user2_funnels_logic.h:93 | kRespawnVf40Ret | RVA / layout / policy (audit) | 0x2B03AA7 | inline constexpr uint32_t kRespawnVf40Ret = 0x2B03AA7, kTraceVf40Ret = 0x2C4A0AD, kTriggerVf40Ret = 0x2C4A36F; |
| N1268 | src/mod/session2/user2_funnels_logic.h:93 | kTraceVf40Ret | RVA / layout / policy (audit) | 0x2C4A0AD | inline constexpr uint32_t kRespawnVf40Ret = 0x2B03AA7, kTraceVf40Ret = 0x2C4A0AD, kTriggerVf40Ret = 0x2C4A36F; |
| N1269 | src/mod/session2/user2_funnels_logic.h:93 | kTriggerVf40Ret | RVA / layout / policy (audit) | 0x2C4A36F | inline constexpr uint32_t kRespawnVf40Ret = 0x2B03AA7, kTraceVf40Ret = 0x2C4A0AD, kTriggerVf40Ret = 0x2C4A36F; |
| N1270 | src/mod/session2/user2_funnels_logic.h:94 | kQuestEventVf40Ret | RVA / layout / policy (audit) | 0x290DC0B | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1271 | src/mod/session2/user2_funnels_logic.h:94 | kMissionUiVf40Ret | RVA / layout / policy (audit) | 0x2A49183 | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1272 | src/mod/session2/user2_funnels_logic.h:94 | kMissionAlarmVf40Ret | RVA / layout / policy (audit) | 0x2A4A03B | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1273 | src/mod/session2/user2_funnels_logic.h:94 | kChallengeDiscoverVf40Ret | RVA / layout / policy (audit) | 0x104C4B3D | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1274 | src/mod/session2/user2_funnels_logic.h:94 | kChallengeTimerVf40Ret | RVA / layout / policy (audit) | 0x2C3409D | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1275 | src/mod/session2/user2_funnels_logic.h:94 | kChangeCharacterVf40Ret | RVA / layout / policy (audit) | 0x2A4A32B | inline constexpr uint32_t kQuestEventVf40Ret = 0x290DC0B, kMissionUiVf40Ret = 0x2A49183, |
| N1276 | src/mod/session2/user2_funnels_logic.h:97 | kOpTraceMission | Troc ID (audit) | 2372 | inline constexpr uint16_t kOpTraceMission = 2372, kOpMissionTrigger = 2112, kOpMissionUiState = 2477, |
| N1277 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionTrigger | Troc ID (audit) | 2112 | inline constexpr uint16_t kOpTraceMission = 2372, kOpMissionTrigger = 2112, kOpMissionUiState = 2477, |
| N1278 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionUiState | Troc ID (audit) | 2477 | inline constexpr uint16_t kOpTraceMission = 2372, kOpMissionTrigger = 2112, kOpMissionUiState = 2477, |
| N1279 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionNewAlarm | Troc ID (audit) | 2354 | inline constexpr uint16_t kOpTraceMission = 2372, kOpMissionTrigger = 2112, kOpMissionUiState = 2477, |
| N1280 | src/mod/session2/user2_funnels_logic.h:97 | kOpChangePlayable | Troc ID (audit) | 2369 | inline constexpr uint16_t kOpTraceMission = 2372, kOpMissionTrigger = 2112, kOpMissionUiState = 2477, |
| N1281 | src/mod/session2/user2_funnels_logic.h:100 | kOpQuestDialogEnd | Troc ID (audit) | 2342 | inline constexpr uint16_t kOpQuestDialogEnd = 2342, kOpSetQuestDialog = 2857, kOpStageBranchState = 2436, |
| N1282 | src/mod/session2/user2_funnels_logic.h:100 | kOpSetQuestDialog | Troc ID (audit) | 2857 | inline constexpr uint16_t kOpQuestDialogEnd = 2342, kOpSetQuestDialog = 2857, kOpStageBranchState = 2436, |
| N1283 | src/mod/session2/user2_funnels_logic.h:100 | kOpStageBranchState | Troc ID (audit) | 2436 | inline constexpr uint16_t kOpQuestDialogEnd = 2342, kOpSetQuestDialog = 2857, kOpStageBranchState = 2436, |
| N1284 | src/mod/session2/user2_funnels_logic.h:100 | kOpUpdateStageChartEvent | Troc ID (audit) | 2591 | inline constexpr uint16_t kOpQuestDialogEnd = 2342, kOpSetQuestDialog = 2857, kOpStageBranchState = 2436, |
| N1285 | src/mod/session2/user2_funnels_logic.h:254 | kQuestCompActor | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kQuestCompActor = 0x08, kEventType = 0x08, kEventActorKey = 0x68; |
| N1286 | src/mod/session2/user2_funnels_logic.h:254 | kEventType | RVA / layout / policy (audit) | 0x08 | inline constexpr uint32_t kQuestCompActor = 0x08, kEventType = 0x08, kEventActorKey = 0x68; |
| N1287 | src/mod/session2/user2_funnels_logic.h:254 | kEventActorKey | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kQuestCompActor = 0x08, kEventType = 0x08, kEventActorKey = 0x68; |
| N1288 | src/mod/session2/user2_funnels_logic.h:263 | kAckHostQuest | signature | {3009, 3088, 2738, 2346} | inline constexpr uint16_t kAckHostQuest[] = {3009, 3088, 2738, 2346}; |
| N1289 | src/mod/session2/user2_funnels_logic.h:482 | kActorComponents | RVA / layout / policy (audit) | 0x68 | inline constexpr uint32_t kActorComponents = 0x68; |
| N1290 | src/mod/session2/user2_funnels_logic.h:483 | kComponentsQuest | RVA / layout / policy (audit) | 0x138 | inline constexpr uint32_t kComponentsQuest = 0x138;  // index 0x27 (0x2A7FDAE) |
| N1291 | src/mod/session2/user2_funnels_logic.h:484 | kQuestComponentManager | RVA / layout / policy (audit) | 0x18 | inline constexpr uint32_t kQuestComponentManager = 0x18; |
| N1292 | src/mod/session2/user2_funnels_logic.h:521 | kOpcodes | Troc ID (audit) | 4096 | inline constexpr size_t kOpcodes = 4096; |
