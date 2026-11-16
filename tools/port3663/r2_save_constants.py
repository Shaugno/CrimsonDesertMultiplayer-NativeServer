"""Verify file-backed operands consumed by paired native save bodies."""
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
def u32(pe,rva): return pe.u32(pe.offset(rva,4))
def literal(pe,rva,expected):
    at=pe.offset(rva,len(expected))
    assert pe.data[at:at+len(expected)]==expected,(hex(rva),expected)
    return dict(rva=hex(rva),value=expected.rstrip(b'\0').decode('ascii'))
assert u32(old,0x5547B6C)==u32(new,0x59728FC)==0x45564153
assert u32(old,0xEC0FE84)^0x48D9F781==201
assert (0xF9D00CD-u32(new,0xB7A5B48))&0xFFFFFFFF==205
rows=dict(magic=dict(old_operand='0x5547B6C',target_operand='0x59728FC',value='0x45564153'),
    memory_only=dict(exact=201,range_begin=205,range_count=3,
        target_delta_operand='0xB7A5B48',target_delta=hex(u32(new,0xB7A5B48))),
    old_prefix=literal(old,0x57A8080,b'slot\0'),
    target_prefix=literal(new,0x5BD4208,b'slot\0'),
    target_formatter=literal(new,0x599A714,b'%#%#\0'),
    target_save_name=literal(new,0x5D410E0,b'save\0'),
    target_lobby_name=literal(new,0x5D410E8,b'lobby\0'))
out=Path(__file__).resolve().parents[2]/'docs/probe-3663/r2-save-constants.json'
out.write_text(json.dumps(dict(reference_sha256=old.sha256,target_sha256=new.sha256,
    scope='Only the listed native operands/literals, not a complete header validator or RVA adoption',
    verified=rows),indent=2)+'\n')
print('Paired native save operands/literals verified; no new function/global adoption')
