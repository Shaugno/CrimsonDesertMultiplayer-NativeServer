# File-only 2976 → 3663 comparison

Run from the repository root. Never execute either game image. Reference files
stay outside the repository. `build/` projects/results are ignored. The reviewed
decisions live in `docs/probe-3663/PORT-REVIEWED.json`; discovery scripts always
emit `adopted=0` and cannot arm the ASI.

The inventory defaults to source commit a8656e87a0c43db400bdf863193c1afb353d6de0,
read through a memory-only `git archive`. This keeps the 2976 row identifiers
stable after 3663 headers are added. `--revision` selects a different source
baseline intentionally; changing it also requires reviewing the match map.

Synthetic validation: `tools/port3663/smoke.ps1` copies the built tools to
`build/e2e`, uses an unused loopback UDP port, disables all native features,
checks remote-state receipt and clean worker shutdown, and stops only its own
newly started peer. This is never two-player evidence.

Tools used: VS 18 / MSVC 14.52.37025, pinned Zydis 4.1.0, Python 3.12.10,
Ghidra 12.1.4 PUBLIC and Temurin JDK 21.0.13+9. Ghidra and Java were installed
only under the task's visualization directory, with no system PATH changes.
Ghidra ZIP SHA-256 DDAC49F903DA9D5BAC833E5CC79395098B9C33CFD3279BE5F31BD00387D2D4DB;
JDK ZIP SHA-256 A4DBAA05FCA4BD89D84584E58C459BD732EFEBB1722AF80AB13C6516832004F9,
checked against the official Adoptium package checksum. Ghidra came from the
official NationalSecurityAgency/ghidra release `Ghidra_12.1.4_build`.

The example variables denote absolute paths: `$reference` is the verified
2976 reference, `$target` the installed 3663 image, `$python` Python 3.12,
`$cmake` the VS-bundled CMake, `$ghidra` Ghidra's root, and `$jdk` the local JDK.

```powershell
& $python tools/port3663/pe_identity.py $reference
& $python tools/port3663/pe_identity.py $target
& $python tools/port3663/inventory.py
& $python tools/port3663/signature_candidates.py --reference $reference --target $target --out build/signature-candidates.json
& $python tools/port3663/rtti_candidates.py $reference --out build/rtti-2976.json
& $python tools/port3663/rtti_candidates.py $target --out build/rtti-candidates.json
& $cmake --build build --config RelWithDebInfo --target port3663_fingerprints
& build/bin/RelWithDebInfo/port3663_fingerprints.exe $reference > build/fingerprints-2976.tsv
& build/bin/RelWithDebInfo/port3663_fingerprints.exe $target > build/fingerprints-3663.tsv
& $python tools/port3663/correlate.py --old-functions build/fingerprints-2976.tsv --new-functions build/fingerprints-3663.tsv --old-rtti build/rtti-2976.json --new-rtti build/rtti-candidates.json --out build/function-correlations.json
& $python tools/port3663/call_neighbors.py --old-functions build/fingerprints-2976.tsv --new-functions build/fingerprints-3663.tsv --correlations build/function-correlations.json --out build/call-neighbors.json
& build/bin/RelWithDebInfo/pkt3663_scan.exe $target
& $python tools/port3663/matches.py
```

Core-descriptor pass:

```powershell
& $python tools/port3663/rtti_candidates.py $target --names 'TrocTr(LoginUserToFieldServerReq|LoginCharacterToFieldServerReq|SaveDataSlotRefreshReq|ConnectCryptogramToServerReq|MainPlayerDataStartAck|MainUserDataStartAck|MoveFieldCompleteReq)|NwSession@|NwAsyncSession@' --out build/core-rtti-3663.json
& $python tools/port3663/descriptor_candidates.py $target --rtti build/core-rtti-3663.json --out build/core-descriptors-3663.json
```

