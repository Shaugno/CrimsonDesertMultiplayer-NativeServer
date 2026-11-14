"""Rank unmatched core functions by already-correlated direct-call neighbours.

All output is weak. Shared allocators/container helpers are not semantic proof.
"""
import argparse
from collections import Counter, defaultdict
import json
import math
from pathlib import Path
from correlate import read


def calls(row):
    return {int(x, 16) for x in row['calls'].split(',') if x}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--old-functions', type=Path, required=True)
    parser.add_argument('--new-functions', type=Path, required=True)
    parser.add_argument('--correlations', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    old, new = read(args.old_functions), read(args.new_functions)
    old_hash, new_hash = defaultdict(list), defaultdict(list)
    for rva, row in old.items(): old_hash[row['fingerprint']].append(rva)
    for rva, row in new.items(): new_hash[row['fingerprint']].append(rva)
    pairs = {v[0]: new_hash[k][0] for k, v in old_hash.items()
             if len(v) == 1 and len(new_hash[k]) == 1 and int(old[v[0]]['instructions']) >= 4}
    reverse = defaultdict(set)
    for rva, row in new.items():
        for callee in calls(row): reverse[callee].add(rva)
    rows = []
    for selected in json.loads(args.correlations.read_text())['selected']:
        rva = int(selected['old_rva'], 16)
        if selected['fingerprint_target'] is not None or rva not in old:
            continue
        neighbours = {pairs[x] for x in calls(old[rva]) if x in pairs}
        counts = Counter(candidate for callee in neighbours for candidate in reverse[callee])
        ranked = []
        for candidate, overlap in counts.items():
            if overlap < 2:
                continue
            shared = neighbours & calls(new[candidate])
            score = sum(math.log((len(new) + 1) / (len(reverse[x]) + 1)) for x in shared)
            ratio = min(int(old[rva]['instructions']), int(new[candidate]['instructions'])) / max(int(old[rva]['instructions']), int(new[candidate]['instructions']))
            ranked.append(dict(rva=f'0x{candidate:X}', shared_calls=len(shared), score=round(score * ratio, 3),
                               instruction_count=int(new[candidate]['instructions'])))
        ranked.sort(key=lambda x: x['score'], reverse=True)
        rows.append(dict(source=selected['source'], symbol=selected['symbol'], old_rva=selected['old_rva'],
                         mapped_callees=[f'0x{x:X}' for x in sorted(neighbours)], candidates=ranked[:5],
                         confidence='weak', adopted=False))
    args.out.write_text(json.dumps(dict(rows=rows, adopted=0), indent=2) + '\n')
    for row in rows:
        if row['symbol'] in {'kAcquireField', 'kEnterField', 'kLoad', 'kPhases', 'kLobbyLoad', 'kServerOnRecvPacket'}:
            print(row['symbol'], row['old_rva'], row['candidates'])


if __name__ == '__main__':
    main()
