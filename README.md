# Modular Qt 6 + libmpv Player

A Windows-first, cross-platform-ready desktop player project. The product name is intentionally temporary; rename it only when the final product identity is decided.

## Current baseline

This repository is the first Git-ready framework commit. It currently provides:

- a modular CMake/Ninja build entry;
- a Qt 6.8+ Qt Quick application bootstrap;
- OpenGL selection before `QGuiApplication` creation;
- a diagnostic QML loading boundary;
- a minimal QML shell split into application window, player screen, video surface, player chrome, and theme ownership;
- architecture decisions and the full development task book;
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

Current source responsibilities:

```text
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
```

## Required toolchain

Install on Windows:

- Git for Windows;
- Visual Studio 2022 Community or Build Tools with **Desktop development with C++**;
- MSVC v143 x64/x86 build tools and a Windows 10/11 SDK;
- Qt 6.8 or newer with the **MSVC 2022 64-bit** kit;
- PowerShell 7 or Windows PowerShell.

Keep reusable portable packages directly beside the repository:

```text
<parent>/
├─ Qt/<version>/msvc2022_64/
├─ libmpv/windows-x64/          # required from R2 onward
├─ cmake/                       # optional standalone portable CMake
├─ ninja/                       # optional standalone portable Ninja
├─ downloads/
├─ cache/cmake/fetchcontent/
└─ <repository>/
```

There is intentionally no named dependency-root directory. Every committed dependency path starts with `../`, so changing the repository folder name or moving the whole parent directory does not require editing paths.

CMake 3.25 or newer and Ninja are discovered in this order: standalone copies under `../cmake` and `../ninja`, Qt Online Installer tools under `../Qt/Tools`, then system `PATH`. You therefore do not need to download duplicate copies when the Qt installation already contains them. `libmpv` is not required for this initial scaffold; its exact build and version will be pinned before R2-01.

## Configure and build

Open an x64 Native Tools Command Prompt for Visual Studio 2022, then run:

```powershell
./scripts/verify-project-layout.ps1
./scripts/bootstrap-workspace.ps1
./scripts/verify-dependencies.ps1
./scripts/configure.ps1
./scripts/build.ps1
```

`bootstrap-workspace.ps1` creates only the project-owned shared locations `../libmpv/windows-x64`, `../downloads`, and `../cache/cmake/fetchcontent`. It does not create or modify the installed `../Qt`, `../cmake`, or `../ninja` directories, and it does not download packages.

The active MSVC target must be x64. `scripts/verify-dependencies.ps1` rejects an x86 developer shell before CMake configuration, because a 32-bit compiler cannot consume the Qt MSVC 2022 64-bit kit.

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
git status
```

## Module growth rule

A file may receive new code only when the code has the same responsibility and reason to change. When an existing file gains a separately evolving responsibility, convert it to a responsibility directory and place the original and new files inside that directory. Do not create source-history copies with names such as `Old`, `New`, `V2`, `Final`, or `Copy`; Git owns history.

## Documentation

- Full execution plan: `docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md`
- Render API decision: `docs/decisions/ADR-0001-libmpv-render-api.md`
- OpenGL decision: `docs/decisions/ADR-0002-opengl-first.md`
- Playback-state ownership: `docs/decisions/ADR-0003-single-playback-owner.md`
- QML boundaries: `docs/decisions/ADR-0004-qml-boundaries.md`
- Media fixture policy: `docs/testing/media-fixture-policy.md`

## Validation record

Validated in the generation environment:

- CMake preset JSON is syntactically valid.
- Top-level CMake parsing reaches the Qt dependency-resolution boundary without an earlier syntax or module-reference failure.
- The repository contains no build output, runtime database, logs, or IDE state.
- CMake module and source references resolve to files inside the repository.
- QML root, shell, screen, feature, and theme files are present with distinct responsibilities.
- The R0-02 README scope matches the task book's mandatory, deferred, and excluded capability lists.
- Every mandatory MVP capability group has an observable acceptance condition.

Not executed in the generation environment:

- Qt configure and compilation, because Qt 6 is not installed in the container.
- QML runtime launch, for the same reason.
- libmpv verification, because libmpv integration is not part of this framework commit.
- Runtime MVP acceptance, because R0-02 defines scope and does not implement playback capabilities.

The first local action after cloning is to place the Qt MSVC kit under `../Qt/<version>/msvc2022_64`, then run `scripts/verify-dependencies.ps1`, `scripts/configure.ps1`, and `scripts/build.ps1` from an MSVC developer shell.

## Change Log

### 2026-08-07

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