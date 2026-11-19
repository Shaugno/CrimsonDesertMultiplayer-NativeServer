"""File-only caller check for old -> new address pairs.
Finds every direct rel32 call/jmp (E8/E9) to the old address in the old
image's executable sections and to the new address in the new image's,
then reports whether the caller sites correspond (same count, each new
caller within a small distance of an old caller's position).
usage: xref.py <old.exe> <new.exe> <old:new> [<old:new> ...]"""
import sys, re, struct, pefile

old_path, new_path = sys.argv[1], sys.argv[2]
pairs = [tuple(int(x, 16) for x in a.split(":")) for a in sys.argv[3:]]


def refs(path, wanted):
    pe = pefile.PE(path, fast_load=True)
    out = {w: [] for w in wanted}
    rx = re.compile(rb"[\xe8\xe9]")
    for s in pe.sections:
        if not s.Characteristics & 0x20000000:
            continue
        data, va = s.get_data(), s.VirtualAddress
        for m in rx.finditer(data, 0, len(data) - 5):
            k = m.start()
            t = va + k + 5 + struct.unpack_from("<i", data, k + 1)[0]
            if t in out:
                out[t].append((va + k, data[k]))
    return out


old_refs = refs(old_path, {o for o, _ in pairs})
new_refs = refs(new_path, {n for _, n in pairs})
for o, n in pairs:
    a = sorted(old_refs[o])
    b = sorted(new_refs[n])
    deltas = []
    for site, op in a:
        near = [s for s, p in b if p == op and abs(s - site) < 0x80000]
        deltas.append(min((s - site for s in near), key=abs) if near else None)
    ok = len(a) == len(b) and a and all(d is not None for d in deltas)
    print(f"{o:#010x} -> {n:#010x}  callers old {len(a)} new {len(b)}  "
          f"{'CONSISTENT' if ok else 'CHECK'}  deltas {[('%+#x' % d) if d is not None else None for d in deltas][:8]}")
