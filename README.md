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
| R7 — 媒体打开与播放列表 | Complete | **R7-01 ~ R7-14 Complete**。R7-14 Queue UI 状态解耦已在 Windows 锁定环境完成 configure/build，Quick **94/94 PASS（55.79 s）**、Full **102/102 PASS（82.32 s）**；startup-smoke **5.98 s**，8 项 windowed-render 合计 **42.10 s**。R7 功能阶段正式收口；剩余 UI/Figma 视觉打磨继续按既定计划后置，不属于 R7 功能收口阻塞项 |
| R8 — 音轨、字幕、章节 | In Progress | **R8-01 ~ R8-12 Complete**。R8-12 将 chapter start 的 finite/non-negative/duration-bound 校验收口到 Playback Chapter domain，ChapterModel 只投影已验证行及 `normalizedTime`；Chapter QML 只发统一 absolute seek intent，Timeline marker 只读 model role，不再在 QML 修正越界数据，position 仍只来自 generation-gated Snapshot。Windows build 已完成，Quick **106/106 PASS（49.22 s）**、Full **114/114 PASS（79.19 s）**；startup-smoke **3.06 s**，8 项 windowed-render 合计 **39.29 s**。Stage R8 的多轨/字幕/章节联合真实媒体 smoke 继续作为阶段关闭项 |

R0/R1 属于既有项目基线。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，不阻断后续 Stage。`成熟播放器行为补强与验收矩阵.md` 是跨 Stage 强制补充基线。

## Technical baseline

- Qt **6.8.3** / C++20 / CMake / Ninja。
- Windows 首发工具链：MSVC 2022 x64。
- 固定 libmpv **0.41.0** sibling package；其 Windows 源码构建固定 zlib **1.3.2**，用于 Matroska zlib content-compression 轨道支持并随 audited runtime package 分发。
- Qt Quick 图形后端固定 OpenGL；视频使用 libmpv OpenGL Render API + `QQuickFramebufferObject`。
- QML 不直接调用 libmpv。
- `PlaybackSession` 是播放状态与媒体代际的唯一权威 owner。
- `MediaOpenCoordinator` 是媒体打开 operation identity / supersession 的唯一 owner；异步 worker 只携带 operation id 与解析结果，结果回到 Coordinator 时必须再次校验 identity，stale result 不得提交 Playlist/Playback，也不得覆盖新 operation 的错误状态。
- Playlist Domain 是队列顺序/current/repeat/shuffle 的唯一 owner；`PlaylistSnapshot` 只提供从该权威状态生成的脱离式只读观测，`currentIndex` 由稳定 `EntryId + order` 推导，不成为第二真值；`PlaylistShuffleState` 只拥有该 Domain 内的 shuffle cycle 状态；QML 只消费 readonly model 并通过 Controller 发 intent。
- Playlist row 的 `selected / keyboard focus / hover` 继续属于 QML interaction state；`PlaylistEntryPlaybackState` 只从已 generation-gated 的 `PlaybackSnapshot` 投影 `pendingLoading / unavailable` 到稳定 EntryId，不拥有 queue/current/generation，也不建立第二套播放真值。
- Track 的 raw `mpv_node` 只在 infrastructure 内转换；`MpvTrackListDecoder` 只把已经复制为 Qt value tree 的 `track-list` 映射为 `TrackDescriptor`，稳定 backend track ID 保持为产品身份，PlaybackSnapshot 继续是 track list/selection 的唯一播放真值，上层与 QML 不解析 mpv node。
- `TrackListModel` 只消费已 generation-gated 的 `PlaybackSnapshot` 并按 Audio/Subtitle kind 投影只读行；`selected` 由 Snapshot 的 `selectedAudioId / selectedSubtitleId` 推导，model 不拥有 generation、selection mutation 或第二套 Track 真值。Opening/new-media Snapshot 清空旧轨道后，后续 current-generation Snapshot 整表替换新轨道。
- Chapter 的 raw `chapter-list` node 只在 mpv infrastructure 内转换；`MpvChapterListDecoder` 把 Qt value tree 映射为 `ChapterDescriptor`，保持 backend 数组顺序、重复时间戳与原始长标题，不排序、不去重，并原子拒绝非数值、非有限或负 start。`PlaybackSnapshot::chapters()` 继续是章节列表的唯一播放真值；`PlaybackChapterState::validatedForDuration()` 在每次投影时保序过滤非法索引/start，并在 duration 有效时过滤 `start > duration`，因此 duration 后到或修正不会永久丢失 raw chapter。
- `ChapterModel` 位于独立 `src/chapters/presentation` responsibility，只消费已 generation-gated 的 `PlaybackSnapshot` 并投影只读 `index / title / time / timeText / normalizedTime`；`ChapterDisplayFormatter` 是无标题或空标题 `Chapter N` fallback 的唯一 owner，Opening/new-media Snapshot 清空旧章节。`ChapterNavigationViewModel` 与 Timeline marker 消费同一 duration-validated chapter projection；Navigation 只拥有当前 generation 的 chapter-seek pending target，current chapter 与 Timeline position 始终从 Snapshot position 投影。QML 只发章节 intent，Absolute Seek 进入既有 PlaybackCommand 主链；marker 只读取 `normalizedTime`，不计算、不 clamp backend start，Timeline thumb 不接受章节 UI 直接写入。
- Track selection 只走 `QML intent → TrackSelectionController → PlaybackCommand(RequestId + MediaGeneration) → mpv aid/sid → generation-gated PlaybackSnapshot → TrackListModel`。同类 Audio/Subtitle selection 复用 RequestTracker supersession lane；Subtitle Off 是显式 `sid=no` 状态；UI 不乐观改写 selected，最终 selected 始终由 backend/Snapshot 决定。
- 外挂字幕只走 `QML file-picker intent → ExternalSubtitleLoader → AddExternalSubtitleCommand(RequestId + MediaGeneration) → mpv sub-add cached → 成功回执后立即/50 ms/250 ms 有界读取 track-list/sid → generation-gated PlaybackSnapshot → 既有 TrackListModel`；mpv property observer 仍保留正常增量通知，但不再作为外部字幕成功后的唯一刷新来源。Loader 只接受可读本地 SRT/ASS 并提交规范化路径，以当前 generation 的 pending/accepted/Snapshot external path key 保证幂等；外挂字幕不建立独立列表或 selection 真值，也不占用 Track selection supersession lane。编码检测仍按 R8 后续任务处理。
- 字幕延迟只走 `QML intent → SubtitleDelayController → SetSubtitleDelayCommand(RequestId + MediaGeneration) → mpv sub-delay → property observer/成功回执确定性回读 → generation-gated PlaybackSnapshot.controls.subtitleDelaySeconds → Controller/HUD`。Controller 只拥有 pending target，不乐观改写确认值；同 generation 字幕延迟请求使用独立 latest-wins supersession lane。固定范围 **-2.0 s ~ +2.0 s**、步进 **50 ms**，reset 回到 `0 ms`。
- 音频延迟独立走 `QML intent → AudioDelayController → SetAudioDelayCommand(RequestId + MediaGeneration) → mpv audio-delay → property observer/成功回执确定性回读 → generation-gated PlaybackSnapshot.controls.audioDelaySeconds → Controller/HUD`。Audio/Subtitle Delay 拥有各自 request supersession lane、pending target、Snapshot 字段和 HUD category，互不覆盖；Audio Delay 同样固定 **-2.0 s ~ +2.0 s**、步进 **50 ms**、reset `0 ms`。Figma `Tracks / Audio Delay Control` canonical node 为 `183:670`，QML 只拥有 adjustment UI，committed value 仍归 application state。
- `PlaylistAdvanceArbiter` 只拥有当前已观测 MediaGeneration 的 manual/terminal advance 竞争门禁，不拥有 queue/current，也不创建第二套 Playback generation。
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
src/tracks/application/          Track selection、外挂字幕、字幕/音频延迟 intent/校验/应用级提交边界
src/tracks/presentation/         Audio/Subtitle Track 的 Snapshot 只读列表投影
src/chapters/presentation/       Chapter Snapshot 只读列表投影与章节导航 intent/pending 投影；不拥有 Playback/Timeline 真值
src/media/domain/                规范化媒体来源值对象
src/media/application/           媒体打开校验、operation supersession 与统一 workflow 编排
src/playlist/domain/             播放队列、Entry/current/repeat/shuffle cycle、Queue Snapshot 与导航策略
src/playlist/application/        Playlist Controller、mutation、advance arbitration、auto-advance 与 load 编排
src/playlist/presentation/       Playlist 只读模型、per-entry playback status 投影
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

