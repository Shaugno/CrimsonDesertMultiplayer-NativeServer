# Actual validation failures and reruns

Default-match build: 101/101, 109.33 seconds. Thread-layout build: 101/101,
90.32 seconds; synthetic loopback passed. Layer-layout build: 101/101,
90.01 seconds; synthetic loopback passed.

Recipient-final build failed test 8 pkt3576_live_host (100/101, 88.45 seconds).
Its PID-only directory reused pkt3576-live-host-40144: labeled files were
created at 09:37, while a later test updated the session/unlabeled files at
10:02. Production correctly refuses an existing labeled archive. The test
now uses PID plus process creation FILETIME; no archive was deleted and
production overwrite protection was not relaxed.

The first post-fix rerun encountered an unmodified game launched at 10:05
during validation. The game-closed guards refused tests 18,25,28 (98/101,
34.82 seconds); fixture test 8 passed. No deployment or passive attachment
had happened. The user quit the game, confirmed closure, and the verified
matcher was paused before preparing the next session.

The final game-closed scripts/build.ps1 x64 RelWithDebInfo run passed all
101 tests in 87.03 seconds. Loopback received 78 remote states, zero rejected,
clean Network stopped/Worker exiting. This uses synthetic state and is not L2.
All raw failed/passed logs are retained with hashes.

An attempted structural join was refused because no result database existed;
the supplemental process was verified still running. It is not completion
evidence. A later authorized live session pauses that exact matcher.

Final session/lock contract build: 101/101, 87.04 seconds; loopback 78 states, zero rejected, clean stop. Session evidence hashes were corrected to canonical Git bytes, preserving original raw hashes; package validation now verifies every live file against its recorded byte count and SHA-256.
