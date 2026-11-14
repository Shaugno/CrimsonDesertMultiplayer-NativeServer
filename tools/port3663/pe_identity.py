"""Read PE identity from disk without loading or executing the image."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


class PE:
    def __init__(self, path):
        self.path = Path(path).resolve()
        self.data = self.path.read_bytes()
        self.sha256 = hashlib.sha256(self.data).hexdigest().upper()
        self.sections = []
        if self.data[:2] != b'MZ':
            raise ValueError('not a DOS image')
        pe = self.u32(0x3c)
        if self.data[pe:pe + 4] != b'PE\0\0' or self.u16(pe + 4) != 0x8664:
            raise ValueError('not an AMD64 PE')
        optional = pe + 24
        if self.u16(optional) != 0x20b or self.u16(pe + 20) < 144:
            raise ValueError('not a complete PE32+ optional header')
        self.timestamp = self.u32(pe + 8)
        self.image_base = self.u64(optional + 24)
        self.image_size = self.u32(optional + 56)
        section_at = optional + self.u16(pe + 20)
        for i in range(self.u16(pe + 6)):
            at = section_at + 40 * i
            name = self.data[at:at + 8].split(b'\0')[0].decode('ascii', errors='replace')
            virtual_size, rva, raw_size, raw = struct.unpack_from('<IIII', self.data, at + 8)
            if raw + raw_size > len(self.data) or rva + max(raw_size, virtual_size) > self.image_size:
                raise ValueError('section exceeds file/image bounds')
            self.sections.append(dict(name=name, rva=rva, raw=raw, size=raw_size,
                                      virtual_size=virtual_size, flags=self.u32(at + 36)))
        self.entries = set()
        self.ranges = []
        self.tls_index_rva = None
        if self.u32(optional + 108) > 9:
            tls_rva, tls_size = struct.unpack_from('<II', self.data, optional + 112 + 9 * 8)
            if tls_rva and tls_size >= 40:
                self.tls_index_rva = self.u64(self.offset(tls_rva, 40) + 16) - self.image_base
                # The loader writes this index; it can legitimately live in BSS.
                if not any(s['rva'] <= self.tls_index_rva and self.tls_index_rva + 4 <=
                           s['rva'] + max(s['size'], s['virtual_size']) for s in self.sections):
                    raise ValueError('TLS index lies outside mapped sections')
        if self.u32(optional + 108) > 3:
            rva, size = struct.unpack_from('<II', self.data, optional + 112 + 3 * 8)
            if size % 12:
                raise ValueError('invalid exception-directory size')
            if size:
                start = self.offset(rva, size)
                for at in range(start, start + size, 12):
                    begin, end, _ = struct.unpack_from('<III', self.data, at)
                    if not 0 < begin < end <= self.image_size:
                        raise ValueError('invalid runtime function range')
                    self.entries.add(begin)
                    self.ranges.append((begin, end))

    def u16(self, at):
        return struct.unpack_from('<H', self.data, at)[0]

    def u32(self, at):
        return struct.unpack_from('<I', self.data, at)[0]

    def u64(self, at):
        return struct.unpack_from('<Q', self.data, at)[0]

    def offset(self, rva, size=1):
        hits = [s['raw'] + rva - s['rva'] for s in self.sections
                if s['rva'] <= rva and rva + size <= s['rva'] + s['size']]
        if len(hits) != 1:
            raise ValueError(f'RVA 0x{rva:X} is not uniquely file-backed')
        return hits[0]

    def find_code(self, signature):
        hits = []
        for section in self.sections:
            if not section['flags'] & 0x20000000:
                continue
            start = section['raw']
            end = start + section['size']
            at = self.data.find(signature, start, end)
            while at >= 0:
                hits.append(at - start + section['rva'])
                at = self.data.find(signature, at + 1, end)
        return hits

    def executable(self, rva):
        return any(s['flags'] & 0x20000000 and s['rva'] <= rva < s['rva'] + s['size'] for s in self.sections)

    def identity(self):
        return dict(path=str(self.path), sha256=self.sha256, file_size=len(self.data),
                    timestamp=f'0x{self.timestamp:08X}', image_base=f'0x{self.image_base:X}',
                    image_size=f'0x{self.image_size:X}', pdata_entries=len(self.entries),
                    tls_index_rva=f'0x{self.tls_index_rva:X}' if self.tls_index_rva is not None else None,
                    sections=self.sections)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exe', type=Path)
    args = parser.parse_args()
    print(json.dumps(PE(args.exe).identity(), indent=2))