### 2026-08-21 — R8-12 Chapter/Timeline One-way Collaboration Complete

- `PlaybackChapterState::validatedForDuration()` 现在是 chapter/timeline 跨轴校验边界：无论 duration 事件先后，均从 Snapshot 保存的 raw chapter list 保序投影，过滤负 index、非有限/负 start，并仅在 duration 有效时过滤 `start > duration`；等于 duration 的合法尾端 marker 保留。`MpvChapterListDecoder` 回归同步补齐 infinity/NaN 原子拒绝。
- `ChapterModel` 新增只读 `normalizedTime` role，Inspector 与 Timeline marker 使用同一份已校验行；Timeline QML 直接消费该 role，移除对 `time / duration` 的 `Math.min/Math.max` 修正。`ChapterDisplayFormatter` 统一 Model 与 Navigation 的缺失/空标题 `Chapter N` fallback；章节点击仍只经 `ChapterNavigationViewModel → SeekCommand{Absolute} → PlaybackCommandBus`，不写 Timeline slider，current chapter/position 仍只由 generation-gated Snapshot 确认。
- 扩展 `playback_snapshot`、`mpv_chapter_list_decoder`、`chapter_model`、`chapter_navigation_view_model`、`player_chapters_qml` 与 `player_timeline_controls` 既有回归，覆盖未知/有效/变化 duration、边界等于/超过 duration、非法 start/index、统一 fallback、只读 normalized marker 及 QML 无越界修正。用户在 HEAD `aeeef9e1b75c560679032a0c2e394547b46d0f90` 的 Windows 锁定环境完成 build；Quick **106/106 PASS，0 failed（49.22 s）**，Full **114/114 PASS，0 failed（79.19 s）**，上述六项专项回归在两轮均通过；startup-smoke **3.06 s**，8 项 windowed-render 合计 **39.29 s**。**R8-12 正式 Complete**。无新增依赖或 CTest target；Stage R8 的多轨/字幕/章节联合真实媒体 smoke 继续保留为阶段关闭项，不记为已执行。

### 2026-08-21 — R8-11 External Subtitle Idempotency Complete

- `ExternalSubtitleLoader` 现在只保存当前 MediaGeneration 的 workflow bookkeeping：本地路径规范化为 canonical key，同一路径在 pending、成功回执后或 Snapshot 已存在 external track 时均幂等返回成功且不重复提交；不同 media generation、shutdown 与 stale result 会清理旧 pending/accepted key。该状态不投影字幕行、不替代 `PlaybackSnapshot::tracks()`，QML 仍只消费既有 `TrackDescriptor` 模型。
- 外挂字幕提交现在返回稳定 RequestId；PlaybackSession/Thread/Composition 将完成结果按 `request type + requestId + MediaGeneration` 送回 Loader，使并行路径的 backend/parse failure 只清理对应 pending，并暴露 `backend-rejected`。提交前缺失、不可读和不支持扩展名在本地边界拒绝；校验后文件被删除或 backend parse failure 则由精确异步回执收敛；既有 `sub-add cached` 保留 backend duplicate gate。成功回执后除立即读取 `track-list/sid` 外，再在 **50 ms / 250 ms** 执行两次有界刷新；所有结果携带原 generation，媒体切换后的 stale refresh 继续由既有 gate 丢弃，shutdown 时 timer context 随 backend 一并销毁。
- 扩展既有 `external_subtitle_flow` 回归，覆盖 pending/成功/Snapshot 三态同路径去重、backend 判重成功但未新增 Track 行、不同请求精确失败、文件校验后被删除、unsupported/parse failure、media switch late result、shutdown cancellation、`sub-add cached` 编码和 QML 不建立额外列表。用户在 HEAD `f28020d24a1afd7574ebecda585dcbcc5833c2b3` 的 Windows 锁定环境完成 build；Quick **106/106 PASS，0 failed（42.99 s）**，Full **114/114 PASS，0 failed（81.78 s）**，`external_subtitle_flow` 在两轮均通过；startup-smoke **3.09 s**，8 项 windowed-render 合计 **41.37 s**。**R8-11 正式 Complete**。无新增依赖、QML 文件或 CMake target；Stage R8 的多轨/字幕/章节联合真实媒体 smoke 继续保留为阶段关闭项，不记为已执行。

### 2026-08-21 — R8-10 Track Selection Request Arbitration Complete

