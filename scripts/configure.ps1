param(
    [ValidateSet("windows-msvc-debug", "windows-msvc-release")]
    [string]$Preset = "windows-msvc-debug"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$modulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $modulePath -Force

Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot "verify-project-layout.ps1")
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot
    $cmake = Resolve-PlayerCMake -Layout $layout
    $ninja = Resolve-PlayerNinja -Layout $layout
    $qtRoot = Resolve-PlayerQtRoot -Layout $layout

    $configureArguments = @(
        "--preset", $Preset,
        "-DPLAYER_QT_ROOT:STRING=$qtRoot",
        "-DPLAYER_LIBMPV_ROOT:STRING=../libmpv/windows-x64",
        "-DPLAYER_FETCHCONTENT_ROOT:STRING=../cache/cmake/fetchcontent",
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
