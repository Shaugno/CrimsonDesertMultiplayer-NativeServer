# Field-sharing trace design — not ready to launch

Question: during one ordinary local login and one fast travel on exact 3663,
which actors in the EnterField roster belong to the travelling user, and in
what order do native acquisition, actor admission and field assignment occur?

Expected observation: the mover's primary actor is obtained through its holder,
and each bounded roster record resolves to an actor with a readable owner. The
trace distinguishes pre-existing actors from native factory-created actors and
records each entry result. A single local run can characterize the normal
roster; it cannot establish arbitrary companion or guest ownership safety.

Time limit: ten minutes from title screen, with the user present. Exact user
steps, once the collector passes preflight: Continue; wait for control; stand
still for ten seconds; make one ordinary fast travel; wait for control; quit
to desktop at the stop signal. Back up all saves with a verified manifest
before deployment, retain logs and hashes, remove the deployment afterwards,
and verify the backup and installed-file restoration. Do not overwrite new
save progress with the backup.

Candidate entry/ABI contracts:

| Role | 3663 candidate | Native arguments/result | Status |
|---|---|---|---|
| AcquireField | 2E08520 | manager, result pointer, pool type, output holder, travelling user; returns result pointer | Paired native body reviewed; standard support pending |
| EnterField | 2ECB130 | mover, result pointer, field; returns result pointer | Paired native caller/register uses reviewed; standard support pending |
| AddActor | 2DFD3A0 | field, result pointer, actor | Paired Denuvo body reviewed; standard support pending |
| AssignField | 2EC9200 | actor mover, result pointer, field | Paired body reviewed; standard support pending |

Required samples are bounded entry/exit records with thread ID, sequence,
arguments, result, field owner, user/actor counts, primary actor owner and
roster keys/owners. Target roster is mover+68/count+70, stride 3C, kind+0,
actor key+4; these are observational native-code findings, not yet a
production adapter layout. Capture the normal execution only. No induced
native lookup, reference acquisition, lock call, packet replay or game-memory
write is permitted. Existing native calls must retain their original ABI,
return value, call order and behavior. Candidate pointers are process-local
diagnostics, never peer identities.

Stop collection on unreadable/mutating chains, count over 64, unexpected
identity or entry bytes, dropped records, user stop or the time limit. A
guard failure must leave ordinary native execution alone; it cannot suppress
or retry a native operation. On a crash, collect context and stop; fix and
run the full suite before another live attempt.

Before requesting presence, the collector still needs implementation and
offline validation: all hooked entries adopted under R2-MATCHES; exact target
entry guards; bounded guarded reads and logging; nesting/thread accounting;
an in-flight detour lifetime barrier and verified unload; backup/deploy/remove
rehearsal; full build/CTest and synthetic smoke. The existing 2976 hook and
probe implementations must not be enabled on 3663 to obtain this trace.

The production adapter remains blocked on roster owner/lifetime consistency,
all factory branches and partial-entry rollback. A future bounded primary-only
adapter must explicitly reject other roster shapes and prove that its
preflight remains valid through native admission. It cannot infer safety by
ignoring companions. Host references and user counts must survive every
failure path, and a failed insertion may undo only its own unused entry under
both locks. This is separate from identifying the function RVAs.
