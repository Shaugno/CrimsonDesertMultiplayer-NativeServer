# R2-P1-20261116-100800 — passive P1, prepared before launch

Question: Does the candidate local-player chain expose a distinct missing-chain or parked-position transition during loading and fast travel on exact 3663?

Expected observation: The guarded candidate position and rotation follow the controllable character, then loading produces a missing chain or a distinctive park interval before placement. The old (0,1000,0) park is only a logged hypothesis, not an adopted readiness rule.

Duration: ten minutes from collector start, at most fifteen minutes for the sitting.

Steps: after Shaugn confirms presence, deploy the closed-guard ASI; launch the game visibly. Start collection at the title screen, select Continue, wait for control, walk a few steps and turn, then use one ordinary fast travel; wait for placement and control, walk and turn again, and quit to desktop at the stop signal.

Stop: any crash, hang, identity drift, user request, or time limit. Never retry a crash blindly. Collect cdmp.log and crash context, fix offline and rerun the suite before another session.

Collector: external PROCESS_VM_READ | PROCESS_QUERY_INFORMATION only. Every copy checks VirtualQueryEx and ReadProcessMemory. No hooks, game writes, native calls, packet replay or network feed. This is not L1 and does not arm Tier A.

Supervisor: before collection, arm r2_session_watchdog.ps1 for the exact game PID. It pins the process handle and closes that test process at fourteen minutes from launch if normal quit has not completed. This separate deadline control does not grant the collector memory-write rights.

Save policy: all saves backed up with a local hash manifest and verified before launch. Afterward verify the backup and compare the live save folder; report normal game changes without overwriting player progress. Never commit saves or the private manifest.

Installed executable SHA256: 8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8
Backup files: 23
Backup manifest SHA256: 096D77AFF0E61E488EFA8CE463FCF5D6137115324CD0BB10F51EA23A7EFD29E5
