"""File-only: check the 3732 constants in src/mod/session2/milestone3663.cpp against the image.
usage: check_milestone.py <3732.exe> src/mod/session2/milestone3663.cpp"""
import sys, re, struct, pefile
pe = pefile.PE(sys.argv[1], fast_load=True); base = pe.OPTIONAL_HEADER.ImageBase
src = open(sys.argv[2], encoding='utf-8').read()
def const(name): return int(re.search(r'\b' + name + r'\s*=\s*(0x[0-9A-Fa-f]+)', src).group(1), 16)
def arr(name): return bytes(int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2})', re.search(r'\b' + name + r'\{([^}]*)\}', src).group(1)))
ok = True
for site, name in (('kOwnerCheck','kOwnerBytes'), ('kOwnerCall','kRejoinBytes'), ('kHook','kHookBytes'), ('kCtor','kCtorBytes'), ('kEmit','kEmitBytes')):
    a, b = const(site), arr(name); same = pe.get_data(a, len(b)) == b; ok &= same
    print(f'{name:<13} at {a:#x} ({len(b)} bytes): {"match" if same else "MISMATCH"}')
q = lambda r: struct.unpack('<Q', pe.get_data(r, 8))[0] - base
for vt, off, slot in (('kVtToTarget',0,'kToTargetSlot0'), ('kVtToTarget',8,'kToTargetSlot1'), ('kVtToTarget',16,'kToTargetSlot2'), ('kVtFinderBase',0,'kBaseSlot0')):
    same = q(const(vt) + off) == const(slot); ok &= same
    print(f'{vt}+{off:<2} == {slot}: {"match" if same else "MISMATCH"}')
print('ALL MATCH' if ok else 'FAILED'); sys.exit(0 if ok else 1)