# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器。播放核心使用 libmpv，UI 使用 Qt 6 / Qt Quick/QML，核心代码采用 C++20，构建系统为 CMake + Ninja。

README 只维护**项目入口、当前状态、关键架构边界和简短变更记录**。Atomic Task 的目标、实现边界、验收矩阵与详细结果统一查看 `docs/plans/stages/`；更早的逐步实施记录保留在 Git 历史中。

## Current status

| Stage | 状态 | 结果 |
|---|---|---|
| R2 — 无 UI libmpv 播放核心 | Complete | 真实 load/play/pause/seek/stop、事件/属性/命令与 headless probe 主链完成 |
| R3 — 领域状态与 PlaybackSession | Complete | PlaybackSnapshot、Reducer、Generation、RequestTracker、Supersession、Session 生命周期完成 |
| R4 — libmpv OpenGL Render API | Complete | 视频进入 Qt Quick，Render 生命周期、DPI/visibility/shutdown 与 1080p/4K 基线完成 |
| R5 — UI 设计系统 | Complete | R5-01 ~ R5-10 Complete；最终 Windows Debug build、QML lint、51/51 CTest 与 startup smoke 已验证 |
| R6 — 播放器主界面与基础交互 | Complete | R6-01 ~ R6-16 Complete；最终 Windows Debug build、QML lint、76/76 CTest 与 startup/exit smoke 已验证 |
| R7 — 媒体打开与播放列表 | In Progress | **R7-01 ~ R7-08 Complete**。R7-08 最终 error-skip 候选已在 Windows 完成 configure/build 与 Full **98/98 PASS（77.61 s）**，EOF、normal/repeat、shuffle、Failed fail-forward 与同 generation 终态去重均保持通过。R7-09 尚未实施，当前需先冻结删除 current 的 UX 策略；剩余 UI/Figma 视觉打磨按用户决定推迟到软件功能完成后统一处理 |

R0/R1 属于既有项目基线。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，不阻断后续 Stage。`成熟播放器行为补强与验收矩阵.md` 是跨 Stage 强制补充基线。

## Technical baseline

- Qt **6.8.3** / C++20 / CMake / Ninja。
- Windows 首发工具链：MSVC 2022 x64。
- 固定 libmpv **0.41.0** sibling package。
- Qt Quick 图形后端固定 OpenGL；视频使用 libmpv OpenGL Render API + `QQuickFramebufferObject`。
- QML 不直接调用 libmpv。
- `PlaybackSession` 是播放状态与媒体代际的唯一权威 owner。
- Playlist Domain 是队列顺序/current/repeat/shuffle 的唯一 owner；`PlaylistShuffleState` 只拥有该 Domain 内的 shuffle cycle 状态；QML 只消费 readonly model 并通过 Controller 发 intent。
- `ThemeMode` 是当前运行期 Light/Dark 模式的唯一 presentation owner；`ColorTokens` / `MaterialTokens` 统一从它解析，Feature 不拥有私有暗色主题。
- Renderer 只拥有 Render/OpenGL 资源，不拥有播放业务状态。
- Persistence/Settings 真值继续按 R9 任务书归 `SettingsController + repository`，当前 R7 不建立第二套设置持久化路径。

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
src/media/domain/                规范化媒体来源值对象
src/media/application/           媒体打开校验与统一 workflow 编排
src/playlist/domain/             播放队列、Entry/current/repeat/shuffle cycle 与导航策略
src/playlist/application/        Playlist Controller、mutation、auto-advance 与 load 编排
src/playlist/presentation/       Playlist 只读模型投影
src/presentation/                QML、Theme、Surfaces 与 presentation 层
src/platform/                    平台能力
src/persistence/                 持久化边界
```

## Taskbooks

执行开发任务时以以下文档为准：

- [完整模块化开发任务书](docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md)
- [R2–R14 Stage 索引](docs/plans/stages/00_INDEX.md)
- [成熟播放器行为补强与验收矩阵](docs/plans/stages/成熟播放器行为补强与验收矩阵.md)
- [R5 — UI 设计系统](docs/plans/stages/R5_UI%20设计系统.md)
- [R6 — 播放器主界面与基础交互](docs/plans/stages/R6_播放器主界面与基础交互.md)
- [R7 — 媒体打开与播放列表](docs/plans/stages/R7_媒体打开与播放列表.md)
- [R8 — 音轨、字幕、章节](docs/plans/stages/R8_音轨、字幕、章节.md)
- [R9 — 历史、恢复播放与设置](docs/plans/stages/R9_历史、恢复播放与设置.md)

第三版 Figma/UI 实施以 `agent/ui-v3-design-plan` 中的 `Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md` 和实际 Figma canonical nodes 为视觉真值；Figma 页面结构不替代产品模块职责边界。

## Build and test

Windows 开发构建：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

日常迭代可使用：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Quick
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Quick -SkipBuild
```

