# Modular Qt 6 + libmpv Player

A Windows-first, cross-platform-ready desktop player project. The product name is intentionally temporary; rename it only when the final product identity is decided.

## Current baseline

This repository is the active R1 modular build scaffold. It currently provides:

- a root CMake entry limited to mandatory project/testing bootstrap and delegation to `cmake/CMakeLists.txt`;
- responsibility-separated CMake modules for dependency, compiler, analysis, target, source, and test configuration;
- a Qt 6.8.3 Qt Quick application bootstrap;
- a dedicated `RuntimePaths` bootstrap module for installed and portable path resolution;
- a dedicated foundation logging module with categories, file sink, rotation, redaction, and shutdown flush;
- repository/build/dependency configuration that stores only repository-relative paths rather than machine-specific drive paths;
- OpenGL selection before `QGuiApplication` creation;
- a diagnostic QML loading boundary;
- a minimal QML shell split into application window, player screen, video surface, player chrome, and theme ownership;
- architecture decisions, the original full development task book, and the approved R2-R14 fast-framework stage taskbooks under `docs/plans/stages/`;
- an explicit Windows/MSVC/Qt/libmpv/FFmpeg identity baseline plus minimum-compatible CMake/Ninja development-tool gates;
- stable configure, build, and test entry scripts that can initialize the Visual Studio x64 build environment from an ordinary Windows CMD/PowerShell session;
- Qt Test targets covering installed/portable RuntimePaths and R1-03 logging behavior;
- no bundled third-party runtime binaries.

The scaffold intentionally does **not** yet implement libmpv loading, playback commands, playback state, rendering, persistence, settings, playlists, platform integrations, or packaging. Those responsibilities will be introduced only in their Atomic Tasks.

## Scope

### Product target

The product is a Qt 6 desktop media player using libmpv as its playback core. Windows 10/11 x64 is the first release platform. The architecture must keep playback, application state, rendering, persistence, platform integration, and QML presentation separated so future features do not accumulate inside the player screen or playback backend.

### MVP delivery scope and acceptance matrix

The following capabilities are frozen for the first release. This table defines observable acceptance, not current implementation status; implementation remains pending in the later Atomic Tasks.

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

The MVP is not complete solely because each feature has code. Release acceptance also requires:

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

These capabilities are planned after MVP and must not be pulled into first-release implementation without an explicit scope change:

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

The following capabilities are not part of MVP and must not be implemented speculatively:

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

Verified on 2026-08-07:

- the Git repository is initialized on `main`;
- `ALL_AI_CODE.md` and `AI_PROJECT_RULES.md` are readable at the repository root;
- `README.md` contains the project baseline and canonical `Change Log`;
- the complete development task book is tracked under `docs/plans/`;
- the top-level `CMakeLists.txt` is present;
- `.gitignore` excludes CMake/Qt outputs, runtime logs and databases, IDE state, packaging output, and local environment files;
- the tracked Git tree contains no build output, generated runtime data, database files, logs, or IDE state.

No source, dependency, interface, configuration, or runtime behavior changed for R0-01. The existing first commit already contained the required governance and repository baseline, so this Atomic Task records and verifies that baseline without duplicating files or modifying later-stage implementation.

### R0-02 — Complete

Verified on 2026-08-07:

- README contains a dedicated first-release Scope section;
- all 25 mandatory MVP capability groups from the task book have observable acceptance conditions;
- cross-cutting completion gates cover performance, lifecycle, stale-event isolation, persistence failure, QML/libmpv separation, independent verification, and clean-machine packaging;
- deferred second-stage enhancements are separated from MVP;
- explicitly excluded first-release capabilities are recorded and cannot be introduced without a deliberate scope change;
- no source, dependency, build, configuration, interface, or runtime behavior changed.

R0-02 freezes product boundaries only. It does not claim that any MVP playback capability is already implemented or verified at runtime.

### R0-03 — Complete

Verified on 2026-08-07:

