"""Rewrite 3663 address literals and expected-byte arrays in the source for 3732.

Reads the reviewed maps (old<TAB>new<TAB>method...) and, in each listed
source file, replaces mapped hex literals in code (never in // or /* */
comments). Byte arrays that sit on the same line as a mapped code address,
or that are named <Symbol>Head next to `<Symbol> = <address>`, are re-read
from the new image at the new address with the same length. Each refreshed
array is compared with the old bytes with rel32 branch targets and RIP
displacements masked: any other difference is reported as REVIEW and the
array is left unchanged so the build-time or runtime check fails closed.

usage: rewrite.py <old.exe> <new.exe> <code-map.tsv> <data-map.tsv> [--apply] <file>...
Without --apply it only reports. File-only; never executes either image."""
import sys, re, pefile, capstone
from capstone import x86

args = sys.argv[1:]
old_path, new_path, code_map, data_map = args[:4]
apply = "--apply" in args
files = [a for a in args[4:] if a != "--apply"]
old = pefile.PE(old_path, fast_load=True)
new = pefile.PE(new_path, fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True

mapping, is_code = {}, {}
for path, code in ((code_map, True), (data_map, False)):
    for line in open(path, encoding="utf-8-sig"):
        c = line.rstrip("\n").split("\t")
        if len(c) >= 2 and c[1] not in ("-", ""):
            mapping[int(c[0], 16)] = int(c[1], 16)
            is_code[int(c[0], 16)] = code

HEX = re.compile(r"\b0x([0-9A-Fa-f]{4,8})\b")
ARRAY = re.compile(r"\{((?:\s*0x[0-9A-Fa-f]{1,2}\s*,){3,}\s*0x[0-9A-Fa-f]{1,2}\s*)\}")


def mask_positions(pe, rva, length):
    """Byte offsets (relative to rva) that hold rel32 targets or RIP displacements."""
    masked = set()
    for ins in md.disasm(pe.get_data(rva, length + 16), rva):
        if ins.address - rva >= length:
            break
        base = ins.address - rva
        rel = ins.mnemonic.startswith("j") or ins.mnemonic == "call"
        for o in ins.operands:
            if o.type == x86.X86_OP_MEM and o.mem.base == x86.X86_REG_RIP:
                masked.update(range(base + ins.disp_offset, base + ins.disp_offset + ins.disp_size))
            if o.type == x86.X86_OP_IMM and rel and ins.imm_size:
                masked.update(range(base + ins.imm_offset, base + ins.imm_offset + ins.imm_size))
    return masked


def split_code_comment(line, in_block):
    """Return list of (text, is_code) segments and the block-comment state at line end."""
    segs, i, cur, code = [], 0, "", not in_block
    in_str = False
    while i < len(line):
        two = line[i:i + 2]
        if code and not in_str and two == "//":
            segs.append((cur, True)); segs.append((line[i:], False)); return segs, False
        if code and not in_str and two == "/*":
            segs.append((cur, True)); cur, code = "/*", False; i += 2; continue
        if not code and two == "*/":
            segs.append((cur + "*/", False)); cur, code = "", True; i += 2; continue
        ch = line[i]
        if code and ch == '"' and (i == 0 or line[i - 1] != "\\"):
            in_str = not in_str
        cur += ch
        i += 1
    segs.append((cur, code))
    return segs, not code


def refresh(old_rva, new_rva, arr_text):
    old_bytes = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{1,2})", arr_text)]
    n = len(old_bytes)
    file_old = list(old.get_data(old_rva, n))
    if file_old != old_bytes:
        return None, f"old array does not equal 3663 bytes at {old_rva:#x}"
    new_bytes = list(new.get_data(new_rva, n))
    mo, mn = mask_positions(old, old_rva, n), mask_positions(new, new_rva, n)
    bad = [k for k in range(n) if k not in mo and k not in mn and old_bytes[k] != new_bytes[k]]
    jmp_len = 0
    for ins in md.disasm(bytes(old_bytes), old_rva):
        if ins.mnemonic == "jmp" and ins.size == 5 and ins.bytes[0] == 0xE9:
            jmp_len = 5
        break
    for ins in md.disasm(bytes(new_bytes), new_rva):
        if not (ins.mnemonic == "jmp" and ins.size == 5 and ins.bytes[0] == 0xE9):
            jmp_len = 0
        break
    if bad and jmp_len and min(bad) >= jmp_len:
        upper = any(c.isupper() for c in "".join(re.findall(r"0x([0-9A-Fa-f]{1,2})", arr_text)))
        sep = ", " if ", " in arr_text else ","
        fmt = (lambda b: f"0x{b:02X}") if upper else (lambda b: f"0x{b:x}")
        return sep.join(fmt(b) for b in new_bytes), "refreshed (thunk: only bytes after its jmp differ)"
    if bad:
        return None, f"REVIEW: bytes differ outside masked fields at offsets {bad}"
    upper = any(c.isupper() for c in "".join(re.findall(r"0x([0-9A-Fa-f]{1,2})", arr_text)))
    sep = ", " if ", " in arr_text else ","
    fmt = (lambda b: f"0x{b:02X}") if upper else (lambda b: f"0x{b:x}")
    return sep.join(fmt(b) for b in new_bytes), ("refreshed" if new_bytes != old_bytes else "unchanged")


old_exec = [(s.VirtualAddress, s.get_data()) for s in old.sections if s.Characteristics & 0x20000000]
d3 = old.OPTIONAL_HEADER.DATA_DIRECTORY[3]
_raw = old.get_data(d3.VirtualAddress, d3.Size)
old_pdata = sorted({__import__("struct").unpack_from("<I", _raw, i)[0] for i in range(0, len(_raw) - 11, 12)})
code_sorted = sorted(k for k in mapping if is_code[k])


