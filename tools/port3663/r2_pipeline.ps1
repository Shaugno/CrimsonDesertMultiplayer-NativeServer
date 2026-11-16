param([Parameter(Mandatory=$true)][string]$AnalysisRoot)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$jobs=@()
foreach ($image in @('old','new')) {
    $jobs += Start-Job -ArgumentList $root,$AnalysisRoot,$image -ScriptBlock {
        param($root,$ar,$image)
        Set-Location -LiteralPath $root
        $ErrorActionPreference='Stop'
        & tools/port3663/r2_analysis.ps1 -Image $image -Mode analyze -Profile nativefix -AnalysisRoot $ar
    }
}
$jobs | Wait-Job | Receive-Job
if (@($jobs | Where-Object State -ne 'Completed').Count) { throw 'Analysis/export job failed' }
Write-Output 'Both whole-image exports saved; starting unchanged VT and BinDiff'
$jobs=@()
$jobs += Start-Job -ArgumentList $root,$AnalysisRoot -ScriptBlock {
    param($root,$ar)
    Set-Location -LiteralPath $root
    $ErrorActionPreference='Stop'
    & tools/port3663/r2_bindiff.ps1 -AnalysisRoot $ar -Profile nativefix
}
$jobs += Start-Job -ArgumentList $root,$AnalysisRoot -ScriptBlock {
    param($root,$ar)
    Set-Location -LiteralPath $root
    $ErrorActionPreference='Stop'
    & tools/port3663/r2_analysis.ps1 -Image new -Mode vt -Profile nativefix -AnalysisRoot $ar
}
$jobs | Wait-Job | Receive-Job
if (@($jobs | Where-Object State -ne 'Completed').Count) { throw 'Standard matcher job failed' }
Write-Output 'Both standard whole-program matchers completed'
