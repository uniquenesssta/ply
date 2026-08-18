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
| R7 — 媒体打开与播放列表 | Complete | **R7-01 ~ R7-13 Complete**。R7-13 已在 Windows 环境完成 configure/build、Quick **93/93 PASS** 与 Full **101/101 PASS（83.69 s）**。剩余 R7-14 Queue UI 状态解耦与 UI/Figma 视觉打磨按用户决定推迟到软件功能完成后统一处理 |
| R8 — 音轨、字幕、章节 | Complete | **R8-01 ~ R8-08 Complete**。Windows 环境 configure/build、Quick **96/96 PASS** 与 Full **104/104 PASS（82.92 s）**；windowed-render 42.36 s、startup-smoke 5.95 s。补强矩阵 R8-09~12 边界已随实现落实 |

R0/R1 属于既有项目基线。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，不阻断后续 Stage。`成熟播放器行为补强与验收矩阵.md` 是跨 Stage 强制补充基线。

## Technical baseline

- Qt **6.8.3** / C++20 / CMake / Ninja。
- Windows 首发工具链：MSVC 2022 x64。
- 固定 libmpv **0.41.0** sibling package。
- Qt Quick 图形后端固定 OpenGL；视频使用 libmpv OpenGL Render API + `QQuickFramebufferObject`。
- QML 不直接调用 libmpv。
- `PlaybackSession` 是播放状态与媒体代际的唯一权威 owner。
- `MediaOpenCoordinator` 是媒体打开 operation identity / supersession 的唯一 owner；异步 worker 只携带 operation id 与解析结果，结果回到 Coordinator 时必须再次校验 identity，stale result 不得提交 Playlist/Playback，也不得覆盖新 operation 的错误状态。
- Playlist Domain 是队列顺序/current/repeat/shuffle 的唯一 owner；`PlaylistSnapshot` 只提供从该权威状态生成的脱离式只读观测，`currentIndex` 由稳定 `EntryId + order` 推导，不成为第二真值；`PlaylistShuffleState` 只拥有该 Domain 内的 shuffle cycle 状态；QML 只消费 readonly model 并通过 Controller 发 intent。
- `PlaylistAdvanceArbiter` 只拥有当前已观测 MediaGeneration 的 manual/terminal advance 竞争门禁，不拥有 queue/current，也不创建第二套 Playback generation。
- `ThemeMode` 是当前运行期 Light/Dark 模式的唯一 presentation owner；`ColorTokens` / `MaterialTokens` 统一从它解析，Feature 不拥有私有暗色主题。
- Renderer 只拥有 Render/OpenGL 资源，不拥有播放业务状态。
- Track/Chapter 真值继续归 `PlaybackSnapshot`；`TrackListModel` / `ChapterListModel` 是只读投影，行身份使用 backend 稳定 track id，不用视觉 index 作命令参数；`TrackSelectionController` / `SubtitleDelayController` / `AudioDelayController` / `ExternalSubtitleLoader` 只携带 intent，不建立第二套轨道状态。
- 外挂字幕成功加载后必须进入同一 `TrackDescriptor` 模型（`sub-add select`），QML 不维护额外字幕列表；重复路径由 `ExternalSubtitleLoader` 幂等拒绝。
- 章节点击只产生统一 `SeekCommand`（`PlayerTimelineViewModel::requestAbsoluteSeek`），Timeline 只从 Snapshot position 更新，chapter marker 是只读投影。
- Persistence/Settings 真值继续按 R9 任务书归 `SettingsController + repository`，当前 R7/R8 不建立第二套设置持久化路径。

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
src/media/application/           媒体打开校验、operation supersession 与统一 workflow 编排
src/playlist/domain/             播放队列、Entry/current/repeat/shuffle cycle、Queue Snapshot 与导航策略
src/playlist/application/        Playlist Controller、mutation、advance arbitration、auto-advance 与 load 编排
src/playlist/presentation/       Playlist 只读模型投影
src/tracks/application/          轨道选择、外挂字幕、字幕/音频延迟 intent 控制器
src/tracks/presentation/         Track/Chapter 只读模型投影
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

