# R3 Step 2 — native tunnel port

Before this change, enabling a 3663 tunnel could reach 2976 constants and
optional join/save callbacks. Reproduce offline by following server_install,
on_server_recv, take_frame and run_network with a 3663 identity: the shared
path had no per-build profile and the target worker ran only the reader.

Expected behavior: exact 3663 identity selects all adopted B constants and
native packet bounds. Its B-only host constructs a private native session on
ServerRunThread, injects through the client's NetworkThread and native Q0,
routes tagged buffers at the receiver, and returns native send buffers to Q2.
Until C is complete it admits only the named cryptogram request. Login, world,
save, optional observers and native reconnect remain inaccessible.

The planned L2 uses an explicit loopback-only test peer. The game-generated
cryptogram request is copied once and echoed into the private session; its
native response crosses the ordinary reliable tunnel. No key or raw packet
is printed. Real Tier A pose samples travel in both directions. This is a
handshake/transport test, not a guest world join or two-player evidence.

Pump adoption evidence: the passive session logged 125528 observations, all
matching ClientNet and readable session/queue fields, zero changed roots.
Travel and normal exit were confirmed. All 23 saves stayed unchanged and
the installation was restored. The OneDrive backup placeholder issue was
recovered with exact hashes; future R3 backups use the external build root.
See pump-result.json, pump-adoption.json and r3_verify_backup.ps1.

Descriptor bounds use paired named slot-2 handler bodies and actual native
constructor RIP stores. Generic initializer BinDiff disagreements are retained
in descriptor-bounds.json. Packet bounds and the full aligned-pointer scan
are recorded in packet-scan-adoptions.json. No raw new RE dumps are committed.

Validation: 103/103 tests passed in 86.64 seconds, followed by a clean
game-free host/peer loopback (78 remote states). The new tunnel
fixture first failed because it queued before initial epoch reset; corrected
and full suite rerun. L2 remains pending. See step2-validation.json.

First L2 launch safely refused at 0.23 seconds: zero initialized handlers, no
native hooks installed. User quit at title. Normal exit, all 23 saves unchanged,
installation restored. Added bounded named descriptor readiness (60 seconds),
including the internal handler kind; no guard is bypassed. Full suite 104/104
(88.22 seconds), game-free smoke clean. Native retry is pending; see
l2-refusal-result.json and l2-readiness-validation.json.
