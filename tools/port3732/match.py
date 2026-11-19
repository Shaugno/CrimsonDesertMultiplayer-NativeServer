"""File-only: locate 3663 code RVAs in 3732. Never executes either image.

Pass 1: masked byte signature (rel32 branch targets and RIP-relative
displacements masked) searched near the old RVA in executable sections.
Pass 2: ambiguous hits are resolved only when exactly one hit equals the
local delta of the nearest unique pass-1 anchors (same section, both sides
agree). Pass 3: a jmp/call thunk is resolved by matching its target, then
finding the unique new thunk branching to the new target.

usage: match.py <old.exe> <new.exe> <rva> [<rva> ...]
output: old  new  method  detail     (new = '-' when unresolved)"""
import sys, re, bisect, struct, pefile, capstone
from capstone import x86

old_path, new_path, rvas = sys.argv[1], sys.argv[2], [int(a, 16) for a in sys.argv[3:]]
old = pefile.PE(old_path, fast_load=True)
new = pefile.PE(new_path, fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True


def exec_secs(pe):
    return [(s.VirtualAddress, s.VirtualAddress + s.Misc_VirtualSize) for s in pe.sections
            if s.Characteristics & 0x20000000]


old_x = exec_secs(old)
new_secs = [(s.VirtualAddress, s.get_data()) for s in new.sections if s.Characteristics & 0x20000000]


def sec_index(rva, secs):
    for i, (lo, hi) in enumerate(secs):
        if lo <= rva < hi:
            return i
    return -1


def first_ins(pe, rva):
    for ins in md.disasm(pe.get_data(rva, 16), rva):
        return ins
    return None


def signature(rva, max_bytes=48):
    pat, n = [], 0
    for ins in md.disasm(old.get_data(rva, 256), rva):
        b = list(ins.bytes)
        mask = [True] * len(b)
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        for op in ins.operands:
            if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP:
                mask[ins.disp_offset:ins.disp_offset + ins.disp_size] = [False] * ins.disp_size
            if op.type == x86.X86_OP_IMM and rel and ins.imm_size:
                mask[ins.imm_offset:ins.imm_offset + ins.imm_size] = [False] * ins.imm_size
        pat += [(x if m else None) for x, m in zip(b, mask)]
        n += len(b)
        if n >= max_bytes or ins.mnemonic in ("ret", "jmp", "int3"):
            break
    return pat


def search(pat, center, radius):
    rx = re.compile(b"".join(re.escape(bytes([x])) if x is not None else b"." for x in pat), re.S)
    hits = []
    for va, data in new_secs:
        lo, hi = max(0, center - va - radius), min(len(data), center - va + radius)
        if lo < hi:
            hits += [va + m.start() for m in rx.finditer(data, lo, hi)]
    return hits


results, pending = {}, {}
for rva in rvas:
    if sec_index(rva, old_x) < 0:
        results[rva] = ("-", "skip", "not in an executable section")
        continue
    pat = signature(rva)
    fixed = sum(1 for x in pat if x is not None)
    hits = search(pat, rva, 0x400000) if fixed >= 12 else []
    if fixed >= 12 and not hits:
        hits = search(pat, rva, 0x2000000)
    if len(hits) == 1:
        results[rva] = (hits[0], "sig", f"{fixed}/{len(pat)} fixed")
    else:
        pending[rva] = (hits, fixed, len(pat))

anchors = sorted((r, n) for r, (n, m, _) in results.items() if m == "sig")
akeys = [a[0] for a in anchors]


def local_delta(rva):
    i = bisect.bisect_left(akeys, rva)
    sec = sec_index(rva, old_x)
    left = next((anchors[j] for j in range(i - 1, -1, -1) if sec_index(anchors[j][0], old_x) == sec), None)
    right = next((anchors[j] for j in range(i, len(anchors)) if sec_index(anchors[j][0], old_x) == sec), None)
    ds = {a[1] - a[0] for a in (left, right) if a}
    return ds.pop() if len(ds) == 1 else None


for rva, (hits, fixed, total) in pending.items():
    d = local_delta(rva)
    ok = [h for h in hits if d is not None and h - rva == d]
    if len(ok) == 1:
        results[rva] = (ok[0], "delta", f"{len(hits)} sig hits, anchors agree on {d:+#x}")
        continue
    ins = first_ins(old, rva)
    if ins and ins.mnemonic in ("jmp", "call") and ins.operands and ins.operands[0].type == x86.X86_OP_IMM:
        target = ins.operands[0].imm
        tsig = signature(target)
        th = search(tsig, target, 0x400000)
        if len(th) == 1:
            nt = th[0]
            op = ins.bytes[0]
            cands = []
            for va, data in new_secs:
                lo, hi = max(0, rva - va - 0x400000), min(len(data) - 5, rva - va + 0x400000)
                start = lo
                while True:
                    k = data.find(bytes([op]), start, hi)
                    if k < 0:
                        break
                    if va + k + 5 + struct.unpack_from("<i", data, k + 1)[0] == nt:
                        cands.append(va + k)
                    start = k + 1
            if len(cands) == 1:
                results[rva] = (cands[0], "thunk", f"target {target:#x} -> {nt:#x}")
                continue
            results[rva] = ("-", "thunk?", f"target {target:#x} -> {nt:#x}, {len(cands)} thunks")
            continue
    near = ",".join(f"{h:#x}" for h in sorted(hits, key=lambda h: abs(h - rva))[:3])
    results[rva] = ("-", "unresolved", f"{len(hits)} hits, {fixed}/{total} fixed, local delta "
                    f"{('%+#x' % d) if d is not None else 'n/a'} {near}")

for rva in rvas:
    n, m, why = results[rva]
    print(f"{rva:#010x}\t{n if n == '-' else f'{n:#010x}'}\t{m}\t{why}")