### 2026-08-18 — Stage R8 Tracks / Subtitles / Chapters Complete

- R8-01/R8-07 的 mpv 侧 track-list/chapter-list 解码、事件与 Snapshot 状态来自 R3/R4 既有主链；本阶段补齐上层：领域命令、只读模型、intent 控制器与 QML 特征。
- 新增 Domain 命令：`SelectTrackCommand`（kind + backend 稳定 id，id≤0 表示 off）、`SetSubtitleDelayCommand`、`SetAudioDelayCommand`、`LoadExternalSubtitleCommand`，全部进入统一 `PlaybackCommandPayload` 校验链与 `MpvCommandEncoder`（`set vid/aid/sid`、`set sub-delay/audio-delay`、`sub-add <path> select`）。
- 属性注册表新增 `sub-delay`/`audio-delay` 观察（observationId 2023/2024），事件 `SubtitleDelayChangedEvent`/`AudioDelayChangedEvent` 进入 reducer，延迟值归 `PlaybackControlsState`；`kMediaRefreshProperties` 随媒体刷新读取。
- RequestTracker 扩展 `SetSubtitleDelay`/`SetAudioDelay`/`LoadExternalSubtitle` 类型；字幕延迟与音频延迟各自独立 supersession lane，同类后请求 supersede 前请求（同 generation），外挂字幕不参与 supersede（幂等由 loader 负责）。
- 新增 `src/tracks/application`：`TrackSelectionController`（能力门控 + intent 转发）、`SubtitleDelayController`/`AudioDelayController`（固定 ±0.5s 步进、±10s 范围、reset、pending 由 Snapshot 真值清除）、`ExternalSubtitleLoader`（扩展名校验、文件可读校验、与已加载 track-list 的 canonical path 幂等去重）。
- 新增 `src/tracks/presentation`：`TrackListModel`（按 kind 过滤的只读投影，行身份 = backend track id，selectedRow 由 selected 标志推导）与 `ChapterListModel`（index/title/startSeconds，无标题用 Chapter N fallback）；两者都从 `PlaybackSnapshot` 整表替换，媒体切换自动清空。
- `PlaybackComposition` 新增 `submitTrackSelection/submitSubtitleDelay/submitAudioDelay/submitExternalSubtitle`，统一走 PlaybackCommandBus；Audio/Subtitle/Video 三个 `TrackListModel`、`ChapterListModel` 与四个 intent 控制器接入 `StatePublisher::snapshotPublished`，并通过 `ApplicationBootstrap` 注入 QML。
- `PlayerTimelineViewModel` 新增 `requestAbsoluteSeek(seconds)`：clamp 到有效时长后产生统一 `SeekCommand`，供章节跳转复用同一 seek 主链（不直接写 Timeline 状态）。
- QML：`PlayerUtilityControls` 的 `subtitlesUnavailableControl` 占位符替换为真实 Audio/Subtitles/Chapters 三组控制；新增 `features/tracks/SubtitleMenu.qml`（轨道选择/Off/加载外挂/延迟±/Reset）、`features/tracks/AudioMenu.qml`（轨道选择/延迟±/Reset）、`features/chapters/ChapterMenu.qml`（点击 → absolute seek）；新增 `audio.svg`/`chapters.svg` 图标与 IconCatalog 注册。
- 测试：`tracks_application`（selection + delay + loader，含 `tracks_delay_controller` 与 `tracks_track_selection` 两个目标）、`tracks_presentation`（模型投影/整表替换/fallback/章节），扩展 `playback_command`、`mpv_command`、`playback_event_mapping`、`playback_reducer`、`request_tracker`、`request_supersession`、`media_generation`、`mpv_property_baseline/observer`、`player_timeline_view_model`、`icon_pipeline`、`playlist_qml` 契约。Windows 本地验证：configure PASS、build PASS、Quick **96/96 PASS**、Full **104/104 PASS（82.92 s）**，windowed-render 42.36 s、startup-smoke 5.95 s。