- Track selection 继续复用通用 `RequestTracker`：Audio、Subtitle、Video 各自拥有独立 same-kind latest-wins lane；每次提交记录 stable `RequestId + MediaGeneration`，superseded、cancelled、duplicate、unknown 或 stale-generation reply 均不能进入当前 selected 状态。新 media generation 通过统一 generation-change cancellation 清理旧 track-selection request，不新增 Track 专属 request owner。
- 新增 Track 专项仲裁回归，把同 generation Audio A→Audio B supersession、Audio/Subtitle lane 并行、旧 Audio late failure 丢弃、最新 Audio reply 仅按匹配 request/generation 完成、media switch 取消剩余 Audio/Subtitle pending，以及取消后的 late reply 不复活串成一个确定性序列。既有 QML/TrackListModel 契约继续保证提交只发 intent，最终 selected 只读取 Snapshot；本轮不增加 optimistic UI mutation。
- 用户在 HEAD `de9ce51930e891a9282413902ac31a1dde513f56` 的 Windows 锁定环境完成 build；Quick **106/106 PASS，0 failed（56.05 s）**，Full **114/114 PASS，0 failed（82.75 s）**，`playback_request_tracker` 与 `player_track_selection_qml` 均通过；startup-smoke **6.00 s**，8 项 windowed-render 合计 **42.43 s**。**R8-10 正式 Complete**。无生产代码、依赖、QML 或 CMake target 变化；Stage R8 的多轨/字幕/章节联合真实媒体 smoke 继续保留为阶段关闭项，不记为已执行。

### 2026-08-20 — R8-09 Track List Generation Scope Complete

- Track list 继续只存在于 generation-gated `PlaybackSnapshot`：新媒体进入 Opening 时由统一 cleanup policy 先清空旧 tracks/selected/capabilities，backend `track-list` 与 video/audio/subtitle selection property 必须携带当前 `MediaGeneration`，stale 或无 generation 事件在进入 Reducer 前丢弃；presentation model 不建立第二份 generation 或 selection 真值。
- 修正 selected property 指向当前列表外 ID 时保留旧 selection 的缺口：Reducer 现在只接受当前列表中同 `TrackKind` 的 stable backend ID，否则仅清除对应 video/audio/subtitle selected 状态，不影响另外两类轨道。扩展 `playback_reducer` 与 `media_generation_gate` 回归；既有 `track_list_model`、`track_selection_controller`、mpv mapper 与 QML 测试继续覆盖 A→Opening B→Ready B 整表替换、缺失 selected ID 不假选中、stable ID 命令、显式 subtitle off 及提交失败不改 UI 真值。
- 用户在 HEAD `68a7aa92353d95e5b9e713c2c0b6f482405ce3f1` 的 Windows 锁定环境完成 build；Quick **106/106 PASS，0 failed（46.30 s）**，Full **114/114 PASS，0 failed（79.29 s）**，`playback_reducer`、`playback_media_generation`、`track_list_model` 与 `player_track_selection_qml` 均通过；startup-smoke **3.06 s**，8 项 windowed-render 合计 **39.30 s**。**R8-09 正式 Complete**。无新增生产依赖或 QML/CMake target。

### 2026-08-20 — R8-08 Chapter UI Complete

- 新增独立 `ChapterNavigationViewModel`：只消费 generation-gated `PlaybackSnapshot`，由 position 推导 current chapter，并单独维护当前 generation 的 pending chapter target。章节 row、上一章与下一章只发出 absolute-seconds intent；`ApplicationContainer` 将 intent 显式包装为 `SeekCommand{Absolute}` 并复用既有 PlaybackCommandBus。媒体切换、non-seekable、提交失败、异步 request failure 或 Snapshot 到达目标都会清理 pending，不建立第二套 Timeline/Playback 真值。
- 按 Figma canonical `Chapters / Content`（`200:2168`）与 `Standard Inspector / Chapters`（`623:1611`）接入 Chapter Inspector：复用 `PlayerInspectorShell`、既有 token 与 58px row geometry，区分 Default / Current / Pending / Focus，并提供无章节 empty state 与 footer 上一章/下一章。OSC 新增 Chapters 入口；Timeline 只从 readonly chapter model 绘制 marker，chapter QML 不调用 scrub API、不写 `timelineSlider.value`、不接触 PlaybackSession/mpv。
- `ChapterModel` 新增只读 `timeText` role；新增 `chapter_navigation_view_model` 与 `player_chapters_qml` CTest，并扩展 ChapterModel、ApplicationContainer、Playlist/Chrome policy 回归，覆盖重复时间戳、点击不改 current、Snapshot acknowledgement、non-seekable、previous/next、request failure、media switch、QML wiring 与 timeline 双写禁令；无新增生产依赖。
- 最终修复 Chapter feature 对内部 `ButtonBase` 的运行时越界引用，并为 Timeline delegate 启用 bound component behavior。用户在 HEAD `f8a4c5245b2be337700ed605672808b0e841fe7b` 的 Windows 锁定环境完成 build；Quick **106/106 PASS，0 failed（40.21 s）**，Full **114/114 PASS，0 failed（78.79 s）**，`qml_module_boundaries`、`player_chapters_qml` 与 `player_app_startup_smoke` 均通过；startup-smoke **3.07 s**，8 项 windowed-render 合计 **38.71 s**。**R8-08 正式 Complete**；多轨/字幕/章节联合真实媒体 smoke 继续保留为 Stage R8 关闭项，不记为已执行。

### 2026-08-20 — R8-07 Chapter Decoder / Model Complete

- 新增独立 `MpvChapterListDecoder` infrastructure responsibility：只把已复制为 Qt value tree 的 mpv `chapter-list` 解码为 `QList<ChapterDescriptor>`。每项必须包含有限、非负的 numeric `time`，`title` 保持 optional；backend 数组顺序直接生成稳定 chapter index，重复时间戳不去重，长标题不截断，未知额外字段忽略，非法 payload 整体拒绝并返回诊断。既有 `MpvChapterModelMapper` 仅保留 property unavailable/error/event 适配，未改 PlaybackSession、Reducer 或 property registry。
- 新增独立 `src/chapters/presentation/ChapterModel`，只消费现有 generation-gated `PlaybackSnapshot::chapters()` 并提供 readonly `index / title / time` roles；无标题或空标题显示 `Chapter N` fallback。ApplicationContainer 将同一 `StatePublisher::snapshotPublished` 接到该 model，因此 Opening/new-media Snapshot 会先清空旧章节，再由当前 generation Snapshot 替换；没有 QML 暴露、Seek intent 或 timeline thumb mutation，R8-08 边界保持不变。
- 新增 `mpv_chapter_list_decoder` 与 `chapter_model` CTest，并扩展 `application_container` ownership 回归；覆盖无章节、多章节、重复时间、长标题、缺失标题 fallback、未知字段、malformed payload、media switch clear/replace 与 readonly role contract。没有新增生产依赖。
- 用户已在 HEAD `188ca44f1e51f13a2d585be5dc012682d6f74aca` 的 Windows 锁定环境完成重新 configure/build；Quick **104/104 PASS，0 failed（44.88 s）**，Full **112/112 PASS，0 failed（79.70 s）**。`application_container`、`mpv_chapter_list_decoder` 与 `chapter_model` 在 Quick/Full 中均通过；Full 的 startup-smoke **3.05 s**，8 项 windowed-render 合计 **39.46 s**。**R8-07 正式 Complete；尚未进入 R8-08。**

