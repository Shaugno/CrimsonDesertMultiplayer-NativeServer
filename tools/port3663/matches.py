"""Render explicitly reviewed matches and a row-by-row unadopted audit queue."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/probe-3663'


def esc(text):
    return str(text).replace('|', '&#124;').replace('\n', ' ')


def main():
    inventory = json.loads((OUT / 'PORT-INVENTORY.json').read_text(encoding='utf-8'))
    reviewed = json.loads((OUT / 'PORT-REVIEWED.json').read_text(encoding='utf-8'))
    adopted = {r['symbol']: r for r in reviewed['matches']}
    count = 0
    header = '''# 2976 → 3663 port matches — incomplete

Static comparison only. Nothing in this report proves two-player behavior.
Both executable hashes are in PHASE1-REFERENCES.md. Phase 3 re-hashed the target:
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8.

17 reviewed values are adopted as static identities, all strong with a second
check. These are an incomplete table; no 3663 gameplay path is armed. The three
PE identity fields are separate file facts, not gameplay addresses. Unreviewed
rows below are blocked for adoption, including policy literals whose role has
not yet been classified; they are not all required gameplay offsets.

Methods: Ghidra 12.1.4 headless targeted disassembly/decompilation; Zydis 4.1.0
normalization of all decodable pdata functions; exact named RTTI/COL/table
chains; unique existing short signatures; independently checked call neighbours;
the existing strict pkt3663 entry verifier. Ghidra was used for targeted paired
functions, not a completed Version Tracking/BSim project-wide analysis.

242245 old / 247296 new pdata functions decoded; 2199 / 2290 refused.
57160 unique normalized candidates. Hashes mask relative branches and RIP
displacements; identical wrappers can call different functions, so no hash
match alone is adopted. 26 existing short signatures were checked in both
files, with only 5 unique in the target. All raw discovery outputs say adopted=0;
the explicit reviewed decisions below are the only promotion step.

Evidence: port-evidence/ghidra-2976-expanded.txt,
ghidra-3663-expanded.txt, ghidra-3663-core-handlers.txt,
ghidra-3663-refresh.txt, function-correlations.json, call-neighbors.json,
signature-candidates.json, RTTI JSON files and file-scan-3663.txt.
Tools and commands are in tools/port3663/README.md.

## Reviewed checks

'''
    for key, proof in reviewed['proofs'].items():
        header += f'- **{key}**: {proof}\n'
    header += '''
## Blocking core gaps

1. Field sharing H1/H2: old AcquireField 0x2B07790 and EnterField 0x2BC04F0
   have no unique normalized match. Call-neighbour ranking finds only one mapped
   common cleanup callee for each, and no corroborated target. Current field
   locks, user-list offsets and rollback contracts are unproved. Smallest next
   experiment: follow the named 3663 LoginCharacterToFieldServerReq handler
   (RTTI slot 2 at 0x2C6AE10) to field acquisition/entry, then corroborate both
   functions and their field-list writers against the 2976 paired bodies.
2. Identity/status: component-load 0x20688A0 has multiple call-graph candidates
   (first 0x22951B0); status phase 0x2941020 has many near-tied candidates.
   No second check validates the post-load identity hook/register contract,
   status number pointer or lobby/status agreement. Lobby slot 0x50 can be
   located by RTTI, but that alone does not validate the full identity path.
3. Player/SAM/root and reconnect barriers: current singleton provenance,
   controlled-character chain, HP convention, exact-key SAM lookup, per-field
   user/actor counts and native logout/removal completion still need independent
   3663 use-site checks. Reusing the 2976 cleanup map is not acceptable.
4. Wire/login/save completeness: new descriptors identify user login 2554,
   character login 3081, main-user 2420, main-player 2844, field complete 2723,
   refresh 3005 and cryptogram 2401. These remain unadopted IDs until their
   handler consumption and rewritten payloads are audited. In particular,
   cryptogram has kind 0x7FFFFFFE, not a generic server Req. Never copy old IDs,
   header meanings, frame tail/tag usage or payload sizes from 2976.
5. Guest save interception and early load ordering are not validated by the
   one matched refresh thunk. A recognized executable alone cannot safely arm
   the guest role or its 2976 hooks.

Fallback decision: coop3663's existing structural decoders and synthetic Feed
cannot provide native FieldManager sharing, identity fixups, initialized native
characters or deferred destruction. They do not cover these core gaps. Preserve
that code and its tests; do not expand it or present fake-backend results as a
native replacement. No bypass, guessed offset or weaker guard is introduced.

## Named inventory rows

Missing-evidence codes: **A** = classify the row's feature/reachability and,
if build-dependent, confirm its current 3663 code use plus an independent check;
**S** = current whole-instruction signature, verified hook boundary and role;
**T** = named current descriptor plus complete handler/writer consumption;
**V** = exact named type/COL/table plus independently matched slot semantics.
Each unresolved row below names the precise original source/symbol/value.

| Row | Source | Symbol | 2976 | 3663 | Method/check | Confidence | Decision / missing |
|---|---|---|---|---|---|---|---|
'''
    for row in inventory['named']:
        match = adopted.get(row['symbol']) if row['source'].split(':')[0] == reviewed['source'] else None
        if match:
            count += 1
            values = [row['id'], row['source'], row['symbol'], row['value'], match['value'], match['proof'], 'strong', 'adopted static identity; runtime unarmed']
        else:
            missing = {'signature': 'S', 'Troc ID (audit)': 'T', 'vtable': 'V'}.get(row['kind'], 'A')
            values = [row['id'], row['source'], row['symbol'], row['value'], 'unresolved', 'no approved second check', 'unreviewed/weak', f'blocked for adoption: {missing}']
        header += '| ' + ' | '.join(esc(x) for x in values) + ' |\n'
    if count != len(adopted):
        raise RuntimeError('Reviewed symbols do not match inventory uniquely')
    supplement = '# Inline-literal match audit\n\nEvery row remains unadopted. Missing: feature/reachability classification, semantic meaning, and current 3663 use-site plus independent check if build-dependent. See PORT-MATCHES.md.\n\n'
    supplement += '| Row | Source | Context | 2976 literal | 3663 | Decision |\n|---|---|---|---|---|---|\n'
    for row in inventory['literals']:
        supplement += '| ' + ' | '.join(esc(x) for x in [row['id'], row['source'], row['symbol'], row['value'], 'unresolved', 'blocked for adoption: A']) + ' |\n'
    (OUT / 'PORT-MATCHES.md').write_text(header, encoding='utf-8')
    (OUT / 'PORT-MATCHES-LITERALS.md').write_text(supplement, encoding='utf-8')
    counts = dict(named=len(inventory['named']), literals=len(inventory['literals']), adopted=count,
                  named_unadopted=len(inventory['named']) - count, literals_unadopted=len(inventory['literals']),
                  complete_core_table=False, gameplay_armed=False)
    (OUT / 'PORT-MATCH-COUNTS.json').write_text(json.dumps(counts, indent=2) + '\n')
    print(json.dumps(counts))


if __name__ == '__main__':
    main()
