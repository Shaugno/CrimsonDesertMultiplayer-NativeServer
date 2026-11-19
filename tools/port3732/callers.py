"""File-only: show the instructions just before every direct call to a function.
usage: callers.py <exe> <rva> [before-bytes-hex] [max-callers]"""
import sys, re, struct, pefile, capstone

pe = pefile.PE(sys.argv[1], fast_load=True)
target = int(sys.argv[2], 16)
back = int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x30
limit = int(sys.argv[4]) if len(sys.argv) > 4 else 8
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
found = 0
for s in pe.sections:
    if not s.Characteristics & 0x20000000 or found >= limit:
        continue
    data, va = s.get_data(), s.VirtualAddress
    for m in re.finditer(rb"\xe8", data):
        k = m.start()
        if k + 5 > len(data) or va + k + 5 + struct.unpack_from("<i", data, k + 1)[0] != target:
            continue
        site = va + k
        print(f"-- call at {site:#x}")
        # decode forward from several start points and keep the one that lands on the call
        for start in range(site - back, site):
            seq = list(md.disasm(pe.get_data(start, site - start + 5), start))
            if seq and seq[-1].address == site:
                for ins in seq[-7:]:
                    print(f"   {ins.address:#x} {ins.mnemonic} {ins.op_str}")
                break
        found += 1
        if found >= limit:
            break
