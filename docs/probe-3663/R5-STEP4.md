# R5 Step 4 — L4 blocked by its L3 prerequisite

**No L4 session ran.** R5 requires a passed L3 before the second-PC Steam
session. R5-L3-1 established native guest login and title refresh, but not
two usable windows, a character-load request, mutual movement, removal,
reconnect or the post-session host reload. See `R5-STEP3.md` and its result
JSON. The attempt budgets are not exhausted: startup 3/5, join 1/3.

No second PC, account change, Steam credentials, firewall exception, Steam
friends bypass or public binary release was requested. No guest kit was
deployed or presented as live-ready. The unrelated original checkout and
parent Start-Grok launcher remain untouched.

Independent preparation exists in `launch-2inst-3663.ps1`: L4Host/L4Guest
select Steam transport, retain the exact executable hash and explicit v1
route, use real backed-up host saves, and require verified guest profile
isolation. Guest-only source manifests are strict. The existing Steam
friends restriction and executable filter remain. These code paths and
game-free transport/Steam initialization tests are not a two-PC test.

After L3 actually passes, the next batched second-PC touchpoint must establish
the installed build/path and the other licensed Steam account, friendship,
the host peer ID, a private same-binary kit with GPL source/attribution and
verified hashes, fresh backups and guest save isolation. Portable arming
paths must be resolved on that PC; do not reuse this PC's absolute paths.
Carry the actual host launch receipt/deadline into the guest watchdog so
the sitting has a common bounded deadline. No transfer or launch occurs
from this preparation note.

Live acceptance remains: mutual native visibility and walking/turning over
Steam, guest removal and reported reconnect outcome, unchanged guest real
save manifest and a successful post-session host save load. The guest does
not travel; the host may load/travel once within the approved scope. Mounts,
combat and optional modules remain off. Update this record with actual
proof before claiming Step 4 acceptance.
