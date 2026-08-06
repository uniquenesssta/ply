$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$modulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $modulePath -Force

Push-Location $projectRoot
try {
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot
    $failures = [System.Collections.Generic.List[string]]::new()

    Write-Host "Dependency parent: .."

    try {
        $cmake = Resolve-PlayerCMake -Layout $layout
        Write-Host "[OK] CMake: $cmake"
    }
    catch {
        $failures.Add($_.Exception.Message)
        Write-Host "[MISSING] CMake"
    }

    try {
        $ninja = Resolve-PlayerNinja -Layout $layout
        Write-Host "[OK] Ninja: $ninja"
    }
    catch {
        $failures.Add($_.Exception.Message)
        Write-Host "[MISSING] Ninja"
    }

    try {
        $qtRoot = Resolve-PlayerQtRoot -Layout $layout
        Write-Host "[OK] Qt: $qtRoot"
    }
    catch {
        $failures.Add($_.Exception.Message)
        Write-Host "[MISSING] Qt 6 MSVC 2022 x64 kit"
    }

    $compiler = Get-Command "cl.exe" -ErrorAction SilentlyContinue
    if ($compiler) {
        Write-Host "[OK] MSVC compiler: $($compiler.Source)"
    }
    else {
        $failures.Add("cl.exe is not available. Run this script from an x64 Native Tools shell for VS 2022 after installing Desktop development with C++.")
        Write-Host "[MISSING] MSVC compiler in the current shell"
    }

    $targetArchitecture = $env:VSCMD_ARG_TGT_ARCH
    if ($targetArchitecture -eq "x64") {
        Write-Host "[OK] MSVC target architecture: x64"
    }
    else {
        $reportedArchitecture = if ([string]::IsNullOrWhiteSpace($targetArchitecture)) { "not initialized" } else { $targetArchitecture }
        $failures.Add("The active MSVC target architecture is '$reportedArchitecture'. Open 'x64 Native Tools Command Prompt for VS 2022' or initialize the developer shell with -arch=x64 -host_arch=x64.")
        Write-Host "[INVALID] MSVC target architecture: $reportedArchitecture"
    }

    $libMpvHeader = Join-Path $layout.LibMpvRoot "include/mpv/client.h"
    if (Test-Path -LiteralPath $libMpvHeader -PathType Leaf) {
        Write-Host "[INFO] libmpv SDK header found: $libMpvHeader"
    }
    else {
        Write-Host "[INFO] libmpv SDK is not installed yet; it is not required before Stage R2."
    }

    if ($failures.Count -gt 0) {
        $failureList = ($failures | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
        throw @"
Dependency verification failed:
$failureList
"@
    }

    Write-Host "All dependencies required by the current scaffold are available."
}
finally {
    Pop-Location
}
