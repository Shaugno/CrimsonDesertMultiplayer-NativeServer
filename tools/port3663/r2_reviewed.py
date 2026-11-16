"""Materialize manually reviewed native layout proofs. RVAs await standard matching."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[2]
docs = root/'docs/probe-3663'
core = json.loads((docs/'R2-CORE-LIST.json').read_text())
result = {}

def prove(tier, source, values, evidence, check):
    for symbol,target in values.items():
        row, = [r for r in core if r['tier']==tier and r['symbol']==symbol and r['source'].startswith(source)]
        result[row['r2']] = dict(target=target,method='paired native code consumption',
            second_check=check,evidence=evidence,old=row['value'],symbol=symbol)

prove('C','src/mod/session2/field_share.h',dict(
    kRefs='0x10',kLock1='0x18',kLock2Ptr='0x08',kOpen='0x4A',kOwner='0x180',
    kUsers='0x2A8',kUserCount='0x2B0',kUserActors='0x2B8',kLevelEntry='0x2D8'),
    'port-evidence/r2/field-addactor-reference.txt; field-addactor-target.txt; field-acquire-enter-target.txt',
    'Native AddActor: lock field+18 then [field+8]; native admission counts and rollback under both locks; AcquireField opens field and assigns traveller+60 as owner. Target AddActor rollback XOR constant 0x1F38148 ^ 0x1F38149 = 1, independently read from image RVA 0x12569640.')
prove('C','src/mod/session2/field_share.h',dict(kTsFieldMover='0x508'),
    'port-evidence/r2/field-mover-reference.txt; field-mover-target.txt; field-acquire-enter-target.txt',
    'Both outer mover and EnterField traverse server actor+68, transform-sync slot+1A0; reference adds 4F0, target adds 508 before the same field-mover role.')
prove('C','src/mod/session2/field_share.h',dict(kFmFieldKey='0x08',kFmField='0x10'),
    'port-evidence/r2/field-leave-reference.txt; field-getter-target.txt',
    'Reference 2BBEAB0 and target 2EC93B0 release current field and reset field pointer+10 and key+8 to 0x30000. This proves offsets, not an unchanged entire FieldMover layout.')
prove('C','src/mod/session2/field_share.h',dict(kKey='0x2D0',kFmBusy='0x4A'),
    'port-evidence/r2/field-ready-reference.txt; field-assign-target.txt',
    'Paired native field assignment 2BBE900/2EC9200 rejects a nonzero u16 busy counter (5A -> 4A) or an existing field at FM+10. On success it copies field key (2C8 -> 2D0) to FM+8 and stores field at FM+10, under the actor lock. Denuvo thunk bodies are included; no obfuscated scalar is needed for these direct accesses.')
prove('C','src/mod/session2/field_share.h',dict(kOutField='0x08',kOutValid='0x10',kOutHolder='0x18',kOutNew='0x20',kOutInfoKey='0x24',kMgrBucketCount='0x48',kMgrEntries='0x4C',kMgrBuckets='0x58',kMgrHolders='0x60'),
    'port-evidence/r2/field-contracts-reference.txt; field-acquire-enter-target.txt',
    'AcquireField: lookup destination key out+24 in manager map 48/4C/58/60; acquire reference, replace out+8/+10, clear holder+18 and set new+20; caller rollback and native release consume those fields.')
prove('C','src/mod/session2/field_share.cpp',dict(kUserKey='0x60',kActorUser='0xA0',FieldLockAcquireVslot='8',FieldLockReleaseVslot='0x10',HostCharacterComponents='0x68',HostTransformSyncComponent='0x1A0'),
    'port-evidence/r2/field-addactor-target.txt; field-mover-target.txt; field-acquire-enter-target.txt',
    'Native travelling-user owner assignment reads user+60; AddActor obtains character owner+ A0 and takes/releases both lock vslots 8/10; mover follows actor+68, transform-sync+1A0.')
prove('C','src/mod/session2/field_share.h',dict(FieldUserEntryLayout='16 B; user +0, actors +8'),
    'port-evidence/r2/field-addactor-reference.txt; field-addactor-target.txt',
    'Native user-vector search advances two qwords, compares first with owner pointer, increments/decrements u32 at second qword and swap-removes exactly 16 bytes.')
prove('A','src/mod/game/offsets.h',dict(kActor_ComponentTable='0x68',kComponents_CharacterControl='0x40',kControl_Inner='0x140',kControlInner_Physics='0x2D0',kPhysics_Position='0x90'),
    'port-evidence/r2/player-inner-reference.txt; player-inner-target.txt; player-access-reference.txt; player-access-target.txt',
    'Paired native control consumer 859A00/96C900 traverses actor+68 -> control+40 -> inner+140 -> physics (2C0 -> 2D0); paired native inner consumer 36A36A0/39F6B20 adds chunk origin to physics+90.')
prove('A','src/mod/game/offsets.h',dict(kWorldSystem_ActorManager='0x30',kActorManager_ControlledActor='0x50'),
    'port-evidence/r2/player-controlled-reference.txt; player-controlled-target.txt; player-manager-getter-target.txt',
    'Paired native consumers DD49D0/FB6F70 pass [WorldSystem+30] to actor getters 8B44E0/9CDC00. Each getter reads manager+50, obtains its vslot C0 validity and returns the actor in an owner/validity wrapper. The consumer accesses that actor component table+68. This proves the two field roles, not adoption of the candidate root RVA.')
prove('A','src/mod/game/offsets.h',dict(kPhysics_AxisX='0x60',kPhysics_AxisY='0x70',kPhysics_AxisZ='0x80'),
    'port-evidence/r2/player-inner-target.txt; player-stat-matrix-target.txt',
    'Physics base constructor 4601320 creates rotation at this+60 with quaternion-to-matrix 44AF690 (rows +0/+10/+20); inverse 44AD6C0 consumes that three-row matrix, with position separately at +90.')
prove('A','src/mod/game/offsets.h',dict(kComponents_Status='0x20',kStatus_StatContainer='0x18',kStatContainer_Entries='0x58',kStatContainer_Count='0x60',kStatEntry_Size='0x90',kStatEntry_Id='0x00',kStatEntry_Current='0x08',kStatEntry_Max='0x18'),
    'port-evidence/r2/player-health-handler-target.txt; player-health-update-target.txt; player-status-target.txt; player-stat-stride-target.txt; player-stat-lookup-target.txt',
    'Named TrocTrVaryHPAck invokes status [[actor+68]+20]; native HP updater uses status+18; stat initialization/lookup consumes vector+58/count+60, u16 key at entry+0, current+8, maximum+18 and 90-byte stride, clamping current to maximum.')
prove('A','src/mod/game/player_layout.h',dict(NativeStatUnits='1000.0f',StatIdReadWidth='2'),
    'port-evidence/r2/player-health-update-target.txt; player-stat-stride-target.txt; player-stat-lookup-target.txt',
    'Native HP updater converts integer units to displayed HP by /1000; initializer converts values with *1000 and writes/compares u16 keys, never the six adjacent padding bytes. Legacy reader width 8 is preserved for 2976 only.')
prove('A','docs/probe-3663/port-evidence/r2/player-health-update-reference.txt',dict(kHpDefinitionsRoot='0x00',kHpDefinitionsTables='0x08',kHpDefinitionsObject='0x10',kHpDefinitionId='0x78'),
    'port-evidence/r2/player-health-update-reference.txt; player-health-update-target.txt',
    'Named HP-handler updaters 62FA00/71C4E0 dereference registry root+0 -> +8 -> +10 and read a u16 HP key. Its field changes A0 -> 78; the key value itself is runtime data.')
prove('B','src/mod/session2/offsets_s2.h',dict(kServerNet_Session='0x18',kSession_Name='0x18',kSession_B0='0xB0',kSession_B8='0xB8',kSession_C0='0xC0'),
    'port-evidence/r2/tunnel-layout-target.txt; ../ghidra-2976-expanded.txt',
    'Native ServerModule initialization E6F8C70 constructs the named NwVirtualAsyncSession, clears qword B0/B8 and word C0, writes UTF16 host name 1 at +18, and stores the session in ServerNet+18. Native PostPacket 3A9960 separately reads B8/B0 as the packet epoch pair. These are member proofs; the initializer RVA is not adopted by this entry.')
prove('B','src/mod/session2/offsets_s2.h',dict(kSession_IdPtr='0x88',kSession_User='0x90',kSession_Crypto='0x98'),
    'port-evidence/r2/tunnel-session-target.txt; tunnel-layout-target.txt; ../ghidra-2976-expanded.txt',
    'Paired session constructors allocate a u32 key and store its pointer at +88, clear +90 and construct crypto at +98. Native server receive 2A0EA70 consumes [session+88] as the deferred key, [session+90] as the user actor, and [session+98] through enabled/decrypt vslots 10/30; client receive 3A9CF0 independently uses the key and crypto fields.')
prove('B','src/mod/session2/offsets_s2.h',dict(kPbSize='0x8030',kPb_FramePtr='0x8000',kPb_Len1='0x8008',kPb_Len2='0x800A',kPb_Cap='0x800C',kPb_IsRequest='0x800E',kPb_Base='0x8028',kFrameOverhead='0x1F',kFrameHeader='3'),
    'port-evidence/r2/tunnel-layout-target.txt; ../ghidra-2976-expanded.txt',
    'Paired native PostPacket 35C670/3A9960 allocates 8030, initializes packet flag 800E to zero, stores frame pointer 8000 and allocation base 8028, sets capacity 800C, copies packet at frame+3, adds 1F to both u16 lengths 8008/800A and writes u16 frame length plus u8 channel. This does not adopt the tag/magic unused-space claim or the mod maximum packet limit.')
prove('B','src/mod/session2/guest.cpp',dict(kNoEpoch='0x7FFFFFFFFFFFFFFF'),
    'port-evidence/r2/tunnel-layout-target.txt; ../ghidra-2976-expanded.txt',
    'Native PostPacket explicitly substitutes 7FFFFFFFFFFFFFFF for both epoch fields when no per-channel session exists, matching the legacy guest epoch sentinel.')
prove('B','src/common/troc_frame.h',dict(kHeaderSize='5'),
    'port-evidence/r2/field-login-target.txt; ../ghidra-3663-core-handlers.txt',
    'Named target LoginCharacter decoder reads u16 payload length at packet+3, requires packet_size-5 equal to it and initializes its decode cursor at packet+5. Native dispatcher reads opcode u16 at packet+0 and handles the slot at +2; internal system messages have separate framing and are not covered by this row.')
prove('C','src/mod/session2/server.cpp',dict(kOpNak='1013'),
    'port-evidence/r2/request-handlers-target.txt',
    'Named target LogoutSessionToServerReq handler 2F55680, body ED59430, constructs a u16 opcode 3F5 and passes 3F5 to the native failure-message writer with its 11-hex-byte packet. This confirms the Nak identifier only; it does not adopt an unreviewed payload schema.')
prove('D','src/mod/session2/offsets_s2.h',dict(kServerActorLookup_SaveManager='0x48',kSaveMgr_Lock='0x00',kSaveMgr_Slots='0x40',kSaveMgr_Count='0x48',kSaveMgr_Storage='0x50',kSaveMgr_Busy='0x60',kSaveMgr_EntrySize='0x78',kSaveStorage_FolderCount='0x40'),
    'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt; ../ghidra-3663-core-handlers.txt; ../ghidra-3663-refresh.txt',
    'Named target refresh handler 2D133B0 reads lookup+48, locks [manager+0], tests/sets manager+60 and calls retained refresh 1A18320. Native LoadSlot uses the same lock/busy/storage fields. Refresh rebuilds vector manager+40 from storage+38/count+40; clear BB1A850 walks count at vector+8 with IMUL 78, and sort BB1DBE0 independently copies/swaps 78-byte entries. ENTRY STRIDE CHANGED 68 -> 78; capacity and lock-object self-offset are not inferred.')
prove('D','src/mod/session2/guest_save_logic.h',dict(NativeSlotNumberDomain='0..3;100..108;200..207'),
    'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt; ../ghidra-3663-refresh.txt',
    'Paired native LoadSlot 186F570/1A16C40 explicitly accepts u32 slot <3, ==3, slot-100<9 or slot-200<8. Native target refresh independently tests that same domain before its special disk exclusions. This does not establish which slots are memory-only.')
domain_row,=[r['r2'] for r in core if r['symbol']=='NativeSlotNumberDomain']
result[domain_row]['members']=dict(kNativeSlotBaseMax=3,kNativeSlotManualBegin=100,
    kNativeSlotManualCount=9,kNativeSlotSpecialBegin=200,kNativeSlotSpecialCount=8)
prove('D','src/mod/session2/guest_save_host.cpp',dict(kActorDescriptor='0x88',kActorUser='0xA0',kActorKey='0x60'),
    'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt; request-handlers-target.txt',
    'Paired native load reads actor+88 descriptor byte+1. Target save/auto-save wrappers traverse actor+A0 to its owning user, compare [user+D0] with that actor and publish actor key+60 in the native request context, matching the legacy save/identity field roles.')
prove('C','src/mod/session2/field_share.cpp',dict(kUserMainCharacter='0xD0'),
    'port-evidence/r2/save-manager-target.txt; request-handlers-target.txt',
    'Native SaveToFile and SaveAuto wrappers require [actor owning user at +A0]+D0 equal the requesting actor; this is the same user main-character ownership gate used by sharing.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kUserMainCharacter='0xD0',kActorKey='0x60'),
    'port-evidence/r2/save-manager-target.txt; request-handlers-target.txt',
    'Native save wrappers verify owning user+D0 is this server character and publish its key at actor+60. These prove the fields only, not disconnect/removal barrier behavior.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kSamMapBuckets='0x98',kSamMapSize='0x9C',kSamMapBucketBase='0xA8',kSamMapHolders='0xB0'),
    'port-evidence/r2/actor-map-reference.txt; field-roster-ownership-target.txt',
    'Both reference lookup candidates 2A82370/2A827B0 and target native actor lookup 2D78E40 check map size+9C and bucket count+98, select key-low20 modulo bucket count, use 100-byte buckets from +A8 and holder array +B0, validate holder key+4 and obtain actor pointer+8 under the manager lock. This proves map fields, not an RVA match or the user-list/removal lifecycle.')
prove('C','src/mod/session2/offsets_s2.h',dict(kSam_ActorCount='0x9C'),
    'port-evidence/r2/actor-map-reference.txt; field-roster-ownership-target.txt',
    'The native server actor lookup checks manager map size at +9C before resolving an actor by key. Reference and target use the same counted map role; the root RVA is still separate.')
prove('C','src/mod/session2/identity_fix.h',dict(
    kNative='0x7FFFFFFFFFFFFFFF',kLobbyOwner='0x08',kLobbyList='0x18',
    kListCount='0x08',kEntryStride='0xD0',kEntryNumber='0x00',
    kUserComponents='0x68',kTableLobby='0x10',kTableStatus='0x20',
    kStatusActor='0x08',kStatusNumber='0x90',kLockObject='0x08'),
    'port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt; identity-entry-reference.txt; identity-entry-target.txt',
    'Paired native lobby loads 2B7F4D0/2E879A0 lock [component+8]+8 and pass INT64_MAX as the character number to entry writers 2143E70/237EA60. Both writers search entries by first qword, use component+18 list/count+8 and D0 stride; target writes the passed number into the first entry qword. Named LoginCharacter implementations follow actor+68, lobby+10 or status+20. Status loads 293FE70/2C160B0 reset the character number through a pointer CHANGED +70 -> +90, and the target login lookup independently compares through +90. Status owner is component+8. No hook ABI or native address is adopted by this field proof.')
prove('C','src/mod/session2/identity_probe.h',dict(kActorComponents='0x68',
    kTableStatus='0x20',kStatusCharacterNo='0x90',kComponentOwner='0x08',
    kActorUser='0xA0',kKey='0x60'),
    'port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt; field-addactor-target.txt; save-manager-target.txt',
    'Native LoginCharacter status lookup consumes actor+68 -> status+20 -> character-number pointer +90; paired status load writes the same pointer (old +70). Native status load invokes its actor at component+8. Independently verified target field admission and save wrappers obtain actor owner +A0 and user/actor key +60. This adopts member roles only, not the surrounding hooks.')
prove('D','src/mod/session2/guest_save_host.cpp',dict(kMgrPendingSlot='0x5C'),
    'port-evidence/r2/save-manager-reference.txt; save-flush-target.txt; save-flush-callers-reference.txt; save-flush-callers-target.txt',
    'Paired native six-argument FlushPendingSaveData 1871930/1A192D0 acquires busy/lock, validates generation, reads pending i32 slot at manager+5C, copies it to the out argument and resets +5C to FFFFFFFF under lock before formatting/committing that slot. This proves the field, not the callable RVA.')
prove('D','src/mod/session2/offsets_s2.h',dict(kSaveMgr_Capacity='0x4C'),
    'port-evidence/r2/title-entry-reference.txt; title-entry-target.txt',
    'Paired native refresh entry-populators 1871D60/1A19720 and fallback constructors 1872620/1A1A080 compare manager count+48 with capacity+4C, reserve vector+40 when full, append an entry and increment count. Target stride is 78, independently corroborating the native vector field roles.')
prove('D','src/mod/session2/slot_refresh.h',dict(kEntrySize='0x78'),
    'port-evidence/r2/title-entry-reference.txt; title-entry-target.txt; save-manager-reference.txt; save-manager-target.txt',
    'The title parser has a separate legacy 68-byte stride from offsets_s2. Both target native refresh population and fallback creation multiply index by 78, agreeing with native clear/sort. The 2976 parser remains unchanged; target routing must supply 78 explicitly.')
prove('D','src/mod/session2/slot_refresh.cpp',dict(TitleSlotNumberOffset='0',TitleSlotTimeOffset='8'),
    'port-evidence/r2/title-entry-reference.txt; title-entry-target.txt; title-entry-copy-reference.txt; title-entry-copy-target.txt',
    'Paired native refresh entry-populators set the first u32 to the parsed slot number and copy the same lobby metadata qword at +68 to entry+8; fallback entries use the requested slot and a zero qword. Native copy constructors 18735F0/1A1B1A0 independently copy u32+0 and qword+8 into vector entries. This proves raw field roles/widths, not new timestamp units or title eligibility.')
prove('B','src/mod/session2/guest.cpp',dict(NativeCryptoEnabledOffset='0x24'),
    'port-evidence/r2/tunnel-crypto-rtti-reference.json; tunnel-crypto-rtti-target.json; tunnel-crypto-reference.txt; tunnel-crypto-target.txt; ../ghidra-3663-core-handlers.txt',
    'The exact CryptogramAesGcm RTTI class has enabled predicate vslot+10 at 12B31D0/14D0E80. Both native bodies compare byte this+24 with one and return that result. Native target receive dispatch calls crypto vslot+10 before decrypting. This proves the AES field only; target extraction must reject unknown crypto object types before using it. Legacy nonzero rejection is conservative relative to the native equality test.')
prove('B','src/mod/session2/guest.cpp',dict(NativePacketEpochOffset='0x8020'),
    'port-evidence/ghidra-2976-expanded.txt; port-evidence/r2/tunnel-layout-target.txt; tunnel-connect-reference.txt; tunnel-connect-target.txt',
    'Paired native PostPacket 35C670/3A9960 copy session+B0 into packet-buffer+8020, or INT64_MAX with no session; +8018 separately receives session+B8. Named OnConnect independently writes a fresh epoch at session+B0. This proves the qword read by guest_send, not safe reuse of the separate tag/magic fields.')
save_constants=json.loads((docs/'r2-save-constants.json').read_text())
prove('B','src/mod/session2/offsets_s2.h',dict(kSyncSession_Epoch='0xB0',kClientNet_Slot4='0x58'),
    'port-evidence/r2/tunnel-connect-reference.txt; tunnel-connect-target.txt; tunnel-control-native-target.txt',
    'Named ConnectToServerAck handlers CB2170/E21EE0 call native OnConnect 8CAFF0/9E45B0 with the current session. On successful result zero, both store a fresh native epoch at session+B0 and the sentinel at +B8. Both obtain client net through its manager+18 and test the server-session slot at net+58 via its virtual alive predicate. This proves data fields, not the OnConnect RVA or game-world loading readiness.')
assert save_constants['target_sha256']=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
prove('D','src/mod/session2/guest_save_host.cpp',dict(kSlotMemoryOnly='0x699',
    kSlotInMemory='0x69C',kFileHeader='0x98'),
    'port-evidence/r2/save-storage-reference.txt; save-slot-construct-reference.txt; save-create-target.txt; save-slot-construct-target.txt; save-addfile-target.txt; save-commit-target.txt; save-storage-target.txt',
    'Native GetOrCreate writes +699 for special numeric slot names; CreateSlot stores its in-memory argument at +69C. Target CommitSlot independently skips disk writes if either byte is nonzero. Paired AddFile constructs the 80-byte native header at file+98; native target serialization copies those 80 bytes before payload. AddFile has its own same memory-only/in-memory gate.')
prove('D','src/mod/session2/guest_save_logic.h',dict(kHeaderSize='0x80',kMagic='0x45564153'),
    'port-evidence/r2/save-storage-reference.txt; save-addfile-target.txt; save-storage-target.txt; r2-save-constants.json',
    'Paired AddFile initializes header magic from independently checked file-backed operands 5547B6C/59728FC, both SAVE. Target writes version/header-size qwords including 800002 at +9C; native serialization copies 80 bytes from +98 and appends payload at +80. This proves size/magic only; the complete validator schema remains a separate blocked row.')
prove('D','src/mod/session2/guest_save_logic.h',dict(kSlotNameMap='0x08',
    kMapBucketCount='0x00',kMapBuckets='0x10',kMapEntries='0x18',
    kBucketSize='0x100',kBucketPairs='0x08',kMaxBucketPairs='31',
    kEntryKey='0x08',kEntryValue='0x10',kKeyChars='0x00',kKeyLength='0x08',
    kSlotNames='0x688',kSlotNameCount='0x690'),
    'port-evidence/r2/save-storage-reference.txt; save-addfile-target.txt; save-name-map-reference.txt; save-name-map-target.txt; save-slot-construct-target.txt',
    'Paired AddFile 235B100/25C0900 inserts at slot+8 and appends its named entry pointer to vector+688/count+690. Native map lookup 612620/40A5E0 uses bucket-count+0, bucket-base+10, entry-pointers+18; hash/index pairs start +8 in 100-byte buckets. Native insertion 235D2E0/25C29A0 explicitly splits at 1F pairs, constructs key pointer+8 and value+10. Lookup consumes key char pointer+0 and cached length+8, recomputing an FFFFFFFF length. This is member evidence, not an AddFile RVA adoption.')
prove('D','src/mod/session2/guest_save_logic.h',dict(NativeMemoryOnlySlots='201;205..207'),
    'port-evidence/r2/save-storage-reference.txt; save-create-target.txt; save-commit-target.txt; r2-save-constants.json',
    'Paired GetOrCreate checks literal slot prefix, numeric suffix, exact 201 or unsigned slot-205<=2, then sets byte+699; target CommitSlot bypasses disk persistence for +699 or +69C. The target obfuscated subtraction operand is read from file: F9D00CD - [B7A5B48] = CD. Reserved mod slot204 remains policy.')
memory_row,=[r['r2'] for r in core if r['symbol']=='NativeMemoryOnlySlots']
result[memory_row]['members']=dict(kNativeMemoryOnlyExact=201,kNativeMemoryOnlyBegin=205,kNativeMemoryOnlyCount=3)
header_row,=[r for r in core if r['symbol']=='NativeSaveHeaderLayout']
prove('D','src/mod/session2/guest_save_logic.h',dict(NativeSaveHeaderLayout=header_row['value']),
    'port-evidence/r2/save-file-header-reference.txt; save-file-header-target.txt; r2-save-constants.json',
    'Paired native file loaders 2357DA0/25BD480 compare SAVE magic, require version 1..2, header size 80, flags masked by FFFFFFFD equal zero, nonzero raw/data sizes and data<=raw, using exact header offsets 0/4/6/C/12/16. Target instructions 25BD65A..25BD6AE establish every check, then copy 80 bytes into file+98 and consume data length at file+AE. Target functions found through actual RIP-relative comparisons to the independently verified magic operand, then paired code consumption; no function RVA is adopted by this schema proof. Local size/ratio caps remain policy.')
result[header_row['r2']]['members']=dict(kSaveHeaderMagicOffset=0,kSaveHeaderVersionOffset=4,
    kSaveHeaderSizeOffset=6,kSaveHeaderFlagsOffset=0xC,kSaveHeaderRawSizeOffset=0x12,
    kSaveHeaderDataSizeOffset=0x16,kSaveHeaderVersionMin=1,kSaveHeaderVersionMax=2,
    kSaveHeaderAllowedFlags=2)
prove('B','src/mod/session2/offsets_s2.h',dict(kTroc_Kind='0x08',kTroc_Opcode='0x0C',
    kTroc_Channel='0x14',kTroc_Flags='0x16',kTrocKindServer='4'),
    'port-evidence/r2/troc-role-reference.txt; troc-role-target.txt; troc-dispatch-lookup-target.txt; troc-dispatch-insert-target.txt; troc-register-target.txt; troc-mode-operands.json; request-candidates.json',
    'Paired native receive 2773C20/2A0EA70 compares the incoming channel to descriptor byte+14 and branches on u16 flags+16. Paired TrocFind 143B2C0/165FAC0 tests descriptor kind dword+8 against its supplied mask. Target registration CB4BE0 loads descriptor u16+0C as the registry key and stores the same descriptor pointer. Named LoginUser/Character request descriptors independently have kind 4 at +8 in both exact files; kind 4 is not confused with the framework-mode global initial 11h. These member/value proofs do not adopt descriptor table boundaries or a new function RVA.')
prove('C','src/mod/session2/identity_fix.cpp',dict(kLoginMgrActors='0xB8',kLoginMgrCount='0xC0'),
    'port-evidence/r2/identity-contracts-reference.txt; identity-find-by-number-target.txt',
    'Paired native FindByCharacterNo 2A83F80/2D7A640 acquires the same manager+68 read lock, walks the pointer vector at manager+B8 using u32 count+C0 and an eight-byte stride, then compares actor+68/status+20/number-pointer (70 -> 90) against the requested u64 number. The native selected-actor reference/result holder and lock release are preserved in both bodies. This proves fields, not the lookup function or validation-site RVAs.')
prove('D','src/mod/session2/offsets_s2.h',dict(kWorldSystem_SaveManager='0x88'),
    'port-evidence/r2/title-entry-reference.txt; title-predicate-candidates-target.txt; title-entry-target.txt',
    'Reference title predicate 65EAD0/body9096890 obtains WorldSystem+88 then slot-vector+40/count+48. Target native title eligibility consumer 758EA0 follows the exact target WorldSystem global through +88 to that same independently identified save-manager vector/count and changed stride78. This establishes the hop, not equality of the two title predicates; target has an additional per-entry requirement-list gate which remains unported.')
prove('C','src/mod/session2/session2.cpp',dict(NativeForbiddenRequests='2447,2042,2004; deny1001..1008(policy)'),
    'port-evidence/r2/tunnel-control-native-target.txt; request-initializers-target.txt; request-handlers-target.txt; r2-wire-reviewed.json; src/mod/session2/control_filter.h',
    'Named native ReserveShutDownServerReq maps 2790 -> 2447; ShutDownServerOnceTimer maps 2038 -> 2042 and its actual handler shuts down the native server; pointer-bearing OnLogoutXxxToServerReq remains 2004 and its handler reads process-local pointers. The complete 1001..1008 interval is conservative tunnel deny policy, including unclassified values: no native handler identity is inferred for 1003/1007. Field-time ID has its own proved row. The helper accepts explicit per-build native IDs; the current production caller remains legacy and the target route stays disabled.')
control_row,=[r['r2'] for r in core if r['symbol']=='NativeForbiddenRequests']
result[control_row]['members']=dict(kForbiddenReserveShutdownOp=2447,kForbiddenShutdownTimerOp=2042,
    kForbiddenPointerRequestOp=2004)
prove('D','src/mod/session2/guest_lifecycle.h',dict(kSamBucketBytes='0x100',kSamKeyIndexMask='0xFFFFF'),
    'port-evidence/r2/lifecycle-lookup-reference.txt; field-roster-factory-followup.txt',
    'Paired native FindActor 2A827B0/2D78E40 masks the requested key with FFFFF, divides by bucket count+98, shifts the remainder left eight bits and adds the bucket base+A8. This proves the low-20-bit key mask and 256-byte bucket stride; the maximum admissible bucket entry count and cleanup lifetime are separate obligations.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kSamUserList='0xB8',kSamUserCount='0xC0'),
    'port-evidence/r2/identity-contracts-reference.txt; identity-find-by-number-target.txt',
    'Paired native character-number lookup 2A83F80/2D7A640 walks the same manager actor-pointer vector+B8/count+C0 under the manager+68 read lock. This is the lifecycle probe vector, which contains actors as well as users; it does not prove the removal barrier itself.')
for source,values in [
    ('src/mod/session2/offsets_s2.h',dict(kUser_Key='0x60',kServerActor_Key='0x60')),
    ('src/mod/session2/identity_fix.h',dict(kActorKey='0x60',kActorUser='0xA0')),
    ('src/mod/session2/clock_gate.h',dict(kActorUser='0xA0',kUserKey='0x60')),
    ('src/mod/game/offsets.h',dict(kActor_Key='0x60'))]:
    prove('C',source,values,
        'port-evidence/r2/save-manager-reference.txt; save-manager-target.txt; request-handlers-target.txt; field-addactor-target.txt; field-acquire-enter-target.txt',
        'Native field admission and save wrappers establish server character+A0 as owning user, user+60 as user key and character+60 as actor key. Paired save/load wrappers verify user+D0 identifies that character before publishing its key. These duplicate core uses refer to those same server object types; the different client actor-owner +90 contract is not inferred from them.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kActorComponents='0x68',kComponentsTransformSync='0x1A0'),
    'port-evidence/r2/lifecycle-handlers-old.txt; lifecycle-handlers-new.txt',
    'Paired named loading-start and loading-complete request handlers obtain actor components+68 then transform-sync+1A0 before invoking the native transition. Acknowledgement handlers independently use the same chain. This proves component fields, not moving offsets or a loading-ready predicate.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kUserComponents='0x68',kComponentsSlots='0x18',kSlotsArray='0x18',kSlotsCount='0x20',kSlotStride='0x10'),
    'port-evidence/r2/lifecycle-lookup-reference.txt; lifecycle-user-release-target.txt; ../rtti-2976.json; ../rtti-3663.json',
    'ServerUserActor RTTI vslot 2 identifies paired native release walkers 2A7D280/2D73960. Both hold the manager+68 lock, follow user+68/component+18, bound the byte-index loop by slots+20, read the slot array+18 with a 16-byte stride, clear its pointer/validity byte and release each actor before the user. These fields do not establish concurrent snapshot coherence by themselves.')
prove('C','src/mod/session2/offsets_s2.h',dict(kSam_FreeKeys='0xD0'),
    'port-evidence/r2/lifecycle-actor-release-old.txt; lifecycle-actor-release-new.txt; lifecycle-user-release-target.txt',
    'Native user release invokes paired actor release 2A83650/2D79D00. Both swap-remove the actor from manager+B8/count+C0, clear actor key+60, append a recycled key to the ring at +C8/+CC/+D4/+E0 and increment its u32 free-key count+D0. Native release continues through the map removal; no cleanup hook or return-site RVA is adopted here.')
prove('C','src/mod/session2/clock_gate.h',dict(kTsActor='0x08'),
    'port-evidence/r2/lifecycle-loading-old.txt; lifecycle-loading-new.txt; lifecycle-handlers-old.txt; lifecycle-handlers-new.txt',
    'Named loading request handlers pass actor transform-sync component+1A0; paired loading-start and loading-complete implementations immediately obtain that component owner from this+8. This is the same server transform-sync type consumed by clock_gate, not an inferred layout for a client component.')
prove('D','src/mod/session2/guest_lifecycle.h',dict(kActorState='0x5E'),
    'port-evidence/r2/lifecycle-loading-old.txt; lifecycle-loading-new.txt',
    'Paired MoveFieldComplete implementations 2BC6BB0/2ED29D0 obtain the component actor at this+8, inspect its u16 state+5E, preserve prior-state+5C and perform the native state transition under actor+18 lock. Factory rollback independently reads the same field. This proves the state offset, not which values establish the full reconnect barrier.')
prove('C','src/mod/session2/clock_gate.h',dict(kTsFieldTime='0x450',kFieldTimeSize='32'),
    'port-evidence/r2/clock-gate-old.txt; clock-gate-new.txt; lifecycle-loading-old.txt; lifecycle-loading-new.txt',
    'Paired six-argument native loading-complete consumers 219D7F0/23DC630 test the fifth-byte flag, store the supplied timestamp at component+468, copy a 32-byte YMM value from component+450 and publish it to the mode-selected global clock before unpausing the field. This proves the field and copy width from actual target instructions, not an assumed structure or the SetFieldTimeReq payload schema.')
prove('D','src/mod/session2/offsets_s2.h',dict(kSaveMgr_LockObj='0x08'),
    'port-evidence/r2/save-lock-native-new.txt; save-manager-reference.txt; save-manager-target.txt; save-lock-rtti-old.json; save-lock-rtti-new.json',
    'Target save-manager constructor 1A15340 stores this+8 in its first word, constructs the WindowsLock<3> object there and writes the independently named vtable 59874C8. It initializes the identified slot vector+40/count+48/capacity+4C, pending-slot+5C, busy+60 and native storage+50. The native LoadSlot/refresh lock-pointer use corroborates the same object. This proves the self-relative lock offset, not any new function/vtable RVA by itself.')
prove('B','src/mod/session2/offsets_s2.h',dict(kClientNet_InQueue='0x18',kClientNet_Session='0x10',kClientNet_FreeC2S='0x28'),
    'port-evidence/r2/network-pump-reference.txt; tunnel-layout-target.txt; ../ghidra-2976-expanded.txt; ../ghidra-3663-expanded.txt',
    'Paired complete client network loops 35B800/3A8AC0 use ClientNet in R15, consume incoming queue+18, flush its buffer queue through synchronous session+10 and drain/free returned C-to-S buffers through queue+28. Session send 14380B0/165CB60 independently queues the buffers; target receive 3A9CF0 is invoked by the same native loop. This proves queue/session fields, not the unresolved target metadata reuse or a live tunnel.')
prove('B','src/mod/session2/offsets_s2.h',dict(kNetLoopPumpPatch='7'),
    'port-evidence/r2/network-pump-reference.txt; tunnel-layout-target.txt',
    'The paired pre-flush pump sites 35BAD4/3A8D94 decode MOV RAX,[R15+E0] as exactly seven bytes, with ClientNet in R15 and the same native queue-drain ordering. It is a mid-hook site, never an inline function entry; its RVA and full guard bytes have separate obligations.')
prove('B','src/mod/session2/offsets_s2.h',dict(kServerNet_InQueue='0x20',kServerNet_FreeC2S='0x78'),
    'port-evidence/r2/server-thread-init-old.txt; server-thread-init-new.txt; server-run-target.txt; tunnel-layout-target.txt',
    'Paired named ServerRunThread setup identifies run wrappers 2773390/2A0E1A0 and loops 27733F0/2A0E200. Both pop packet buffers from ServerNet+20, dispatch a packet using its frame pointer/length with the native session+18, then push the same buffer to the C-to-S return queue+78. The client thread drains its return queue+28 and frees base+8028. These explicit consumers establish the field roles.')
prove('B','src/mod/session2/offsets_s2.h',dict(kPb_Tag='0x8010',kPb_Magic='0x8018'),
    'port-evidence/r2/server-thread-init-old.txt; server-run-target.txt; network-pump-reference.txt; tunnel-layout-target.txt; src/mod/session2/server.cpp',
    'Storage reuse is proved only for the mod-owned C-to-S injection path: allocate a complete buffer, set request-kind+800E to zero, frame/base pointers to itself and push directly to ServerNet Q0. Paired server loops read +8010 only in the nonzero request-kind branch; the zero packet branch passes frame bytes to native ServerOnRecvPacket and returns the unchanged buffer through +78. Paired client return-queue drains read +8028, clear frame pointer and free that base without reading +8010/+8018/+8020. PostPacket epoch metadata is not a free-space claim for native or request buffers; this route bypasses native PostPacket/forwarder and never alters a native-created buffer. Existing mod epoch accounting remains mandatory before any target injection.')
prove('B','src/mod/session2/offsets_s2.h',dict(kServerNet_FreeS2C='0x80'),
    'port-evidence/r2/virtual-queue-bind-reference.txt; virtual-queue-bind-target.txt; virtual-queue-callers-target.txt; virtual-queue-producers-old.txt; virtual-queue-producers-new.txt; virtual-queue-block-producer-target.txt',
    'Paired framework producers 9E2C260/9BA4680 form the same four-queue descriptor from one framework queue block (root member 140 -> 148). Target client setup loads that framework+148 block and passes B/B+80/B+100/B+180 through 3A7E30/8968C40 to 3A8040. Server startup 2A5E840 -> 2A600C0 -> 2A0CE20 passes the same descriptor. Native binding writes ServerNet+80 and ClientNet+20 from B+100, and ServerNet+78/ClientNet+28 from B+180. B897BA is only a decoded client caller fragment, not an adopted function entry.')
for symbol in ('kPb_Tag','kPb_Magic'):
    row,=[r['r2'] for r in core if r['tier']=='B' and r['symbol']==symbol]
    result[row]['evidence'] += '; virtual-queue-bind-reference.txt; virtual-queue-bind-target.txt; virtual-queue-callers-target.txt; virtual-queue-producers-old.txt; virtual-queue-producers-new.txt; virtual-queue-block-producer-target.txt'
    result[row]['second_check'] += ' Native framework producers and queue binders independently establish ServerNet+78 == ClientNet+28 == B+180, closing the actual return/free queue identity; the target framework queue-block member is +148, not legacy +140.'
signatures=docs/'r2-signatures.json'
if signatures.exists():
    result.update(json.loads(signatures.read_text())['rows'])
wire=docs/'r2-wire-reviewed.json'
if wire.exists():
    result.update(json.loads(wire.read_text())['rows'])
(docs/'r2-reviewed.json').write_text(json.dumps(result,indent=2)+'\n')
print('Manually reviewed native contracts',len(result),'(layouts and entry guards; no new function/global RVA adoption)')