### 2026-08-18 — R7-13 Async Media Open Supersession Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 为 `MediaOpenCoordinator` 增加单一、单调的 `MediaOpenOperationId`。每次 replace-open workflow 开始都会生成新 identity 并立即使前一 operation stale；异步解析完成只能通过 `completeOpenSource(s)` 回到 Coordinator，提交前再次校验 identity。stale completion 直接拒绝，不进入 Playlist/Playback submission，也不覆盖较新 operation 的 `lastErrorKey`。
- 现有同步 `openSource/openSources/openLocalFile(s)/openSourceUrls` 继续保持原公共行为，但内部统一走 begin→validate/resolve→complete operation 主链；`UrlOpenWorkflow` 在 URL validation **之前**获取 operation identity，因此新的 URL 请求即使验证失败，也已经使旧异步 open 结果失效。没有让 QML、drawer 或 readonly model 承担取消职责。
- `ApplicationContainer::shutdown()` 在销毁 media workflows/coordinator 前先调用 `MediaOpenCoordinator::beginShutdown()`，永久停止接收新 operation 并使当前 pending identity 失效；之后到达的旧解析结果无法再提交。
- 新增独立 `media_open_supersession` 测试目标，覆盖 A→B supersession、stale single/batch completion、stale result 不污染错误状态、operation single-use、同步 open supersede pending async、URL workflow 在 validation 前 supersede、scoped cancel、shutdown cancel/reject。没有新增生产依赖，也没有实现当前不存在的虚假异步 parser/thread；后续真正的 file/URL/playlist async parser 必须携带此 operation id 并在 Coordinator 线程回交结果。
- 因新增 source-list header 与 CTest target，本候选需要重新 configure；标准 Full 套件从 **100 增至 101**。Windows 验证中发现既有 `player_url_open` 边界契约断言仍锁定旧的 `mediaOpenCoordinator_.openSource(source)` 调用，未随 `700856c` 的 workflow 改动同步；已在本地工作区将其重新锁定为 `beginReplaceOpenOperation()` / `completeOpenSource(operationId, source)` / `cancelOpenOperation(operationId)` 新边界（该测试契约改动暂未提交）。
- Windows 本地验证结果：`configure.ps1` PASS、`build.ps1` PASS（1164 步）、Quick **93/93 PASS（42.17 s）**、Full **101/101 PASS，0 failed（83.69 s）**；`media_open_supersession` 与 `player_url_open` 均 PASS，startup-smoke **3.14 s**，8 项 windowed-render 合计 **41.66 s**。**R7-13 正式 Complete。**

