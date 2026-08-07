# Modular Qt 6 + libmpv Player

A Windows-first, cross-platform-ready desktop player project. The product name is intentionally temporary; rename it only when the final product identity is decided.

## Current baseline

This repository is now the active R2 modular playback-core scaffold. It currently provides:

- a root CMake entry limited to mandatory project/testing bootstrap and delegation to `cmake/CMakeLists.txt`;
- responsibility-separated CMake modules for dependency, compiler, analysis, target, source, and test configuration;
- a Qt 6.8.3 Qt Quick application bootstrap;
- a dedicated `RuntimePaths` bootstrap module for installed and portable path resolution;
- a dedicated foundation logging module with categories, file sink, rotation, redaction, and shutdown flush;
- an `ApplicationContainer` composition root that owns the current top-level runtime objects and defines their shutdown order;
- repository/build/dependency configuration that stores only repository-relative paths rather than machine-specific drive paths;
- OpenGL selection before `QGuiApplication` creation plus a real OpenGL context/renderer probe before QML loading;
- a QML bootstrap boundary that records concrete `QQmlEngine` warnings and returns diagnostic failure text to startup logging;
- a minimal QML shell split into `App.qml`, `MainWindow.qml`, `PlayerScreen.qml`, video placeholder, chrome placeholder, and theme ownership;
- a fixed-root `FindLibMpv.cmake` integration that creates the single `LibMpv::LibMpv` imported target only from `../libmpv/0.41.0/windows-x64`;
- an R2-01 mpv runtime module that validates client-API compatibility, the actual loaded DLL path, and the staged dependency-manifest identity before QML startup;
- a modular MSYS2 CLANG64 source-build chain that produces the project-controlled libmpv SDK/runtime package from pinned upstream source identities without placing build trees or third-party binaries in Git;
- automatic staging of the fixed libmpv package runtime DLLs plus its dependency manifest beside executable/test targets;
- architecture decisions, the original full development task book, and the approved R2-R14 fast-framework stage taskbooks under `docs/plans/stages/`;
- an explicit Windows/MSVC/Qt/libmpv/FFmpeg/transitive-source identity baseline plus minimum-compatible CMake/Ninja development-tool gates;
- stable configure, build, and test entry scripts that can initialize the Visual Studio x64 build environment from an ordinary Windows CMD/PowerShell session;
- five Qt Test targets after R2-01: RuntimePaths, logging, graphics backend, application container, and libmpv runtime probe;
- no third-party SDK, DLL, import library, generated runtime file, MSYS2 installation, or dependency build tree tracked inside the repository.

The scaffold intentionally does **not** yet implement `mpv_handle` ownership, mpv initialization profiles, playback commands, mpv event loops, property observation, PlaybackSession state, persistence, playlists, platform integrations, or packaging. Those responsibilities are introduced only in their Atomic Tasks.

## Scope

### Product target

The product is a Qt 6 desktop media player using libmpv as its playback core. Windows 10/11 x64 is the first release platform. The architecture keeps playback, application state, rendering, persistence, platform integration, and QML presentation separated so future features do not accumulate inside the player screen or playback backend.

### MVP delivery scope and acceptance matrix

