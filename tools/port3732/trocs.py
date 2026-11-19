"""File-only: packet descriptor table (TrocTr<Name> instances) per build.
Each instance: +0 vtable (RTTI .?AVTrocTr<Name>@pa@@), +8 kind u32, +0xC opcode u16.
Scans [begin, end) at 8-byte steps in each image and compares opcodes by name.
usage: trocs.py <old.exe> <old_begin:old_end> <new.exe> <new_begin:new_end> [names...]"""
import sys, struct, pefile


def table(path, span):
    pe = pefile.PE(path, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    size = pe.OPTIONAL_HEADER.SizeOfImage
    lo, hi = (int(x, 16) for x in span.split(":"))
    data = pe.get_data(lo, hi - lo)
    out, names = {}, {}

    def name_of(vt):
        if vt in names:
            return names[vt]
        n = None
        try:
            col = struct.unpack("<Q", pe.get_data(vt - 8, 8))[0] - base
            if 0 < col < size and struct.unpack("<I", pe.get_data(col, 4))[0] == 1:
                td = struct.unpack("<I", pe.get_data(col + 12, 4))[0]
                s = pe.get_data(td + 16, 160).split(b"\0")[0].decode(errors="replace")
                if s.startswith(".?AVTrocTr"):
                    n = s[len(".?AVTrocTr"):].split("@")[0]
        except Exception:
            n = None
        names[vt] = n
        return n

    for k in range(0, len(data) - 16, 8):
        vt = struct.unpack_from("<Q", data, k)[0] - base
        if not 0 < vt < size:
            continue
        n = name_of(vt)
        if n is None:
            continue
        kind, opcode = struct.unpack_from("<IH", data, k + 8)
        out.setdefault(n, (opcode, kind, lo + k))
    return out


a = table(sys.argv[1], sys.argv[2])
b = table(sys.argv[3], sys.argv[4])
wanted = sys.argv[5:]
same = sum(1 for n in a if n in b and a[n][0] == b[n][0])
changed = sorted((n for n in a if n in b and a[n][0] != b[n][0]), key=lambda n: a[n][0])
print(f"descriptors old {len(a)} new {len(b)} common {len(a.keys() & b.keys())} same-opcode {same} "
      f"changed {len(changed)} only-old {len(a.keys() - b.keys())} only-new {len(b.keys() - a.keys())}")
for n in wanted:
    print(f"  {n:<40} old {a.get(n, ('-',))[0]!s:>5}  new {b.get(n, ('-',))[0]!s:>5}")
if changed:
    print("first changed:")
    for n in changed[:15]:
        print(f"  {n:<40} {a[n][0]} -> {b[n][0]}")
