"""File-only: compare a data table between builds with pointer qwords masked.
Qwords whose value lies inside the image (base .. base+SizeOfImage) are
treated as pointers and only checked to be pointers on both sides; every
other byte must be equal. Reports differing qword slots.
usage: tablecmp.py <old.exe> <old_begin:old_end> <new.exe> <new_begin>"""
import sys, struct, pefile

old = pefile.PE(sys.argv[1], fast_load=True)
new = pefile.PE(sys.argv[3], fast_load=True)
lo, hi = (int(x, 16) for x in sys.argv[2].split(":"))
nlo = int(sys.argv[4], 16)
a, b = old.get_data(lo, hi - lo), new.get_data(nlo, hi - lo)


def is_ptr(pe, v):
    base = pe.OPTIONAL_HEADER.ImageBase
    return base <= v < base + pe.OPTIONAL_HEADER.SizeOfImage


diffs, ptrs = [], 0
for k in range(0, len(a) - 7, 8):
    x, y = struct.unpack_from("<Q", a, k)[0], struct.unpack_from("<Q", b, k)[0]
    if is_ptr(old, x) and is_ptr(new, y):
        ptrs += 1
        continue
    if x != y:
        diffs.append((k, x, y))
print(f"table {lo:#x}..{hi:#x} ({(hi - lo) // 8} qwords) vs {nlo:#x}: pointer slots {ptrs}, differing slots {len(diffs)}")
for k, x, y in diffs[:20]:
    print(f"  +{k:#x}  old {x:#018x}  new {y:#018x}")