- `LICENSES/README.md` identifies authoritative sources and distribution obligations for Qt, libmpv, FFmpeg, transitive dependencies, and the MSVC runtime;
- the selected Qt open-source route uses dynamically linked LGPLv3 components and excludes GPL-only Qt modules unless separately approved;
- libmpv must be built reproducibly from the official mpv source with `-Dgpl=false` and may not be replaced by an unverified prebuilt DLL;
- FFmpeg must remain LGPL-compatible and must not use `--enable-gpl` or `--enable-nonfree`;
- static linking, unknown-license components, untracked transitive dependencies, missing corresponding source, and mismatched notices are release blockers;
- packaging records must include exact versions, hashes, build options, linkage, notices, corresponding source, modifications, and dependency scans;
- exact dependency versions are assigned to R0-04, while acquired artifact hashes and actual binary verification remain assigned to integration and packaging tasks;
- no source code, dependency binary, build configuration, interface, or runtime behavior changed.

R0-03 defines the engineering compliance route and release gate. It does not constitute legal advice or approve any binary that has not yet been selected and audited.

### R0-04 — Complete

Verified on 2026-08-07:

- `cmake/DependencyVersions.cmake` established the original exact toolchain/dependency baseline used by CMake and PowerShell tooling;
- Windows, Windows SDK, Visual Studio, MSVC, Qt, CMake, Ninja, mpv/libmpv, FFmpeg, and C++ identities were recorded;
- Qt, libmpv, downloads, and FetchContent cache use versioned repository-parent-relative locations;
- automatic highest-version selection and generic unversioned dependency discovery were removed;
- Qt remains exact and product/runtime dependency identity remains explicit;
- PowerShell verification validates an installed libmpv manifest when Stage R2 files are present;
- the bootstrap script creates only project-owned shared staging/cache directories and does not create or modify Qt installations;
- no third-party SDK, archive, runtime binary, or generated build output was added to Git.

R0-04 remains the historical dependency identity baseline. The user's later R1 direction to keep early framework work fast and avoid over-hard development conditions supersedes the **exact patch-level requirement for development tools only**. Product/runtime dependencies remain controlled, while CMake/Ninja and servicing-level Visual Studio/Windows SDK checks now use compatible minimum/family gates documented below.

### R0-05 — Complete

Verified on 2026-08-07:

- `ADR-0001` selects the in-process libmpv Render API and explicitly rejects `--wid`/native child-window embedding, an external `mpv.exe` playback process, deprecated `opengl-cb`, and CPU frame-copy rendering for the MVP;
- `ADR-0002` fixes Qt Quick to OpenGL and selects `QQuickFramebufferObject` plus a dedicated renderer as the first-release composition path;
- GUI, playback, and Qt Quick render-thread responsibilities are separated, with no normal libmpv client calls or QML access from the render thread;
- render update and wakeup callbacks are signal-only bridges and cannot execute rendering, blocking work, or application-state mutation;
- initialization requires the OpenGL backend before `QGuiApplication`, an initialized mpv core, a current render-thread OpenGL context, and a ready render context before video playback starts;
- shutdown invalidates callbacks, waits for active rendering to leave its critical section, frees the render context while the matching OpenGL context is current, and destroys the mpv core only afterward;
- OpenGL or render-context initialization failure is an explicit startup/render error and does not silently fall back to `wid`, another Qt graphics API, or a second renderer;
- the existing bootstrap already calls `QQuickWindow::setGraphicsApi(OpenGL)` before constructing `QGuiApplication`;
- no source code, dependency, public interface, configuration value, or runtime behavior changed.

R0-05 freezes the rendering architecture and lifecycle contract. Actual libmpv rendering, runtime thread assertions, resize/DPI behavior, and shutdown-race tests remain assigned to Stages R2 and R4.

### R0-06 — Skipped by explicit user direction

On 2026-08-07 the user directed the project to proceed directly to Stage R1 without implementing R0-06.

- no media fixture policy, manifest, sample, or legal-source record was added or changed;
- R0-06 is not accepted or complete;
- later playback tests must not claim complete legal fixture coverage until this task is deliberately resumed or replaced by an approved equivalent.

### R1-01 — Implemented; exact Windows validation pending

Implemented on 2026-08-07 in the shared R1 stage branch:

