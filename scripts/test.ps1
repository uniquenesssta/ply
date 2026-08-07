param(
    [ValidateSet("windows-msvc-debug")]
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
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout
    $ctest = Resolve-PlayerCTest -Layout $layout
    $qtRoot = Resolve-PlayerQtRoot -Layout $layout

    $qtBinRelative = Join-Path $qtRoot "bin"
    $qtPluginsRelative = Join-Path $qtRoot "plugins"
    $qtPlatformPluginsRelative = Join-Path $qtPluginsRelative "platforms"

    foreach ($runtimePath in @($qtBinRelative, $qtPluginsRelative, $qtPlatformPluginsRelative)) {
        if (-not (Test-Path -LiteralPath $runtimePath -PathType Container)) {
            throw "Required Qt runtime directory was not found at relative path '$runtimePath'."
        }
    }

    # Source control keeps only repository-parent-relative paths. Windows DLL
    # loading requires concrete process paths, so resolve them only in this
    # PowerShell process immediately before CTest starts.
    $qtBin = (Resolve-Path -LiteralPath $qtBinRelative).Path
    $qtPlugins = (Resolve-Path -LiteralPath $qtPluginsRelative).Path
    $qtPlatformPlugins = (Resolve-Path -LiteralPath $qtPlatformPluginsRelative).Path

    $originalPath = [Environment]::GetEnvironmentVariable("PATH", "Process")
    $originalQtPluginPath = [Environment]::GetEnvironmentVariable("QT_PLUGIN_PATH", "Process")
    $originalQtQpaPlatformPluginPath = [Environment]::GetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", "Process")

    try {
        $testPath = if ([string]::IsNullOrEmpty($originalPath)) { $qtBin } else { "$qtBin;$originalPath" }
        [Environment]::SetEnvironmentVariable("PATH", $testPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_PLUGIN_PATH", $qtPlugins, "Process")
        [Environment]::SetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", $qtPlatformPlugins, "Process")

        & $cmake --build --preset $Preset
        if ($LASTEXITCODE -ne 0) {
            throw "Test build failed with exit code $LASTEXITCODE."
        }

        & $ctest --preset $Preset
        if ($LASTEXITCODE -ne 0) {
            throw "CTest failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        [Environment]::SetEnvironmentVariable("PATH", $originalPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_PLUGIN_PATH", $originalQtPluginPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", $originalQtQpaPlatformPluginPath, "Process")
    }
}
finally {
    Pop-Location
}
