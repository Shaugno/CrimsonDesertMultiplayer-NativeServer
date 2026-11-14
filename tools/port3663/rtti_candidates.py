"""File-only MSVC RTTI/COL/vtable candidate discovery; never adopts a layout."""
import argparse
import json
from pathlib import Path
import re
import struct
from bisect import bisect_right
from pe_identity import PE


def find_all(data, pattern, start, end):
    at = data.find(pattern, start, end)
    while at >= 0:
        yield at
        at = data.find(pattern, at + 1, end)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=Path)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--names', default='NwVirtual|ActorManager|CharacterControlActorComponent|ClientChildOnlyInGameActor|ServerUserActor|ToNear|ToWorld|ToField|WorldSystem')
    args = parser.parse_args()
    image = PE(args.exe)
    wanted = re.compile(args.names)
    types = {}
    # Match complete mangled names only in data-bearing sections.
    sections = [s for s in image.sections if not s['flags'] & 0x20000000 or s['flags'] & 0x80000000]
    for section in sections:
        for match in re.finditer(rb'\.\?A[UV][^\x00\r\n]{1,250}\x00', image.data[section['raw']:section['raw'] + section['size']]):
            name = match[0][:-1].decode('ascii', errors='replace')
            if wanted.search(name):
                td = section['rva'] + match.start() - 16
                if td >= section['rva']:
                    types[td] = dict(name=name, descriptor=f'0x{td:X}', locators=[], vtables=[])
    locators = {}
    for section in sections:
        for at in find_all(image.data, b'\x01\0\0\0', section['raw'], section['raw'] + section['size'] - 23):
            rva = at - section['raw'] + section['rva']
            if rva % 4:
                continue
            sig, offset, cd_offset, td, hierarchy, self_rva = struct.unpack_from('<IIIIII', image.data, at)
            if self_rva == rva and td in types:
                try:
                    image.offset(hierarchy, 16)
                except ValueError:
                    continue
                locators[rva] = td
                types[td]['locators'].append(dict(rva=f'0x{rva:X}', this_offset=offset, ctor_offset=cd_offset))
    for locator, td in locators.items():
        for section in sections:
            for at in find_all(image.data, struct.pack('<Q', image.image_base + locator), section['raw'], section['raw'] + section['size']):
                rva = at - section['raw'] + section['rva'] + 8
                if rva % 8:
                    continue
                slots = []
                for i in range(32):
                    try:
                        target = image.u64(image.offset(rva + i * 8, 8)) - image.image_base
                        image.offset(target)
                    except (ValueError, struct.error):
                        break
                    if not image.executable(target):
                        break
                    slots.append(dict(index=i, rva=f'0x{target:X}', pdata_entry=target in image.entries))
                if slots:
                    types[td]['vtables'].append(dict(rva=f'0x{rva:X}', locator=f'0x{locator:X}', slots=slots))
    tables = {int(v['rva'], 16): v for t in types.values() for v in t['vtables']}
    ranges = sorted(image.ranges)
    starts = [r[0] for r in ranges]
    for table in tables.values():
        table['code_lea_candidates'] = []
    for section in image.sections:
        if not section['flags'] & 0x20000000:
            continue
        for at in find_all(image.data, b'\x48\x8d', section['raw'], section['raw'] + section['size'] - 6):
            if image.data[at + 2] not in (0x05, 0x0d, 0x15, 0x1d, 0x25, 0x2d, 0x35, 0x3d):
                continue
            rva = at - section['raw'] + section['rva']
            dest = rva + 7 + struct.unpack_from('<i', image.data, at + 3)[0]
            if dest in tables:
                index = bisect_right(starts, rva) - 1
                owner = ranges[index][0] if index >= 0 and rva < ranges[index][1] else None
                tables[dest]['code_lea_candidates'].append(dict(rva=f'0x{rva:X}',
                    pdata_owner=f'0x{owner:X}' if owner is not None else None,
                    verified_instruction_boundary=False))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(dict(identity=image.identity(), adopted=0, types=list(types.values())), indent=2) + '\n', encoding='utf-8')
    print(f'{len(types)} named RTTI candidates; {sum(len(t["vtables"]) for t in types.values())} tables; adopted=0')


if __name__ == '__main__':
    main()