- the root `CMakeLists.txt` owns only mandatory project declaration, the test gate, top-level CTest enablement, and delegation to `cmake/`;
- `cmake/CMakeLists.txt` owns dependency/version module loading, build options, exact Qt resolution, Qt project setup, and source/test directory orchestration;
- `src/CMakeLists.txt`, `src/app/CMakeLists.txt`, `src/presentation/CMakeLists.txt`, and `tests/CMakeLists.txt` remain authoritative target/source/test boundaries;
- application output remains under `build/<preset>/src/Player.exe`;
- `scripts/verify-project-layout.ps1` enforces the build responsibility boundary.

A controlled structural verification completed configure, compilation, and the root test target using CMake 3.31.6 with a local Qt 6.8.3 contract package and stub source implementations. This verifies CMake scope propagation without claiming a real Qt/MSVC build.

### R1-02 — Implemented; exact Qt/Windows test execution pending

Implemented on 2026-08-07 in `agent/r1-stage`:

- `src/app/bootstrap/runtime_paths.*` is the single owner of runtime path resolution;
- installed mode uses `QStandardPaths` for configuration/application data and derives logs/screenshots from those runtime locations;
- portable mode is explicitly selected by a `portable.flag` beside the executable and keeps `config`, `data`, `logs`, and `screenshots` beside the executable;
- path resolution is side-effect free and does not create directories;
- `tests/unit/app/bootstrap/runtime_paths_test.cpp` covers installed/portable behavior, marker detection, path placement, and absence of directory-creation side effects.

RuntimePaths may naturally return absolute operating-system paths at runtime (for example a Windows user data directory). Those are runtime values, not committed machine-specific configuration, and are intentionally not converted into repository-relative paths.

### R1-03 — Implemented; Windows/Qt execution pending

Implemented on 2026-08-07 in `agent/r1-stage`:

- added `src/foundation/` as a real build/module boundary and `player_foundation` target;
- added logging categories for `app.lifecycle`, `app.bootstrap`, `playback.mpv`, `playback.render`, `ui.interaction`, and `persistence`;
- added a thread-safe `LogFileSink` installed through Qt's message handler, with UTC timestamps, severity, thread ID, category, bounded size rotation, explicit flush/stop, and retained diagnostic error text;
- default rotation is `player.log` plus up to three archives with a 4 MiB active-file threshold;
- added `LogRedactor` for token/API-key/password/authorization values, bearer tokens, URL user credentials, and home-directory path prefixes;
- added `LoggingBootstrap`, initialized after RuntimePaths and before QML, and explicitly stopped after the event loop; an unavailable log directory reports a warning rather than preventing the application from starting;
- added executable Qt Test coverage for redaction, Qt message-handler writes and shutdown flush, multi-threaded writes, rotation, and an invalid/uncreatable log directory;
- the approved R2-R14 “快速框架实施版” is committed as individual Markdown stage documents under `docs/plans/stages/`.

#### R1 current parent-workspace and relative-path policy

The repository may be renamed or moved, but its parent directory is the dependency workspace. The supported shape is:

```text
<parent>/
├─ cache/
├─ downloads/
├─ libmpv/
├─ Qt/
├─ <repository>/
└─ <other repository, optional>/
```

The current rules are:

