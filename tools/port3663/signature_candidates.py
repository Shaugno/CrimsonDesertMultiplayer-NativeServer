"""Compare existing short 2976 signatures to a verified reference and target.

Prologue uniqueness and pdata entry checks are candidate discovery only.
Every result stays weak/unadopted until an independent semantic check exists.
No game bytes are exported, and neither image is executed.
"""
import argparse
import json
from pathlib import Path
import re
from pe_identity import PE

ROOT = Path(__file__).resolve().parents[2]
REFERENCE_HASH = '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path)
    parser.add_argument('--target', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    target = PE(args.target)
    if (target.timestamp, target.image_base, target.image_size) != (0x6AF5D198, 0x140000000, 0x144E9000):
        raise SystemExit('Target build drift: identity differs from 3663; no scan performed')
    reference = PE(args.reference) if args.reference else None
    if reference and reference.sha256 != REFERENCE_HASH:
        raise SystemExit('2976 reference SHA-256 mismatch; no comparison performed')
    results = []
    for rel in ('src/mod/game/offsets.h', 'src/mod/session2/offsets_s2.h'):
        source = (ROOT / rel).read_text(encoding='utf-8')
        source = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', lambda m: '\n' * m[0].count('\n'), source)
        for match in re.finditer(r'constexpr\s+uint8_t\s+(\w+)\[\]\s*=\s*\{([^}]+)\}', source):
            signature = bytes(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]+)', match[2]))
            if not signature:
                continue
            hits = target.find_code(signature)
            old_hits = reference.find_code(signature) if reference else []
            results.append(dict(source=f'{rel}:{source.count(chr(10), 0, match.start()) + 1}',
                                symbol=match[1], length=len(signature),
                                reference_occurrences=len(old_hits) if reference else None,
                                reference_entries=[f'0x{x:X}' for x in old_hits if x in reference.entries] if reference else [],
                                target_occurrences=len(hits),
                                target_entries=[f'0x{x:X}' for x in hits if x in target.entries],
                                confidence='weak', adopted=False,
                                missing='Independent semantic/call-graph/RTTI correspondence and 3663 layout/ABI validation'))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(dict(reference=reference.identity() if reference else None,
                                        target=target.identity(), adopted=0, signatures=results), indent=2) + '\n', encoding='utf-8')
    print(f'{len(results)} existing signatures scanned; {sum(r["target_occurrences"] == 1 for r in results)} unique; adopted=0')


if __name__ == '__main__':
    main()