### 2026-08-20 — R8-06 Audio Delay Complete

- R8-06 后续 Windows Full 回归已在 `a31d9c779a7fce7b5f64eb7becc9736e9e62aee9` 基线完成：**110/110 PASS，0 failed（78.43 s）**；`audio_delay_flow` 与 `audio_delay_controller` 均 PASS，startup-smoke **3.06 s**、8 项 windowed-render 合计 **38.52 s**。该 Full gate 覆盖 R8-06 A1/A2 已注册回归，用户随后明确推进 R8-07；Stage R8 的最终真实多轨/字幕/章节联合 smoke 仍保留到阶段收口，不在此记录中虚构为已执行。

### 2026-08-20 — R8-06 Audio Delay A2 Candidate

- R8-06-A1 已正式关闭：`9d29077e351e9a150f27016653471a8950e8de90` 建立独立 `SetAudioDelayCommand → RequestTracker AudioDelay lane → mpv audio-delay → property/readback → generation-gated PlaybackSnapshot.controls.audioDelaySeconds` 权威链，`78d1cbb0130da9192ff8be2990dfa0001c1385bc` 只加固测试编译可移植性。Windows 重新 configure/build 已通过；Quick **101/101 PASS（46.56 s）**，Full **109/109 PASS（79.78 s）**；`audio_delay_flow` 在 Quick/Full 均 PASS，Full startup-smoke **3.07 s**、8 项 windowed-render 合计 **39.56 s**。
- A2 候选 `1eb4ecd24abf29880ddb051ed685edbbbbc62b47` 新增职责独立的 `AudioDelayController`，只拥有当前 generation 的 pending target，确认值只读取 `PlaybackSnapshot.controls.audioDelaySeconds`；媒体 generation 变化、异步 request failure、未选中 Audio 或 backend value unavailable 都不会制造 optimistic confirmed state。`PlaybackComposition/ApplicationContainer` 只负责把该 Controller 接入既有 command/state publisher，并沿用同一个 failure observer 分发 Audio/Subtitle 各自失败，未建立第二套 Playback owner。
- Figma canonical `Tracks / Audio Delay Control`（183:670）已实现为独立 `AudioDelayControl.qml`：**324×54** Neutral/Pending/Disabled，78px identity、118×32 slider、32×32 reset；Audio 与 Subtitle 只共享通用 delay geometry tokens，不共享 Controller、pending、Snapshot 字段或 HUD coalescing category。控件位于 Audio track section，QML 不调用 mpv；确认后的 Audio Delay 使用独立 HUD `audioDelay` key。
- 新增 `audio_delay_controller` CTest，并扩展 `player_track_selection_qml` 与 HUD queue 回归，覆盖正/负/reset、50 ms 量化、范围/NaN 拒绝、generation 清 pending、提交失败、无已选 Audio 时 disabled、QML wiring/mpv 边界以及 signed-ms HUD。由于 A2 新增 CMake target/QML file，Windows 必须重新 configure；A2 的 build、Quick/Full 与真实媒体 `+250 ms / -250 ms / reset`、同时调整字幕延迟互不影响的 smoke 目前尚未执行，因此保持 Candidate，无新增生产依赖。

### 2026-08-20 — R8-05 Subtitle Delay Complete

- zlib 兼容修复已在 Windows 完成实际 package rebuild：固定 zlib **1.3.2**，mpv 配置确认 `zlib=enabled`，libmpv/runtime package staging 完成。随后 Player build PASS；首次完整回归暴露与 zlib 无关的 `PlaybackSessionTest::rapidReplacementKeepsLatestGeneration()` 竞态，未通过增加 timeout 或弱化断言绕过。
- `208b8e495a5a3c82264c2c66e68d4333f8f692ee` 把快速媒体替换代际归因从“未知 START_FILE 消费 pending FIFO”改为 mpv `loadfile` COMMAND_REPLY 的 `requestId + playlist_entry_id` 与 START_FILE `playlist_entry_id` 确定性相关。直接相关 3/3 PASS，`playback_session` 连续 **20 次 PASS**；随后 Quick **100/100 PASS（44.09 s）**、Full **108/108 PASS（79.42 s）**，startup-smoke **3.04 s**、8 项 windowed-render 合计 **39.14 s**。
- 同一问题 MKV 的真实复验确认内嵌 `S_HDMV/PGS` 已进入 `pgssub` decoder，日志不再出现 “mpv has not been compiled with support for zlib compression” / subtitle `Skipping track`；用户同时确认字幕延迟真实操作可按 OK 收口，包括正/负/reset。R8-05 因此正式 **Complete**。日志中剩余 HEVC/Dolby Vision RPU warning 与本任务无关，不阻断收口。

### 2026-08-19 — R8-05 embedded subtitle zlib compatibility repair Candidate

- 真实媒体诊断已把“外挂字幕可显示、内嵌字幕无法正常显示”定位到自建 libmpv 的 Matroska 能力缺口，而不是字体链：该 MKV 的内嵌字幕为 `S_HDMV/PGS`，多条轨道被记录为 zlib content-compression；mpv 明确报出未编译 zlib 支持并跳过轨道。`c0e4e67711165f2744b8c38e403a0067c8fc1440` 只增加 `MPV_EVENT_LOG_MESSAGE` 的字幕/字体/容器诊断转发，不改变字幕渲染行为。
- 根因源码位于 `scripts/libmpv/clang64/build/mpv.sh` 的显式 `-Dzlib=disabled`。修复 commit `fdb9e108108d1168321f5e3dd8870d7575bb5d04` 固定 zlib **1.3.2** / `v1.3.2` / commit `da607da739fa6047df13e66a2af6b8bec7c2a498`，新增独立 `zlib.sh` 源码构建 responsibility，以 shared library 安装进现有隔离 prefix，再把 mpv 改为 `-Dzlib=enabled`；没有修改 PlaybackSession、Track selection、QML 或字幕延迟真值链。
- audited libmpv package 同步纳入 zlib source identity、build policy、runtime artifact hash 与 zlib License；runtime staging 明确要求恰好一个 `libz.dll` / `zlib1.dll`，build-layout 门禁禁止重新出现 `-Dzlib=disabled`。这新增一个宽松 zlib License 的小型运行时依赖，目的仅为支持 Matroska zlib-compressed content；没有新增应用层生产依赖。
- 当前连接环境不能执行 MSYS2 CLANG64 dependency rebuild 或 Windows Qt/MSVC 回归，因此 `scripts/libmpv/verify-build-layout.ps1`、新 libmpv package build/verify、Player build、Quick/Full CTest 与同一 MKV 的内嵌 PGS 字幕实机 smoke 都仍待 Windows 复验。R8-05 继续为 Candidate，R8-06 未开始。

### 2026-08-19 — R8-05 Subtitle delay Candidate

