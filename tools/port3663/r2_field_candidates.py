"""Rank the shared AcquireField/EnterField caller. Candidates require standard corroboration."""
from collections import defaultdict
import json
import math
from pathlib import Path
from correlate import read

root = Path(__file__).resolve().parents[2]
old = read(root/'build/fingerprints-2976.tsv')
new = read(root/'build/fingerprints-3663.tsv')
old_hash,new_hash = defaultdict(list),defaultdict(list)
for address,row in old.items(): old_hash[row['fingerprint']].append(address)
for address,row in new.items(): new_hash[row['fingerprint']].append(address)
mapping = {v[0]:new_hash[k][0] for k,v in old_hash.items() if len(v)==1 and len(new_hash[k])==1 and int(old[v[0]]['instructions'])>=8}
def calls(row): return {int(s,16) for s in row['calls'].split(',') if s}
reverse = defaultdict(set)
for address,row in new.items():
    for callee in calls(row): reverse[callee].add(address)
result = []
for address in (0x2BBFA80,0x2B07790,0x2BC04F0,0x1408ECE0,0x14092400,0x941E20):
    source = old[address]
    mapped = {mapping[c] for c in calls(source) if c in mapping}
    candidates = set().union(*(reverse[c] for c in mapped))
    ranked = []
    for candidate in candidates:
        shared = mapped & calls(new[candidate])
        if len(shared)<2: continue
        score = sum(math.log((len(new)+1)/(len(reverse[c])+1)) for c in shared)
        ratio = min(int(source['instructions']),int(new[candidate]['instructions']))/max(int(source['instructions']),int(new[candidate]['instructions']))
        ranked.append(dict(target=hex(candidate),shared=len(shared),score=round(score*ratio,3),instructions=int(new[candidate]['instructions'])))
    ranked.sort(key=lambda r:r['score'],reverse=True)
    result.append(dict(source=hex(address),mapped_neighbours=[hex(s) for s in sorted(mapped)],candidates=ranked[:8],adopted=False))
(root/'build/r2/field-candidates.json').write_text(json.dumps(result,indent=2)+'\n')
for row in result: print(row['source'],row['candidates'][:4])
