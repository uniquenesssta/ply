# Modular Qt 6 + libmpv Player

A Windows-first, cross-platform-ready desktop player project. The product name is intentionally temporary; rename it only when the final product identity is decided.

## Current baseline

This repository is the active R1 modular build scaffold. It currently provides:

- a root CMake entry limited to mandatory project/testing bootstrap and delegation to `cmake/CMakeLists.txt`;
- responsibility-separated CMake modules for dependency, compiler, analysis, target, source, and test configuration;
- a Qt 6.8.3 Qt Quick application bootstrap;
- OpenGL selection before `QGuiApplication` creation;
- a diagnostic QML loading boundary;
- a minimal QML shell split into application window, player screen, video surface, player chrome, and theme ownership;
- architecture decisions and the full development task book;
- a pinned Windows/MSVC/Qt/CMake/Ninja/mpv/FFmpeg version matrix;
- stable configure, build, and test entry scripts;
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
- a media asset-management system;
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

- `cmake/DependencyVersions.cmake` is the single exact version source used by CMake and PowerShell tooling;
- Windows, Windows SDK, Visual Studio, MSVC, Qt, CMake, Ninja, mpv/libmpv, FFmpeg, and C++ versions are pinned;
- Qt, CMake, Ninja, libmpv, downloads, and FetchContent cache use one versioned repository-parent layout whose committed paths all begin with `../`;
- automatic highest-version selection and generic unversioned tool paths were removed;
- CMake requires the exact Qt patch and the pinned minimum CMake release;
- PowerShell verification rejects mismatched toolchain versions and validates an installed libmpv manifest when Stage R2 files are present;
- the bootstrap script creates only project-owned shared staging/cache directories and does not create or modify Qt, CMake, or Ninja installations;
- clean-tree verification confirms shared dependencies resolve outside the repository and rejects staged SDK/cache/build directories;
- no third-party SDK, archive, runtime binary, or generated build output was added to Git.

R0-04 changes dependency-selection and validation behavior only. It does not download dependencies, build libmpv, or claim that the pinned Windows toolchain has been executed in this environment.

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

- the root `CMakeLists.txt` now owns only mandatory project declaration, the existing test gate, top-level CTest enablement, and delegation to `cmake/`;
- new `cmake/CMakeLists.txt` owns dependency/version module loading, build options, exact Qt resolution, Qt project setup, and source/test directory orchestration;
- `src/CMakeLists.txt`, `src/app/CMakeLists.txt`, `src/presentation/CMakeLists.txt`, and `tests/CMakeLists.txt` remain the authoritative target/source/test boundaries;
- application output remains under `build/<preset>/src/Player.exe`;
- `scripts/verify-project-layout.ps1` now requires the build orchestrator and rejects dependency lookup, target creation, target mutation, installation, or direct source/test orchestration in the root file;
- no application source, public interface, dependency version, runtime behavior, QML behavior, or package content changed.

A controlled structural verification completed configure, compilation, and the root `test` target using CMake 3.31.6 with a local Qt 6.8.3 contract package and stub source implementations. This verifies CMake scope propagation, child binary directories, target configuration, output placement, and root CTest discovery without claiming a real Qt/MSVC build.

R1-01 remains pending its hard acceptance check until the pinned Windows 10/11, Visual Studio 17.14.37, MSVC 19.44, Windows SDK 10.0.26100.0, CMake 3.31.12, Ninja 1.13.2, and Qt 6.8.3 environment successfully runs `scripts/configure.ps1` and `scripts/build.ps1`.

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
  Owns only project declaration, the existing test gate, top-level CTest
  enablement, and delegation to cmake/.

cmake/CMakeLists.txt
  Owns global build orchestration: version/path modules, compiler policies,
  Qt package resolution, and entry into src/ and tests/.

