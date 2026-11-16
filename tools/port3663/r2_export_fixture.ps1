param([Parameter(Mandatory=$true)][string]$AnalysisRoot,[string]$Run='validated')
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$env:JAVA_HOME=Join-Path $AnalysisRoot 'jdk-21.0.13+9'
$env:GHIDRA_HEADLESS_MAXMEM='4G'
$headless=Join-Path $AnalysisRoot 'ghidra_12.1.4_PUBLIC\support\analyzeHeadless.bat'
$folder=Join-Path $root 'build\r2\export-fixture'
$fixture=Join-Path $folder 'Release\r2_export_fixture.exe'
$jar=Join-Path $AnalysisRoot 'BinExport-defined-skip.jar'
foreach ($kind in @('stock','fixed')) {
    $output=Join-Path $folder "$kind.BinExport"
    $log=Join-Path $folder "$kind.log"
    $project="$Run-$kind"
    if (Test-Path (Join-Path $folder "$project.gpr")) { throw "Fixture project already exists: $project" }
    $export=if ($kind -eq 'stock') { @('R2Export.java',$output) } else { @('R2ExportFixed.java',$output,$jar) }
    & $headless $folder $project -import $fixture -scriptPath $PSScriptRoot -postScript R2FixtureOrphan.java -postScript @export -postScript R2FixtureCheck.java *> $log
    if ($LASTEXITCODE -ne 0 -or (Select-String -LiteralPath $log -Pattern 'REPORT SCRIPT ERROR') -or
        !(Select-String -LiteralPath $log -Pattern 'R2 fixture undefined function restored:')) {
        Get-Content $log -Tail 35
        throw "$kind fixture failed"
    }
}
$log=Join-Path $folder 'compare.log'
& $headless $folder "$Run-fixed" -process r2_export_fixture.exe -noanalysis -scriptPath $PSScriptRoot -postScript R2CompareExports.java (Join-Path $folder 'stock.BinExport') (Join-Path $folder 'fixed.BinExport') *> $log
if ($LASTEXITCODE -ne 0 -or (Select-String -LiteralPath $log -Pattern 'REPORT SCRIPT ERROR') -or
    !(Select-String -LiteralPath $log -Pattern 'R2 exporter fixture PASS:')) {
    Get-Content $log -Tail 35
    throw 'Exporter equivalence failed'
}
Select-String -LiteralPath $log -Pattern 'R2 exporter fixture PASS:'
