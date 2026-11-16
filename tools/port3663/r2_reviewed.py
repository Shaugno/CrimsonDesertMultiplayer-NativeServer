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
prove('A','src/mod/game/offsets.h',dict(kPhysics_AxisX='0x60',kPhysics_AxisY='0x70',kPhysics_AxisZ='0x80'),
    'port-evidence/r2/player-inner-target.txt; player-stat-matrix-target.txt',
    'Physics base constructor 4601320 creates rotation at this+60 with quaternion-to-matrix 44AF690 (rows +0/+10/+20); inverse 44AD6C0 consumes that three-row matrix, with position separately at +90.')
prove('A','src/mod/game/offsets.h',dict(kComponents_Status='0x20',kStatus_StatContainer='0x18',kStatContainer_Entries='0x58',kStatContainer_Count='0x60',kStatEntry_Size='0x90',kStatEntry_Id='0x00',kStatEntry_Current='0x08',kStatEntry_Max='0x18'),
    'port-evidence/r2/player-health-handler-target.txt; player-health-update-target.txt; player-status-target.txt; player-stat-stride-target.txt; player-stat-lookup-target.txt',
    'Named TrocTrVaryHPAck invokes status [[actor+68]+20]; native HP updater uses status+18; stat initialization/lookup consumes vector+58/count+60, u16 key at entry+0, current+8, maximum+18 and 90-byte stride, clamping current to maximum.')
prove('A','src/mod/game/player_layout.h',dict(NativeStatUnits='1000.0f',StatIdReadWidth='2'),
    'port-evidence/r2/player-health-update-target.txt; player-stat-stride-target.txt; player-stat-lookup-target.txt',
    'Native HP updater converts integer units to displayed HP by /1000; initializer converts values with *1000 and writes/compares u16 keys, never the six adjacent padding bytes. Legacy reader width 8 is preserved for 2976 only.')
(docs/'r2-reviewed.json').write_text(json.dumps(result,indent=2)+'\n')
print('Manually reviewed layout contracts',len(result),'(no new function/global RVA adoption)')
