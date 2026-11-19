"""File-only: map 3663 data RVAs (vtables, globals) to 3732 through the
RIP-relative instructions that reference them. Never executes either image.

1. In the old image, find every instruction whose RIP-relative operand
   resolves exactly to the wanted address (numpy candidate scan, then a
   capstone decode confirms the instruction and its displacement offset).
2. Map the instruction into the new image: enclosing function (pdata)
   located by unique masked signature, same offset, masked bytes equal.
3. Decode the new instruction and take its RIP target. Report the votes.

usage: dataref.py <old.exe> <new.exe> <rva> [<rva> ...]
output: old  new  votes/references  (new '-' if no agreement)"""
import sys, re, bisect, struct, collections
import numpy as np, pefile, capstone
from capstone import x86

old_path, new_path, wanted = sys.argv[1], sys.argv[2], [int(a, 16) for a in sys.argv[3:]]
old = pefile.PE(old_path, fast_load=True)
new = pefile.PE(new_path, fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
MAX_REFS = 12


def exec_secs(pe):
    return [(s.VirtualAddress, s.get_data()) for s in pe.sections if s.Characteristics & 0x20000000]


old_x, new_x = exec_secs(old), exec_secs(new)


def pdata(pe):
    d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
    raw = pe.get_data(d.VirtualAddress, d.Size)
    return sorted({struct.unpack_from("<I", raw, i)[0] for i in range(0, len(raw) - 11, 12)})


old_funcs = pdata(old)


def rip_target(ins):
    for op in ins.operands:
        if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP and op.mem.index == 0:
            return ins.address + ins.size + op.mem.disp
    return None


def decode_at(pe, rva):
    for ins in md.disasm(pe.get_data(rva, 16), rva):
        return ins
    return None


def old_refs(targets):
    out = collections.defaultdict(list)
    want = np.array(sorted(targets), dtype=np.int64)
    for va, data in old_x:
        a = np.frombuffer(data, dtype=np.uint8)
        n = len(a) - 8
        d = (a[:n].astype(np.int64) | (a[1:n + 1].astype(np.int64) << 8) |
             (a[2:n + 2].astype(np.int64) << 16) | (a[3:n + 3].astype(np.int64) << 24))
        d = np.where(d >= 1 << 31, d - (1 << 32), d)
        pos = np.arange(n, dtype=np.int64) + va
        for tail in (0, 1, 2, 4):
            tgt = pos + 4 + tail + d
            hit = np.nonzero(np.isin(tgt, want))[0]
            for k in hit:
                disp_rva, t = int(pos[k]), int(tgt[k])
                for back in range(1, 8):
                    ins = decode_at(old, disp_rva - back)
                    if ins and ins.disp_offset == back and rip_target(ins) == t:
                        out[t].append(ins.address)
                        break
    return out


def signature(pe, rva, max_bytes=48):
    pat, n = [], 0
    for ins in md.disasm(pe.get_data(rva, 256), rva):
        b, mask = list(ins.bytes), [True] * ins.size
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        for op in ins.operands:
            if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP:
                mask[ins.disp_offset:ins.disp_offset + ins.disp_size] = [False] * ins.disp_size
            if op.type == x86.X86_OP_IMM and rel and ins.imm_size:
                mask[ins.imm_offset:ins.imm_offset + ins.imm_size] = [False] * ins.imm_size
        pat += [(x if m else None) for x, m in zip(b, mask)]
        n += ins.size
        if n >= max_bytes or ins.mnemonic in ("ret", "jmp", "int3"):
            break
    return pat


def search(pat, center, radius):
    rx = re.compile(b"".join(re.escape(bytes([x])) if x is not None else b"." for x in pat), re.S)
    hits = []
    for va, data in new_x:
        lo, hi = max(0, center - va - radius), min(len(data), center - va + radius)
        if lo < hi:
            hits += [va + m.start() for m in rx.finditer(data, lo, hi)]
    return hits


func_cache = {}


def locate(f):
    if f not in func_cache:
        pat = signature(old, f)
        hits = []
        if sum(1 for x in pat if x is not None) >= 16:
            hits = search(pat, f, 0x400000) or search(pat, f, 0x2000000)
        func_cache[f] = hits[0] if len(hits) == 1 else None
    return func_cache[f]


def masked_equal(pat, rva):
    data = new.get_data(rva, len(pat))
    return len(data) == len(pat) and all(x is None or x == y for x, y in zip(pat, data))


refs = old_refs(wanted)
for t in wanted:
    sites = refs.get(t, [])
    votes = collections.Counter()
    tried = 0
    for s in sites[:MAX_REFS * 3]:
        if tried >= MAX_REFS:
            break
        i = bisect.bisect_right(old_funcs, s) - 1
        if i < 0:
            continue
        f = old_funcs[i]
        nf = locate(f)
        if nf is None:
            continue
        tried += 1
        ns = nf + (s - f)
        if not masked_equal(signature(old, s, 1), ns):
            continue
        ins = decode_at(new, ns)
        nt = rip_target(ins) if ins else None
        if nt is not None:
            votes[nt] += 1
    if votes:
        best, n = votes.most_common(1)[0]
        agree = n == sum(votes.values())
        print(f"{t:#010x}\t{best:#010x}\t{n}/{len(sites)} refs\t{'unanimous' if agree else 'SPLIT ' + str(dict((f'{k:#x}', v) for k, v in votes.items()))}\tdelta {best - t:+#x}")
    else:
        print(f"{t:#010x}\t-\t0/{len(sites)} refs\tno mapped reference\told sites "
              + ",".join(f"{s:#x}" for s in sites[:8]))