Ghidra imports use separate projects so the identically named input files do
not collide. Each import uses `-noanalysis`, then targeted function creation,
flow-body fixup and 30-second bounded decompilation per requested entry.
This R1 workflow is paired targeted analysis; it is not complete Version
Tracking/BSim evidence. R2 uses separate whole-image projects described below.
A five-byte thunk can lead to a Denuvo region; trace the
destination explicitly when a claim depends on its body.

```powershell
$env:JAVA_HOME = $jdk
$env:PATH = (Join-Path $jdk 'bin') + ';' + $env:PATH
New-Item -ItemType Directory -Force build/ghidra | Out-Null
& "$ghidra/support/analyzeHeadless.bat" "$PWD/build/ghidra" Port3663 -import $reference -noanalysis -scriptPath "$PWD/tools/port3663" -postScript PortFunctionEvidence.java "$PWD/build/ghidra-2976-expanded.txt" 0x1437740 0x14380B0 0x1436EE0 0x35DF50 0x2773C20 0x2A75860 0x2A75880 0x2A759B0 0x1870C20 0xCBD92D0 0x47EF9B0 0x47EFA78 0x35C670
& "$ghidra/support/analyzeHeadless.bat" "$PWD/build/ghidra" PortTarget3663 -import $target -noanalysis -scriptPath "$PWD/tools/port3663" -postScript PortFunctionEvidence.java "$PWD/build/ghidra-3663-expanded.txt" 0x165C1F0 0x165CB60 0x165B9D0 0xE6F8C70 0x3AB4C0 0x4B7E0C0 0x4B7E188 0x2D6C210 0x2D6C230 0x2D6C360 0x3A9960 0x3A9CF0 0x3A8AC0
& "$ghidra/support/analyzeHeadless.bat" "$PWD/build/ghidra" PortTarget3663 -process CrimsonDesert.exe -noanalysis -scriptPath "$PWD/tools/port3663" -postScript PortFunctionEvidence.java "$PWD/build/ghidra-3663-core-handlers.txt" 0x2A0EA70 0x2D133B0 0x25E320 0x25E140 0x2588D0 0x25E1E0 0x1522D0 0x14BC00
& "$ghidra/support/analyzeHeadless.bat" "$PWD/build/ghidra" PortTarget3663 -process CrimsonDesert.exe -noanalysis -scriptPath "$PWD/tools/port3663" -postScript PortFunctionEvidence.java "$PWD/build/ghidra-3663-refresh.txt" 0x1A18320
```

For an existing project replace `-import <path>` with
`-process CrimsonDesert.exe`. The reports contain disassembly/decompilation,
not raw executable dumps. Projects, downloaded tools and game binaries are
never committed. All adopted values still require the complete minimal path
and game-free validation before any live test is eligible.

## R2 whole-image pipeline

Read `docs/probe-3663/R2-EXPORTER.md` before reproducing. Required tools are
Ghidra 12.1.4, JDK 21, Gradle 8.14.3 and BinDiff 8; Google BinExport source
revision is pinned in `r2_build_exporter.ps1`. Build its separate corrected
jar and compare it with the installed original on the owned CMake fixture
using `r2_export_fixture.ps1`. The original extension remains installed.

Run `r2_pipeline.ps1 -AnalysisRoot <tools-root>` with the game closed. It
hash-checks both images, imports each whole image independently with RTTI
seeds and recorded native options, saves analysis, exports the entire graph,
then runs stock Auto Version Tracking and BinDiff. Output goes to
`<tools-root>/r2-nativefix`; project databases remain local in build/r2/projects.
Interrupted default/graph/direct/native jobs are not completion evidence.

Only after both standard outputs finish, run `r2_matches.py --vt <tsv>
--bindiff <database> --reviewed docs/probe-3663/r2-reviewed.json`. Native
consumption proofs and independent RVA checks are generated separately by
`r2_reviewed.py` and `r2_rva_checks.py`. `r2_match_tests.py` covers contradictory
matches and insufficient evidence; its synthetic outputs never enter docs.
Do not treat a prepared script or a small fixture as a real matcher result.
