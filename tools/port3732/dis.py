"""File-only disassembly of a PE image at given RVAs. Never executes the image.
usage: dis.py <exe> <rva> [count]"""
import sys, pefile, capstone

exe, rva = sys.argv[1], int(sys.argv[2], 16)
count = int(sys.argv[3]) if len(sys.argv) > 3 else 40
pe = pefile.PE(exe, fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
data = pe.get_data(rva, count * 16)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
n = 0
for ins in md.disasm(data, base + rva):
    print(f"{ins.address:x} {ins.bytes.hex():<24} {ins.mnemonic} {ins.op_str}")
    n += 1
    if n >= count:
        break
