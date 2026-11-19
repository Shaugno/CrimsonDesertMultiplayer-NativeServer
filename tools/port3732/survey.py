"""File-only whole-image survey of struct displacement changes between builds.

Pairing (first executable section, .pdata functions):
1. anchors: functions whose raw bytes are unique in each image and equal
   between them (no decoding; exact).
2. a function between two consecutive anchors that agree on the delta is
   paired with the new .pdata function at old + delta.
Pairs whose raw bytes differ are decoded; instructions with the same
mnemonic and operand shape (address fields excluded) whose only difference
is a displacement or immediate are counted as old -> new changes.

usage: survey.py <old.exe> <new.exe> <out-prefix>
writes <prefix>-disp.tsv   old new count functions example_old example_new instruction
       <prefix>-pairs.tsv  old_fn new_fn status"""
import sys, re, struct, bisect, hashlib, collections, pefile, capstone
from capstone import x86

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[2], fast_load=True)
prefix = sys.argv[3]
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True


def funcs(pe):
    s = pe.sections[0]
    lo, hi, data = s.VirtualAddress, s.VirtualAddress + s.Misc_VirtualSize, s.get_data()
    d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
    raw = pe.get_data(d.VirtualAddress, d.Size)
    rows = {}
    for i in range(0, len(raw) - 11, 12):
        b, e, _ = struct.unpack_from("<III", raw, i)
        if lo <= b < e <= hi:
            rows.setdefault(b, e)
    return sorted(rows.items()), lo, data


of, ov, od = funcs(old)
nf, nv, nd = funcs(new)
nbegin = {b: e for b, e in nf}


def digest(data, va, b, e):
    return hashlib.blake2b(data[b - va:e - va], digest_size=16).digest()


def unique(fl, va, data):
    seen = collections.defaultdict(list)
    for b, e in fl:
        seen[digest(data, va, b, e)].append(b)
    return {h: v[0] for h, v in seen.items() if len(v) == 1}


uo, un = unique(of, ov, od), unique(nf, nv, nd)
anchors = sorted((uo[h], un[h]) for h in uo.keys() & un.keys())
# keep only monotonic anchors (drop moved/duplicated code that breaks order)
mono, last = [], -1
for a, b in anchors:
    if b > last:
        mono.append((a, b))
        last = b
akeys = [a for a, _ in mono]
print(f"functions old {len(of)} new {len(nf)} anchors {len(anchors)} monotonic {len(mono)}")

OPS = re.compile(r"0x[0-9a-f]+|\b\d+\b")


def decode(data, va, b, e):
    out = []
    for ins in md.disasm(data[b - va:e - va], b):
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        rip = any(o.type == x86.X86_OP_MEM and o.mem.base == x86.X86_REG_RIP for o in ins.operands)
        out.append((ins.mnemonic, ins.op_str, rel or rip))
    return out


disp = collections.Counter()
fn_count = collections.defaultdict(set)
example = {}
stats = collections.Counter()
with open(f"{prefix}-pairs.tsv", "w", encoding="utf-8") as pf:
    for b, e in of:
        i = bisect.bisect_right(akeys, b) - 1
        if i < 0 or i + 1 >= len(mono):
            stats["outside anchors"] += 1
            continue
        d1, d2 = mono[i][1] - mono[i][0], mono[i + 1][1] - mono[i + 1][0]
        if d1 != d2:
            stats["delta changes between anchors"] += 1
            continue
        nb = b + d1
        if nb not in nbegin:
            stats["no new function at predicted start"] += 1
            continue
        ne = nbegin[nb]
        if od[b - ov:e - ov] == nd[nb - nv:ne - nv]:
            stats["identical bytes"] += 1
            continue
        a, c = decode(od, ov, b, e), decode(nd, nv, nb, ne)
        if len(a) != len(c):
            stats["different length"] += 1
            pf.write(f"{b:#x}\t{nb:#x}\tlength {len(a)}/{len(c)}\n")
            continue
        changed = False
        for x, y in zip(a, c):
            if x == y or x[2] or y[2]:
                continue
            if x[0] != y[0] or OPS.sub("#", x[1]) != OPS.sub("#", y[1]):
                changed = True
                continue
            for p, q in zip(OPS.findall(x[1]), OPS.findall(y[1])):
                if p != q:
                    changed = True
                    disp[(p, q)] += 1
                    fn_count[(p, q)].add(b)
                    example.setdefault((p, q), (b, nb, f"{x[0]} {x[1]}"))
        stats["changed operands" if changed else "only address fields differ"] += 1
        if changed:
            pf.write(f"{b:#x}\t{nb:#x}\tchanged\n")
with open(f"{prefix}-disp.tsv", "w", encoding="utf-8") as f:
    for (p, q), n in sorted(disp.items(), key=lambda kv: (-len(fn_count[kv[0]]), -kv[1])):
        eb, en, txt = example[(p, q)]
        f.write(f"{p}\t{q}\t{n}\t{len(fn_count[(p, q)])}\t{eb:#x}\t{en:#x}\t{txt}\n")
for k, v in stats.most_common():
    print(f"{k}: {v}")
print(f"distinct operand changes {len(disp)}")
