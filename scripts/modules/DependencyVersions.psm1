Set-StrictMode -Version Latest

function Get-PlayerDependencyVersions {
    [CmdletBinding()]
    param(
        [string]$ProjectRoot = "."
    )

    $versionFile = Join-Path $ProjectRoot "cmake/DependencyVersions.cmake"
    if (-not (Test-Path -LiteralPath $versionFile -PathType Leaf)) {
        throw "Dependency version manifest was not found at relative path '$versionFile'."
    }

    $rawValues = @{}
    foreach ($line in Get-Content -LiteralPath $versionFile) {
        if ($line -match '^\s*set\(\s*(PLAYER_[A-Z0-9_]+)\s+"([^"]+)"(?:\s+CACHE\s+INTERNAL\s+"[^"]*")?\s*\)\s*$') {
            $rawValues[$Matches[1]] = $Matches[2]
        }
    }

    $required = [ordered]@{
        WindowsMinBuild         = "PLAYER_WINDOWS_MIN_BUILD"
        WindowsPrimaryBuild     = "PLAYER_WINDOWS_PRIMARY_BUILD"
        WindowsSdkVersion       = "PLAYER_WINDOWS_SDK_VERSION"
        WindowsSdkRelease       = "PLAYER_WINDOWS_SDK_RELEASE"
        VisualStudioVersion     = "PLAYER_VISUAL_STUDIO_VERSION"
        VisualStudioBuild       = "PLAYER_VISUAL_STUDIO_BUILD"
        MsvcToolsetVersion      = "PLAYER_MSVC_TOOLSET_VERSION"
        MsvcCompilerVersion     = "PLAYER_MSVC_COMPILER_VERSION"
        QtVersion               = "PLAYER_QT_VERSION"
        CMakeVersion            = "PLAYER_CMAKE_VERSION"
        NinjaVersion            = "PLAYER_NINJA_VERSION"
        MpvVersion              = "PLAYER_MPV_VERSION"
        MpvTag                  = "PLAYER_MPV_TAG"
        MpvCommit               = "PLAYER_MPV_COMMIT"
        FfmpegVersion           = "PLAYER_FFMPEG_VERSION"
        FfmpegRef               = "PLAYER_FFMPEG_REF"
        FfmpegCommit            = "PLAYER_FFMPEG_COMMIT"
        FfmpegArchiveUrl        = "PLAYER_FFMPEG_ARCHIVE_URL"
        FfmpegArchiveSignatureUrl = "PLAYER_FFMPEG_ARCHIVE_SIGNATURE_URL"
        FfmpegSigningKeyUrl     = "PLAYER_FFMPEG_SIGNING_KEY_URL"
        FfmpegSigningFingerprint = "PLAYER_FFMPEG_SIGNING_FINGERPRINT"
        LibplaceboVersion       = "PLAYER_LIBPLACEBO_VERSION"
        LibplaceboRef           = "PLAYER_LIBPLACEBO_REF"
        LibplaceboCommit        = "PLAYER_LIBPLACEBO_COMMIT"
        LibassVersion           = "PLAYER_LIBASS_VERSION"
        LibassRef               = "PLAYER_LIBASS_REF"
        LibassCommit            = "PLAYER_LIBASS_COMMIT"
        FreetypeVersion         = "PLAYER_FREETYPE_VERSION"
        FreetypeRef             = "PLAYER_FREETYPE_REF"
        FreetypeCommit          = "PLAYER_FREETYPE_COMMIT"
        FribidiVersion          = "PLAYER_FRIBIDI_VERSION"
        FribidiRef              = "PLAYER_FRIBIDI_REF"
        FribidiCommit           = "PLAYER_FRIBIDI_COMMIT"
        HarfbuzzVersion         = "PLAYER_HARFBUZZ_VERSION"
        HarfbuzzRef             = "PLAYER_HARFBUZZ_REF"
        HarfbuzzCommit          = "PLAYER_HARFBUZZ_COMMIT"
        CxxStandard             = "PLAYER_CXX_STANDARD"
    }

    $resolved = [ordered]@{}
    foreach ($propertyName in $required.Keys) {
        $variableName = $required[$propertyName]
        if (-not $rawValues.ContainsKey($variableName) -or
            [string]::IsNullOrWhiteSpace($rawValues[$variableName])) {
            throw "Required dependency version '$variableName' is missing from relative manifest '$versionFile'."
        }
        $resolved[$propertyName] = $rawValues[$variableName]
    }

    [pscustomobject]$resolved
}

Export-ModuleMember -Function "Get-PlayerDependencyVersions"