- 新增独立 `SubtitleDelayController` responsibility 与 `SetSubtitleDelayCommand`。Controller 只保存当前 generation 的 pending target；确认值唯一来自 `PlaybackSnapshot.controls.subtitleDelaySeconds`。媒体 generation 变化或异步 request failure 会清理 pending；成功回执会主动回读一次 `sub-delay`，避免仅依赖增量 property event 导致 UI 长时间停在 Pending。
- mpv 适配保持单一权威链：`set sub-delay <seconds>`，并把 `sub-delay` 注册为 Double property，经既有 observer/event mapper/generation gate/reducer 回到 Snapshot。字幕延迟 request 绑定 MediaGeneration，并使用独立 SubtitleDelay supersession lane；不与 Track selection、外挂字幕或后续 R8-06 Audio Delay 共用状态。固定范围 **±2.0 s**、步进 **50 ms**，覆盖正值、负值与 reset `0 ms`。
- QML 新增职责独立的 `SubtitleDelayControl.qml`，按 Figma `Subtitles / Delay Control`（191:1313）实现 **324×54** Neutral/Pending/Disabled、118×32 slider、32×32 reset 与 pending target 文案；几何进入现有 Size/Layout Token，不留下 Feature raw metric。确认后的实际值进入现有 HUD，按 `+250 ms / -100 ms / 0 ms` 显示；QML 不调用 mpv。
- 新增 `subtitle_delay_flow` CTest，并扩展 PlaybackCommand、Request supersession、mpv property baseline/observer 与 HUD queue 回归，覆盖正/负/reset、范围拒绝、50 ms 量化、generation 清理、同 generation latest-wins、`set sub-delay` 编码、property→Snapshot 确认真值和 HUD signed-ms。当前连接环境不能执行 Windows Qt/MSVC configure/build、QML lint 或 CTest，也没有真实字幕时序效果 smoke；这些均不记为通过。R8-05 保持 Candidate，无新增生产依赖。
- 用户首轮 Windows build 在 `subtitle_delay_flow_test.cpp` 编译阶段失败，`application::RequestTracker / RequestTrackStatus / PlaybackRequestType` 在测试自身的 `player::tracks::application` 作用域内被错误解析到当前 namespace，产生 C2039/C2065/C3083 连锁错误；build 因此以 exit code 1 停止，后续 `-Quick -SkipBuild` 也因 development runtime marker 未生成而未执行。生产字幕延迟链未被该日志判定失败；测试已改为明确引用 `player::playback::application::...`，修复 commit `b27022f438f7483d08de61221ca3a32800053830`。
- 第二轮 Windows build 通过，但 Quick 为 **99/100**，唯一失败是 `qml_module_boundaries`：`SubtitleDelayControl.qml` 在 Feature 层直接导入 `Player.Presentation.Primitives/Surfaces`。没有放宽边界测试；`20a8db6cbba7ae2f996daa8e6ee83d42b397627e` 仅把该 Feature 改回既有公开 `Controls + Theme` 边界，根 Surface/文字使用 QtQuick 原生项和 Theme tokens，未改 playback/application/mpv/state 行为。
- `20a8db6...` 随后在 Windows 锁定环境完成 build，Quick **100/100 PASS（40.48 s）**、Full **108/108 PASS（80.29 s）**；`subtitle_delay_flow` 与 `qml_module_boundaries` 均 PASS，startup-smoke **3.05 s**、8 项 windowed-render 合计 **39.79 s**。自动门禁已关闭；真实 +250 ms / -250 ms / reset 时序 smoke 因上述内嵌字幕 zlib 兼容缺口被阻塞，因此 R8-05 仍保持 Candidate。

### 2026-08-19 — R8-04 External subtitle Complete

- R8-04 修复版 HEAD `da7400e1637804b046aed971821cea29dd456469` 的自动门禁证据保持不变：Windows build PASS，Quick **99/99 PASS（42.29 s）**、Full **107/107 PASS（78.80 s）**，startup-smoke **3.05 s**、8 项 windowed-render 合计 **38.84 s**；`external_subtitle_flow` 与 `player_fullscreen_controls` 均进入 Quick/Full 回归。
- 用户随后实机确认本地外挂字幕可以添加，且 Track Popup 内双击不再穿透到底层视频触发全屏，并明确允许本 Atomic Task 暂时收口。因此 R8-04 按当前产品实机结果记为 **Complete**；本次不把未单独报告的 SRT/ASS 双格式逐项矩阵伪写为已验证，Stage R8 关闭前仍需按阶段任务书补真实多音轨/多字幕整体 smoke。
- 编码检测、更完整的重复加载治理与可见 Toast 错误反馈仍未在 R8-04 提前扩张；完整 D4 Inspector 视觉重组也不作为本次功能收口结果。Stage R8 仍为 In Progress，下一 Atomic Task 为 R8-05。

### 2026-08-19 — R8-04 real-media smoke repair Candidate

- HEAD `2d450e53aca9693d7d3d54601ca68fc882fbb6d0` 已在 Windows 锁定环境完成 build，Quick **99/99 PASS（40.83 s）**、Full **107/107 PASS（79.00 s）**，startup-smoke **2.76 s**、8 项 windowed-render 合计 **38.80 s**；但随后真实本地 smoke 明确暴露两条候选缺陷：视频内建音轨/字幕可选择，而外挂字幕未能完成产品链；Track Popup 内双击仍会触发底层视频全屏手势。因此自动门禁全绿不作为 R8-04 完成证据。
- 全屏双击根因已定位为 `FullscreenGestureLayer` 的全 VideoViewport `TapHandler.DragThreshold` 在 Popup/Drawer/Modal/OSC 控件交互期间仍保持启用；Qt 6.8 passive grab 会允许底层 handler 同时观察 pointer sequence。修复候选新增单一 interaction gate，在 popup、menu、drawer、modal、error overlay、OSC hover/focus 期间禁用底层双击手势，轨道 row 第一次点击关闭 Popup 后也不会把该次 pointer sequence 留给底层组成 double-tap。
- 外挂字幕不改变 `sub-add <path> cached` 协议或建立第二套 model。成功 `AddExternalSubtitle` command reply 现在由 PlaybackSession 触发 backend 确定性刷新 `track-list + sid`，复用同一个 property read/map/event path 回到 generation-gated PlaybackSnapshot 与既有 Subtitle `TrackListModel`；正常 mpv property observer 继续保留，但不再是成功后的唯一刷新来源。
- `ExternalSubtitleLoader` 现在记录同步校验/提交拒绝和已提交事件；PlaybackSessionThread 记录 `AddExternalSubtitle` 异步 backend failure，使下一次真实失败可以从日志区分 `file validation/submission` 与 `mpv command reply`。修复版 HEAD `da7400e1637804b046aed971821cea29dd456469` 已在 Windows 锁定环境完成 build，Quick **99/99 PASS（42.29 s）**、Full **107/107 PASS（78.80 s）**；`external_subtitle_flow` 在 Quick/Full 分别 **0.20 s / 0.04 s PASS**，`player_fullscreen_controls` 分别 **0.11 s / 0.02 s PASS**，Full 的 startup-smoke **3.05 s**、8 项 windowed-render 合计 **38.84 s**。该日志未包含真实 Player SRT/ASS 加载或 Track Popup 双击实机结果，因此当前只关闭自动回归门禁，R8-04 继续保持 Candidate。无新增生产依赖。

