"""File-only: print the qwords around RVAs in two images side by side, and
whether each RVA lies in a section's file-backed part (beyond it the
loader zero-fills, so the file holds no content to compare).
usage: qwords.py <old.exe> <new.exe> <old:new> [...]"""
import sys, struct, pefile

pes = [pefile.PE(p, fast_load=True) for p in sys.argv[1:3]]


def where(pe, rva):
    for s in pe.sections:
        if s.VirtualAddress <= rva < s.VirtualAddress + s.Misc_VirtualSize:
            name = s.Name.rstrip(b"\0").decode(errors="replace")
            backed = rva < s.VirtualAddress + s.SizeOfRawData
            return f"{name} {'file-backed' if backed else 'zero-filled'}"
    return "no section"


for arg in sys.argv[3:]:
    pair = [int(x, 16) for x in arg.split(":")]
    for label, pe, rva in zip(("old", "new"), pes, pair):
        d = pe.get_data(rva - 0x20, 0x40)
        words = " ".join(f"{struct.unpack_from('<Q', d, i)[0]:x}" for i in range(0, len(d) - 7, 8))
        print(f"{label} {rva:#x} [{where(pe, rva)}] {words}")
