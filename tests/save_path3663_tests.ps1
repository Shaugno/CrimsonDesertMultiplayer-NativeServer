param([Parameter(Mandatory=$true)][string]$FixtureExe)
$ErrorActionPreference='Stop'
$baselineOutput=& $FixtureExe
if($LASTEXITCODE -or $baselineOutput -notmatch '^RESOLVED='){throw 'Native resolver baseline failed'}
$baseline=$baselineOutput.Substring(9)
$temp=[IO.Path]::GetFullPath([IO.Path]::GetTempPath())
foreach($mode in @('LOCALAPPDATA','USERPROFILE','both')){
    $start=New-Object Diagnostics.ProcessStartInfo
    $start.FileName=$FixtureExe;$start.UseShellExecute=$false;$start.CreateNoWindow=$true
    $start.RedirectStandardOutput=$true;$start.RedirectStandardError=$true
    $start.Arguments='"'+$baseline+'"'
    $override=Join-Path $temp ('cdmp-r5-override-'+[guid]::NewGuid().ToString('N'))
    $isolatedLocal=Join-Path $override 'AppData\Local'
    New-Item -ItemType Directory -Path $isolatedLocal | Out-Null
    if($mode -in @('LOCALAPPDATA','both')){$start.EnvironmentVariables['LOCALAPPDATA']=$isolatedLocal}
    if($mode -in @('USERPROFILE','both')){$start.EnvironmentVariables['USERPROFILE']=$override}
    $expected=if($mode -eq 'LOCALAPPDATA'){$baseline}else{$isolatedLocal}
    $start.Arguments='"'+$expected+'"'
    try{
        $child=[Diagnostics.Process]::Start($start)
        $output=$child.StandardOutput.ReadToEnd();$errorText=$child.StandardError.ReadToEnd()
        if(!$child.WaitForExit(5000) -or $child.ExitCode -ne 0){throw "Native override probe failed: $mode exit=$($child.ExitCode) output=$output error=$errorText"}
        if($output.Trim() -cne ('RESOLVED='+$expected)){throw 'Native known-folder resolution differed from expected profile'}
        Write-Output "$mode native resolver PASS; redirected=$($mode -ne 'LOCALAPPDATA'); existing isolated profile directory required"
    }finally{
        $full=[IO.Path]::GetFullPath($override)
        if(!$full.StartsWith($temp,[StringComparison]::OrdinalIgnoreCase) -or (Split-Path $full -Leaf) -notlike 'cdmp-r5-override-*'){throw 'Invalid override cleanup'}
        Remove-Item -LiteralPath $full -Recurse -Force
    }
}
