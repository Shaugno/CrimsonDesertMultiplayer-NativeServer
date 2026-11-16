"""Emit only manually proved layouts into the unarmed target header."""
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parents[2]
proofs = json.loads((root/'docs/probe-3663/r2-reviewed.json').read_text())
core = {r['r2']:r for r in json.loads((root/'docs/probe-3663/R2-CORE-LIST.json').read_text())}
header = root/'src/mod/game/build_3663.h'
text = header.read_text()
for row in core.values():
    if not row['adopted_r1']: continue
    pattern = r'(inline constexpr \w+ '+row['symbol']+r' = [^\n]+)'
    text = re.sub(pattern,lambda m: re.sub(r'; R2 [ABCD]\d+', '', m[1])+'; R2 '+row['r2'],text)
begin,end = '// R2 reviewed layouts BEGIN','// R2 reviewed layouts END'
if begin in text: text = text[:text.index(begin)] + text[text.index(end)+len(end):]
section = begin+'\n// Layout proof catalog: docs/probe-3663/r2-reviewed.json. No tier is armed.\n'
for tier,namespace in [('A','player_layout'),('B','tunnel_layout'),('C','field_layout')]:
    section += 'namespace '+namespace+' {\n'
    for key,row in proofs.items():
        if core[key]['tier']!=tier: continue
        symbol = row['symbol'] if row['symbol'].startswith('k') else 'k'+row['symbol']
        if row['symbol']=='FieldUserEntryLayout':
            for name,value in [('Size','16'),('User','0'),('Actors','8')]:
                section += f'inline constexpr uint32_t kFieldUserEntry{name} = {value}; // 2976 {value}; R2 {key}\n'
            continue
        value = row['target']
        if value.startswith('{'):
            assert re.fullmatch(r'\{0x[0-9A-F]{2}(?:, 0x[0-9A-F]{2})*\}',value),value
            section += f'inline constexpr uint8_t {symbol}[] = {value}; // 2976 signature: {core[key]["source"]}; R2 {key}\n'
            continue
        assert re.fullmatch(r'0x[0-9A-Fa-f]+|[0-9]+|[0-9]+\.0f',value),value
        kind = 'float' if value.endswith('f') and '.' in value else 'uint64_t' if int(value,0)>0xFFFFFFFF else 'uint32_t'
        section += f"inline constexpr {kind} {symbol} = {value}; // 2976 {row['old']}; R2 {key}\n"
    section += '} // namespace '+namespace+'\n'
section += end+'\n\n'
text = text.replace('}  // namespace cdmp::game::build_3663',section+'}  // namespace cdmp::game::build_3663')
text = re.sub(r'\n{3,}', '\n\n', text)
header.write_text(text)
print('Wrote',len(proofs),'proved layout contracts; all completeness gates remain false')
