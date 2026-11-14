# Phase 1 — static reference executables

The user completed the single Steam depot touchpoint:
`download_depot 3321460 3321461 8201393788450828800`.

Downloaded input:
`C:\Program Files (x86)\Steam\steamapps\content\app_3321460\depot_3321461\bin64\CrimsonDesert.exe`.

Verified, separate analysis copy (outside the Git repository):
`C:\Users\shaug\.codex\visualizations\2026\11\14\01a1dbbd-2726-7441-9a57-1b454819b3e7\reference-bins\2976\CrimsonDesert.exe`.

Both copies have SHA-256
`57DA440D72F4DB974F25FEF047CF84C4DADD999A88CB2A3C5AF4C9BD67FDE1E7`,
384,521,624 bytes, FileVersion 1.0.0.2976. The copied file was hashed again.
PE facts are recorded in `port-evidence/reference-2976-identity.json`.
The hash equals `docs/research/cheat-tables-versions.md` §1.

Installed target, read only:
`D:\SteamLibrary\steamapps\common\Crimson Desert\bin64\CrimsonDesert.exe`,
335,440,280 bytes, FileVersion 1.0.0.3663, timestamp 0x6AF5D198,
SizeOfImage 0x144E9000, preferred base 0x140000000, SHA-256
`8BE67178D9B9A149C361A731051665E0BA4D441E250AE1DFE645BD6E524345C8`.
Full facts: `port-evidence/installed-3663-identity.json`.

Verification commands: `Get-FileHash -Algorithm SHA256 -LiteralPath <path>`;
FileVersion via `(Get-Item -LiteralPath <path>).VersionInfo.FileVersion`;
PE headers parsed as ordinary file bytes. No executable was loaded or run.
The installed game and Steam install pointer were not modified. No saves
were accessed or changed. No game binaries are staged or committed.

Optional 3576 midpoint: no executable found in prior workspaces, depot content,
or the documented historical D:\RE location. The repository records SHA-256
C94AD6C3EA54970A6D2DA7A2719753210A7728A91698ED3405DD9125470ED98F,
but does not establish a corresponding Steam manifest. The local depot cache
contains manifests 532620525636668883 and 5865785040848247748 as well as 2976;
neither is yet proven to identify 3576. Do not label a cached manifest by guess.
This optional missing midpoint does not block direct 2976-to-3663 work.