### 2026-08-19 — R8-04 External subtitle Candidate

- 新增独立 `ExternalSubtitleLoader` responsibility：只接收本地 `QUrl`，检查文件存在、为普通文件、可读，并将 `.srt/.ass`（大小写不敏感）规范化为 canonical path 后提交；远程 URL、缺失/不可读文件、非 SRT/ASS 与提交拒绝均进入明确 error key。编码检测及更完整的重复加载治理未提前实现。
- Playback 主链新增 `AddExternalSubtitleCommand` / `PlaybackRequestType::AddExternalSubtitle` / `MpvExternalSubtitleRequest`。请求绑定当前 MediaGeneration，媒体切换会取消旧 generation 的 pending add；外挂字幕 add 不进入 Audio/Subtitle selection supersession lane。mpv 0.41.0 适配为 `sub-add <path> cached`，成功后的 `track-list/sid` 继续经现有 property observer、generation gate、reducer 回到 PlaybackSnapshot 与同一个 `TrackListModel`，没有新建外挂字幕列表或 optimistic selected 状态。
- QML 文件选择职责独立放入 `ExternalSubtitleOpenDialog.qml`；R8-03 `TrackSelectionPopup` 只增加“Add External Subtitle…”入口并发出 intent，不直接持有 FileDialog、PlaybackSession 或 mpv。Figma D4-05 `Subtitles / Add External Action`（190:1224）的“只拥有 file-picker affordance”交互职责已核对；视觉稿同时展示 SSA/VTT，但当前 R8-04 任务书只要求 SRT/ASS，因此本候选不虚假开放未实现格式，也不在本 Atomic Task 重建完整 D4 Inspector。
- 新增 `external_subtitle_flow` CTest，覆盖可读 SRT/ASS、非本地/缺失/不支持格式、提交失败、generation scope、`sub-add cached` 映射及 QML ownership/wiring；同时扩展 PlaybackCommand、mpv command executor 与 request supersession 既有契约。当前连接环境不能运行 Windows Qt/MSVC build、QML lint 或 CTest，也未执行真实 SRT/ASS 渲染与同模型刷新 smoke；这些验证均不记为通过，R8-04 保持 Candidate。无新增生产依赖。

### 2026-08-19 — R8-03 Track selection Complete

- 代码 HEAD `1ec652763387624dbfbdd3588eb91cf7c73d1364` 已在 Windows 锁定环境完成 build；此前 `TrackSelectionPopup.qml` 的 6 条 `qmllint [unqualified]` 警告在加入 `pragma ComponentBehavior: Bound` 后不再出现，构建无新的 QML lint 警告。
- 最终 Quick **98/98 PASS，0 failed（40.66 s）**；Full **106/106 PASS，0 failed（79.72 s）**。Full 中 startup-smoke **3.05 s**，8 项 windowed-render 合计 **39.22 s**；`track_selection_controller`、`track_list_model`、`theme_tokens`、`player_playlist_qml`、`player_track_selection_qml` 均实际通过。
- R8-03 的 stable track ID → command → backend/snapshot 主链、Subtitle Off、snapshot-authoritative selection、same-kind supersession 与 QML wiring 已进入完整回归。按 R8 任务书统一策略，尚未覆盖的手工矩阵不阻塞单个 Atomic Task；真实多音轨/多字幕本地媒体 smoke 尚未执行，继续保留为 Stage R8 关闭前必须补齐的本地验证项，不记为已通过。**R8-03 正式 Complete。**

### 2026-08-19 — R8-03 Quick gate repair Candidate

- 首轮 Windows 锁定环境已完成 R8-03 重新 configure/build；Quick 实际为 **96/98 PASS、2 failed（45.97 s）**，CTest exit code 8。失败仅为 `theme_tokens` 检出的 `TrackSelectionPopup.qml` raw visual metrics，以及 `player_playlist_qml` 仍要求已被真实 Track selection 控件替代的 `subtitlesUnavailableControl` 旧契约。
- `TrackSelectionPopup` 保持原 280px popup、8px padding、4px item gap 的可观察几何，但把这些值移入现有 Theme responsibility：新增 `SizePrimitives.size280`、`SpacingPrimitives.space4/space8` 与对应 `LayoutTokens.trackSelectionPopupWidth`、`SpacingTokens.trackSelectionPopupGap/trackSelectionPopupPadding`；row 宽度由 popup 实际左右 padding 推导，不再复制 264px literal。没有放宽或跳过 `theme_tokens` 门禁。
- `player_playlist_qml` 的旧 placeholder 断言改为验证真实 `trackSelectionButton` 与 `TrackSelectionPopup`，同时保留 Subtitles/Track → Playlist → Fullscreen 的 canonical 顺序及既有 spacing 断言；没有为了旧测试重新引入不可用占位控件。
- 本连接环境不能执行 Windows Qt/MSVC build/CTest，因此修复后的 Quick **98/98**、Full **106/106** 仍是待用户复验目标，不记为已通过；真实多音轨/多字幕媒体 smoke 也仍未执行。R8-03 继续保持 Candidate。

### 2026-08-18 — R8-03 Track selection Candidate

- 新增独立 `TrackSelectionCommand` 与 `TrackSelectionController` responsibility：QML 只提交 backend stable `trackId` intent，Audio/Subtitle 选择均进入现有 PlaybackCommandBus；Subtitle Off 使用显式 `trackId=nullopt → sid=no`，Audio Off 在 domain validation 与 mpv encoder 两层拒绝。没有使用 ListView index，也没有新增第二套 Track owner。
- Track selection request 已接入既有 RequestTracker/MediaGeneration 规则：Audio、Subtitle 分属独立 supersession lane，同类后一次请求会 supersede 同 generation 的前一次；媒体 generation 变化会取消旧 track-selection pending request，superseded/stale/duplicate reply 不进入当前状态副作用。
- mpv adapter 只负责把 domain selection 映射为 `set aid <id>`、`set sid <id>` 或 `set sid no`。Backend property/event 仍经现有 MediaGeneration gate 与 reducer 回到 PlaybackSnapshot；`TrackListModel.selectedTrackId` 和 row `selected` 只从 Snapshot 权威 selected ID 推导，command submission/reply 失败不会制造 UI 假选中。
- 新增功能性 `TrackSelectionPopup.qml` 与 OSC wiring，提供 Audio、Subtitles、Off 入口，并把 popup open 状态接入既有 chrome/cursor interaction policy；本轮没有进行 Figma 或视觉重构。新增 `track_selection_controller`、`player_track_selection_qml` CTest，并扩展 PlaybackCommand、RequestTracker、mpv command 与 TrackListModel 测试。没有新增生产依赖。
- 当前环境未执行 Windows configure/build/CTest，也未执行真实多音轨/多字幕媒体 smoke，故本任务仍为 Candidate 而非 Complete。由于新增 CMake target/subdirectory，Windows 验证必须先重新 configure；自动化通过后还需用本地合法多轨媒体验证 Audio 切换、Subtitle 切换、Subtitle Off、无音轨但有字幕以及失败后不假选中。预计重新 configure 后 Quick 为 98 项、Full 为 106 项，仅作为候选计数预期，不代表已通过。

