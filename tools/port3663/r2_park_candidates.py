"""Find literal park vectors for native code review, never auto-adopt them."""
import json
from pathlib import Path
import struct
import sys
from pe_identity import PE

image = PE(sys.argv[1])
if image.sha256 not in ('57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7',
                       '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'):
    raise SystemExit('Unexpected image')
pattern = struct.pack('<fff',0,1000,0)
rows = []
for section in image.sections:
    if section['flags'] & 0x20000000: continue
    start,end = section['raw'],section['raw']+section['size']
    at = image.data.find(pattern,start,end)
    while at>=0:
        rva = section['rva']+at-start
        if rva%4==0: rows.append(dict(rva=hex(rva),section=section['name']))
        at = image.data.find(pattern,at+1,end)
Path(sys.argv[2]).write_text(json.dumps(dict(identity=image.identity(),candidates=rows,adopted=False),indent=2)+'\n')
print(rows)
