param(
    [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force

Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot "verify-project-layout.ps1")
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $versions = Get-PlayerDependencyVersions -ProjectRoot $projectRoot
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout
    $ninja = Resolve-PlayerNinja -Layout $layout
    Resolve-PlayerQtRoot -Layout $layout | Out-Null

    $configureArguments = @(
        "--preset", $Preset,
        "-DCMAKE_MAKE_PROGRAM:FILEPATH=$ninja"
    )

    & $cmake @configureArguments
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
