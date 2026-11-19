"""File-only: per-offset use counts in old/new function pairs.
For each offset X, counts instructions in the old function whose memory
displacement or immediate equals X, and in the new function those equal to
X, X-0x10, X-8, X+8 and X+0x10. A field that moved shows its old count under
a shifted column. Prints the old instructions for context.
usage: fieldrefs.py <old.exe> <new.exe> <offsets comma-hex> <old_fn:new_fn>...
(function starts; lengths from .pdata)"""
import sys, struct, pefile, capstone
from capstone import x86

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[2], fast_load=True)
offs = [int(x, 16) for x in sys.argv[3].split(",")]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
SHIFTS = (-0x10, -8, 0, 8, 0x10)


def pdata(pe):
    d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
    raw = pe.get_data(d.VirtualAddress, d.Size)
    rows = {}
    for i in range(0, len(raw) - 11, 12):
        b, e, _ = struct.unpack_from("<III", raw, i)
        rows.setdefault(b, e)
    return rows


po, pn = pdata(old), pdata(new)


def values(pe, rva, end):
    out = []
    for ins in md.disasm(pe.get_data(rva, end - rva), rva):
        vals = set()
        for o in ins.operands:
            if o.type == x86.X86_OP_MEM and o.mem.base not in (0, x86.X86_REG_RIP, x86.X86_REG_RSP):
                vals.add(o.mem.disp)
            if o.type == x86.X86_OP_IMM and not (ins.mnemonic.startswith("j") or ins.mnemonic == "call"):
                vals.add(o.imm)
        out.append((ins, vals))
    return out


for arg in sys.argv[4:]:
    o, n = (int(x, 16) for x in arg.split(":"))
    if o not in po or n not in pn:
        print(f"== {o:#x} -> {n:#x}: not a .pdata function start ({o in po}, {n in pn})")
        continue
    a, b = values(old, o, po[o]), values(new, n, pn[n])
    print(f"== {o:#x} ({po[o] - o:#x} bytes) -> {n:#x} ({pn[n] - n:#x} bytes)")
    print("   offset   old | new " + " ".join(f"{s:+#x}".rjust(6) for s in SHIFTS))
    for x in offs:
        co = sum(1 for _, v in a if x in v)
        cn = [sum(1 for _, v in b if x + s in v) for s in SHIFTS]
        if co or any(cn):
            flag = "" if cn[2] == co else "   <-- differs"
            print(f"   {x:#7x} {co:5} | " + " ".join(f"{c:6}" for c in cn) + flag)
