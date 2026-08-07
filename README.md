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
- automatic staging of the fixed libmpv package runtime DLLs plus its dependency manifest beside executable/test targets without committing third-party binaries into Git;
- architecture decisions, the original full development task book, and the approved R2-R14 fast-framework stage taskbooks under `docs/plans/stages/`;
- an explicit Windows/MSVC/Qt/libmpv/FFmpeg identity baseline plus minimum-compatible CMake/Ninja development-tool gates;
- stable configure, build, and test entry scripts that can initialize the Visual Studio x64 build environment from an ordinary Windows CMD/PowerShell session;
- five Qt Test targets after R2-01: RuntimePaths, logging, graphics backend, application container, and libmpv runtime probe;
- no third-party SDK, DLL, import library, or generated runtime file tracked inside the repository.

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
- relocation-safe configure and the full Ninja build succeed;
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

### R2-01 — Implemented; Windows libmpv package acceptance pending

Implemented on 2026-08-07 in the new `agent/r2-stage` branch:

- added `cmake/FindLibMpv.cmake` as the only CMake discovery boundary for the R0-04 fixed libmpv sibling package;
- discovery is constrained to `../libmpv/0.41.0/windows-x64` and does not search a system mpv installation or arbitrary PATH location;
- `FindLibMpv.cmake` requires `include/mpv/client.h`, one MSVC-compatible import `.lib`, one primary `libmpv-2.dll` or `mpv-2.dll`, and `dependency-manifest.json`;
- the manifest identity is checked against mpv `0.41.0`, tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413`, and FFmpeg `8.0.3` during configure;
- a single global imported target `LibMpv::LibMpv` owns include, import-library, runtime-DLL, runtime-package, and manifest metadata;
- `player_stage_libmpv_runtime()` copies only DLLs from the fixed package root/bin plus the same manifest into a target's output tree; no third-party binary is added to Git;
- added `src/playback/infrastructure/mpv/runtime/` with separate manifest parsing and runtime-probe responsibilities;
- `MpvRuntimeProbe` calls `mpv_client_api_version()`, enforces compatible client API major/minor semantics, resolves the actually loaded Windows module path, requires the DLL to come from the staged executable directory, and revalidates the staged manifest identity;
- `ApplicationBootstrap` now performs the libmpv runtime gate after logging starts and before graphics/QML startup, recording mpv/tag/commit/FFmpeg/client-API/DLL/manifest diagnostics;
- `verify-dependencies.ps1` now treats libmpv as required for R2, validates the fixed package through the shared PowerShell path module, and prints SHA-256 values for the selected runtime DLL and MSVC import library;
- added the fifth `mpv_runtime_probe` Qt Test and stages the same audited runtime package beside that test executable;
- no `mpv_handle`, initialization profile, event loop, command encoder, property observer, render context, PlaybackSession, or QML playback behavior was introduced in R2-01.

Connected-environment verification completed:

- R2-01 taskbook/rule/dependency-boundary review;
- CMake/source/module ownership review;
- Windows path-policy review confirming no machine drive path was introduced;
- final branch diff review remains required before the formal R2-01 commit is closed.

Still required on the user's Windows workspace:

- the fixed sibling libmpv package must physically contain the required header/import-library/runtime DLL/manifest artifacts;
- `verify-dependencies.ps1` must validate the real package identity and print the real artifact SHA-256 values;
- fresh configure/build must link against `LibMpv::LibMpv` and stage its runtime files;
- CTest must pass all five tests including `mpv_runtime_probe`;
- `build/windows-msvc-debug/Player.exe` must start using the staged DLL and log `libmpv runtime validated` with the actual DLL path and manifest identity.

The exact artifact hashes and complete release-time dependency graph are not claimed until the user's real project-controlled libmpv build is present and inspected. R13 still performs the final binary dependency/license scan for the distributable package.

## Parent-workspace and relative-path policy

The repository may be renamed or moved, but its parent directory is the dependency workspace:

```text
<parent>/
├─ cache/
├─ downloads/
├─ libmpv/
├─ Qt/
├─ <repository>/
└─ <other repository, optional>/
```

Rules:

- committed build/dependency configuration contains no machine-specific drive path;
- Qt, libmpv, downloads, and FetchContent cache are addressed through `../Qt/...`, `../libmpv/...`, `../downloads`, and `../cache/...`;
- there are no required `../cmake` or `../ninja` sibling directories;
- CMake `3.30.5+` and Ninja `1.12.1+` are compatible development-tool floors;
- scripts prefer tools on `PATH`, then the Qt Installer locations under `../Qt/Tools`;
- CTest is resolved independently and invoked through `ctest --preset`;
- Windows path validation treats `..\Qt\...` and `../Qt/...` as the same repository-relative form;
- Visual Studio x64 environment initialization is automatic through `vswhere.exe` and `vcvars64.bat`;
- Qt DLL/plugin paths for tests are resolved from the relative Qt root and exist only in the child process environment;
- `configure.ps1` uses CMake `--fresh` because generated CMake cache data is intentionally disposable and location-specific;
- starting with R2, libmpv is a required sibling dependency rather than an optional future dependency.

Qt `6.8.3`, MSVC compiler family `19.44`/toolset `14.44`, mpv/libmpv `0.41.0`, and FFmpeg `8.0.3` remain identity-sensitive.

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
  R2-01 fixed sibling-package discovery, imported target identity,
  and target-local runtime staging. It never searches random system mpv.

src/app/bootstrap/
  Startup ordering, application metadata, libmpv/graphics probes,
  RuntimePaths creation, logging/QML startup calls, and startup failures.

src/app/bootstrap/qml_bootstrap.*
  Owns the QQmlApplicationEngine load boundary and QML-load diagnostics.
  It does not own presentation business state.

src/app/composition/application_container.*
  Sole current composition root. Owns top-level runtime objects and
  their destruction order; contains no playback/business algorithm.

src/foundation/logging/
  Logging categories, file sink, rotation, redaction, and flush internals.

src/playback/infrastructure/mpv/runtime/
  Owns dependency-manifest parsing and validation of the actually loaded
  libmpv runtime/client-API identity. It does not own mpv_handle or playback state.

src/presentation/qml/App.qml
  Root QML entry only.

src/presentation/qml/shell/MainWindow.qml
  Visible application window and PlayerScreen composition only.

src/presentation/qml/screens/player/PlayerScreen.qml
  Minimal player presentation composition; no backend business logic.

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
| FFmpeg for libmpv build | exact `8.0.3`, LGPL-compatible configuration required by R0-03 |
| C++ | C++20 |

## Relative dependency and tool layout

```text
<parent>/
├─ cache/
│  └─ cmake/fetchcontent/
├─ downloads/
├─ libmpv/
│  └─ 0.41.0/windows-x64/
│     ├─ include/
│     │  └─ mpv/client.h
│     ├─ lib/
│     │  └─ <one MSVC-compatible mpv/libmpv import .lib>
│     ├─ bin/
│     │  ├─ <one primary libmpv-2.dll or mpv-2.dll>
│     │  └─ <runtime DLL dependencies from the same audited package>
│     └─ dependency-manifest.json
├─ Qt/
│  ├─ 6.8.3/msvc2022_64/
│  └─ Tools/
│     ├─ CMake_64/bin/cmake.exe
│     ├─ CMake_64/bin/ctest.exe
│     └─ Ninja/ninja.exe
└─ <repository>/
```

The import library and primary runtime DLL may also be placed directly in the fixed `windows-x64` root; the R2-01 resolver accepts only the explicitly enumerated names inside that same root. It does not scan other versions, sibling folders, PATH, or system mpv installations.

`scripts/bootstrap-workspace.ps1` may create only:

```text
../libmpv/0.41.0/windows-x64/
../downloads/
../cache/cmake/fetchcontent/
```

It does not create or modify Qt or installed development tools, and it does not fabricate a libmpv SDK.

The required libmpv dependency manifest is:

```json
{
  "mpv": {
    "version": "0.41.0",
    "tag": "v0.41.0",
    "commit": "41f6a645068483470267271e1d09966ca3b9f413"
  },
  "ffmpeg": {
    "version": "8.0.3"
  }
}
```

R2-01 prints SHA-256 values for the selected runtime DLL and import library during local dependency verification. The project does not invent hash values in source control before the actual project-controlled binary artifacts are available.

## Configure, build, and test

Run from the repository root:

```powershell
./scripts/verify-project-layout.ps1
./scripts/bootstrap-workspace.ps1
./scripts/verify-dependencies.ps1
./scripts/configure.ps1
./scripts/build.ps1
./scripts/test.ps1
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

