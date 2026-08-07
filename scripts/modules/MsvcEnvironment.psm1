Set-StrictMode -Version Latest

function Get-PlayerVsWherePath {
    [CmdletBinding()]
    param()

    $programFilesX86 = [Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFilesX86)
    $defaultPath = Join-Path $programFilesX86 "Microsoft Visual Studio/Installer/vswhere.exe"
    if (Test-Path -LiteralPath $defaultPath -PathType Leaf) {
        return $defaultPath
    }

    $command = Get-Command "vswhere.exe" -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    throw "vswhere.exe was not found. Install Visual Studio 2022 with the Desktop development with C++ workload."
}

function Find-PlayerVisualStudioInstallation {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions
    )

    $parsed = [version]$Versions.VisualStudioVersion
    $lower = "$($parsed.Major).$($parsed.Minor)"
    $upper = "$($parsed.Major).$($parsed.Minor + 1)"
    $vswhere = Get-PlayerVsWherePath

    $installationPath = [string](& $vswhere `
        -latest `
        -products * `
        -version "[$lower,$upper)" `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath `
        | Select-Object -First 1)

    $installationPath = $installationPath.Trim()
    if ([string]::IsNullOrWhiteSpace($installationPath)) {
        throw "Visual Studio 2022 $lower with the C++ x64 tools was not found."
    }

    return $installationPath
}

function Initialize-PlayerMsvcEnvironment {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions
    )

    $existingCompiler = Get-Command "cl.exe" -ErrorAction SilentlyContinue
    if ($existingCompiler -and $env:VSCMD_ARG_TGT_ARCH -eq "x64") {
        return
    }

    $installationPath = Find-PlayerVisualStudioInstallation -Versions $Versions
    $vcvars64 = Join-Path $installationPath "VC/Auxiliary/Build/vcvars64.bat"
    if (-not (Test-Path -LiteralPath $vcvars64 -PathType Leaf)) {
        throw "vcvars64.bat was not found under the Visual Studio installation '$installationPath'."
    }

    $commandProcessor = if ($env:ComSpec) { $env:ComSpec } else { "cmd.exe" }
    $commandLine = "call `"$vcvars64`" >nul && set"
    $environmentLines = @(& $commandProcessor /d /c $commandLine)
    if ($LASTEXITCODE -ne 0) {
        throw "Visual Studio x64 environment initialization failed with exit code $LASTEXITCODE."
    }

    foreach ($line in $environmentLines) {
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        $separator = $line.IndexOf('=')
        if ($separator -le 0) {
            continue
        }

        $name = $line.Substring(0, $separator)
        $value = $line.Substring($separator + 1)
        [Environment]::SetEnvironmentVariable($name, $value, "Process")
    }

    $compiler = Get-Command "cl.exe" -ErrorAction SilentlyContinue
    if (-not $compiler -or $env:VSCMD_ARG_TGT_ARCH -ne "x64") {
        throw "Visual Studio environment initialization completed, but the x64 MSVC compiler is still unavailable."
    }
}

Export-ModuleMember -Function @(
    "Get-PlayerVsWherePath",
    "Find-PlayerVisualStudioInstallation",
    "Initialize-PlayerMsvcEnvironment"
)
