"""Join standard VT and BinDiff results; never infer an unchanged layout.

Run only after both correlators finish. Raw outputs are hashed, not embedded.
R1 adoptions are preserved independently of new matches. Additional manually
reviewed layout/ABI proofs belong in r2-reviewed.json with concrete evidence.
"""
import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import re
import sqlite3

root = Path(__file__).resolve().parents[2]
docs = root/'docs/probe-3663'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--vt',type=Path,required=True)
parser.add_argument('--bindiff',type=Path,required=True)
parser.add_argument('--reviewed',type=Path)
args = parser.parse_args()
vt = defaultdict(list)
with args.vt.open(encoding='utf-8-sig') as stream:
    for row in csv.DictReader(stream,delimiter='\t'):
        if row['status'] != 'ACCEPTED': continue
        source = int(row['source'].split(':')[-1],16)-0x140000000
        destination = int(row['destination'].split(':')[-1],16)-0x140000000
        if source>0 and destination>0: vt[source].append(dict(row, target=destination))
db = sqlite3.connect(args.bindiff.resolve().as_uri()+'?mode=ro',uri=True)
bd = {int(a)-0x140000000:dict(target=int(b)-0x140000000,similarity=similarity,confidence=confidence)
      for a,b,similarity,confidence in db.execute('SELECT address1,address2,similarity,confidence FROM function')}
db.close()
r1 = {r['symbol']:r for r in json.loads((docs/'PORT-REVIEWED.json').read_text())['matches']}
reviewed = json.loads(args.reviewed.read_text()) if args.reviewed else {}
core = json.loads((docs/'R2-CORE-LIST.json').read_text())
results = []
for row in core:
    result = dict(tier=row['tier'],row=row['r2'],symbol=row['symbol'],old=row['value'],target=None,
                  adopted=False,method='none',second_check='none',confidence='blocked')
    value = row['value']
    missing = f"Confirm 3663 code consumes {row['symbol']} for the same native role as {row['source']} ({value})"
    if row['adopted_r1']:
        proof = r1[row['symbol']]
        result.update(target=proof['value'],adopted=True,method='retained R1 '+proof['proof'],
                      second_check='PORT-REVIEWED.json proof and paired disassembly',confidence='high')
    elif re.fullmatch(r'0x[0-9A-Fa-f]+',value) and 0x10000<int(value,16)<0x173AB000:
        source = int(value,16)
        matches = vt.get(source,[])
        destinations = {m['target'] for m in matches if m['type']=='FUNCTION'}
        binary = bd.get(source)
        if binary and len(destinations)==1 and binary['target'] in destinations and binary['confidence']>=0.8:
            result.update(target=hex(binary['target']),adopted=True,
                method='VT '+', '.join(sorted({m['method'] for m in matches})),
                second_check=f"BinDiff similarity={binary['similarity']:.6f}, confidence={binary['confidence']:.6f}",confidence='high')
        else:
            result['method'] = 'VT '+', '.join(f"{hex(m['target'])}:{m['method']}" for m in matches) if matches else 'VT no accepted pair'
            result['second_check'] = f"BinDiff {hex(binary['target'])} confidence={binary['confidence']:.6f}" if binary else 'BinDiff no function pair'
            missing = f"Resolve standard correlator agreement or add an independent code/RTTI/xref proof for {row['symbol']} at 2976 {value}"
    if row['r2'] in reviewed:
        manual = reviewed[row['r2']]
        assert manual['evidence'] and manual['second_check'] and manual['target'] is not None
        result.update(manual,adopted=True,confidence='high')
    result['missing'] = '' if result['adopted'] else missing
    results.append(result)
manifest = [dict(path=str(p.resolve()),bytes=p.stat().st_size,sha256=hashlib.file_digest(p.open('rb'),'sha256').hexdigest())
            for p in (args.vt,args.bindiff)]
payload = dict(standard_outputs=manifest,rows=results)
(docs/'R2-MATCHES.json').write_text(json.dumps(payload,indent=2)+'\n')
text = '# R2 matches — standard correlators and independent checks\n\n'
for item in manifest: text += f"- Local raw output `{item['path']}`; {item['bytes']} bytes; SHA-256 `{item['sha256']}`.\n"
text += '\nBoth standard function matches require an unambiguous accepted VT function association and BinDiff confidence >=0.8. Layouts and native payload contracts require separate code evidence. RTTI symbol seeds are not counted again as independent function proof. A matched RVA does not prove its hook signature or ABI.\n\n'
for tier in 'ABCD':
    selected = [r for r in results if r['tier']==tier]
    text += f"Tier {tier}: {sum(r['adopted'] for r in selected)} adopted, {sum(not r['adopted'] for r in selected)} blocked.\n\n"
text += '| Tier | Row / symbol | 3663 | Method | Second check | Confidence / missing fact |\n|---|---|---|---|---|---|\n'
for row in results:
    cells = [row['tier'],row['row']+' '+row['symbol'],str(row['target'] or 'unadopted'),row['method'],row['second_check'],row['confidence']+(': '+row['missing'] if row['missing'] else '')]
    text += '| '+' | '.join(s.replace('|','\\|').replace('\n',' ') for s in cells)+' |\n'
(docs/'R2-MATCHES.md').write_text(text)
print(Counter((r['tier'],r['adopted']) for r in results))