| MVP capability | Observable acceptance condition |
|---|---|
| Open local files | A supported local video or audio file can be selected and loaded. Cancelling selection leaves the current session unchanged. Missing, unreadable, or unsupported files produce a clear error without crashing. |
| Drag-and-drop playback | Dropping one supported file opens it; dropping multiple supported files produces a deterministic playlist order. Directories, unsupported items, and invalid paths are rejected or reported without corrupting the current session. |
| Open network URLs | A valid supported media URL can be submitted and loaded. Empty, malformed, unsupported-scheme, unreachable, and failed URLs produce a clear result. Non-seekable live media does not expose a false seek capability. |
| Play, pause, and stop | Play and pause transition the active media between the corresponding states. Stop ends active playback and resets transient media state according to the playback state model. Repeated commands are safe and do not create duplicate sessions. |
| Absolute and relative Seek | Absolute Seek reaches a requested valid position and relative Seek moves forward or backward by the requested offset. Requests are bounded by media limits, rejected for non-seekable media, and do not block the UI thread. |
| Progress display and scrub preview | Current position and duration remain synchronized with playback. During dragging, the preview position remains under user control and is not pulled back by background updates. Committing a drag submits one final Seek; cancelling restores the actual position. |
| Volume and mute | Volume changes are reflected in playback and UI state within the supported range. Muting silences playback, and unmuting restores the prior non-muted volume instead of forcing an unrelated value. |
| Playback speed | A supported speed value changes playback rate and is reflected in the UI. Reset returns to 1.0×. Invalid values are rejected without leaving UI and backend state inconsistent. |
| Fullscreen | The player can enter and exit fullscreen through the supported UI and shortcut paths. Escape exits fullscreen, and returning to windowed mode restores valid window geometry without losing playback. |
| Playlist | Media items can be added, removed, reordered, selected, and identified as the current item. Empty, selected, hover, invalid-item, and current-playing states remain distinct. Removing or advancing from the current item never leaves a dangling current ID. |
| Audio-track selection | Available audio tracks are listed with stable identity and useful metadata. Selecting a track changes playback and selected state together; missing or failed selections produce a clear result and do not retain a false selection. |
| Subtitle-track selection | Available subtitle tracks are listed and can be selected or disabled. The selected/off state remains synchronized with playback, and media changes fully replace the prior track list. |
| Load external subtitles | A supported external subtitle file can be added to the active media and selected. Invalid encoding, unsupported files, duplicate loading, and backend failure are reported without damaging the existing track state. |
| Subtitle and audio delay | Positive and negative subtitle/audio delay values can be applied and reset. Playback effect, displayed value, and feedback remain consistent after repeated adjustments and media changes. |
| Chapter selection | Chapter metadata is listed when present. Selecting a chapter performs a Seek to its position. Media without chapters presents an explicit empty state, and stale chapters are removed after media changes. |
| Current media information | The active media exposes current title or source identity, duration when known, and available media/track metadata. Information refreshes on media changes and is cleared or replaced after stop, failure, or unload. |
| Playback error feedback | Damaged media, unsupported codecs or formats, unreadable files, backend failures, and unreachable URLs produce a user-visible, actionable error while diagnostic detail remains available for logging. |
| Loading, buffering, paused, and ended states | Loading, buffering, paused, playing-ended, and error presentation follows explicit priority and mutual-exclusion rules. Each state appears and clears on the corresponding playback transition without stale overlays from the previous media. |
| Recent media | Successfully opened media appears in a recent list in deterministic recency order. Missing local paths remain identifiable rather than being silently deleted, and selecting a valid recent item re-enters the normal media-open workflow. |
| Remember playback position | Eligible media positions are persisted and can be restored when the same media is reopened according to the resume policy and user setting. Completed or near-complete media is not incorrectly resumed as unfinished, and persistence failure does not crash playback. |
| Keyboard shortcuts | Documented default shortcuts invoke the same registered actions as the UI. Context rules prevent conflicts with text entry and modal interaction. Unsupported or conflicting combinations are rejected or clearly reported. |
| System media keys | Supported system play/pause and related media keys invoke the registered player actions, including when the window is not foreground where the platform permits it. Handlers are released during shutdown and do not fire after exit. |
| Prevent system sleep during playback | Sleep inhibition is active only while the playback policy requires it, including active video playback. It is released on pause, stop, media end, failure, or application exit and remains safe under repeated state changes. |
| Single instance and file associations | Launching a second instance forwards its file or URL arguments to the primary instance and then exits. Registered media files open through the same media-open workflow. Install, upgrade, and uninstall leave association state consistent with packaging policy. |
| Windows installer | On a clean supported Windows environment, the package installs, starts, opens and plays a supported file, and uninstalls without relying on the developer PATH, Qt installation, or a system mpv installation. Uninstall does not remove user media or other files not owned by the application. |

### Cross-cutting MVP release gates

- 4K local media can play, pause, Seek, and switch fullscreen without blocking the UI thread;
- at least 100 consecutive media open/close cycles complete without a crash or retained playback thread;
- damaged files, unsupported codecs, unreadable files, and unreachable URLs produce explicit errors;
- scrub interaction is not overridden by background position updates;
- events from an older media generation cannot mutate the current media state;
- render resources are released before the mpv handle during shutdown;
- stop, end, failure, and media replacement correctly clear or replace tracks, chapters, duration, and other media-scoped state;
- configuration or history persistence failure cannot crash core playback;
- QML contains no direct `mpv_command`, `mpv_set_property`, or C-pointer access;
- every core module has an independently executable unit or integration verification path;
- the installer passes install, launch, playback, and uninstall checks on a clean Windows environment.

### Deferred second-stage enhancements

- screenshots;
- A-B loop;
- aspect ratio, crop, and rotation controls;
- subtitle styling;
- playback-quality presets;
- shader management;
- mini player;
- picture-in-picture;
- advanced playback statistics;
- macOS adaptation;
- Linux adaptation.

### Explicitly outside the first release

- online-site parsing or media extraction;
- media-server functionality;
- DLNA, AirPlay, or Chromecast;
- cloud synchronization;
- accounts;
- online subtitle search;
- a plugin marketplace;
- a script store;
- video editing or transcoding;
- media asset-management system;
- AI subtitle or AI image-quality enhancement.

Any change to these frozen boundaries requires an explicit user requirement, an updated README scope and acceptance definition, and a separately reviewable Atomic Task before implementation begins.

## Atomic Task status

### R0-01 — Complete

- repository governance files, root README, plan documents, top-level CMake, and generated-output exclusions are present.

### R0-02 — Complete

- README freezes the first-release scope and observable acceptance matrix;
- deferred and explicitly excluded capabilities are separated from MVP.

### R0-03 — Complete

- `LICENSES/README.md` records the LGPL-compatible dynamic-linking route for Qt, libmpv, and FFmpeg;
- static linking, GPL/nonfree FFmpeg configuration, unknown-license components, missing corresponding source, and mismatched notices remain release blockers.

