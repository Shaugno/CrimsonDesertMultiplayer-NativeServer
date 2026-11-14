# Phase 2 — option-D scope and audit boundary

The selected path is session2/option D. README lines 48–65 attributes the
upstream live results to ByJunter. The older OPCION-D-EMULADOR.md describes
the transition from independent-world puppets and its later status additions;
it is not evidence that the default capture kit enables multiplayer.

| Required behavior | Implementation to port | 3663 status |
|---|---|---|
| Startup, executable filter, guarded reads | dllmain, build_guard, safe_memory, player_reader | Gameplay denied pending matches |
| Steam friends or UDP, session protocol | steam_transport, udp, protocol, client_session, reliable, send_gate | Game-free synthetic path available; in-game denied |
| Ordered native tunnel | troc_frame, troc_link, session2, guest | Native path denied |
| Guest remote login and host session | server, guest, offsets_s2, field_share, identity_fix | Native path denied |
| Native movement and actor creation | server recipient finders, guest delivery, remote_puppet shared helpers | Native path denied; no handcrafted spawn replacement |
| Removal, disconnect, reconnect | guest_lifecycle, session2, server, guest/rejoin | Native path denied |
| Save isolation and guest protection | save_hooks, report_guard, guest_save_*, host_save_export, title refresh | Native path denied |

`mod.cpp` obtains allow_reads and allow_hooks from the central startup decision
before deriving game_build_ok. session2, guest and every game-address installer
are reached through that permission. The 3663 identity is currently unsupported,
so all those permissions and in-game networking are false. A guest-role process
ends before the legacy save redirect can be installed. The unit guard tests
exercise these callback boundaries using synthetic PE headers.

Disabled on 3663: mounts/horse/ride; rope/tree; shops; docked gimmicks; NPC/enemy
relay, schedules, scenes, combat and damage probes; minimap/map/overlay extras;
all remaining optional session2 behavior. Both top-level coop/session2 config
switches default false. The explicit startup denial also applies if an ini
enables them. Do not change the legacy defaults on 2976. Before a future 3663
table can arm, it needs its own minimal feature policy and must not inherit
2976 suboptions that default true.

PORT-INVENTORY.md, PORT-INVENTORY-LITERALS.md and PORT-INVENTORY.json enumerate
named constants and inline scalar occurrences conservatively, preserving
file:line, context, value and source statement. Reproduce with
`python tools/port3663/inventory.py`. The seed list includes core implementations
and recursively included project headers. Disabled-feature headers remain in
the conservative superset because the legacy modules are coupled. A lexical
superset is not a completed semantic port inventory: reachability, unnamed
literal meaning and the per-row 2976 evidence still need human-readable audit.
These are explicitly unadopted rows, not guesses that offsets stayed unchanged.

No production path or config defaults changed in Phase 2. No game launch,
injection, memory reads, deployment or save changes. Phase 0's complete suite
remains the validation for the unchanged production gate.
