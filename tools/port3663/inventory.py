"""Conservative, reproducible source inventory; never emits adopted addresses.

Run from the repository root: python tools/port3663/inventory.py
Includes core implementations and their recursively included project headers.
Extras reached through headers are deliberately retained as an audit superset.
This lexical inventory is not a proof of runtime reachability or RE semantics.
"""
from pathlib import Path
import json
import re

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/probe-3663'
SEEDS = '''mod/mod.cpp mod/config.cpp mod/dllmain.cpp
mod/game/player_reader.cpp mod/game/safe_memory.cpp mod/game/rtti.cpp
mod/game/actor_lookup.cpp mod/game/hook_prepare.cpp mod/game/game_fault.cpp
mod/game/game_errors.cpp mod/net/steam_transport.cpp mod/net/troc_sniffer.cpp
mod/net/remote_puppet.cpp mod/session2/session2.cpp mod/session2/server.cpp
mod/session2/guest.cpp mod/session2/guest_lifecycle.cpp
mod/session2/field_share.cpp mod/session2/identity_fix.cpp
mod/session2/identity_probe.cpp
mod/session2/report_guard.cpp mod/session2/save_hooks.cpp
mod/session2/guest_save_guest.cpp mod/session2/guest_save_host.cpp
mod/session2/guest_save_tunnel.cpp mod/session2/host_save_export.cpp
mod/session2/clock_gate.cpp mod/session2/guest_reload_wait.cpp
mod/session2/slot_refresh.cpp mod/session2/load_trip.cpp
common/protocol.cpp common/client_session.cpp common/reliable.cpp
common/udp.cpp common/send_gate.cpp common/troc_frame.cpp common/troc_link.cpp
common/save_guard.cpp'''.split()


def masked(text):
    # Preserve positions/newlines, removing strings and comments before scanning.
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|(?<![\w])\'(?:\\.|[^\'\\\n])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def collect():
    pending = [ROOT / 'src' / p for p in SEEDS]
    files = set()
    while pending:
        path = pending.pop()
        if path in files or not path.is_file():
            continue
        files.add(path)
        for inc in re.findall(r'^\s*#include "([^"]+)"', path.read_text(encoding='utf-8'), re.M):
            candidate = ROOT / 'src' / inc
            if candidate.is_file():
                pending.append(candidate)
    return sorted(files)


def esc(value):
    return str(value).replace('|', '&#124;').replace('\n', ' ').strip()


