"""Check every adopted R2 struct-offset row against 3732.

Mode 'funcs': print the 3663 functions named in the rows' evidence as
'2976/3663' pairs (second of each pair), for match.py.
Mode 'check': with a match.py output (old<TAB>new<TAB>method...), walk each
row's evidence functions in both images (recursive descent, jumps followed,
calls not entered) and compare how often the row's target offset is used as a
displacement or immediate. Status per row:
  same      every mapped evidence function uses the offset equally often
  DIFFERS   some evidence function uses it a different number of times
  unmapped  no evidence function could be mapped / none uses the offset
usage: rowcheck.py funcs <R2-MATCHES.json>
       rowcheck.py check <R2-MATCHES.json> <old.exe> <new.exe> <matches.tsv>"""
import sys, re, json, collections

mode, path = sys.argv[1], sys.argv[2]
doc = json.load(open(path, encoding="utf-8"))
rows = [r for r in doc["rows"] if r.get("adopted") and re.fullmatch(r"0x[0-9A-Fa-f]{1,4}", str(r.get("target", "")))]
PAIR = re.compile(r"\b([0-9A-F]{5,8})/([0-9A-F]{5,8})\b")


import os
EVROOT = os.path.dirname(os.path.abspath(path))
RVA_LINE = re.compile(r"^RVA=(0x[0-9A-Fa-f]+|\d+)\s*$", re.M)
FUN = re.compile(r"\bFUN_14([0-9a-f]{7})\b")


def file_funcs(r):
    """3663 functions listed in the row's target-side evidence files."""
    out = set()
    for tok in re.split(r"[;,]\s*", r.get("evidence", "")):
        tok = tok.strip()
        if not tok or "reference" in tok or "old" in os.path.basename(tok) or "2976" in tok:
            continue
        name = tok if "/" in tok else "port-evidence/r2/" + tok
        p = os.path.normpath(os.path.join(EVROOT, name))
        if not p.endswith(".txt") or not os.path.exists(p):
            continue
        text = open(p, encoding="utf-8", errors="replace").read()
        for v in RVA_LINE.findall(text):
            out.add(int(v, 0))
        for v in FUN.findall(text):
            out.add(int(v, 16))
    return out


def evidence_funcs(r):
    return sorted({int(b, 16) for _, b in PAIR.findall(r.get("second_check", ""))} | file_funcs(r))


if mode == "funcs":
    print(" ".join(sorted({f"{f:X}" for r in rows for f in evidence_funcs(r)})))
    sys.exit()

import pefile, capstone
from capstone import x86

old = pefile.PE(sys.argv[3], fast_load=True)
new = pefile.PE(sys.argv[4], fast_load=True)
mapping = {}
for line in open(sys.argv[5], encoding="utf-8-sig"):
    c = line.rstrip("\n").split("\t")
    if len(c) >= 2 and c[1] not in ("-", ""):
        mapping[int(c[0], 16)] = int(c[1], 16)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
cache = {}


def values(pe, entry, limit=6000):
    key = (id(pe), entry)
    if key in cache:
        return cache[key]
    seen, work, vals, n = set(), [entry], collections.Counter(), 0
    while work and n < limit:
        pc = work.pop()
        while pc not in seen and n < limit:
            ins = next(md.disasm(pe.get_data(pc, 16), pc), None)
            if ins is None:
                break
            seen.add(pc)
            n += 1
            for o in ins.operands:
                if o.type == x86.X86_OP_MEM and o.mem.base not in (0, x86.X86_REG_RIP, x86.X86_REG_RSP):
                    vals[o.mem.disp] += 1
                elif o.type == x86.X86_OP_IMM and not (ins.mnemonic.startswith("j") or ins.mnemonic == "call"):
                    vals[o.imm] += 1
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
    cache[key] = vals
    return vals


for r in rows:
    t = int(r["target"], 16)
    detail, status = [], "unmapped"
    used = False
    differs = False
    for f in evidence_funcs(r):
        if f not in mapping:
            detail.append(f"{f:#x}:unmapped")
            continue
        a, b = values(old, f)[t], values(new, mapping[f])[t]
        shifted = {s: values(new, mapping[f])[t + s] for s in (-0x10, -8, 8, 0x10)}
        if a == 0 and b == 0:
            detail.append(f"{f:#x}:unused")
            continue
        used = True
        if a != b:
            differs = True
            detail.append(f"{f:#x}->{mapping[f]:#x}: {a} vs {b} (shifted {shifted})")
        else:
            detail.append(f"{f:#x}:{a}={b}")
    if differs:
        status = "DIFFERS"
    elif used:
        status = "same"
    print(f"{r['row']}\t{r['symbol']}\t{r['target']}\t{status}\t{'; '.join(detail)}")
