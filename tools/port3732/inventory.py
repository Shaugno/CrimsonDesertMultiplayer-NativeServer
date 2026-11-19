"""List 3663 RVA-like literals in 3663 source files (comments stripped), classified
by the 3663 section they fall in. File-only.
usage: inventory.py <3663.exe> <src-root> <out.tsv>"""
import sys, re, pathlib, pefile

exe, root, out = sys.argv[1], pathlib.Path(sys.argv[2]), sys.argv[3]
pe = pefile.PE(exe, fast_load=True)
size = pe.OPTIONAL_HEADER.SizeOfImage
secs = [(s.VirtualAddress, s.VirtualAddress + s.Misc_VirtualSize, s.Characteristics,
         s.Name.rstrip(b"\0").decode(errors="replace")) for s in pe.sections]
NAME = re.compile(r"3663|r3_|r4_|r5_|player_layout|health_definition|stat_id|tunnel_layout|field_share")
HEX = re.compile(r"\b0x([0-9A-Fa-f]{4,8})\b")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def kind(rva):
    for lo, hi, ch, name in secs:
        if lo <= rva < hi:
            return ("code" if ch & 0x20000000 else "data") + ":" + name
    return "none"


rows = {}
for path in sorted(root.rglob("*")):
    if path.suffix not in (".h", ".cpp") or not NAME.search(path.name):
        continue
    for lineno, line in enumerate(strip_comments(path.read_text(encoding="utf-8", errors="replace")).splitlines(), 1):
        for m in HEX.finditer(line):
            v = int(m.group(1), 16)
            if 0x1000 <= v < size:
                rows.setdefault(v, set()).add(f"{path.relative_to(root).as_posix()}")
with open(out, "w", encoding="utf-8") as f:
    for v in sorted(rows):
        f.write(f"{v:#010x}\t{kind(v)}\t{','.join(sorted(rows[v]))}\n")
by = {}
for v in rows:
    by[kind(v).split(':')[0]] = by.get(kind(v).split(':')[0], 0) + 1
print(len(rows), "unique values", by)
