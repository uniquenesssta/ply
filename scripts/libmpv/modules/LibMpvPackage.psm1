Set-StrictMode -Version Latest

function Read-PlayerLibMpvMetadataValue {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout,

        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $path = Join-Path $Layout.CacheRootRelative "metadata/$Name"
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "libmpv build metadata '$Name' was not found at '$path'."
    }

    return (Get-Content -LiteralPath $path -Raw).Trim()
}

function New-PlayerLibMpvMsvcImportLibrary {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $definitionFile = Join-Path $Layout.PackageRootRelative "lib/mpv.def"
    $outputLibrary = Join-Path $Layout.PackageRootRelative "lib/mpv.lib"

    if (-not (Test-Path -LiteralPath $definitionFile -PathType Leaf)) {
        throw "libmpv export definition was not generated at '$definitionFile'."
    }

    $libCommand = Get-Command "lib.exe" -ErrorAction SilentlyContinue
    if (-not $libCommand) {
        throw "MSVC lib.exe is unavailable. Initialize the Visual Studio x64 environment before creating mpv.lib."
    }

    & $libCommand.Source "/nologo" "/def:$definitionFile" "/machine:x64" "/out:$outputLibrary"
    if ($LASTEXITCODE -ne 0) {
        throw "MSVC import-library generation failed with exit code $LASTEXITCODE."
    }

    if (-not (Test-Path -LiteralPath $outputLibrary -PathType Leaf)) {
        throw "MSVC import-library generation completed without producing '$outputLibrary'."
    }

    return $outputLibrary
}

function Get-PlayerArtifactRecord {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$PackageRoot,

        [Parameter(Mandatory = $true)]
        [System.IO.FileInfo]$File
    )

    $packageRootFull = [System.IO.Path]::GetFullPath($PackageRoot).TrimEnd([char]92, [char]47)
    $fileFull = $File.FullName
    $relative = $fileFull.Substring($packageRootFull.Length).TrimStart([char]92, [char]47) -replace '\\', '/'
    $hash = (Get-FileHash -LiteralPath $fileFull -Algorithm SHA256).Hash.ToLowerInvariant()

    [ordered]@{
        path = $relative
        sha256 = $hash
        bytes = [int64]$File.Length
    }
}

function Write-PlayerLibMpvDependencyManifest {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions,

        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $packageRoot = [System.IO.Path]::GetFullPath($Layout.PackageRootRelative)
    $manifestPath = Join-Path $packageRoot "dependency-manifest.json"

    $artifactFiles = @(
        Get-ChildItem -LiteralPath (Join-Path $packageRoot "bin") -Filter "*.dll" -File -ErrorAction Stop
        Get-ChildItem -LiteralPath (Join-Path $packageRoot "lib") -Filter "*.lib" -File -ErrorAction Stop
        Get-Item -LiteralPath (Join-Path $packageRoot "include/mpv/client.h") -ErrorAction Stop
    ) | Sort-Object FullName -Unique

    $artifacts = @($artifactFiles | ForEach-Object {
        Get-PlayerArtifactRecord -PackageRoot $packageRoot -File $_
    })

    $sourceNames = @(
        "mpv",
        "ffmpeg",
        "libplacebo",
        "libass",
        "freetype",
        "fribidi",
        "harfbuzz"
    )

    $sourceCommits = [ordered]@{}
    foreach ($sourceName in $sourceNames) {
        $sourceCommits[$sourceName] = Read-PlayerLibMpvMetadataValue -Layout $Layout -Name "$sourceName.commit"
    }

    $commitChecks = @(
        @("mpv", $sourceCommits.mpv, $Versions.MpvCommit),
        @("ffmpeg", $sourceCommits.ffmpeg, $Versions.FfmpegCommit),
        @("libplacebo", $sourceCommits.libplacebo, $Versions.LibplaceboCommit),
        @("libass", $sourceCommits.libass, $Versions.LibassCommit),
        @("freetype", $sourceCommits.freetype, $Versions.FreetypeCommit),
        @("fribidi", $sourceCommits.fribidi, $Versions.FribidiCommit),
        @("harfbuzz", $sourceCommits.harfbuzz, $Versions.HarfbuzzCommit)
    )
    foreach ($check in $commitChecks) {
        if ($check[1] -ne $check[2]) {
            throw "Built $($check[0]) source commit mismatch. Expected $($check[2]), found $($check[1])."
        }
    }

    $manifest = [ordered]@{
        schemaVersion = 1
        target = "windows-x64"
        build = [ordered]@{
            toolchain = "MSYS2 CLANG64"
            generatedAtUtc = [DateTime]::UtcNow.ToString("o")
        }
        mpv = [ordered]@{
            version = $Versions.MpvVersion
            tag = $Versions.MpvTag
            commit = $Versions.MpvCommit
            sourceCommit = $sourceCommits.mpv
        }
        ffmpeg = [ordered]@{
            version = $Versions.FfmpegVersion
            ref = $Versions.FfmpegRef
            commit = $Versions.FfmpegCommit
            sourceCommit = $sourceCommits.ffmpeg
        }
        libplacebo = [ordered]@{
            version = $Versions.LibplaceboVersion
            ref = $Versions.LibplaceboRef
            commit = $Versions.LibplaceboCommit
            sourceCommit = $sourceCommits.libplacebo
        }
        libass = [ordered]@{
            version = $Versions.LibassVersion
            ref = $Versions.LibassRef
            commit = $Versions.LibassCommit
            sourceCommit = $sourceCommits.libass
        }
        freetype = [ordered]@{
            version = $Versions.FreetypeVersion
            ref = $Versions.FreetypeRef
            commit = $Versions.FreetypeCommit
            sourceCommit = $sourceCommits.freetype
        }
        fribidi = [ordered]@{
            version = $Versions.FribidiVersion
            ref = $Versions.FribidiRef
            commit = $Versions.FribidiCommit
            sourceCommit = $sourceCommits.fribidi
        }
        harfbuzz = [ordered]@{
            version = $Versions.HarfbuzzVersion
            ref = $Versions.HarfbuzzRef
            commit = $Versions.HarfbuzzCommit
            sourceCommit = $sourceCommits.harfbuzz
        }
        buildPolicy = [ordered]@{
            mpv = @(
                "gpl=false",
                "cplayer=false",
                "libmpv=true",
                "default_library=shared",
                "build-date=false"
            )
            ffmpeg = @(
                "disable-autodetect",
                "disable-gpl",
                "disable-nonfree",
                "enable-shared",
                "disable-static"
            )
        }
        artifacts = $artifacts
    }

    $json = $manifest | ConvertTo-Json -Depth 8
    [System.IO.File]::WriteAllText(
        $manifestPath,
        $json,
        [System.Text.UTF8Encoding]::new($false)
    )
    return $manifestPath
}

