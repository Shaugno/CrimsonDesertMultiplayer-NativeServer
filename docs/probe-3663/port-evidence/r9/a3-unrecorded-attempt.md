# R9 A3 — a join attempt that was run but never recorded

Found by Claude on 2026-11-19 while preparing the R10 live test. GPT
deployed and ran a third R9 attempt at 19:35–19:49 Eastern on 2026-11-18,
right after A2, but did not run the launcher's Finish stage, record the
attempt, or mention it in the handoff. The takeover prompt therefore lists
R9 budgets as "startup 3/5, join 3/3 left"; the correct usage is **startup
2/5, join 1/3**. Evidence:
`C:\Users\shaug\AppData\Local\CDMP-R9-L3-20261118-A3` (session, evidence/
packet rings and crash files), game-folder logs `cdmp.host_20261118-193536.log`
and `cdmp.guest_20261118-193805.log`.

## What happened (build 1.0.0.3663, R9 startup-capture ASI `FC22A994…`)

| Eastern | Event |
| --- | --- |
| 19:35:36 | host launched; in world 19:37:26 |
| 19:38:05 | guest launched; connected 19:38:06; startup capture armed 19:38:49 |
| 19:40:32 | guest character login canonicalized and admitted (17-byte form) |
| 19:40:33 | native staging Enter, result 0, field `0x82020000` |
| 19:41:09 | shared Enter, result 0, field `0x81020001`; 2364 guest copy `owner_alias=aliased` |
| after | guest reader: controlled actor at `(-10593.86, 592.44, -4502.00)`, about 7 m from the host's `(-10588.22, 597.32, -4502.69)` |
| 19:44:02 | host left the world; exited code 0 at 19:44:05 |
| 19:49:37 | guest still running, stopped at the watchdog deadline (code 13923 = `0x3663`) |

Guest packet ring after shared Enter (UTC): 2364 at 00:41:09.561 → 2723 at
00:41:17.292 → 68 × 2005 → 2268 at 00:41:18.616 → 2792 at 00:41:18.618, then
**no 2941, no 2789, no 2891**. The ring filled at 00:47:59.

The guest did not crash this time. It sat in the host's field with an
unfinished load: the same missing 2941 that R10 Step 1 traced to the
owner-only branch of MoveFieldCompleteCore, and that R10 Step 2 addresses.

The R9 loading sampler logged 180 samples from 19:41:09, all `state byte
0x00, candidate marker 94`. A marker of 94 contradicts the 3576 reading
("below two while waiting"), so the sampler's offsets do not carry over to
3663 as assumed. The packet ring is the evidence that counts.

## Safety follow-up

- Finish never ran, so saves were never verified. Verified on 2026-11-19
  (`tools/port3732/savecheck.ps1`): the real save folder matches the A3 manifest
  and the before-guest checkpoint for all 23 files; the host backup and guest
  sandbox are unchanged.
- The controlled installation was never restored. `cdmp.asi`, `cdmp.pdb`,
  `winmm.dll`, both inis and the `cdmp.r4-test-lock` marker naming A3 were
  still in the game folder on 2026-11-19 (none existed before A3's deployment).
- **Restored 2026-11-19 on Shaugn's approval.** Each of the six files matched
  its `installed_sha256` in A3's `deployment.json`, and none existed before.
  All six were removed. A3's four game-folder logs (host and guest session
  logs and their `cdmp.host.log`/`cdmp.guest.log` copies) were copied into
  the A3 `evidence` folder with hash checks, then removed from the game
  folder. Receipt: `finish-r10-manual.json` in the A3 session folder. The
  game executable remains 1.0.0.3732.
- No user observation was recorded. What each window showed is unknown.
