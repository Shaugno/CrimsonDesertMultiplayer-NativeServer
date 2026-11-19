"""File-only MSVC x64 RTTI check for vtables.
For each old vtable RVA: read vtable[-1] -> CompleteObjectLocator -> type
name and object offset. Find the new vtable(s) whose locator names the same
type with the same offset. With old:new pairs, confirm the pair instead.
usage: rtti.py <old.exe> <new.exe> <rva|old:new> [...]"""
import sys, struct, pefile

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[2], fast_load=True)


def u32(pe, rva):
    return struct.unpack("<I", pe.get_data(rva, 4))[0]


def u64(pe, rva):
    return struct.unpack("<Q", pe.get_data(rva, 8))[0]


def col_info(pe, vt):
    base = pe.OPTIONAL_HEADER.ImageBase
    col = u64(pe, vt - 8) - base
    if not 0 < col < pe.OPTIONAL_HEADER.SizeOfImage or u32(pe, col) != 1 or u32(pe, col + 20) != col:
        return None
    td = u32(pe, col + 12)
    name = pe.get_data(td + 16, 256).split(b"\0")[0].decode(errors="replace")
    return name, u32(pe, col + 4), col


def find_new(name, offset):
    base = new.OPTIONAL_HEADER.ImageBase
    needle = name.encode() + b"\0"
    out = []
    for s in new.sections:
        data = s.get_data()
        k = data.find(needle)
        while k >= 0:
            td = s.VirtualAddress + k - 16
            for t in new.sections:
                tdat = t.get_data()
                j = tdat.find(struct.pack("<I", td))
                while j >= 0:
                    col = t.VirtualAddress + j - 12
                    if col >= t.VirtualAddress and u32(new, col) == 1 and u32(new, col + 20) == col \
                            and u32(new, col + 4) == offset:
                        ref = struct.pack("<Q", base + col)
                        for v in new.sections:
                            vdat = v.get_data()
                            m = vdat.find(ref)
                            while m >= 0:
                                out.append(v.VirtualAddress + m + 8)
                                m = vdat.find(ref, m + 1)
                    j = tdat.find(struct.pack("<I", td), j + 1)
            k = data.find(needle, k + 1)
    return sorted(set(out))


for arg in sys.argv[3:]:
    o, _, n = arg.partition(":")
    o = int(o, 16)
    info = col_info(old, o)
    if not info:
        print(f"{o:#010x}\t-\tno RTTI locator at vtable[-1]")
        continue
    name, off, _ = info
    if n:
        n = int(n, 16)
        ni = col_info(new, n)
        same = ni is not None and ni[0] == name and ni[1] == off
        print(f"{o:#010x}\t{n:#010x}\t{'SAME' if same else 'DIFFERENT'}\t{name} +{off:#x}" +
              ("" if same else f" vs {ni}"))
    else:
        cands = find_new(name, off)
        print(f"{o:#010x}\t{','.join(f'{c:#010x}' for c in cands) or '-'}\t{len(cands)} found\t{name} +{off:#x}")
