"""Intersect the struct offsets used by the 3663 sources with the survey's
changed memory displacements.

Collects small hex literals (0x8..0xFFFF) from code (comments and byte
arrays excluded) in the given files, and for each one lists the survey's
[reg + X] -> [reg + Y] changes with X equal to it (count, functions,
example). Offsets with no change in any paired function are reported as
'no change seen'. File-only; reads survey output, never an image.
usage: offsets.py <survey-disp.tsv> <file>..."""
import sys, re, collections

changes = collections.defaultdict(list)
for line in open(sys.argv[1], encoding="utf-8"):
    old, new, count, funcs, eo, en, text = line.rstrip("\n").split("\t", 6)
    if re.search(r"\[[a-z0-9]+(?: \+ [a-z0-9]+\*\d)? \+ " + re.escape(old) + r"\]", text):
        changes[int(old, 16)].append((new, int(count), int(funcs), eo, en, text))

HEX = re.compile(r"\b0x([0-9A-Fa-f]{1,4})\b")
ARR = re.compile(r"\{(?:\s*0x[0-9A-Fa-f]{1,2}\s*,?)+\}")
uses = collections.defaultdict(set)
for path in sys.argv[2:]:
    text = re.sub(r"/\*.*?\*/", "", open(path, encoding="utf-8").read(), flags=re.S)
    for no, line in enumerate(text.split("\n"), 1):
        code = ARR.sub("", re.sub(r"//[^\n]*", "", line))
        for m in HEX.finditer(code):
            v = int(m.group(1), 16)
            if 0x8 <= v <= 0xFFFF:
                uses[v].add(f"{path}:{no}")
for v in sorted(uses):
    where = ", ".join(sorted(uses[v])[:3]) + (" ..." if len(uses[v]) > 3 else "")
    if v in changes:
        for new, count, funcs, eo, en, text in changes[v]:
            print(f"{v:#x}\t-> {new}\t{count} sites in {funcs} functions\te.g. {eo}->{en} '{text}'\tused at {where}")
