param(
    [ValidateSet("windows-msvc-debug")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$modulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $modulePath -Force

Push-Location $projectRoot
try {
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot
    $cmake = Resolve-PlayerCMake -Layout $layout

    & $cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        throw "Test build failed with exit code $LASTEXITCODE."
    }

    & $cmake --build --preset $Preset --target test
    if ($LASTEXITCODE -ne 0) {
        throw "CTest target failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
