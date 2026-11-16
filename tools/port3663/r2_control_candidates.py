"""Join only core control descriptors for native review; does not adopt them."""
import json
from pathlib import Path
folder=Path(__file__).resolve().parents[2]/'build/r2'
names=('LogoutCharacterToFieldServerAck','ShutDownServerOnceTimer','OnLogoutXxxToServerReq',
    'ConnectToServerAck','DisconnectToServerAck','OnConnectToServerReq','OnDisconnectToServerReq')
rows=[]
for image in ('old','new'):
    rtti=json.loads((folder/f'tunnel-control-rtti-{image}.json').read_text())
    types={row['name']:row for row in rtti['types']}
    descriptors=json.loads((folder/f'tunnel-control-descriptors-{image}.json').read_text())
    assert rtti['identity']['sha256']==descriptors['identity']['sha256']
    for short in names:
        name=f'.?AVTrocTr{short}@pa@@'
        row,=[r for r in descriptors['descriptors'] if r['name']==name]
        table,=[t for t in types[name]['vtables'] if t['rva']==row['vtable']]
        slot,=[s for s in table['slots'] if s['index']==2]
        rows.append(dict(image=image,short=short,handler=slot['rva'],**row))
(folder/'tunnel-control-candidates.json').write_text(json.dumps(rows,indent=2)+'\n')
print('Core native control candidates',len(rows),'; not adoption')
