# Whole-image exporter correction

The official Google BinExport source is revision
`9648cacb21a71aa6c21b3fca504f146c39164a53` (Apache 2.0). Its
`BinExport2Builder.promoteUndefinedFunctions` comment says to skip functions
already defined or external, but its condition skips only external functions:

```diff
- if (func != null && func.isExternal()) {
+ if (func != null) {
```

Consequently it repeatedly invokes Ghidra's UndefinedFunction search inside
existing functions, for every basic block. The unmodified whole-image native
jobs completed analysis (18.69/19.52 million instructions) but still spent
over an hour exporting; repeated thread samples showed that same search.
They were stopped incomplete. They provide no BinDiff/VT result.

`r2_build_exporter.ps1` checks the exact source revision, preserves unrelated
edits, applies this one-line correction and builds a separate jar. The
installed original extension is unchanged. `R2ExportFixed.java` loads only
the builder's package from that jar; Ghidra/protobuf types come from the
installed extension's parent loader. The stock VT and BinDiff algorithms and
the whole-image export scope are unchanged. This is a patched exporter,
not an unmodified-exporter result.

The owned C++ fixture exports a branching/calling function. Before each
export, its function definition is removed while its instructions remain.
Both exporters restore it. Complete parsed protobuf graphs compare equal:
759 instructions and 68 flow graphs. Comparison clears export timestamps
and sorts the complete call-edge multiset because Ghidra's
`getCalledFunctions` returns an unordered set. Every instruction, operand,
basic block, vertex, edge, reference, and other field must still match.

The first loader attempt failed because the script's classloader could not
resolve a Ghidra exception; selecting the installed extension's classloader
fixed it. The first comparison rejected cross-JVM call-edge ordering;
inspection showed permutation only, then the narrowly normalized comparison
passed. These failures were confined to offline fixtures.

Reproduce with `r2_build_exporter.ps1 -AnalysisRoot <tools-root>`, CMake
configure/build of `tools/port3663/export-fixture` to
`build/r2/export-fixture` in Release, then `r2_export_fixture.ps1` with a fresh
`-Run` name. The compiled fixture is never run. Local tool/output hashes are
in `port-evidence/r2/exporter-fixture.json`.

The `nativefix` pipeline imports both full executables independently, uses
the recorded native analysis profile and RTTI seeds, saves each analysis in
its own headless process, then exports the full memory/code graph. Only after
both exports and database closure do VT and BinDiff start. Projects are
R2NativeSavedSource/old and R2NativeSavedTarget/new; raw output is
analysis-tools/r2-nativefix. This avoids losing completed analysis to a long
export. All production 3663 tiers remain disabled until their separate
adoption and validation requirements pass.
