param([Parameter(Mandatory=$true)][string]$AnalysisRoot)
$ErrorActionPreference='Stop'
$checkout=Join-Path $AnalysisRoot 'binexport-source'
$revision=& git -C $checkout rev-parse HEAD
if ($LASTEXITCODE -or $revision -ne '9648cacb21a71aa6c21b3fca504f146c39164a53') { throw 'Unexpected BinExport source revision' }
$source=Join-Path $checkout 'java\src\main\java\com\google\security\binexport\BinExport2Builder.java'
$original=& git -C $checkout show 'HEAD:java/src/main/java/com/google/security/binexport/BinExport2Builder.java'
if ($LASTEXITCODE) { throw 'Cannot read original builder' }
$text=($original -join "`n")+"`n"
$old='if (func != null && func.isExternal()) {'
if (($text.Split($old).Count-1) -ne 1) { throw 'Patch precondition failed' }
$patched=$text.Replace($old,'if (func != null) {')
$current=[IO.File]::ReadAllText($source).Replace("`r`n","`n")
if ($current -ne $text -and $current -ne $patched) { throw 'Builder has unrelated edits; preserve them' }
[IO.File]::WriteAllText($source,$patched,[Text.UTF8Encoding]::new($false))
$env:JAVA_HOME=Join-Path $AnalysisRoot 'jdk-21.0.13+9'
$env:GHIDRA_INSTALL_DIR=Join-Path $AnalysisRoot 'ghidra_12.1.4_PUBLIC'
& (Join-Path $AnalysisRoot 'gradle-8.14.3\bin\gradle.bat') -p (Join-Path $checkout 'java') --offline --no-daemon jar
if ($LASTEXITCODE) { throw 'Fixed exporter build failed' }
$jar=Join-Path $AnalysisRoot 'BinExport-defined-skip.jar'
Copy-Item -LiteralPath (Join-Path $checkout 'java\build\libs\BinExport.jar') -Destination $jar
Get-FileHash -LiteralPath $source,$jar -Algorithm SHA256
