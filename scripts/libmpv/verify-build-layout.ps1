$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$requiredFiles = @(
    "scripts/libmpv/bootstrap-build-environment.ps1",
    "scripts/libmpv/build-package.ps1",
    "scripts/libmpv/verify-package.ps1",
    "scripts/libmpv/modules/LibMpvBuildPaths.psm1",
    "scripts/libmpv/modules/Msys2Environment.psm1",
    "scripts/libmpv/modules/LibMpvPackage.psm1",
    "scripts/libmpv/clang64/common.sh",
    "scripts/libmpv/clang64/build-all.sh",
    "scripts/libmpv/clang64/package-runtime.sh",
    "scripts/libmpv/clang64/build/freetype.sh",
    "scripts/libmpv/clang64/build/fribidi.sh",
    "scripts/libmpv/clang64/build/harfbuzz.sh",
    "scripts/libmpv/clang64/build/libass.sh",
    "scripts/libmpv/clang64/build/libplacebo.sh",
    "scripts/libmpv/clang64/build/ffmpeg.sh",
    "scripts/libmpv/clang64/build/mpv.sh"
)

Push-Location $projectRoot
try {
    foreach ($relativePath in $requiredFiles) {
        if (-not (Test-Path -LiteralPath $relativePath -PathType Leaf)) {
            throw "Required libmpv build-chain file is missing: $relativePath"
        }
    }

    $versionFile = Get-Content -LiteralPath "cmake/DependencyVersions.cmake" -Raw
    foreach ($fragment in @(
        'PLAYER_MPV_VERSION "0.41.0"',
        'PLAYER_MPV_COMMIT "41f6a645068483470267271e1d09966ca3b9f413"',
        'PLAYER_FFMPEG_VERSION "8.0.3"',
        'PLAYER_FFMPEG_COMMIT "8ae0b34901ba60a802f183ee75a250a9fc3e09a5"',
        'PLAYER_LIBPLACEBO_VERSION "7.351.0"',
        'PLAYER_LIBPLACEBO_COMMIT "3188549fba13bbdf3a5a98de2a38c2e71f04e21e"',
        'PLAYER_LIBASS_VERSION "0.17.4"',
        'PLAYER_LIBASS_COMMIT "bbb3c7f1570a4a021e52683f3fbdf74fe492ae84"',
        'PLAYER_FREETYPE_VERSION "2.13.3"',
        'PLAYER_FREETYPE_COMMIT "42608f77f20749dd6ddc9e0536788eaad70ea4b5"',
        'PLAYER_FRIBIDI_VERSION "1.0.16"',
        'PLAYER_FRIBIDI_COMMIT "68162babff4f39c4e2dc164a5e825af93bda9983"',
        'PLAYER_HARFBUZZ_VERSION "10.2.0"',
        'PLAYER_HARFBUZZ_COMMIT "7b27c8edd46c674e01dd226fa9e1aa7549f5c436"'
    )) {
        if (-not $versionFile.Contains($fragment)) {
            throw "DependencyVersions.cmake is missing required libmpv source-build identity: $fragment"
        }
    }

    $buildPathModule = Get-Content -LiteralPath "scripts/libmpv/modules/LibMpvBuildPaths.psm1" -Raw
    foreach ($fragment in @("../downloads/libmpv", "../cache/libmpv-build", "../libmpv/")) {
        if (-not $buildPathModule.Contains($fragment)) {
            throw "LibMpvBuildPaths.psm1 is missing required parent-relative build path: $fragment"
        }
    }
    if ($buildPathModule -match '[A-Za-z]:[/\\]') {
        throw "LibMpvBuildPaths.psm1 contains a machine-absolute Windows path."
    }

    $msysInvocationModule = Get-Content -LiteralPath "scripts/libmpv/modules/Msys2Environment.psm1" -Raw
    foreach ($fragment in @(
        'Get-PlayerMsys2Root',
        'Join-Path $msysRoot "tmp"',
        '$msysScriptPath = "/tmp/$scriptName"',
        '[System.IO.File]::WriteAllText(',
        '[System.Text.UTF8Encoding]::new($false)',
        '& $BashPath --login $msysScriptPath',
        'Remove-Item -LiteralPath $temporaryScript -Force'
    )) {
        if (-not $msysInvocationModule.Contains($fragment)) {
            throw "Msys2Environment.psm1 is missing MSYS-root temporary-script invocation fragment: $fragment"
        }
    }
    if ($msysInvocationModule.Contains('cygpath.exe')) {
        throw "Msys2Environment.psm1 must not depend on cygpath for its own temporary script path."
    }
    if ($msysInvocationModule.Contains('& $BashPath --login -lc $Command')) {
        throw "Msys2Environment.psm1 must not pass arbitrary command text directly through bash -lc."
    }

    $sourceBuildCommitVariables = [ordered]@{
        "scripts/libmpv/clang64/build/freetype.sh"   = "PLAYER_FREETYPE_COMMIT"
        "scripts/libmpv/clang64/build/fribidi.sh"    = "PLAYER_FRIBIDI_COMMIT"
        "scripts/libmpv/clang64/build/harfbuzz.sh"   = "PLAYER_HARFBUZZ_COMMIT"
        "scripts/libmpv/clang64/build/libass.sh"     = "PLAYER_LIBASS_COMMIT"
        "scripts/libmpv/clang64/build/libplacebo.sh" = "PLAYER_LIBPLACEBO_COMMIT"
        "scripts/libmpv/clang64/build/ffmpeg.sh"     = "PLAYER_FFMPEG_COMMIT"
        "scripts/libmpv/clang64/build/mpv.sh"        = "PLAYER_MPV_COMMIT"
    }
    foreach ($entry in $sourceBuildCommitVariables.GetEnumerator()) {
        $sourceBuild = Get-Content -LiteralPath $entry.Key -Raw
        if (-not $sourceBuild.Contains($entry.Value)) {
            throw "$($entry.Key) does not enforce pinned source commit $($entry.Value)."
        }
    }

    $ffmpegBuild = Get-Content -LiteralPath "scripts/libmpv/clang64/build/ffmpeg.sh" -Raw
    foreach ($fragment in @(
        "--disable-autodetect",
        "--disable-gpl",
        "--disable-nonfree",
        "--enable-shared",
        "--disable-static"
    )) {
        if (-not $ffmpegBuild.Contains($fragment)) {
            throw "FFmpeg build policy is missing required fragment: $fragment"
        }
    }
    if ($ffmpegBuild.Contains("--enable-gpl") -or $ffmpegBuild.Contains("--enable-nonfree")) {
        throw "FFmpeg build policy re-enabled GPL or nonfree mode."
    }

    $mpvBuild = Get-Content -LiteralPath "scripts/libmpv/clang64/build/mpv.sh" -Raw
    foreach ($fragment in @(
        "-Dgpl=false",
        "-Dcplayer=false",
        "-Dlibmpv=true",
        "-Dbuild-date=false",
        "-Dplain-gl=enabled"
    )) {
        if (-not $mpvBuild.Contains($fragment)) {
            throw "mpv build policy is missing required fragment: $fragment"
        }
    }

    $commonBuild = Get-Content -LiteralPath "scripts/libmpv/clang64/common.sh" -Raw
    if (-not $commonBuild.Contains("--default-library=shared")) {
        throw "The shared Meson build helper must keep third-party libraries dynamic."
    }
    foreach ($fragment in @(
        'git clone "$url" "$source_dir" >&2',
        'player_source_worktree_is_empty',
        'player_source_status_is_only_initial_deletions',
        'player_recover_incomplete_no_checkout_clone',
        'git -C "$source_dir" reset --hard HEAD',
        'printf ''Recovering incomplete no-checkout source clone: %s\n'' "$source_dir" >&2',
        'git -C "$source_dir" fetch --force --tags origin "$ref" >&2',
        'git -C "$source_dir" checkout --detach FETCH_HEAD >&2',
        'git -C "$source_dir" submodule sync --recursive >&2',
        'git -C "$source_dir" submodule update --init --recursive >&2',
        'Source checkout contains local changes and will not be overwritten'
    )) {
        if (-not $commonBuild.Contains($fragment)) {
            throw "common.sh is missing safe source-checkout/stdout-contract fragment: $fragment"
        }
    }
    if ($commonBuild.Contains('git clone --no-checkout')) {
        throw "common.sh must not create new source repositories with an intentionally empty no-checkout worktree."
    }

    Write-Host "libmpv MSYS2 CLANG64 source-build layout, pinned source commits, safe source checkout/stdout contract, MSYS-root multiline invocation, and LGPL-oriented build policy are complete."
}
finally {
    Pop-Location
}
