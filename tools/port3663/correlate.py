"""Join candidate function fingerprints and independent named RTTI slot pairs.

This produces a review queue, not an adopted address table. Fingerprints mask
relative addresses and may identify identical wrappers with different targets.
"""
import argparse
from collections import defaultdict
import csv
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]


def read(path):
    with path.open(encoding='utf-8-sig') as stream:
        return {int(r['begin'], 16): r for r in csv.DictReader((l for l in stream if not l.startswith('#')), delimiter='\t')}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--old-functions', type=Path, required=True)
    parser.add_argument('--new-functions', type=Path, required=True)
    parser.add_argument('--old-rtti', type=Path, required=True)
    parser.add_argument('--new-rtti', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    old, new = read(args.old_functions), read(args.new_functions)
    old_by_hash, new_by_hash = defaultdict(list), defaultdict(list)
    for rva, row in old.items():
        old_by_hash[row['fingerprint']].append(rva)
    for rva, row in new.items():
        new_by_hash[row['fingerprint']].append(rva)
    unique = {rvas[0]: new_by_hash[key][0] for key, rvas in old_by_hash.items()
              if len(rvas) == 1 and len(new_by_hash[key]) == 1 and int(old[rvas[0]]['instructions']) >= 4}
    old_types = json.loads(args.old_rtti.read_text())['types']
    new_types = {t['name']: t for t in json.loads(args.new_rtti.read_text())['types']}
    slots = defaultdict(list)
    for typ in old_types:
        other = new_types.get(typ['name'])
        if not other or len(typ['vtables']) != 1 or len(other['vtables']) != 1:
            continue
        for left, right in zip(typ['vtables'][0]['slots'], other['vtables'][0]['slots']):
            slots[int(left['rva'], 16)].append(dict(name=typ['name'], slot=left['index'], target=right['rva']))
    rows = []
    inventory = json.loads((ROOT / 'docs/probe-3663/PORT-INVENTORY.json').read_text())
    for rel in inventory['files']:
        text = (ROOT / rel).read_text(encoding='utf-8')
        for m in re.finditer(r'constexpr\s+uintptr_t\s+(k\w+)\s*=\s*(0x[0-9a-fA-F]+)', text):
            old_rva = int(m[2], 16)
            if old_rva not in old and old_rva not in slots:
                continue
            dest = unique.get(old_rva)
            checks = slots.get(old_rva, [])
            rows.append(dict(source=f'{rel}:{text.count(chr(10), 0, m.start()) + 1}', symbol=m[1], old_rva=m[2],
                             fingerprint_target=f'0x{dest:X}' if dest is not None else None,
                             old_instruction_count=int(old[old_rva]['instructions']) if old_rva in old else None,
                             rtti_slot_candidates=checks, independent_slot_agreement=bool(dest is not None and any(int(c['target'], 16) == dest for c in checks)),
                             old_calls=old[old_rva]['calls'] if old_rva in old else None,
                             new_calls=new[dest]['calls'] if dest is not None else None,
                             confidence='weak; semantic audit pending', adopted=False))
    args.out.write_text(json.dumps(dict(unique_normalized_functions=len(unique), selected=rows, adopted=0), indent=2) + '\n')
    print(f'{len(unique)} unique normalized candidates, {len(rows)} selected header functions; adopted=0')
    for row in rows:
        if row['fingerprint_target'] is None and not row['rtti_slot_candidates']:
            continue
        print(f'{row["symbol"]}: {row["old_rva"]} -> {row["fingerprint_target"]}, RTTI agreement={row["independent_slot_agreement"]}')


if __name__ == '__main__':
    main()