### R0-04 — Complete

- the original Windows/MSVC/Qt/CMake/Ninja/mpv/FFmpeg identity baseline is recorded in `cmake/DependencyVersions.cmake`;
- later R1 direction relaxes exact patch-level requirements only for development tools while keeping identity-sensitive runtime/product dependencies controlled;
- shared dependencies remain repository-parent-relative.

### R0-05 — Complete

- the project uses the in-process libmpv Render API;
- Qt Quick is fixed to OpenGL for the first-release renderer;
- `QQuickFramebufferObject` is the planned video composition path;
- render context must be released before the mpv handle;
- no `wid` embedding, external `mpv.exe`, or silent graphics fallback is part of the formal architecture.

### R0-06 — Skipped by explicit user direction

R0-06 is not accepted or complete. Later playback tests must not claim complete legal fixture coverage until the fixture policy is deliberately resumed or replaced by an approved equivalent.

### R1-01 — Complete

- modular CMake boundaries are active;
- compatible CMake/Ninja, Qt 6.8.3, Visual Studio 2022 17.14 family, MSVC 19.44/14.44, x64, and Windows SDK 10.0.26100.0 verification is green on the user's Windows workspace;
- relocation-safe configure and the full Ninja build succeeds;
- the application output is `build/<preset>/Player.exe`.

### R1-02 — Complete

- `RuntimePaths` owns installed/portable runtime directory resolution;
- portable mode is selected by `portable.flag` beside the executable;
- path resolution is side-effect free;
- the `runtime_paths` Qt Test passes on the user's Windows/Qt environment.

Runtime paths returned by Windows/Qt may naturally be absolute operating-system values. Those are runtime values, not committed machine-specific dependency configuration.

### R1-03 — Complete

- `src/foundation/logging/` owns logging categories, rotating file sink, redaction, and flush behavior;
- `LoggingBootstrap` connects RuntimePaths to the file sink without moving sink internals into startup code;
- logging failure does not prevent core application startup;
- the `logging` Qt Test passes on the user's Windows/Qt environment.

### R1-04 — Complete

- `src/app/bootstrap/graphics_backend/` separates pre-application OpenGL selection from runtime graphics probing;
- the application validates a real OpenGL context before QML loads and records vendor/renderer/version diagnostics;
- failure is explicit and does not silently fall back to another graphics architecture;
- the `graphics_backend` Qt Test passes on the user's Windows/Qt environment;
- the actual executable path is `build/windows-msvc-debug/Player.exe`.

### R1-05 — Complete

- `src/app/composition/application_container.*` is the current composition root;
- the container owns `RuntimePaths`, `LoggingBootstrap`, and `QmlBootstrap`;
- shutdown destroys the QML engine/object tree before stopping logging;
- `shutdown()` is idempotent and destructor re-entry is safe;
- startup failures use the same RAII cleanup path;
- no empty playback/persistence/platform composition shells were created prematurely;
- the fourth `application_container` Qt Test was added;
- the user explicitly accepted R1-05 after local verification on 2026-08-07.

### R1-06 — Complete

- the existing modular `App.qml -> MainWindow.qml -> PlayerScreen.qml` shell is the accepted R1 minimum UI shell;
- `VideoSurface`, `PlayerChrome`, and `Theme` remain presentation placeholders only and contain no playback backend logic;
- `QmlBootstrap` captures concrete `QQmlEngine::warnings` and returns fatal root-load diagnostics through `lastError()`;
- `ApplicationBootstrap` records the detailed QML failure instead of only a generic startup failure;
- the layout verifier enforces the QML module/shell boundaries and rejects direct libmpv calls from `PlayerScreen.qml`;
- the user explicitly accepted R1-06 after local configure/build/test/window verification on 2026-08-07.

### R2-01 — Implemented; source-built libmpv package acceptance pending

Implemented on 2026-08-07 in `agent/r2-stage`:

