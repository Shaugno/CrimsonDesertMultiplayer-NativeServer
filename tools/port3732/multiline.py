"""List byte arrays that span several source lines and check them in both images.
Each array must occur exactly once in the old image (or at a given owner);
it is then compared at the mapped new address with address fields masked.
usage: multiline.py <old.exe> <new.exe> <code-map.tsv> <file>...
Report only; never edits."""
import sys, re, bisect, struct, pefile, capstone
from capstone import x86

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[2], fast_load=True)
mapping = {}
for line in open(sys.argv[3], encoding="utf-8-sig"):
    c = line.rstrip("\n").split("\t")
    if len(c) >= 2 and c[1] not in ("-", ""):
        mapping[int(c[0], 16)] = int(c[1], 16)
code_sorted = sorted(mapping)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
ARR = re.compile(r"(\w+)\s*(?:\[\])?\s*(?:=\s*)?\{((?:\s*0x[0-9A-Fa-f]{1,2}\s*,?)+)\}", re.S)
d3 = old.OPTIONAL_HEADER.DATA_DIRECTORY[3]
raw = old.get_data(d3.VirtualAddress, d3.Size)
pdata = sorted({struct.unpack_from("<I", raw, i)[0] for i in range(0, len(raw) - 11, 12)})
execs = [(s.VirtualAddress, s.get_data()) for s in old.sections if s.Characteristics & 0x20000000]


def masks(pe, rva, n):
    m = set()
    for ins in md.disasm(pe.get_data(rva, n + 16), rva):
        if ins.address - rva >= n:
            break
        b = ins.address - rva
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        for o in ins.operands:
            if o.type == x86.X86_OP_MEM and o.mem.base == x86.X86_REG_RIP:
                m.update(range(b + ins.disp_offset, b + ins.disp_offset + ins.disp_size))
            if o.type == x86.X86_OP_IMM and rel and ins.imm_size:
                m.update(range(b + ins.imm_offset, b + ins.imm_offset + ins.imm_size))
    return m


for path in sys.argv[4:]:
    text = open(path, encoding="utf-8").read()
    for m in ARR.finditer(text):
        if "\n" not in m.group(2):
            continue
        data = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{1,2})", m.group(2)))
        line = text.count("\n", 0, m.start()) + 1
        hits = []
        for va, sec in execs:
            k = sec.find(data)
            while k >= 0 and len(hits) < 3:
                hits.append(va + k)
                k = sec.find(data, k + 1)
        if len(hits) != 1:
            print(f"{path}:{line}\t{m.group(1)}\t{len(data)} bytes\t{len(hits)} exact occurrences in 3663")
            continue
        o = hits[0]
        i = bisect.bisect_right(pdata, o) - 1
        j = bisect.bisect_right(code_sorted, o) - 1
        n_rva = None
        for a in [x for x in (code_sorted[j] if j >= 0 else None,
                              code_sorted[j + 1] if j + 1 < len(code_sorted) else None) if x is not None]:
            if a == o or bisect.bisect_right(pdata, a) - 1 == i:
                n_rva = mapping[a] + (o - a)
                break
        if n_rva is None:
            print(f"{path}:{line}\t{m.group(1)}\t{len(data)} bytes at {o:#x}\tno mapped address in its function")
            continue
        nb = new.get_data(n_rva, len(data))
        mo, mn = masks(old, o, len(data)), masks(new, n_rva, len(data))
        bad = [k for k in range(len(data)) if k not in mo and k not in mn and data[k] != nb[k]]
        status = "REVIEW " + str(bad) if bad else ("unchanged" if nb == data else "address fields differ")
        print(f"{path}:{line}\t{m.group(1)}\t{len(data)} bytes {o:#x}->{n_rva:#x}\t{status}")