- no committed build/dependency configuration may contain a machine-specific absolute Windows drive path such as `F:\...`;
- Qt, libmpv, downloads, and FetchContent cache are always addressed from the repository root through `../Qt/...`, `../libmpv/...`, `../downloads`, and `../cache/...`;
- CMake and Ninja are **not** additional parent-level dependency directories;
- CMake `3.30.5` and Ninja `1.12.1` are the minimum compatible development-tool versions for the current scaffold; scripts resolve them from the active `PATH` first, then from the official Qt installer tool locations `../Qt/Tools/CMake_64/bin/cmake.exe` and `../Qt/Tools/Ninja/ninja.exe`;
- Windows path validation normalizes `..\Qt\...` and `../Qt/...` to the same repository-relative form, so PowerShell `Join-Path` cannot turn a valid parent-relative path into a false rejection;
- CMake source/test directory references, build preset output directories, PowerShell manifest lookup, and test source include paths use relative forms;
- `scripts/verify-project-layout.ps1` rejects machine-absolute drive literals and rejects invented `../cmake/...` or `../ninja/...` parent roots in the dependency-path modules;
- the Windows PowerShell 5.1 `$Name:` interpolation parse failure is fixed by using `${Name}:`;
- the Windows PowerShell 5.1 empty generic-list binding failure is fixed by explicitly allowing an empty `Failures` collection and suppressing the return value from `List.Add()`;
- `verify-dependencies.ps1` initializes Visual Studio x64 tools automatically through `vswhere.exe` and `vcvars64.bat`; `configure.ps1`, `build.ps1`, and `test.ps1` therefore work from an ordinary CMD/PowerShell session when a compatible Visual Studio installation is present;
- `cl.exe` is not executed without input merely to read its banner; MSVC compiler identity is read from the executable file version, avoiding Windows PowerShell 5.1 `NativeCommandError` behavior.

Qt `6.8.3`, MSVC compiler family `19.44`/toolset `14.44`, mpv/libmpv `0.41.0`, and FFmpeg `8.0.3` remain identity-sensitive. Development tools are accepted when they satisfy the compatible baseline instead of matching an arbitrary servicing patch exactly.

## Architecture boundary

```text
QML presentation
      ↓ user intent / projected state
Application layer (future playback session)
      ↓ commands / events
libmpv adapter (future R2)
      ↓ public C API
libmpv
```

Current build and source responsibilities:

```text
CMakeLists.txt
  Owns only project declaration, test gate, top-level CTest enablement,
  and delegation to cmake/.

cmake/CMakeLists.txt
  Owns global build orchestration and enters ../src and ../tests through
  relative source/binary paths.

cmake/*.cmake
  Each owns one reusable build responsibility such as dependency identity,
  dependency paths, compiler policy, analysis, or application target policy.

scripts/modules/MsvcEnvironment.psm1
  Owns Visual Studio discovery and process-local x64 developer-environment
  initialization for normal CMD/PowerShell entry points.

src/CMakeLists.txt
  Owns application target finalization and the foundation/app/presentation
  module composition.

src/foundation/
  Owns architecture-level primitives shared by multiple top-level modules.
  R1-03 introduces the first real foundation responsibility: logging.

src/foundation/logging/
  Owns categories, file-sink behavior, rotation, redaction, and file-log
  diagnostics. It does not own RuntimePaths or application startup.

src/app/bootstrap/
  Owns startup ordering, graphics backend selection, application metadata,
  RuntimePaths, LoggingBootstrap, QML engine lifetime, and startup failures.

src/app/bootstrap/runtime_paths.*
  Resolves installed/portable runtime directories without creating them.

src/app/bootstrap/logging_bootstrap.*
  Connects RuntimePaths to the foundation log sink and owns its application
  lifetime; it does not implement sink/rotation/redaction internals.

src/presentation/qml/App.qml
  Owns only the root QML component.

src/presentation/qml/screens/player/PlayerScreen.qml
  Composes player presentation features without owning playback logic.

tests/CMakeLists.txt
  Owns test registration for RuntimePaths and logging.
```

## Toolchain and dependency compatibility matrix

`cmake/DependencyVersions.cmake` is authoritative for the current development compatibility floor and identity-sensitive dependencies.

