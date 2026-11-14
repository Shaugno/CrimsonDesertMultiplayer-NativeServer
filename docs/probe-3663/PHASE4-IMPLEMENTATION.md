# Phase 4 — recognition implemented; gameplay port blocked

Implemented independently of the unresolved core rows:

- `build_3663.h` contains the three file identity facts and only the 17 reviewed
  static values from PORT-REVIEWED.json. Every value records its old counterpart
  and the Phase 3 inventory row (frozen at commit 878d14f).
- The central gate recognizes Build3663 only for timestamp 0x6AF5D198,
  mapped base 0x140000000 and SizeOfImage 0x144E9000. Recognition grants no
  gameplay permissions. A guest ends before any legacy save hook can run.
- Diagnostics explicitly say the core port is incomplete and not armed.
- Tests cover 2976, 3576, 3663, unreadable/unknown, wrong 3663 timestamp/base/size,
  header-only classification, blocked host/guest callback effects, and preserved
  test-host synthetic/network/overlay behavior. Focused guard tests pass;
  output is `port-evidence/phase4-guard.txt`.
  `cmake --build build --config RelWithDebInfo --target cdmp_mod` also passed.

Not implemented: a complete per-build address accessor routed through all core
modules, a native 3663 player source, minimal option-D installers, or permission
to execute the corroborated functions. Those depend on the unresolved field
sharing, identity/status, wire, player/SAM and save/lifecycle rows in PORT-MATCHES.
No placeholder or weak address is substituted. All optional modules remain
compiled and unreachable on 3663 through the central gate regardless of ini.

The 2976 address tables, module implementations and config defaults are
unchanged. The existing 2976 gate branch is preserved. 3576 stays recognized
and unarmed. The wire format and kVersion are unchanged, as are the Steam
friends restriction, executable filter and explicit test-host paths.

This is a partial Phase 4 checkpoint, not a functioning 3663 port. Phase 5
validates the resulting guarded build without claiming native functionality.
