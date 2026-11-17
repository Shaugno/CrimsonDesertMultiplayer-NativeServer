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

## Startup trial R4-L3-20261116-184928

Source c0e5984. Host PID 752 launched at 18:49:33.2526762 Eastern with the
same exact executable identity, verified sandbox and pinned watchdog. The
same SteamAPI error prevented native descriptor readiness. No guest launched
and no native join ran. The process exited at 18:50:34.9306115 with
0xC0000005; Windows records an execute fault at ntdll+166DE4, inside
NtProtectVirtualMemory (+14 from its export). The existing local Windows
crash dump is retained outside Git; its SHA256 is
A0B1A66245D88A54D79EC189B1894697912E7ED3F2B7751EB80FBDF95F301CF3.
The bounded file-only reader `tools/port3663/minidump_summary.py` reports
module/exception facts; its stack words are explicitly not unwound frames.

Cleanup restored the installation and verified real saves/both backups.
Host log SHA256:
`665B9B6B7C7D7769B458808E57D9ADFF9235AE832F45C75265969D845EA0DD6C`.
Steam also started untagged copies while the controlled marker existed;
the marker refused them before normal game startup. Their trigger is not
established. Subsequent ordinary Steam launches occurred after cleanup.

The owned SteamAPI fixture failed both without hooks and with hooks in the
controller and the user's normal desktop context. Same user, integrity,
logon and session were verified. After the user restarted Steam, baseline
initialization succeeded, establishing a separate client IPC problem.
Sandbox initialization still failed, which exposed two further file-layer
compatibility defects without more game launches:

- DOS named-pipe opens were treated as invalid disk names. They now bind
  explicitly to the native NamedPipe device; no mutable disk alias bypass.
- Steam reopens its pipe-directory handle using an empty relative native
  name. That path now passes only for a kernel-verified PIPE handle. Disk
  and unknown handles still undergo normal save identity/path validation.

Owned pipe fixtures cover connection, directory reopen, and the native
PIPE_NOT_AVAILABLE result for an occupied single-instance endpoint. After
both corrections, SteamAPI_InitFlat returns success with and without the
sandbox, with zero sandbox refusals and unchanged fixture real saves.
The [Windows pipe naming contract](https://learn.microsoft.com/en-us/windows/win32/ipc/pipe-names)
distinguishes these IPC names from filesystem save paths. Runtime success
in the real game remains a separate acceptance question.

The shutdown fault was independently reproduced by letting the fixture
return through ordinary CRT teardown with native file hooks installed:
exit 0xC0000005. SafetyHook's disable operation swaps its target/trampoline
arguments, while the earlier NtProtectVirtualMemory protection exception
checked only the first range. Both ranges now receive that existing
execute-protection exception, including VirtualProtect. The expanded
fixture exits normally, and the intermediate full suite passes 114/114
in 114.07 seconds. Final combined validation is recorded separately.
Upstream source hashes, attribution and the regenerated patch are retained.

Record after this trial: two failed controlled startups, one startup crash,
zero guest launches and zero native join attempts. The three-fix native
join crash budget has not been exercised. Live work stopped on the crash;
all subsequent investigation used owned fixture processes and existing
crash files. The user restarted Steam and ran the comparison manually;
no UI control was used.

Final combined validation before the next live experiment: **114/114 passed,
94.22 seconds**. The fresh synthetic smoke and source/binary receipt are in
`port-evidence/r4/startup-fixed-validation.json`. The two failed startup
trials, cleanup receipts and offline reproductions are sealed in
`port-evidence/r4/startup-trials.json`. No L3 pass is claimed.
