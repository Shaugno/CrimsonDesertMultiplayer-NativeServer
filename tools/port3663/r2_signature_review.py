"""Extract entry guards only for already adopted R1 native functions.

This does not match functions or prove a new RVA. Paired native disassembly
and the R1 identity proofs are prerequisites, listed explicitly below.
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
payload=dict(reference_sha256=old.sha256,target_sha256=new.sha256,rows=result)
(docs/'r2-signatures.json').write_text(json.dumps(payload,indent=2)+'\n')
print('Reviewed native entry guards',len(result),'(no new RVA matching)')
