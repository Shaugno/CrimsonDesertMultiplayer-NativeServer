"""Verify file-backed native descriptor/mode operands after paired code review."""
import argparse
import json
from pathlib import Path
from pe_identity import PE

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--reference',type=Path,required=True)
p.add_argument('--target',type=Path,required=True)
args=p.parse_args()
old,new=PE(args.reference),PE(args.target)
assert old.sha256=='57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
rows=[]
for pe,rva in ((old,0x6A65564),(new,0x6F29B70)):
    value=pe.u32(pe.offset(rva,4))
    rows.append(dict(sha256=pe.sha256,rva=hex(rva),initial_file_value=value,
        role='ServerOnRecvPacket native TrocFind mask operand; runtime mutability is not inferred'))
root=Path(__file__).resolve().parents[2]
descriptors=json.loads((root/'docs/probe-3663/port-evidence/r2/request-candidates.json').read_text())
kind_rows=[]
for image,pe in (('old',old),('new',new)):
    for name in ('LoginUserToFieldServerReq','LoginCharacterToFieldServerReq'):
        row,=[r for r in descriptors if r['image']==image and r['name']==f'.?AVTrocTr{name}@pa@@']
        address=int(row['instance'],0)
        kind=pe.u32(pe.offset(address+8,4))
        assert kind==4 and row['kind']==kind
        kind_rows.append(dict(image=image,name=name,instance=hex(address),kind=kind))
out=root/'docs/probe-3663/port-evidence/r2/troc-mode-operands.json'
out.write_text(json.dumps(dict(operands=rows,server_request_kinds=kind_rows,adopted_rva=False,
    evidence='troc-role-reference.txt; troc-role-target.txt; troc-dispatch-lookup-target.txt'),indent=2)+'\n')
print(json.dumps(rows,indent=2))