### 2026-08-18 — R8-02 Track list models Complete

- 新增独立 `src/tracks/presentation/TrackListModel` responsibility，并按 Audio / Subtitle 创建两个实例。Model 只消费 `PlaybackSnapshot::tracks()`，按 kind 过滤并整表投影 backend stable `trackId`、title/language/codec、selected/default/forced/external/external filename；没有 mutation API、mpv property 字符串、visual index 命令参数或第二套 Track owner。
- selection 不直接信任 `TrackDescriptor.selected` 缓存字段，而是由 Snapshot 权威的 `selectedAudioId / selectedSubtitleId` 推导；selected ID 不存在于当前列表时所有行均为未选中，避免 UI 假选中。R8-03 才负责 selection command / failure / subtitle-off 的写入链，本轮不提前实现。
- 两个 model 直接订阅现有 `StatePublisher::snapshotPublished`。PlaybackSession 的 MediaGeneration gate 继续在上游拒绝 stale media event；现有 reducer 在 Opening/Stop/Failed 时清空 `state.tracks`，因此 A→B 时 model 先收到空列表，再由 B current-generation Snapshot 整表替换，不在 presentation 层创建第二个 generation gate。
- `ApplicationBootstrap → MainWindow → PlayerScreen` 已增加只读 `audioTrackModel / subtitleTrackModel` 数据边界，供后续 R8-03 Track feature 使用；本轮没有增加可见 Track 面板、选择控件或 Figma/视觉修改。新增独立 `track_list_model` CTest，覆盖 Audio/Subtitle 分离、stable ID/metadata roles、Snapshot selected-ID 真值、selected ID 缺失时无假选中、A→Opening B→Ready B 清空替换、readonly flags。没有新增生产依赖。
- 用户在 HEAD `101df83ab84d64ae97c505ce03d667eeb12ae33c` 的 Windows 锁定环境完成重新 configure 与 build；Quick **96/96 PASS，0 failed，40.86 s**，Full **104/104 PASS，0 failed，79.84 s**；`track_list_model` 在 Quick/Full 均 PASS，Full 的 startup-smoke **3.04 s**，8 项 windowed-render 合计 **38.99 s**。任务书 R8-02 的 A→B model replace / old-track cleanup 已由该 CTest 覆盖并进入全量回归。真实多音轨/多字幕媒体 smoke 继续保留为 R8 阶段本地验证项，待后续具备可交互 Track selection 后执行。**R8-02 正式 Complete。**

### 2026-08-18 — R8-01 Track decoder Complete

- R8-01 不重建第二套 Tracks 真值：既有 `TrackDescriptor` / `PlaybackTrackState` / reducer / MediaGeneration gate 继续保持。新增独立 infrastructure responsibility `MpvTrackListDecoder`，只负责把 `MpvNodeDecoder` 已经复制出的 `QVariantList/QVariantMap` track tree 解码为 `QList<TrackDescriptor>`；`MpvTrackModelMapper` 保留 property unavailable 与 selection-property 适配，只把 `track-list` 的结构解析委托给 decoder。上层没有新增 `mpv_node`、property 字符串或 UI index 依赖。
- decoder 保留既有产品语义：track `id` 必须是 backend 正整数稳定 ID；支持 video/audio/subtitle kind；title/language/codec/external filename 为 optional；selected/default/forced/external/image/album-art 缺失时为 false；未知额外 backend 字段忽略；非法顶层、非法 entry、缺失/非法 id/type、错误 optional 字段类型会整体拒绝，不输出半解析列表。没有新增生产依赖。
- 新增独立 `mpv_track_list_decoder` CTest，覆盖多 audio/subtitle metadata、最小字段、未知字段兼容、空列表、malformed payload 原子拒绝，以及 mapper 对有效列表与 unavailable property 的既有语义。现有主链仍为 `mpv node → MpvNodeDecoder → MpvPropertyChange → MpvTrackListDecoder/Mapper → TrackListChangedEvent → MediaGeneration gate → reducer → PlaybackSnapshot`。
- 用户在 HEAD `c19632945f886603b292daecd3a2e98a98554033` 的 Windows 锁定环境完成重新 configure 与 build；Quick **95/95 PASS，0 failed，40.58 s**，Full **103/103 PASS，0 failed，78.87 s**；新增 `mpv_track_list_decoder` 在 Quick/Full 均 PASS，Full 的 startup-smoke **3.05 s**、8 项 windowed-render 合计 **38.90 s**。任务书要求的真实多音轨/多字幕媒体基本验证尚未执行：项目因 R0-06 已跳过而没有可合法分发 fixture，且 R8-02 前没有面向 QML 的只读 Track model 可用于稳定观察真实媒体列表，因此该 smoke 明确保留为 R8 阶段本地验证项，不伪称已覆盖，也不阻断 R8-02。**R8-01 正式 Complete。**