- `cmake/FindLibMpv.cmake` is the only CMake discovery boundary for the fixed libmpv sibling package;
- discovery is constrained to `../libmpv/0.41.0/windows-x64` and does not search a system mpv installation or arbitrary PATH location;
- `FindLibMpv.cmake` requires `include/mpv/client.h`, one MSVC-compatible import `.lib`, one primary `libmpv-2.dll` or `mpv-2.dll`, and `dependency-manifest.json`;
- the manifest identity is checked during configure against mpv `0.41.0`/`v0.41.0`/commit `41f6a645068483470267271e1d09966ca3b9f413`, FFmpeg `8.0.3`, libplacebo `7.351.0`, libass `0.17.4`, FreeType `2.13.3`, FriBidi `1.0.16`, and HarfBuzz `10.2.0`;
- a single global imported target `LibMpv::LibMpv` owns include, import-library, runtime-DLL, runtime-package, and manifest metadata;
- `player_stage_libmpv_runtime()` copies DLLs from the fixed package plus the same manifest into target output trees; no third-party binary is added to Git;
- `src/playback/infrastructure/mpv/runtime/` separates manifest parsing from actual loaded-DLL/client-API runtime probing;
- `ApplicationBootstrap` performs the libmpv runtime gate after logging starts and before graphics/QML startup;
- `verify-dependencies.ps1` treats libmpv as required for R2 and checks the complete pinned dependency identity;
- the fifth `mpv_runtime_probe` Qt Test stages and validates the same fixed runtime package;
- after the user's first R2-01 run correctly stopped on the missing `../libmpv/.../include/mpv/client.h`, a project-controlled source-build chain was added rather than weakening the dependency gate or accepting an arbitrary prebuilt DLL;
- `scripts/libmpv/bootstrap-build-environment.ps1` installs/verifies build-only MSYS2 CLANG64 tools but does not install runtime libraries from MSYS2 packages;
- `scripts/libmpv/build-package.ps1` is the Windows-side build entry and keeps source/build/package paths separated;
- Git source checkouts live under `../downloads/libmpv/sources`, immutable release archives live under `../downloads/libmpv/archives`, generated build/extracted-source/trust state lives under `../cache/libmpv-build`, and only the final SDK/runtime package lives under `../libmpv/0.41.0/windows-x64`;
- separate CLANG64 build modules build FreeType, FriBidi, HarfBuzz, libass, libplacebo, FFmpeg, and mpv in dependency order;
- FFmpeg is built from the official `ffmpeg-8.0.3.tar.xz` release archive after isolated PGP verification against fingerprint `FCF986EA15E6E293A5644F10B4322F04D67658D8` plus a small remote-tag-to-commit check; its compile policy remains `--disable-autodetect --disable-gpl --disable-nonfree --enable-shared --disable-static`;
- mpv is built with `-Dgpl=false -Dcplayer=false -Dlibmpv=true -Dbuild-date=false` and the shared-library path;
- the finalizer recursively stages required CLANG64 runtime DLLs actually referenced by produced DLLs, generates an export `.def`, uses the pinned Visual Studio x64 `lib.exe` to create the MSVC-compatible `mpv.lib`, and writes a UTF-8-no-BOM dependency manifest containing source commits and SHA-256 hashes;
- `scripts/libmpv/verify-package.ps1` independently rechecks package identity and every recorded artifact hash;
- the source-build layout/policy has its own structural verifier `scripts/libmpv/verify-build-layout.ps1`;
- `Msys2Environment.psm1` executes arbitrary CLANG64 shell text through a temporary UTF-8-no-BOM LF `.sh` file written directly under the resolved MSYS2 `tmp` directory and invokes it through the stable `/tmp/...` path, avoiding both multiline `bash -lc` quoting and an extra `cygpath` conversion step;
- ordinary Git source acquisition is isolated in `scripts/libmpv/clang64/source/source_checkout.sh`: fixed refs are installed through bounded three-attempt shallow fetches (`--depth=1 --no-tags`) into owned staging directories, Git network operations use command-local HTTP/1.1 without changing user configuration, existing local changes/non-Git content remain protected, the earlier empty-worktree recovery remains narrow, networked submodule operations retry explicitly with shallow single-job fetches, and `player_fetch_source` stdout remains reserved for the final source path;
- large signed release acquisition is isolated in `scripts/libmpv/clang64/source/source_archive.sh`: FFmpeg downloads use resumable curl transfer, the official release archive/signature/key are kept outside Git, the public key is imported only into an isolated build-cache keyring and must match the pinned fingerprint, extraction occurs only after signature/tag identity validation, and partial downloads remain resumable rather than restarting a Git pack;
- no `mpv_handle`, initialization profile, event loop, command encoder, property observer, render context, PlaybackSession, or QML playback behavior was introduced in R2-01.

The approved source-build set is intentionally narrow. MSYS2/CLANG64 is a **build-only** toolchain; it is not a Player production dependency. The source build does not consume MSYS2-packaged FFmpeg/libass/libplacebo/etc. The release-time dependency/license scan in R13 remains mandatory, including any compiler runtime DLL or bundled source component actually present in the final binary graph.

Still required on the user's Windows workspace:

- the original MSYS2 CLANG64 bootstrap is locally verified; rerun `bootstrap-build-environment.ps1` once after this change so curl/GnuPG/tar/xz archive-verification tools are explicitly present;
- the full project-controlled source build must rerun through the signed FFmpeg release-archive path, complete, and create the fixed sibling package;
- `verify-package.ps1` and `verify-dependencies.ps1` must validate the real package identity and artifact hashes;
- fresh configure/build must link against `LibMpv::LibMpv` and stage its runtime files;
- CTest must pass all five tests including `mpv_runtime_probe`;
- `build/windows-msvc-debug/Player.exe` must start using the staged DLL and log `libmpv runtime validated` with the actual DLL path and manifest identity.

The exact artifact hashes, transitive DLL set, source commits for dependencies other than the already pinned mpv commit, and final package size are not claimed until the user's real source build completes. R13 still performs the final clean-machine binary dependency/license scan for the distributable package.

## Parent-workspace and relative-path policy

The repository may be renamed or moved, but its parent directory is the dependency workspace:

```text
<parent>/
├─ cache/
│  ├─ cmake/fetchcontent/
│  └─ libmpv-build/
├─ downloads/
│  └─ libmpv/
│     ├─ sources/
│     └─ archives/
├─ libmpv/
├─ Qt/
├─ <repository>/
└─ <other repository, optional>/
```

Rules:

- committed build/dependency configuration contains no machine-specific drive path;
- Qt, libmpv, downloads, and caches are addressed through `../Qt/...`, `../libmpv/...`, `../downloads/...`, and `../cache/...`;
- there are no required `../cmake`, `../ninja`, or `../msys2` sibling directories;
- CMake `3.30.5+` and Ninja `1.12.1+` remain compatible Player-development-tool floors;
- scripts prefer tools on `PATH`, then the Qt Installer locations under `../Qt/Tools` for the MSVC Player build;
- CTest is resolved independently and invoked through `ctest --preset`;
- Windows path validation treats `..\Qt\...` and `../Qt/...` as the same repository-relative form;
- Visual Studio x64 environment initialization is automatic through `vswhere.exe` and `vcvars64.bat`;
- Qt DLL/plugin paths for tests are resolved from the relative Qt root and exist only in the child process environment;
- `configure.ps1` uses CMake `--fresh` because generated CMake cache data is disposable and location-specific;
- starting with R2, libmpv is a required sibling dependency rather than an optional future dependency;
- MSYS2 is an external build tool installation discovered at runtime through `MSYS2_ROOT`, PATH, or the normal system-drive installation; it is not stored in the repository-parent dependency workspace.

## Architecture boundary

```text
QML presentation
      ↓ user intent / projected state
Application layer (PlaybackSession arrives in R3)
      ↓ commands / events
libmpv infrastructure
      ↓ public C API
libmpv
```

Current ownership and build responsibilities:

```text
CMakeLists.txt
  Project declaration, test gate, top-level CTest enablement,
  and delegation to cmake/ only.

cmake/CMakeLists.txt
  Global build orchestration and relative entry into ../src and ../tests.

cmake/FindLibMpv.cmake
  R2-01 fixed sibling-package discovery, full manifest identity checks,
  imported target metadata, and target-local runtime staging.

scripts/libmpv/
  Project-controlled third-party source build only. PowerShell owns Windows
  orchestration/import-library/manifest work; clang64/ owns source compilation.

src/app/bootstrap/
  Startup ordering, application metadata, libmpv/graphics probes,
  RuntimePaths creation, logging/QML startup calls, and startup failures.

src/app/bootstrap/qml_bootstrap.*
  Owns the QQmlApplicationEngine load boundary and QML-load diagnostics.

src/app/composition/application_container.*
  Sole current composition root. Owns top-level runtime objects and
  their destruction order; contains no playback/business algorithm.

src/foundation/logging/
  Logging categories, file sink, rotation, redaction, and flush internals.

src/playback/infrastructure/mpv/runtime/
  Owns dependency-manifest parsing and validation of the actually loaded
  libmpv runtime/client-API identity. It does not own mpv_handle or playback state.

src/presentation/qml/
  Presentation shell only; no direct libmpv API access.

tests/CMakeLists.txt
  Registers RuntimePaths, logging, graphics-backend, ApplicationContainer,
  and R2-01 libmpv-runtime regression targets.
```

Future `playback_composition`, `persistence_composition`, and `platform_composition` belong under `src/app/composition/` only when the corresponding real subsystems exist. Formal UI token/control/surface expansion remains assigned to R5/R6.

## Toolchain and dependency compatibility matrix

`cmake/DependencyVersions.cmake` is authoritative for the current development compatibility floor and identity-sensitive dependencies.

