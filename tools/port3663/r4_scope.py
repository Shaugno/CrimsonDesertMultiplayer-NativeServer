"""Reproduce R4's native request IDs; never execute either game image.

Consumes the local R3 named-descriptor scan and R4 paired native exports.
Only summaries/hashes are published. A handler address here is provenance,
not permission to hook or call it.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
from pe_identity import PE

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--reference', required=True)
parser.add_argument('--target', default=r'D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
expected = {'old': '57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7',
            'new': '8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8'}
contracts = {
    'ReloadStageStartReq': (2283, 3018, 'Fast-travel/reload request; decoded arguments reach 2B586B0 and its mover preparation. Refuse every guest payload.'),
    'MoveFieldCheatReq': (2838, 2647, 'Field-move cheat request. Target handler decodes 4/12/1/4 and currently returns success without a world mutation; refuse by v1 scope anyway.'),
    'SetFieldTimeReq': (2125, 2150, 'Time publication request. Target thunk reaches native TransformSync at actor components+1A0 and consumer 2ED4D20. Refuse every guest payload; no payload layout guessed.'),
    'AITeleportReq': (2091, 2236, 'AI teleport control, with actor/owner validation before its native teleport component. Optional AI control is outside the walking/turning v1 scope.'),
}

def sections(path):
    text = path.read_text(encoding='utf-8-sig')
    parts = re.split(r'^RVA=(0x[0-9A-Fa-f]+)\s*$', text, flags=re.M)
    return {int(parts[i], 0): parts[i + 1].lower() for i in range(1, len(parts), 2)}

records = {name: {'policy': text, 'native': {}} for name, (_, _, text) in contracts.items()}
proofs = []
for image, path in [('old', args.reference), ('new', args.target)]:
    pe = PE(path)
    assert pe.sha256 == expected[image], 'Image drift'
    catalog_path = root / f'build/r3/troc-descriptors-{image}.json'
    catalog = json.loads(catalog_path.read_text(encoding='utf-8'))
    assert catalog['identity']['sha256'] == pe.sha256
    export_path = root / f'build/r4/scope-{image}-native.txt'
    exports = sections(export_path)
    for name, (old_id, new_id, _) in contracts.items():
        row, = [r for r in catalog['descriptors'] if r['name'] == f'.?AVTrocTr{name}@pa@@']
        opcode = old_id if image == 'old' else new_id
        assert row['kind'] == 4 and row['opcode'] == opcode
        instance, vtable, initializer = (int(row[k], 0) for k in ('instance', 'vtable', 'initializer'))
        assert pe.u16(pe.offset(instance + 12, 2)) == opcode
        handler = pe.u64(pe.offset(vtable + 16, 8)) - pe.image_base
        assert pe.executable(handler)
        init = exports[initializer]
        assert f'lea rax,[0x{pe.image_base + vtable:x}]' in init
        assert f'mov qword ptr [0x{pe.image_base + instance:x}],rax' in init
        body = exports[handler]
        assert 'function=' in body and 'blocked' not in body
        assert '+ 3)' in body and ('- 5 ==' in body or 'sub rax,0x5' in body)
        records[name]['native'][image] = {**{k: row[k] for k in ('opcode', 'kind', 'vtable', 'initializer', 'instance')},
                                         'handler': f'0x{handler:X}'}
    if image == 'new':
        assert 'fun_142b586b0(' in exports[0x2D55170]
        assert '+ 0x1a0)' in exports[0x2CA68F0] and 'fun_142ed4d20(' in exports[0x2CA68F0]
        assert 'fun_142dbf5b0(' in exports[0x2C60210]
    for proof in (catalog_path, export_path):
        proofs.append({'path': str(proof.relative_to(root)), 'bytes': proof.stat().st_size,
                       'sha256': hashlib.sha256(proof.read_bytes()).hexdigest().upper()})

consumer = root / 'build/r4/scope-reload-consumer-new.txt'
body = sections(consumer)
assert 'fun_142ea0920(' in body[0x2B586B0]
proofs.append({'path': str(consumer.relative_to(root)), 'bytes': consumer.stat().st_size,
               'sha256': hashlib.sha256(consumer.read_bytes()).hexdigest().upper()})
out = root / 'docs/probe-3663/port-evidence/r4/scope-result.json'
out.write_bytes((json.dumps({'step': 2, 'identities': expected, 'requests': records, 'proofs': proofs,
    'retired_by_scope': [f'C{i:03}' for i in range(110, 116)],
    'loading_requests_not_refused': {'LoginUser': 2554, 'LoginCharacter': 3081,
        'GameLoadingStart': 2350, 'MoveFieldComplete': 2723, 'LoadingComplete': 2789, 'PreLoadSequencer': 2372},
    'adoption': 'Native IDs and refusal policy only. No function RVA, payload rewrite, legacy byte patch or native call adopted.'}, indent=2) + '\n').encode('utf-8'))
header = '''#pragma once
// Native IDs only, not handler addresses or permission to replay requests.
// Reproduced by tools/port3663/r4_scope.py from paired descriptor/handler evidence.
#include <array>
#include <cstdint>
namespace cdmp::session2::scope3663 {
'''
for name, (_, opcode, _) in contracts.items():
    header += f'inline constexpr uint16_t k{name} = {opcode};\n'
header += '''inline constexpr std::array<uint16_t,4> kRefusedRequests{
    kReloadStageStartReq,kMoveFieldCheatReq,kSetFieldTimeReq,kAITeleportReq};
}
'''
(root / 'src/mod/session2/scope3663.h').write_bytes(header.encode('utf-8'))
print('R4: four native request IDs verified; guest control refusal; six scope retirements.')
