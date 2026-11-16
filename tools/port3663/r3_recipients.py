"""Apply user-approved R3 rule 2 to named recipient sibling conflicts.

Reuses the reviewed R2 native bodies; produces a small result, no raw dump.
This is not permission to install a Tier C hook.
"""
import hashlib
import json
from pathlib import Path
import sqlite3
import struct
from pe_identity import PE

ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT/'docs/probe-3663'
LOCAL = Path(r'C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7')
old = PE(LOCAL/'reference-bins/2976/CrimsonDesert.exe')
new = PE(Path(r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe'))
assert old.sha256 == '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'
assert new.sha256 == '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
core = {r['r2']: r for r in json.loads((DOCS/'R2-CORE-LIST.json').read_text(encoding='utf-8'))}
db = sqlite3.connect((LOCAL/'analysis-tools/r2-nativefix/bindiff/old_vs_new.BinDiff').as_uri()+'?mode=ro', uri=True)
types = [json.loads((DOCS/f'port-evidence/rtti-{v}.json').read_text(encoding='utf-8'))['types'] for v in ('2976','3663')]
exports = [(DOCS/f'port-evidence/r2/recipient-finders-{v}.txt').read_text(encoding='utf-8') for v in ('old','new')]
rows = {}
for suffix,table_row,function_row,source_table,target_table,source_fn,target_fn in [
    ('ExceptOneControl','C009','C012',0x5B25358,0x5F686C0,0x2A759B0,0x2D6C360),
    ('ExceptOneSelf','C010','C013',0x5B25378,0x5F686A0,0x2A75880,0x2D6C230),
    ('','C011','C014',0x5B25418,0x5F68780,0x2A75860,0x2D6C210)]:
    name=f'.?AVToNear{suffix}SendTargetFinder@pa@@'
    for image,typelist,table,entry in [(old,types[0],source_table,source_fn),(new,types[1],target_table,target_fn)]:
        typ, = [t for t in typelist if t['name']==name]
        vt, = [v for v in typ['vtables'] if int(v['rva'],0)==table]
        slot, = [s for s in vt['slots'] if s['index']==1]
        assert int(slot['rva'],0)==entry
        assert image.u64(image.offset(table+8,8)) == image.image_base+entry
    if suffix:
        for text,entry,mover,getter in [(exports[0],source_fn,'0x4f0','0x142bbf340'),(exports[1],target_fn,'0x508','0x142ec9c70')]:
            body=text.split('RVA='+hex(entry).upper().replace('0X','0x'),1)[1].split('RVA=',1)[0]
            assert ' + 0x10]' in body and ' + 0x1a0]' in body and mover in body
            assert 'CALL '+getter in body and ' + 0x148]' in body
        behavior='Reviewed native bodies obtain actor/TS from finder+10, use mover 4F0->508, acquire/release the native field holder and do not fill the output. Named RTTI disambiguates near-identical siblings.'
    else:
        for image,entry,callee in [(old,source_fn,0x2BC51E0),(new,target_fn,0x2ED0860)]:
            at=image.offset(entry,20)
            assert image.data[at:at+15] == bytes.fromhex('488b4110488b4868488b89a0010000')
            assert image.data[at+15]==0xE9
            assert entry+20+struct.unpack_from('<i',image.data,at+16)[0]==callee
        behavior='Named slot-1 leaf wrappers tail-dispatch the independently reviewed near-recipient collection; both traverse finder+10, actor+68, TS+1A0 and jump to matched 2BC51E0/2ED0860.'
    conflict=[dict(target=hex(b-image.image_base),similarity=s,confidence=c) for b,s,c in db.execute('select address2,similarity,confidence from function where address1=?',(old.image_base+source_fn,)) for image in [new]]
    for row,target in [(table_row,target_table),(function_row,target_fn)]:
        rows[row]=dict(symbol=core[row]['symbol'],old=core[row]['value'],target=hex(target),adopted=True,
            method='Approved R3 rule 2: exact named RTTI slot plus independent native behavior',
            native_check=behavior,bindiff_result=conflict,conflict_retained=True,tier_armed=False)
for row,entry in [('C015',0x2D6C360),('C016',0x2D6C210)]:
    import re
    expected=bytes(int(b,16) for b in re.findall(r'0x([0-9A-Fa-f]{2})(?=\s*[,}])',core[row]['value']))
    data=new.data[new.offset(entry,len(expected)):][:len(expected)]
    if row=='C015':
        assert data==new.data[new.offset(0x2D6C230,len(data)):][:len(data)]
    rows[row]=dict(symbol=core[row]['symbol'],old=core[row]['value'],target=data.hex(' '),adopted=True,
        method='Native entry prefix for the R3 named-slot adoption; not an unchanged ABI claim',tier_armed=False)
proofs=[]
for name in ['recipient-finders-old.txt','recipient-finders-new.txt','recipient-near-complete-old.txt','recipient-near-helpers-new.txt']:
    p=DOCS/'port-evidence/r2'/name
    proofs.append(dict(path=str(p.relative_to(ROOT)),sha256=hashlib.sha256(p.read_bytes()).hexdigest().upper()))
out=DOCS/'port-evidence/r3/recipient-adoptions.json'
out.write_bytes((json.dumps(dict(authority='User-approved R3 rule 2 overrides R2 conflict veto for these named siblings',rows=rows,evidence=proofs),indent=2)+'\n').encode())
print('R3 named recipient adoptions:',len(rows),'Tier C remains unarmed')