function Test-PlayerLibMpvDependencyManifest {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [psobject]$Versions,

        [Parameter(Mandatory = $true)]
        [psobject]$Layout
    )

    $manifestPath = Join-Path $Layout.PackageRootRelative "dependency-manifest.json"
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        throw "libmpv dependency manifest was not found at '$manifestPath'."
    }

    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    $checks = @(
        @("mpv.version", [string]$manifest.mpv.version, $Versions.MpvVersion),
        @("mpv.tag", [string]$manifest.mpv.tag, $Versions.MpvTag),
        @("mpv.commit", [string]$manifest.mpv.commit, $Versions.MpvCommit),
        @("mpv.sourceCommit", [string]$manifest.mpv.sourceCommit, $Versions.MpvCommit),
        @("ffmpeg.version", [string]$manifest.ffmpeg.version, $Versions.FfmpegVersion),
        @("ffmpeg.commit", [string]$manifest.ffmpeg.commit, $Versions.FfmpegCommit),
        @("ffmpeg.sourceCommit", [string]$manifest.ffmpeg.sourceCommit, $Versions.FfmpegCommit),
        @("libplacebo.version", [string]$manifest.libplacebo.version, $Versions.LibplaceboVersion),
        @("libplacebo.commit", [string]$manifest.libplacebo.commit, $Versions.LibplaceboCommit),
        @("libplacebo.sourceCommit", [string]$manifest.libplacebo.sourceCommit, $Versions.LibplaceboCommit),
        @("libass.version", [string]$manifest.libass.version, $Versions.LibassVersion),
        @("libass.commit", [string]$manifest.libass.commit, $Versions.LibassCommit),
        @("libass.sourceCommit", [string]$manifest.libass.sourceCommit, $Versions.LibassCommit),
        @("freetype.version", [string]$manifest.freetype.version, $Versions.FreetypeVersion),
        @("freetype.commit", [string]$manifest.freetype.commit, $Versions.FreetypeCommit),
        @("freetype.sourceCommit", [string]$manifest.freetype.sourceCommit, $Versions.FreetypeCommit),
        @("fribidi.version", [string]$manifest.fribidi.version, $Versions.FribidiVersion),
        @("fribidi.commit", [string]$manifest.fribidi.commit, $Versions.FribidiCommit),
        @("fribidi.sourceCommit", [string]$manifest.fribidi.sourceCommit, $Versions.FribidiCommit),
        @("harfbuzz.version", [string]$manifest.harfbuzz.version, $Versions.HarfbuzzVersion),
        @("harfbuzz.commit", [string]$manifest.harfbuzz.commit, $Versions.HarfbuzzCommit),
        @("harfbuzz.sourceCommit", [string]$manifest.harfbuzz.sourceCommit, $Versions.HarfbuzzCommit)
    )

    foreach ($check in $checks) {
        if ($check[1] -ne $check[2]) {
            throw "libmpv manifest $($check[0]) mismatch. Expected '$($check[2])', found '$($check[1])'."
        }
    }

    $artifactEntries = @($manifest.artifacts)
    if ($artifactEntries.Count -eq 0) {
        throw "libmpv dependency manifest contains no artifact hashes."
    }

    foreach ($artifact in $artifactEntries) {
        $artifactPath = Join-Path $Layout.PackageRootRelative ([string]$artifact.path)
        if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
            throw "Manifest artifact is missing from the package: '$($artifact.path)'."
        }

        $actualHash = (Get-FileHash -LiteralPath $artifactPath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne ([string]$artifact.sha256).ToLowerInvariant()) {
            throw "Manifest artifact hash mismatch for '$($artifact.path)'."
        }
    }

    return $manifest
}

Export-ModuleMember -Function @(
    "New-PlayerLibMpvMsvcImportLibrary",
    "Write-PlayerLibMpvDependencyManifest",
    "Test-PlayerLibMpvDependencyManifest"
)
