"""Prioritize independent file-only checks; output remains candidates, not adoption."""
import csv
import json
from collections import defaultdict
from pathlib import Path

root = Path(__file__).resolve().parents[2]
def read(name):
    with (root/name).open() as stream:
        return {int(r['begin'],16):r for r in csv.DictReader((s for s in stream if not s.startswith('#')),delimiter='\t')}
old = read('build/fingerprints-2976.tsv')
new = read('build/fingerprints-3663.tsv')
by_hash = defaultdict(list)
for address,row in new.items(): by_hash[row['fingerprint']].append(address)
by_target = defaultdict(dict)
with (root/'build/r2/player-refs-old.tsv').open() as stream:
    for row in csv.DictReader((s for s in stream if not s.startswith('#')),delimiter='\t'):
        if not row.get('function','').startswith('0x'): continue
        address = int(row['function'],16)
        source = old.get(address)
        if not source: continue
        hits = by_hash[source['fingerprint']]
        by_target[row['target']][address] = dict(source=hex(address), target=[hex(x) for x in hits],
            instructions=int(source['instructions']), method='R1 custom fingerprint; needs standard match and code check')
result = {t:sorted(v.values(),key=lambda r:(len(r['target'])!=1,r['instructions'])) for t,v in by_target.items()}
(root/'build/r2/player-candidates.json').write_text(json.dumps(result,indent=2)+'\n')
for target, rows in result.items():
    print(target, 'functions',len(rows), 'short unique candidates',rows[:4])
