"""Extract R1 entry guards and independently reviewed validation windows.

This does not match functions or prove a new RVA. Paired native disassembly
and the R1 identity proofs are prerequisites for entry guards. Explicit identity
validation windows below still need separate standard support for their sites.
"""
import argparse
import json
from pathlib import Path
import re
from pe_identity import PE

root=Path(__file__).resolve().parents[2]
docs=root/'docs/probe-3663'
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--reference',type=Path,required=True)
p.add_argument('--target',type=Path,required=True)
args=p.parse_args()
old,new=PE(args.reference),PE(args.target)
assert old.sha256=='57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
core=json.loads((docs/'R2-CORE-LIST.json').read_text())
r1={r['symbol']:int(r['value'],0) for r in json.loads((docs/'PORT-REVIEWED.json').read_text())['matches']}
contracts=[('kServerOnRecvPacketHead','kServerOnRecvPacket'),('kSessionSendHead','kSessionSend'),
           ('kNwSessionCtorHead','kNwSessionCtor'),('kQueuePushHead','kQueuePush'),
           ('kAllocGlobalHead','kAllocGlobal'),('kAllocThreadBytes','kAllocThread'),
           ('kSyncSessionSendHead','kSyncSessionSend')]
result={}
for signature,function in contracts:
    row,=[r for r in core if r['tier']=='B' and r['symbol']==signature]
    entry,=[r for r in core if r['tier']=='B' and r['symbol']==function]
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    source=int(entry['value'],0); target=r1[function]
    assert source in old.entries and target in new.entries
    assert old.data[old.offset(source,len(previous)):old.offset(source,len(previous))+len(previous)]==previous
    length=len(previous)
    if signature=='kAllocThreadBytes':
        ranges=[end-begin for begin,end in new.ranges if begin==target]
        assert len(ranges)==1
        length=ranges[0]
    at=new.offset(target,length)
    data=new.data[at:at+length]
    if signature=='kAllocThreadBytes': assert data[-5]==0xE9  # verified native tail JMP
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='native entry bytes at retained R1 function',old=row['value'],symbol=signature,
        second_check=f'R1 PORT-REVIEWED.json {function}; paired native disassembly confirms entry and role; old prefix checked against source file; target .pdata entry; {length} bytes. The allocator guard covers its complete entry wrapper through the verified tail JMP; it does not imply equality of the tail callee.',
        evidence='port-evidence/ghidra-2976-expanded.txt; ghidra-3663-expanded.txt; ghidra-3663-core-handlers.txt')
for symbol,source,target,old_owner,new_owner in [
    ('kFindByNoScanBytes',0x2A84066,0x2D7A726,0x2A83F80,0x2D7A640),
    ('kFindByNoReadBytes',0x2A84090,0x2D7A750,0x2A83F80,0x2D7A640)]:
    row,=[r for r in core if r['tier']=='C' and r['symbol']==symbol]
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    for pe,site,owner in ((old,source,old_owner),(new,target,new_owner)):
        assert any(begin==owner and begin<=site and site+len(previous)<=end for begin,end in pe.ranges)
    at=old.offset(source,len(previous))
    assert old.data[at:at+len(previous)]==previous
    at=new.offset(target,len(previous));data=new.data[at:at+len(previous)]
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='paired native validation-window bytes',old=row['value'],symbol=symbol,
        second_check='Reviewed native FindByCharacterNo scans manager+B8/count+C0 and reads actor+68/status+20, then the changed number pointer+90. Both instruction windows are decoded boundaries within the paired .pdata functions. These are validation windows, not inline-hook entries; C083/C085 still require standard support separately.',
        evidence='port-evidence/r2/identity-contracts-reference.txt; identity-find-by-number-target.txt')