| Component | Current baseline |
|---|---|
| Supported Windows minimum | Windows 10 22H2, build `10.0.19045` |
| Primary Windows validation family | Windows 11 24H2, build `10.0.26100` |
| Windows SDK | minimum compatible SDK `10.0.26100.0`; reference servicing release `10.0.26100.8876` |
| Visual Studio | Visual Studio 2022 `17.14` family; `17.14.37` / build `17.14.37516.0` retained as reference validation point |
| MSVC | v143 toolset family `14.44`, compiler family `19.44`, x64 target |
| Qt | exact Qt `6.8.3`, MSVC 2022 64-bit kit |
| CMake | minimum `3.30.5` |
| Ninja | minimum `1.12.1` |
| mpv/libmpv | exact `0.41.0`, signed tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413` |
| FFmpeg for libmpv build | exact `8.0.3`, LGPL-compatible configuration required by R0-03 |
| C++ | C++20 |

Changing an identity-sensitive product/runtime dependency still requires a separately reviewable task. Development-tool servicing versions may move within the documented compatible family/minimum when real configure/build/test evidence supports the change and README is updated.

## Relative dependency and tool layout

For a checkout such as `F:\QT6-PLAYER\qt6-player r1`, only the parent relationship matters. The committed configuration is equivalent to:

```text
<parent>/
├─ cache/
│  └─ cmake/fetchcontent/
├─ downloads/
├─ libmpv/
│  └─ 0.41.0/windows-x64/
├─ Qt/
│  ├─ 6.8.3/msvc2022_64/
│  └─ Tools/
│     ├─ CMake_64/bin/cmake.exe
│     └─ Ninja/ninja.exe
└─ <repository>/
```

The four parent-level dependency roots are therefore `../Qt`, `../libmpv`, `../downloads`, and `../cache`. There is no required `../cmake` or `../ninja` sibling directory.

No drive letter or repository-specific absolute path is stored in source control. CMake and Qt may internally resolve a relative source/dependency path to an absolute filesystem path while configuring or running; that transient runtime resolution is not committed configuration. Visual Studio itself is an installed development tool and is discovered at runtime through `vswhere.exe`; its installation path is never committed as project configuration.

`scripts/bootstrap-workspace.ps1` creates only these project-owned relative locations when missing:

```text
../libmpv/0.41.0/windows-x64/
../downloads/
../cache/cmake/fetchcontent/
```

It does not create or modify Qt or tool installations.

When the libmpv SDK is installed during R2, its relative root must contain `dependency-manifest.json` with at least:

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

## Configure and build

Place Qt at `../Qt/6.8.3/msvc2022_64`. The Qt Installer-provided CMake/Ninja under `../Qt/Tools` are accepted when they meet the minimum compatible versions; newer compatible tools on `PATH` are also accepted. A normal Windows CMD or PowerShell session is supported because the verification script initializes the installed Visual Studio 2022 x64 environment automatically through `vswhere.exe` and `vcvars64.bat`.

Run from the repository root:

```powershell
./scripts/verify-project-layout.ps1
./scripts/bootstrap-workspace.ps1
./scripts/verify-dependencies.ps1
./scripts/configure.ps1
./scripts/build.ps1
./scripts/test.ps1
```

`configure.ps1`, `build.ps1`, and `test.ps1` invoke dependency verification before doing work, so each standalone command receives the same process-local MSVC environment. `verify-dependencies.ps1` checks minimum/family compatibility for development tools and exact identity for Qt and later libmpv manifest data. libmpv remains optional until Stage R2; once libmpv headers exist, its dependency manifest becomes mandatory.

Run the generated executable from the repository-relative output:

```text
build/windows-msvc-debug/src/Player.exe
```

## Module growth rule

A file may receive new code only when the code has the same responsibility and reason to change. When an existing file gains a separately evolving responsibility, convert it to a responsibility directory and place the original and new files inside that directory. Do not create source-history copies with names such as `Old`, `New`, `V2`, `Final`, or `Copy`; Git owns history.

## Documentation

- Original full execution plan: `docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md`
- Approved R2-R14 fast-framework stage plans: `docs/plans/stages/00_INDEX.md`
- Third-party license inventory: `LICENSES/README.md`
- Build orchestrator: `cmake/CMakeLists.txt`
- Toolchain/dependency compatibility manifest: `cmake/DependencyVersions.cmake`
- Relative shared dependency paths: `cmake/DependencyPaths.cmake`
- Render embedding and lifecycle decision: `docs/decisions/ADR-0001-libmpv-render-api.md`
- OpenGL and Qt Quick FBO decision: `docs/decisions/ADR-0002-opengl-first.md`
- Playback-state ownership: `docs/decisions/ADR-0003-single-playback-owner.md`
- QML boundaries: `docs/decisions/ADR-0004-qml-boundaries.md`
- Media fixture policy: `docs/testing/media-fixture-policy.md` (existing baseline only; R0-06 was skipped and not accepted)

## Validation record

Observed on the user's Windows 10 `10.0.19045.5917` workspace after syncing commit `533a668`:

- `scripts/verify-project-layout.ps1` passed with parent-relative path and Windows-separator validation;
- Qt `6.8.3` was found through `../Qt/6.8.3/msvc2022_64`;
- CMake `3.30.5` and Ninja `1.12.1` were found from the existing Qt Tools environment;
- Visual Studio x64 environment initialization through `vswhere.exe` + `vcvars64.bat` succeeded;
- `cl.exe` resolved to compiler family `19.44` (`19.44.35228` banner observed);
- the remaining failure was caused by invoking `cl.exe` without a compilation input solely to parse its banner: Windows PowerShell 5.1 promoted the native stderr output to `NativeCommandError` before later checks could run;
- libmpv remained correctly optional until R2.

Implemented in the current follow-up:

- CMake `3.30.5` and Ninja `1.12.1` are now minimum-compatible development-tool baselines instead of exact patch pins;
- top-level `cmake_minimum_required` and `CMakePresets.json` were aligned to CMake `3.30.5`;
- CMake/Ninja verification accepts newer compatible versions rather than demanding equality;
- MSVC compiler identity is read from `cl.exe` file-version metadata instead of executing `cl.exe` without input;
- the v143 `14.44` toolset family is independently checked through `VCToolsVersion`;
- Visual Studio validation accepts the `17.14` family instead of one servicing build only;
- Windows SDK validation uses `10.0.26100.0` as a minimum compatible SDK rather than requiring one exact servicing identity;
- project-layout verification now guards the compatible-tool verifier structure;
- no R1-03 logging behavior, Qt version, libmpv/FFmpeg identity, relative dependency layout, or R2-R14 taskbook was changed.

Not executed in the connected generation environment:

- Windows PowerShell runtime execution of this follow-up, because the connected environment does not provide Windows PowerShell;
- the user's next dependency verification after the `cl.exe` probe fix;
- configure/build against the real Qt 6.8.3 MSVC kit;
- `runtime_paths` and `logging` Qt Test execution;
- application runtime log-directory verification;
- later libmpv/render/media validation;
- binary license scanning and legal/patent review.

The next local Windows verification after syncing this follow-up is:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify-project-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\verify-dependencies.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

If a subsequent check fails, the output should now represent a real missing/incompatible component or a compile/test defect rather than the already-fixed absolute-path, separator, empty-list, developer-shell, or `cl.exe` banner-probe issues.

## Change Log

### 2026-08-07

- Replaced exact CMake/Ninja patch pins with minimum-compatible development-tool gates matching the existing Qt Tools environment: CMake `3.30.5+` and Ninja `1.12.1+`; product/runtime dependency identity remains controlled.
- Fixed Windows PowerShell 5.1 `NativeCommandError` during MSVC verification by reading the `cl.exe` file version instead of executing `cl.exe` without input; added an independent v143 `14.44` toolset-family check.
- Fixed Windows parent-relative path validation so PowerShell `Join-Path` output such as `..\Qt\Tools\...` is accepted as the same repository-relative location as `../Qt/Tools/...`.
- Added modular automatic Visual Studio x64 environment initialization through `vswhere.exe` + `vcvars64.bat`, allowing verify/configure/build/test scripts to be launched from an ordinary CMD/PowerShell session.
- Corrected the R1 parent-workspace contract to match the actual layout: only `Qt`, `libmpv`, `downloads`, and `cache` are parent-level dependency roots; removed the invented `../cmake` and `../ninja` sibling assumptions and added Qt-tools fallback discovery.
- Fixed the Windows PowerShell 5.1 empty generic-list binding failure in `verify-dependencies.ps1` by allowing an initially empty failure collection.
- Implemented Atomic Task R1-03 with modular logging categories, a thread-safe rotating file sink, secret/path redaction, startup/shutdown logging bootstrap, and executable logging tests.
- Fixed the Windows PowerShell 5.1 `InvalidVariableReferenceWithDrive` parser failure by changing `$Name:` interpolations to `${Name}:` in dependency verification.
- Added the approved R2-R14 “快速框架实施版” as individual Markdown taskbooks under `docs/plans/stages/`.
- Implemented Atomic Task R1-02 with a side-effect-free RuntimePaths module, deterministic `portable.flag` mode selection, installed/portable path contracts, and the first executable Qt Test target.
- Started Stage R1 on `agent/r1-stage` and implemented R1-01 by moving global dependency, Qt, compiler, target, source, and test orchestration under `cmake/`, reducing the root build file to mandatory bootstrap/delegation, and adding a build-boundary verifier.
- Recorded R0-06 as explicitly skipped by user direction; it remains unaccepted and must not be represented as completed fixture coverage.
- Completed Atomic Task R0-05 by freezing the libmpv Render API, OpenGL/QQuickFramebufferObject integration, thread ownership, callback behavior, startup failure policy, and render-before-core shutdown order.
- Completed Atomic Task R0-04 by pinning the original Windows/MSVC/Qt/CMake/Ninja/mpv/FFmpeg baseline and adding strict dependency/version validation; later R1 user direction relaxed only development-tool servicing gates.
- Completed Atomic Task R0-03 by selecting the LGPL-compatible dynamic-linking route for Qt, libmpv, and FFmpeg and defining source, notice, manifest, transitive-dependency, and release-blocking requirements.
- Completed Atomic Task R0-02 by freezing the first-release scope, adding observable acceptance for every mandatory MVP capability group, and separating deferred and excluded capabilities.
- Completed and verified Atomic Task R0-01 governance and repository baseline; confirmed all required root artifacts and a generated-artifact-free tracked Git tree.

#### R1-04 — GraphicsBackendBootstrap

Implemented on 2026-08-07 in `agent/r1-stage`:

- upgraded the former single `graphics_backend_bootstrap.*` files into `src/app/bootstrap/graphics_backend/`, keeping pre-application backend selection separate from runtime graphics probing;
- `GraphicsBackendBootstrap::configure()` still forces Qt Quick to OpenGL before `QGuiApplication` construction;
- added `GraphicsBackendProbe`, executed after logging starts and before QML loads, to verify that Qt Quick is configured for OpenGL, create a real `QOpenGLContext` with a matching `QOffscreenSurface`, make the context current, and query `GL_VENDOR`, `GL_RENDERER`, `GL_VERSION`, GLSL version, OpenGL/ES profile, and negotiated context version;
- successful startup records the actual OpenGL renderer information through `app.bootstrap` logging; OpenGL context/probe failure is a startup failure and does not silently fall back to Direct3D, Vulkan, software rendering, or `wid` embedding;
- added an executable `graphics_backend` Qt Test covering the forced OpenGL API and real offscreen context/renderer probe;
- `scripts/verify-project-layout.ps1` now enforces the graphics-backend directory module and rejects the obsolete pre-R1-04 root bootstrap files;
- no libmpv Render API object, playback state, QML player behavior, third-party dependency, persisted data, or platform-specific renderer fallback was introduced in R1-04.

Validation performed in the connected environment:

- source/CMake/module-boundary review completed;
- Qt 6.8 API usage was checked against the official Qt documentation for `QQuickWindow::setGraphicsApi()`, `QQuickWindow::graphicsApi()`, `QOpenGLContext`, and `QOpenGLFunctions`;
- the earlier implementation draft was corrected before commit because Qt 6.8 declares `QOpenGLFunctions::initializeOpenGLFunctions()` as `void`, so the final probe uses the ready `QOpenGLContext::functions()` resolver while the context is current.

Validation still required on the user's Windows workspace:

- `verify-project-layout.ps1`, dependency verification, configure, build, and CTest;
- the `graphics_backend` test must create a real OpenGL context and report a non-empty renderer/version;
- running `Player.exe` must produce a `Graphics backend validated` log line containing the actual vendor/renderer/version information.
