# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器。播放核心使用 libmpv，UI 使用 Qt 6 / Qt Quick / QML，核心代码采用 C++20，构建系统为 CMake + Ninja。

README 只维护**项目入口、当前状态、关键架构边界和简短变更记录**。Atomic Task 的目标、实现边界、验收矩阵与详细结果统一查看 `docs/plans/stages/`；README 不再重复完整实施过程。

## Current status

| Stage | 状态 | 结果 |
|---|---|---|
| R2 — 无 UI libmpv 播放核心 | Complete | 真实 load/play/pause/seek/stop、事件/属性/命令与 headless probe 主链完成 |
| R3 — 领域状态与 PlaybackSession | Complete | PlaybackSnapshot、Reducer、Generation、RequestTracker、Supersession、Session 生命周期完成 |
| R4 — libmpv OpenGL Render API | Complete | 视频进入 Qt Quick，Render 生命周期、DPI/visibility/shutdown 与 1080p/4K 基线完成 |
| R5 — UI 设计系统 | In Progress | R5-01 Windows configure/build、无 warning `player_qml_lint` 与 42/42 CTest 已 PASS；仅剩 `Player.exe` 启动 smoke 后正式关闭 |

R0/R1 属于现有项目基线，R2–R14 快速任务书不重新定义其历史状态。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，仅在明确调用时执行，不阻断 R5。

当前保留的已知非阻塞事项：R2-01 的仓库根 `player.log` 落盘缺口仍未关闭。

## Technical baseline

- Qt **6.8.3** / C++20 / CMake / Ninja。
- Windows 首发工具链：MSVC 2022 x64。
- 固定 libmpv **0.41.0** sibling package。
- Qt Quick 图形后端固定 OpenGL；视频使用 libmpv OpenGL Render API + `QQuickFramebufferObject`。
- QML 不直接调用 libmpv。
- `PlaybackSession` 是播放状态与媒体代际的唯一权威 owner。
- libmpv 字符串命令、属性、事件和 C API 类型限制在 `src/playback/infrastructure/mpv/`。
- Renderer 只拥有 Render/OpenGL 资源，不拥有播放业务状态。
- Playlist、Persistence、Platform、Presentation 各自保持独立职责边界。

## Core architecture

```text
QML / Presentation
        ↓ intent / read-only state
Application / PlaybackSession
        ↓ PlaybackCommand / PlaybackEvent
Playback Domain
        ↓
Infrastructure / libmpv
        ↓
libmpv

libmpv redraw callback
        ↓
Render update bridge
        ↓
MpvVideoItem / MpvVideoRenderer
        ↓
Qt Quick Scene Graph
```

核心目录职责：

```text
src/app/                         应用启动、composition、顶层生命周期
src/foundation/                  通用基础设施与稳定值类型
src/playback/domain/             后端无关的播放命令、事件、状态与规则
src/playback/application/        PlaybackSession、请求生命周期与应用编排
src/playback/infrastructure/mpv/ libmpv client/event/property/command/render 适配
src/presentation/                QML 与 presentation 层
src/platform/                    平台能力
src/persistence/                 持久化边界
```

## Taskbooks

执行开发任务时以以下文档为准：

- [完整模块化开发任务书](docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md)
- [R2–R14 Stage 索引](docs/plans/stages/00_INDEX.md)
- [成熟播放器行为补强与验收矩阵](docs/plans/stages/成熟播放器行为补强与验收矩阵.md)
- [R2 — 无 UI libmpv 播放核心](docs/plans/stages/R2_无%20UI%20libmpv%20播放核心.md)
- [R3 — 领域状态与 PlaybackSession](docs/plans/stages/R3_领域状态与%20PlaybackSession.md)
- [R4 — libmpv OpenGL Render API](docs/plans/stages/R4_libmpv%20OpenGL%20Render%20API.md)
- [R5 — UI 设计系统](docs/plans/stages/R5_UI%20设计系统.md)
- [R4 后置 — PlaybackSession 职责边界优化](docs/plans/stages/R4后置_PlaybackSession职责边界优化.md)

