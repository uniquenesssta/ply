param(
    [ValidateSet("windows-msvc-debug")]
    [string]$Preset = "windows-msvc-debug",
    [switch]$Quick,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
$developmentRuntimeModulePath = Join-Path $PSScriptRoot "modules/DevelopmentRuntime.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force
Import-Module $developmentRuntimeModulePath -Force

Push-Location $projectRoot
try {
    & (Join-Path $PSScriptRoot "verify-dependencies.ps1")

    $versions = Get-PlayerDependencyVersions -ProjectRoot "."
    $layout = Get-PlayerWorkspaceLayout -Versions $versions
    $cmake = Resolve-PlayerCMake -Layout $layout
    $ctest = Resolve-PlayerCTest -Layout $layout
    $qtRoot = Resolve-PlayerQtRoot -Layout $layout

    Assert-PlayerDevelopmentRuntimeMarker `
        -Preset $Preset `
        -ProjectRoot $projectRoot

    $qtBinRelative = Join-Path $qtRoot "bin"
    $qtPluginsRelative = Join-Path $qtRoot "plugins"
    $qtPlatformPluginsRelative = Join-Path $qtPluginsRelative "platforms"
    $qtQmlImportsRelative = Join-Path $qtRoot "qml"

    foreach ($runtimePath in @($qtBinRelative, $qtPluginsRelative, $qtPlatformPluginsRelative, $qtQmlImportsRelative)) {
        if (-not (Test-Path -LiteralPath $runtimePath -PathType Container)) {
            throw "Required Qt runtime directory was not found at relative path '$runtimePath'."
        }
    }

    # Source control keeps only repository-parent-relative paths. Windows DLL
    # loading and QML module discovery require concrete process paths, so
    # resolve them only in this PowerShell process immediately before CTest.
    $qtBin = (Resolve-Path -LiteralPath $qtBinRelative).Path
    $qtPlugins = (Resolve-Path -LiteralPath $qtPluginsRelative).Path
    $qtPlatformPlugins = (Resolve-Path -LiteralPath $qtPlatformPluginsRelative).Path
    $qtQmlImports = (Resolve-Path -LiteralPath $qtQmlImportsRelative).Path

    $originalPath = [Environment]::GetEnvironmentVariable("PATH", "Process")
    $originalQtPluginPath = [Environment]::GetEnvironmentVariable("QT_PLUGIN_PATH", "Process")
    $originalQtQpaPlatformPluginPath = [Environment]::GetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", "Process")
    $originalQmlImportPath = [Environment]::GetEnvironmentVariable("QML_IMPORT_PATH", "Process")

    try {
        $testPath = if ([string]::IsNullOrEmpty($originalPath)) { $qtBin } else { "$qtBin;$originalPath" }
        $testQmlImportPath = if ([string]::IsNullOrEmpty($originalQmlImportPath)) { $qtQmlImports } else { "$qtQmlImports;$originalQmlImportPath" }
        [Environment]::SetEnvironmentVariable("PATH", $testPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_PLUGIN_PATH", $qtPlugins, "Process")
        [Environment]::SetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", $qtPlatformPlugins, "Process")
        [Environment]::SetEnvironmentVariable("QML_IMPORT_PATH", $testQmlImportPath, "Process")

        if ($SkipBuild) {
            Write-Host "[INFO] Test build skipped by -SkipBuild; existing binaries will be used."
        }
        else {
            & $cmake --build --preset $Preset
            if ($LASTEXITCODE -ne 0) {
                throw "Test build failed with exit code $LASTEXITCODE."
            }
        }

        $ctestArguments = @("--preset", $Preset)
        if ($Quick) {
            $ctestArguments += @("--label-exclude", "windowed-render")
            Write-Host "[INFO] Quick validation excludes CTest label 'windowed-render'."
            Write-Host "[INFO] Quick validation is not sufficient for Atomic Task or Stage closure."
        }

        & $ctest @ctestArguments
        if ($LASTEXITCODE -ne 0) {
            throw "CTest failed with exit code $LASTEXITCODE."
        }
    }
    finally {
        [Environment]::SetEnvironmentVariable("PATH", $originalPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_PLUGIN_PATH", $originalQtPluginPath, "Process")
        [Environment]::SetEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH", $originalQtQpaPlatformPluginPath, "Process")
        [Environment]::SetEnvironmentVariable("QML_IMPORT_PATH", $originalQmlImportPath, "Process")
    }
}
finally {
    Pop-Location
}
