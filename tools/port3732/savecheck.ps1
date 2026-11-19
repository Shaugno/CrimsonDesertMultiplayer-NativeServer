<#
Read-only: compare a save tree with a CDMP-R4-SAVE-1 manifest (sha256<TAB>size<TAB>relative path).
Reports files that are unchanged, changed, missing and extra. Never writes to either tree.
usage: savecheck.ps1 -Manifest <file.tsv> -Root <save folder>
#>
param([Parameter(Mandatory=$true)][string]$Manifest,[Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference='Stop'
$lines=Get-Content -LiteralPath $Manifest
if($lines[0] -ne 'CDMP-R4-SAVE-1'){throw 'Unknown manifest format'}
$want=@{}
foreach($l in $lines | Select-Object -Skip 1){ if($l){ $c=$l -split "`t"; $want[$c[2]]=@{sha=$c[0];size=[long]$c[1]} } }
$rootFull=(Resolve-Path -LiteralPath $Root).Path.TrimEnd('\')
$have=@{}
Get-ChildItem -LiteralPath $rootFull -Recurse -File | ForEach-Object { $have[$_.FullName.Substring($rootFull.Length+1)]=$_ }
$same=0;$changed=@();$missing=@();$extra=@()
foreach($k in $want.Keys){
    if(!$have.ContainsKey($k)){$missing+=$k;continue}
    $h=(Get-FileHash -LiteralPath $have[$k].FullName -Algorithm SHA256).Hash
    if($h -eq $want[$k].sha){$same++}else{$changed+="$k (modified $($have[$k].LastWriteTime))"}
}
foreach($k in $have.Keys){ if(!$want.ContainsKey($k)){$extra+="$k (modified $($have[$k].LastWriteTime))"} }
"manifest files $($want.Count): unchanged $same, changed $($changed.Count), missing $($missing.Count), extra $($extra.Count)"
$changed | ForEach-Object { "  changed: $_" }
$missing | ForEach-Object { "  missing: $_" }
$extra | ForEach-Object { "  extra:   $_" }
