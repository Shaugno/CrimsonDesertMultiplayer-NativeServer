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
parser.add_argument('--checks',type=Path,default=docs/'r2-rva-checks.json')
parser.add_argument('--output-dir',type=Path,default=docs)
args = parser.parse_args()
vt = defaultdict(list)
with args.vt.open(encoding='utf-8-sig') as stream:
    for row in csv.DictReader(stream,delimiter='\t'):
        if row['status'] != 'ACCEPTED': continue
        source = int(row['source'].split(':')[-1],16)-0x140000000
        destination = int(row['destination'].split(':')[-1],16)-0x140000000
        if source>0 and destination>0: vt[source].append(dict(row, target=destination))
db = sqlite3.connect(args.bindiff.resolve().as_uri()+'?mode=ro',uri=True)
binary_rows=list(db.execute('SELECT address1,address2,similarity,confidence FROM function'))
assert len(binary_rows)==len({a for a,_,_,_ in binary_rows}),'Ambiguous duplicate BinDiff source addresses'
bd = {int(a)-0x140000000:dict(target=int(b)-0x140000000,similarity=similarity,confidence=confidence)
      for a,b,similarity,confidence in binary_rows}
db.close()
r1 = {r['symbol']:r for r in json.loads((docs/'PORT-REVIEWED.json').read_text())['matches']}
reviewed = json.loads(args.reviewed.read_text()) if args.reviewed else {}
checks = json.loads(args.checks.read_text()) if args.checks.exists() else {}
def standard_conflict(source,target):
    destinations={m['target'] for m in vt.get(source,[])}
    binary=bd.get(source)
    return bool((destinations and destinations!={target}) or
        (binary and binary['confidence']>=0.8 and binary['target']!=target))

def standard_support(source,target):
    matches=vt.get(source,[])
    destinations={m['target'] for m in matches}
    support=[]
    binary=bd.get(source)
    # Contradictory standard evidence needs explicit investigation. A native
    # candidate catalog must never silently conceal another accepted result.
    if standard_conflict(source,target): return []
    if destinations=={target}:
        support.append('VT '+', '.join(sorted({m['method'] for m in matches})))
    if binary and binary['target']==target and binary['confidence']>=0.8:
        support.append(f"BinDiff similarity={binary['similarity']:.6f}, confidence={binary['confidence']:.6f}")
    return support
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
        if binary and len(destinations)==1 and binary['target'] in destinations and binary['confidence']>=0.8 and binary['similarity']>=0.8 and not standard_conflict(source,binary['target']):
            result.update(target=hex(binary['target']),adopted=True,
                method='VT '+', '.join(sorted({m['method'] for m in matches})),
                second_check=f"BinDiff similarity={binary['similarity']:.6f}, confidence={binary['confidence']:.6f}",confidence='high')
        else:
            result['method'] = 'VT '+', '.join(f"{hex(m['target'])}:{m['method']}" for m in matches) if matches else 'VT no accepted pair'
            result['second_check'] = f"BinDiff {hex(binary['target'])} confidence={binary['confidence']:.6f}" if binary else 'BinDiff no function pair'
            missing = f"Resolve standard correlator agreement or add an independent code/RTTI/xref proof for {row['symbol']} at 2976 {value}"
    if row['r2'] in checks and not result['adopted']:
        check=checks[row['r2']]
        assert int(check['old'],0)==int(value,0) and check['symbol']==row['symbol']
        support=standard_support(int(value,0),int(check['target'],0))
        anchor=check.get('anchor')
        direct_conflict=standard_conflict(int(value,0),int(check['target'],0))
        if not direct_conflict and not support and anchor:
            support=[f'Native operand in anchor {hex(anchor[0])} -> {hex(anchor[1])}: '+s
                     for s in standard_support(*anchor)]
        if support:
            result.update(target=check['target'],adopted=True,method='; '.join(support),
                second_check=check['second_check'],evidence=check['evidence'],confidence='high')
        else:
            result.update(candidate=check['target'],independent_check=check['second_check'],evidence=check['evidence'])
            missing=(f"Resolve contradictory direct standard evidence for candidate {check['target']}; its anchor cannot override the conflict. " if direct_conflict else
                f"Standard support for candidate {check['target']} (or its documented native operand anchor) is absent; ")+f"independent check: {check['second_check']}"
    if row['r2'] in reviewed:
        manual = reviewed[row['r2']]
        assert row['kind'] not in ('function/global RVA','vtable RVA'),f"Manual RVA proof cannot bypass standard matching: {row['r2']}"
        assert manual['evidence'] and manual['second_check'] and manual['target'] is not None
        assert manual['old']==value and manual['symbol']==row['symbol'],row['r2']
        result.update(manual,adopted=True,confidence='high')
    result['missing'] = '' if result['adopted'] else missing
    results.append(result)
manifest = [dict(path=str(p.resolve()),bytes=p.stat().st_size,sha256=hashlib.file_digest(p.open('rb'),'sha256').hexdigest())
            for p in (args.vt,args.bindiff)]
payload = dict(standard_outputs=manifest,rows=results)
args.output_dir.mkdir(parents=True,exist_ok=True)
(args.output_dir/'R2-MATCHES.json').write_text(json.dumps(payload,indent=2)+'\n')
text = '# R2 matches — standard correlators and independent checks\n\n'
for item in manifest: text += f"- Local raw output `{item['path']}`; {item['bytes']} bytes; SHA-256 `{item['sha256']}`.\n"
text += '\nDual function adoption requires an unambiguous accepted VT function association plus BinDiff similarity and confidence >=0.8. A single standard match requires the separately recorded native check; a global may be derived from the verified operand of a standard-matched native consumer. Layouts and native payload contracts require separate code evidence. RTTI symbol seeds are not counted again as independent function proof. A matched RVA does not prove its hook signature or ABI.\n\n'
for tier in 'ABCD':
    selected = [r for r in results if r['tier']==tier]
    text += f"Tier {tier}: {sum(r['adopted'] for r in selected)} adopted, {sum(not r['adopted'] for r in selected)} blocked.\n\n"
text += '| Tier | Row / symbol | 3663 | Method | Second check | Confidence / missing fact |\n|---|---|---|---|---|---|\n'
for row in results:
    cells = [row['tier'],row['row']+' '+row['symbol'],str(row['target'] or 'unadopted'),row['method'],row['second_check'],row['confidence']+(': '+row['missing'] if row['missing'] else '')]
    text += '| '+' | '.join(s.replace('|','\\|').replace('\n',' ') for s in cells)+' |\n'
(args.output_dir/'R2-MATCHES.md').write_text(text)
print(Counter((r['tier'],r['adopted']) for r in results))
