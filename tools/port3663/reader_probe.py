"""Passive HP-definition or loading trace. External read/query only; never a feed.

Run --self-test without a game. A live run requires the R2 presence/backup/deploy
session driver. Candidate RVAs come only from build_3663.h, not CLI overrides.
"""
import argparse
import ctypes as C
from ctypes import wintypes as W
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
EXPECTED = Path(r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe')
HASH = '8be67178d9b9a149c361a731051665e0ba4d441e250ae1dfe645bd6e524345c8'

def constants():
    text = (ROOT/'src/mod/game/build_3663.h').read_text()
    def block(name):
        section = re.search(r'namespace '+name+r' \{(.*?)\}',text,re.S)[1]
        return {n:int(v,0) for n,v in re.findall(r'constexpr (?:uintptr_t|uint32_t) (\w+) = (0x[0-9A-Fa-f]+|\d+);',section)}
    identity={name:int(re.search(r'constexpr \w+ '+name+r' = (0x[0-9A-Fa-f]+);',text)[1],16)
              for name in ('kImageBase','kPeTimestamp','kSizeOfImage')}
    return block('diagnostic_candidates'),block('player_layout'),identity

class View:
    def __init__(self,read,base,rv,layout):
        self.read,self.base,self.rv,self.layout = read,base,rv,layout
    def value(self,address,fmt):
        raw = self.read(address,struct.calcsize(fmt))
        return struct.unpack(fmt,raw)[0] if raw is not None else None
    def pointer(self,address):
        value = self.value(address,'<Q')
        return value if value is not None and 0x10000<=value<=0x7FFFFFFFFFFF else None
    def hp_id(self,slot):
        p = self.pointer(self.base+self.rv[slot])
        if p is None: return None
        for name in ('kHpDefinitionsRoot','kHpDefinitionsTables','kHpDefinitionsObject'):
            p = self.pointer(p+self.layout[name])
            if p is None: return None
        return self.value(p+self.layout['kHpDefinitionId'],'<H')
    def object_at(self,address,vt):
        p = self.pointer(address)
        return p if p is not None and self.value(p,'<Q')==self.base+self.rv[vt] else None
    def actor_status(self):
        world = self.pointer(self.base+self.rv['kWorldSystemSlot'])
        manager = self.object_at(world+self.rv['kWorldSystem_ActorManager'],'kVtClientActorManager') if world else None
        actor = self.object_at(manager+self.rv['kActorManager_ControlledActor'],'kVtControlledActor') if manager else None
        table = self.pointer(actor+self.layout['kActor_ComponentTable']) if actor else None
        status = self.object_at(table+self.layout['kComponents_Status'],'kVtStatusComponent') if table else None
        return actor,status
    def stats_source(self):
        actor,status=self.actor_status()
        container=self.pointer(status+self.layout['kStatus_StatContainer']) if status else None
        entries=self.pointer(container+self.layout['kStatContainer_Entries']) if container else None
        count=self.value(container+self.layout['kStatContainer_Count'],'<I') if container else None
        return actor,status,container,entries,count
    def sample(self):
        ids = [self.hp_id(s) for s in ('kHpDefinitionsSlot','kHpDefinitionsAltSlot')]
        actor,status,container,entries,count = self.stats_source()
        result = dict(kind='candidate_hp_trace',normal_id=ids[0],alternate_id=ids[1],
                      coherent=False,actor=hex(actor) if actor else None,status=hex(status) if status else None)
        if entries is None or count is None or count>64 or entries>0x7FFFFFFFFFFF-64*self.layout['kStatEntry_Size']:
            result['reason'] = 'definition_or_actor_not_ready'
            return result
        matches = {str(key):[] for key in ids if key is not None}
        for index in range(count):
            entry = entries+index*self.layout['kStatEntry_Size']
            key = self.value(entry+self.layout['kStatEntry_Id'],'<H')
            if key is None:
                result['reason'] = 'unreadable_entry'
                return result
            if str(key) in matches:
                current = self.value(entry+self.layout['kStatEntry_Current'],'<q')
                maximum = self.value(entry+self.layout['kStatEntry_Max'],'<q')
                matches[str(key)].append(dict(current_raw=current,maximum_raw=maximum))
        stable = ids==[self.hp_id(s) for s in ('kHpDefinitionsSlot','kHpDefinitionsAltSlot')] and (actor,status,container,entries,count)==self.stats_source()
        valid = all(len(values)==1 and values[0]['current_raw'] is not None and values[0]['maximum_raw'] is not None and
                    0<=values[0]['current_raw']<=values[0]['maximum_raw'] and values[0]['maximum_raw']>0
                    for values in matches.values())
        reason='changed_during_sample' if not stable else 'definition_not_ready' if not matches else 'invalid_or_ambiguous_entry' if not valid else 'observational_only'
        result.update(entries=count,matches=matches,coherent=stable and valid and bool(matches),reason=reason)
        return result

class LoadingView(View):
    def chain(self):
        world=self.pointer(self.base+self.rv['kWorldSystemSlot'])
        manager=self.object_at(world+self.layout['kWorldSystem_ActorManager'],'kVtClientActorManager') if world else None
        actor=self.object_at(manager+self.layout['kActorManager_ControlledActor'],'kVtControlledActor') if manager else None
        table=self.pointer(actor+self.layout['kActor_ComponentTable']) if actor else None
        control=self.object_at(table+self.layout['kComponents_CharacterControl'],'kVtCharacterControl') if table else None
        inner=self.pointer(control+self.layout['kControl_Inner']) if control else None
        physics=self.object_at(inner+self.layout['kControlInner_Physics'],'kVtPhysicsObject') if inner else None
        return world,manager,actor,table,control,inner,physics
    def vector(self,address):
        raw=self.read(address,12)
        if raw is None: return None
        values=struct.unpack('<3f',raw)
        return values if all(math.isfinite(v) for v in values) else None
    def sample(self):
        chain=self.chain()
        result=dict(kind='candidate_loading_trace',coherent=False,reason='chain_not_ready')
        if not all(chain): return result
        physics=chain[-1]
        local=self.vector(physics+self.layout['kPhysics_Position'])
        origin=self.vector(self.base+self.rv['kChunkOrigin'])
        axes=[self.vector(physics+self.layout[key]) for key in ('kPhysics_AxisX','kPhysics_AxisY','kPhysics_AxisZ')]
        static=self.vector(self.base+self.rv['kStaticPlayerPos'])
        if local is None or origin is None or any(v is None for v in axes):
            result['reason']='unreadable_or_nonfinite_pose'
            return result
        if chain!=self.chain():
            result['reason']='chain_changed_during_sample'
            return result
        world=tuple(a+b for a,b in zip(local,origin))
        # Report a hypothesis, never turn it into production readiness evidence.
        parked=abs(world[0])<5 and abs(world[2])<5 and abs(world[1]-1000)<5
        result.update(coherent=True,reason='observational_only',actor=hex(chain[2]),
            position=world,local_position=local,chunk_origin=origin,axes=axes,
            static_position=static,candidate_legacy_park=parked)
        return result

def self_test():
    rv,layout,identity = constants()
    memory = {}
    def put(address,fmt,value):
        for i,b in enumerate(struct.pack(fmt,value)): memory[address+i]=b
    def read(address,size):
        try: return bytes(memory[address+i] for i in range(size))
        except KeyError: return None
    base=identity['kImageBase']
    view=View(read,base,rv,layout)
    assert view.hp_id('kHpDefinitionsSlot') is None
    for slot in ('kHpDefinitionsSlot','kHpDefinitionsAltSlot'): put(base+rv[slot],'<Q',0x100000)
    put(0x100000+layout['kHpDefinitionsRoot'],'<Q',0x110000)
    put(0x110000+layout['kHpDefinitionsTables'],'<Q',0x120000)
    put(0x120000+layout['kHpDefinitionsObject'],'<Q',0x130000)
    put(0x130000+layout['kHpDefinitionId'],'<H',37)
    assert view.hp_id('kHpDefinitionsSlot')==37  # no hardcoded zero
    put(base+rv['kWorldSystemSlot'],'<Q',0x200000)
    put(0x200000+rv['kWorldSystem_ActorManager'],'<Q',0x210000)
    put(0x210000,'<Q',base+rv['kVtClientActorManager'])
    put(0x210000+rv['kActorManager_ControlledActor'],'<Q',0x220000)
    put(0x220000,'<Q',base+rv['kVtControlledActor'])
    put(0x220000+layout['kActor_ComponentTable'],'<Q',0x230000)
    put(0x230000+layout['kComponents_Status'],'<Q',0x240000)
    put(0x240000,'<Q',base+rv['kVtStatusComponent'])
    put(0x240000+layout['kStatus_StatContainer'],'<Q',0x250000)
    put(0x250000+layout['kStatContainer_Entries'],'<Q',0x260000)
    put(0x250000+layout['kStatContainer_Count'],'<I',1)
    put(0x260000+layout['kStatEntry_Id'],'<Q',0xABCD000000000025) # dirty padding, ID37
    put(0x260000+layout['kStatEntry_Current'],'<q',123000)
    put(0x260000+layout['kStatEntry_Max'],'<q',300000)
    row=view.sample()
    assert row['coherent'] and row['matches']['37'][0]['current_raw']==123000
    def moving(address,size):
        raw=read(address,size)
        if address==0x260000+layout['kStatEntry_Current']:
            put(0x250000+layout['kStatContainer_Entries'],'<Q',0x270000)
        return raw
    assert not View(moving,base,rv,layout).sample()['coherent']
    put(0x250000+layout['kStatContainer_Entries'],'<Q',0x260000)
    put(0x250000+layout['kStatContainer_Count'],'<I',65)
    assert not view.sample()['coherent']
    put(0x250000+layout['kStatContainer_Count'],'<I',1)
    put(0x240000,'<Q',0xBAD)
    assert not view.sample()['coherent']
    print('P1 pure logic: dynamic ID, dirty padding, bounds, bad vtable, missing pointer PASS; no process opened')
    loading=LoadingView(read,base,rv,layout)
    put(0x230000+layout['kComponents_CharacterControl'],'<Q',0x280000)
    put(0x280000,'<Q',base+rv['kVtCharacterControl'])
    put(0x280000+layout['kControl_Inner'],'<Q',0x290000)
    put(0x290000+layout['kControlInner_Physics'],'<Q',0x300000)
    put(0x300000,'<Q',base+rv['kVtPhysicsObject'])
    def vector(at,values):
        for i,value in enumerate(values): put(at+4*i,'<f',value)
    vector(base+rv['kChunkOrigin'],(10,20,30))
    vector(base+rv['kStaticPlayerPos'],(11,22,33))
    vector(0x300000+layout['kPhysics_Position'],(1,2,3))
    for key,values in zip(('kPhysics_AxisX','kPhysics_AxisY','kPhysics_AxisZ'),((1,0,0),(0,1,0),(0,0,1))):
        vector(0x300000+layout[key],values)
    assert loading.sample()['position']==(11,22,33)
    vector(0x300000+layout['kPhysics_Position'],(-10,980,-30))
    assert loading.sample()['candidate_legacy_park']
    vector(0x300000+layout['kPhysics_Position'],(1,float('nan'),3))
    assert not loading.sample()['coherent']
    vector(0x300000+layout['kPhysics_Position'],(1,2,3))
    def changing_pose(address,size):
        raw=read(address,size)
        if address==0x300000+layout['kPhysics_Position']:
            put(0x210000+layout['kActorManager_ControlledActor'],'<Q',0)
        return raw
    assert LoadingView(changing_pose,base,rv,layout).sample()['reason']=='chain_changed_during_sample'
    assert not loading.sample()['coherent']
    put(0x210000+layout['kActorManager_ControlledActor'],'<Q',0x220000)
    put(0x300000,'<Q',0xBAD)
    assert not loading.sample()['coherent']
    print('Loading pure logic: origin+local, park hypothesis, NaN, changed identity, missing actor, bad physics type PASS; no process opened')

class Remote:
    class Region(C.Structure):
        _fields_=[('base',C.c_void_p),('allocation',C.c_void_p),('allocation_protect',W.DWORD),
                  ('partition',W.WORD),('size',C.c_size_t),('state',W.DWORD),('protect',W.DWORD),('type',W.DWORD)]
    def __init__(self,pid,expected_path=EXPECTED):
        self.api=C.WinDLL('kernel32',use_last_error=True)
        self.api.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD]; self.api.OpenProcess.restype=W.HANDLE
        self.api.CloseHandle.argtypes=[W.HANDLE]; self.api.CloseHandle.restype=W.BOOL
        self.api.VirtualQueryEx.argtypes=[W.HANDLE,C.c_void_p,C.POINTER(self.Region),C.c_size_t]
        self.api.VirtualQueryEx.restype=C.c_size_t
        self.api.ReadProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)]
        self.api.ReadProcessMemory.restype=W.BOOL
        self.api.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)]; self.api.GetExitCodeProcess.restype=W.BOOL
        self.api.QueryFullProcessImageNameW.argtypes=[W.HANDLE,W.DWORD,W.LPWSTR,C.POINTER(W.DWORD)]
        self.api.QueryFullProcessImageNameW.restype=W.BOOL
        self.handle=self.api.OpenProcess(0x410,False,pid) # VM_READ | QUERY_INFORMATION only
        if not self.handle: raise C.WinError(C.get_last_error())
        name=C.create_unicode_buffer(32768); length=W.DWORD(len(name))
        if not self.api.QueryFullProcessImageNameW(self.handle,0,name,C.byref(length)):
            self.close(); raise RuntimeError('Cannot verify process executable')
        if str(Path(name.value).resolve()).casefold()!=str(Path(expected_path).resolve()).casefold():
            self.close(); raise RuntimeError('Executable path mismatch')
    def close(self):
        if self.handle: self.api.CloseHandle(self.handle); self.handle=None
    def alive(self):
        value=W.DWORD()
        return bool(self.api.GetExitCodeProcess(self.handle,C.byref(value))) and value.value==259
    def read(self,address,size):
        if not 0<size<=4096 or address<0x10000 or address>0x7FFFFFFFFFFF-size: return None
        cursor=address
        while cursor<address+size:
            region=self.Region()
            if not self.api.VirtualQueryEx(self.handle,C.c_void_p(cursor),C.byref(region),C.sizeof(region)): return None
            if region.state!=0x1000 or region.protect&0x101 or not region.protect&0xEE: return None
            end=int(region.base or 0)+region.size
            if end<=cursor: return None
            cursor=end
        buffer=C.create_string_buffer(size); got=C.c_size_t()
        if not self.api.ReadProcessMemory(self.handle,C.c_void_p(address),buffer,size,C.byref(got)) or got.value!=size: return None
        return buffer.raw

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test',action='store_true')
    parser.add_argument('--api-self-test',action='store_true')
    parser.add_argument('--pid',type=int)
    parser.add_argument('--seconds',type=int,default=600)
    parser.add_argument('--mode',choices=('hp','loading'),default='hp')
    args=parser.parse_args()
    if args.self_test: self_test(); return
    if args.api_self_test:
        import os
        buffer=C.create_string_buffer(b'P1 read-only API fixture')
        remote=Remote(os.getpid(),Path(sys.executable))
        try:
            assert C.sizeof(Remote.Region)==48
            assert remote.read(C.addressof(buffer),len(buffer))==buffer.raw
            assert remote.read(0,8) is None
            assert remote.read(C.addressof(buffer),4097) is None
            assert remote.read(0x7FFFFFFFFFFF,8) is None
            assert remote.alive()
        finally: remote.close()
        print('P1 Windows API self-test PASS: own process only, read/query handle, guarded reads and bounds')
        return
    if not args.pid or args.pid<1 or not 1<=args.seconds<=900: parser.error('PID and bounded duration required')
    with EXPECTED.open('rb') as stream: digest=hashlib.file_digest(stream,'sha256').hexdigest()
    if digest!=HASH: raise RuntimeError('Installed build drift')
    rv,layout,identity=constants(); remote=Remote(args.pid)
    try:
        base=identity['kImageBase']
        dos=remote.read(base,64)
        if dos is None or dos[:2]!=b'MZ': raise RuntimeError('Mapped base/PE mismatch')
        pe=struct.unpack_from('<I',dos,60)[0]
        if not 64<=pe<=1024: raise RuntimeError('Invalid PE offset')
        header=remote.read(base+pe,24+60)
        if header is None or header[:4]!=b'PE\0\0' or struct.unpack_from('<H',header,4)[0]!=0x8664 or struct.unpack_from('<H',header,24)[0]!=0x20B or struct.unpack_from('<I',header,8)[0]!=identity['kPeTimestamp'] or struct.unpack_from('<I',header,80)[0]!=identity['kSizeOfImage']:
            raise RuntimeError('Mapped identity mismatch')
        question='Which native HP definition selects the local status entry?' if args.mode=='hp' else 'Does the candidate local-player chain expose a distinct loading/park transition?'
        print(json.dumps(dict(kind='passive_external_read_only',mode=args.mode,pid=args.pid,sha256=digest,base=hex(base),
              production_tiers='unarmed',question=question,
              access_mask='0x410',hooks=False,writes=False,native_calls=False)),flush=True)
        view=(View if args.mode=='hp' else LoadingView)(remote.read,base,rv,layout)
        start=time.monotonic(); last=None; last_log=-1
        while time.monotonic()-start<args.seconds and remote.alive():
            now=time.monotonic()-start; row=view.sample()
            if row!=last or now-last_log>=1:
                print(json.dumps(dict(seconds=round(now,3),**row)),flush=True); last=row; last_log=now
            time.sleep(.25)
        print(json.dumps(dict(kind='trace_end',seconds=round(time.monotonic()-start,3),alive=remote.alive())),flush=True)
    finally: remote.close()

if __name__=='__main__':
    main()