for symbol,source,target in [('kLoadingHead',0x219D7F0,0x23DC630),
                             ('kLoadingFlagCmpBytes',0x219D82B,0x23DC66B)]:
    row,=[r for r in core if r['tier']=='C' and r['symbol']==symbol]
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    for pe,site,owner in ((old,source,0x219D7F0),(new,target,0x23DC630)):
        assert any(begin==owner and begin<=site and site+len(previous)<=end for begin,end in pe.ranges)
    at=old.offset(source,len(previous))
    assert old.data[at:at+len(previous)]==previous
    at=new.offset(target,len(previous));data=new.data[at:at+len(previous)]
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='paired native completion entry/branch bytes',old=row['value'],symbol=symbol,
        second_check='Named LoadingCompleteReq reaches paired six-argument completion 219D7F0/23DC630. Native disassembly confirms the entry and fifth-argument CMP/JZ publication gate; exact old bytes and both .pdata owner ranges checked. The entry and branch RVAs still require standard support independently.',
        evidence='port-evidence/r2/clock-gate-old.txt; clock-gate-new.txt; lifecycle-loading-old.txt; lifecycle-loading-new.txt')
for symbol,source,target,old_owner,new_owner in [
    ('kNetLoopPumpHead',0x35BAD4,0x3A8D94,0x35B800,0x3A8AC0),
    ('kNetLoopPumpAob',0x35BAD4,0x3A8D94,0x35B800,0x3A8AC0),
    ('kClientOnRecvPacketHead',0x35CA00,0x3A9CF0,0x35CA00,0x3A9CF0)]:
    row,=[r for r in core if r['tier']=='B' and r['symbol']==symbol]
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    for pe,site,owner in ((old,source,old_owner),(new,target,new_owner)):
        assert any(begin==owner and begin<=site and site+len(previous)<=end for begin,end in pe.ranges)
    at=old.offset(source,len(previous))
    assert old.data[at:at+len(previous)]==previous
    at=new.offset(target,len(previous));data=new.data[at:at+len(previous)]
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='paired native network entry/midpoint guard bytes',old=row['value'],symbol=symbol,
        second_check='Paired complete NetworkThread loops and native client receive bodies reviewed. The pump starts at the seven-byte MOV RAX,[R15+E0] with ClientNet in R15 before native queue flush; receive is a verified function entry. Exact source bytes and both .pdata owner ranges checked; guard prefixes are not claims of whole-body equality. Candidate site/entry RVAs require separate standard support.',
        evidence='port-evidence/r2/network-pump-reference.txt; tunnel-layout-target.txt; tunnel-client-receive-reference.txt; client-receive-verified-target.txt')