`-Quick` 会排除真实 `windowed-render` 回归，不能替代 Atomic Task / Stage 收口或发布前完整验证。不得把未执行、被阻塞或失败的验证描述为通过。

## Change log

### 2026-08-18 — R7-08 Complete / R7-09 policy pending

- R7-07 已按用户此前 Windows 验证收口：**95/95 测试通过**，`Ctrl+Shift+D` 全局 Light/Dark 快捷键可用；剩余 Playlist/Figma 视觉打磨不阻塞当前功能阶段。
- R7-08 已建立独立 `PlaylistNavigation` domain policy 与 `PlaylistAutoAdvance` application workflow；Playlist Domain 继续唯一拥有 queue/current/repeat/shuffle，AutoAdvance 只消费 PlaybackSnapshot 的终态投影并通过既有 `PlaylistController` 提交下一媒体，不建立第二套 queue 或 Playback owner。
- normal 模式按实际 queue 顺序推进；尾项 + Repeat Off 停留；Repeat All 尾项回到第一项；Repeat One 与单项 Repeat All 复用现有 load callback 重载 current。同一 MediaGeneration 的终态只允许一次自动提交，避免 late/repeated Ended 造成 double load。
- Shuffle 使用独立 `PlaylistShuffleState` cycle bag；Repeat Off 遍历当前 cycle 后停止，Repeat All 在自然 EOF 时可建立新 cycle 且排除刚结束项；成功 selection 才消费候选，因此即时 load submission rejection 不改变 current 或 shuffle history。
- 用户最新 Windows 锁定环境验证确认 shuffle 候选：`configure.ps1` PASS、`build.ps1` PASS、完整 `scripts\test.ps1 -SkipBuild` **98/98 PASS，0 failed，94.96 s**；`playlist_navigation`、`playlist_shuffle_state`、`playlist_auto_advance` 均实际通过。
- 复核根任务书发现 R7-08 验收还明确包含“错误跳过”，因此 R7-08 不能仅凭上述 98/98 提前关闭。本候选新增 `PlaybackLifecycleState::Failed → PlaylistAutoAdvance::acceptPlaybackFailure()` 链，并把 Ended/Failed 统一到同一 generation terminal 去重门禁。
- Failed 媒体采用 fail-forward 策略：normal 只向后跳过，不因 Repeat One 重试失败 current，也不因 Repeat All 在队尾回绕；shuffle 只消费当前未完成 cycle，不在 failure 路径重启 Repeat All cycle。这样连续损坏媒体会向后收敛而不是形成无限自动重载环。
- 扩展 `playlist_navigation` 与 `playlist_auto_advance` 测试覆盖 Failed、Repeat One/All、shuffle failure cycle、同 generation Ended/Failed 双终态去重。没有新增生产依赖，没有修改 libmpv/Render/QML UI。最终 error-skip 候选已由用户在 Windows 锁定环境验证：`configure.ps1` PASS、`build.ps1` PASS、完整 `scripts\test.ps1 -SkipBuild` **98/98 PASS，0 failed，77.61 s**；其中 `playlist_navigation`、`playlist_shuffle_state`、`playlist_auto_advance` 均 PASS。**R7-08 正式 Complete。**
- R7-09 按强制补充矩阵必须在实现前冻结删除 current 的 UX：仅一项时 stop/unload + current=null；中间项删除后选 next 还是 previous；最后一项删除后选 previous 还是 stop；以及 load 失败后的 current identity 语义。该策略尚未由用户确认，因此 R7-09 当前不实施。

