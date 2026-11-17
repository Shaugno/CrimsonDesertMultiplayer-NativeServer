"""Bounded file-only crash metadata. Stack words are candidates, not unwound frames."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

parser=argparse.ArgumentParser()
parser.add_argument('dump',type=Path)
parser.add_argument('--out',type=Path,required=True)
args=parser.parse_args()
data=args.dump.read_bytes()
def unpack(fmt,at):
    size=struct.calcsize(fmt)
    if at<0 or at+size>len(data):raise ValueError('dump bounds')
    return struct.unpack_from(fmt,data,at)
assert data[:4]==b'MDMP'
count,directory=unpack('<II',8)
assert count<=1024
streams={kind:(rva,size) for kind,size,rva in (unpack('<III',directory+12*i) for i in range(count))}
modules=[]
if 4 in streams:
    at,_=streams[4];total,=unpack('<I',at);assert total<=65536
    for i in range(total):
        base,size,checksum,timestamp,name=unpack('<QIIII',at+4+108*i)
        length,=unpack('<I',name);assert length<=65536
        if name+4+length>len(data):raise ValueError('module name bounds')
        label=data[name+4:name+4+length].decode('utf-16-le').replace('\\','/').rsplit('/',1)[-1]
        modules.append(dict(name=label,base=base,size=size,timestamp=timestamp))
def address(value):
    for module in modules:
        if module['base']<=value<module['base']+module['size']:
            return dict(module=module['name'],rva=hex(value-module['base']))
    return dict(unresolved=hex(value))
at,_=streams[6]
thread,=unpack('<I',at)
code,flags,record,pc,nparams=unpack('<IIQQI',at+8)
assert nparams<=15
parameters=list(unpack('<'+'Q'*nparams,at+40))
ctx_size,ctx=unpack('<II',at+160)
assert ctx_size>=256
rsp,=unpack('<Q',ctx+152)
rip,=unpack('<Q',ctx+248)
result=dict(dump_name=args.dump.name,dump_sha256=hashlib.sha256(data).hexdigest().upper(),
    exception_thread=thread,exception_code=hex(code),exception_address=address(pc),
    rip=address(rip),parameters=[hex(v) for v in parameters],stack_candidates=[],
    rsp=hex(rsp),
    stack_warning='module-valued stack words only; not unwound or verified call frames')
if 16 in streams:
    at,_=streams[16];header,stride,total=unpack('<IIQ',at);assert total<=10000000 and stride>=48
    for i in range(total):
        base,allocation,alloc_protect,unused,size,state,protect,kind=unpack('<QQIIQIII',at+header+stride*i)
        if base<=rip<base+size:
            result['rip_memory_region']=dict(state=hex(state),protect=hex(protect),allocation_protect=hex(alloc_protect),type=hex(kind))
            break
if 3 in streams:
    at,_=streams[3];total,=unpack('<I',at);assert total<=65536
    for i in range(total):
        entry=at+4+48*i;tid,=unpack('<I',entry)
        if tid!=thread:continue
        stack_base,stack_size,stack_rva=unpack('<QII',entry+24)
        result['thread_stack']=dict(base=hex(stack_base),size=stack_size,rsp_in_range=stack_base<=rsp<stack_base+stack_size)
        if stack_base<=rsp<stack_base+stack_size:
            for delta in range(0,min(1024,stack_base+stack_size-rsp)-7,8):
                value,=unpack('<Q',stack_rva+rsp-stack_base+delta)
                where=address(value)
                if 'module' in where:result['stack_candidates'].append(dict(stack_offset=hex(delta),**where))
        break
args.out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(json.dumps(result,indent=2))