### 2026-08-18 — R7-12 Auto Advance arbitration Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 新增独立 application responsibility `PlaylistAdvanceArbiter`，只对当前已观测 `MediaGeneration` 的 terminal advance 与 manual Next/Previous 做一次性竞争仲裁。Playlist 继续唯一拥有 queue/current/repeat/shuffle，PlaybackSession 继续唯一拥有媒体 generation；Arbiter 不复制任何队列或播放状态。
- 同一已观测 generation 内，manual Next/Previous 与 Ended/Failed 只有第一个有效推进可以取得 claim；新的 generation 被 Snapshot 观测后自动重新武装。低于当前已观测 generation 的 late terminal 直接拒绝，因此旧 A 的 EOF 不会在 B 已开始后再次推进。
- `PlaybackComposition` 仅增加 accepted Load/Stop supersession observer：LoadMedia 或 Stop 成功进入现有 PlaybackCommandBus 后通知 Arbiter 抑制当前已观测 generation，覆盖“手动选 B / Next 已排队，但 PlaybackSession 尚未来得及分配 B generation，此时 A late EOF 到达”的 command-queue 窗口。Play/Pause/Seek/Volume/Mute 不进入该门禁。
- `PlaylistController::nextEntry()/previousEntry()` 复用同一 Arbiter；manual load 即时拒绝会释放该 manual claim，使同 generation 后续真实 EOF 仍可执行正常 Auto Advance；accepted load 则由上述 observer 固化 claim。Repeat One/All、Shuffle cycle、R7-09 current-removal UX 与现有 navigation policy 均未改写。
- 扩展 `playlist_auto_advance` 既有测试覆盖两种竞态顺序：manual Next→late EOF、EOF→manual Next；同时覆盖 rapid Next×10 每 generation 至多一次推进、Stop suppression、stale generation、新 generation re-arm、manual load rejection release，以及既有 Repeat/Shuffle/Failure 去重回归。没有新增生产依赖，也没有新增 CTest target。
- 用户随后在 HEAD `eea42d39ab62a219ed0c80aa2381cb1f5fbac36a` 的 Windows 锁定环境完成重新 configure 与 build，并执行完整 `scripts\test.ps1 -SkipBuild`：**100/100 PASS，0 failed，79.64 s**；`playlist_mutation_semantics` PASS（0.12 s），`playlist_auto_advance` PASS（0.12 s），startup-smoke **3.02 s**，8 项 windowed-render 合计 **38.56 s**。该日志未包含 Quick 单独运行，但 Full 已覆盖正式收口门禁。**R7-12 正式 Complete。**

### 2026-08-18 — R7-11 Queue Mutation semantics Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 补齐 Queue Mutation 明确语义：纯 queue append/insert 保持 current EntryId 且不触发 Playback load；remove/reorder 延续既有稳定身份策略；clear 在存在 current 时必须先由既有 Stop callback 接受，拒绝则 queue/current 不变；Repeat/Shuffle mode 切换只修改所属 Domain 模式状态，不改变 ordered entries 或 current identity。既有 `openSource/openSources` 的“追加后选择并加载本次首项”媒体打开行为保持不变。
- 新增独立 Domain 值对象 `PlaylistReplacement` 作为**脱离式、不可写的 prepared replacement**：先验证全部来源并预分配新的单调 EntryId，但不消耗权威 ID allocator、不改变旧 queue/current/shuffle；`PlaylistController::replaceSources()` 只有在目标首项 load 被现有 command submission 链即时接受后才一次提交 replacement。即时 load rejection 因此保留旧 queue/current，同时不会跳过未提交的 EntryId。替换成功后 current 明确指向新队列第一项，Repeat/Shuffle 配置保持，shuffle cycle 重新开始。
- `PlaylistNavigation` 新增显式 manual Next/Previous policy。顺序模式严格按稳定 EntryId 所在 order 推进；Repeat All 允许首尾回绕；Repeat One 只影响自然 EOF，不把用户手动 Next/Previous 变成 current reload。Shuffle manual Next 使用 `previewNext + commitNextSelection` 两阶段语义，load rejection 不初始化/消费 shuffle cycle；现阶段没有权威 shuffle history，因此 manual Previous 与 `CanPrevious` 继续保持不可用，统一竞态门禁归 R7-12。
- `PlaylistMutation` / `PlaylistController` 新增职责内的 append、insert、replace、clear、manual next/previous、repeat/shuffle intent；QML、libmpv、PlaybackSession、R7-09 current-removal UX 均未修改，也没有新增生产依赖。新增独立 `playlist_mutation_semantics` 测试目标，并扩展 `playlist_domain`、`playlist_navigation`、`playlist_shuffle_state` 现有测试，覆盖 prepared replacement 脱离性、ID 不消耗、insert/current 稳定、clear Stop rejection、manual navigation load rejection、Shuffle preview/commit、mode identity 等契约。
- 用户随后在 HEAD `b9ad1573b69bcfda09223e286f8e2f00c0026983` 的 Windows 锁定环境完成重新 configure 与 build，并执行完整 `scripts\test.ps1 -SkipBuild`：**100/100 PASS，0 failed，79.20 s**；`playlist_mutation_semantics` PASS（0.13 s），startup-smoke **3.01 s**，8 项 windowed-render 合计 **38.49 s**。该日志未包含 Quick 单独运行，但 Full 已覆盖正式收口门禁。**R7-11 正式 Complete。**