### 2026-08-18 — R7-14 Queue UI state decoupling Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 把 Playlist row 的状态所有权明确拆开：Playlist Domain 继续唯一拥有 `current EntryId`；QML 只拥有 selected、keyboard focus、hover 交互状态；新增独立 presentation responsibility `PlaylistEntryPlaybackState`，只从现有 generation-gated `PlaybackSnapshot` + 当前稳定 EntryId 投影 pending loading / unavailable。没有新增 MediaGeneration、RequestId 或第二套 queue/playback owner。
- `PlaylistListModel` 新增只读 `pendingLoading` / `unavailable` roles，并保留既有构造路径与原 role 编号；ApplicationContainer 在同一 `StatePublisher` 主链中先把 Snapshot 投影给 row state，再执行 Auto Advance，因此失败项可在 current 推进前被标记。Opening 进入 pending，Ready 清 pending/unavailable，Failed 清 pending 并标 unavailable；切换 current 会清理旧 pending，删除 entry 会裁剪 unavailable，重试 Opening 会清除该 entry 的 unavailable。由于输入来自 PlaybackSession 已过滤后的 Snapshot，不新增 stale generation 写入路径。
- `PlaylistRow.qml` 不再用 `current || selected` 合并蓝色高亮：selected 只控制 selection fill，activeFocus 只控制 focus ring，hover 使用独立 hover fill，current 只显示 playing rail；pending/unavailable 通过单独状态 indicator 与现有 `ColorTokens/MaterialTokens` 表达。R7-09 已验证的 current 删除语义现已恢复到 UI：current row 的删除动作继续只经 `PlaylistController::removeEntry()`，不增加 QML→Playback/mpv 旁路。
- 新增 `playlist_entry_playback_state` CTest，并扩展 `playlist_list_model` 与 `player_playlist_qml`，覆盖 current+unavailable 共存、Opening/Ready/Failed、retry、current change、removed-entry prune、只读 roles，以及 current/selected/focus/hover/pending/unavailable 不再共用单一 highlighted 状态。shutdown 时 `PlaylistListModel` 和 `PlaylistEntryPlaybackState` 均在 Playlist/PlaybackComposition 销毁前释放；没有新增 timer、thread、callback owner 或生产依赖。
- 用户在 HEAD `d1fa23dc12c71e9c31c8c3b520d8b50d86894507` 的 Windows 锁定环境完成 configure/build；Quick **94/94 PASS，0 failed，55.79 s**，Full **102/102 PASS，0 failed，82.32 s**。`playlist_list_model`、`playlist_entry_playback_state`、`player_playlist_qml` 均在 Quick/Full 通过；Full 的 startup-smoke **5.98 s**，8 项 windowed-render 合计 **42.10 s**。Build 仅继续出现此前多轮构建已存在的 Qt 6.8 / MSVC 生成 QML 路径 `C4702` unreachable-code 警告，没有项目源码编译错误或测试失败。**R7-14 正式 Complete；Stage R7 正式 Complete。**

### 2026-08-18 — R7-13 Async Media Open Supersession Complete

- 按 `成熟播放器行为补强与验收矩阵.md` 为 `MediaOpenCoordinator` 增加单一、单调的 `MediaOpenOperationId`。每次 replace-open workflow 开始都会生成新 identity 并立即使前一 operation stale；异步解析完成只能通过 `completeOpenSource(s)` 回到 Coordinator，提交前再次校验 identity。stale completion 直接拒绝，不进入 Playlist/Playback submission，也不覆盖较新 operation 的 `lastErrorKey`。
- 现有同步 `openSource/openSources/openLocalFile(s)/openSourceUrls` 继续保持原公共行为，但内部统一走 begin→validate/resolve→complete operation 主链；`UrlOpenWorkflow` 在 URL validation **之前**获取 operation identity，因此新的 URL 请求即使验证失败，也已经使旧异步 open 结果失效。没有让 QML、drawer 或 readonly model 承担取消职责。
- `ApplicationContainer::shutdown()` 在销毁 media workflows/coordinator 前先调用 `MediaOpenCoordinator::beginShutdown()`，永久停止接收新 operation 并使当前 pending identity 失效；之后到达的旧解析结果无法再提交。
- 新增独立 `media_open_supersession` 测试目标，覆盖 A→B supersession、stale single/batch completion、stale result 不污染错误状态、operation single-use、同步 open supersede pending async、URL workflow 在 validation 前 supersede、scoped cancel、shutdown cancel/reject。没有新增生产依赖，也没有实现当前不存在的虚假异步 parser/thread；后续真正的 file/URL/playlist async parser 必须携带此 operation id 并在 Coordinator 线程回交结果。
- 首次 Windows 验证在 HEAD `395978cfae0104bc1f799254bec7cddd785aa4f1` 完成 configure/build；Quick **92/93 PASS**、Full **100/101 PASS，1 failed，78.79 s**，唯一失败均来自旧 `player_url_open` 静态契约仍要求已经被 R7-13 正式替换的直连 `openSource(source)`。生产实现未回退；测试契约改为验证 `beginReplaceOpenOperation → validation/cancel → completeOpenSource(operationId, source)`，并继续保留禁止 `PlaybackSession/libmpv/mpv_` 旁路的断言。
- 修正后的 HEAD `63f1539c5c0030a605874c2c07fe13781708b5e1` 已重新 build，并通过 Quick **93/93 PASS（40.71 s）** 与 Full **101/101 PASS，0 failed，79.40 s**；`media_open_supersession`、`player_url_open` 均 PASS，startup-smoke **2.77 s**，8 项 windowed-render 合计 **38.51 s**。R7-13 规定的 operation identity、新请求 supersede、stale async result 失效、QML 不承担取消、shutdown 取消 pending operation 均有实现与验证证据。**R7-13 正式 Complete。**

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
- Shuffle 使用独立 `PlaylistShuffleState` cycle bag；Repeat Off 遍历当前 cycle 后停止，Repeat All 在自然 EOF 时可建立新 cycle且排除刚结束项；成功 selection 才消费候选，因此即时 load submission rejection 不改变 current 或 shuffle history。
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

- Main Player 按 canonical 1320×700 reference 重建：500×54 Floating Header、880×124 Standard OSC；Playlist Inspector 打开时 OSC 收为 730×124并左移；Fullscreen 使用 440×50 Header + 828×106 Compact OSC。
- Playlist Inspector 为 368×652 floating overlay，不挤压 Video Viewport；Search 324×42、Row 332×58、Playing Rail 3×32、Footer 324×54。搜索/双击选择/删除非 current/重排继续走 readonly model + `PlaylistController`。
- Inspector 已通过 `VideoViewport → ShaderEffectSource → MultiEffect` 建立真实局部 backdrop capture blur；通用 Surface 没有被改造成任意祖先捕获器。
- `PlayerMediaViewModel` 只从现有 Snapshot 投影真实 title / metadata；Playlist queue 尚无权威 per-entry duration，因此列表第二行继续显示真实 sourceLocation，不虚构 Figma 示例时长。
- Figma semantic system 已建立 Light Mist / Dark 两个全局 mode；Dark 通过共享 Theme tokens覆盖播放器全局，不再存在 `inspectorDark*` 等 Feature-scoped palette。
- R10 负责的 native frameless / hit-test / DWM / Snap 仍未提前实现。

### 2026-08-17 — R7-06 Playlist Controller Complete

- Playlist Domain → Mutation → Controller → readonly Model → QML/MediaOpen 的单一主链已建立；所有生产媒体入口经 `PlaylistController` 再进入 Playback load，没有 QML/libmpv 旁路。
- 批量添加、稳定 Entry ID、选择、非 current 删除和 reorder 已接通；即时 load submission rejection 会回滚 queue/current，current deletion policy 仍留给 R7-09。
- Windows 锁定环境已验证：Debug configure/build PASS，Quick **86/86 PASS**，Full **93/93 CTest PASS（73.31 s）**。R7-06 已正式 Complete。
