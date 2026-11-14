# Phase 0 — baseline and review fixes

Base: a8656e87a0c43db400bdf863193c1afb353d6de0, branch gpt/3663-port.
The task workspace was empty/unborn with no origin. Fetched the explicitly
requested source branch from Shaugno/CrimsonDesertMultiplayer-NativeServer,
created the requested branch, and copied the supplied plan into docs/.
The previous working checkout was read only and was not modified.

Reproduction before the fix: 32 retirement records could be displaced by other
peers, allowing a stale generation to be admitted again. A lower, never-retired
generation could also replace a newer one. Expected: only strictly increasing
generations are admitted per peer throughout a Feed lifetime.

Changes: per-peer highest admitted generation, never evicted; 4096 historical
peers maximum with refusal of new peers at capacity, 8 active sessions maximum.
Failed retirement preserves the previous actor/session and allows retry of the
proposed generation. Single-threaded caller serialization is explicit.
disconnect_all returns the count of failed removals, and attempts all sessions.
One shared quaternion helper accepts finite norm 0.5–2 inclusive. Removed dead
heading thresholds, used opcode_is for movement, and fixed iterator signedness.
The offline library remains unlinked from cdmp.asi; production 2976 behavior is
unchanged by these fixes.

Validation (game closed, no deployment):

- `scripts/build.ps1`, x64 RelWithDebInfo, VS 18 Community, MSVC toolset
  14.52.37025 / compiler 19.52.37029.0.
- Baseline: 101/101 CTest passed, 139.68 seconds. Full output:
  `port-evidence/phase0-baseline.txt`. Initial sandbox-only configure could not
  resolve github.com for pinned dependencies; authorized network-enabled build
  succeeded. No dependency versions were changed.
- Focused rebuilt `cdmp_tests_coop3663`: passed.
- Post-fix complete build and CTest: 101/101 passed, 93.77 seconds. Full output:
  `port-evidence/phase0-fixed.txt`.
- Regression cases cover 40 other peers churning, older generations after a
  newer session ends, duplicate admission, history exhaustion without eviction,
  generation wrap refusal, failed removal/retry, all-session failure counts,
  and identical feed/decoder rotation boundaries including NaN/infinity.
- `git diff --check`: passed.

Phase 1 reference download completed through the user's Steam touchpoint.
2976 reference SHA-256 matches the required value; documentation follows in
the separate Phase 1 commit. No game executable was run.