cmake/*.cmake
  Each owns one reusable build responsibility such as dependency identity,
  dependency paths, compiler policy, analysis, or application target policy.

src/CMakeLists.txt
  Owns creation/finalization and installation of the application target.

src/app/main.cpp
  Creates the process and delegates startup.

src/app/bootstrap/
  Owns startup ordering, graphics backend selection, application metadata,
  QML engine lifetime, and startup failure reporting.

src/presentation/qml/App.qml
  Owns only the root QML component.

src/presentation/qml/shell/MainWindow.qml
  Owns only the desktop application window.

src/presentation/qml/screens/player/PlayerScreen.qml
  Composes player presentation features without owning playback logic.

src/presentation/qml/features/player/video/VideoSurface.qml
  Reserves the future video-rendering presentation boundary.

src/presentation/qml/features/player/chrome/PlayerChrome.qml
  Owns the currently visible player chrome shell.

src/presentation/qml/theme/Theme.qml
  Owns the small set of visual tokens required by the current shell.

tests/CMakeLists.txt
  Owns test target registration when independently testable modules exist.
```

## Pinned toolchain matrix

`cmake/DependencyVersions.cmake` is authoritative for development, CI, dependency acquisition, and release builds.

| Component | Pinned baseline |
|---|---|
| Supported Windows minimum | Windows 10 22H2, build `10.0.19045` |
| Primary Windows validation family | Windows 11 24H2, build `10.0.26100` |
| Windows SDK | SDK family `10.0.26100.0`, servicing release `10.0.26100.8876` |
| Visual Studio | Visual Studio 2022 `17.14.37`, installation build `17.14.37516.0` |
| MSVC | v143 toolset `14.44`, compiler family `19.44`, x64 target |
| Qt | Qt `6.8.3`, MSVC 2022 64-bit kit |
| CMake | `3.31.12` |
| Ninja | `1.13.2` |
| mpv/libmpv | `0.41.0`, signed tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413` |
| FFmpeg for libmpv build | `8.0.3`, LGPL-compatible configuration required by R0-03 |
| C++ | C++20 |

A version change requires a separately reviewable Atomic Task. Local development and CI must not select a newer compatible installation silently.

## Shared dependency workspace

Keep reusable packages directly beside every repository that uses this baseline:

```text
<parent>/
├─ Qt/6.8.3/msvc2022_64/
├─ libmpv/0.41.0/windows-x64/
├─ cmake/3.31.12/bin/cmake.exe
├─ ninja/1.13.2/ninja.exe
├─ downloads/
├─ cache/cmake/fetchcontent/
└─ <repository>/
```

Committed dependency paths are derived from the pinned version manifest and always begin with `../`. There is no named aggregate dependency-root directory and no absolute machine path in source control.

The same parent-level Qt, libmpv, CMake, Ninja, download archive, and FetchContent cache can be reused by a second sibling repository. Deleting this repository's `build/` directory does not delete or redownload those shared dependencies.

`scripts/bootstrap-workspace.ps1` creates only these project-owned locations when missing:

```text
../libmpv/0.41.0/windows-x64/
../downloads/
../cache/cmake/fetchcontent/
```

It never creates or modifies installed Qt, CMake, or Ninja directories.

When the libmpv SDK is installed during R2, its root must contain `dependency-manifest.json` with at least:

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

Artifact SHA-256 values, complete build flags, and transitive dependency records are added to that manifest when the source archives and binaries actually exist; they are not guessed in R0-04.

## Configure and build

Install the exact pinned toolchain under the shared workspace and open the x64 Native Tools Command Prompt for Visual Studio 2022 `17.14.37`, then run:

```powershell
./scripts/verify-project-layout.ps1
./scripts/bootstrap-workspace.ps1
./scripts/verify-dependencies.ps1
./scripts/configure.ps1
./scripts/build.ps1
```

`verify-project-layout.ps1` validates both required files and the R1 CMake responsibility boundary before dependency checks begin. `verify-dependencies.ps1` rejects a mismatched Qt patch, CMake/Ninja version, Visual Studio release, MSVC compiler family, Windows SDK, or target architecture. It reports libmpv as optional until Stage R2; once libmpv headers exist, the version manifest becomes mandatory.

Run the generated executable from:

```text
build/windows-msvc-debug/src/Player.exe
```

Run tests:

```powershell
./scripts/test.ps1
```

There are no executable test cases in this framework commit. Test targets will be added with the first independently testable module rather than introducing placeholder tests.

## First Git submission

```powershell
git init
git add .
git commit -m "chore: establish modular Qt player scaffold"
git branch -M main
git remote add origin <your-repository-url>
git push -u origin main
```

Before committing, verify that no build output or local runtime files are staged:

```powershell
./scripts/verify-clean-tree.ps1
git status
```

## Module growth rule

A file may receive new code only when the code has the same responsibility and reason to change. When an existing file gains a separately evolving responsibility, convert it to a responsibility directory and place the original and new files inside that directory. Do not create source-history copies with names such as `Old`, `New`, `V2`, `Final`, or `Copy`; Git owns history.

## Documentation

- Full execution plan: `docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md`
- Third-party license inventory: `LICENSES/README.md`
- Build orchestrator: `cmake/CMakeLists.txt`
- Toolchain version manifest: `cmake/DependencyVersions.cmake`
- Shared dependency paths: `cmake/DependencyPaths.cmake`
- Render embedding and lifecycle decision: `docs/decisions/ADR-0001-libmpv-render-api.md`
- OpenGL and Qt Quick FBO decision: `docs/decisions/ADR-0002-opengl-first.md`
- Playback-state ownership: `docs/decisions/ADR-0003-single-playback-owner.md`
- QML boundaries: `docs/decisions/ADR-0004-qml-boundaries.md`
- Media fixture policy: `docs/testing/media-fixture-policy.md` (existing baseline only; R0-06 was skipped and not accepted)

## Validation record

Validated in the generation environment:

- `CMakePresets.json` is syntactically valid and requires CMake `3.31.12`;
- `cmake/DependencyVersions.cmake` parses successfully and contains every required matrix entry exactly once;
- CMake and PowerShell derive the same exact versioned repository-parent paths;
- every committed shared dependency path begins with `../` and no absolute path is stored;
- CMake requests Qt `6.8.3` with `EXACT` matching;
- the PowerShell dependency scripts use the version manifest rather than selecting the highest discovered installation;
- the workspace bootstrap does not create or modify Qt, CMake, or Ninja directories;
- the libmpv manifest contract includes the pinned mpv release/tag/commit and FFmpeg release;
- no third-party SDK, archive, runtime binary, cache, or generated build output was added to the repository;
- previous R0-01 through R0-05 documentation and architecture records remain present;
- R0-06 is explicitly recorded as skipped rather than complete;
- the root CMake file contains no dependency lookup, Qt setup, target creation/mutation, installation, or direct source/test subdirectory logic;
- `cmake/CMakeLists.txt` owns all global build modules and adds the existing source and test boundaries with explicit binary directories;
- a controlled CMake 3.31.6 structural run configured, compiled, linked `src/Player`, generated the root CTest file, and executed the empty `test` target successfully;
- the R0-05 ADRs explicitly choose Render API over `wid` and define OpenGL, QQuickFramebufferObject, thread ownership, callback constraints, initialization, failure, and destruction order;
- `src/app/main.cpp` invokes graphics backend configuration before `QGuiApplication`, and `GraphicsBackendBootstrap` selects `QSGRendererInterface::OpenGL`.

Not executed in the generation environment:

- Windows PowerShell script execution, because the connected execution environment is not Windows and does not contain PowerShell;
- exact Windows/Visual Studio/MSVC/Qt/CMake/Ninja verification, because the pinned Windows toolchain is not installed in the environment;
- configure and compilation against the real Qt 6.8.3 MSVC kit, so R1-01 hard acceptance remains pending;
- QML runtime launch or CTest against the real application for the same reason;
- libmpv Render API initialization and frame rendering, because those implementations begin in Stages R2 and R4;
- render-thread assertions, resize/DPI/minimize behavior, repeated render-context creation/destruction, and shutdown-race tests, because the render modules do not exist yet;
- binary license scanning, because no Qt, libmpv, FFmpeg, or transitive runtime binary is committed;
- legal-counsel and codec-patent review, which remain required before public distribution.

The required Windows acceptance action for R1-01 is to install the exact matrix under the documented sibling paths, open the pinned x64 Visual Studio developer shell, and run `scripts/verify-project-layout.ps1`, `scripts/verify-dependencies.ps1`, `scripts/configure.ps1`, and `scripts/build.ps1`.

## Change Log

### 2026-08-07

- Started Stage R1 on `agent/r1-stage` and implemented R1-01 by moving global dependency, Qt, compiler, target, source, and test orchestration under `cmake/`, reducing the root build file to mandatory bootstrap/delegation, and adding a build-boundary verifier.
- Recorded R0-06 as explicitly skipped by user direction; it remains unaccepted and must not be represented as completed fixture coverage.
- Completed Atomic Task R0-05 by freezing the libmpv Render API, OpenGL/QQuickFramebufferObject integration, thread ownership, callback behavior, startup failure policy, and render-before-core shutdown order.
- Completed Atomic Task R0-04 by pinning the Windows/MSVC/Qt/CMake/Ninja/mpv/FFmpeg matrix, replacing automatic tool discovery with exact versioned sibling paths, and adding strict dependency and Git-boundary validation.
- Completed Atomic Task R0-03 by selecting the LGPL-compatible dynamic-linking route for Qt, libmpv, and FFmpeg and defining source, notice, manifest, transitive-dependency, and release-blocking requirements.
- Completed Atomic Task R0-02 by freezing the first-release scope, adding observable acceptance for every mandatory MVP capability group, and separating deferred and excluded capabilities.
- Completed and verified Atomic Task R0-01 governance and repository baseline; confirmed all required root artifacts and a generated-artifact-free tracked Git tree.
- Disabled automatic C++ QML type registration for the QML-only scaffold so Qt does not feed an empty or malformed generated metatypes JSON file to `qmltyperegistrar`; this must be removed when the first `Q_OBJECT`/`QML_ELEMENT` type is introduced.
- Fixed workspace bootstrap path resolution so PowerShell location changes cannot redirect `../` creation into the Visual Studio installation directory.
- Stopped bootstrap from creating or touching Qt, CMake, and Ninja installation directories.
- Added complete-project verification before configure to detect a partial update package used as a standalone repository.
- Added an x64 MSVC target check so a 32-bit developer shell fails before Qt package resolution.
- Changed all reusable package paths to direct repository-parent paths beginning with `../`; removed the named dependency-root directory.
- Added automatic sibling discovery for Qt, portable CMake, and Ninja, with shared libmpv and FetchContent locations reserved for later stages.
- Established the initial Git-ready Qt 6/QML framework.
- Added modular startup and presentation boundaries.
- Selected the OpenGL Qt Quick backend for the future libmpv Render API path.
- Added project governance files, architecture decisions, media fixture policy, and the complete development task book.
- Added Windows configure/build/test scripts and build-output exclusions.