### 2026-08-17 — Global Light/Dark runtime theme switch

- 全局主题继续由单一 `ThemeMode` owner 管理；新增 `toggle()` 只在 `lightMode / darkMode` 间切换。Main Player、Fullscreen、Header、OSC、Playlist Inspector、Search、Rows、Footer、文字、图标、Timeline、Selection/Focus 均通过现有 `ColorTokens` / `MaterialTokens` 同步响应，不创建 Playlist 或其他 Feature 私有暗色状态。
- 新增职责独立的 `qml/shell/theme/ThemeModeShortcut.qml`，由 `MainWindow` 组合应用级 `Ctrl+Shift+D` 快捷键，使当前全局 Dark 设计可在真实运行时直接进入和返回 Light；播放器 canonical 几何没有增加临时主题按钮，也没有修改 Playback/Playlist/Render 主链。
- 当前切换只作用于本次运行。主题设置的可见 Preferences UI、`SettingsController` 真值和重启后持久化继续归 R9 Settings；本轮没有用 QSettings/临时 JSON/第二套 repository 提前绕过任务书。
- 后续用户提供的 Windows 验证为 95/95 tests PASS 且快捷键可用；该候选已随 R7-07 正式收口。剩余视觉打磨按用户决定推迟到软件功能完成后处理。

### 2026-08-17 — Canonical Figma UI reimplementation / global theme foundation

- Main Player 按 canonical 1320×700 reference 重建：500×54 Floating Header、880×124 Standard OSC；Playlist Inspector 打开时 OSC 收为 730×124 并左移；Fullscreen 使用 440×50 Header + 828×106 Compact OSC。
- Playlist Inspector 为 368×652 floating overlay，不挤压 Video Viewport；Search 324×42、Row 332×58、Playing Rail 3×32、Footer 324×54。搜索/双击选择/删除非 current/重排继续走 readonly model + `PlaylistController`。
- Inspector 已通过 `VideoViewport → ShaderEffectSource → MultiEffect` 建立真实局部 backdrop capture blur；通用 Surface 没有被改造成任意祖先捕获器。
- `PlayerMediaViewModel` 只从现有 Snapshot 投影真实 title / metadata；Playlist queue 尚无权威 per-entry duration，因此列表第二行继续显示真实 sourceLocation，不虚构 Figma 示例时长。
- Figma semantic system 已建立 Light Mist / Dark 两个全局 mode；Dark 通过共享 Theme tokens覆盖播放器全局，不再存在 `inspectorDark*` 等 Feature-scoped palette。
- R10 负责的 native frameless / hit-test / DWM / Snap 仍未提前实现。

### 2026-08-17 — R7-06 Playlist Controller Complete

- Playlist Domain → Mutation → Controller → readonly Model → QML/MediaOpen 的单一主链已建立；所有生产媒体入口经 `PlaylistController` 再进入 Playback load，没有 QML/libmpv 旁路。
- 批量添加、稳定 Entry ID、选择、非 current 删除和 reorder 已接通；即时 load submission rejection 会回滚 queue/current，current deletion policy 仍留给 R7-09。
- Windows 锁定环境已验证：Debug configure/build PASS，Quick **86/86 PASS**，Full **93/93 CTest PASS（73.31 s）**。R7-06 已正式 Complete。
