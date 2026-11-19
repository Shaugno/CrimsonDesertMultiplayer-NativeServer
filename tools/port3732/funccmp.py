"""File-only: compare whole function bodies between builds.
For each old:new pair, take the old function containing the old RVA (pdata
begin/end), the new function at the same offset, decode both linearly and
compare instruction by instruction with rel32 branch targets and RIP
displacements masked. Identical bodies mean every struct displacement and
immediate the function uses is unchanged.
usage: funccmp.py <old.exe> <new.exe> <map.tsv>   (rows: old<TAB>new<TAB>...)"""
import sys, bisect, struct, pefile, capstone
from capstone import x86

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[2], fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True


def pdata(pe):
    d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
    raw = pe.get_data(d.VirtualAddress, d.Size)
    rows = {}
    for i in range(0, len(raw) - 11, 12):
        b, e, _ = struct.unpack_from("<III", raw, i)
        rows.setdefault(b, e)
    return sorted(rows.items())


op, np_ = pdata(old), pdata(new)
ob = [b for b, _ in op]
nb = [b for b, _ in np_]


def body(pe, begin, end):
    out = []
    for ins in md.disasm(pe.get_data(begin, end - begin), begin):
        b = bytearray(ins.bytes)
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        for o in ins.operands:
            if o.type == x86.X86_OP_MEM and o.mem.base == x86.X86_REG_RIP:
                b[ins.disp_offset:ins.disp_offset + ins.disp_size] = b"\0" * ins.disp_size
            if o.type == x86.X86_OP_IMM and rel and ins.imm_size:
                b[ins.imm_offset:ins.imm_offset + ins.imm_size] = b"\0" * ins.imm_size
        out.append((ins.address - begin, bytes(b), f"{ins.mnemonic} {ins.op_str}"))
    return out


seen = set()
for line in open(sys.argv[3], encoding="utf-8-sig"):
    c = line.rstrip("\n").split("\t")
    if len(c) < 2 or c[1] == "-":
        continue
    o, n = int(c[0], 16), int(c[1], 16)
    i = bisect.bisect_right(ob, o) - 1
    if i < 0 or not ob[i] <= o < op[i][1]:
        print(f"{o:#010x}\t{n:#010x}\tno-pdata\tnot inside an old pdata function")
        continue
    fb, fe = op[i]
    if fb in seen:
        continue
    seen.add(fb)
    nfb = n - (o - fb)
    j = bisect.bisect_left(nb, nfb)
    if j >= len(nb) or nb[j] != nfb:
        print(f"{o:#010x}\t{n:#010x}\tno-new-pdata\tfunction {fb:#x} -> {nfb:#x} has no pdata entry")
        continue
    nfe = np_[j][1]
    a, b = body(old, fb, fe), body(new, nfb, nfe)
    diffs = [(x, y) for x, y in zip(a, b) if x[0] != y[0] or x[1] != y[1]]
    if len(a) == len(b) and not diffs and fe - fb == nfe - nfb:
        print(f"{o:#010x}\t{n:#010x}\tidentical\tfunction {fb:#x} -> {nfb:#x}, {len(a)} instructions")
    else:
        first = diffs[0] if diffs else None
        where = (f"first difference at +{first[0][0]:#x}: '{first[0][2]}' vs '{first[1][2]}'"
                 if first else f"length {fe - fb:#x} vs {nfe - nfb:#x}")
        print(f"{o:#010x}\t{n:#010x}\tDIFFERENT\tfunction {fb:#x} -> {nfb:#x}: {len(diffs)} differing "
              f"instructions of {len(a)}/{len(b)}; {where}")
