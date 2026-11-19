"""File-only: every RIP-relative instruction in an image that resolves to a
target RVA, with its enclosing .pdata function. numpy candidate scan plus a
capstone decode at each candidate.
usage: refs.py <exe> <target-rva>... [--max N]"""
import sys, struct, bisect, collections
import numpy as np, pefile, capstone
from capstone import x86

args = sys.argv[1:]
limit = 40
if "--max" in args:
    i = args.index("--max")
    limit = int(args[i + 1])
    del args[i:i + 2]
pe = pefile.PE(args[0], fast_load=True)
targets = [int(a, 16) for a in args[1:]]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
raw = pe.get_data(d.VirtualAddress, d.Size)
starts = sorted({struct.unpack_from("<I", raw, i)[0] for i in range(0, len(raw) - 11, 12)})
want = np.array(sorted(targets), dtype=np.int64)
found = collections.defaultdict(list)
for s in pe.sections:
    if not s.Characteristics & 0x20000000:
        continue
    va, data = s.VirtualAddress, s.get_data()
    a = np.frombuffer(data, dtype=np.uint8)
    n = len(a) - 8
    disp = (a[:n].astype(np.int64) | (a[1:n + 1].astype(np.int64) << 8) |
            (a[2:n + 2].astype(np.int64) << 16) | (a[3:n + 3].astype(np.int64) << 24))
    disp = np.where(disp >= 1 << 31, disp - (1 << 32), disp)
    pos = np.arange(n, dtype=np.int64) + va
    for tail in (0, 1, 2, 4):
        tgt = pos + 4 + tail + disp
        for k in np.nonzero(np.isin(tgt, want))[0]:
            dr, t = int(pos[k]), int(tgt[k])
            for back in range(7, 0, -1):
                ins = next(md.disasm(pe.get_data(dr - back, 16), dr - back), None)
                if ins and ins.disp_offset == back and any(
                        o.type == x86.X86_OP_MEM and o.mem.base == x86.X86_REG_RIP for o in ins.operands) \
                        and ins.address + ins.size + struct.unpack_from("<i", ins.bytes, back)[0] == t:
                    i = bisect.bisect_right(starts, ins.address) - 1
                    found[t].append((ins.address, f"{ins.mnemonic} {ins.op_str}", starts[i] if i >= 0 else 0))
                    break
for t in targets:
    rows = sorted(set(found[t]))
    print(f"target {t:#x}: {len(rows)} references")
    for a, txt, f in rows[:limit]:
        print(f"   {a:#x}  {txt:<40} in {f:#x}")
