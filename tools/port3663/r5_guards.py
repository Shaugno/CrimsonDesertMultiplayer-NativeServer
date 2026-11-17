"""File-only 3576/3663 guard cross-check. Never executes or maps an EXE."""
import argparse
import json
from pathlib import Path
import re
import struct
from pe_identity import PE

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--reference',type=Path,required=True)
p.add_argument('--target',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();reference=PE(a.reference);target=PE(a.target)
assert reference.sha256=='C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F'
assert target.sha256=='8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'
source=(Path(__file__).resolve().parents[2]/'src/mod/session2/r5_guards3663.cpp').read_text()
addresses={name:int(value,16) for name,value in re.findall(r'(\w+)=0x([0-9A-F]+)',source)}
arrays={name:bytes(int(x,16) for x in re.findall(r'0x([0-9A-F]+)',body)) for name,body in
    re.findall(r'constexpr std::array<uint8_t,\d+> (\w+)\{([^}]+)\};',source)}
def data(pe,rva,n):
    at=pe.offset(rva,n);return pe.data[at:at+n]
def slot(pe,vt,n):return struct.unpack('<Q',data(pe,vt+8*n,8))[0]-pe.image_base
checks=[]
for name,key in [('leave_head','leave_entry'),('leave_bytes','leave_point'),('own_bytes','own_call'),
                 ('unlock_head','unlock'),('achievement_bytes','achievement_point'),('cleanup_bytes','cleanup')]:
    rva=addresses[key];assert data(target,rva,len(arrays[name]))==arrays[name],name
    checks.append(dict(check=name,rva=hex(rva),bytes=len(arrays[name]),matched=True))
assert data(target,0x19E931D,len(arrays['epilogue']))==arrays['epilogue']
assert slot(target,addresses['sequencer_vt'],34)==addresses['leave_entry']
assert slot(target,addresses['achievement_vt'],2)==addresses['unlock']
assert slot(reference,0x5EF7040,34)==0x23BBD00
assert slot(reference,0x5B783F8,2)==0x1983400
assert addresses['leave_entry'] in target.entries and addresses['unlock'] in target.entries
assert 0x23BBD00 in reference.entries and 0x1983400 in reference.entries
assert data(reference,0x23BBD00,len(arrays['leave_head']))==arrays['leave_head']
assert data(reference,0x23BBD1E,len(arrays['leave_bytes']))==arrays['leave_bytes']
assert data(reference,0x23BBD2B,len(arrays['own_bytes']))==arrays['own_bytes']
assert data(reference,0x1983433,len(arrays['achievement_bytes']))==arrays['achievement_bytes']
# The original owner-null branch goes to the chosen own-manager continuation.
branch=data(target,addresses['leave_entry']+0x1C,2);assert branch==b'\x74\x0d'
assert addresses['leave_entry']+0x1E+branch[1]==addresses['own_call']
cleanup=addresses['cleanup'];call_targets=[]
for offset in [3,12]:
    assert data(target,cleanup+offset,1)==b'\xe8'
    call_targets.append(cleanup+offset+5+struct.unpack('<i',data(target,cleanup+offset+1,4))[0])
assert call_targets==[0x3DB480,0x3E6F50]
assert data(target,cleanup+19,2)==b'\xeb\x3a' and cleanup+21+0x3A==0x19E931D
result=dict(reference_commit='a3904af3e1a32056dcf7a63bc7a469eb4231084b',credit='Samuel Cardillo; original project GPL v3',
    reference_sha256=reference.sha256,target_sha256=target.sha256,executed_images=False,
    checked_cpp_arrays=checks,named_vtable_slots_verified=True,native_branches_verified=True,
    cleanup_calls=[hex(x) for x in call_targets],epilogue_verified=True,
    native_context='R5-UPSTREAM-REFERENCE.md and retained paired Ghidra function evidence',
    limitation='Static contracts only; no live firing or behavioral acceptance implied.')
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n')
print('Verified all target guard arrays, paired vtable entries, original branches and owned-argument cleanup; no EXE executed.')
