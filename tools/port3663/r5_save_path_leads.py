"""Raw file-only save-path leads; native boundaries/context must be checked separately."""
from bisect import bisect_right
import json
from pathlib import Path
import re
import struct
from pe_identity import PE
root=Path(__file__).resolve().parents[2]
pe=PE(Path(r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'))
assert pe.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
ranges=sorted(pe.ranges);starts=[p[0] for p in ranges]
def owner(rva):
    n=bisect_right(starts,rva)-1
    return hex(starts[n]) if n>=0 and rva<ranges[n][1] else None
targets={0x5613640:'SHGetFolderPathW IAT',0x5613658:'SHGetKnownFolderPath IAT',
    0x5613ec8:'GetEnvironmentVariableA IAT',0x5613ed0:'GetEnvironmentVariableW IAT'}
strings=[]
for s in pe.sections:
    data=pe.data[s['raw']:s['raw']+s['size']]
    for name in ['Pearl Abyss','LOCALAPPDATA','USERPROFILE']:
        for encoding in ['ascii','utf-16-le']:
            pattern=name.encode(encoding);at=-1
            while True:
                at=data.find(pattern,at+1)
                if at<0:break
                rva=s['rva']+at;targets[rva]=name+' '+encoding
                strings.append(dict(rva=hex(rva),name=name,encoding=encoding))
rows=[]
for s in pe.sections:
    if not s['flags']&0x20000000:continue
    data=pe.data[s['raw']:s['raw']+s['size']]
    for pattern,size,disp in [(rb'\xff[\x15\x25]....',6,2),
        (rb'[\x48\x4c][\x8b\x8d][\x05\x0d\x15\x1d\x25\x2d\x35\x3d]....',7,3)]:
        for match in re.finditer(pattern,data,re.DOTALL):
            at=match.start();rva=s['rva']+at;target=rva+size+struct.unpack_from('<i',data,at+disp)[0]
            if target in targets:rows.append(dict(rva=hex(rva),target=hex(target),name=targets[target],pdata_owner=owner(rva)))
out=dict(executable_sha256=pe.sha256,strings=strings,leads=rows,
    warning='Raw leads only; no function address adoption or game execution')
(root/'build/r5/save-path-leads.json').write_text(json.dumps(out,indent=2)+'\n',encoding='utf-8')
print(json.dumps(out,indent=2))