| Component | Current baseline |
|---|---|
| Supported Windows minimum | Windows 10 22H2, build `10.0.19045` |
| Primary Windows validation family | Windows 11 24H2, build `10.0.26100` |
| Windows SDK | minimum `10.0.26100.0`; reference servicing release `10.0.26100.8876` |
| Visual Studio | Visual Studio 2022 `17.14` family |
| MSVC | v143 toolset family `14.44`, compiler family `19.44`, x64 target |
| Qt | exact Qt `6.8.3`, MSVC 2022 64-bit kit |
| CMake | minimum `3.30.5` |
| Ninja | minimum `1.12.1` |
| mpv/libmpv | exact `0.41.0`, tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413` |
| FFmpeg | exact `8.0.3`, ref `n8.0.3`, commit `8ae0b34901ba60a802f183ee75a250a9fc3e09a5`, official signed xz release archive, signing fingerprint `FCF986EA15E6E293A5644F10B4322F04D67658D8`, LGPL-oriented build with GPL/nonfree/autodetect disabled |
| libplacebo | exact `7.351.0`, ref `v7.351.0`, LGPL-2.1-or-later route |
| libass | exact `0.17.4`, ref `0.17.4`, ISC |
| FreeType | exact `2.13.3`, ref `VER-2-13-3`, FreeType License route |
| FriBidi | exact `1.0.16`, ref `v1.0.16`, LGPL-2.1-or-later |
| HarfBuzz | exact `10.2.0`, ref `10.2.0`, permissive HarfBuzz license route |
| libmpv source-build toolchain | MSYS2 CLANG64; build-only, not staged as a directory or runtime dependency itself |
| C++ | C++20 |

## Relative libmpv build and package layout

```text
<parent>/
├─ downloads/
│  └─ libmpv/
│     ├─ sources/
│     │  ├─ freetype/
│     │  ├─ fribidi/
│     │  ├─ harfbuzz/
│     │  ├─ libass/
│     │  ├─ libplacebo/
│     │  └─ mpv/
│     └─ archives/
│        ├─ ffmpeg-8.0.3.tar.xz
│        ├─ ffmpeg-8.0.3.tar.xz.asc
│        └─ ffmpeg-release-signing-key.asc
├─ cache/
│  └─ libmpv-build/
│     ├─ build/
│     ├─ prefix/
│     ├─ archive-sources/
│     ├─ source-trust/
│     └─ metadata/
└─ libmpv/
   └─ 0.41.0/windows-x64/
      ├─ include/mpv/
      ├─ lib/
      │  ├─ mpv.def
      │  └─ mpv.lib
      ├─ bin/
      │  ├─ libmpv-2.dll or mpv-2.dll
      │  └─ actual dynamic runtime dependencies
      ├─ licenses/
      └─ dependency-manifest.json
```

`source_checkout.sh` owns Git-backed fixed-ref source acquisition: staging repositories use shallow HTTP/1.1 fetches, exact commits are verified, failed staging is removed, and existing local changes are protected. `source_archive.sh` owns immutable signed release archives: the FFmpeg tarball/signature/key download is resumable under `../downloads/libmpv/archives`, the release key is checked in an isolated cache keyring against the pinned fingerprint, the signed archive is verified before extraction, and only a tiny peeled-tag lookup is used to confirm `n8.0.3` still maps to the pinned release commit. Verified archive extraction is disposable under `../cache/libmpv-build/archive-sources`; it does not overwrite user-maintained Git source checkouts or change global Git/GPG configuration.

The manifest generated after the real build includes at least:

```json
{
  "mpv": {
    "version": "0.41.0",
    "tag": "v0.41.0",
    "commit": "41f6a645068483470267271e1d09966ca3b9f413",
    "sourceCommit": "..."
  },
  "ffmpeg": { "version": "8.0.3", "ref": "n8.0.3", "sourceCommit": "..." },
  "libplacebo": { "version": "7.351.0", "ref": "v7.351.0", "sourceCommit": "..." },
  "libass": { "version": "0.17.4", "ref": "0.17.4", "sourceCommit": "..." },
  "freetype": { "version": "2.13.3", "ref": "VER-2-13-3", "sourceCommit": "..." },
  "fribidi": { "version": "1.0.16", "ref": "v1.0.16", "sourceCommit": "..." },
  "harfbuzz": { "version": "10.2.0", "ref": "10.2.0", "sourceCommit": "..." },
  "artifacts": [
    { "path": "bin/...dll", "sha256": "...", "bytes": 0 },
    { "path": "lib/mpv.lib", "sha256": "...", "bytes": 0 }
  ]
}
```

The real values replace the ellipses only after a successful local source build. The project does not invent binary hashes or unresolved source commits in source control.

## Build the libmpv dependency package

MSYS2 must first be installed from its official distribution. It remains outside the repository and parent dependency workspace. Then run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\libmpv\verify-build-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\bootstrap-build-environment.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\build-package.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\verify-package.ps1
```

`bootstrap-build-environment.ps1` installs only build tools into MSYS2/CLANG64: compiler/toolchain, Meson, Ninja, pkg-config, NASM, Python, Git, curl, GnuPG, tar/xz and base build utilities. It does **not** install FFmpeg/libplacebo/libass/FreeType/FriBidi/HarfBuzz/mpv binary packages from MSYS2.

If MSYS2 is not at the normal system-drive `msys64` location, set `MSYS2_ROOT` to the installation directory before running the scripts. If MSYS2 itself reports that a full system update/restart is required, complete the normal MSYS2 update first; the project bootstrap intentionally does not perform an unsafe partial package-database upgrade.

## Configure, build, and test Player

After the libmpv package verifies, run from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify-project-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\verify-dependencies.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

The Windows debug executable is:

```text
build/windows-msvc-debug/Player.exe
```

The generic preset output contract is:

```text
build/<preset>/Player.exe
```

After R2-01, a successful application startup must log a line beginning with:

```text
libmpv runtime validated:
```

and include the actual loaded DLL path, manifest path, mpv/tag/commit/FFmpeg identity, client API version, and staged runtime location.

## Module growth rule

A file may receive new code only when the code has the same responsibility and reason to change. When a real second responsibility appears, upgrade the module into a responsibility directory instead of accumulating unrelated logic. Do not create source-history copies with suffixes such as `Old`, `New`, `V2`, `Final`, or `Copy`; Git owns history.

