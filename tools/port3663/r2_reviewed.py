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
signatures=docs/'r2-signatures.json'
if signatures.exists():
    result.update(json.loads(signatures.read_text())['rows'])
(docs/'r2-reviewed.json').write_text(json.dumps(result,indent=2)+'\n')
print('Manually reviewed native contracts',len(result),'(layouts and entry guards; no new function/global RVA adoption)')
