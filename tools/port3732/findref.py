"""File-only: list RIP-relative instructions within +-window of an RVA whose
operand resolves to a given target RVA.
usage: findref.py <exe> <center-rva> <target-rva> [window-hex]"""
import sys, struct, pefile, capstone
from capstone import x86

pe = pefile.PE(sys.argv[1], fast_load=True)
center, target = int(sys.argv[2], 16), int(sys.argv[3], 16)
window = int(sys.argv[4], 16) if len(sys.argv) > 4 else 0x4000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True
lo = center - window
data = pe.get_data(lo, 2 * window + 16)
for k in range(len(data) - 8):
    disp = struct.unpack_from("<i", data, k)[0]
    for tail in (0, 1, 2, 4):
        if lo + k + 4 + tail + disp != target:
            continue
        for back in range(3, 0, -1):
            start = lo + k - back
            for ins in md.disasm(pe.get_data(start, 16), start):
                if ins.disp_offset == back and any(
                        op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP for op in ins.operands):
                    print(f"{ins.address:#x} {ins.bytes.hex()} {ins.mnemonic} {ins.op_str}")
                break
