# R7 Step 2 offline networking follow-up

## What the live log established

The R7-S2 guest log for PID 51912 has one sandbox counter snapshot, at
10:19:54.378 AM Eastern. It reports verified NT isolation, two redirects and
zero refusals. There are no per-site refusal counts, denied object names or
later sandbox counter snapshots in that log. The startup dialog therefore
does not identify which subsystem failed.

## Reproduction

The same owned save tree was used in two fresh fixture processes. The control
ran without hooks. The second installed the guest NT sandbox before invoking
the Windows networking APIs. The fixture logs each name-based sandbox
refusal and its call site. Raw paths stay in local evidence.

| Probe | Unhooked control | Sandbox before fix | Sandbox after fix |
| --- | --- | --- | --- |
| GetAdaptersAddresses | 0 | 5 (access denied) | 0 |
| getaddrinfo("steamcommunity.com") | 0 | 11001 (host not found) | 0 |
| TCP connect to an owned loopback listener | 0 | 0 | 0 |
| TCP connect to steamcommunity.com:443 | 0 | Skipped after DNS failed | 0 |
| Sandbox refusals during networking | 0 | 5 NtCreateFile refusals of `\??\Nsi` | 0 |

Two NSI denials accumulated during adapter enumeration and three more
during DNS resolution. The first alias fix recovered all four API checks
and revealed six additional uppercase NetBT adapter opens. The final
fixture passed those opens through as well. The post-fix diagnostic
confirmed that both the owned original save and frozen backup keep their
pretest SHA-256 manifests.

This reproduces a sandbox-induced DNS failure on this Windows installation.
The older live log has no matching trace, so the cause of the actual game
dialog and its activation status remain unconfirmed.

## Fix and coverage

For a bare DOS alias, the sandbox checks the current `QueryDosDeviceW`
mapping against exact native network device names. A matching open uses
the verified native path directly with `OBJ_DONT_REPARSE`. The case-insensitive
NetBT rule requires the exact adapter-device format and a complete GUID.
Disk targets, missing or unrecognized aliases, and extra path components
receive the existing path validation. The owned disk-alias regression
refuses an open of the fixture original. The rest of the save fixture still
checks redirected disk operations and unchanged source and backup manifests.

The regular CTest fixture uses adapter enumeration, local name lookup,
and an owned TCP listener. The optional public DNS and TCP comparison
uses no Steam API or game process. See Microsoft's
[DOS device mapping contract](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-querydosdevicew),
[object attributes](https://learn.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_object_attributes),
and [NetBT adapter-device example](https://learn.microsoft.com/en-us/windows/win32/api/lmserver/ns-lmserver-server_transport_info_3).

To reproduce the before/after public probe on the relevant fixture binary:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tests/file_sandbox3663_tests.ps1 -FixtureExe <fixture-exe> -NetworkingDiagnostic
```

The full `scripts/build.ps1` run passed 119/119 tests in 173.34 seconds.
The eight-second synthetic smoke received 78 remote states, rejected zero
and unloaded cleanly. The source seal and tested ASI are linked by
`build/r7/network-fix-arming-receipt.json` in this checkout.
[Validation and evidence hashes](port-evidence/r7/network-fix-validation.json).

The next live test is a single guest-only Step 2 retry on a later day, using
the new validation receipt, a fresh backup, and the user's presence. R7
remains at startup 1/5 and join 0/3. L3 still depends on a clean Step 2, and
L4 and the separate cleanup plan still require user approval after L3.
