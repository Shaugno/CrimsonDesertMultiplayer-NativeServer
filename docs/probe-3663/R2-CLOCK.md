# R2 native clock publication audit - Tier C remains closed

The named LoadingCompleteReq handler calls server transform-sync completion
2BCAF00 -> 2ED7960. That native implementation calls six-argument completion
219D7F0 -> 23DC630 with the same component, result, field, actor, byte flag
and timestamp roles. Both test the fifth argument before publishing time.
Target CMP/JZ at 23DC66B skips to the loading continuation at 23DC781;
the native entry and branch bytes are catalogued separately. Function and
site RVAs remain candidates pending standard matching.

Native completion writes the timestamp at component+468 and copies one
32-byte YMM block from component+450. It publishes to the TLS-selected clock
bank, then clears field+17C and continues loading. Source bank 6A6D238 maps
to target 6F31C98; the other bank is 6A6D218 -> 6F31C78. The core's bank
is not replaced with the other one. The member offset and copy width are
proved independently of the still-unadopted global/entry/site RVAs. These
reads do not establish the separate SetFieldTimeReq payload schema.

Rejected lead: target FieldMover caller invokes 2ECB660 before EnterField,
but full body review shows roster position preparation, including a
ten-argument helper 2ECC000. It does not reproduce the legacy clock-owner
comparison from SyncFieldTimeOnMove 2BC0A30. Positional proximity or a call
neighbour is insufficient to redirect that two-argument detour. The actual
travel clock publication path and its ownership gate remain unresolved.

The passive client loading question remains separate. Client controlled actor
vslot C0 at 5D93C0 tail-calls a reference acquisition implementation that takes
locks/increments references and examines TLS modes. It must not be invoked by
the external read-only collector or used as a pure readiness predicate.

All exports are file-only Ghidra disassembly/decompilation. clock-gate-old.txt,
clock-gate-new.txt, clock-publish-helper-new.txt and player-reference-gate-new.txt
retain actual roles and rejected leads. One simultaneous query failed because
both queries used the same targeted project; its log was preserved locally and
the client-gate query was rerun sequentially. No game ran during this work.

Next: obtain standard support for completion and its operand anchors, identify
the actual travel clock publisher, then implement and test the per-build clock
gate without changing 2976 behavior. Do not arm joining without this protection.
