"""Join named native request descriptors for targeted code review, not adoption."""
import json
from pathlib import Path

root=Path(__file__).resolve().parents[2]
folder=root/'build/r2'
names=('ConnectCryptogramToServerReq','LoginUserToFieldServerReq','LoginCharacterToFieldServerReq',
    'MainPlayerDataStartAck','MainUserDataStartAck','MoveFieldAck','ScopeSenderAck','ActorLoadDataAck',
    'CharacterLobbyDataAck','SetFieldTimeReq','ReserveShutDownServerReq','SaveAutoReq',
    'SaveToFileCurrentPlayerReq','SaveToFileCurrentPlayerAck','CreateCharacterReq','NewGameStartReq',
    'LogoutSessionToServerReq','LogoutSessionToServerAck','LogoutUserToServerAck')
output=[]
for image in ('old','new'):
    rtti=json.loads((folder/f'request-rtti-{image}.json').read_text())
    descriptors=json.loads((folder/f'request-descriptors-{image}.json').read_text())
    types={r['name']:r for r in rtti['types']}
    for short in names:
        name=f'.?AVTrocTr{short}@pa@@'
        matches=[r for r in descriptors['descriptors'] if r['name']==name]
        assert len(matches)==1,(image,name,matches)
        descriptor=matches[0]
        table,=[t for t in types[name]['vtables'] if t['rva']==descriptor['vtable']]
        slot,=[s for s in table['slots'] if s['index']==2]
        output.append(dict(image=image,**descriptor,handler=slot['rva'],
            pending='Verify initializer instruction boundaries/store and handler role; descriptor numbers alone do not prove payload layout'))
(folder/'request-candidates.json').write_text(json.dumps(output,indent=2)+'\n')
for row in output:
    if row['image']=='new': print(row['name'],row['opcode'],row['initializer'],row['handler'])