# Short arrays that occur more than once in 3663: owner address by the code that
# checks them (each still has to equal the 3663 bytes at that address).
OWNER = {
    "kClientOnRecvPacketHead": 0x3A9CF0,   # guest.cpp / guest3663.cpp head_matches(kClientOnRecvPacket, ...)
    "kAcquireFieldHead": 0x2E08520,        # head_ok(kAcquireField, ...)
    "kTryAddRefHead": 0x1658E90,           # head_ok(kTryAddRef, ...)
    "kVecPushHead": 0x4CAE10,              # head_ok(kVecPush, ...)
    "kWindowsLockVf1Head": 0x1593240,      # WindowsLock vf1 (lock) = kLock3
    "kWindowsLockVf2Head": 0xE297F0,       # WindowsLock vf2 (unlock) = kUnlock3
    "update_edge": 0x972B82,               # character_guard3663 matches(update_call, update_edge)
    "leave_bytes": 0x242D4FE,              # r5_guards3663 leave_point
    "epilogue": 0x19E931D,                 # r5_guards3663 matches(0x19E931D, epilogue)
}


def locate_by_bytes(arr_text, label=""):
    """Unique exact occurrence of the array in the old image, mapped to the new image."""
    import bisect
    data = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{1,2})", arr_text))
    if label in OWNER:
        o = OWNER[label]
        if old.get_data(o, len(data)) != data:
            return None, f"owner {o:#x} bytes do not equal the array"
        if o not in mapping:
            return None, f"owner {o:#x} not in the code map"
        return (o, mapping[o]), f"owner {o:#x}"
    hits = []
    for va, sec in old_exec:
        k = sec.find(data)
        while k >= 0 and len(hits) < 3:
            hits.append(va + k)
            k = sec.find(data, k + 1)
    if len(hits) != 1:
        return None, f"{len(hits)} exact occurrences in 3663"
    o = hits[0]
    if o in mapping:
        return (o, mapping[o]), "mapped"
    i = bisect.bisect_right(old_pdata, o) - 1
    f = old_pdata[i] if i >= 0 else None
    j = bisect.bisect_right(code_sorted, o) - 1
    cands = [code_sorted[j]] if j >= 0 else []
    if j + 1 < len(code_sorted):
        cands.append(code_sorted[j + 1])
    for m in cands:
        if f is not None and bisect.bisect_right(old_pdata, m) - 1 == i:
            return (o, mapping[m] + (o - m)), f"same function as mapped {m:#x}"
    return None, f"at {o:#x}; no mapped address in its function {f:#x}" if f else f"at {o:#x}; no function"


report = []
for path in files:
    text = open(path, encoding="utf-8").read()
    lines = text.split("\n")
    head_targets = {}
    for line in lines:
        m = re.search(r"\b(k\w+)\s*=\s*0x([0-9A-Fa-f]{4,8})\b", line)
        if m and int(m.group(2), 16) in mapping and is_code.get(int(m.group(2), 16)):
            head_targets[m.group(1) + "Head"] = int(m.group(2), 16)
    in_block, out = False, []
    for no, line in enumerate(lines, 1):
        segs, in_block = split_code_comment(line, in_block)
        code_text = "".join(t for t, c in segs if c)
        rvas = [int(h, 16) for h in HEX.findall(code_text) if int(h, 16) in mapping and is_code.get(int(h, 16))]
        arr_rva = rvas[0] if len(rvas) == 1 else None
        hm = re.search(r"\b(k\w+Head)\s*\[\]", code_text)
        if arr_rva is None and hm and hm.group(1) in head_targets:
            arr_rva = head_targets[hm.group(1)]
        new_segs = []
        for t, c in segs:
            if c:
                am = ARRAY.search(t)
                if am and arr_rva is not None:
                    body, status = refresh(arr_rva, mapping[arr_rva], am.group(1))
                    report.append(f"{path}:{no}\tbytes {arr_rva:#x}->{mapping[arr_rva]:#x}\t{status}")
                    if body is not None:
                        t = t[:am.start(1)] + body + t[am.end(1):]
                elif am:
                    name = re.search(r"\b(\w+)\s*(?:\[\])?\s*(?:=)?\s*\{", t)
                    label = name.group(1) if name else t.strip()[:40]
                    pair, how = locate_by_bytes(am.group(1), label)
                    if pair is None:
                        report.append(f"{path}:{no}\tbytes UNRESOLVED {label}\t{how}")
                    else:
                        o, nrva = pair
                        body, status = refresh(o, nrva, am.group(1))
                        report.append(f"{path}:{no}\tbytes {o:#x}->{nrva:#x} {label} ({how})\t{status}")
                        if body is not None:
                            t = t[:am.start(1)] + body + t[am.end(1):]
                def sub(m):
                    v = int(m.group(1), 16)
                    if v not in mapping:
                        return m.group(0)
                    h = m.group(1)
                    nv = mapping[v]
                    s = f"{nv:X}" if h.upper() == h and any(ch.isalpha() for ch in h) else f"{nv:x}"
                    if not any(ch.isalpha() for ch in h):
                        s = f"{nv:X}"
                    s = s.rjust(len(h), "0") if h.startswith("0") else s
                    report.append(f"{path}:{no}\taddr {v:#x}->{nv:#x}")
                    return "0x" + s
                t = HEX.sub(sub, t)
            new_segs.append(t)
        out.append("".join(new_segs))
    if apply:
        open(path, "w", encoding="utf-8", newline="").write("\n".join(out))
for r in report:
    print(r)