R6–R14 的任务书继续由 [Stage 索引](docs/plans/stages/00_INDEX.md) 统一导航。

## Build and test

Windows 开发构建使用项目脚本：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

Release 构建：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1 -Preset windows-msvc-release
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Preset windows-msvc-release
```

不得把未执行、被阻塞或失败的验证描述为通过；具体 Stage 的验收数字记录在对应 Stage 文档中。

## Change log

### 2026-08-13 — R5-01 QML type metadata

- 显式 tooling typeinfo 最终候选已在锁定 Windows 环境验证：configure **PASS**、Debug build **PASS**、development runtime deployment **PASS**、`player_qml_lint` **无 warning**、全量 **42/42 CTest PASS**。
- `MpvVideoItem was not found`、anchors/objectName 派生 warning 以及 export/meta-object revision warning 均已关闭；`exportMetaObjectRevisions` 使用 Qt `QTypeRevision` 的 1.0 编码值 `256`。
- 运行时仍使用已验证的 `qmlRegisterType<MpvVideoItem>()`；未修改 `MpvVideoItem`、Renderer、Playback/libmpv、公共播放接口或视觉 token，也未新增生产依赖。
- R5-01 当前仅剩一次 `Player.exe` 实际启动 smoke；确认窗口正常创建且无 QML root/type registration 错误后即可标 Complete。

### 2026-08-13 — R5-01 validation fixes

- Windows Qt 6.8.3 / MSVC 已确认 configure 与 build 通过；早期 `qmltyperegistrar` Generate 阻断已消失。
- `qml_module_boundaries` 的 `QtQuick` import path 已通过测试脚本显式注入 `<Qt>/qml` 修复，并在测试后恢复原环境。
- 全量回归捕获的 R4-08 late-update 竞态已通过 consumer-side shutdown gate 修复；公共接口和播放状态所有权不变。
- 修复后 Windows 全量回归已达到 **42/42 PASS**；后续仅继续治理 qmllint 的 `MpvVideoItem` tooling metadata。

### 2026-08-12 — R5-01

- 建立 `Theme/Primitives/Controls/Surfaces` 四个公开 QML URI，现有 Theme 使用点改为显式模块 import。
- `Player.Presentation` 保留 App 公共入口，其余现有 Shell/Screen/Feature 类型收为 internal；播放接口、依赖版本和视觉 token 不变。
- Windows configure 暴露 Qt 6.8.3 TARGET-based QML dependency 的 deferred `qmltyperegistrar` 生成失败；已改为 URI dependency 并保留显式 backing-target 链接。
- 新增 QML public-module 最小加载测试；R5-01 暂不标 Complete。详细记录见 R5 Stage 文档。

### 2026-08-11 — README documentation policy

- README 收敛为项目入口、当前状态、架构边界和简短变更记录；不再复制 Atomic Task 的完整实施过程。
- 详细任务、实现边界、测试矩阵与阶段验收记录由 `docs/plans/stages/` 承担；历史 README 内容仍保留在 Git 历史中。
- 文档职责调整，不改变源码、公共接口、配置、依赖或运行行为。

### 2026-08-11 — R4-09 / Stage R4

- 修复默认 libmpv Render 模式下错误的 update gate，解决 1080p windowed 稳态停帧。
- Windows Debug：**41/41 CTest PASS，0 failed，58.78 s**。
- Release probe：1080p/4K 的 windowed/fullscreen 均约 **300 frames / 10 s**，新增 frame/decoder/delayed drop 均为 **0**。
- **R4-01~R4-09 Complete；Stage R4 Complete。** 详细数据见 R4 Stage 文档。

### Stage R3

- **R3-01~R3-12 Complete；Stage R3 Complete。**
- 建立后端无关播放领域状态、PlaybackSession 单一真值、MediaGeneration stale gate、RequestTracker、cleanup matrix 与 supersession/cancellation。

### Stage R2

- **R2-01~R2-11 完成 Stage 主链；Stage R2 Complete。**
- 建立固定 libmpv 运行链、RAII client、初始化、命令、属性、事件、错误映射与 headless playback probe。
- R2-01 的根 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留。
