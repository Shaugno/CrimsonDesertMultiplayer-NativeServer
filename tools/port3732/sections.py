"""Section table with byte entropy per section (file-only)."""
import sys, math, pefile

for exe in sys.argv[1:]:
    pe = pefile.PE(exe, fast_load=True)
    print(f"== {exe}\n   timestamp {pe.FILE_HEADER.TimeDateStamp:#x} entry {pe.OPTIONAL_HEADER.AddressOfEntryPoint:#x} "
          f"sizeofimage {pe.OPTIONAL_HEADER.SizeOfImage:#x}")
    for s in pe.sections:
        raw = s.get_data()[: 8 * 1024 * 1024]
        if raw:
            counts = [0] * 256
            for b in raw:
                counts[b] += 1
            ent = -sum(c / len(raw) * math.log2(c / len(raw)) for c in counts if c)
            zero = counts[0] / len(raw)
        else:
            ent, zero = 0.0, 1.0
        flags = ("X" if s.Characteristics & 0x20000000 else "-") + ("W" if s.Characteristics & 0x80000000 else "-")
        print(f"   {s.Name.rstrip(b'\0').decode(errors='replace'):<9} va {s.VirtualAddress:#010x} vsize {s.Misc_VirtualSize:#010x} "
              f"raw {s.SizeOfRawData:#010x} {flags} entropy {ent:.2f} zero {zero:.2f}")