def main():
    rows, literals = [], []
    files = collect()
    for path in files:
        text = path.read_text(encoding='utf-8')
        code = masked(text)
        rel = path.relative_to(ROOT).as_posix()
        # Semicolon-terminated constexpr definitions, including byte signatures.
        defs = list(re.finditer(r'\bconstexpr\s+[^;{}=]+?\s+(k\w+)\s*(?:\[\])?\s*=\s*([\s\S]*?);', code))
        occupied = set()
        for definition in defs:
            first = code.count('\n', 0, definition.start()) + 1
            last = code.count('\n', 0, definition.end()) + 1
            occupied.update(range(first, last + 1))
            # Multiple declarations on one statement each get their own row.
            body = definition.group(2)
            pieces = [(definition.group(1), body)]
            if not body.lstrip().startswith('{'):
                pieces = []
                for i, part in enumerate(body.split(',')):
                    if i == 0:
                        pieces.append((definition.group(1), part))
                    else:
                        match = re.match(r'\s*(k\w+)\s*=\s*(.*)', part)
                        if match:
                            pieces.append((match[1], match[2]))
            for symbol, value in pieces:
                kind = ('signature' if value.lstrip().startswith('{') else
                        'vtable' if 'Vtable' in symbol or 'Vt' in symbol else
                        'Troc ID (audit)' if symbol.startswith('kOp') else
                        'RVA / layout / policy (audit)')
                context = text.splitlines()[first - 1].strip()
                rows.append(dict(id=f'N{len(rows)+1:04}', source=f'{rel}:{first}', symbol=symbol,
                                 kind=kind, value=' '.join(value.split()), use=context))
        symbol = '(file scope)'
        for line_no, line in enumerate(code.splitlines(), 1):
            function = re.search(r'\b(\w+(?:::\w+)*)\s*\([^;]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{', line)
            if function and function[1] not in {'if', 'for', 'while', 'switch', 'catch'}:
                symbol = function[1]
            if line_no in occupied or line.lstrip().startswith('#'):
                continue
            for number in re.finditer(r'(?<![\w.])(?:0[xX][0-9a-fA-F\']+|\d[\d\']*(?:\.\d+)?)(?:[uUlLfF]+)?(?![\w.])', line):
                literals.append(dict(id=f'L{len(literals)+1:05}', source=f'{rel}:{line_no}:{number.start()+1}',
                                     symbol=symbol, kind='inline literal (semantic audit required)', value=number[0],
                                     use=text.splitlines()[line_no - 1].strip()))
    OUT.mkdir(parents=True, exist_ok=True)
    data = dict(scope='Conservative lexical superset, not a completed semantic port map',
                files=[p.relative_to(ROOT).as_posix() for p in files], named=rows, literals=literals)
    (OUT / 'PORT-INVENTORY.json').write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    header = '''# 2976 core-path port inventory

Generated with `python tools/port3663/inventory.py` from the explicitly listed
core implementation seeds and their recursive project-header includes.
The validated walk-around uses option D (`session2`); `remote_puppet` supplies
shared helpers. Sources: README "Upstream co-op behavior", ROADMAP,
OPCION-D-EMULADOR, `session2::install`, `server_install`, and `guest_install`.
`kit/cdmp.ini` is a capture-only preset, not the validated multiplayer preset.

Minimal target: identity/startup, guarded player reads, UDP/Steam transport,
bounded handshake/tunnel, host session allocation/accept, guest remote login,
native movement relay, character lifecycle, reconnect, and save protection.
Preserve the Steam-friends restriction and the process executable filter.

This is a conservative inventory, not evidence that every row is needed or
verified. All named values in the scanned files are retained; raw inline
literals (including ordinary policy bounds) are in PORT-INVENTORY-LITERALS.md.
The JSON records exact source locations and the complete file list. Separate
semantic/reachability review is still required before any row is adopted.

2976 verification: the address headers cite `hallazgos-en-vivo.md`,
`opcion-d-d0.md`, `segunda-sesion.md`, and `troc-dispatch.md`; local implementation
comments cite further research. Those are historical author claims. They have
not been individually revalidated in this run. The 2976 reference is now
available with its required hash; per-row semantic audit remains. Each row
below therefore has **historical source only; revalidation pending**, and its
use column quotes its source statement rather than inventing a meaning.

Disabled on 3663: mounts/horse/ride, rope/tree, shops, docked gimmicks, NPC/enemy
relay/control, combat/damage probes, observer scene/schedule extras, map/minimap,
overlay, and every other optional session2 feature. Current production startup
denies ALL 3663 gameplay; config cannot reach these installers. Parent switches
`coop_enabled` and `s2_enabled` default false. Individual legacy suboptions may
default true on 2976, so they must not be reused as a 3663 preset.

No existing 2976 path or config defaults are changed by this inventory.

'''
    for filename, heading, entries in [
        ('PORT-INVENTORY.md', header, rows),
        ('PORT-INVENTORY-LITERALS.md', '# Inline-literal audit supplement\n\nSee PORT-INVENTORY.md for scope and evidence limits. These rows need semantic review.\n\n', literals),
    ]:
        table = '| Row | File:line | Symbol/context | Kind | 2976 value | Source use |\n|---|---|---|---|---|---|\n'
        table += ''.join('| ' + ' | '.join(esc(row[k]) for k in ['id', 'source', 'symbol', 'kind', 'value', 'use']) + ' |\n' for row in entries)
        (OUT / filename).write_text(heading + f'{len(entries)} rows; {len(files)} scanned files.\n\n' + table, encoding='utf-8')
    print(f'{len(rows)} named rows, {len(literals)} inline literals, {len(files)} files; no adopted values')


if __name__ == '__main__':
    main()
