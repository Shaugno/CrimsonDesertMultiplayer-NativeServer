# R4 Step 1 — file sandbox implementation record

Reproduction before the change: the legacy redirector covers named Win32
operations but does not intercept native handle-based rename/disposition,
pre-existing writable handles, or mapped writes. A fixture can therefore
change the source through SetFileInformationByHandle or a handle opened
before installation. Creating an empty guest directory is also insufficient
proof that it was copied from a verified backup.

Expected behavior: a separate 3663 file layer redirects native path opens and
queries from the real save tree to a verified plain copy, and refuses source
handles or operations it cannot classify safely. Native rename/link, delete,
write and file mapping must not provide an alternate route to the source.
The startup check verifies the backup/manifest/copy and an actual redirected
create/read/delete before reporting readiness. The game keeps its guest
startup denial until that readiness and later C/D prerequisites are satisfied.
The 2976 implementation remains on its existing path.

API contracts used: Microsoft [NtCreateFile](https://learn.microsoft.com/windows/win32/api/winternl/nf-winternl-ntcreatefile)
defines counted names, RootDirectory and FILE_OPEN_BY_FILE_ID;
[FILE_INFORMATION_CLASS](https://learn.microsoft.com/windows-hardware/drivers/ddi/wdm/ne-wdm-_file_information_class)
defines rename/link/disposition operations; [GetFinalPathNameByHandleW](https://learn.microsoft.com/windows/win32/api/fileapi/nf-fileapi-getfinalpathnamebyhandlew)
provides normalized names for existing objects. No game RVA is involved.

Baseline: 106/106 tests, 86.67 seconds, game closed. Further implementation,
fixture results and coverage limits will be recorded before any live session.

Implementation: `file_sandbox3663.cpp` is a separate native file layer; the
2976 redirector is unchanged. It verifies three plain, disjoint trees against
a SHA256 manifest, rejects hard links/reparse points, pins source/sandbox roots,
rejects existing protected mappings, then exercises redirected create/write/read/
delete and rechecks the real manifest. Optional newer NT exports are hooked
when present. Missing required APIs or an unverifiable disk handle refuse.
Disk opens request READ_ATTRIBUTES so subsequent handle identity checks work,
including execute-only opens by the system DLL loader. Callbacks preserve the
thread's Win32 last-error value. Private validation bypass permits reads/queries;
the startup mutation check uses the active redirector.

The native layer covers path creation/open/deletion/attributes/by-name queries;
handle read/write, scatter/gather/copy, information/rename/disposition, EA,
security, flush, locks, directory queries and filesystem control; section
creation/mapping; and delayed Win32 moves. Source handles are denied; mapped
source views present at startup refuse installation. File-ID opens, hard-link
creation, reparse mutation, deferred moves and renames across the sandbox
boundary refuse. This is process-local protection for normal Windows file
calls, not a security boundary against deliberately issuing custom syscalls or
another process. Both R4 game instances will use separate sandbox copies, so
the host experiment can require an unchanged real-save manifest too.

`launch-2inst-3663.ps1 -Stage Prepare` creates fresh, verified host/guest backups
outside OneDrive and copies each sandbox from its corresponding backup. Verify
compares real saves and both backups by SHA256; it retains sandbox progress.
Run currently refuses until the Step 4 C/D arming path exists. Production source
selection is fixed to the real save root and executable digest; fixture mode is
restricted to temporary directories and cannot launch. The worker requires the
3663 sandbox for every target guest and exits on setup failure. The existing
C/D denial still applies even when the sandbox verifies. Test hosts never
install these process-lifetime hooks. No game has been launched or attached.

Offline debugging found and corrected:
- Native exports may share a page with NtProtectVirtualMemory. The upstream
  VirtualProtect executable-page exception now also covers its native callee,
  avoiding an execute fault while restoring protection. The checked upstream
  source/hash and third-party attribution are preserved; its patch is regenerated.
- A Win32 rename fixture supplied an unterminated name that KernelBase scanned
  beyond the provided count on this OS; its terminator now lies outside the
  declared count. Native callback names remain counted and bounded.
- CopyFile's system-DLL load requires metadata access for the handle guard;
  new disk opens now request it, and NT callbacks preserve last-error state.
- MEM_MAPPED also includes anonymous pagefile mappings. The startup scan uses
  the documented QueryVirtualMemoryInformation region flags to distinguish
  those from data-file mappings; unresolved file mappings still refuse.

The debugger helper has no attach mode and starts only the owned sandbox
fixture executable. Diagnostic runs and binary artifacts remain local.

Validation history (all retained locally with hashes in step1-result.json):
- Baseline 106/106, 86.67 seconds.
- First full run 107/108, 115.49 seconds: new pre-existing-map scan refused an
  anonymous mapping; fixed as described above.
- Second full run 107/108, 109.51 seconds: sandbox and launcher passed; existing
  optd_s3 Ring concurrent test failed its final `ok == 1024` assertion. Its
  unchanged implementation reserves sequence numbers before writers take a
  slot, so a delayed earlier writer can publish over a later sequence. That
  optional module remains unreachable on 3663; this run is not a clean pass
  and the underlying existing ring issue is not claimed fixed.
- Final build/suite and synthetic smoke: see the result record below.

Additional API contracts: [NtCopyFileChunk](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntcopyfilechunk),
[QueryVirtualMemoryInformation](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-queryvirtualmemoryinformation)
and [region flags](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/ns-memoryapi-win32_memory_region_information).

Manual runtime plan (Step 5 only): launch under fresh prepared receipts with
Shaugn present; confirm each role logs its own sandbox before title, complete
the written movement/disconnect experiment, quit both, run Verify and remove
the test installation. Offline results do not establish runtime compatibility.

Final validation: **108/108 passed, 93.03 seconds**, after the final expanded
native-operation build. Synthetic smoke: eight seconds, 78 received states,
zero rejected packets, clean worker exit and ASI unload. Game closed; mod and
loader absent from the installation. The sandbox has not been tested in-game.

R4 rule 2 retires D020–D023, D063–D067, D100–D104 and D130 by v1 scope:
the new 3663 path uses verified file-level copies and never enters those legacy
save classification/title patch paths. This grants no C/D permission and no
native title parser or save-callsite RVA adoption. Remaining lifecycle rows
are still required for the later arming decision.
