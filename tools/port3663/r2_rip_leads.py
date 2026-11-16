"""Locate file-only RIP operand/rel32 call leads; all need native disassembly."""
import argparse
from bisect import bisect_right
import json
from pathlib import Path
import struct
from pe_identity import PE

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('image',type=Path)
p.add_argument('--rvas',nargs='+',required=True,type=lambda v:int(v,0))
p.add_argument('--operation',choices=('lea','cmp','mov','call','jump','simd'),default='lea')
p.add_argument('--out',type=Path,required=True)
args=p.parse_args()
image=PE(args.image)
assert image.sha256 in ('57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7',
    '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8')
assert 0<len(args.rvas)<=64 and all(0<r<image.image_size for r in args.rvas)
wanted=set(args.rvas)
ranges=sorted(image.ranges)
starts=[r[0] for r in ranges]
rows=[]
for section in image.sections:
    if not section['flags']&0x20000000: continue
    data=image.data[section['raw']:section['raw']+section['size']]
    prefixes={'lea':(b'\x48\x8d',b'\x4c\x8d'),'cmp':(b'\x3b',),
              'call':(b'\xe8',),'jump':(b'\xe9',),'mov':(b'\x8b',),
              'simd':(b'\x0f\x10',b'\x0f\x11',b'\xc5\xf8\x10',b'\xc5\xf8\x11',
                      b'\xc5\xfc\x10',b'\xc5\xfc\x11',b'\xc5\xfa\x10',b'\xc5\xfa\x11',
                      b'\xc5\xfb\x10',b'\xc5\xfb\x11')}[args.operation]
    for prefix in prefixes:
        index=-1
        while True:
            index=data.find(prefix,index+1)
            if index<0: break
            operand=index+len(prefix)
            if args.operation in ('call','jump'):
                if operand+4>len(data): continue
                end=operand+4
                displacement=operand
            else:
                if operand+5>len(data) or data[operand]&0xC7!=5: continue
                end=operand+5
                displacement=operand+1
            rva=section['rva']+index
            target=section['rva']+end+struct.unpack_from('<i',data,displacement)[0]
            if target not in wanted: continue
            at=bisect_right(starts,rva)-1
            owner=ranges[at][0] if at>=0 and rva<ranges[at][1] else None
            rows.append(dict(rva=hex(rva),operand_rva=hex(target),
                pdata_owner=hex(owner) if owner else None,verified_instruction_boundary=False))
result=dict(identity=image.identity(),operation=args.operation,leads=rows,adopted=False,
    warning='Raw byte-pattern leads only; verify native instruction boundary, entry, operand and role')
args.out.write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(rows,indent=2))
