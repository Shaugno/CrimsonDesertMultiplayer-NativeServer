# Read-only recovery verification for task backups that OneDrive made placeholders.
# Does not change saves, links, hydration policy or the general session guard.
# CLOUD through CLOUD_F: installed Windows SDK winnt.h IO_REPARSE_TAG_CLOUD_*.
param([Parameter(Mandatory=$true)][ValidatePattern('^R2-P1-[0-9]{8}-[0-9]{6}$')][string]$Session)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$backup=[IO.Path]::GetFullPath((Join-Path $root "backups\$Session"))
$saved=Join-Path $backup 'save'
$evidence=Join-Path $root "build\r2\live\$Session"
$live=Join-Path $env:LOCALAPPDATA 'Pearl Abyss\CD\save'
if (Get-Process CrimsonDesert -ErrorAction SilentlyContinue) { throw 'Game must be closed' }
$allowed=@(0..15 | ForEach-Object { [Convert]::ToUInt32('9000001A',16)+($_*4096) })
$tags=@()
foreach($item in @(Get-Item -LiteralPath $backup)+@(Get-ChildItem -LiteralPath $backup -Recurse -Force)) {
    if (!($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) { continue }
    $raw=(& fsutil reparsepoint query $item.FullName) -join "`n"
    if ($LASTEXITCODE -ne 0 -or $raw -notmatch 'Reparse Tag Value\s*:\s*0x([0-9a-fA-F]+)') { throw 'Cannot inspect reparse tag' }
    $tag=[Convert]::ToUInt32($Matches[1],16)
    if ($tag -notin $allowed) { throw 'Backup contains a non-cloud reparse point; refuse traversal' }
    $tags += ('0x{0:X8}' -f $tag)
}
$manifest=Join-Path $backup 'save-manifest.json'
$question=Get-Content -Raw -LiteralPath (Join-Path $evidence 'question.md')
if ($question -notmatch 'Backup manifest SHA256:\s*([A-Fa-f0-9]{64})') { throw 'Missing pre-session manifest digest' }
if ((Get-FileHash -LiteralPath $manifest).Hash -ne $Matches[1]) { throw 'Backup manifest changed' }
$before=@(Get-Content -Raw -LiteralPath $manifest | ConvertFrom-Json)
foreach($file in $before) {
    if ([IO.Path]::IsPathRooted($file.path) -or $file.path -match '(^|[\\/])\.\.([\\/]|$)') { throw 'Invalid backup-relative path' }
    $path=[IO.Path]::GetFullPath((Join-Path $saved $file.path))
    if (!$path.StartsWith($saved+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'Backup path escaped' }
    $item=Get-Item -LiteralPath $path
    if ($item.Length -ne $file.bytes -or (Get-FileHash -LiteralPath $path).Hash -ne $file.sha256) { throw 'Backup bytes differ' }
}
if (@(Get-ChildItem -LiteralPath $live -Recurse -Force | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) {
    throw 'Live save links are not allowed by this recovery verifier'
}
$after=@(Get-ChildItem -LiteralPath $live -Recurse -File -Force | Sort-Object FullName | ForEach-Object {
    [pscustomobject]@{path=$_.FullName.Substring($live.Length).TrimStart('\');bytes=$_.Length;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash}
})
$unchanged=(ConvertTo-Json -InputObject @($before) -Depth 8 -Compress) -eq (ConvertTo-Json -InputObject @($after) -Depth 8 -Compress)
$result=[ordered]@{session=$Session;backup_verified=$true;live_saves_unchanged=$unchanged;
    before_files=$before.Count;after_files=$after.Count;cloud_tags=@($tags | Sort-Object -Unique);
    cloud_reparse_count=$tags.Count;save_writes=$false;verification='Exact manifest digest and every backup file size/hash; only Windows CLOUD tags accepted; live saves retain the strict no-reparse guard'}
$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $evidence 'save-verification.json')
$result | ConvertTo-Json -Depth 8