and include the actual loaded DLL path, manifest path, mpv/tag/commit/FFmpeg identity, and client API version.

## Module growth rule

A file may receive new code only when the code has the same responsibility and reason to change. When a real second responsibility appears, upgrade the module into a responsibility directory instead of accumulating unrelated logic. Do not create source-history copies with suffixes such as `Old`, `New`, `V2`, `Final`, or `Copy`; Git owns history.

## Documentation

- Original full execution plan: `docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md`
- Approved R2-R14 fast-framework stage plans: `docs/plans/stages/00_INDEX.md`
- Third-party license inventory: `LICENSES/README.md`
- Build orchestrator: `cmake/CMakeLists.txt`
- Toolchain/dependency compatibility manifest: `cmake/DependencyVersions.cmake`
- Relative shared dependency paths: `cmake/DependencyPaths.cmake`
- R2 libmpv imported-target integration: `cmake/FindLibMpv.cmake`
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

R2-01 connected-environment validation is limited to source/CMake/module/path/final-diff review because the connected environment does not expose the user's sibling `../libmpv` binary package or Windows loader. Real libmpv dependency verification, linking, runtime staging, fifth CTest execution, and application runtime-probe output remain local acceptance requirements.

## Change Log

### 2026-08-07

- Started Stage R2 on the new `agent/r2-stage` branch from the accepted R1-06 head.
- Implemented R2-01 fixed-root libmpv integration with `LibMpv::LibMpv`, exact manifest identity checks, target-local runtime staging, a modular manifest parser/runtime probe, required R2 dependency verification, and the fifth `mpv_runtime_probe` Qt Test.
- Marked R1-06 accepted after explicit user confirmation of the local verification result.
- Implemented R1-06 QML shell diagnostics by preserving the modular `App -> MainWindow -> PlayerScreen` shell and adding concrete `QQmlEngine` warning capture plus detailed startup failure reporting.
- Implemented and accepted R1-05 Composition Root with `ApplicationContainer`, explicit top-level ownership, QML-before-logging shutdown ordering, idempotent shutdown, and an executable composition-root Qt Test.
- Corrected the documented application output path to `build/<preset>/Player.exe`; the Windows debug build is `build/windows-msvc-debug/Player.exe`.
- Recorded successful R1-01 through R1-04 Windows validation and closed their outdated pending-validation notes.
- Fixed Qt Test runtime injection, direct `ctest --preset` execution, relocation-safe CMake `--fresh`, compatible development-tool gates, Windows PowerShell 5.1 compatibility, automatic Visual Studio x64 initialization, and the parent-workspace relative-path contract during R1.
- Implemented R1-03 modular logging and committed the approved R2-R14 fast-framework Markdown taskbooks.
- Implemented R1-02 RuntimePaths and R1-01 modular CMake scaffold on the shared `agent/r1-stage` branch.
- Recorded R0-06 as explicitly skipped and unaccepted; R0-01 through R0-05 remain completed according to their recorded scope.

### R2-01 local verification after sync

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

R2-01 acceptance requires the real fixed sibling libmpv package to pass identity/artifact checks, the build to link and stage its runtime package, all five tests to pass, and the application to log `libmpv runtime validated` while loading the DLL from the staged executable directory. If the local sibling package is incomplete, the dependency/configure error is expected to name the missing header, MSVC import library, runtime DLL, or manifest instead of falling back to a system mpv.
