"""File-only: find instructions that store a 16-bit opcode immediate to memory
(`mov word ptr [...], imm16`), the way packet serializers write their
opcode, and report the enclosing .pdata function.
usage: opwriters.py <exe> <opcode-decimal>..."""
import sys, re, struct, bisect, pefile, capstone

pe = pefile.PE(sys.argv[1], fast_load=True)
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
raw = pe.get_data(d.VirtualAddress, d.Size)
starts = sorted({struct.unpack_from("<I", raw, i)[0] for i in range(0, len(raw) - 11, 12)})
for op in (int(x) for x in sys.argv[2:]):
    imm = struct.pack("<H", op)
    found = []
    for s in pe.sections:
        if not s.Characteristics & 0x20000000:
            continue
        data, va = s.get_data(), s.VirtualAddress
        for m in re.finditer(re.escape(imm), data):
            k = m.start()
            for back in range(3, 9):
                st = va + k - back
                ins = next(md.disasm(pe.get_data(st, 16), st), None)
                if ins and ins.mnemonic == "mov" and ins.op_str.startswith("word ptr [") \
                        and ins.op_str.endswith(f", {op:#x}") and ins.address + ins.size == va + k + 2:
                    i = bisect.bisect_right(starts, ins.address) - 1
                    found.append((ins.address, f"{ins.mnemonic} {ins.op_str}", starts[i] if i >= 0 else None))
                    break
    print(f"opcode {op} ({op:#x}): {len(found)} writer(s)")
    for a, t, f in found[:12]:
        print(f"   {a:#x}  {t}   in function {f:#x}" if f else f"   {a:#x}  {t}")
