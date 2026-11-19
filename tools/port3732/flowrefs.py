"""File-only recursive-descent walk from a function entry (follows jmp/jcc,
does not enter calls) collecting memory displacements and immediates that
equal any given offset. Handles protected code split into jump-linked
chunks. Compare the per-offset counts between builds.
usage: flowrefs.py <old.exe> <new.exe> <offsets comma-hex> <old:new>... [--max N]"""
import sys, collections, pefile, capstone
from capstone import x86

args = sys.argv[1:]
limit = 20000
if "--max" in args:
    i = args.index("--max")
    limit = int(args[i + 1])
    del args[i:i + 2]
old = pefile.PE(args[0], fast_load=True)
new = pefile.PE(args[1], fast_load=True)
offs = [int(x, 16) for x in args[2].split(",")]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
SHIFTS = (-0x10, -8, 0, 8, 0x10)


def walk(pe, entry):
    seen, work, vals, n = set(), [entry], collections.Counter(), 0
    examples = collections.defaultdict(list)
    while work and n < limit:
        pc = work.pop()
        while pc not in seen and n < limit:
            data = pe.get_data(pc, 16)
            ins = next(md.disasm(data, pc), None)
            if ins is None:
                break
            seen.add(pc)
            n += 1
            for o in ins.operands:
                v = None
                if o.type == x86.X86_OP_MEM and o.mem.base not in (0, x86.X86_REG_RIP, x86.X86_REG_RSP):
                    v = o.mem.disp
                elif o.type == x86.X86_OP_IMM and not (ins.mnemonic.startswith("j") or ins.mnemonic == "call"):
                    v = o.imm
                if v is not None:
                    vals[v] += 1
                    if len(examples[v]) < 3:
                        examples[v].append(f"{ins.address:#x} {ins.mnemonic} {ins.op_str}")
            m = ins.mnemonic
            if m in ("ret", "int3", "ud2"):
                break
            if m.startswith("j"):
                op = ins.operands[0] if ins.operands else None
                if op is not None and op.type == x86.X86_OP_IMM:
                    if m == "jmp":
                        pc = op.imm
                        continue
                    work.append(op.imm)
                elif m == "jmp":
                    break
            pc = ins.address + ins.size
    return vals, examples, n


for arg in args[3:]:
    o, nn = (int(x, 16) for x in arg.split(":"))
    va, ea, na = walk(old, o)
    vb, eb, nb = walk(new, nn)
    print(f"== {o:#x} ({na} instructions) -> {nn:#x} ({nb} instructions)")
    print("   offset   old | new " + " ".join(f"{s:+#x}".rjust(6) for s in SHIFTS))
    for x in offs:
        cn = [vb[x + s] for s in SHIFTS]
        if va[x] or any(cn):
            flag = "" if cn[2] == va[x] else "   <-- differs"
            print(f"   {x:#7x} {va[x]:5} | " + " ".join(f"{c:6}" for c in cn) + flag)
            for e in ea[x][:2]:
                print(f"            old {e}")