## Documentation

- Original full execution plan: `docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md`
- Approved R2-R14 fast-framework stage plans: `docs/plans/stages/00_INDEX.md`
- Third-party license inventory and release gate: `LICENSES/README.md`
- Build orchestrator: `cmake/CMakeLists.txt`
- Toolchain/dependency compatibility manifest: `cmake/DependencyVersions.cmake`
- Relative shared dependency paths: `cmake/DependencyPaths.cmake`
- R2 libmpv imported-target integration: `cmake/FindLibMpv.cmake`
- R2 source-build entry: `scripts/libmpv/build-package.ps1`
- R2 Git source checkout/network boundary: `scripts/libmpv/clang64/source/source_checkout.sh`
- R2 signed release archive boundary: `scripts/libmpv/clang64/source/source_archive.sh`
- R2 source-build structural gate: `scripts/libmpv/verify-build-layout.ps1`
- R2 package hash/identity gate: `scripts/libmpv/verify-package.ps1`
- Render embedding and lifecycle decision: `docs/decisions/ADR-0001-libmpv-render-api.md`
- OpenGL and Qt Quick FBO decision: `docs/decisions/ADR-0002-opengl-first.md`
- Playback-state ownership: `docs/decisions/ADR-0003-single-playback-owner.md`
- QML boundaries: `docs/decisions/ADR-0004-qml-boundaries.md`
- Media fixture policy: `docs/testing/media-fixture-policy.md` (existing baseline only; R0-06 was skipped and not accepted)

## Validation record

Confirmed by the user on the Windows 10 development workspace through R1-06:

- project layout/dependency/configure/build/test workflow is operational from a normal CMD/PowerShell session;
- CMake `3.30.5`, Ninja `1.12.1`, Qt `6.8.3`, Visual Studio 2022 17.14 family, MSVC `19.44`/toolset `14.44`, x64, and Windows SDK `10.0.26100.0` are accepted by the current compatibility gates;
- relocation-safe configure succeeds;
- all four R1 CTest targets pass;
- the application output path is `build/windows-msvc-debug/Player.exe`;
- the minimal QML shell displays and closes normally;
- the user explicitly marked R1-06 accepted on 2026-08-07.

The user's first R2-01 local verification on 2026-08-07 confirmed that the new R2 hard gate correctly stops all configure/build/test entry points when `../libmpv/0.41.0/windows-x64/include/mpv/client.h` is absent. This is expected behavior and is not treated as a source failure. The source-build chain was then added to create that package from the approved source identities instead of weakening the gate.

The MSYS2 bootstrap path is locally verified through the original CLANG64 tool set: package installation completed and the post-install check returned `MSYS2 CLANG64 build environment is ready.` The current signed-archive follow-up adds curl/GnuPG/tar/xz as explicit build-only requirements and still needs one local bootstrap rerun to confirm their presence through the same entry point.

The first real `build-package.ps1` run then reached FreeType source acquisition and cloned the repository successfully, but the source-safety check immediately rejected that fresh checkout as locally modified. Review confirmed the cause was `git clone --no-checkout` followed by `git status --porcelain`: the intentionally empty worktree appears as tracked deletions. The source checkout recovery now only repairs that provably empty/all-tracked-deletions state; actual local changes remain protected.

The next local rerun recovered the FreeType checkout, fetched `VER-2-13-3`, detached at the pinned `42608f77...` commit, and reached Meson. Meson then received an invalid source path beginning with `Recovering incomplete no-checkout source clone:` because `player_fetch_source()` is consumed through command substitution and that recovery diagnostic was still written to stdout. The source checkout module now reserves stdout exclusively for the final source-path return value and routes recovery plus Git operational output to stderr.

The following local run confirmed that correction by completing the FreeType configure, compile, and install stages. The build then moved to FriBidi, where the first full GitHub clone timed out after 300000 ms. Because the old source function relied on shell `errexit` inside command substitution, it continued after the failed clone and emitted several misleading missing-directory errors plus an empty source-identity mismatch. Source acquisition was isolated in `source/source_checkout.sh`; network operations gained bounded three-attempt retry, staging/cleanup, and explicit failure propagation.

The next local run progressed beyond FriBidi and reached HarfBuzz. HarfBuzz full-history clone failed on all three attempts with OpenSSL unexpected EOF, HTTP/2 stream reset, and connection-abort errors while transferring a repository with more than 130,000 objects. The retry/cleanup boundary worked correctly and installed no failed HarfBuzz checkout. The source-network implementation then switched Git-backed dependencies to shallow fixed-ref HTTP/1.1 fetches.

The latest local run confirmed that the shallow-fetch change progressed through HarfBuzz and subsequent dependencies to FFmpeg. FFmpeg still required 9,850 objects even at depth 1, and all three Git pack attempts failed mid-transfer with `curl 18`, `curl 56`, EOF, and invalid `index-pack` output. This confirms the remaining failure is transport instability on a relatively large Git pack rather than an FFmpeg version/configuration/compiler failure. FFmpeg acquisition now uses its official approximately 11 MB `ffmpeg-8.0.3.tar.xz` release archive, resumable curl transfer, official `.asc` signature, an isolated GPG keyring pinned to release fingerprint `FCF986EA15E6E293A5644F10B4322F04D67658D8`, and only a small peeled-tag lookup to retain the pinned `n8.0.3 -> 8ae0b349...` commit identity check.

