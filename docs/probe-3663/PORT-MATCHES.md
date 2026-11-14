# 2976 → 3663 port matches — incomplete

Static comparison only. Nothing in this report proves two-player behavior.
Both executable hashes are in PHASE1-REFERENCES.md. Phase 3 re-hashed the target:
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8.

17 reviewed values are adopted as static identities, all strong with a second
check. These are an incomplete table; no 3663 gameplay path is armed. The three
PE identity fields are separate file facts, not gameplay addresses. Unreviewed
rows below are blocked for adoption, including policy literals whose role has
not yet been classified; they are not all required gameplay offsets.

Methods: Ghidra 12.1.4 headless targeted disassembly/decompilation; Zydis 4.1.0
normalization of all decodable pdata functions; exact named RTTI/COL/table
chains; unique existing short signatures; independently checked call neighbours;
the existing strict pkt3663 entry verifier. Ghidra was used for targeted paired
functions, not a completed Version Tracking/BSim project-wide analysis.

242245 old / 247296 new pdata functions decoded; 2199 / 2290 refused.
57160 unique normalized candidates. Hashes mask relative branches and RIP
displacements; identical wrappers can call different functions, so no hash
match alone is adopted. 26 existing short signatures were checked in both
files, with only 5 unique in the target. All raw discovery outputs say adopted=0;
the explicit reviewed decisions below are the only promotion step.

Evidence: port-evidence/ghidra-2976-expanded.txt,
ghidra-3663-expanded.txt, ghidra-3663-core-handlers.txt,
ghidra-3663-refresh.txt, function-correlations.json, call-neighbors.json,
signature-candidates.json, RTTI JSON files and file-scan-3663.txt.
Tools and commands are in tools/port3663/README.md.

## Reviewed checks

- **send**: Unique whole-function normalized fingerprints in both files; unique short signatures; named NwVirtualAsyncSession slot 9 / NwVirtualSyncSession slot 6; paired Ghidra bodies confirm queue forwarding and result-pointer ABI.
- **queue**: Unique whole-function normalized fingerprint; paired Ghidra lock/queue bodies; both independently matched session-send functions call the same new queue push.
- **alloc**: Unique whole-function normalized fingerprints; paired Ghidra thread/global heap semantics; independent 3663 PostPacket and native session-construction callers select these same allocators.
- **ctor**: Paired Ghidra base-session constructor bodies; exact named NwSession COL/vtable identity; native 3663 async-session construction allocates 0xD0 then calls this constructor with flag zero before installing the derived vtable.
- **tls-index**: Paired allocator instructions load the index via this RIP-relative slot; independent PE TLS directory AddressOfIndex is RVA 0x715BFD0. It is BSS and is not read as file-backed data.
- **tls-flag**: Old constructor/PostPacket use +0x1FD; independently decoded 3663 constructor, PostPacket and native ServerModule initialization use +0x1FE before choosing the same matched heap functions.
- **async-table**: Exact NwVirtualAsyncSession mangled type, self-consistent COL, and primary table in both files; independently matched Send is slot 9; native 3663 construction writes this derived table.
- **async-base-table**: Exact NwAsyncSession mangled type/COL/table in both files; native 3663 construction writes this base table before the derived virtual-session table.
- **size**: Native 3663 initialization allocates 0xD0 for this exact named async-session object; constructor/derived initialization writes through the last qword at +0xC8, consistent with the independently delimited 15-slot table and 2976 construction contract.
- **vtable-count**: Both exact named primary async-session tables contain 15 executable pointers before the next locator/data entry; Send remains at independently matched slot 9.
- **out-queue**: Paired Send bodies read session+0xC8; independent native 3663 ServerModule initialization initializes and assigns that same field before the session is used.
- **frame**: Paired Send bodies convert AsyncRequestSend* to the queue item at +0x30; the independently matched queue push receives exactly that derived pointer, as on 2976.
- **server-recv**: Direct-call-neighbour matching ranks 0x2A0EA70 first (six mapped callees); paired Ghidra bodies independently confirm six-argument receive ABI, framed-length check, server kind/channel dispatch and session handling. This does not validate every downstream payload or session layout.
- **save-refresh**: Named SaveDataSlotRefreshReq RTTI slot 2 leads to the locked busy-flag sequence calling 0x1A18320; independently followed thunk reaches .xtls 0xBB0CBC0, whose paired body enumerates slot folders and rebuilds the same manager table. Other guest save/lifecycle paths remain unverified.

## Blocking core gaps

1. Field sharing H1/H2: old AcquireField 0x2B07790 and EnterField 0x2BC04F0
   have no unique normalized match. Call-neighbour ranking finds only one mapped
   common cleanup callee for each, and no corroborated target. Current field
   locks, user-list offsets and rollback contracts are unproved. Smallest next
   experiment: follow the named 3663 LoginCharacterToFieldServerReq handler
   (RTTI slot 2 at 0x2C6AE10) to field acquisition/entry, then corroborate both
   functions and their field-list writers against the 2976 paired bodies.
2. Identity/status: component-load 0x20688A0 has multiple call-graph candidates
   (first 0x22951B0); status phase 0x2941020 has many near-tied candidates.
   No second check validates the post-load identity hook/register contract,
   status number pointer or lobby/status agreement. Lobby slot 0x50 can be
   located by RTTI, but that alone does not validate the full identity path.
3. Player/SAM/root and reconnect barriers: current singleton provenance,
   controlled-character chain, HP convention, exact-key SAM lookup, per-field
   user/actor counts and native logout/removal completion still need independent
   3663 use-site checks. Reusing the 2976 cleanup map is not acceptable.
4. Wire/login/save completeness: new descriptors identify user login 2554,
   character login 3081, main-user 2420, main-player 2844, field complete 2723,
   refresh 3005 and cryptogram 2401. These remain unadopted IDs until their
   handler consumption and rewritten payloads are audited. In particular,
   cryptogram has kind 0x7FFFFFFE, not a generic server Req. Never copy old IDs,
   header meanings, frame tail/tag usage or payload sizes from 2976.
5. Guest save interception and early load ordering are not validated by the
   one matched refresh thunk. A recognized executable alone cannot safely arm
   the guest role or its 2976 hooks.

Fallback decision: coop3663's existing structural decoders and synthetic Feed
cannot provide native FieldManager sharing, identity fixups, initialized native
characters or deferred destruction. They do not cover these core gaps. Preserve
that code and its tests; do not expand it or present fake-backend results as a
native replacement. No bypass, guessed offset or weaker guard is introduced.

## Named inventory rows

Missing-evidence codes: **A** = classify the row's feature/reachability and,
if build-dependent, confirm its current 3663 code use plus an independent check;
**S** = current whole-instruction signature, verified hook boundary and role;
**T** = named current descriptor plus complete handler/writer consumption;
**V** = exact named type/COL/table plus independently matched slot semantics.
Each unresolved row below names the precise original source/symbol/value.

