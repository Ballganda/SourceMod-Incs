# Compiles every tests\*.sp against scripting\include with the spcomp that ships with the SourceMod install on
# the local test server. Each include has at least one test plugin, so this checks every include compiles.
#   .\build.ps1            compile only
#   .\build.ps1 -Deploy    compile, then copy the .smx files to the server
param(
    [switch]$Deploy,
    [string]$SourceModDir = $(if ($env:SOURCEMOD_DIR) { $env:SOURCEMOD_DIR } else { 'C:\Users\user\cssds\cstrike\addons\sourcemod' })
)

$spcomp = Join-Path $SourceModDir 'scripting\spcomp.exe'
if (-not (Test-Path $spcomp)) { Write-Error "spcomp not found at $spcomp"; exit 1 }

$inc = Join-Path $PSScriptRoot 'scripting\include'
$out = Join-Path $PSScriptRoot 'plugins'
New-Item -ItemType Directory -Force $out | Out-Null

$failed = 0
foreach ($sp in Get-ChildItem (Join-Path $PSScriptRoot 'tests') -Filter *.sp) {
    $smx = Join-Path $out ($sp.BaseName + '.smx')
    & $spcomp $sp.FullName "-i$inc" "-i$SourceModDir\scripting\include" "-o$smx"
    if ($LASTEXITCODE -ne 0) { $failed++; continue }
    if ($Deploy) {
        Copy-Item $smx (Join-Path $SourceModDir 'plugins') -Force
        Write-Host "Deployed $($sp.BaseName).smx"
    }
}
if ($failed) { Write-Error "$failed test plugin(s) failed to compile"; exit 1 }