checks=json.loads((docs/'r2-rva-checks.json').read_text())
joined_path=docs/'R2-MATCHES.json'
joined={r['row']:r for r in json.loads(joined_path.read_text())['rows']} if joined_path.exists() else {}
for tier,signature,function in [
    ('C','kAcquireFieldHead','kAcquireField'),('C','kEnterFieldHead','kEnterField'),
    ('C','kTryAddRefHead','kTryAddRef'),('C','kVecPushHead','kVecPush'),
    ('C','kLobbyLoadHead','kLobbyLoad'),('C','kLoadHead','kLoad'),
    ('C','kGetterHead','kGetter'),('C','kActionLayerAdvanceHead','kActionLayerAdvance'),
    ('C','kActionLayersAdvanceHead','kActionLayersAdvance'),
    ('D','kWindowsLockVf1Head','kWindowsLockVf1'),('D','kWindowsLockVf2Head','kWindowsLockVf2'),
    ('D','kSaveMgrRefreshSlotsHead','kSaveMgrRefreshSlots'),('D','kLoadSlotHead','kLoadSlot'),
    ('D','kSaveHead','kSave'),('D','kFlushHead','kFlush'),
    ('D','kGetOrCreateHead','kGetOrCreate'),('D','kAddFileHead','kAddFile')]:
    row,=[r for r in core if r['tier']==tier and r['symbol']==signature]
    entry,=[r for r in core if r['tier']==tier and r['symbol']==function]
    proof=checks.get(entry['r2'])
    accepted=joined.get(entry['r2'],{})
    target_value=(proof or {}).get('target') or (accepted.get('target') if accepted.get('adopted') else None)
    if target_value is None: continue
    source=int(entry['value'],0);target=int(target_value,0)
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    at=old.offset(source,len(previous))
    assert old.data[at:at+len(previous)]==previous,(signature,'reference prefix')
    bodies=[end-begin for begin,end in new.ranges if begin==target]
    if bodies:
        length=min(len(previous),max(bodies))
        assert length>=5,(signature,'target body too short')
        form='verified .pdata entry'
    else:
        if function in ('kWindowsLockVf1','kWindowsLockVf2'):
            index=1 if function=='kWindowsLockVf1' else 2
            for image,table,method in [('old',0x557C120,source),('new',0x59874C8,target)]:
                types=json.loads((docs/f'port-evidence/r2/save-lock-rtti-{image}.json').read_text())['types']
                typ,=[t for t in types if t['name']=='.?AV?$WindowsLock@$02@pa@@']
                vt,=[v for v in typ['vtables'] if int(v['rva'],0)==table]
                slot,=[s for s in vt['slots'] if s['index']==index]
                assert int(slot['rva'],0)==method
            expected=bytes([0x48,0x8B,0x01,0x48,0xFF,0x60,0x18 if index==1 else 0x20])
            at=new.offset(target,7);assert new.data[at:at+7]==expected
            length=7;form='RTTI-slot-verified complete virtual-dispatch thunk'
        else:
            at=new.offset(target,5)
            assert new.data[at]==0xE9,(signature,'unverified non-pdata entry')
            length=5;form='native direct-jump thunk'
    at=new.offset(target,length);data=new.data[at:at+length]
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='verified native entry guard bytes',old=row['value'],symbol=signature,
        second_check=f'{function}: reference {hex(source)}, target candidate {hex(target)}, {form}; exact reference guard checked, target {length}-byte prefix extracted within its entry body. A short thunk guard covers that complete wrapper. Native role evidence or dual standard correspondence is recorded separately for {entry["r2"]}; guard bytes do not adopt its RVA, unchanged ABI, or authorize a legacy detour.',
        evidence=(proof or {}).get('evidence') or 'R2-MATCHES.json; PORT-REVIEWED.json; paired whole-image exports',
        entry_rva_row=entry['r2'],entry_rva_adopted=bool(accepted.get('adopted')))
for symbol,source,target in [('kLobbyConstantBytes',0x2B7F66C,0x2E87DB0),
                             ('kLobbyAddCallBytes',0x2B7F6DB,0x2E87E10)]:
    row,=[r for r in core if r['tier']=='C' and r['symbol']==symbol]
    previous=bytes(int(s,16) for s in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',row['value']))
    for pe,site,owner in ((old,source,0x2B7F4D0),(new,target,0x2E879A0)):
        assert any(begin==owner and begin<=site and site+len(previous)<=end for begin,end in pe.ranges)
    at=old.offset(source,len(previous));assert old.data[at:at+len(previous)]==previous
    at=new.offset(target,len(previous));data=new.data[at:at+len(previous)]
    result[row['r2']]=dict(target='{'+', '.join(f'0x{b:02X}' for b in data)+'}',
        method='paired native lobby validation-window bytes',old=row['value'],symbol=symbol,
        second_check='Paired named lobby loaders construct INT64_MAX and pass its address to the native lobby-entry writer. Decoded MOV/stack-store and E8 call roles are reviewed; exact source bytes and both parent .pdata ranges checked. The corresponding instruction/call RVAs require standard support separately; the changed native entry ABI is not bypassed.',
        evidence='port-evidence/r2/identity-contracts-reference.txt; identity-contracts-target.txt; identity-entry-reference.txt; identity-entry-target.txt')
payload=dict(reference_sha256=old.sha256,target_sha256=new.sha256,rows=result)
(docs/'r2-signatures.json').write_text(json.dumps(payload,indent=2)+'\n')
print('Reviewed native entry guards/validation windows',len(result),'(no new RVA matching)')