### 2026-08-18 — R7-10 Queue Snapshot Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 新增独立 Domain 值对象 `PlaylistSnapshot`，一次性复制 ordered entries、稳定 `EntryId`、current `EntryId`、由 `EntryId + order` 推导的 0-based current index、Repeat mode、Shuffle mode 以及当前 shuffle cycle 的只读 bookkeeping。Playlist 仍是唯一可写 owner，Snapshot 不持有 mutation API，也不建立第二套 queue/current 真值。
- `PlaylistShuffleState` 只新增 cycle initialized 与 remaining EntryId 的 const 读取，用于构造 Snapshot；shuffle cycle 的创建、消费与重置仍由原 Domain 状态机负责。`PlaylistNavigation::capabilities(snapshot)` 从同一快照推导 `CanNext / CanPrevious`：顺序模式尊重队列边界与 Repeat All；现有 shuffle 仅有 forward cycle bag，因此在建立权威 shuffle Previous history 前不虚假宣称 shuffle Previous 可用。
- `PlaylistController` 保留既有 `playlist()` const accessor 兼容路径，同时新增 `snapshot()`；`PlaylistListModel::refresh()` 改为每次只消费一个脱离式 Snapshot，再从其 current index/current ID 投影现有 1-based `currentPosition` 与 row `current`，避免一次刷新期间分别读取多份可变队列状态。
- 新增独立 `playlist_snapshot` 单元测试目标，覆盖空队列、稳定 ID 与 reorder/current-removal 后 current index 推导、Snapshot 脱离性、shuffle bookkeeping、顺序/Repeat All/Shuffle navigation capability。用户随后在 HEAD `3649332002502cd95b36872e6c1f19a99be3e86a` 的 Windows 锁定环境完成重新 configure 与 build，并执行完整回归：**99/99 PASS，0 failed，79.44 s**；其中 `playlist_domain`、`playlist_navigation`、`playlist_shuffle_state`、`playlist_snapshot`、`playlist_application`、`playlist_auto_advance`、`playlist_list_model` 均 PASS，startup-smoke 与 8 项 windowed-render 回归同时通过。**R7-10 正式 Complete。**

### 2026-08-18 — R7-09 Complete

- 用户已冻结 R7-09 删除 current 的 UX：队列仅 1 项时先提交 Stop/Unload，再删除并令 `current=null`；删除中间 current 时切到下一项；删除最后一项 current 时切到前一项；目标媒体 load 的即时提交失败时删除整体不提交并保留原 queue/current。为保持同一原子语义，单项 Stop 的即时提交失败也不修改 queue/current。
- `PlaylistNavigation::forCurrentRemoval()` 集中计算上述 next/previous/stop 决策；`Playlist::removeCurrentAndSelect()` 在 Playlist Domain 内原子删除 current 并切换稳定 EntryId，避免任何对外可见的 dangling current。Repeat/Shuffle 不改变用户显式删除 current 时的 next/previous UX；shuffle bookkeeping 随实际 remove/select 同步更新。
- `PlaylistController::removeEntry()` 现在对 current 走协调链：replacement load 或 Stop 必须先被现有 PlaybackCommandBus 接受，之后才提交 Playlist mutation；即时 command submission rejection 不修改 queue/current。非 current 删除和 reorder 语义保持不变。
- `PlaybackComposition` 新增返回提交结果的 `submitMediaStop()`，ApplicationContainer 只把该应用级 Stop callback 注入 PlaylistController；QML 继续只调用现有 `removeEntry()`，没有新增 QML→Playback/mpv 旁路，也没有新增生产依赖。
- 扩展 `playlist_domain`、`playlist_navigation`、`playlist_application` 现有测试覆盖：中间 current→next、尾项 current→previous、单项 current→Stop+empty、replacement load rejection、Stop rejection、原子 current replacement 与非 current 回归。用户随后在 HEAD `99ec6363956425aa55a4b2df0531e13cfc08c89a` 的 Windows 锁定环境完成 `scripts\build.ps1`，并执行完整 `scripts\test.ps1 -SkipBuild`：**98/98 PASS，0 failed，79.52 s**；Playlist Domain/Navigation/Shuffle/Application/AutoAdvance 与其余回归均通过。**R7-09 正式 Complete。**

