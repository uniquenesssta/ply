$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent $PSScriptRoot
$versionModulePath = Join-Path $PSScriptRoot "modules/DependencyVersions.psm1"
$pathModulePath = Join-Path $PSScriptRoot "modules/DependencyPaths.psm1"
Import-Module $versionModulePath -Force
Import-Module $pathModulePath -Force

Push-Location $projectRoot
try {
    $stagedFiles = @(git diff --cached --name-only --diff-filter=ACMR)
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to inspect the Git staging area."
    }

    $forbiddenPatterns = @(
        '(^|/)(Qt|qt|libmpv|downloads|cache)(/|$)',
        '(^|/)(build|out|dist)(/|$)'
    )
    $forbiddenStagedFiles = @(
        $stagedFiles | Where-Object {
            $path = ($_ -replace '\\', '/')
            $forbiddenPatterns | Where-Object { $path -match $_ }
        }
    )
    if ($forbiddenStagedFiles.Count -gt 0) {
        throw "Third-party SDK, download, cache, or build files are staged:`n$($forbiddenStagedFiles -join [Environment]::NewLine)"
    }

    $changes = git status --porcelain
    if ($LASTEXITCODE -ne 0) {
        throw "git status failed."
    }
    if ($changes) {
        throw "The working tree is not clean:`n$changes"
    }

    $versions = Get-PlayerDependencyVersions -ProjectRoot $projectRoot
    $layout = Get-PlayerWorkspaceLayout -ProjectRoot $projectRoot -Versions $versions
    foreach ($sharedPath in @(
        $layout.QtRoot,
        $layout.LibMpvRoot,
        $layout.CMakeRoot,
        $layout.NinjaRoot,
        $layout.DownloadsRoot,
        $layout.FetchContentRoot
    )) {
        $relativeToRepository = [System.IO.Path]::GetRelativePath($layout.ProjectRoot, $sharedPath)
        if (-not $relativeToRepository.StartsWith("..", [System.StringComparison]::Ordinal)) {
            throw "Shared dependency path unexpectedly resolves inside the repository: $sharedPath"
        }
    }

    Write-Host "Working tree is clean and shared dependency locations remain outside the repository."
}
finally {
    Pop-Location
}
