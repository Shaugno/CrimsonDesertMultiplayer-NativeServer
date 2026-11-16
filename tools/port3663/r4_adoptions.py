"""R4 additive fact dispositions; preserve prior ledgers and matching conflicts."""
from pathlib import Path
import hashlib
import json
import sqlite3
from pe_identity import PE

root=Path(__file__).resolve().parents[2]
docs=root/'docs/probe-3663'
local=Path(r'C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7')
old=PE(local/'reference-bins/2976/CrimsonDesert.exe')
new=PE(Path(r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'))
assert old.sha256=='57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
trace=json.loads((docs/'port-evidence/r4/passive-result.json').read_text(encoding='utf-8'))
assert trace['events']==161 and not trace['invalid_events'] and trace['vector_pairs']==2
assert trace['load_stub_dispatch_calls']==1 and trace['key_class_pairs']==['definition 2 -> class A','definition 3 -> class B']
db=sqlite3.connect((local/'analysis-tools/r2-nativefix/bindiff/old_vs_new.BinDiff').as_uri()+'?mode=ro',uri=True)
def match(source,target):
    rows=db.execute('select address2,similarity,confidence from function where address1=?',(old.image_base+source,)).fetchall()
    assert len(rows)==1 and rows[0][0]==new.image_base+target
    return dict(source=hex(source),target=hex(target),similarity=rows[0][1],confidence=rows[0][2])
standards=[match(a,b) for a,b in [(0x2775800,0x2A10830),(0x27779F0,0x2A12A30),
    (0x2C3E030,0x2F4AE80),(0x1434880,0x1659380),(0x2A827B0,0x2D78E40),
    (0x2A82590,0x2D78C20),(0x2BBF340,0x2EC9C70),(0x21A1580,0x23E1090)]]
assert all(s['confidence']>.98 for s in standards)
before=(root/'build/r4/disconnect-native-old.txt').read_text(encoding='utf-8')
after=(root/'build/r4/disconnect-native-new.txt').read_text(encoding='utf-8')
assert 'CMP byte ptr [RSI + 0x58]' in before and 'CMP byte ptr [RSI + 0x48]' in after
rows={}
def add(ids,target,reason,disposition='adopted'):
    for row in ids.split():rows[row]=dict(target=target,reason=reason,disposition=disposition,
        authority='port-evidence/r4/core-adoptions.json',missing='')
add('C001','0x7283378','Strong paired SAM getter returns this holder; actual teardown55288C0, not weak54EEA70; live root equals login-manager SAM and exact named vtable5F69100.')
add('C047','0x4CAE10','Complete paired vector allocator/growth/copy/free contract; actual AddActor call EB6883B and two paired passive growth events. Conflicting default generic-helper match14AD670 rejected in this role.')
add('C066','0xA','Key producer2D740D0 installs definition type bits at2D741EA; live primary definition2 ->classA and children definition3 ->classB, distinct from roster kind0.')
add('C070','0x2E87DB0','Native constant and its stack lifetime measured at the approved passive point. New adapter normalizes after original LobbyLoad; no midpoint instruction patch is required.','replaced_by_adapter')
add('C072','0x2E87E10','Native entry writer consumes live R8 pointer to INT64_MAX under owner lock; new adapter does not hook or invoke this many-argument callsite. Guarded parent entry/return replaces the old patch.','replaced_by_adapter')
add('C074','0x2F4AF52','Strong matched character-login parent and exact native component/owner loads; passive sample agrees with actual user/lobby.')
add('C076','0x2F4AF79','Strong matched parent, native list load and clear routine; passive pre-clear list count1 and INT64_MAX.')
add('C080','0x2A12A58','Strong matched native parent2A12A30 loads72456E0 then dereferences manager. No live site hit; independent roots/getter establish its exact SAM object, not a fabricated hit.')
for row,rva,length in [('C075',0x2F4AF52,12),('C077',0x2F4AF79,4),('C081',0x2A12A58,10)]:
    add(row,new.data[new.offset(rva,length):][:length].hex(' '),'Exact target instruction bytes from independently matched native context; all guarded separately in new adapter.')
add('C103','0x3ABEE0','Exact named IActorComponent primary slot10 and many derived tables; paired zero-result thunk bodies. One passive native component-dispatch hit returned through the actual shared stub.')
add('C122','0','Retain the native session constructor key unchanged; native receive loads its ID from+88; passive title ID0 and completed R3 private-session crypto path corroborate. No forced nonzero ID or table index.')
add('C144','0x90','u32 controller/owning-user key in named MoveActorReq and adopted recipient finder C007; passive comparison uses vf170 returned key. Distinct pointer+A0 is not substituted.')
add('D014','0x1593240','Exact named WindowsLock3 slot1 and paired dispatch-through+18 thunk, now corroborated by actual native save-manager lock and vslot1.')
add('D015','0xE297F0','Exact named WindowsLock3 slot2 and paired dispatch-through+20 thunk, now corroborated by actual native save-manager lock and vslot2.')
add('D030','31','Conservative geometry bound: proved 256-byte bucket, 8-byte header, 8-byte pairs =>31. Reject larger counts before reading; not a claim that observed counts reached31.','adopted_policy')
add('D044','0x3DC','Strong matched native movement predicate23E1090, actual CMP at23E1106; passive owner/component chain agrees. Fragment23E0FF0 rejected.')
add('D045','0x48','Strong paired FieldMover::GetField2BBF340->2EC9C70 compares moving flag58->48, refuses access while set, increments busy5A->4A only after acceptance. Native field-access destructor balances that access.')
add('D115',new.data[new.offset(0x1536D20,16):][:16].hex(' '),'Adopted reporter1536D20, complete native body and exact target prologue. This fact alone does not install/suppress any reporter.')
proofs=[]
for name in ['build/r4/disconnect-native-old.txt','build/r4/disconnect-native-new.txt',
             'build/r4/reference-lifecycle-old.txt','build/r4/reference-lifecycle-new.txt',
             'build/r4/typed-pins-new.txt','build/r4/field-scope-destructor-new.txt',
             'docs/probe-3663/port-evidence/r4/native-bindings.json','docs/probe-3663/port-evidence/r4/passive-result.json']:
    path=root/name;proofs.append(dict(path=str(path.resolve()),sha256=hashlib.sha256(path.read_bytes()).hexdigest().upper()))
out=dict(rows=rows,standard_contexts=standards,evidence=proofs,
         limits='These close address/layout/policy obligations. They do not establish guest live execution or authorize an incomplete production route. C/D arming remains a separate Step4 gate.')
(docs/'port-evidence/r4/core-adoptions.json').write_bytes((json.dumps(out,indent=2)+'\n').encode('utf-8'))
print(f'{len(rows)} additive C/D fact dispositions; tier permissions unchanged')