### 2026-08-18 — R7-08 Complete / R7-09 policy pending

- R7-07 已按用户此前 Windows 验证收口：**95/95 测试通过**，`Ctrl+Shift+D` 全局 Light/Dark 快捷键可用；剩余 Playlist/Figma 视觉打磨不阻塞当前功能阶段。
- R7-08 已建立独立 `PlaylistNavigation` domain policy 与 `PlaylistAutoAdvance` application workflow；Playlist Domain 继续唯一拥有 queue/current/repeat/shuffle，AutoAdvance 只消费 PlaybackSnapshot 的终态投影并通过既有 `PlaylistController` 提交下一媒体，不建立第二套 queue 或 Playback owner。
- normal 模式按实际 queue 顺序推进；尾项 + Repeat Off 停留；Repeat All 尾项回到第一项；Repeat One 与单项 Repeat All 复用现有 load callback 重载 current。同一 MediaGeneration 的终态只允许一次自动提交，避免 late/repeated Ended 造成 double load。
- Shuffle 使用独立 `PlaylistShuffleState` cycle bag；Repeat Off 遍历当前 cycle 后停止，Repeat All 在自然 EOF 时可建立新 cycle 且排除刚结束项；成功 selection 才消费候选，因此即时 load submission rejection 不改变 current 或 shuffle history。
- 用户最新 Windows 锁定环境验证确认 shuffle 候选：`configure.ps1` PASS、`build.ps1` PASS、完整 `scripts\test.ps1 -SkipBuild` **98/98 PASS，0 failed，94.96 s**；`playlist_navigation`、`playlist_shuffle_state`、`playlist_auto_advance` 均实际通过。
- 复核根任务书发现 R7-08 验收还明确包含“错误跳过”，因此 R7-08 不能仅凭上述 98/98 提前关闭。本候选新增 `PlaybackLifecycleState::Failed → PlaylistAutoAdvance::acceptPlaybackFailure()` 链，并把 Ended/Failed 统一到同一 generation terminal 去重门禁。
- Failed 媒体采用 fail-forward 策略：normal 只向后跳过，不因 Repeat One 重试失败 current，也不因 Repeat All 在队尾回绕；shuffle 只消费当前未完成 cycle，不在 failure 路径重启 Repeat All cycle。这样连续损坏媒体会向后收敛而不是形成无限自动重载环。
- 扩展 `playlist_navigation` 与 `playlist_auto_advance` 测试覆盖 Failed、Repeat One/All、shuffle failure cycle、同 generation Ended/Failed 双终态去重。没有新增生产依赖，没有修改 libmpv/Render/QML UI。最终 error-skip 候选已由用户在 Windows 锁定环境验证：`configure.ps1` PASS、`build.ps1` PASS、完整 `scripts\test.ps1 -SkipBuild` **98/98 PASS，0 failed，77.61 s**；其中 `playlist_navigation`、`playlist_shuffle_state`、`playlist_auto_advance` 均 PASS。**R7-08 正式 Complete。**
- R7-09 按强制补充矩阵必须在实现前冻结删除 current 的 UX：仅一项时 stop/unload + current=null；中间项删除后选 next 还是 previous；最后一项删除后选 previous 还是 stop；以及 load 失败后的 current identity 语义。该策略随后已由用户确认，并在上方 R7-09 候选中实施。

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