Connected-environment verification for this follow-up covers structural review of the new archive trust boundary and the existing source/package contracts. This environment cannot execute the user's Windows MSYS2 network/download path, so the real signed archive download/resume, GPG verification, FFmpeg compile, remaining mpv build, produced DLL graph, import-library generation, package hashes, Player linking, five CTests, and runtime loader probe remain local acceptance requirements.

## Change Log

### 2026-08-08

- Replaced FFmpeg's remaining Git-pack source transfer with the official signed `ffmpeg-8.0.3.tar.xz` release path after three depth-1 fetches still failed while transferring 9,850 objects; downloads are resumable, release signature verification uses an isolated GPG keyring pinned to the official release fingerprint, the remote release tag is still checked against the pinned commit, and no global Git/GPG configuration is modified.
- Added curl/GnuPG/tar/xz as build-only MSYS2 requirements for signed release archive verification; they are not Player runtime dependencies.
- Replaced R2-01 dependency full-history clones with pinned-ref shallow fetches after HarfBuzz repeatedly failed during a 130k-object transfer; Git network operations now use command-local HTTP/1.1, bounded retry, shallow single-job submodule updates, exact commit verification, and staging cleanup without changing the user's global Git configuration.

### 2026-08-07

- Hardened the R2-01 source-network boundary after the real FriBidi GitHub clone timed out: source acquisition is now a dedicated module with bounded clone/fetch/submodule retry, explicit error propagation from command substitution, temporary clone staging/cleanup, protection for existing source content, and no cascade into false missing-directory or identity-mismatch errors.
- Fixed the R2-01 source-path return contract after the recovered FreeType checkout reached Meson but a recovery diagnostic polluted command-substitution stdout; `player_fetch_source` now returns only the source path on stdout and sends all recovery/Git operational output to stderr, including submodule operations used by later dependencies.
- Fixed R2-01 source acquisition after the first real FreeType build exposed a false dirty-worktree result from `git clone --no-checkout`; new source repositories now use materialized clones, and only the provably empty/all-tracked-deletions state left by the old implementation is auto-recovered while real local changes remain protected.
- Removed the R2-01 bootstrap's `cygpath.exe` dependency after the real Windows follow-up returned exit code `-1` during temporary-script conversion; CLANG64 command scripts are now written directly to the resolved MSYS2 `tmp` directory and executed through `/tmp/...`, while retaining UTF-8-no-BOM/LF normalization and guaranteed cleanup.
- Added the R2-01 project-controlled MSYS2 CLANG64 libmpv source-build pipeline after the real Windows verification correctly exposed the missing sibling SDK/runtime package.
- Pinned and wired the approved source-build dependency set: FFmpeg `8.0.3`, libplacebo `7.351.0`, libass `0.17.4`, FreeType `2.13.3`, FriBidi `1.0.16`, and HarfBuzz `10.2.0`, while retaining mpv `0.41.0` at the already pinned commit.
- Added responsibility-separated PowerShell modules for MSYS2 discovery/invocation, parent-relative build paths, MSVC import-library generation, manifest creation, and artifact-hash verification.
- Added separate CLANG64 build steps for every approved source dependency, a clean build orchestrator, recursive runtime-DLL staging, license-file staging, and export-definition generation.
- Added independent build-layout and final-package verification entries. No MSYS2 runtime library package or arbitrary prebuilt mpv package is accepted by this workflow.
- Started Stage R2 on `agent/r2-stage` from the accepted R1-06 head and implemented the initial fixed-root `LibMpv::LibMpv`/runtime-probe integration plus the fifth CTest.
- Marked R1-06 accepted after explicit user confirmation of the local verification result.
- Implemented and accepted R1-05 Composition Root and retained all completed R1 architecture/tooling fixes documented above.
- Recorded R0-06 as explicitly skipped and unaccepted; R0-01 through R0-05 remain completed according to their recorded scope.

### R2-01 local verification after sync

First create and verify the sibling libmpv package:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\libmpv\verify-build-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\bootstrap-build-environment.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\build-package.ps1
powershell -ExecutionPolicy Bypass -File scripts\libmpv\verify-package.ps1
```

Then validate Player:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify-project-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\verify-dependencies.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

Expected CTest registration after R2-01: five tests (`runtime_paths`, `logging`, `graphics_backend`, `application_container`, `mpv_runtime_probe`). Then launch:

```text
build\windows-msvc-debug\Player.exe
```

R2-01 acceptance requires the source-built fixed sibling package to pass identity/hash checks, the build to link and stage its actual runtime dependency graph, all five tests to pass, and the application to log `libmpv runtime validated` while loading the DLL from the staged executable directory. Any source-build/package failure must remain explicit; do not replace it with a random system or third-party mpv binary.