| Row | Source | Symbol | 2976 | 3663 | Method/check | Confidence | Decision / missing |
|---|---|---|---|---|---|---|---|
| N0001 | src/common/capture_file.h:21 | kFileHeaderSize | 12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0002 | src/common/capture_file.h:22 | kRecordHeaderSize | 24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0003 | src/common/capture_file.h:44 | kGapSession | 0xFFFFFFFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0004 | src/common/client_session.h:39 | kHelloRetryMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0005 | src/common/client_session.h:40 | kPingIntervalMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0006 | src/common/client_session.h:41 | kTimeoutMs | 5000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0007 | src/common/clock_mapper.h:24 | kWindowMs | 4000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0008 | src/common/clock_mapper.h:25 | kSlewPerSample | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0009 | src/common/clock_mapper.h:26 | kJumpThreshold | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0010 | src/common/clock_mapper.h:27 | kLateRunToReset | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0011 | src/common/loop_health.h:33 | kPhases | static_cast<size_t>(Phase::Count) | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0012 | src/common/lz_block.h:29 | kMinMatch | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0013 | src/common/lz_block.h:30 | kMaxOffset | 65535 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0014 | src/common/protocol.h:24 | kMagic | 0x504D4443 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0015 | src/common/protocol.h:31 | kVersion | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0016 | src/common/protocol.h:35 | kHeaderSize | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0017 | src/common/protocol.h:36 | kMaxPacketSize | 1200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0018 | src/common/protocol.h:37 | kMaxNameLen | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0019 | src/common/protocol.h:74 | kMaxEventKind | 12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0020 | src/common/protocol.h:77 | kEnemyPosEventSize | 40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0021 | src/common/protocol.h:82 | kDamageEventSize | 25 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0022 | src/common/protocol.h:83 | kMaxEventPayload | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0023 | src/common/protocol.h:96 | kMaxRelKind | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0024 | src/common/protocol.h:101 | kSelfMarker | 0xA010F00F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0025 | src/common/protocol.h:102 | kRelChunk | 1100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0026 | src/common/protocol.h:103 | kMaxRelMessage | 256 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0027 | src/common/reliable.h:38 | kResendMs | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0028 | src/common/reliable.h:39 | kMaxResendMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0029 | src/common/reliable.h:40 | kMaxResendPerTick | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0030 | src/common/reliable.h:41 | kWindow | 128 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0031 | src/common/reliable.h:42 | kMaxWindow | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0032 | src/common/reliable.h:43 | kMaxQueued | 2048 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0033 | src/common/reliable.h:44 | kMaxPartials | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0034 | src/common/reliable.h:45 | kMaxAhead | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0035 | src/common/save_guard.cpp:10 | kGenericAll | 0x10000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0036 | src/common/save_guard.cpp:11 | kGenericWrite | 0x40000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0037 | src/common/save_guard.cpp:12 | kMaximumAllowed | 0x02000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0038 | src/common/save_guard.cpp:13 | kDelete | 0x00010000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0039 | src/common/save_guard.cpp:14 | kWriteDac | 0x00040000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0040 | src/common/save_guard.cpp:15 | kWriteOwner | 0x00080000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0041 | src/common/save_guard.cpp:16 | kFileWriteData | 0x0002 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0042 | src/common/save_guard.cpp:17 | kFileAppendData | 0x0004 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0043 | src/common/save_guard.cpp:18 | kFileWriteEa | 0x0010 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0044 | src/common/save_guard.cpp:19 | kFileWriteAttributes | 0x0100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0045 | src/common/save_guard.cpp:20 | kWriteMask | kGenericAll &#124; kGenericWrite &#124; kMaximumAllowed &#124; kDelete &#124; kWriteDac &#124; kWriteOwner &#124; kFileWriteData &#124; kFileAppendData &#124; kFileWriteEa &#124; kFileWriteAttributes | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0046 | src/common/save_guard.cpp:23 | kCreateNew | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0047 | src/common/save_guard.cpp:23 | kCreateAlways | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0048 | src/common/save_guard.cpp:23 | kOpenAlways | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0049 | src/common/save_guard.cpp:23 | kTruncateExisting | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0050 | src/common/save_guard.cpp:24 | kFlagDeleteOnClose | 0x04000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0051 | src/common/save_guard.cpp:38 | kAccessBits | { {0x80000000, }, {0x40000000, }, {0x20000000, }, {0x10000000, }, {0x02000000, }, {0x00100000, }, {0x00080000, }, {0x00040000, }, {0x00020000, }, {0x00010000, }, {0x0100, }, {0x0080, }, {0x0040, }, {0x0020, }, {0x0010, }, {0x0008, }, {0x0004, }, {0x0002, }, {0x0001, }, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0052 | src/common/save_guard.cpp:47 | kShareBits | { {0x1, }, {0x2, }, {0x4, }, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0053 | src/common/save_guard.cpp:51 | kFileFlagBits | { {0x80000000, }, {0x40000000, }, {0x20000000, }, {0x10000000, }, {0x08000000, }, {0x04000000, }, {0x02000000, }, {0x01000000, }, {0x00200000, }, {0x0100, }, {0x0080, }, {0x0020, }, {0x0010, }, {0x0004, }, {0x0002, }, {0x0001, }, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0054 | src/common/save_guard.cpp:61 | kMoveBits | { {0x01, }, {0x02, }, {0x04, }, {0x08, }, {0x10, }, {0x20, }, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0055 | src/common/save_guard.cpp:128 | kNames | { , , , , } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0056 | src/common/save_guard.h:84 | kNoMatch | static_cast<size_t>(-1) | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0057 | src/common/sha256.h:129 | kDigits |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0058 | src/common/steam_config.h:20 | kSendBufferSize | 9 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0059 | src/common/steam_config.h:21 | kSendRateMin | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0060 | src/common/steam_config.h:22 | kSendRateMax | 11 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0061 | src/common/steam_config.h:26 | kRecvBufferSize | 47 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0062 | src/common/steam_config.h:27 | kRecvBufferMessages | 48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0063 | src/common/steam_config.h:28 | kMaxScannedId | 512 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0064 | src/common/tick_profile.h:23 | kMaxParts | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0065 | src/common/troc_frame.h:30 | kHeaderSize | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0066 | src/common/troc_frame.h:33 | kMaxPacketSize | 0x8000 - 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0067 | src/common/troc_frame.h:60 | kOpLogoutSessionReq | 2001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0068 | src/common/troc_frame.h:61 | kOpLogoutSessionAck | 2002 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0069 | src/common/troc_frame.h:62 | kOpLogoutUserAck | 2003 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0070 | src/common/troc_frame.h:63 | kInternalHeaderSize | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0071 | src/common/troc_frame.h:64 | kLogoutAckSize | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0072 | src/common/troc_frame.h:78 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0073 | src/common/troc_frame.h:79 | kControlHostReady | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0074 | src/common/troc_frame.h:80 | kControlHostDead | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0075 | src/common/troc_frame.h:83 | kControlHostCharacter | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0076 | src/common/troc_frame.h:87 | kControlRejoin | 13 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0077 | src/common/troc_frame.h:88 | kControlEpoch | 14 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0078 | src/common/troc_frame.h:106 | kCapsOpcode | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0079 | src/common/troc_frame.h:107 | kCapsCompressed | 0x01 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0080 | src/common/troc_frame.h:108 | kCapsGuestSave | 0x02 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0081 | src/common/troc_frame.h:110 | kCapsRejoin | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0082 | src/common/troc_frame.h:116 | kBatchHeader | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0083 | src/common/troc_frame.h:117 | kMaxBatchPackets | 0xFFFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0084 | src/common/troc_frame.h:130 | kCompressedFlag | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0085 | src/common/troc_frame.h:131 | kCompressedHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0086 | src/common/troc_frame.h:132 | kMaxRawBatchBytes | 256 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0087 | src/common/troc_frame.h:185 | kMaxBatchBytes | 64 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0088 | src/common/troc_frame.h:187 | kMaxBacklogBytes | 16 * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0089 | src/common/troc_frame.h:194 | kMaxCompressPerFlush | 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0090 | src/common/troc_frame.h:196 | kMinCompressBytes | 256 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0091 | src/common/troc_link.h:33 | kMaxPendingFragments | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0092 | src/mod/bg_speed_logic.h:84 | kMaxFailures | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0093 | src/mod/config.cpp:12 | kDefaultIni | L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L L | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0094 | src/mod/game/actor_lookup.cpp:15 | kManager_Table | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0095 | src/mod/game/actor_lookup.cpp:16 | kTable_BucketCount | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0096 | src/mod/game/actor_lookup.cpp:17 | kTable_Buckets | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0097 | src/mod/game/actor_lookup.cpp:18 | kTable_Nodes | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0098 | src/mod/game/actor_lookup.cpp:19 | kBucketStride | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0099 | src/mod/game/actor_lookup.cpp:20 | kBucketMaxEntries | (kBucketStride - 8) / 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0100 | src/mod/game/actor_lookup.cpp:21 | kNode_Key | 0x04 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0101 | src/mod/game/actor_lookup.cpp:22 | kNode_Actor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0102 | src/mod/game/actor_lookup.cpp:24 | kComponents_TransformSync | 0x1A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0103 | src/mod/game/actor_lookup.cpp:25 | kTransformSync_Position | 0xB4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0104 | src/mod/game/actor_lookup.cpp:26 | kTransformSync_Sector | 0xC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0105 | src/mod/game/actor_lookup.cpp:27 | kTransformSync_Rotation | 0xA4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0106 | src/mod/game/actor_lookup.cpp:28 | kSectorSize | 1000.0f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0107 | src/mod/game/build_3576.h:13 | kPeTimestamp | 0x6AED2E85 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0108 | src/mod/game/build_3576.h:14 | kImageBase | 0x140000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0109 | src/mod/game/build_3576.h:15 | kSizeOfImage | 0x13CC4000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0110 | src/mod/game/build_guard.h:24 | kMaxELfanew | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0111 | src/mod/game/build_guard.h:25 | kDosMagic | 0x5A4D | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0112 | src/mod/game/build_guard.h:26 | kPeSignature | 0x00004550 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0113 | src/mod/game/build_guard.h:27 | kPe32PlusMagic | 0x020B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0114 | src/mod/game/game_errors.cpp:25 | kErrorLogger | 0x35D8D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0115 | src/mod/game/game_errors.cpp:26 | kErrorLoggerHead | {0xE9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0116 | src/mod/game/game_errors.cpp:27 | kArgCode | 0x50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0117 | src/mod/game/game_errors.cpp:27 | kArgSeverity | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0118 | src/mod/game/game_errors.cpp:27 | kArgText | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0119 | src/mod/game/game_errors.cpp:27 | kArgValue | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0120 | src/mod/game/game_errors.cpp:28 | kExtraValues | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0121 | src/mod/game/game_errors.cpp:69 | kActorCreateFailedFrom | 0x8BDF7C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0122 | src/mod/game/game_fault.cpp:12 | kImageSpan | 0x20000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0123 | src/mod/game/hook_prepare.cpp:16 | kTwoGb | 0x7FFFFFFFull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0124 | src/mod/game/hook_prepare.cpp:17 | kGranule | 0x10000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0125 | src/mod/game/hook_prepare.cpp:18 | kNearWords | 6 + 48 * 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0126 | src/mod/game/hook_prepare.cpp:183 | kWhat | { , , } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0127 | src/mod/game/hook_prepare.h:35 | kHookTries | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0128 | src/mod/game/hook_prepare.h:36 | kHookRetryMs | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0129 | src/mod/game/hook_prepare.h:37 | kNeighbourhoodLines | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0130 | src/mod/game/offsets.h:11 | kPeTimestamp | 0x6AB28F00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0131 | src/mod/game/offsets.h:12 | kImageBase | 0x140000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0132 | src/mod/game/offsets.h:15 | kWorldSystemSlot | 0x6D69190 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0133 | src/mod/game/offsets.h:16 | kChunkOrigin | 0x6D56500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0134 | src/mod/game/offsets.h:17 | kStaticPlayerPos | 0x6D77858 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0135 | src/mod/game/offsets.h:20 | kVtClientActorManager | 0x55B62C8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0136 | src/mod/game/offsets.h:21 | kVtControlledActor | 0x5585C40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0137 | src/mod/game/offsets.h:22 | kVtCharacterControl | 0x55B5460 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0138 | src/mod/game/offsets.h:23 | kVtStatusComponent | 0x558D868 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0139 | src/mod/game/offsets.h:24 | kVtPhysicsObject | 0x5D00F88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0140 | src/mod/game/offsets.h:27 | kWorldSystem_ActorManager | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0141 | src/mod/game/offsets.h:29 | kActorManager_ControlledActor | 0x50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0142 | src/mod/game/offsets.h:31 | kActor_ComponentTable | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0143 | src/mod/game/offsets.h:33 | kComponents_Status | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0144 | src/mod/game/offsets.h:34 | kComponents_CharacterControl | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0145 | src/mod/game/offsets.h:36 | kControl_Inner | 0x140 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0146 | src/mod/game/offsets.h:37 | kControlInner_Physics | 0x2C0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0147 | src/mod/game/offsets.h:39 | kPhysics_AxisX | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0148 | src/mod/game/offsets.h:40 | kPhysics_AxisY | 0x70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0149 | src/mod/game/offsets.h:41 | kPhysics_AxisZ | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0150 | src/mod/game/offsets.h:42 | kPhysics_Position | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0151 | src/mod/game/offsets.h:47 | kStatus_StatContainer | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0152 | src/mod/game/offsets.h:48 | kStatContainer_Entries | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0153 | src/mod/game/offsets.h:49 | kStatContainer_Count | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0154 | src/mod/game/offsets.h:50 | kStatEntry_Size | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0155 | src/mod/game/offsets.h:51 | kStatEntry_Id | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0156 | src/mod/game/offsets.h:52 | kStatEntry_Current | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0157 | src/mod/game/offsets.h:53 | kStatEntry_Max | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0158 | src/mod/game/offsets.h:54 | kStatId_Health | 0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0159 | src/mod/game/offsets.h:55 | kStatId_Stamina | 22 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0160 | src/mod/game/offsets.h:56 | kMaxStatEntries | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0161 | src/mod/game/offsets.h:59 | kCameraSlot | 0x6C8CF30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0162 | src/mod/game/offsets.h:60 | kVtCamera | 0x5D20718 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0163 | src/mod/game/offsets.h:61 | kCamera_Block | 0x428 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0164 | src/mod/game/offsets.h:62 | kCameraBlock_Position | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0165 | src/mod/game/offsets.h:63 | kCameraBlock_Forward | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0166 | src/mod/game/offsets.h:64 | kCameraBlock_View | 0x3E0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0167 | src/mod/game/offsets.h:65 | kCameraBlock_Projection | 0x4E0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0168 | src/mod/game/offsets.h:66 | kCameraBlock_Sentinel | 0xAC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0169 | src/mod/game/offsets.h:67 | kCameraBlockSentinelValue | 6360000.0f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0170 | src/mod/game/offsets.h:71 | kClientPostPacket | 0x35C670 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0171 | src/mod/game/offsets.h:72 | kClientOnRecvPacket | 0x35CA00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0172 | src/mod/game/offsets.h:74 | kClientPostPacketHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x55, 0x56, 0x57, 0x41} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0173 | src/mod/game/offsets.h:76 | kClientOnRecvPacketHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0174 | src/mod/game/offsets.h:79 | kHookArgSizeStackOffset | 0x28 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0175 | src/mod/game/offsets.h:81 | kTrocInstancesBegin | 0x6A51AE0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0176 | src/mod/game/offsets.h:82 | kTrocInstancesEnd | 0x6A82C80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0177 | src/mod/game/offsets.h:83 | kTroc_Kind | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0178 | src/mod/game/offsets.h:84 | kTroc_Opcode | 0x0C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0179 | src/mod/game/offsets.h:86 | kOpActorLoadDataAck | 2005 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0180 | src/mod/game/offsets.h:87 | kOpRemoveActorAck | 2516 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0181 | src/mod/game/offsets.h:88 | kOpMoveActorReq | 2556 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0182 | src/mod/game/offsets.h:89 | kOpMoveActorAck | 2697 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0183 | src/mod/game/offsets.h:90 | kOpStartActionReq | 2223 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0184 | src/mod/game/offsets.h:91 | kOpStartActionAck | 2991 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0185 | src/mod/game/offsets.h:92 | kOpRotateActorReq | 2828 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0186 | src/mod/game/offsets.h:93 | kOpRotateActorAck | 2687 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0187 | src/mod/game/offsets.h:94 | kOpUpdateWallUpDataAck | 2893 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0188 | src/mod/game/offsets.h:95 | kOpUnequipItemAck | 2280 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0189 | src/mod/game/offsets.h:96 | kOpOtherEquipItemAck | 2757 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0190 | src/mod/game/offsets.h:97 | kOpOtherUnequipItemAck | 3073 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0191 | src/mod/game/offsets.h:110 | kOpCreateOrChangePinMarkerAck | 2824 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0192 | src/mod/game/offsets.h:111 | kOpRemovePinMarkerAck | 2992 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0193 | src/mod/game/offsets.h:112 | kCoopPinId | 0x7FDD00000000F001ull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0194 | src/mod/game/offsets.h:113 | kPinGroupNormal | 0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0195 | src/mod/game/offsets.h:114 | kPinShapeCount | 14 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0196 | src/mod/game/offsets.h:114 | kPinColourCount | 15 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0197 | src/mod/game/offsets.h:119 | kMinimapIconDecide | 0x9E89C0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0198 | src/mod/game/offsets.h:120 | kMinimapIconDecideHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x18, 0x55} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0199 | src/mod/game/offsets.h:122 | kMinimapIconRoot | 0x6D69528 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0200 | src/mod/game/offsets.h:123 | kMinimapIconRootAlt | 0x6D69530 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0201 | src/mod/game/offsets.h:124 | kMinimapIconPlayerArrow | 0x126 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0202 | src/mod/game/offsets.h:125 | kMinimapIconCompanion | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0203 | src/mod/game/offsets.h:126 | kMinimapIconOutlinedArrow | 0x124 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0204 | src/mod/game/offsets.h:127 | kActor_Key | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0205 | src/mod/game/offsets.h:135 | kServerOtherEquipWriter | 0x29ED1F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0206 | src/mod/game/offsets.h:136 | kServerOtherEquipWriterHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0207 | src/mod/game/offsets.h:138 | kServerActor_Key | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0208 | src/mod/game/offsets.h:145 | kOpChangeDockingSocketByFrameEventAck | 2747 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0209 | src/mod/game/offsets.h:159 | kRelayedAcks | { {2147, false, 92, {0x54}}, {2586, false, 90, {0x54}}, {2434, false, 41, {}}, {2506, true, 77, {0x18, 0x1C, 0x21}}, {3007, true, 77, {0x18, 0x1C, 0x21}}, {2252, false, 42, {}}, {2642, false, 118, {0x28}}, {2530, true, 19, {0x04}}, {2899, true, 0, {}}, {3053, true, 0, {}}, {2475, true, 0, {}}, {2622, true, 279, {}}, {2763, true, 97, {}}, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0210 | src/mod/game/offsets.h:176 | kServerDockingWriter | 0x2969BC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0211 | src/mod/game/offsets.h:177 | kServerDockingWriterHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0212 | src/mod/game/offsets.h:180 | kLoadRotationFromEnd | 142 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0213 | src/mod/game/offsets.h:184 | kControl_MoveController | 0xC8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0214 | src/mod/game/offsets.h:185 | kMoveController_Method | 0xF0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0215 | src/mod/game/offsets.h:186 | kMoveController_WallUp | 0x120 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0216 | src/mod/game/offsets.h:187 | kVtWallUpMovementMethod | 0x55968B8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0217 | src/mod/game/offsets.h:188 | kMoveMethodWallUp | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0218 | src/mod/game/offsets.h:189 | kControl_WallData | 0xD8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0219 | src/mod/game/offsets.h:194 | kPointClimbEnter | 0x691400 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0220 | src/mod/game/offsets.h:195 | kPointClimbEnterHead | {0x4C, 0x8B, 0xDC, 0x49, 0x89, 0x5B, 0x10, 0x49, 0x89, 0x6B, 0x18, 0x49, 0x89, 0x73, 0x20, 0x57} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0221 | src/mod/game/offsets.h:207 | kActionLayerAdvance | 0x1F63F70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0222 | src/mod/game/offsets.h:208 | kActionLayerAdvanceHead | {0x48, 0x8B, 0xC4, 0x57, 0x48, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0xC5, 0xF8, 0x29, 0x78, 0xD8} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0223 | src/mod/game/offsets.h:218 | kActionLayersAdvance | 0x2329700 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0224 | src/mod/game/offsets.h:219 | kActionLayersAdvanceHead | {0x48, 0x8B, 0xC4, 0x41, 0x54, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x68, 0xC5, 0xF8, 0x29, 0x70, 0xC8} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0225 | src/mod/game/offsets.h:221 | kControl_Layer0 | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0226 | src/mod/game/offsets.h:222 | kLayer_Playback | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0227 | src/mod/game/offsets.h:223 | kLayer_Chart | 0x110 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0228 | src/mod/game/offsets.h:224 | kRideOnEnter | 0x681C80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0229 | src/mod/game/offsets.h:225 | kRideOffEnter | 0x682C20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0230 | src/mod/game/offsets.h:226 | kRideEnterHead | {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x18, 0x55, 0x57, 0x41, 0x54, 0x41} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0231 | src/mod/game/offsets.h:228 | kLoadKindMount | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0232 | src/mod/game/offsets.h:229 | kMethod_Actor | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0233 | src/mod/game/offsets.h:230 | kActor_OwnerUser | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0234 | src/mod/game/offsets.h:233 | kLoadPositionFromEnd | 154 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0235 | src/mod/game/offsets.h:236 | kLoadingParkHeight | 1000.0f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0236 | src/mod/game/offsets.h:237 | kLoadingParkRadius | 5.0f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0237 | src/mod/game/safe_memory.cpp:11 | kMinAddress | 0x10000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0238 | src/mod/game/safe_memory.cpp:12 | kMaxAddress | 0x00007FFFFFFFFFFFull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0239 | src/mod/log.h:50 | kEmergencyFlushMs | 50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0240 | src/mod/log.h:52 | kCrashFlushMs | 400 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0241 | src/mod/log.h:54 | kTerminateFlushMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0242 | src/mod/log.h:60 | kStackLine | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0243 | src/mod/log_ring.h:39 | kCapacity | Capacity | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0244 | src/mod/log_ring.h:41 | kMaxLine | Capacity / 8 < 16384 ? Capacity / 8 : 16384 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0245 | src/mod/mod.cpp:79 | kModBuild | CDMP_BUILD_ID | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0246 | src/mod/mod.cpp:395 | kSlowLines | 200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0247 | src/mod/net/coop.h:30 | kEchoOffset | 2.5f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0248 | src/mod/net/enemy_relay.h:20 | kEnemyCopyBase | 0xB01F0000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0249 | src/mod/net/keys.h:15 | kClassUser | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0250 | src/mod/net/keys.h:16 | kClassPlayer | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0251 | src/mod/net/keys.h:17 | kClassWorld | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0252 | src/mod/net/keys.h:19 | kFirstModIndex | 0x8000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0253 | src/mod/net/keys.h:23 | kMaxGeneration | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0254 | src/mod/net/remote_puppet.cpp:38 | kPuppetUser | 0x9010F001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0255 | src/mod/net/remote_puppet.cpp:39 | kPuppetPlayer | 0xA010F001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0256 | src/mod/net/remote_puppet.cpp:40 | kPuppetBody | 0xB010F001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0257 | src/mod/net/remote_puppet.cpp:43 | kPuppetHorse | 0xB010F002 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0258 | src/mod/net/remote_puppet.cpp:44 | kPuppetHorseParty | 0xB010F003 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0259 | src/mod/net/remote_puppet.cpp:51 | kSenderCharacterNo | 0x7FFFFFFFFFFFFFFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0260 | src/mod/net/remote_puppet.cpp:52 | kPuppetCharacterNo | 0x7FFFFFFFFFFFF001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0261 | src/mod/net/remote_puppet.cpp:54 | kCharacterNoAt | {0x17, 0x43} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0262 | src/mod/net/remote_puppet.cpp:192 | kStaleMovementMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0263 | src/mod/net/remote_puppet.cpp:359 | kMoveSilenceMs | 150 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0264 | src/mod/net/remote_puppet.cpp:368 | kGapMinMs | 150 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0265 | src/mod/net/remote_puppet.cpp:369 | kGapMaxMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0266 | src/mod/net/remote_puppet.cpp:370 | kGapMinJump | 0.5f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0267 | src/mod/net/remote_puppet.cpp:371 | kGapMaxJump | 15.0f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0268 | src/mod/net/remote_puppet.cpp:372 | kGapStepMs | 50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0269 | src/mod/net/remote_puppet.cpp:562 | kHeldTooLongMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0270 | src/mod/net/remote_puppet.cpp:595 | kLayer_CurrentAction | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0271 | src/mod/net/remote_puppet.cpp:1411 | kLead | 40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0272 | src/mod/net/steam_transport.cpp:77 | kIdentitySteamID | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0273 | src/mod/net/steam_transport.cpp:78 | kSendUnreliableNoNagle | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0274 | src/mod/net/steam_transport.cpp:79 | kSendAutoRestartBrokenSession | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0275 | src/mod/net/steam_transport.cpp:80 | kFriendFlagImmediate | 0x04 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0276 | src/mod/net/steam_transport.cpp:81 | kResultOK | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0277 | src/mod/net/steam_transport.cpp:82 | kResultLimitExceeded | 25 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0278 | src/mod/net/steam_transport.cpp:83 | kSessionConnected | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0279 | src/mod/net/steam_transport.cpp:89 | kConfigScopeGlobal | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0280 | src/mod/net/steam_transport.cpp:90 | kConfigTypeInt32 | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0281 | src/mod/net/steam_transport.cpp:91 | kGetConfigOK | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0282 | src/mod/net/steam_transport.cpp:91 | kGetConfigOKInherited | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0283 | src/mod/net/steam_transport.cpp:92 | kAppId | 3321460 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0284 | src/mod/net/steam_transport.cpp:93 | kPollEveryMs | 500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0285 | src/mod/net/steam_transport.cpp:94 | kReadyTimeoutMs | 120000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0286 | src/mod/net/steam_transport.cpp:568 | kBatch | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0287 | src/mod/net/steam_transport.h:78 | kSampleEveryMs | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0288 | src/mod/net/steam_transport.h:79 | kHeldRetryMs | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0289 | src/mod/net/steam_transport.h:80 | kMaxHeldBytes | 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0290 | src/mod/net/troc_sniffer.cpp:40 | kRingBytes | 64ull * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0291 | src/mod/net/troc_sniffer.cpp:41 | kMaxCopy | 32768 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0292 | src/mod/net/troc_sniffer.cpp:42 | kPreviewBytes | 48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0293 | src/mod/net/troc_sniffer.cpp:60 | kRecModExtra | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0294 | src/mod/net/troc_sniffer.cpp:77 | kExtraCap | 512 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0295 | src/mod/net/troc_sniffer.cpp:156 | kMaxC2SWaitMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0296 | src/mod/net/troc_sniffer.cpp:397 | kDiskTextBytes | 1u << 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0297 | src/mod/net/troc_sniffer.cpp:397 | kDiskBinBytes | 4u << 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0298 | src/mod/net/troc_sniffer.cpp:399 | kDiskReserve | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0299 | src/mod/net/troc_sniffer.cpp:400 | kDiskPassBytes | 1u << 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0300 | src/mod/role_logic.h:24 | kEnvVar | L | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0301 | src/mod/role_logic.h:27 | kGuestSaveFolder | L | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0302 | src/mod/session2/child_hold.h:170 | kActorKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0303 | src/mod/session2/child_hold.h:171 | kActorComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0304 | src/mod/session2/child_hold.h:172 | kComponentsCc | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0305 | src/mod/session2/child_hold.h:173 | kComponentsTs | 0x1A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0306 | src/mod/session2/child_hold.h:174 | kActorInfo | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0307 | src/mod/session2/child_hold.h:175 | kInfoType | 0x01 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0308 | src/mod/session2/child_hold.h:176 | kActorOwner | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0309 | src/mod/session2/child_hold.h:177 | kCcActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0310 | src/mod/session2/child_hold.h:178 | kCcRestore | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0311 | src/mod/session2/child_hold.h:179 | kRestoreByte | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0312 | src/mod/session2/child_hold.h:180 | kTsActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0313 | src/mod/session2/child_hold.h:181 | kTsSector | 0x540 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0314 | src/mod/session2/child_hold.h:182 | kSectorObservers | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0315 | src/mod/session2/child_hold.h:183 | kSectorObserverCount | 0x70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0316 | src/mod/session2/child_hold.h:184 | kObserverStride | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0317 | src/mod/session2/child_hold.h:185 | kMaxObservers | 256 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0318 | src/mod/session2/child_hold.h:186 | kUserKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0319 | src/mod/session2/child_hold.h:187 | kChildKeyClass | 0xB | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0320 | src/mod/session2/child_hold.h:188 | kTableCapacity | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0321 | src/mod/session2/child_hold.h:189 | kTimeoutMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0322 | src/mod/session2/child_hold.h:191 | kActorController | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0323 | src/mod/session2/child_hold.h:200 | kNpcTableCapacity | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0324 | src/mod/session2/child_hold.h:203 | kNpcTimeoutMs | 10000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0325 | src/mod/session2/child_hold.h:207 | kLoadDeferMaxMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0326 | src/mod/session2/child_hold.h:209 | kLoadDeferLogged | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0327 | src/mod/session2/child_hold.h:212 | kNpcPassMs | 100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0328 | src/mod/session2/child_hold.h:213 | kNpcPassBudget | 256 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0329 | src/mod/session2/child_hold.h:215 | kNpcFallbackMs | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0330 | src/mod/session2/child_hold.h:220 | kReqWrapperSize | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0331 | src/mod/session2/child_hold.h:221 | kReqWrapperRaw | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0332 | src/mod/session2/child_hold.h:222 | kReqRawLen | 0x03 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0333 | src/mod/session2/child_hold.h:223 | kReqRawPayload | 0x05 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0334 | src/mod/session2/child_hold.h:225 | kCharKeyClass | 0xA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0335 | src/mod/session2/child_hold.h:230 | kUserCharacter | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0336 | src/mod/session2/child_hold.h:232 | kCharTableCapacity | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0337 | src/mod/session2/child_hold.h:236 | kCharTimeoutMs | 15000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0338 | src/mod/session2/child_hold.h:237 | kCharPassMs | 50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0339 | src/mod/session2/child_hold.h:239 | kSettleTableCapacity | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0340 | src/mod/session2/child_hold.h:240 | kAnnounceLogSlots | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0341 | src/mod/session2/child_hold.h:247 | kComponentsHierarchy | 0x1B8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0342 | src/mod/session2/child_hold.h:248 | kHierActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0343 | src/mod/session2/child_hold.h:249 | kHierParent | 0x1C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0344 | src/mod/session2/child_hold.h:253 | kFamilyTableCapacity | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0345 | src/mod/session2/child_hold.h:256 | kFamilyMaxHoldMs | kLoadDeferMaxMs + 3 * kNpcTimeoutMs | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0346 | src/mod/session2/child_hold.h:259 | kFamilyOrphanMs | kNpcTimeoutMs | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0347 | src/mod/session2/child_hold.h:261 | kFamilyKnownSlots | 16384 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0348 | src/mod/session2/child_hold.h:266 | kActorParent | 0xAC | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0349 | src/mod/session2/child_hold.h:268 | kNativeThreads | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0350 | src/mod/session2/child_hold.h:270 | kDeferWarnMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0351 | src/mod/session2/clock_gate.cpp:21 | kTravel | 0x2BC0A30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0352 | src/mod/session2/clock_gate.cpp:22 | kTravelHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0353 | src/mod/session2/clock_gate.cpp:25 | kTravelOwnerCmp | 0x2BC0A78 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0354 | src/mod/session2/clock_gate.cpp:26 | kTravelOwnerCmpBytes | {0x39, 0x82, 0x80, 0x01, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0355 | src/mod/session2/clock_gate.cpp:27 | kTravelGetterCall | 0x2BC0CC7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0356 | src/mod/session2/clock_gate.cpp:28 | kTravelGetterCallBytes | {0xE8, 0xD4, 0x48, 0x52, 0xFF} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0357 | src/mod/session2/clock_gate.cpp:29 | kLoading | 0x219D7F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0358 | src/mod/session2/clock_gate.cpp:30 | kLoadingHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x60, 0x4D, 0x8B, 0xE9, 0x49, 0x8B, 0xF0, 0x4C, 0x8B, 0xF2, 0x48, 0x8B, 0xD9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0359 | src/mod/session2/clock_gate.cpp:34 | kLoadingFlagCmp | 0x219D82B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0360 | src/mod/session2/clock_gate.cpp:35 | kLoadingFlagCmpBytes | {0x44, 0x38, 0x65, 0x50, 0x0F, 0x84, 0x0C, 0x01, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0361 | src/mod/session2/clock_gate.cpp:36 | kGetter | 0x20E55A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0362 | src/mod/session2/clock_gate.cpp:37 | kGetterHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x60, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0xFA, 0x48, 0x8D, 0x54, 0x24, 0x70, 0x48, 0x8B, 0xD9, 0xFF, 0x50, 0x60} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0363 | src/mod/session2/clock_gate.cpp:39 | kMaxLogged | 24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0364 | src/mod/session2/clock_gate.cpp:48 | kMaxWatchLogged | 60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0365 | src/mod/session2/clock_gate.h:41 | kTsActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0366 | src/mod/session2/clock_gate.h:42 | kActorUser | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0367 | src/mod/session2/clock_gate.h:43 | kUserKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0368 | src/mod/session2/clock_gate.h:44 | kTsFieldTime | 0x450 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0369 | src/mod/session2/clock_gate.h:45 | kFieldTimeSize | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0370 | src/mod/session2/clock_gate.h:46 | kWorldTime | 0x6A6D238 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0371 | src/mod/session2/clock_push_logic.h:27 | kOpLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0372 | src/mod/session2/clock_push_logic.h:28 | kOpSetFieldTimeAck | 2816 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0373 | src/mod/session2/clock_push_logic.h:29 | kOpSyncWeatherStateReq | 2560 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0374 | src/mod/session2/clock_push_logic.h:30 | kHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0375 | src/mod/session2/clock_push_logic.h:31 | kFieldTimeMem | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0376 | src/mod/session2/clock_push_logic.h:32 | kFieldTimeWire | 31 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0377 | src/mod/session2/clock_push_logic.h:33 | kAckPayload | 4 + kFieldTimeWire | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0378 | src/mod/session2/clock_push_logic.h:34 | kAckPacket | kHeader + kAckPayload | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0379 | src/mod/session2/clock_push_logic.h:35 | kLoadingAckPayload | 13 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0380 | src/mod/session2/clock_push_logic.h:38 | kServerClock | 0x6A6D238 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0381 | src/mod/session2/clock_push_logic.h:39 | kClientClock | 0x6A6D218 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0382 | src/mod/session2/clock_push_logic.h:40 | kWorldSystem | 0x6D69190 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0383 | src/mod/session2/clock_push_logic.h:41 | kWorldTimeMgr | 0x28 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0384 | src/mod/session2/clock_push_logic.h:42 | kMgrFlags | 0x28 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0385 | src/mod/session2/clock_push_logic.h:44 | kMgrPrivateTime | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0386 | src/mod/session2/clock_push_logic.h:45 | kMgrDumpSize | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0387 | src/mod/session2/clock_push_logic.h:46 | kClockBias | 0x6D69438 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0388 | src/mod/session2/companion_probe.h:48 | kActorKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0389 | src/mod/session2/companion_probe.h:49 | kActorInfo | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0390 | src/mod/session2/companion_probe.h:50 | kActorOwnerUser | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0391 | src/mod/session2/companion_probe.h:51 | kActorState | 0x5E | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0392 | src/mod/session2/companion_probe.h:52 | kActorReady | 0x4A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0393 | src/mod/session2/companion_probe.h:53 | kUserMain | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0394 | src/mod/session2/companion_probe.h:54 | kCcActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0395 | src/mod/session2/companion_probe.h:55 | kCcRestore | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0396 | src/mod/session2/companion_probe.h:56 | kRestoreByte | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0397 | src/mod/session2/companion_probe.h:57 | kCcLayer0 | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0398 | src/mod/session2/companion_probe.h:58 | kCcPending | 0xA8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0399 | src/mod/session2/companion_probe.h:59 | kLayerCurrent | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0400 | src/mod/session2/companion_probe.h:60 | kLayerPlayback | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0401 | src/mod/session2/companion_probe.h:61 | kLayerChart | 0x110 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0402 | src/mod/session2/companion_probe.h:68 | kRestoreSite | 0x8646F2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0403 | src/mod/session2/companion_probe.h:69 | kRestoreSiteBytes | {0xC5, 0xF8, 0x29, 0x70, 0xB8, 0xC5, 0xF8, 0x29, 0x78, 0xA8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xE9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0404 | src/mod/session2/companion_probe.h:71 | kRestoreHook | kRestoreSite + 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0405 | src/mod/session2/companion_probe.h:73 | kRestoreLock | 0x86473F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0406 | src/mod/session2/companion_probe.h:74 | kRestoreLockBytes | {0x49, 0x8B, 0x45, 0x40, 0x44, 0x38, 0x70, 0x30, 0x75, 0x59} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0407 | src/mod/session2/companion_probe.h:78 | kRestoreExit | 0x864B1A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0408 | src/mod/session2/companion_probe.h:79 | kRestoreExitBytes | {0x44, 0x89, 0x36, 0x48, 0x8B, 0xC6, 0x4C, 0x8D, 0x9C, 0x24, 0x60, 0x01, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0409 | src/mod/session2/companion_probe.h:81 | kRestoreExitHook | kRestoreExit + 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0410 | src/mod/session2/companion_probe.h:85 | kRestartCall | 0x864AEA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0411 | src/mod/session2/companion_probe.h:86 | kRestartCallBytes | {0x4C, 0x8D, 0x44, 0x24, 0x50, 0x48, 0x8D, 0x95, 0xA0, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xCD, 0xE8, 0x82, 0xED, 0xFF, 0xFF, 0x8B, 0x00, 0x85, 0xC0, 0x74, 0x04, 0x89, 0x06} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0412 | src/mod/session2/companion_probe.h:92 | kLayerRestoreHash | 0x3C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0413 | src/mod/session2/companion_probe.h:93 | kCcDefaultChart0 | 0x70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0414 | src/mod/session2/fast_travel_logic.h:72 | kOpReloadStageStartReq | 2283 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0415 | src/mod/session2/fast_travel_logic.h:73 | kReloadPayload | 53 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0416 | src/mod/session2/fast_travel_logic.h:168 | kActorComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0417 | src/mod/session2/fast_travel_logic.h:169 | kComponentsSequencer | 0xA8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0418 | src/mod/session2/fast_travel_logic.h:170 | kSeqComponentManager | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0419 | src/mod/session2/fast_travel_logic.h:171 | kManagerActor | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0420 | src/mod/session2/fast_travel_logic.h:174 | kManagerLockObject | 0x8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0421 | src/mod/session2/fast_travel_logic.h:211 | kReloadVf48Ret | 0x2A5F7FC | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0422 | src/mod/session2/fast_travel_logic.h:218 | kApplyVf48Ret | 0x2BC4C53 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0423 | src/mod/session2/fast_travel_logic.h:223 | kSubstituteBlock | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0424 | src/mod/session2/fast_travel_logic.h:289 | kOpReviveLoadingReq | 2362 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0425 | src/mod/session2/field_share.cpp:20 | kAcquireField | 0x2B07790 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0426 | src/mod/session2/field_share.cpp:21 | kAcquireFieldHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x70} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0427 | src/mod/session2/field_share.cpp:24 | kEnterField | 0x2BC04F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0428 | src/mod/session2/field_share.cpp:25 | kEnterFieldHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xC9, 0x48, 0x81, 0xEC} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0429 | src/mod/session2/field_share.cpp:28 | kTryAddRef | 0x14343A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0430 | src/mod/session2/field_share.cpp:29 | kTryAddRefHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0431 | src/mod/session2/field_share.cpp:31 | kVecPush | 0x40B5B0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0432 | src/mod/session2/field_share.cpp:32 | kVecPushHead | {0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0xD9, 0x4C, 0x8B, 0xF2, 0x8B, 0x49, 0x08, 0x8B, 0x43, 0x0C} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0433 | src/mod/session2/field_share.cpp:35 | kUserMainCharacter | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0434 | src/mod/session2/field_share.cpp:36 | kUserKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0435 | src/mod/session2/field_share.cpp:37 | kActorUser | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0436 | src/mod/session2/field_share.cpp:38 | kMaxLogged | 24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0437 | src/mod/session2/field_share.h:41 | kRefs | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0438 | src/mod/session2/field_share.h:42 | kLock1 | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0439 | src/mod/session2/field_share.h:43 | kLock2Ptr | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0440 | src/mod/session2/field_share.h:44 | kOpen | 0x4A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0441 | src/mod/session2/field_share.h:45 | kOwner | 0x180 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0442 | src/mod/session2/field_share.h:46 | kUsers | 0x2A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0443 | src/mod/session2/field_share.h:47 | kUserCount | 0x2A8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0444 | src/mod/session2/field_share.h:48 | kUserActors | 0x2B0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0445 | src/mod/session2/field_share.h:49 | kKey | 0x2C8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0446 | src/mod/session2/field_share.h:50 | kLevelEntry | 0x2D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0447 | src/mod/session2/field_share.h:52 | kTsFieldMover | 0x4F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0448 | src/mod/session2/field_share.h:53 | kFmFieldKey | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0449 | src/mod/session2/field_share.h:54 | kFmField | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0450 | src/mod/session2/field_share.h:55 | kFmBusy | 0x5A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0451 | src/mod/session2/field_share.h:58 | kOutField | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0452 | src/mod/session2/field_share.h:59 | kOutValid | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0453 | src/mod/session2/field_share.h:60 | kOutHolder | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0454 | src/mod/session2/field_share.h:61 | kOutNew | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0455 | src/mod/session2/field_share.h:62 | kOutInfoKey | 0x24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0456 | src/mod/session2/field_share.h:66 | kMgrBucketCount | 0x48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0457 | src/mod/session2/field_share.h:67 | kMgrEntries | 0x4C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0458 | src/mod/session2/field_share.h:68 | kMgrBuckets | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0459 | src/mod/session2/field_share.h:69 | kMgrHolders | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0460 | src/mod/session2/funcnpc_trade_policy.h:19 | kResources | {{ { , 0xA7132561u}, { , 0x978345CAu}, }} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0461 | src/mod/session2/funcnpc_trade_policy.h:46 | kDictSeed | 0x000C5EDEu | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0462 | src/mod/session2/guest.cpp:71 | kOpConnectCryptogramToServerReq | 2319 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0463 | src/mod/session2/guest.cpp:72 | kNoEpoch | 0x7FFFFFFFFFFFFFFFull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0464 | src/mod/session2/guest.cpp:73 | kHostReadyFreshMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0465 | src/mod/session2/guest.cpp:76 | kOpScopeSenderAck | 2042 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0466 | src/mod/session2/guest.cpp:77 | kOpActorAppear | 2005 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0467 | src/mod/session2/guest.cpp:78 | kOpDataAck | 2869 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0468 | src/mod/session2/guest.cpp:79 | kOp2679 | 2679 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0469 | src/mod/session2/guest.cpp:80 | kOpLoginCharacterToFieldServerReq | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0470 | src/mod/session2/guest.cpp:81 | kOpReserveShutDownServerReq | 2790 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0471 | src/mod/session2/guest.cpp:84 | kServerFrameworkMode | 0x6A65564 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0472 | src/mod/session2/guest.cpp:85 | kMaxDecisionLogged | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0473 | src/mod/session2/guest.cpp:85 | kMaxInternalLogged | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0474 | src/mod/session2/guest.cpp:85 | kMaxLoginLogged | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0475 | src/mod/session2/guest.cpp:125 | kRjLines | 40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0476 | src/mod/session2/guest.cpp:195 | kGoodbyeExitCode | 0x2001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0477 | src/mod/session2/guest.cpp:205 | kLocalAfterRemoteExitCode | 0x2003 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0478 | src/mod/session2/guest.cpp:214 | kOpLoginUserToFieldServerReq | 2490 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0479 | src/mod/session2/guest.cpp:215 | kSlotTestKey | ~0ull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0480 | src/mod/session2/guest.cpp:1812 | kReanLines | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0481 | src/mod/session2/guest_actor_diag_logic.h:281 | kUsed | 0x8C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0482 | src/mod/session2/guest_actor_diag_logic.h:281 | kBuckets | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0483 | src/mod/session2/guest_actor_diag_logic.h:281 | kBucketArray | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0484 | src/mod/session2/guest_actor_diag_logic.h:281 | kEntries | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0485 | src/mod/session2/guest_actor_diag_logic.h:282 | kBucketSize | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0486 | src/mod/session2/guest_actor_diag_logic.h:282 | kBucketMax | 31 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0487 | src/mod/session2/guest_actor_diag_logic.h:283 | kEntryKey | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0488 | src/mod/session2/guest_actor_diag_logic.h:283 | kEntryActor | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0489 | src/mod/session2/guest_goodbye_logic.h:32 | kAnswerMs | 5000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0490 | src/mod/session2/guest_goodbye_logic.h:33 | kSilentMs | 5000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0491 | src/mod/session2/guest_goodbye_logic.h:34 | kFieldCapMs | 30000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0492 | src/mod/session2/guest_inbound_logic.h:54 | kMaxPerTurn | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0493 | src/mod/session2/guest_lifecycle.cpp:16 | kMinPtr | 0x10000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0494 | src/mod/session2/guest_lifecycle.cpp:17 | kMaxPtr | 0x00007FFFFFFFFFFFull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0495 | src/mod/session2/guest_lifecycle.cpp:473 | kTagHigh | 0xD0D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0496 | src/mod/session2/guest_lifecycle.h:50 | kSamMapBuckets | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0497 | src/mod/session2/guest_lifecycle.h:51 | kSamMapSize | 0x9C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0498 | src/mod/session2/guest_lifecycle.h:52 | kSamMapBucketBase | 0xA8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0499 | src/mod/session2/guest_lifecycle.h:53 | kSamMapHolders | 0xB0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0500 | src/mod/session2/guest_lifecycle.h:54 | kSamBucketBytes | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0501 | src/mod/session2/guest_lifecycle.h:55 | kSamBucketMaxEntries | (kSamBucketBytes - 8) / 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0502 | src/mod/session2/guest_lifecycle.h:56 | kSamKeyIndexMask | 0xFFFFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0503 | src/mod/session2/guest_lifecycle.h:58 | kSamUserList | 0xB8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0504 | src/mod/session2/guest_lifecycle.h:59 | kSamUserCount | 0xC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0505 | src/mod/session2/guest_lifecycle.h:62 | kSamUserListMax | 0x4000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0506 | src/mod/session2/guest_lifecycle.h:64 | kActorKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0507 | src/mod/session2/guest_lifecycle.h:68 | kUserMainCharacter | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0508 | src/mod/session2/guest_lifecycle.h:69 | kUserComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0509 | src/mod/session2/guest_lifecycle.h:70 | kComponentsSlots | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0510 | src/mod/session2/guest_lifecycle.h:71 | kSlotsArray | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0511 | src/mod/session2/guest_lifecycle.h:72 | kSlotsCount | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0512 | src/mod/session2/guest_lifecycle.h:73 | kSlotStride | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0513 | src/mod/session2/guest_lifecycle.h:185 | kMaxDepartedKeys | 160 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0514 | src/mod/session2/guest_lifecycle.h:196 | kActorState | 0x5E | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0515 | src/mod/session2/guest_lifecycle.h:197 | kActorComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0516 | src/mod/session2/guest_lifecycle.h:198 | kComponentsTransformSync | 0x1A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0517 | src/mod/session2/guest_lifecycle.h:199 | kTsMoving | 0x3DC | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0518 | src/mod/session2/guest_lifecycle.h:200 | kFmMoving | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0519 | src/mod/session2/guest_lifecycle.h:215 | kLoginCharacterReq | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0520 | src/mod/session2/guest_lifecycle.h:216 | kGameLoadingStartReq | 2437 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0521 | src/mod/session2/guest_lifecycle.h:217 | kMoveFieldCompleteReq | 2549 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0522 | src/mod/session2/guest_lifecycle.h:218 | kLoadingCompleteReq | 2445 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0523 | src/mod/session2/guest_lifecycle.h:219 | kMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0524 | src/mod/session2/guest_lifecycle.h:220 | kMoveFieldCompleteAck | 2144 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0525 | src/mod/session2/guest_lifecycle.h:221 | kLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0526 | src/mod/session2/guest_reload_wait.cpp:35 | kWaitHead | 0x779E8B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0527 | src/mod/session2/guest_reload_wait.cpp:36 | kWaitHeadBytes | {0x4C, 0x8B, 0xB7, 0xB8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x5E, 0x10, 0x48, 0x89, 0x5D, 0x40, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x49, 0x8B, 0x76, 0x08, 0x8B, 0x86, 0x80, 0x00, 0x00, 0x00, 0x85, 0xC0, 0x75, 0x1E} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0528 | src/mod/session2/guest_reload_wait.cpp:40 | kWaitLoop | 0x779ED0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0529 | src/mod/session2/guest_reload_wait.cpp:41 | kWaitLoopBytes | {0x48, 0x8B, 0x56, 0x78, 0x8B, 0x12, 0x49, 0x8B, 0x4E, 0x08, 0xE8, 0xB1, 0xA3, 0xCE, 0xFF, 0x48, 0x85, 0xC0, 0x74, 0x13, 0x48, 0x8B, 0x08, 0x48, 0x8B, 0x91, 0xD8, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xC8, 0xFF, 0xD2, 0x84, 0xC0, 0x74, 0x1D} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0530 | src/mod/session2/guest_reload_wait.cpp:45 | kWaitRemove | 0x779EF7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0531 | src/mod/session2/guest_reload_wait.cpp:46 | kWaitRemoveBytes | {0x8B, 0x86, 0x80, 0x00, 0x00, 0x00, 0x83, 0xE8, 0x01, 0x74, 0x09, 0x48, 0x8B, 0x56, 0x78, 0x8B, 0x0C, 0x82, 0x89, 0x0A, 0x83, 0x86, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x75, 0xBC, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0532 | src/mod/session2/guest_reload_wait.cpp:50 | kReady | 0x4FE240 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0533 | src/mod/session2/guest_reload_wait.cpp:51 | kReadyBytes | {0x80, 0xB9, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x75, 0x03, 0x32, 0xC0, 0xC3, 0x0F, 0xB6, 0x81, 0xCF, 0x00, 0x00, 0x00, 0xC3} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0534 | src/mod/session2/guest_reload_wait.cpp:54 | kFind | 0x464290 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0535 | src/mod/session2/guest_reload_wait.cpp:55 | kFindBytes | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x83, 0xB9, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x44, 0x8B, 0xC2, 0x4C, 0x8B, 0xD1, 0x74, 0x4F, 0x8B, 0x89, 0x88, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0536 | src/mod/session2/guest_reload_wait.cpp:58 | kP_State | 0x15C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0537 | src/mod/session2/guest_reload_wait.cpp:58 | kP_Flags | 0x160 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0538 | src/mod/session2/guest_reload_wait.cpp:58 | kP_290 | 0x290 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0539 | src/mod/session2/guest_reload_wait.cpp:58 | kP_Manager | 0xB8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0540 | src/mod/session2/guest_reload_wait.cpp:59 | kM_Lock | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0541 | src/mod/session2/guest_reload_wait.cpp:59 | kM_Container | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0542 | src/mod/session2/guest_reload_wait.cpp:60 | kLock_Acquire | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0543 | src/mod/session2/guest_reload_wait.cpp:60 | kLock_Release | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0544 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_State | 0x5E | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0545 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_C2 | 0xC2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0546 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_C3 | 0xC3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0547 | src/mod/session2/guest_reload_wait.cpp:61 | kActor_CF | 0xCF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0548 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_Parent | 0xAC | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0549 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_Children | 0xB0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0550 | src/mod/session2/guest_reload_wait.cpp:62 | kActor_ChildCount | 0xB8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0551 | src/mod/session2/guest_reload_wait.cpp:63 | kVf_Ready | 0x1D8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0552 | src/mod/session2/guest_reload_wait.cpp:65 | kActor_Comps | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0553 | src/mod/session2/guest_reload_wait.cpp:65 | kComps_Seq | 0xA8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0554 | src/mod/session2/guest_reload_wait.cpp:65 | kSeqComp_Mgr | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0555 | src/mod/session2/guest_reload_wait.cpp:65 | kMgr_Ready | 0x231 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0556 | src/mod/session2/guest_reload_wait.cpp:66 | kVtStageManager | 0x55C9720 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0557 | src/mod/session2/guest_reload_wait.cpp:68 | kSnapMax | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0558 | src/mod/session2/guest_reload_wait.cpp:71 | kDropCap | 128 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0559 | src/mod/session2/guest_reload_wait.cpp:71 | kWarnDrops | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0560 | src/mod/session2/guest_reload_wait.cpp:72 | kGraceUs | 2'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0561 | src/mod/session2/guest_reload_wait.cpp:73 | kStillUs | 5'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0562 | src/mod/session2/guest_reload_wait.cpp:74 | kTraceLines | 800 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0563 | src/mod/session2/guest_reload_wait.cpp:74 | kBlockLines | 240 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0564 | src/mod/session2/guest_reload_wait.cpp:74 | kDropLines | 400 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0565 | src/mod/session2/guest_reload_wait.cpp:74 | kSnapLines | 48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0566 | src/mod/session2/guest_reload_wait.cpp:75 | kShownChildren | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0567 | src/mod/session2/guest_reload_wait_logic.h:27 | kStateIdle | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0568 | src/mod/session2/guest_reload_wait_logic.h:28 | kStateWait | 0x12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0569 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0570 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpReloadPos | 2643 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0571 | src/mod/session2/guest_reload_wait_logic.h:29 | kOpPostAck | 3066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0572 | src/mod/session2/guest_reload_wait_logic.h:31 | kOpTravelReq | 2283 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0573 | src/mod/session2/guest_reload_wait_logic.h:33 | kStaleBeginUs | 10'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0574 | src/mod/session2/guest_reload_wait_logic.h:115 | kStateOwn2322 | 0x11 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0575 | src/mod/session2/guest_reload_wait_logic.h:116 | kOwn2322WarnUs | 10'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0576 | src/mod/session2/guest_reload_wait_logic.h:277 | kKeys | 0x78 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0577 | src/mod/session2/guest_reload_wait_logic.h:277 | kCount | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0578 | src/mod/session2/guest_save_guest.cpp:29 | kKeepIncoming | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0579 | src/mod/session2/guest_save_guest.cpp:30 | kAccountWaitMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0580 | src/mod/session2/guest_save_host.cpp:42 | kLoadSlot | 0x186F570 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0581 | src/mod/session2/guest_save_host.cpp:43 | kLoadSlotHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x55, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x50, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xB0, 0x01, 0x00, 0x00, 0x4D, 0x8B, 0xF9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0582 | src/mod/session2/guest_save_host.cpp:47 | kSave | 0x186E450 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0583 | src/mod/session2/guest_save_host.cpp:48 | kSaveHead | {0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18, 0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x78, 0xFE, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x88, 0x02, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0584 | src/mod/session2/guest_save_host.cpp:53 | kFlush | 0x1871930 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0585 | src/mod/session2/guest_save_host.cpp:54 | kFlushHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x40, 0xFF, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xC0, 0x01, 0x00, 0x00, 0x45, 0x0F, 0xB7, 0xF9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0586 | src/mod/session2/guest_save_host.cpp:59 | kGetOrCreate | 0x2355EC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0587 | src/mod/session2/guest_save_host.cpp:60 | kGetOrCreateHead | {0xE9, 0xDB, 0xD8, 0xBF, 0x0C} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0588 | src/mod/session2/guest_save_host.cpp:61 | kAddFile | 0x235B100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0589 | src/mod/session2/guest_save_host.cpp:62 | kAddFileHead | {0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xEA, 0x4C, 0x8B, 0xF9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0590 | src/mod/session2/guest_save_host.cpp:70 | kCallSites | { {0x2C17881, kLoadSlot}, {0x2C3E1CE, kLoadSlot}, {0x1042ABF3, kLoadSlot}, {0xCBD86CE, kSave}, {0x103BE99A, kSave}, {0x28F273A, kFlush}, {0x108393C8, kFlush}, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0591 | src/mod/session2/guest_save_host.cpp:79 | kErrBusy | {0x186F6DF, 0x6CF7CA4, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0592 | src/mod/session2/guest_save_host.cpp:80 | kErrInvalidSlot | {0x186F62C, 0x6CF7900, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0593 | src/mod/session2/guest_save_host.cpp:81 | kErrAbsentSlot | {0x2069615, 0x6CF781C, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0594 | src/mod/session2/guest_save_host.cpp:82 | kErrSaveFail | {0x186E780, 0x6CF7CA0, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0595 | src/mod/session2/guest_save_host.cpp:85 | kSlotMemoryOnly | 0x699 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0596 | src/mod/session2/guest_save_host.cpp:86 | kSlotInMemory | 0x69C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0597 | src/mod/session2/guest_save_host.cpp:87 | kFileHeader | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0598 | src/mod/session2/guest_save_host.cpp:88 | kActorDescriptor | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0599 | src/mod/session2/guest_save_host.cpp:89 | kActorUser | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0600 | src/mod/session2/guest_save_host.cpp:90 | kActorKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0601 | src/mod/session2/guest_save_host.cpp:91 | kMgrPendingSlot | 0x5C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0602 | src/mod/session2/guest_save_logic.h:77 | kGuestSlot | 204 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0603 | src/mod/session2/guest_save_logic.h:102 | kHeaderSize | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0604 | src/mod/session2/guest_save_logic.h:103 | kMagic | 0x45564153 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0605 | src/mod/session2/guest_save_logic.h:104 | kMaxFileSize | 16ull * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0606 | src/mod/session2/guest_save_logic.h:109 | kMaxRawSize | 64ull * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0607 | src/mod/session2/guest_save_logic.h:110 | kMaxRawRatio | 255 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0608 | src/mod/session2/guest_save_logic.h:199 | kSlotNameMap | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0609 | src/mod/session2/guest_save_logic.h:200 | kMapBucketCount | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0610 | src/mod/session2/guest_save_logic.h:201 | kMapBuckets | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0611 | src/mod/session2/guest_save_logic.h:202 | kMapEntries | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0612 | src/mod/session2/guest_save_logic.h:203 | kBucketSize | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0613 | src/mod/session2/guest_save_logic.h:204 | kBucketPairs | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0614 | src/mod/session2/guest_save_logic.h:205 | kMaxBucketPairs | (kBucketSize - kBucketPairs) / 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0615 | src/mod/session2/guest_save_logic.h:206 | kEntryKey | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0616 | src/mod/session2/guest_save_logic.h:207 | kEntryValue | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0617 | src/mod/session2/guest_save_logic.h:208 | kKeyChars | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0618 | src/mod/session2/guest_save_logic.h:209 | kKeyLength | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0619 | src/mod/session2/guest_save_logic.h:210 | kSlotNames | 0x688 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0620 | src/mod/session2/guest_save_logic.h:211 | kSlotNameCount | 0x690 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0621 | src/mod/session2/guest_save_logic.h:212 | kMaxBuckets | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0622 | src/mod/session2/guest_save_logic.h:213 | kMaxEntryIndex | 1u << 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0623 | src/mod/session2/guest_save_logic.h:214 | kMaxSlotFiles | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0624 | src/mod/session2/guest_save_logic.h:462 | kRetUserLogin | 0x2C17886 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0625 | src/mod/session2/guest_save_logic.h:463 | kRetLobby3093 | 0x2C3E1D3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0626 | src/mod/session2/guest_save_logic.h:464 | kRetCharacter3093 | 0x1042ABF8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0627 | src/mod/session2/guest_save_logic.h:483 | kRetAutosave | 0xCBD86D3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0628 | src/mod/session2/guest_save_logic.h:484 | kRetManual2910 | 0x103BE99F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0629 | src/mod/session2/guest_save_logic.h:501 | kRetFlushLogout | 0x28F273F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0630 | src/mod/session2/guest_save_logic.h:502 | kRetFlushTimer | 0x108393CD | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0631 | src/mod/session2/guest_save_logic.h:624 | kKeepGenerations | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0632 | src/mod/session2/guest_save_logic.h:720 | kDigits |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0633 | src/mod/session2/guest_save_wire.h:52 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0634 | src/mod/session2/guest_save_wire.h:53 | kStateOffer | 6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0635 | src/mod/session2/guest_save_wire.h:53 | kStateChunk | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0636 | src/mod/session2/guest_save_wire.h:53 | kStateAck | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0637 | src/mod/session2/guest_save_wire.h:53 | kStateResult | 9 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0638 | src/mod/session2/guest_save_wire.h:53 | kStateStatus | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0639 | src/mod/session2/guest_save_wire.h:53 | kStateResultAck | 11 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0640 | src/mod/session2/guest_save_wire.h:55 | kWireVersion | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0641 | src/mod/session2/guest_save_wire.h:56 | kResultVersion | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0642 | src/mod/session2/guest_save_wire.h:57 | kCapsGuestSave | 0x02 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0643 | src/mod/session2/guest_save_wire.h:59 | kMaxChunk | 28672 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0644 | src/mod/session2/guest_save_wire.h:60 | kTrocHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0645 | src/mod/session2/guest_save_wire.h:61 | kOfferSize | 97 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0646 | src/mod/session2/guest_save_wire.h:61 | kChunkHeader | 12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0647 | src/mod/session2/guest_save_wire.h:61 | kAckSize | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0648 | src/mod/session2/guest_save_wire.h:61 | kResultSize | 162 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0649 | src/mod/session2/guest_save_wire.h:61 | kStatusSize | 23 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0650 | src/mod/session2/guest_save_wire.h:64 | kLoginHoldMs | 30000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0651 | src/mod/session2/guest_save_wire.h:65 | kFinalHoldMs | 15000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0652 | src/mod/session2/guest_save_wire.h:66 | kOfferRetries | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0653 | src/mod/session2/guest_save_wire.h:67 | kOpSaveAutoReq | 3037 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0654 | src/mod/session2/guest_save_wire.h:68 | kOpSaveToFileCurrentPlayerReq | 2910 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0655 | src/mod/session2/guest_save_wire.h:69 | kOpSaveToFileCurrentPlayerAck | 3016 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0656 | src/mod/session2/guest_save_wire.h:70 | kOpCreateCharacterReq | 2711 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0657 | src/mod/session2/guest_save_wire.h:71 | kOpNewGameStartReq | 2178 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0658 | src/mod/session2/guest_save_wire.h:72 | kOpLoginCharacterReq | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0659 | src/mod/session2/guest_save_wire.h:73 | kOpLogoutSessionReq | 2001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0660 | src/mod/session2/guest_save_wire.h:77 | kFiles | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0661 | src/mod/session2/guest_save_wire.h:374 | kFinalSaveSlot | 100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0662 | src/mod/session2/guest_save_wire.h:375 | kFinalSaveName | u | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0663 | src/mod/session2/guest_save_wire.h:376 | kMaxSaveName | 30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0664 | src/mod/session2/guest_save_wire.h:685 | kDownloadBacklog | 256 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0665 | src/mod/session2/guest_save_wire.h:686 | kResultAckTimeoutMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0666 | src/mod/session2/guest_save_wire.h:687 | kMaxResultAttempts | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0667 | src/mod/session2/guest_save_wire.h:853 | kRecentReqMs | 20000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0668 | src/mod/session2/guest_seq_route_logic.h:39 | kPoints | static_cast<size_t>(Point::Count) | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0669 | src/mod/session2/guest_seq_route_logic.h:40 | kPointOp | {2661, 2981, 2810, 2322, 2524} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0670 | src/mod/session2/guest_seq_route_logic.h:73 | kCharClass | 0xA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0671 | src/mod/session2/guest_seq_route_logic.h:142 | kMaxBranchTable | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0672 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchActorAt | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0673 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchSeqAt | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0674 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchStateAt | 13 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0675 | src/mod/session2/guest_seq_route_logic.h:155 | kBranchMinPayload | 17 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0676 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareActorAt | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0677 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareSeqAt | 9 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0678 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareStageAt | 19 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0679 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareResourceAt | 27 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0680 | src/mod/session2/guest_seq_route_logic.h:158 | kPrepareMinPayload | 31 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0681 | src/mod/session2/guest_seq_route_logic.h:255 | kSeqBinds | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0682 | src/mod/session2/guest_seq_route_logic.h:475 | kSchedPoints | static_cast<size_t>(SchedPoint::Count) | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0683 | src/mod/session2/guest_seq_route_logic.h:476 | kSchedOp | {2335, 2126, 2958, 2909} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0684 | src/mod/session2/guest_seq_route_logic.h:655 | kSchedLinesPerKey | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0685 | src/mod/session2/guest_seq_route_logic.h:668 | kMaxCensusStages | 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0686 | src/mod/session2/guest_seq_route_logic.h:668 | kMaxCensusNodes | 256 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0687 | src/mod/session2/guest_seq_route_logic.h:775 | kBindCap | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0688 | src/mod/session2/guest_seq_route_logic.h:782 | kBucketCount | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0689 | src/mod/session2/guest_seq_route_logic.h:782 | kElements | 0x8C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0690 | src/mod/session2/guest_seq_route_logic.h:782 | kBuckets | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0691 | src/mod/session2/guest_seq_route_logic.h:782 | kNodes | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0692 | src/mod/session2/guest_seq_route_logic.h:783 | kBucketStride | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0693 | src/mod/session2/guest_seq_route_logic.h:783 | kBucketMax | (0x100 - 8) / 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0694 | src/mod/session2/guest_seq_route_logic.h:783 | kNodeKey | 0x04 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0695 | src/mod/session2/guest_seq_route_logic.h:783 | kNodeActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0696 | src/mod/session2/guest_seq_route_logic.h:889 | kRetryTimeoutUs | 120'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0697 | src/mod/session2/guest_seq_route_logic.h:890 | kRetryPeriodUs | 250'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0698 | src/mod/session2/guest_seq_route_logic.h:891 | kRetryCallsPerPass | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0699 | src/mod/session2/guest_seq_route_logic.h:892 | kRetryCap | 512 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0700 | src/mod/session2/guest_seq_route_logic.h:895 | kRetryStillZeroMax | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0701 | src/mod/session2/guest_seq_route_logic.h:896 | kRetryRefusedMax | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0702 | src/mod/session2/guest_seq_route_logic.h:1078 | kActorTypeNpc | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0703 | src/mod/session2/guest_seq_route_logic.h:1134 | kPrefix |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0704 | src/mod/session2/guest_seq_route_logic.h:1160 | kMgr_MapCount | 0x10C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0705 | src/mod/session2/guest_seq_route_logic.h:1160 | kMgr_MapNodes | 0x120 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0706 | src/mod/session2/guest_seq_route_logic.h:1160 | kMapNode_Seq | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0707 | src/mod/session2/guest_seq_route_logic.h:1160 | kMapNode_Inst | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0708 | src/mod/session2/guest_seq_route_logic.h:1160 | kInst_Dead | 0x15 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0709 | src/mod/session2/guest_seq_route_logic.h:1162 | kMaxMapNodes | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0710 | src/mod/session2/guest_seq_route_logic.h:1167 | kStage_KeyOff | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0711 | src/mod/session2/guest_seq_route_logic.h:1167 | kStage_InfoOff | 0x48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0712 | src/mod/session2/guest_seq_route_logic.h:1167 | kInfo_NameOff | 0x1B8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0713 | src/mod/session2/guest_seq_route_logic.h:1167 | kInst_StageOff | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0714 | src/mod/session2/guest_x_veto_logic.h:41 | kCharClass | 0xA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0715 | src/mod/session2/guest_x_veto_logic.h:119 | kContextMaxAgeUs | 500'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0716 | src/mod/session2/guest_x_veto_logic.h:204 | kFns | static_cast<size_t>(Fn::Count) | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0717 | src/mod/session2/guest_x_veto_logic.h:293 | kDefaultAllow |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0718 | src/mod/session2/guest_x_veto_logic.h:397 | kOpLoginCharacterToFieldServerReq | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0719 | src/mod/session2/guest_x_veto_logic.h:398 | kOpMoveFieldCompleteReq | 2549 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0720 | src/mod/session2/guest_x_veto_logic.h:399 | kStateInWorld | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0721 | src/mod/session2/guest_x_veto_logic.h:404 | kStuckMs | 45'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0722 | src/mod/session2/guest_x_veto_logic.h:405 | kStillMs | 20'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0723 | src/mod/session2/hold_scene_order_logic.h:53 | kOpPrepare | 2661 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0724 | src/mod/session2/hold_scene_order_logic.h:53 | kOpBranch | 2981 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0725 | src/mod/session2/hold_scene_order_logic.h:53 | kOpOnBranch | 2524 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0726 | src/mod/session2/hold_scene_order_logic.h:53 | kOpDelete | 2810 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0727 | src/mod/session2/hold_scene_order_logic.h:53 | kOpAnnounce | 2005 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0728 | src/mod/session2/hold_scene_order_logic.h:53 | kOpRemove | 2516 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0729 | src/mod/session2/hold_scene_order_logic.h:53 | kOpMoveField | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0730 | src/mod/session2/hold_scene_order_logic.h:53 | kOpLoadingField | 2643 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0731 | src/mod/session2/hold_scene_order_logic.h:53 | kOpBoundary2756 | 2756 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0732 | src/mod/session2/hold_scene_order_logic.h:53 | kOpChartEvent | 2111 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0733 | src/mod/session2/hold_scene_order_logic.h:74 | kPreparePrefix | 91 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0734 | src/mod/session2/hold_scene_order_logic.h:74 | kBindingWire | 11 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0735 | src/mod/session2/host_save_export.cpp:39 | kMaxManifestBytes | 16ull * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0736 | src/mod/session2/host_save_export.cpp:40 | kMaxJournalBytes | 16ull * 1024 * 1024 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0737 | src/mod/session2/host_save_export.cpp:41 | kKeepJournals | 200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0738 | src/mod/session2/host_save_export.cpp:42 | kMaxDepth | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0739 | src/mod/session2/host_save_logic.h:68 | kDefaultKeep | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0740 | src/mod/session2/host_save_logic.h:69 | kMaxKeep | 500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0741 | src/mod/session2/host_save_logic.h:171 | kPartialSuffix | L | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0742 | src/mod/session2/host_save_logic.h:218 | kAsideInfix | L | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0743 | src/mod/session2/host_save_logic.h:341 | kMaxAside | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0744 | src/mod/session2/host_save_logic.h:510 | kManifestHeader |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0745 | src/mod/session2/host_save_logic.h:511 | kManifestFile |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0746 | src/mod/session2/host_save_logic.h:698 | kJournalHeader |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0747 | src/mod/session2/host_save_logic.h:699 | kJournalTruncated |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0748 | src/mod/session2/host_save_logic.h:700 | kMaxJournalEvents | 4000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0749 | src/mod/session2/host_save_logic.h:727 | kNoSlot | 0xFFFFFFFFu | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0750 | src/mod/session2/host_save_logic.h:952 | kPairGapMs | 10000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0751 | src/mod/session2/identity_fix.cpp:215 | kLobbyLoad | 0x2B7F4D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0752 | src/mod/session2/identity_fix.cpp:216 | kLobbyLoadHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xF2, 0x4C, 0x8B, 0xF9, 0x49, 0x8B, 0x39} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0753 | src/mod/session2/identity_fix.cpp:219 | kLobbyVtable | 0x5B2BE58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0754 | src/mod/session2/identity_fix.cpp:220 | kLobbyConstant | 0x2B7F66C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0755 | src/mod/session2/identity_fix.cpp:221 | kLobbyConstantBytes | {0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x48, 0x89, 0x44, 0x24, 0x70} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0756 | src/mod/session2/identity_fix.cpp:223 | kLobbyAddCall | 0x2B7F6DB | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0757 | src/mod/session2/identity_fix.cpp:224 | kLobbyAddCallBytes | {0xE8, 0x90, 0x47, 0x5C, 0xFF} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0758 | src/mod/session2/identity_fix.cpp:226 | kClearLobbySite | 0x2C3E0FF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0759 | src/mod/session2/identity_fix.cpp:227 | kClearLobbySiteBytes | {0x49, 0x8B, 0x47, 0x68, 0x48, 0x8B, 0x58, 0x10, 0x48, 0x8B, 0x7B, 0x08} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0760 | src/mod/session2/identity_fix.cpp:228 | kClearListSite | 0x2C3E126 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0761 | src/mod/session2/identity_fix.cpp:229 | kClearListSiteBytes | {0x48, 0x8B, 0x4B, 0x18} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0762 | src/mod/session2/identity_fix.cpp:230 | kStatusVtable | 0x5B203F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0763 | src/mod/session2/identity_fix.cpp:231 | kStatusLoad | 0x293FE70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0764 | src/mod/session2/identity_fix.cpp:234 | kLoginMgrSite | 0x2777A18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0765 | src/mod/session2/identity_fix.cpp:235 | kLoginMgrSiteBytes | {0x48, 0x8B, 0x0D, 0x89, 0x1C, 0x5F, 0x04, 0x48, 0x8B, 0x09} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0766 | src/mod/session2/identity_fix.cpp:236 | kLoginMgrSlot | 0x6D696A8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0767 | src/mod/session2/identity_fix.cpp:237 | kFindByNoScan | 0x2A84066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0768 | src/mod/session2/identity_fix.cpp:238 | kFindByNoScanBytes | {0x48, 0x8B, 0x8D, 0xB8, 0x00, 0x00, 0x00, 0x8B, 0x85, 0xC0, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x14, 0xC1} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0769 | src/mod/session2/identity_fix.cpp:240 | kFindByNoRead | 0x2A84090 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0770 | src/mod/session2/identity_fix.cpp:241 | kFindByNoReadBytes | {0x4C, 0x8B, 0x31, 0x49, 0x8B, 0x46, 0x68, 0x48, 0x8B, 0x40, 0x20} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0771 | src/mod/session2/identity_fix.cpp:242 | kLoginMgrActors | 0xB8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0772 | src/mod/session2/identity_fix.cpp:243 | kLoginMgrCount | 0xC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0773 | src/mod/session2/identity_fix.cpp:244 | kOpLoginUser | 2490 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0774 | src/mod/session2/identity_fix.cpp:245 | kOpLoginCharacter | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0775 | src/mod/session2/identity_fix.cpp:246 | kMaxLogged | 48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0776 | src/mod/session2/identity_fix.cpp:246 | kMaxRefusalsLogged | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0777 | src/mod/session2/identity_fix.h:45 | kNative | 0x7FFFFFFFFFFFFFFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0778 | src/mod/session2/identity_fix.h:46 | kUser2 | 0x7FFFFFFFFFFFF002 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0779 | src/mod/session2/identity_fix.h:47 | kOldPuppet | 0x7FFFFFFFFFFFF001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0780 | src/mod/session2/identity_fix.h:48 | kLobbyOwner | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0781 | src/mod/session2/identity_fix.h:49 | kLobbyList | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0782 | src/mod/session2/identity_fix.h:50 | kListCount | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0783 | src/mod/session2/identity_fix.h:51 | kEntryStride | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0784 | src/mod/session2/identity_fix.h:52 | kEntryNumber | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0785 | src/mod/session2/identity_fix.h:53 | kUserComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0786 | src/mod/session2/identity_fix.h:54 | kTableLobby | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0787 | src/mod/session2/identity_fix.h:55 | kTableStatus | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0788 | src/mod/session2/identity_fix.h:56 | kStatusActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0789 | src/mod/session2/identity_fix.h:57 | kStatusNumber | 0x70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0790 | src/mod/session2/identity_fix.h:58 | kActorKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0791 | src/mod/session2/identity_fix.h:59 | kActorUser | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0792 | src/mod/session2/identity_fix.h:60 | kLockObject | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0793 | src/mod/session2/identity_fix.h:61 | kMaxEntries | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0794 | src/mod/session2/identity_fix.h:62 | kMaxActors | 0x4000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0795 | src/mod/session2/identity_fix.h:63 | kCharacterClass | 0xA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0796 | src/mod/session2/identity_probe.cpp:31 | kLoad | 0x20688A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0797 | src/mod/session2/identity_probe.cpp:32 | kLoadHead | {0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x68, 0xFD, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00, 0x49, 0x8B, 0xD9, 0x4D, 0x8B, 0xE8, 0x48, 0x8B, 0xF2, 0x4C, 0x8B, 0xF1} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0798 | src/mod/session2/identity_probe.cpp:39 | kComponentSite | 0x20693CD | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0799 | src/mod/session2/identity_probe.cpp:40 | kComponentSiteBytes | {0x49, 0x8B, 0x07, 0x8B, 0x8D, 0x00, 0x03, 0x00, 0x00, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x8D, 0x4D, 0xC0, 0x44, 0x8B, 0xC6, 0x48, 0x8D, 0x55, 0x30, 0x49, 0x8B, 0xCF, 0xFF, 0x50, 0x50, 0x8B, 0x18, 0x85, 0xDB, 0x0F, 0x85, 0x8D, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0800 | src/mod/session2/identity_probe.cpp:44 | kComponentHook | kComponentSite + 30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0801 | src/mod/session2/identity_probe.cpp:45 | kOwner | 0x2AE1010 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0802 | src/mod/session2/identity_probe.cpp:46 | kOwnerHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x48, 0x89, 0x7C, 0x24, 0x20, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x59, 0x08, 0x49, 0x8B, 0xF9, 0x4D, 0x8B, 0xF0, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0x43, 0x68, 0x48, 0x8B, 0xA8, 0x18, 0x01, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0803 | src/mod/session2/identity_probe.cpp:50 | kPhases | 0x2941020 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0804 | src/mod/session2/identity_probe.cpp:51 | kPhasesHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x44, 0x89, 0x4C, 0x24, 0x20, 0x4C, 0x89, 0x44, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0x6C, 0x24, 0xD9, 0x48, 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xFA, 0x4C, 0x8B, 0x71, 0x08, 0x49, 0x8B, 0x7E, 0x78} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0805 | src/mod/session2/identity_probe.cpp:55 | kAck | 0x28F31F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0806 | src/mod/session2/identity_probe.cpp:56 | kAckHead | {0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0xC8, 0x75, 0xFF, 0xFF, 0xB8, 0x38, 0x8B, 0x00, 0x00, 0xE8, 0xAD, 0x30, 0xFA, 0x01, 0x48, 0x2B, 0xE0, 0xC5, 0xF8, 0x29, 0xB4, 0x24, 0x20, 0x8B, 0x00, 0x00, 0x4C, 0x8B, 0xF1} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0807 | src/mod/session2/identity_probe.cpp:60 | kPacker | 0x17908C0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0808 | src/mod/session2/identity_probe.cpp:61 | kPackerHead | {0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x28, 0x4C, 0x8B, 0xF9, 0x48, 0x8B, 0xDA, 0x48, 0x83, 0xC1, 0x30, 0xE8, 0x8D, 0x8C, 0xBF, 0xFE, 0x41, 0xB8, 0x04, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xCB} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0809 | src/mod/session2/identity_probe.cpp:67 | kVersionThis | 0x179199C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0810 | src/mod/session2/identity_probe.cpp:68 | kVersionThisBytes | {0x49, 0x8B, 0xD9, 0x4C, 0x8B, 0xE2, 0x48, 0x8B, 0xE9, 0x48, 0x8B, 0x41, 0x08} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0811 | src/mod/session2/identity_probe.cpp:69 | kVersionSite | 0x1791B94 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0812 | src/mod/session2/identity_probe.cpp:70 | kVersionSiteBytes | {0x41, 0x8B, 0x43, 0x60, 0x89, 0x05, 0xFA, 0xB1, 0x4F, 0x05, 0x4C, 0x8B, 0x1D, 0xA3, 0x7E, 0x5D, 0x05} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0813 | src/mod/session2/identity_probe.cpp:72 | kVersionHook | kVersionSite + 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0814 | src/mod/session2/identity_probe.cpp:75 | kClientStatus | 0xB742B0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0815 | src/mod/session2/identity_probe.cpp:76 | kClientStatusBytes | {0x48, 0x8B, 0x4B, 0x70, 0x48, 0x8B, 0x45, 0xC0, 0x48, 0x89, 0x01} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0816 | src/mod/session2/identity_probe.cpp:77 | kClientVersionSite | 0xB742F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0817 | src/mod/session2/identity_probe.cpp:78 | kClientVersionSiteBytes | {0x48, 0x8B, 0x4B, 0x18, 0xE8, 0xB7, 0x69, 0xC3, 0x00, 0x48, 0x8B, 0x4B, 0x18, 0xE8, 0xCE, 0x75, 0xC3, 0x00, 0x44, 0x89, 0x35, 0x8F, 0x8A, 0x11, 0x06} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0818 | src/mod/session2/identity_probe.cpp:81 | kClientVersionHook | kClientVersionSite + 18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0819 | src/mod/session2/identity_probe.cpp:82 | kRestoreVersion | 0x6C8CD98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0820 | src/mod/session2/identity_probe.cpp:83 | kStatusVtable | 0x5B203F0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0821 | src/mod/session2/identity_probe.cpp:84 | kStatusLoad | 0x293FE70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0822 | src/mod/session2/identity_probe.cpp:85 | kLoadStub | 0x35EA90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0823 | src/mod/session2/identity_probe.cpp:94 | kMaxHostLines | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0824 | src/mod/session2/identity_probe.cpp:95 | kMaxLogged | 160 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0825 | src/mod/session2/identity_probe.cpp:98 | kWindowMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0826 | src/mod/session2/identity_probe.cpp:136 | kMaxDepth | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0827 | src/mod/session2/identity_probe.h:48 | kActorComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0828 | src/mod/session2/identity_probe.h:49 | kTableStatus | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0829 | src/mod/session2/identity_probe.h:50 | kStatusCharacterNo | 0x70 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0830 | src/mod/session2/identity_probe.h:51 | kComponentOwner | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0831 | src/mod/session2/identity_probe.h:52 | kActorUser | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0832 | src/mod/session2/identity_probe.h:53 | kKey | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0833 | src/mod/session2/load_trip.cpp:22 | kMaxLines | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0834 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0835 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldCompleteAck | 2144 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0836 | src/mod/session2/load_trip_logic.h:32 | kOpPostAck | 3066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0837 | src/mod/session2/load_trip_logic.h:32 | kOpMilestone | 2322 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0838 | src/mod/session2/load_trip_logic.h:32 | kOpLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0839 | src/mod/session2/load_trip_logic.h:32 | kOpBundle | 2042 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0840 | src/mod/session2/load_trip_logic.h:32 | kOpMoveFieldCompleteReq | 2549 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0841 | src/mod/session2/load_trip_logic.h:32 | kOpLoadingCompleteReq | 2445 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0842 | src/mod/session2/load_trip_logic.h:46 | kSettleUs | 6'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0843 | src/mod/session2/load_trip_logic.h:48 | kMaxTrips | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0844 | src/mod/session2/mount_sync_logic.h:45 | kMaxSeats | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0845 | src/mod/session2/mount_sync_logic.h:49 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0846 | src/mod/session2/mount_sync_logic.h:50 | kControlMountSync | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0847 | src/mod/session2/mount_sync_logic.h:51 | kVersion | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0848 | src/mod/session2/mount_sync_logic.h:52 | kHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0849 | src/mod/session2/mount_sync_logic.h:53 | kPayloadSize | 44 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0850 | src/mod/session2/mount_sync_logic.h:69 | kFlagOk | 0x01 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0851 | src/mod/session2/mount_sync_logic.h:70 | kFlagRiderWasLinked | 0x02 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0852 | src/mod/session2/mount_sync_logic.h:71 | kFlagSeatWasTaken | 0x04 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0853 | src/mod/session2/mount_sync_logic.h:76 | kFlagObserved | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0854 | src/mod/session2/mount_sync_logic.h:80 | kClientDetachFlag | 12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0855 | src/mod/session2/mount_sync_logic.h:83 | kFlagNoPublish | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0856 | src/mod/session2/mount_sync_logic.h:168 | kStatusEveryMs | 15000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0857 | src/mod/session2/mount_sync_logic.h:169 | kStatusShowMs | 45000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0858 | src/mod/session2/mount_sync_logic.h:224 | kDepRiderComps | 0x001 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0859 | src/mod/session2/mount_sync_logic.h:225 | kDepRiderCC | 0x002 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0860 | src/mod/session2/mount_sync_logic.h:226 | kDepRiderCCSlot | 0x004 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0861 | src/mod/session2/mount_sync_logic.h:227 | kDepRiderComps50 | 0x008 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0862 | src/mod/session2/mount_sync_logic.h:228 | kDepHorseComps | 0x010 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0863 | src/mod/session2/mount_sync_logic.h:229 | kDepHorseComps1A0 | 0x020 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0864 | src/mod/session2/mount_sync_logic.h:230 | kDepHorseComps90 | 0x040 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0865 | src/mod/session2/mount_sync_logic.h:231 | kDepHorse88 | 0x080 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0866 | src/mod/session2/mount_sync_logic.h:232 | kDepHorseCC | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0867 | src/mod/session2/mount_sync_logic.h:233 | kDepHorseCompsB0 | 0x200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0868 | src/mod/session2/mount_sync_logic.h:276 | kMethodRideOn | 6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0869 | src/mod/session2/mount_sync_logic.h:276 | kMethodRideOff | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0870 | src/mod/session2/mount_sync_logic.h:381 | kCtlGoneMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0871 | src/mod/session2/mount_sync_logic.h:410 | kGeoSpeeds | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0872 | src/mod/session2/mount_sync_logic.h:411 | kGeoErrs | 6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0873 | src/mod/session2/mount_sync_logic.h:412 | kGeoEveryMs | 30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0874 | src/mod/session2/mount_sync_logic.h:434 | kGeoNotLinked | 0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0875 | src/mod/session2/mount_sync_logic.h:434 | kGeoLinked | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0876 | src/mod/session2/mount_sync_logic.h:434 | kGeoSeated | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0877 | src/mod/session2/mount_sync_logic.h:434 | kGeoLinks | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0878 | src/mod/session2/mount_sync_logic.h:438 | kAttachNone | 9 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0879 | src/mod/session2/mount_sync_logic.h:438 | kAttachSeat | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0880 | src/mod/session2/mount_sync_logic.h:444 | kFrozenRiderM | 0.01f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0881 | src/mod/session2/mount_sync_logic.h:444 | kFrozenHorseM | 0.5f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0882 | src/mod/session2/mount_sync_logic.h:492 | kCommitWaitMs | 500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0883 | src/mod/session2/mount_sync_logic.h:493 | kApplyWaitMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0884 | src/mod/session2/mount_sync_logic.h:494 | kDetachForceMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0885 | src/mod/session2/mount_sync_logic.h:498 | kDetachObservedForceMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0886 | src/mod/session2/mount_sync_logic.h:502 | kDetachNoEventMs | 6000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0887 | src/mod/session2/mount_sync_logic.h:505 | kClaimStuckMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0888 | src/mod/session2/mount_sync_logic.h:508 | kMaxAttempts | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0889 | src/mod/session2/mount_sync_logic.h:509 | kHoldMaxPackets | 2048 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0890 | src/mod/session2/mount_sync_logic.h:510 | kHoldMaxBytes | 4u << 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0891 | src/mod/session2/mount_sync_logic.h:511 | kHoldMaxReq | 256 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0892 | src/mod/session2/mount_sync_logic.h:628 | kNativeFallbackMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0893 | src/mod/session2/mount_sync_logic.h:630 | kNativeFallbackCapMs | 30000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0894 | src/mod/session2/mount_sync_logic.h:634 | kNativeUnreadableMs | 15000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0895 | src/mod/session2/mount_sync_logic.h:635 | kNativeDetachFlag | 0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0896 | src/mod/session2/mount_sync_logic.h:682 | kSlots | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0897 | src/mod/session2/mount_sync_logic.h:1098 | kGateEntrySize | 0x78 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0898 | src/mod/session2/mount_sync_logic.h:1099 | kGateEntries | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0899 | src/mod/session2/mount_sync_logic.h:1100 | kGateBytes | kGateEntrySize * kGateEntries | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0900 | src/mod/session2/mount_sync_logic.h:1101 | kEntryHash | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0901 | src/mod/session2/mount_sync_logic.h:1101 | kEntryDef | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0902 | src/mod/session2/mount_sync_logic.h:1101 | kEntryNet | 0x50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0903 | src/mod/session2/mount_sync_logic.h:1101 | kEntryForce | 0x51 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0904 | src/mod/session2/mount_sync_logic.h:1101 | kEntryLocal | 0x52 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0905 | src/mod/session2/mount_sync_logic.h:1102 | kEntryNetBit | 0x02 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0906 | src/mod/session2/mount_sync_logic.h:1102 | kEntryForceBit | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0907 | src/mod/session2/mount_sync_logic.h:1102 | kEntryLocalBit | 0x01 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0908 | src/mod/session2/mount_sync_logic.h:1103 | kDefDismount | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0909 | src/mod/session2/mount_sync_logic.h:1104 | kDefFlags | 0x0A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0910 | src/mod/session2/mount_sync_logic.h:1187 | kGateDegradeAfter | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0911 | src/mod/session2/mount_sync_logic.h:1243 | kActorPlayer | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0912 | src/mod/session2/mount_sync_logic.h:1243 | kActorWorld | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0913 | src/mod/session2/mount_sync_logic.h:1270 | kVf178Actor | 0x4FE260 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0914 | src/mod/session2/mount_sync_logic.h:1270 | kVf178User | 0x8A67D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0915 | src/mod/session2/mount_sync_logic.h:1382 | kSeatReleaseMs | 1500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0916 | src/mod/session2/mount_sync_logic.h:1383 | kSeatRideCapMs | 6000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0917 | src/mod/session2/mount_sync_logic.h:1384 | kSeatLostMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0918 | src/mod/session2/mount_sync_logic.h:1499 | kMcEmptyFlag | 0x3D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0919 | src/mod/session2/mount_sync_logic.h:1529 | kOpStartActionReq | 2223 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0920 | src/mod/session2/observer_hold_acks_logic.h:41 | kOpAnnounce | 2005 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0921 | src/mod/session2/observer_hold_acks_logic.h:42 | kOpScopeSender | 2042 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0922 | src/mod/session2/observer_hold_acks_logic.h:43 | kNpcKeyClass | 0xB | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0923 | src/mod/session2/observer_hold_acks_logic.h:46 | kHeldOps | { 2723, 2404, 2586, 2991, 2697, 2642, 2506, 2402, 2645, 2434, 2356, } | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0924 | src/mod/session2/observer_hold_acks_logic.h:59 | kOpCount | kHeldOps.size() | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0925 | src/mod/session2/observer_hold_acks_logic.h:86 | kCharKeyClass | 0xA | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0926 | src/mod/session2/observer_milestone_logic.h:46 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0927 | src/mod/session2/observer_milestone_logic.h:47 | kOpMoveFieldCompleteAck | 2144 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0928 | src/mod/session2/observer_milestone_logic.h:48 | kOpMoveFieldCompletePostAck | 3066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0929 | src/mod/session2/observer_milestone_logic.h:49 | kOpMilestone | 2322 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0930 | src/mod/session2/observer_milestone_logic.h:50 | kOpLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0931 | src/mod/session2/observer_milestone_logic.h:51 | kOpScopeSenderAck | 2042 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0932 | src/mod/session2/observer_milestone_logic.h:52 | kOpMoveFieldCompleteReq | 2549 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0933 | src/mod/session2/observer_milestone_logic.h:53 | kOpLoadingCompleteReq | 2445 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0934 | src/mod/session2/observer_milestone_reload_logic.h:57 | kOpPreLoadSequencerReq | 2873 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0935 | src/mod/session2/observer_milestone_reload_logic.h:58 | kOpReloadStageStartReq | 2283 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0936 | src/mod/session2/observer_milestone_reload_logic.h:59 | kOpGameLoadingStartReq | 2437 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0937 | src/mod/session2/observer_milestone_reload_logic.h:60 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0938 | src/mod/session2/observer_milestone_reload_logic.h:61 | kOpLoadingFieldDataAck | 2643 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0939 | src/mod/session2/observer_milestone_reload_logic.h:62 | kOpMoveFieldCompletePostAck | 3066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0940 | src/mod/session2/observer_milestone_reload_logic.h:63 | kOpMilestone | 2322 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0941 | src/mod/session2/observer_milestone_reload_logic.h:64 | kOpLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0942 | src/mod/session2/observer_milestone_reload_logic.h:65 | kOpLogoutCharacterAck | 2679 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0943 | src/mod/session2/observer_milestone_reload_logic.h:66 | kOpReviveLoadingReq | 2362 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N0944 | src/mod/session2/observer_milestone_reload_logic.h:68 | kMinDelayUs | 300'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0945 | src/mod/session2/observer_milestone_reload_logic.h:69 | kTimeoutUs | 15'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0946 | src/mod/session2/observer_milestone_reload_logic.h:70 | kAttemptGapUs | 50'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0947 | src/mod/session2/observer_milestone_reload_logic.h:71 | kMaxAttempts | 400 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0948 | src/mod/session2/observer_milestone_reload_logic.h:72 | kMaxAgeUs | 45'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0949 | src/mod/session2/observer_milestone_reload_logic.h:73 | kNativeInReloadLines | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0950 | src/mod/session2/observer_milestone_reload_logic.h:293 | kNoteBase | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0951 | src/mod/session2/offsets_s2.h:12 | kServerNetSlot | 0x6D69680 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0952 | src/mod/session2/offsets_s2.h:13 | kClientNetSlot | 0x6D69170 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0953 | src/mod/session2/offsets_s2.h:14 | kSamSlot | 0x6DA67D8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0954 | src/mod/session2/offsets_s2.h:17 | kServerNet_Session | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0955 | src/mod/session2/offsets_s2.h:18 | kServerNet_InQueue | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0956 | src/mod/session2/offsets_s2.h:19 | kServerNet_FreeC2S | 0x78 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0957 | src/mod/session2/offsets_s2.h:20 | kServerNet_FreeS2C | 0x80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0958 | src/mod/session2/offsets_s2.h:22 | kClientNet_InQueue | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0959 | src/mod/session2/offsets_s2.h:24 | kSam_FreeKeys | 0xD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0960 | src/mod/session2/offsets_s2.h:25 | kSam_ActorCount | 0x9C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0961 | src/mod/session2/offsets_s2.h:28 | kNwVirtualAsyncSessionVtable | 0x57260C0 | 0x5B40310 | async-table | strong | adopted static identity; runtime unarmed |
| N0962 | src/mod/session2/offsets_s2.h:29 | kNwAsyncSessionVtable | 0x5726140 | 0x5B403A8 | async-base-table | strong | adopted static identity; runtime unarmed |
| N0963 | src/mod/session2/offsets_s2.h:30 | kSessionVtableEntries | 15 | 15 | vtable-count | strong | adopted static identity; runtime unarmed |
| N0964 | src/mod/session2/offsets_s2.h:31 | kSessionVfSend | 9 | 9 | async-table | strong | adopted static identity; runtime unarmed |
| N0965 | src/mod/session2/offsets_s2.h:32 | kSessionSize | 0xD0 | 0xD0 | size | strong | adopted static identity; runtime unarmed |
| N0966 | src/mod/session2/offsets_s2.h:33 | kSession_Name | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0967 | src/mod/session2/offsets_s2.h:34 | kSession_IdPtr | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0968 | src/mod/session2/offsets_s2.h:35 | kSession_User | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0969 | src/mod/session2/offsets_s2.h:36 | kSession_Crypto | 0x98 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0970 | src/mod/session2/offsets_s2.h:37 | kSession_B0 | 0xB0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0971 | src/mod/session2/offsets_s2.h:38 | kSession_B8 | 0xB8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0972 | src/mod/session2/offsets_s2.h:39 | kSession_C0 | 0xC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0973 | src/mod/session2/offsets_s2.h:40 | kSession_OutQueue | 0xC8 | 0xC8 | out-queue | strong | adopted static identity; runtime unarmed |
| N0974 | src/mod/session2/offsets_s2.h:43 | kServerUserActorVtable | 0x5B257A8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0975 | src/mod/session2/offsets_s2.h:44 | kUser_Key | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0976 | src/mod/session2/offsets_s2.h:45 | kUser_Session | 0x1F8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0977 | src/mod/session2/offsets_s2.h:47 | kServerActor_Controller | 0x90 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0978 | src/mod/session2/offsets_s2.h:48 | kServerActor_Key | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0979 | src/mod/session2/offsets_s2.h:51 | kServerOnRecvPacket | 0x2773C20 | 0x2A0EA70 | server-recv | strong | adopted static identity; runtime unarmed |
| N0980 | src/mod/session2/offsets_s2.h:52 | kServerOnRecvPacketHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x4C, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x54} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0981 | src/mod/session2/offsets_s2.h:54 | kSessionSend | 0x1437740 | 0x165C1F0 | send | strong | adopted static identity; runtime unarmed |
| N0982 | src/mod/session2/offsets_s2.h:55 | kSessionSendHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x49, 0x8D, 0x40, 0x30} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0983 | src/mod/session2/offsets_s2.h:57 | kNwSessionCtor | 0x1436EE0 | 0x165B9D0 | ctor | strong | adopted static identity; runtime unarmed |
| N0984 | src/mod/session2/offsets_s2.h:58 | kNwSessionCtorHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x6C, 0x24, 0x20, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x56, 0x57, 0x41, 0x54, 0x41, 0x56} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0985 | src/mod/session2/offsets_s2.h:60 | kQueuePush | 0x35DF50 | 0x3AB4C0 | queue | strong | adopted static identity; runtime unarmed |
| N0986 | src/mod/session2/offsets_s2.h:61 | kQueuePushHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF2, 0x48, 0x8B, 0xF9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0987 | src/mod/session2/offsets_s2.h:71 | kAllocGlobal | 0x47EFA78 | 0x4B7E188 | alloc | strong | adopted static identity; runtime unarmed |
| N0988 | src/mod/session2/offsets_s2.h:72 | kAllocGlobalHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xF9, 0x48, 0x8B, 0xDA} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0989 | src/mod/session2/offsets_s2.h:74 | kAllocThread | 0x47EF9B0 | 0x4B7E0C0 | alloc | strong | adopted static identity; runtime unarmed |
| N0990 | src/mod/session2/offsets_s2.h:76 | kAllocThreadBytes | { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48, 0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xFA, 0x44, 0x8B, 0x05, 0x7E, 0x92, 0x49, 0x02, 0x48, 0x8B, 0xF1, 0x4A, 0x8B, 0x1C, 0xC0, 0xB8, 0x98, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x18, 0x00, 0x75, 0x05, 0xE8, 0xDB, 0x59, 0x0A, 0x00, 0xB9, 0x90, 0x00, 0x00, 0x00, 0x4C, 0x8B, 0xC7, 0x48, 0x8B, 0xD6, 0x48, 0x8B, 0x0C, 0x19, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xE9, 0x3C, 0xFC, 0xFF, 0xFF} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N0991 | src/mod/session2/offsets_s2.h:83 | kTlsIndexSlot | 0x6C88C50 | 0x715BFD0 | tls-index | strong | adopted static identity; runtime unarmed |
| N0992 | src/mod/session2/offsets_s2.h:84 | kTls_UseGlobalHeap | 0x1FD | 0x1FE | tls-flag | strong | adopted static identity; runtime unarmed |
| N0993 | src/mod/session2/offsets_s2.h:89 | kServerNet_Threads | 0xF8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0994 | src/mod/session2/offsets_s2.h:90 | kServerNet_ThreadCount | 0x100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0995 | src/mod/session2/offsets_s2.h:91 | kServerThreadEntry | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N0996 | src/mod/session2/offsets_s2.h:95 | kToNearExceptOneControlVtable | 0x5B25358 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0997 | src/mod/session2/offsets_s2.h:96 | kToNearExceptOneSelfVtable | 0x5B25378 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0998 | src/mod/session2/offsets_s2.h:97 | kToNearVtable | 0x5B25418 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N0999 | src/mod/session2/offsets_s2.h:98 | kExceptOneVf1 | 0x2A759B0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1000 | src/mod/session2/offsets_s2.h:99 | kExceptOneSelfVf1 | 0x2A75880 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1001 | src/mod/session2/offsets_s2.h:100 | kToNearVf1 | 0x2A75860 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1002 | src/mod/session2/offsets_s2.h:101 | kExceptOneVf1Head | {0x40, 0x53, 0x48, 0x83, 0xEC, 0x50, 0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x99, 0xA0, 0x01, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1003 | src/mod/session2/offsets_s2.h:103 | kToNearVf1Head | {0x48, 0x8B, 0x41, 0x10, 0x48, 0x8B, 0x48, 0x68, 0x48, 0x8B, 0x89, 0xA0, 0x01, 0x00, 0x00, 0xE9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1004 | src/mod/session2/offsets_s2.h:105 | kFinder_Actor | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1005 | src/mod/session2/offsets_s2.h:107 | kArray_Count | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1006 | src/mod/session2/offsets_s2.h:108 | kArray_Capacity | 0x0C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1007 | src/mod/session2/offsets_s2.h:111 | kPbSize | 0x8030 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1008 | src/mod/session2/offsets_s2.h:112 | kPb_FramePtr | 0x8000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1009 | src/mod/session2/offsets_s2.h:113 | kPb_Len1 | 0x8008 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1010 | src/mod/session2/offsets_s2.h:114 | kPb_Len2 | 0x800A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1011 | src/mod/session2/offsets_s2.h:115 | kPb_Cap | 0x800C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1012 | src/mod/session2/offsets_s2.h:116 | kPb_IsRequest | 0x800E | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1013 | src/mod/session2/offsets_s2.h:117 | kPb_Tag | 0x8010 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1014 | src/mod/session2/offsets_s2.h:118 | kPb_Magic | 0x8018 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1015 | src/mod/session2/offsets_s2.h:119 | kPb_Base | 0x8028 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1016 | src/mod/session2/offsets_s2.h:120 | kFrameOverhead | 0x1F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1017 | src/mod/session2/offsets_s2.h:121 | kFrameHeader | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1018 | src/mod/session2/offsets_s2.h:123 | kRq_Frame | 0x30 | 0x30 | frame | strong | adopted static identity; runtime unarmed |
| N1019 | src/mod/session2/offsets_s2.h:127 | kTrocInstancesBegin | 0x6A51AE0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1020 | src/mod/session2/offsets_s2.h:128 | kTrocInstancesEnd | 0x6A82C80 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1021 | src/mod/session2/offsets_s2.h:129 | kTroc_Kind | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1022 | src/mod/session2/offsets_s2.h:130 | kTroc_Opcode | 0x0C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1023 | src/mod/session2/offsets_s2.h:131 | kTroc_Channel | 0x14 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1024 | src/mod/session2/offsets_s2.h:132 | kTroc_Flags | 0x16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1025 | src/mod/session2/offsets_s2.h:133 | kTrocKindServer | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1026 | src/mod/session2/offsets_s2.h:136 | kClientNet_Session | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1027 | src/mod/session2/offsets_s2.h:137 | kClientNet_FreeC2S | 0x28 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1028 | src/mod/session2/offsets_s2.h:138 | kClientNet_Slot4 | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1029 | src/mod/session2/offsets_s2.h:139 | kSyncSession_Epoch | 0xB0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1030 | src/mod/session2/offsets_s2.h:140 | kSyncSessionSend | 0x14380B0 | 0x165CB60 | send | strong | adopted static identity; runtime unarmed |
| N1031 | src/mod/session2/offsets_s2.h:141 | kSyncSessionSendHead | {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x89, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDA} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1032 | src/mod/session2/offsets_s2.h:143 | kNetLoopPump | 0x35BAD4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1033 | src/mod/session2/offsets_s2.h:144 | kNetLoopPumpHead | {0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1034 | src/mod/session2/offsets_s2.h:153 | kNetLoopPumpPatch | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1035 | src/mod/session2/offsets_s2.h:154 | kNetLoopPumpAob | { 0x49, 0x8B, 0x87, 0xE0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x38, 0x48, 0x89, 0x7D, 0xC7, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x08, 0x90, 0x8B, 0x5F, 0x58, 0x48, 0x8B, 0x07, 0x48, 0x8B, 0xCF, 0xFF, 0x50, 0x10, 0x85, 0xDB, 0x74} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1036 | src/mod/session2/offsets_s2.h:157 | kClientOnRecvPacket | 0x35CA00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1037 | src/mod/session2/offsets_s2.h:158 | kClientOnRecvPacketHead | {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1038 | src/mod/session2/offsets_s2.h:165 | kWorldSystemSlot | 0x6D69190 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1039 | src/mod/session2/offsets_s2.h:166 | kWorldSystem_SaveManager | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1040 | src/mod/session2/offsets_s2.h:167 | kServerActorLookupSlot | 0x6D69698 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1041 | src/mod/session2/offsets_s2.h:168 | kServerActorLookup_SaveManager | 0x48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1042 | src/mod/session2/offsets_s2.h:170 | kSaveMgr_Lock | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1043 | src/mod/session2/offsets_s2.h:171 | kSaveMgr_LockObj | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1044 | src/mod/session2/offsets_s2.h:172 | kSaveMgr_Slots | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1045 | src/mod/session2/offsets_s2.h:173 | kSaveMgr_Count | 0x48 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1046 | src/mod/session2/offsets_s2.h:174 | kSaveMgr_Capacity | 0x4C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1047 | src/mod/session2/offsets_s2.h:175 | kSaveMgr_Storage | 0x50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1048 | src/mod/session2/offsets_s2.h:176 | kSaveMgr_Busy | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1049 | src/mod/session2/offsets_s2.h:177 | kSaveMgr_EntrySize | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1050 | src/mod/session2/offsets_s2.h:178 | kSaveStorage_FolderCount | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1051 | src/mod/session2/offsets_s2.h:179 | kWindowsLock3Vtable | 0x557C120 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N1052 | src/mod/session2/offsets_s2.h:180 | kWindowsLockVf1 | 0x1371340 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1053 | src/mod/session2/offsets_s2.h:181 | kWindowsLockVf2 | 0xCE4EF0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1054 | src/mod/session2/offsets_s2.h:182 | kWindowsLockVf1Head | {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x18} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1055 | src/mod/session2/offsets_s2.h:183 | kWindowsLockVf2Head | {0x48, 0x8B, 0x01, 0x48, 0xFF, 0x60, 0x20} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1056 | src/mod/session2/offsets_s2.h:185 | kSaveMgrRefreshSlots | 0x1870C20 | 0x1A18320 | save-refresh | strong | adopted static identity; runtime unarmed |
| N1057 | src/mod/session2/offsets_s2.h:186 | kSaveMgrRefreshSlotsHead | {0xE9, 0xAB, 0x86, 0x36, 0x0B} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1058 | src/mod/session2/offsets_s2.h:190 | kSlotRefreshReqSequence | 0x2A22475 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1059 | src/mod/session2/offsets_s2.h:191 | kSlotRefreshReqSequenceBytes | { 0x48, 0x8B, 0x05, 0x1C, 0x72, 0x34, 0x04, 0x4C, 0x8B, 0x78, 0x48, 0x4C, 0x89, 0x7D, 0x07, 0x45, 0x32, 0xF6, 0x44, 0x88, 0x75, 0x0F, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x45, 0x38, 0x77, 0x60, 0x75, 0x0C, 0x41, 0xC6, 0x47, 0x60, 0x01, 0x41, 0xB6, 0x01, 0x44, 0x88, 0x75, 0x0F, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10, 0x90, 0x45, 0x84, 0xF6, 0x75, 0x05, 0xE9, 0x91, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0xCF, 0x71, 0x34, 0x04, 0x48, 0x8B, 0x49, 0x48, 0xE8, 0x4E, 0xE7, 0xE4, 0xFE, 0x90, 0x49, 0x8B, 0x1F, 0x48, 0x89, 0x5D, 0x77, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x08, 0x90, 0x41, 0xC6, 0x47, 0x60, 0x00, 0x48, 0x8B, 0x03, 0x48, 0x8B, 0xCB, 0xFF, 0x50, 0x10} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1060 | src/mod/session2/offsets_s2.h:211 | kTitlePlayReadsSaveMgr | 0x65DC3E | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1061 | src/mod/session2/offsets_s2.h:212 | kTitlePlayReadsSaveMgrBytes | {0x4C, 0x8B, 0x1D, 0x4B, 0xB5, 0x70, 0x06, 0x49, 0x8B, 0x83, 0x88, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1062 | src/mod/session2/offsets_s2.h:215 | kImageBase | 0x140000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1063 | src/mod/session2/offsets_s2.h:216 | kPeTimestamp | 0x6AB28F00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1064 | src/mod/session2/p8_probe_logic.h:150 | kMaxProbe | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1065 | src/mod/session2/p8_probe_logic.h:151 | kProbe | N < kMaxProbe ? N : kMaxProbe | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1066 | src/mod/session2/p8_probe_logic.h:152 | kMaxKeys | N / 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1067 | src/mod/session2/p8_probe_logic.h:245 | kTable | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1068 | src/mod/session2/p8_probe_logic.h:245 | kCount | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1069 | src/mod/session2/p8_probe_logic.h:245 | kTripleSize | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1070 | src/mod/session2/p8_probe_logic.h:245 | kHashInThird | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1071 | src/mod/session2/p8_probe_logic.h:355 | kHashSeqBodyAnim | 0x837487D4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1072 | src/mod/session2/p8_probe_logic.h:356 | kHashSeqBodyAnimLoop | 0x35DD392F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1073 | src/mod/session2/reannounce_logic.h:40 | kControlReannounce | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1074 | src/mod/session2/reannounce_logic.h:41 | kPayloadSize | 10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1075 | src/mod/session2/reannounce_logic.h:42 | kMaxAttempts | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1076 | src/mod/session2/reannounce_logic.h:43 | kMinGapMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1077 | src/mod/session2/reannounce_logic.h:76 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1078 | src/mod/session2/reannounce_logic.h:77 | kFrameSize | 5 + kPayloadSize | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1079 | src/mod/session2/reannounce_logic.h:110 | kVerdicts | 6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1080 | src/mod/session2/reannounce_logic.h:125 | kMaxPerEpoch | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1081 | src/mod/session2/rejoin_logic.h:47 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1082 | src/mod/session2/rejoin_logic.h:48 | kControlRejoin | 13 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1083 | src/mod/session2/rejoin_logic.h:49 | kControlEpoch | 14 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1084 | src/mod/session2/rejoin_logic.h:50 | kCapsRejoin | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1085 | src/mod/session2/rejoin_logic.h:51 | kWireVersion | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1086 | src/mod/session2/rejoin_logic.h:52 | kRejoinPayload | 11 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1087 | src/mod/session2/rejoin_logic.h:52 | kEpochPayload | 12 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1088 | src/mod/session2/rejoin_logic.h:53 | kRetryMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1089 | src/mod/session2/rejoin_logic.h:54 | kMaxAsks | 30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1090 | src/mod/session2/rejoin_logic.h:223 | kGrantUnusedMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1091 | src/mod/session2/report_guard.cpp:19 | kReporter | 0x1317D40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1092 | src/mod/session2/report_guard.cpp:20 | kReporterHead | {0x48, 0x89, 0x5C, 0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1093 | src/mod/session2/report_guard.cpp:22 | kMaxPaths | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1094 | src/mod/session2/report_guard.cpp:23 | kMaxText | 260 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1095 | src/mod/session2/report_guard.cpp:24 | kMaxLogged | 16 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1096 | src/mod/session2/ride_filter_logic.h:50 | kOpStartActionAck | 2991 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1097 | src/mod/session2/ride_filter_logic.h:51 | kHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1098 | src/mod/session2/ride_filter_logic.h:52 | kAckSize | 725 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1099 | src/mod/session2/ride_filter_logic.h:53 | kA0 | 0x004 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1100 | src/mod/session2/ride_filter_logic.h:54 | kA1 | 0x049 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1101 | src/mod/session2/ride_filter_logic.h:55 | kBlock | 69 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1102 | src/mod/session2/ride_filter_logic.h:56 | kBlockHash | 0x04 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1103 | src/mod/session2/ride_filter_logic.h:57 | kParent | 0x117 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1104 | src/mod/session2/ride_filter_logic.h:58 | kTarget | 0x16D | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1105 | src/mod/session2/ride_filter_logic.h:60 | kMountA0 | {0x91605616u, 0x872A6750u} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1106 | src/mod/session2/ride_filter_logic.h:61 | kDismountA1 | 0x7B6AFF57u | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1107 | src/mod/session2/ride_filter_logic.h:62 | kFootHash | 0x26107D3Cu | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1108 | src/mod/session2/ride_filter_logic.h:63 | kRidingHash | 0xF79C7424u | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1109 | src/mod/session2/ride_filter_logic.h:67 | kBlockFoot | { 0x00, 0x00, 0x00, 0x00, 0x3C, 0x7D, 0x10, 0x26, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x89, 0x88, 0x08, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD7, 0xA3, 0x70, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1110 | src/mod/session2/ride_filter_logic.h:72 | kBlockRiding | { 0x00, 0x00, 0x00, 0x00, 0x24, 0x74, 0x9C, 0xF7, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x89, 0x88, 0x08, 0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD7, 0xA3, 0x70, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1111 | src/mod/session2/ride_filter_logic.h:77 | kBlockEmpty | { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x57, 0x57, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1112 | src/mod/session2/ride_filter_logic.h:121 | kKinds | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1113 | src/mod/session2/ride_filter_logic.h:181 | kWindowMs | 3000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1114 | src/mod/session2/ride_filter_logic.h:222 | kSlots | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1115 | src/mod/session2/ride_filter_logic.h:302 | kGateDismountA0 | {0xC9356CEBu, 0x412C2C2Bu} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1116 | src/mod/session2/ride_filter_logic.h:307 | kSlots | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1117 | src/mod/session2/save_hooks.cpp:142 | kTraceLines | 5000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1118 | src/mod/session2/save_hooks.cpp:143 | kTraceNonWriteLines | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1119 | src/mod/session2/save_hooks.cpp:259 | kName |  | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1120 | src/mod/session2/save_hooks.cpp:281 | kRedirectLines | 200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1121 | src/mod/session2/save_hooks.cpp:282 | kRedirectErrorLines | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1122 | src/mod/session2/save_hooks.cpp:444 | kWrite | [](const auto& | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1123 | src/mod/session2/save_hooks.cpp:445 | kQuery | [](const auto& | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1124 | src/mod/session2/save_hooks.cpp:447 | kWriteIfSecond | [](const auto& m | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1125 | src/mod/session2/save_hooks.cpp:449 | kBoolText | [](BOOL ok | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1126 | src/mod/session2/save_hooks.cpp:450 | kHandleText | [](HANDLE h | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1127 | src/mod/session2/save_hooks.cpp:451 | kNoText | [](auto | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1128 | src/mod/session2/server.cpp:68 | kMagic | 0x32534553'44504F43ull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1129 | src/mod/session2/server.cpp:76 | kSessionId | 0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1130 | src/mod/session2/server.cpp:77 | kMaxOutgoingBytes | 32u << 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1131 | src/mod/session2/server.cpp:114 | kEpochSlots | kMaxGuestEpochs + 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1132 | src/mod/session2/server.cpp:123 | kNotReq | 0xFF | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1133 | src/mod/session2/server.cpp:123 | kBlocked | 0xFE | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1134 | src/mod/session2/server.cpp:133 | kOpLoginCharacterToFieldServerReq | 3093 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1135 | src/mod/session2/server.cpp:134 | kOpLoginUserToFieldServerReq | 2490 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1136 | src/mod/session2/server.cpp:137 | kOpMoveActorReq | 2556 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1137 | src/mod/session2/server.cpp:137 | kOpStartActionReq | 2223 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1138 | src/mod/session2/server.cpp:137 | kOpRotateActorReq | 2828 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1139 | src/mod/session2/server.cpp:138 | kOpMainPlayerDataStartAck | 2756 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1140 | src/mod/session2/server.cpp:139 | kOpMainUserDataStartAck | 2646 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1141 | src/mod/session2/server.cpp:140 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1142 | src/mod/session2/server.cpp:159 | kMaxMirrored | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1143 | src/mod/session2/server.cpp:168 | kLogoutPendingMs | 60000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1144 | src/mod/session2/server.cpp:181 | kOpNak | 1013 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1145 | src/mod/session2/server.cpp:182 | kOpRemoveActorAck2516 | 2516 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1146 | src/mod/session2/server.cpp:207 | kMaxExceptionNotes | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1147 | src/mod/session2/server.h:152 | kMaxGuestEpochs | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1148 | src/mod/session2/session2.cpp:80 | kMaxEarly | 512 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1149 | src/mod/session2/session2.cpp:82 | kHoldRetryMs | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1150 | src/mod/session2/session2.cpp:93 | kLinkLostMs | 15000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1151 | src/mod/session2/session2.cpp:109 | kSampleMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1152 | src/mod/session2/session2.cpp:109 | kBarrierMs | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1153 | src/mod/session2/session2.cpp:120 | kRejoinLines | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1154 | src/mod/session2/session2.cpp:161 | kSlowTickUs | 50'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1155 | src/mod/session2/session2.cpp:162 | kSlowTickLines | 200 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1156 | src/mod/session2/session2.cpp:276 | kOpSetFieldTimeReq | 2125 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1157 | src/mod/session2/session2.cpp:1471 | kReanLines | 32 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1158 | src/mod/session2/slot_refresh.h:75 | kEntrySize | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1159 | src/mod/session2/slot_refresh.h:76 | kMaxEntriesRead | 64 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1160 | src/mod/session2/travel_probe.h:55 | kTravelContext | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1161 | src/mod/session2/travel_probe.h:55 | kTravelPrepared | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1162 | src/mod/session2/travel_probe.h:55 | kTravelConsume | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1163 | src/mod/session2/travel_probe.h:55 | kTravelRevive | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1164 | src/mod/session2/travel_probe.h:59 | kTravelHead | 24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1165 | src/mod/session2/travel_probe_logic.h:26 | kOpReloadStageStartReq | 2283 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1166 | src/mod/session2/travel_probe_logic.h:27 | kOpGameLoadingStartReq | 2437 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1167 | src/mod/session2/travel_probe_logic.h:28 | kOpMoveFieldAck | 2174 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1168 | src/mod/session2/travel_probe_logic.h:29 | kOpLoadingFieldDataAck | 2643 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1169 | src/mod/session2/travel_probe_logic.h:30 | kOpMoveFieldCompletePostAck | 3066 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1170 | src/mod/session2/travel_probe_logic.h:31 | kOpMoveFieldCompleteAck | 2144 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1171 | src/mod/session2/travel_probe_logic.h:32 | kOpPlayAllPreLoadSequencerAck | 2322 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1172 | src/mod/session2/travel_probe_logic.h:33 | kOpLoadingCompleteAck | 2762 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1173 | src/mod/session2/travel_probe_logic.h:34 | kOpLogoutCharacterAck | 2679 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1174 | src/mod/session2/travel_probe_logic.h:35 | kOpNak | 1013 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1175 | src/mod/session2/travel_probe_logic.h:38 | kOpReviveLoadingReq | 2362 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1176 | src/mod/session2/travel_probe_logic.h:40 | kOwnerAt | 9 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1177 | src/mod/session2/travel_probe_logic.h:41 | kMoveFieldHead | 17 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1178 | src/mod/session2/travel_probe_logic.h:42 | kLoadingFieldHead | 20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1179 | src/mod/session2/travel_probe_logic.h:43 | kRecord | 0x30 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1180 | src/mod/session2/travel_probe_logic.h:157 | kErrInvalidFieldInfoKey | 0x8E19630A | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1181 | src/mod/session2/travel_probe_logic.h:287 | kMaxAcks | 24 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1182 | src/mod/session2/travel_probe_logic.h:288 | kWindowMs | 20000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1183 | src/mod/session2/travel_probe_logic.h:289 | kAlwaysLines | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1184 | src/mod/session2/travel_probe_logic.h:322 | kOpPreLoadSequencerReq | 2873 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1185 | src/mod/session2/travel_probe_logic.h:323 | kOpPrepareSequencerAck | 2661 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1186 | src/mod/session2/travel_probe_logic.h:324 | kOpStageBranchStateAck | 2981 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1187 | src/mod/session2/travel_probe_logic.h:325 | kHostBurstUs | 3'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1188 | src/mod/session2/travel_probe_logic.h:326 | kHostOwnBeforeUs | 1'000'000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1189 | src/mod/session2/tree_bend_logic.h:45 | kVf2 | 0x117F7E0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1190 | src/mod/session2/tree_bend_logic.h:48 | kVf2Patch | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1191 | src/mod/session2/tree_bend_logic.h:49 | kVf2Slot | 0x56FFB58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1192 | src/mod/session2/tree_bend_logic.h:51 | kApply | 0x1190FD0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1193 | src/mod/session2/tree_bend_logic.h:53 | kApplyPatch | 7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1194 | src/mod/session2/tree_bend_logic.h:54 | kApplyCall | 0x117F911 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1195 | src/mod/session2/tree_bend_logic.h:57 | kSetBend | 0x349EA10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1196 | src/mod/session2/tree_bend_logic.h:59 | kSetBendPatch | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1197 | src/mod/session2/tree_bend_logic.h:60 | kSetBendCall | 0x1191532 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1198 | src/mod/session2/tree_bend_logic.h:65 | kTick | 0x59C0D0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1199 | src/mod/session2/tree_bend_logic.h:67 | kTickBoundaries | {2, 3, 4, 5, 7, 9} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1200 | src/mod/session2/tree_bend_logic.h:68 | kTickPatch | {5} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1201 | src/mod/session2/tree_bend_logic.h:69 | kTickJump | 0x8A5DA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1202 | src/mod/session2/tree_bend_logic.h:74 | kGetTarget | 0x5B1600 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1203 | src/mod/session2/tree_bend_logic.h:76 | kFindTree | 0x491AC0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1204 | src/mod/session2/tree_bend_logic.h:81 | kActor_Key | 0x60 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1205 | src/mod/session2/tree_bend_logic.h:82 | kActor_Control | 0xC8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1206 | src/mod/session2/tree_bend_logic.h:83 | kActor_Comps | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1207 | src/mod/session2/tree_bend_logic.h:84 | kActor_LockObj | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1208 | src/mod/session2/tree_bend_logic.h:85 | kComps_RemoteCatch | 0x78 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1209 | src/mod/session2/tree_bend_logic.h:86 | kComps_Control | 0x40 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1210 | src/mod/session2/tree_bend_logic.h:87 | kControl_F6a | 0x88 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1211 | src/mod/session2/tree_bend_logic.h:88 | kControl_F6b | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1212 | src/mod/session2/tree_bend_logic.h:89 | kControl_F6 | 0xF6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1213 | src/mod/session2/tree_bend_logic.h:90 | kComp_Owner | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1214 | src/mod/session2/tree_bend_logic.h:91 | kClientRemoteCatchVtbl | 0x55890D8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: V |
| N1215 | src/mod/session2/tree_bend_logic.h:92 | kLockVf | {0x18, 0x20, 0x30, 0x40} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1216 | src/mod/session2/tree_bend_logic.h:93 | kSlotBase | 0x20 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1217 | src/mod/session2/tree_bend_logic.h:94 | kSlotStride | 0x58 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1218 | src/mod/session2/tree_bend_logic.h:95 | kSlot_Kind | 0x00 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1219 | src/mod/session2/tree_bend_logic.h:96 | kSlot_Scene | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1220 | src/mod/session2/tree_bend_logic.h:97 | kSlotEmpty | 6 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1221 | src/mod/session2/tree_bend_logic.h:98 | kStateBase | 0x9BC | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1222 | src/mod/session2/tree_bend_logic.h:99 | kStateStride | 0x1A0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1223 | src/mod/session2/tree_bend_logic.h:100 | kStateAttached | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1224 | src/mod/session2/tree_bend_logic.h:101 | kSlots | 2 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1225 | src/mod/session2/tree_bend_logic.h:105 | kState_Amount | 0x378 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1226 | src/mod/session2/tree_bend_logic.h:106 | kState_Dir | 0x380 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1227 | src/mod/session2/tree_bend_logic.h:108 | kTree_Amount | 0x2A4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1228 | src/mod/session2/tree_bend_logic.h:109 | kTree_Set | 0x2B4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1229 | src/mod/session2/tree_bend_logic.h:110 | kTree_Flags | 0x59 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1230 | src/mod/session2/tree_bend_logic.h:111 | kMainThreadId | 0x6A652D4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1231 | src/mod/session2/tree_bend_logic.h:122 | kControlOpcode | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1232 | src/mod/session2/tree_bend_logic.h:123 | kControlTreeBend | 15 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1233 | src/mod/session2/tree_bend_logic.h:124 | kVersion | 1 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1234 | src/mod/session2/tree_bend_logic.h:125 | kHeader | 5 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1235 | src/mod/session2/tree_bend_logic.h:126 | kPayloadSize | 52 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1236 | src/mod/session2/tree_bend_logic.h:127 | kFrameSize | kHeader + kPayloadSize | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1237 | src/mod/session2/tree_bend_logic.h:130 | kFlagApplyFlag | 0x01 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1238 | src/mod/session2/tree_bend_logic.h:230 | kMinGapMs | 25 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1239 | src/mod/session2/tree_bend_logic.h:231 | kPeriodMs | 50 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1240 | src/mod/session2/tree_bend_logic.h:232 | kJump | 0.15f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1241 | src/mod/session2/tree_bend_logic.h:233 | kStill | 0.01f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1242 | src/mod/session2/tree_bend_logic.h:234 | kKeepAliveMs | 250 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1243 | src/mod/session2/tree_bend_logic.h:235 | kEndIdleMs | 150 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1244 | src/mod/session2/tree_bend_logic.h:252 | kChannels | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1245 | src/mod/session2/tree_bend_logic.h:409 | kMax | 8 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1246 | src/mod/session2/tree_bend_logic.h:440 | kMaxAgeMs | 500 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1247 | src/mod/session2/tree_bend_logic.h:441 | kRestartMs | 2000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1248 | src/mod/session2/tree_bend_logic.h:442 | kDelayMs | 100 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1249 | src/mod/session2/tree_bend_logic.h:443 | kMaxSpanMs | 1000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1250 | src/mod/session2/tree_bend_logic.h:469 | kEntries | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1251 | src/mod/session2/tree_bend_logic.h:672 | kRunGapUs | 200000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1252 | src/mod/session2/tree_bend_logic.h:821 | kOwnBendUs | kEndIdleMs * 1000ull | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1253 | src/mod/session2/tree_bend_logic.h:848 | kReadTries | 4 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1254 | src/mod/session2/tree_bend_logic.h:887 | kDuringApplyUs | 200000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1255 | src/mod/session2/tree_bend_logic.h:892 | kApplyLineUs | 1000000 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1256 | src/mod/session2/ui_pause_logic.h:107 | kPauseBelow | 0.05f | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1257 | src/mod/session2/ui_pause_logic.h:186 | kWorldList | 0x38 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1258 | src/mod/session2/ui_pause_logic.h:187 | kListCount | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1259 | src/mod/session2/ui_pause_logic.h:188 | kEntryHolder | 0xA0 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1260 | src/mod/session2/ui_pause_logic.h:189 | kHolderView | 0x10 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1261 | src/mod/session2/ui_pause_logic.h:190 | kViewState | 0x26F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1262 | src/mod/session2/ui_pause_logic.h:191 | kViewRoot | 0x118 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1263 | src/mod/session2/ui_pause_logic.h:192 | kViewScale | 0x23C | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1264 | src/mod/session2/ui_pause_logic.h:193 | kRootScaleSlot | 0x168 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1265 | src/mod/session2/ui_pause_logic.h:194 | kMaxEntries | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1266 | src/mod/session2/ui_pause_logic.h:197 | kMaxHits | 3 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1267 | src/mod/session2/user2_funnels_logic.h:93 | kRespawnVf40Ret | 0x2B03AA7 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1268 | src/mod/session2/user2_funnels_logic.h:93 | kTraceVf40Ret | 0x2C4A0AD | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1269 | src/mod/session2/user2_funnels_logic.h:93 | kTriggerVf40Ret | 0x2C4A36F | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1270 | src/mod/session2/user2_funnels_logic.h:94 | kQuestEventVf40Ret | 0x290DC0B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1271 | src/mod/session2/user2_funnels_logic.h:94 | kMissionUiVf40Ret | 0x2A49183 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1272 | src/mod/session2/user2_funnels_logic.h:94 | kMissionAlarmVf40Ret | 0x2A4A03B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1273 | src/mod/session2/user2_funnels_logic.h:94 | kChallengeDiscoverVf40Ret | 0x104C4B3D | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1274 | src/mod/session2/user2_funnels_logic.h:94 | kChallengeTimerVf40Ret | 0x2C3409D | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1275 | src/mod/session2/user2_funnels_logic.h:94 | kChangeCharacterVf40Ret | 0x2A4A32B | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1276 | src/mod/session2/user2_funnels_logic.h:97 | kOpTraceMission | 2372 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1277 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionTrigger | 2112 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1278 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionUiState | 2477 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1279 | src/mod/session2/user2_funnels_logic.h:97 | kOpMissionNewAlarm | 2354 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1280 | src/mod/session2/user2_funnels_logic.h:97 | kOpChangePlayable | 2369 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1281 | src/mod/session2/user2_funnels_logic.h:100 | kOpQuestDialogEnd | 2342 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1282 | src/mod/session2/user2_funnels_logic.h:100 | kOpSetQuestDialog | 2857 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1283 | src/mod/session2/user2_funnels_logic.h:100 | kOpStageBranchState | 2436 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1284 | src/mod/session2/user2_funnels_logic.h:100 | kOpUpdateStageChartEvent | 2591 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
| N1285 | src/mod/session2/user2_funnels_logic.h:254 | kQuestCompActor | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1286 | src/mod/session2/user2_funnels_logic.h:254 | kEventType | 0x08 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1287 | src/mod/session2/user2_funnels_logic.h:254 | kEventActorKey | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1288 | src/mod/session2/user2_funnels_logic.h:263 | kAckHostQuest | {3009, 3088, 2738, 2346} | unresolved | no approved second check | unreviewed/weak | blocked for adoption: S |
| N1289 | src/mod/session2/user2_funnels_logic.h:482 | kActorComponents | 0x68 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1290 | src/mod/session2/user2_funnels_logic.h:483 | kComponentsQuest | 0x138 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1291 | src/mod/session2/user2_funnels_logic.h:484 | kQuestComponentManager | 0x18 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: A |
| N1292 | src/mod/session2/user2_funnels_logic.h:521 | kOpcodes | 4096 | unresolved | no approved second check | unreviewed/weak | blocked for adoption: T |
