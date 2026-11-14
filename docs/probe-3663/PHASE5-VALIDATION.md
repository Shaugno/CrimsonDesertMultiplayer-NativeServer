# Phase 5 — game-free validation of the guarded checkpoint

This validates the partial, unarmed build, not a completed native 3663 port.

Configuration: x64 RelWithDebInfo, Visual Studio 18 Community, MSVC toolset
14.52.37025 (compiler 19.52.37029.0), Windows SDK 10.0.26100.0, C++20, static CRT.
The ASI build ID is source commit 3c3ec37, clean source tree; later commits add
documentation and analysis tooling. Output hashes are in
`port-evidence/phase5-binary-hashes.json`; ASI SHA-256:
5E2FC9BEF6D8802150936FC381F23AE89C061AB53A78F5E3DEEA9592D67C9FEE.
Binaries remain local under build/bin/RelWithDebInfo and are not published.

Commands and results:

- `scripts/build.ps1`: complete build passed; 101/101 CTest passed in
  111.81 seconds. Full output: `port-evidence/phase5-suite.txt`.
- `tools/port3663/smoke.ps1`: cdmp_peer ghost on loopback UDP 39763, then
  `build/e2e/cdmp_host.exe <absolute build/e2e/cdmp.asi> 8`.
  The host sets CDMP_TEST_HOST. Loaded, ran and unloaded cleanly, exit 0.
  At the 5-second summary: sent 85, received 83, rejected 0, remote states 78,
  RTT 0.5 ms. Local source explicitly synthetic. Network stopped, worker exited.
  Output/config: `port-evidence/phase5-loopback-{run,host}.txt` and
  `phase5-loopback.ini`. The peer process started by the script was stopped.
- The smoke script now also asserts nonzero remote-state receipt and shutdown;
  those conditions were independently checked against the retained run log.
- `python -m compileall -q tools/port3663`: passed. An initial py_compile
  invocation with a literal PowerShell wildcard was corrected to compileall;
  it was an invocation error, not a suppressed syntax failure.
- Fingerprint scanner against cdmp_host.exe: correctly refused, exit 2.
- Pinned inventory regeneration: 1292 named rows / 8261 inline occurrences,
  193 files; reviewed map resolves exactly 17 unique adopted rows.
- `git diff --check`: passed after normalizing evidence trailing whitespace.
- DX12 overlay smoke is not required: no overlay/render code was touched.

Self-review against AGENTS.md:

- Lifetime/identity: Feed keeps per-peer high generations without eviction,
  bounded at 4096 histories and 8 active sessions; removal failures preserve
  active state, return counts and permit retry. Regression tests cover churn,
  exhaustion, wrap refusal and partial disconnect failure.
- Network bounds/protocol: shared quaternion policy rejects invalid values;
  opcode check is bounded by the existing header/length check. No wire format
  changed. Steam-friends restriction and protocol version are untouched.
- Memory/hooks: no new gameplay read, hook or native call is enabled. 3663
  header reads use the existing guarded identity copier. Wrong size/base/time
  and unknown headers stay rejected; guest fails before legacy save redirect.
- Version isolation: 2976 offset tables and module implementations remain
  unchanged; supported branch semantics remain covered by the same guard suite.
  3576 is still unarmed. Test-host paths remain explicit and functional.
- Reconnect/cleanup: fake-backend tests do not prove native removal. Current
  field/SAM barriers are unresolved and prevent a live attempt.
- Analysis reproducibility: inventory now reads the pinned 2976 source via
  memory-only git archive so new 3663 headers cannot renumber reviewed rows.
  Discovery outputs are separated from the 17 explicit reviewed promotions.

Game remained closed. No game launch/attachment/deployment, memory writes,
save changes, capture or two-player observation occurred.
