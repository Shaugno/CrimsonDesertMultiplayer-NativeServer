# R2-P1-20261116-072000 — passive P1, prepared before launch

Question: Which native HP definition key selects the local actor's status entry on the exact 3663 executable?

Expected observation: The native definition key selects one status entry whose current/max integer values, divided by the proved unit scale 1000, agree with the character's HP display. Both definition registries are logged separately; no ID is guessed.

Duration: ten minutes from collector start, at most fifteen minutes for the sitting.

Steps: after Shaugn confirms presence, deploy the closed-guard ASI; launch the game visibly. Select Continue and wait for control; show the character/status HP display and note current/max HP; let ordinary regeneration or a healing action change HP if available; quit to desktop at the stop signal.

Stop: any crash, hang, identity drift, user request, or time limit. Never retry a crash blindly. Collect cdmp.log and crash context, fix offline and rerun the suite before another session.

Collector: external PROCESS_VM_READ | PROCESS_QUERY_INFORMATION only. Every copy checks VirtualQueryEx and ReadProcessMemory. No hooks, game writes, native calls, packet replay or network feed. This is not L1 and does not arm Tier A.

Save policy: all saves backed up with a local hash manifest and verified before launch. Afterward verify the backup and compare the live save folder; report normal game changes without overwriting player progress. Never commit saves or the private manifest.

Installed executable SHA256: 8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8
Backup files: 23
Backup manifest SHA256: 57CD997A6AF309A5B93EA3324C9353E1BC0EF58D25F94D18D1760B84E3224406
