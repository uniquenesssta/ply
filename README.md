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

Not executed in the generation environment:

- Qt configure and compilation, because Qt 6 is not installed in the container.
- QML runtime launch, for the same reason.
- libmpv verification, because libmpv integration is not part of this framework commit.

The first local action after cloning is to place the Qt MSVC kit under `../Qt/<version>/msvc2022_64`, then run `scripts/verify-dependencies.ps1`, `scripts/configure.ps1`, and `scripts/build.ps1` from an MSVC developer shell.

## Change Log

### 2026-08-07

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
