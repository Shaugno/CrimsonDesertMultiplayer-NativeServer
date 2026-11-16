# R4 Step 5 — live experiment record

## Startup trial R4-L3-20261116-184248

Source 8c541af, final Step 4 binary. Question: can two real build-3663
processes join through the restricted v1 tunnel, show each other moving,
and remove the guest while leaving real saves unchanged? User confirmed
presence. No UI control was used.

Only the host launched (PID 41380, creation 18:42:52.8985814 Eastern).
Its exact executable hash was
8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8.
File redirection and the fourteen-minute pinned watchdog verified. Steam
reported `MapViewOfFile returned NULL (errno=5)` and displayed a SteamAPI
initialization error. Native descriptor readiness timed out without native
server hooks; the v1 route refused. No guest launched or join ran.

The user selected Exit, but the verified host retained its Error window.
The controller terminated only that identity-verified process at 18:45:31,
before the watchdog deadline of 18:56:52. Installation cleanup completed;
real saves and both backups match the fresh SHA256 manifest. No real saves
were restored. The screenshot supplied by the user remains local, outside Git.

Local session root:
`C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\r4-live\R4-L3-20261116-184248`.
Host log SHA256:
`8BC024B5C5E36650A1531053FF1065D90EBF04282467596121E391FDD184E3B1`.
Real-save manifest SHA256:
`DA50EB8F0220066593A30DD81D51CFE9C484389AAA96A948348C4BB8B45042D7`.

Offline reproduction used an owned named pagefile mapping, then opened it
with MAP_READ/WRITE rights and mapped it under the active sandbox. This
failed with the same Win32 error 5 and preserved the fixture real-save
manifest. `build/r4/ipc-repro-test.txt` retains the failing result. The old
fallback asked NtQuerySection about a handle lacking SECTION_QUERY and
classified the failure as unsafe, although pagefile-backed IPC has no save
file to expose. This reproduces the compatibility defect; a live retry is
still required to establish that it resolves this Steam startup failure.

The focused fix queries the mapped region and accepts only its proven
MappedPageFile flag before file classification. Unknown/file views remain
fail-closed. A reduced-rights duplicate of a real-save file section remains
denied in the expanded fixture. This follows Microsoft's
[region backing flags](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/ns-memoryapi-win32_memory_region_information)
and [QueryVirtualMemoryInformation](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-queryvirtualmemoryinformation).

Current acceptance: L3 not established; L4 cannot run before L3 passes.
Record: one failed startup trial, zero guest launches, zero native join
attempts, zero failed native join attempts against the three-fix budget.

Offline retry validation: **114/114 passed, 94.27 seconds**, including the
new MAP-only IPC reproduction and protected-file reduced-rights refusal.
The refreshed synthetic smoke and arming receipt are sealed separately in
`port-evidence/r4/ipc-fixed-validation.json`; original Step 4 proof is retained.
