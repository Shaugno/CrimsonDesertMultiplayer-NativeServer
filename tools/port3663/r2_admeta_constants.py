"""Check operands from the native .admeta producer review, not RVA adoption."""
import argparse
import json
from pathlib import Path
import struct
from pe_identity import PE

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--reference',type=Path,required=True)
p.add_argument('--target',type=Path,required=True)
args=p.parse_args()
old,new=PE(args.reference),PE(args.target)
assert old.sha256=='57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
operands=[]
for pe,site,prefix,target in (
    (old,0x128D3B6,b'\x4c\x8d\x05',0x5709C10),
    (new,0x14AA776,b'\x4c\x8d\x05',0x5B23468),
    (new,0x14AA9D2,b'\x48\x8d\x0d',0x71AEAA0),
    (new,0x14AA9E9,b'\x48\x8b\x05',0x71AEAA0),
    (new,0x14AA9F3,b'\xff\x05',0x71AEAA8)):
    at=pe.offset(site,len(prefix)+4)
    assert pe.data[at:at+len(prefix)]==prefix
    destination=site+len(prefix)+4+struct.unpack_from('<i',pe.data,at+len(prefix))[0]
    assert destination==target
    operands.append(dict(sha256=pe.sha256,instruction_rva=hex(site),operand_rva=hex(target)))
for pe,rva in ((old,0x5709C10),(new,0x5B23468)):
    at=pe.offset(rva,9)
    assert pe.data[at:at+9]==b'*.admeta\0'
root=Path(__file__).resolve().parents[2]
result=dict(reference_sha256=old.sha256,target_sha256=new.sha256,
    pattern='*.admeta',producer_candidate=dict(reference='0x128D360',target='0x14AA720'),
    target_id_vector_candidate=dict(data='0x71AEAA0',count='0x71AEAA8',item_bytes=4,
        old_counterpart=None),operands=operands,adopted_rva=False,
    review='Reference producer builds a metadata-name vector. Target additionally reads a u32 from each metadata file and appends it to the new ID vector. Native title eligibility requires each saved entry ID in that available set; no DLC meaning is assumed.',
    evidence='title-requirement-producer-reference.txt; title-requirement-producers-target.txt; title-predicate-candidates-target.txt')
(root/'docs/probe-3663/port-evidence/r2/admeta-operands.json').write_text(json.dumps(result,indent=2)+'\n')
print('Verified .admeta pattern and native ID-vector operands; new global candidates remain unadopted')
