"""Read file descriptors identified by RTTI initializer store candidates.

Initializer instruction boundaries and handler semantics require Ghidra review.
This scanner never treats a raw opcode or descriptor min/max as a wire schema.
"""
import argparse
import json
from pathlib import Path
import struct
from pe_identity import PE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=Path)
    parser.add_argument('--rtti', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    image = PE(args.exe)
    rtti = json.loads(args.rtti.read_text())
    if rtti['identity']['sha256'] != image.sha256:
        raise SystemExit('RTTI evidence/executable hash mismatch')
    rows = []
    for typ in rtti['types']:
        if not typ['name'].startswith('.?AVTrocTr'):
            continue
        for table in typ['vtables']:
            for ref in table['code_lea_candidates']:
                lea = int(ref['rva'], 16)
                for delta in range(7, 33):
                    try:
                        at = image.offset(lea + delta, 7)
                        if image.data[at:at + 3] != b'\x48\x89\x05':
                            continue
                        dest = lea + delta + 7 + struct.unpack_from('<i', image.data, at + 3)[0]
                        data = image.offset(dest, 28)
                    except (ValueError, struct.error):
                        continue
                    row = dict(name=typ['name'], vtable=table['rva'], initializer=ref['pdata_owner'],
                               store=f'0x{lea + delta:X}', instance=f'0x{dest:X}',
                               kind=image.u32(data + 8), opcode=image.u16(data + 12),
                               flags=f'0x{image.u32(data + 20):X}',
                               descriptor_min=image.u16(data + 24), descriptor_max=image.u16(data + 26),
                               adopted=False, missing='Handler consumption and opcode identity cross-check')
                    rows.append(row)
    args.out.write_text(json.dumps(dict(identity=image.identity(), descriptors=rows, adopted=0), indent=2) + '\n')
    for row in rows:
        print(f'{row["name"]}: kind={row["kind"]} op={row["opcode"]} instance={row["instance"]} (unadopted)')


if __name__ == '__main__':
    main()
