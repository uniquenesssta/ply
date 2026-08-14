# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器。播放核心使用 libmpv，UI 使用 Qt 6 / Qt Quick/QML，核心代码采用 C++20，构建系统为 CMake + Ninja。

README 只维护**项目入口、当前状态、关键架构边界和简短变更记录**。Atomic Task 的目标、实现边界、验收矩阵与详细结果统一查看 `docs/plans/stages/`；README 不再重复完整实施过程。

## Current status

| Stage | 状态 | 结果 |
|---|---|---|
| R2 — 无 UI libmpv 播放核心 | Complete | 真实 load/play/pause/seek/stop、事件/属性/命令与 headless probe 主链完成 |
| R3 — 领域状态与 PlaybackSession | Complete | PlaybackSnapshot、Reducer、Generation、RequestTracker、Supersession、Session 生命周期完成 |
| R4 — libmpv OpenGL Render API | Complete | 视频进入 Qt Quick，Render 生命周期、DPI/visibility/shutdown 与 1080p/4K 基线完成 |
| R5 — UI 设计系统 | Complete | **R5-01 ~ R5-10 全部 Complete**；最终 Windows Debug build PASS、QML lint 门禁通过、**51/51 CTest PASS（53.29 s）**、`Player.exe` startup smoke PASS |
| R6 — 播放器主界面与基础交互 | In Progress | **R6-01 ~ R6-05 均 Complete**；R6-06 Timeline implementation candidate 已提交，新增 scrub/pending/Absolute Seek 主链与 3 个定向 CTest，预期 **59 → 62**；Windows configure/build/QML lint/CTest/startup pending |

R0/R1 属于现有项目基线，R2–R14 快速任务书不重新定义其历史状态。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，仅在明确调用时执行，不阻断后续 Stage。

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
- [R6 — 播放器主界面与基础交互](docs/plans/stages/R6_播放器主界面与基础交互.md)
- [R4 后置 — PlaybackSession 职责边界优化](docs/plans/stages/R4后置_PlaybackSession职责边界优化.md)

R7–R14 的任务书继续由 [Stage 索引](docs/plans/stages/00_INDEX.md) 统一导航。

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

### 2026-08-14 — R6-06 Timeline implementation candidate

- 新增职责独立的 `TimelineScrubSession` 与 `PlayerTimelineViewModel`。Scrub session 只拥有 `Idle / Scrubbing / PendingCommit` 交互状态、MediaGeneration 与 normalized preview；真实 position/duration/seekable/seeking 继续来自 `PlaybackSnapshot.timeline`，没有建立第二套 Playback 真值。
- Timeline VM 增加 `canSeek/isScrubbing/seekPending/backendSeeking/displayedNormalized/durationSeconds/positionText/durationText` 投影；拖动时 preview 屏蔽后台旧 position，commit 只发一次 absolute target。commit 后冻结 absolute target seconds，即使同媒体 duration 刷新也不会重算成错误目标；现有 mpv mapper/encoder 的 Absolute Seek 已确认走 `absolute+exact`，PendingCommit 只在实际 position 到达冻结 target ±0.75s 后释放，避免 `seeking:true→false` 事件先到造成 thumb 回弹。
- `PlaybackComposition` 继续复用 R6-05 的同一个 `PlaybackSessionThread`、共享 `PlaybackRequestIdGenerator` 与 `PlaybackCommandBus`，新增 `SeekCommand{absoluteSeconds, SeekMode::Absolute}` 提交；CommandBus 立即拒绝时显式释放 pending preview。具体 SeekCommand 依赖保持在 composition `.cpp`，没有扩散到公共头。
- `ApplicationBootstrap → MainWindow → PlayerScreen → TimelineControls` 通过 initial properties 显式传递 Timeline VM；新增 `features/player/timeline/TimelineControls.qml` 只 import Theme/Controls，复用既有通用 `Slider` pointer/keyboard 状态机，不复制 `MouseArea` 或直接接触 PlaybackSession/libmpv。
- Timeline 几何重新核对最终 Figma Standard `4:20` 与 Compact `4:63`：继续复用 12/11px Geist Mono timecode、3px track、10px rest thumb、16px hit target 与 26/28px OSC inset。wrapper 的半像素纵向补偿由既有 1px thumb-border token 的一半推导；R6-04 timeline slot 改为 `clip:false` 允许 canonical thumb 略超出 28/26px lane，外层 OSC Surface 仍是裁切 owner，其他 Feature slot 不变。
- Domain `playback_selectors` 新增 `canSeek()`；新增 `timeline_scrub_session`、`player_timeline_view_model`、`player_timeline_controls` 三个 CTest target，并扩展 selector 与 R6-04 slot contract，预期全量测试数 **59 → 62**。覆盖 drag/cancel/one-commit、stale position、pending target、duration refresh、generation change、non-seekable、unknown duration、seeking event ordering 与立即提交失败。
- 当前环境无法执行用户锁定的 Windows Qt 6.8.3 / MSVC 2022 x64 / libmpv 0.41.0 链，因此 `configure.ps1`、Debug `build.ps1`、QML lint、**62 项 CTest** 与 `Player.exe` startup 均尚未执行；当前只完成源码、最终 Figma contract、模块边界与 diff 静态审计。产品媒体打开入口仍属于 R7，所以 live-media 产品手工 scrub 也尚不可执行。**R6-06 仍是 implementation candidate，不是 Complete。**

### 2026-08-14 — R6-05 Transport Complete

- 建立 `PlaybackSnapshot → playback_selectors → PlayerTransportViewModel → QML intent → PlaybackComposition → PlaybackCommandBus → PlaybackSession` 真实 Transport 主链；ViewModel 不拥有播放真值，QML 不直接访问 PlaybackSession/libmpv。`PlaybackComposition` 同时引入共享 `PlaybackRequestIdGenerator`，为后续 Timeline/Volume 共用 RequestId 所有权。
- 常驻 OSC Transport 按用户确认的最终定义保持 **Previous / Primary Play-Pause Toggle / Next**；Previous/Next 在 R7 Playlist 提供真实 navigation state/command 前保持禁用。Stop 已完整存在于 selector/VM/CommandBus/PlaybackSession 能力链，但不作为常驻 OSC 按钮。
- 为关闭设计资产缺口，最终 Figma 新增 `D8 / Canonical Transport Assets` Section `745:3` 和唯一 `Icon / Transport / Stop` Component `746:2`（内部 glyph `746:3`）：22×22 canvas、中心 8×8 R1 filled glyph，绑定既有 `icon/primary`；主 OSC `4:20` 的 Previous `4:26` / Play Button `4:28` / Next `4:31` geometry 未改。当前产品没有 Stop 可见 consumer，因此仓库没有新增无使用方的 `stop.svg`。
- 首轮 Windows `configure.ps1` **PASS**（Configuring 4.4 s / Generating 1.9 s），随后 Debug build 因 `playback_composition.h` 错误前置声明 `TransportAction` 与 canonical `TransportAction : quint8` 冲突而 **FAILED**；`test.ps1` 因 development marker 未生成被正确阻断，没有把未执行测试描述为通过。
- 修复 commit `2509e25f01481e31efd94a9e1bb6cd7dd65d4e8c` 改为直接引用 canonical `transport_command.h` 并删除重复枚举前置声明，不改变 TransportAction、PlaybackCommand、CommandBus 或用户可观察播放语义。
- 修复后 Windows Debug `build.ps1` **PASS**，Qt runtime deployment 正常；`scripts/test.ps1` QML lint **PASS**；全量 **59/59 CTest PASS，0 failed，57.65 s**。新增 `playback_selectors`、`playback_request_ids`、`player_transport_controls`、`player_transport_view_model` 与扩展的 `application_container` 全部 PASS；`Player.exe` startup **PASS**，无媒体时 Previous / Play / Next 全部 Disabled 与 Snapshot/VM 状态一致。
- 产品媒体打开入口仍属于 R7，所以本轮没有伪装成已在产品 UI 中实际载入媒体后手工点击 Play/Pause；真实 `playback_session` WAV 主链已覆盖 load→pause→play→pause→stop，selector/VM/QML 定向测试覆盖本轮映射。按 R6 快速框架策略该项不再阻断，**R6-05 正式 Complete；下一项 R6-06。**

### 2026-08-14 — R6-04 Bottom control region Complete

- `PlayerBottomRegion` 保持 replaceable Host；`PlayerOscLayout` 只拥有 OSC 两层布局与 Timeline / Transport / Volume / Utility 四类 slot，`OscControlRow` 只拥有 control cluster 排列与窄宽 clip；各 Feature 实现没有回流 `PlayerScreen.qml`。
- 新增 `OscSurface` 并复用既有 Panel 材质；Standard/Compact 使用冻结的 880px max width、124/106px height 与对应 Material/Radius/Z-order token。Utility 靠右，Transport→Volume 保持 leading 顺序，窄宽时只裁切 leading 可用区，不缩小交互目标。
- R6-04 没有预造 R6-05~07 的静态假按钮/Timeline/Volume；新增独立 `player_bottom_region`，并扩展 `surface_controls` 覆盖 Standard/Compact `OscSurface` runtime contract。
- 用户 Windows 锁定环境最终验收：`configure.ps1` **PASS**（CMake Configuring 4.5 s / Generating 1.9 s）；Debug build **PASS**；`scripts/test.ps1` 完成 QML lint；全量 **55/55 CTest PASS，0 failed，53.23 s**，新增 `player_bottom_region` **0.11 s PASS**；随后实际启动 `Player.exe`。用户明确要求收口，**R6-04 正式 Complete**。
- 本地已有 `.gitignore`、`r4-04-qml-diagnostics/`、R4-09 1080p/4K JSON 继续作为受保护内容保留；没有 reset/clean，也没有修改 PlaybackSession、libmpv、Renderer、配置、数据结构或持久化。

### 2026-08-14 — R6-03 Floating Header Complete

- `PlayerTopRegion` 继续保持 R6-01 的 replaceable Host 职责；真实 Header 内容拆入 `screens/player/header/PlayerFloatingHeader.qml`、`MediaInfoPod.qml`、`WindowActionsPod.qml` 三个 screen-local 模块。`PlayerScreen.qml` 只提供 `mediaTitle` / `mediaMetadataText` / `windowExpanded` presentation 输入并转发 minimize / maximize-restore / close intent，没有把文本、按钮实现或窗口命令塞回 Screen。
- 已重新读取第三版最终 Figma D2-03 与 D8-05 canonical Floating Header：Standard 使用 **420×54 Media Info + 110×54 Window Actions**，Compact 使用 **360×50 + 110×50**；两种尺寸都保持 Media Info 真正水平居中，Header 左右使用 **26px safe edge**；Compact 隐藏 metadata，title 继续消费既有 `TitleText.MediaCompact` 单行 ENDING ellipsis。无 title 时 `MediaInfoPod` 完全不渲染，Window Actions 始终保留。
- Window Actions 直接复用 R5 已冻结的 **32×32 `IconButton` hit target / 22×22 glyph**，并按 D8-05 使用既有 `SizePrimitives.size3` 的 3px gap（新增 `LayoutTokens.headerActionGap` 只建立语义别名，不新增新的 primitive 数值）。没有为了早期 D2 22px 示意缩小可访问点击区，也没有复制 Button hover/pressed/focus 状态机。
- Minimize / Maximize / Close 三枚 glyph 已从最终 Figma D8-02 canonical 组件 `441:5` / `441:10` / `441:16` 只读 SVG export，仓库提交的是 Figma 导出的原始 geometry，不再保留候选阶段的手工矢量。D8-02 canonical 源组明确只包含 Minimize / Maximize / Close，**没有 Restore glyph**，因此删除了临时 `restore.svg`，不伪造缺失资产；最大化后的同一 action 继续使用 canonical Maximize glyph，但 Tooltip/Accessible description 与命令语义切换为 Restore。
- `WindowActionsPod` 只发出 intent，真正 Qt 通用窗口操作由 `MainWindow.qml` 统一执行 `showMinimized()` / `showMaximized()` / `showNormal()` / `close()`；没有加入 `Qt.FramelessWindowHint`、Win32 native event、hit-test、resize 或 Snap。R10-04 仍是无边框窗口与 Windows hit-test 的唯一任务，因此当前中间阶段**仍保留系统原生标题栏，原生窗口按钮与 Floating Header actions 会暂时共存**，本任务不提前消除它。
- 当前仓库仍没有 `PlayerViewModel` / playback presentation composition，所以 `mediaTitle` / `mediaMetadataText` 只是后续可绑定的数据边界，未伪装成 `PlaybackSnapshot → Header` live binding；应用默认没有媒体 identity 时只显示 Window Actions。R6-03 不创建静态假媒体标题，也不把 PlaybackSession/libmpv 引入 QML。
- 新增职责独立的 `player_top_region` CTest target，静态锁定 Host→Header→POD 模块边界、居中/Compact/无标题优先级、Window intent owner、R10 native-window 禁区以及 Figma canonical window glyph geometry；既有 `icon_pipeline` 同步纳入新增 Minimize/Maximize 资产并继续禁止产品 QML 绕过 Icon primitive。
- 用户锁定 Windows 环境最终验证：`configure.ps1` **PASS**（CMake Configuring 4.5 s / Generating 1.8 s）；Debug `build.ps1` **PASS**；`scripts/test.ps1` 的 QML lint 门禁完成；全量 **54/54 CTest PASS，0 failed，54.36 s**，新增 `player_top_region` **0.11 s PASS**，既有 R2–R6-02 全部回归保持 PASS。构建日志中的 MSVC `/showIncludes` 中文乱码仍只是控制台编码显示，`WrapVulkanHeaders` 未找到在固定 OpenGL backend 下没有形成阻断。
- `Player.exe` 实机手工验收 **PASS**：启动正常；Floating Window Actions 的最小化、最大化、恢复、关闭全部正常；窗口缩放后 Header/Actions 无错位。当前产品仍无媒体打开入口，因此实际长媒体标题/metadata 没有进行产品运行手工展示；其单行 ellipsis 与无标题降级由既有 `TitleText` contract 和 R6-03 定向测试锁定。真实 fullscreen 进入/退出/Esc/双击交互仍属于 R6-08，不在 R6-03 冒充已完成。
- 本地验证前已有受保护内容 `.gitignore`、`r4-04-qml-diagnostics/` 与 R4-09 1080p/4K JSON；pull/build/test 未覆盖、删除或清理这些内容。没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期、公共播放接口、配置、数据结构或持久化。**R6-03 正式 Complete；下一项 R6-04。**

### 2026-08-14 — R6-02 Video viewport Complete

- `VideoViewport.qml` 新增 `hasMedia` / `hasVideo` 的 presentation 输入契约，默认保持空媒体；`hasMedia && !hasVideo` 映射纯音频，只有 `hasMedia && hasVideo` 才显示 `VideoSurface`。空媒体、纯音频、视频态分别消费既有 `surfaceEmpty` / `surfaceAudio` / `surfaceLetterbox`，没有新增颜色真值或裸色值。
- `VideoSurface.qml` 继续作为 R4 `MpvVideoItem` 的唯一 QML Render Surface owner；`MpvVideoItem` 仍 `anchors.fill: parent`，未迁移进 `PlayerScreen` / `VideoViewport`。视频 underlay 改为既有 `surfaceLetterbox`，aspect-fit 继续由已验证的 R4/libmpv Render 路径保持正确纵横比，R6-02 不复制第二套 QML 裁切/缩放算法。
- 在既有 `tests/unit/presentation/qml/player_screen/` 模块中新增职责独立的 `video_viewport_test.cpp` 与 `player_video_viewport` CTest target，锁定媒体能力输入、空媒体/纯音频/视频三态 semantic background、VideoSurface 可见性以及 `MpvVideoItem` 不得上移到 viewport 的模块边界；既有 `player_screen_structure` 测试未混入 R6-02 断言。
- 当前仓库尚无 `PlayerViewModel` / playback presentation composition，因此本任务建立的是 `PlaybackSnapshot/media capability` 后续绑定所需的 viewport presentation 边界，不提前把 ViewModel/CommandBus/PlaybackSession composition 塞进 R6-02；没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期、公共播放接口、配置、数据结构或持久化。
- 用户 Windows 锁定环境实测：`configure.ps1` **PASS**（CMake Configuring 4.5 s / Generating 1.8 s）；Debug `build.ps1` **PASS**，新增 `player_video_viewport_tests.exe` 正常编译链接并完成 Qt runtime deployment；`scripts/test.ps1` 的 QML lint 门禁完成，全量 **53/53 CTest PASS，0 failed，53.38 s**，新增 `player_video_viewport` PASS，既有 R2–R6-01 全部回归保持 PASS。构建日志中的 `/showIncludes` 中文乱码是已知控制台编码显示问题；`WrapVulkanHeaders` 未找到在当前固定 OpenGL backend 下未形成 configure/build/test 阻断。
- 本地验证前工作区已有受保护内容：`.gitignore` 修改，以及 `r4-04-qml-diagnostics/`、R4-09 1080p/4K JSON 文件；本任务 pull/build/test 均未覆盖、删除或清理这些文件。**R6-02 正式 Complete；R6-03 未开始。**

### 2026-08-14 — R6-01 PlayerScreen composition Complete

- 从已正式关闭的 R5 HEAD `470ea1a58739be59dc4f24ff910a90dfb8c6c506` 创建独立 `agent/r6-stage`，R6-01 只推进播放器页面组合骨架，不提前进入 R6-02 的媒体状态/aspect fit 或 R6-03/R6-04 的真实 Header/OSC 内容。
- 按 R6-01 与第三版 D2 Window System 契约，将 `PlayerScreen` 从旧的 `VideoSurface + PlayerChrome` 两层占位结构拆为职责独立的 `VideoViewport`、`PlayerTopRegion`、`PlayerBottomRegion`、`PlayerOverlayStack`、`PlayerDrawerHost`。`PlayerScreen.qml` 现在只负责 Host 组合、安全边距、尺寸约束和层级，不包含按钮、文字、播放业务或 libmpv 调用。
- `VideoViewport.qml` 继续包裹现有 `VideoSurface`，因此 R4 已验证的 `MpvVideoItem` Render 链没有被复制或改写；本任务不把 `MpvVideoItem` 直接移入 Screen，也不改变 Renderer/Render context 生命周期。
- Top/Bottom/Overlay/Drawer 四个 Host 都只提供 replaceable content slot，并直接消费 R5 已冻结的 `LayoutTokens` / `SpacingTokens` / `ZOrderTokens`。DrawerHost 作为 z50 Inspector overlay 覆盖在视频之上，不通过改变 VideoViewport 宽度挤压视频，符合最终 Figma Player+Inspector 组合规则。
- 已删除被新结构完整替代的 `features/player/chrome/PlayerChrome.qml`。该旧文件同时拥有顶部和底部占位 UI，继续保留会形成重复路径；删除不改变任何已实现业务操作，因为其中只有框架提示文字和玻璃占位矩形。
- 目标结构中的 `states/` 本轮没有创建空文件：R6-01 尚无独立状态 owner，Loading/Buffering/Ended/Error selector 属于后续 R6-11；不为了目录形式制造透明转发或推测状态抽象。当前仓库也尚无 `PlayerViewModel`，因此 R6-01 只建立 `PlayerViewModel → PlayerScreen → child features` 链中的 Screen/Host 边界，不把尚未存在的 VM 伪装为已接通。
- 新增独立 `player_screen_structure` CTest，静态验证 PlayerScreen 只组合五类 Host、Host 使用正确 z-order/slot、VideoViewport 只包装 VideoSurface、Screen/Host 不出现 PlaybackSession/libmpv/mpv_ 业务词，并强制旧 `PlayerChrome.qml` 不再存在。全量测试数由 **51 → 52**。
- 首轮 Windows 验证锁定代码 HEAD `630bc2249b5024408590b7b9fd803daf82bfb5c5`：`configure.ps1` 在真正调用 CMake 前被 `verify-project-layout.ps1` 阻断，原因是旧 scaffold 校验仍把已删除的 `PlayerChrome.qml` 当作必需文件；随后 `build.ps1` 的 CMake 自动重跑正常，Debug build PASS，`scripts/test.ps1` 的 QML lint 门禁完成且无新增 warning/error，全量 **52/52 CTest PASS，0 failed，52.79 s**，新增 `player_screen_structure` PASS；`Player.exe` 实际启动 PASS。
- 同源修复更新 `scripts/verify-project-layout.ps1`：required-files 真值改为 R6-01 五个 Screen Host + `VideoSurface`，旧 `PlayerChrome.qml` 改为 obsolete path；Presentation CMake 校验同步要求五个 Host 已进入 QML module；PlayerScreen composition 校验同步改为新五层结构。没有恢复旧 Chrome，也没有弱化校验。
- 修复后 `configure.ps1` 在锁定 Windows 环境复验 **PASS**：layout verifier 明确识别 R6-01 PlayerScreen composition 完整，CMake **Configuring 3.7 s / Generating 1.7 s**；此前 build/QML lint/52-test 结果保持有效，因为修复只涉及 verifier 与文档。
- 最终手工 basic resize 验证 **PASS**：窗口缩小、放大均正常，内容始终铺满，没有错位或运行时报错；`Player.exe` 再次启动正常。未修改 PlaybackSession、libmpv、Renderer、Render 生命周期、公共播放接口、配置、数据结构或持久化。**R6-01 正式 Complete；R6-02 未开始。**

### 2026-08-14 — R5-10 Accessibility baseline / Stage R5 Complete

- 在既有基础 Controls 上补齐最小可访问性元数据，不建立第二套交互状态：internal `ButtonBase` 统一提供 `accessibleName/accessibileDescription` 输入，并映射 `Accessible.Button`、name/description、focusable、pressed、checkable/checked 与 press action；`Accessible.onPressAction` 继续调用既有 `activate()`，因此鼠标、键盘和辅助功能入口共享同一点击/切换链路。
- `TextButton` / `ToggleButton` 默认从可见 `text` 派生 accessible name，缺少文字时回退既有 tooltip；`IconButton` 默认继承 `ButtonBase` 的 tooltip name，业务 consumer 仍可显式覆写 `accessibleName`。没有根据 `iconId` 猜测本地化名称，避免把资产标识当成用户语义。
- `Slider` 新增显式 `accessibleName/accessibileDescription` 并映射 `Accessible.Slider`、name/description、focusable；现有 Left/Down、Right/Up、pointer、wheel、normalized value 与 signal 行为不变。R5-10 不提前实现高级 screen-reader value/action 适配，符合任务书“高级 screen reader 验证后续补”的边界。
- 既有视觉 Focus 真值不改：Button 继续消费已冻结 focus ring 82%，Slider 继续消费 1.5px / 82% focus ring。为自动化验证给既有 Button focus border 增加内部 `objectName=buttonFocusRing`，不改变 geometry/color/z-order 或用户可观察外观。
- 新增独立 `tests/unit/presentation/qml/accessibility/` 模块，`accessibility_controls` 覆盖 Accessible metadata 声明、Text/Icon/Toggle/Slider semantic name、Tab traversal、disabled control skip 与 Button/Slider focus-visible；测试目录独立于既有 Button/Slider tests，避免继续堆积到单个测试文件。
- 第三版设计任务书的 Accessibility 基线要求 Focus visible、点击目标不因视觉缩小而缩小、Reduce Motion 与可解释键盘导航；现有 R5-06/R5-07 已保留 32/40px Button hit target、32px Slider host、键盘激活/调整和 Reduce Motion，本轮只补缺失的 accessibility metadata 与跨控件 Tab/focus 回归，不修改 Design Token。
- R5-01 Feature import 门禁保持不变；本轮仅修改 Controls 与 accessibility tests，没有新增生产依赖，也没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期、Feature、配置、数据结构或持久化。新增 CTest 后全量测试数 **50 → 51**。
- 首轮锁定 Windows 验证：configure **PASS**（Configuring 5.9 s / Generating 1.8 s）、Debug build **PASS**；build 继续出现已记录的 Qt 6.8.3 `qvariant.h` 生成代码路径 MSVC C4702 warning。CTest 为 **50/51 PASS，1 failed，63.00 s**，唯一失败 `accessibility_controls::tabNavigationKeepsFocusVisible`；Accessible metadata、semantic names、Button Tab focus、disabled skip 均 PASS。失败现场已确认 Slider 获得 `activeFocus` 且 focus border width > 0，但测试在同一时刻同步读取 `opacity`，而 Slider 的 focus opacity 由既有 `Behavior on opacity / NumberAnimation` 过渡，因此读到动画起点 0。
- 修复严格限定在测试时序：将 Slider focus opacity 的同步 `QVERIFY` 改为 `QTRY_VERIFY_WITH_TIMEOUT(..., 1000)`，等待既有 Focus 动画进入可见状态；生产 `Slider.qml`、Motion token、Focus 视觉、公共 API 与交互行为均未修改。
- 修正后锁定 Windows 复验：Debug build **PASS**；`scripts/test.ps1` 的 QML lint 门禁完成；全量 **51/51 CTest PASS，0 failed，53.29 s**，其中 `accessibility_controls`、`feedback_controls` 以及全部既有 49 项回归均 PASS；`Player.exe` 实际启动 smoke **PASS**，无 QML/运行时错误输出。MSVC `/showIncludes` 中文控制台编码乱码继续存在，但没有形成 compiler warning/error 或测试失败；字体目录 warning 仍属于 R5-05 已记录的字体未捆绑限制。
- R5-01 ~ R5-10 已全部完成，R6 所需 Theme / Primitives / Controls / Surfaces / Feedback / Accessibility 基础链齐全，Design System 内仍不包含 libmpv/playback 业务逻辑。**R5-10 正式 Complete；Stage R5 正式 Complete。**

### 2026-08-14 — R5-09 Feedback controls Complete

- 按 R5-09 开发任务和第三版 D5/D8 反馈规则建立独立 `Player.Presentation.Feedback` QML module，物理目录为 `src/presentation/qml/feedback/`；公开类型为 `Toast`、`ErrorFeedback`、`LoadingFeedback`、`EmptyFeedback`，内部 `StatusFeedbackBody` / `ActionFeedbackBody` 只负责复用文本和可选 action 组合，不形成万能反馈组件。
- 四类语义保持严格分层：Toast 是 z80 的非阻断结果玻璃 Surface；Error/Loading/Empty 是 z35 的媒体状态展示内容，其中 Error/Empty 可提供 action，Loading 不拥有 action、Timer 或大型 Spinner 卡片。组件只接受 title/detail/action 等展示输入，不判断错误来源、不读取 PlaybackSession、不调用 libmpv，也不拥有 timeout/dismiss 业务生命周期。
- Toast 没有发明新几何/材质 token，直接复用已经冻结的 `surfaceToast R22`、Popover-grade fill 52% / blur18、control shadow 16px·y5·8%、18px padding、feedback semantic colors 与 `ZOrderTokens.toast=80`。Error/Loading/Empty 复用既有 Typography/feedback colors/overlay z-order；没有新增图标资产或猜测 D5 未暴露的尺寸。
- 设计来源限制已明确核对：第三版 D5/D8 任务书定义了 Feedback Priority 与 Toast/Error/Loading/Empty/HUD/Dialog 的职责，但当前可访问的最终 Figma 文件页只暴露 Framework/D0 节点，未暴露独立 D5 feedback component node。因此 R5-09 只实现任务书明确语义并复用已有冻结 token，不把缺失的 live Figma 几何自行补成“最终设计事实”。
- Feedback module 只向内组合 Theme / Primitives / Controls / Surfaces；R5-01 的 Feature import 门禁完全不变，Feature 仍只允许直接 import Theme/Controls。本任务没有把 Feedback 或 Surfaces 加入 Feature 直连白名单，也没有修改现有公共播放 API。
- `player_qml_lint` 纳入 `player_presentation_feedback_qmllint`；独立 `feedback_controls` CTest 验证四类公开类型可加载且 role 不混用、Loading 无 action、Error/Empty 有 action、Toast 复用冻结 Surface contract，并以静态边界门禁禁止 Feedback QML 出现 Playback/backend 调用词或 `Timer`。
- 锁定 Windows 最终验收：configure **PASS**（Configuring 4.4 s / Generating 1.7 s）、Debug build **PASS**，`scripts/test.ps1` 的 QML lint 门禁完成且无 warning/error；全量 **50/50 CTest PASS，0 failed，52.39 s**，其中 `qml_module_boundaries`、`button_controls`、`slider_controls`、`surface_controls`、`feedback_controls` 全部 PASS；`Player.exe` startup smoke **PASS**，无 QML/运行时错误输出。Build 中 MSVC `/showIncludes` 中文输出存在控制台编码乱码，但没有形成 compiler warning/error 或门禁失败。
- 没有新增生产依赖，没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期、配置、数据结构或持久化。**R5-09 正式 Complete。**

### 2026-08-14 — R5-08 Surface controls Complete

- 重新核对第三版最终 Figma D8 Surface 规则、`Floating Inspector` 真实设计上下文以及本地 Effect Styles/Variables：Inspector 为 **R32 / fill 38% / blur 42 / shadow 42px·y12·12%**；Popover 为 **R20 / fill 52% / blur 18 / shadow 16px·y5·8%**；HUD 为 **R24 / blur 18 / shadow 16px·y5·8%**；Surface stroke 为 **1px**，z-order 保持 Overlay 35 → Inspector 50 → Popover 60 → HUD 70。
- 新增 `surfaces/Panel.qml`、`Drawer.qml`、`Popover.qml`、`Hud.qml`。`Panel` 是唯一通用材质与 replaceable content-slot owner；`Drawer` 复用 Inspector-grade Airy Glass 并只提升到 Inspector 层，不引入旧式黑色 Drawer；Popover/Hud 只覆写各自 Material/Radius/Shadow/Padding/Z semantic，不复制 Surface 渲染实现。
- `Panel` 真实渲染半透明 semantic fill、独立 glass border、圆角和 `QtQuick.Effects.MultiEffect` drop shadow，并通过 `contentItem` + `contentPadding` 承载可替换业务内容。`backdropBlurRadius` 保留最终 Figma role contract，但通用 Surface 不捕获任意祖先背景，也不把自身像素 blur 冒充真实 backdrop blur；实际落地范围为 fill / border / radius / shadow / z / slot。
- Figma 当前没有独立 `alpha/glass/hud-fill` 变量；HUD Effect Style 与 Control Glass 同属 blur18/control-shadow 层级，因此 HUD fill 复用既有 `MaterialTokens.controlFillAlpha = 48%`。R5-08 只新增最终 Figma `stroke/surface` 明确支持的 `LayoutTokens.surfaceBorderWidth = 1`。
- `Player.Presentation.Surfaces` 显式依赖既有 Theme 与锁定 Qt 6.8.3 已使用的 `QtQuick.Effects`，没有新增外部生产依赖；Feature import 门禁保持 R5-01 冻结规则。
- 新增独立 `surface_controls` CTest，覆盖四个公开 Surface role 的材质 contract、replaceable slot/padding、重叠 z-order 与 Surfaces 不依赖 Playback/Controls/Primitives 的静态边界；全量测试数由 **48 → 49**。
- 首轮锁定 Windows 验证：configure **PASS**（Configuring 4.4 s / Generating 1.7 s）、Debug build **PASS**、全量 **49/49 CTest PASS，0 failed，53.13 s**。该轮 surfaces qmllint 同时报告 `Panel.qml` 四条 `Unqualified access` warning，均来自 `layer.effect: MultiEffect` 内对外层 `root` 的引用，因此没有提前按 warning-free 标准关闭。
- 修正严格限定在 lint 根因：`Panel.qml` 增加 Qt 6.8 qmllint 建议的 `pragma ComponentBehavior: Bound`，Surface API、材质数值、slot、z-order 和 Playback/Render 边界均不变。修正后 Windows Debug build **PASS**，`scripts/test.ps1` 的 surfaces qmllint 不再输出上述 warning；全量 **49/49 CTest PASS，0 failed，52.19 s**，`surface_controls` 与全部既有回归均 PASS；`Player.exe` 实际启动 smoke **PASS**，无 QML/运行时错误输出。
- 未修改 PlaybackSession、libmpv、Renderer、Render 生命周期、播放接口、配置或持久化。**R5-08 正式 Complete。**

### 2026-08-13 — R5-07 Slider controls Complete

- 重新核对第三版最终 Figma `Control / Slider` 与 D3 OSC 设计规则：默认组件为 **180×32**，命中区 **132×16**，视觉轨道 **126×3**，值区 **36px** + **12px** gap；Thumb 为 **10 / 12 / 14px（Default/Focus / Hover / Pressed）**，Focus ring **1.5px / 82%**，Disabled **38%**。视觉轨道继续使用既有 `control/track`、`control/progress`、`control/thumb-border` semantic color。
- 新增业务无关 `controls/sliders/Slider.qml`：公开 normalized `value`（0..1）、`stepSize`、`wheelStep`、`showValue/valueText`，只把 pointer/keyboard/wheel 输入转换成 normalized value，并发出 `interactionStarted/valueEdited/interactionFinished/interactionCanceled`；不调用 Seek、Volume、PlaybackSession，也不拥有媒体业务状态。
- Pointer 通过独立 16px hit target 处理 press/drag，视觉轨道保持 3px；键盘 Left/Down 与 Right/Up 使用 `stepSize`，Wheel 使用 `wheelStep`；Disabled 阻断用户输入但不阻止外部程序设置值。根 `value` 越界时收敛到 0..1。
- Slider 默认值文本复用 `TimecodeText.ExtraSmall`（Geist Mono 10 Regular），因此 Controls 继续只经既有 Theme + Primitives 边界消费基础能力；Feature import 规则不变，Timeline/Volume 后续应包装 Slider，而不是复制输入状态机。
- R5-07 首次真实消费补齐 Slider 几何 semantic token：180/132/16/3、10/12/14 thumb、36 value width、12 gap、1px thumb border 与 1.5px focus ring；未改写既有颜色、Opacity、Radius、Motion 真值。
- 新增独立 `slider_controls` CTest，覆盖 Figma 几何、pointer drag、keyboard、wheel、disabled/clamp 与 Reduce Motion 传播；全量测试数由 **47 → 48**。
- 首轮锁定 Windows验证：configure **PASS**（Configuring 4.3 s / Generating 1.6 s）、Debug build **PASS**，既有 Qt 6.8.3 `qvariant.h` C4702 warning 仍存在；`player_qml_lint` 新增一条 `OpacityTokens.disabled` missing-property warning。CTest 为 **47/48 PASS**，唯一失败 `slider_controls`：wheel 用例期望 0.5 + 0.2 = 0.7，但候选错误把 `wheelStep=0.2` 同时当成 quantization grid，结果偏离预期；Disabled 用例则因错误 semantic 名称得到 `undefined`。测试中的 QFontDatabase font-directory warning 与 R5-05 已记录的未捆绑字体 fallback 限制一致。
- 已针对这两处实现缺陷修复：Disabled 改为既有 `OpacityTokens.controlDisabled`；`stepSize` 保持 Slider 值域 quantization grid，`wheelStep` 只作为单次 wheel 输入增量，避免 wheelStep 改写值域离散规则。公开 Slider API、pointer/keyboard/wheel signal、Theme/Primitives/Controls 边界均不变；未修改 PlaybackSession、libmpv、Renderer 或 Render 生命周期。
- 修复后锁定 Windows 复验：Debug build **PASS**；`scripts/test.ps1` 的 QML lint 门禁完成，原 `OpacityTokens.disabled` warning 已消失；全量 **48/48 CTest PASS，0 failed，51.88 s**，其中 `qml_module_boundaries`、`button_controls`、`slider_controls` 均 PASS；`Player.exe` 实际启动 smoke **PASS**，无 QML/运行时错误输出。Qt 6.8.3 `qvariant.h` 的 C4702 warning 仍为已记录的外部工具链 warning，未通过 suppression/白名单掩盖。**R5-07 正式 Complete；R5-08 未开始。**

### 2026-08-13 — R5-06 Button controls Complete

- 重新读取第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 的 D3-04 Transport 与 D3-06 Utility 设计上下文：Secondary/Utility 固定 **32px hit / 22px visual / R16**，Primary Playback 固定 **40px / 24px visual / R20**；交互权重使用 rest 72%、hover 100%、pressed 84%、disabled 38%、focus ring 82%，Primary glass alpha 使用 48/52/42%，hover/focus 使用既有 120ms Ease Out、pressed 0ms，Reduce Motion 继续降为 0ms。
- 新增 `controls/buttons/ButtonBase.qml`、`IconButton.qml`、`TextButton.qml`、`ToggleButton.qml`。内部 `ButtonBase` 统一拥有 pointer hover/press、Space/Enter/Return 键盘激活、focus、disabled gate、toggle/checked 与 tooltip-request 状态；三个公开控件只消费该输入契约并负责各自视觉语义，不复制输入状态机、不持有播放业务状态。
- `IconButton` 支持 Secondary / Primary 两级 emphasis，并暴露 `opticalOffsetX` 给后续 Transport consumer 应用已冻结的 Prev/Play/Next 光学修正；`ToggleButton` 复用 D3-06 open-selection 语义，selection fill 66% 与 selection border 28% 分层实现；`TextButton` 采用低权重 semantic text 状态，不凭空新增品牌色实心按钮变体。
- R5-06 首次真实消费暴露两个基础 token 缺口：补充 `SizePrimitives.size24 → LayoutTokens.playbackIcon` 与 `OpacityPrimitives.focus → OpacityTokens.focusRing`，分别承载 Figma 的 24px Primary glyph 与 82% focus ring；未改写既有 72/84/38/100% 或 Radius/Material/Motion 真值。
- `Player.Presentation.Controls` 现在显式依赖 Theme + Primitives；Feature import 规则保持 R5-01 冻结状态，Feature 仍只直接消费 Theme/Controls，不允许重新出现 Feature→Primitives 深依赖。`player_qml_lint` 已纳入 controls lint。
- Tooltip 本轮提供 `toolTipText + toolTipVisible` 请求/状态契约，没有伪造 Figma 尚未冻结的 Tooltip Surface；真正 overlay 材质与宿主仍由后续 Surface/Feedback 层负责。Primary 控件当前消费玻璃填充/边框/交互 alpha，不在通用 Button 内复制依赖背景捕获的 backdrop-blur 实现。
- 收口静态审查发现并修正 Primary `IconButton` 的 Focus 视觉偏差：`activeFocus` 不再触发 hover glass alpha，键盘 Focus 保持 Figma 冻结的 **48% rest glass + 82% focus ring**；同时 `button_controls` 新增真实 pointer hover/press、Primary hover 52% 与 Focus 48% 的直接断言，避免只测 click 而遗漏视觉状态契约。
- 首轮锁定 Windows 验证：configure PASS、Debug build PASS；build 在 Qt 6.8.3 `qvariant.h` 的 QML 生成代码编译路径出现 MSVC **C4702 unreachable code** warning，属于当前 MSVC/Qt system-header 组合的已知外部工具链告警，项目未新增 suppress/白名单。CTest 为 **46/47 PASS**，唯一失败 `button_controls`；5 个失败用例均在组件解析阶段报 `ButtonBase is not a type`，其他既有 46 项回归全部 PASS。
- 该失败根因是 Controls 的 QML 源位于 `buttons/` 子目录，而 internal `ButtonBase` 与公开按钮没有像已验证的主 Presentation 模块一样统一落到 canonical QML resource root。修复保持源码目录模块化不变：为 Controls QML 文件设置 basename `QT_RESOURCE_ALIAS`，并按 Qt 6.8 对 alias 的约束为该 module 使用 `NO_GENERATE_EXTRA_QMLDIRS`；`ButtonBase` 继续为 internal type，没有扩大为公共 QML API或引入 path/deep import。
- 最终锁定 Windows 验证：configure **PASS**（Configuring 3.6 s / Generating 1.6 s）、Debug build **PASS**、controls qmllint 门禁完成；全量 **47/47 CTest PASS，0 failed，50.62 s**，其中 `qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline`、`typography_primitives`、`button_controls` 均 PASS；`Player.exe` 实际启动 smoke **PASS**，无 QML/运行时错误输出。Qt 6.8.3 system header `qvariant.h` 的生成代码路径仍输出 C4702 warning，未通过 suppression/白名单掩盖，当前不影响 build/test/runtime。没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期或播放接口。**R5-06 正式 Complete；R5-07 未开始。**

### 2026-08-13 — R5-05 Typography primitives Complete

- 重新读取第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 本地 Text Styles；现有 `TypographyTokens` 的 Inter / Noto Sans SC / Geist Mono、字号和字重已经与最终设计一致，因此 R5-05 不复制或改写字体真值。
- 新增 `primitives/text/TitleText.qml`、`BodyText.qml`、`CaptionText.qml`、`TimecodeText.qml`。四类 primitive 分别消费既有 semantic typography/color token，并覆盖 Inspector/Media 标题、Supporting/Control 正文、Meta/Technical/Strong 说明和 M/S/XS Timecode 变体；标题和说明默认单行右侧 ellipsis，Timecode 固定单行并使用 Geist Mono 语义字体。
- 首轮 Windows configure/build 均 PASS；`typography_primitives` 自身 PASS，但全量回归为 **45/46 PASS**。唯一失败是既有 `qml_module_boundaries`：候选把 `PlayerChrome` 直接改为 `import Player.Presentation.Primitives`，违反 R5-01 已冻结的 Feature 仅直接消费 Theme/Controls 的边界。
- 修复没有放宽 R5-01 门禁：撤回 `PlayerChrome → Primitives` 直接依赖并恢复既有 Theme/TypographyTokens 消费，同时删除错误新增的“所有产品 QML 必须直接使用 typography primitive”扫描；`typography_primitives` 继续独立验证 semantic style、长标题 ellipsis 与 Geist Mono Timecode 契约，供后续 Controls/Surfaces 组合使用。
- 修复后 Windows Debug build PASS，`scripts/test.ps1` 的 QML lint 门禁完成且无 warning/error；全量 **46/46 CTest PASS，0 failed，49.49 s**，其中 `qml_module_boundaries` 与 `typography_primitives` 均 PASS；`Player.exe` 实际启动 smoke 无 QML/运行时错误输出。没有新增生产依赖，也没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期或播放接口。字体文件仍未随应用捆绑，沿用 R5-02 已记录的系统 fallback 限制。**R5-05 正式 Complete。**

### 2026-08-13 — R5-04 Complete

- 从第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 直接导出并纳入当前已存在的主操作 glyph：`previous/play/next/volume/subtitles/playlist/fullscreen/close/search`；SVG path 数据未手工重绘。Figma 当前没有独立 Pause/Mute/Exit-Fullscreen 等最终 glyph，因此本任务不伪造缺失资产，未知 `iconId` 由 `Icon.diagnostic` 明确暴露。
- 新增 `Player.Presentation.Primitives/Icon.qml` 与内部 `IconCatalog.qml`：Catalog 只拥有 `icon id → qrc resource / 默认色角色`，Icon 只负责加载、intrinsic size、semantic color 与状态诊断。默认 primary/secondary 颜色映射既有 `ColorTokens.iconPrimary/iconSecondary`。
- 首轮 Windows configure 在 `daf6f17` 暴露 Qt 6.8 硬约束：QML 类型不能同时标记 `QT_QML_SINGLETON_TYPE` 与 `QT_QML_INTERNAL_TYPE`。后续 `rules.ninja` 和 development marker 缺失均是 configure 未完成的连锁结果，不作为独立故障。修复后 `IconCatalog` 改为仅 internal 的普通 QML type，由 `Icon` 内部实例化；公共 `Icon` API、iconId、SVG 资源和着色语义均不变，也没有把 Catalog 扩大为公共 QML API。
- SVG 统一放入 `src/presentation/qml/assets/icons/`，由 `player_presentation_primitives` QML module 以固定 resource alias 打包；Icon semantic tint 使用锁定 Qt **6.8.3** 已包含的 `QtQuick.Effects.MultiEffect`。没有新增外部包或第三方许可证，但 primitives 模块新增对现有 `QtQuick.Effects` QML runtime module 的依赖，Windows 部署需包含既有 `effectsplugin.dll`；用户当前 Qt 安装已确认该模块存在。相比引入 QtSvg/C++ image provider 或复制多套预着色 SVG，该方案保持单一 Figma SVG 真值且只对 22/24 px 图标增加轻量 colorization pass。
- `player_qml_lint` 包含 primitives lint；`icon_pipeline` CTest 验证九个当前 Figma glyph 全部从 qrc 加载、Figma intrinsic 22/24 尺寸、primary/secondary semantic tint、未知 id 诊断、资产清单和产品 QML 不绕过 Icon primitive。
- 锁定 Windows 环境最终实测：全量 **45/45 CTest PASS，0 failed，49.83 s**；`qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline` 均 PASS；`Player.exe` 实际启动无报错。未修改 PlaybackSession、libmpv、Renderer、Render 生命周期或播放接口。**R5-04 正式 Complete。**

### 2026-08-13 — R5-03 Complete

- Radius、Blur/Material、Elevation、Motion、Opacity、Z-order primitive/semantic token 已建立；Reduce Motion 统一将 transition duration 降为 0 并切换 Linear easing，OSC inactivity hide-delay 继续保持 **2200 ms**。
- 新增 `theme_effect_tokens` 合同测试与产品 QML raw radius/z/opacity/duration 回流门禁。
- 锁定 Windows 环境实测已确认：configure、Debug build、无 warning `player_qml_lint`、**44/44 CTest PASS（49.52 s）** 与 `Player.exe` 启动 smoke 均通过。当前仍是播放器骨架界面；Panel/Popover/HUD 等实际 Surface 消费属于 R5-08。**R5-03 正式 Complete。**

### 2026-08-13 — R5-02 Complete

- 按第三版 Airy Glass 最终 Figma 变量/文本样式建立 `ColorPrimitives/ColorTokens`、`TypographyPrimitives/TypographyTokens`、`SpacingPrimitives/SpacingTokens`；为清除现有核心 QML 的裸尺寸，补充职责独立的 `SizePrimitives/LayoutTokens`。
- `Theme.qml` 收敛为兼容 facade；`MainWindow`、`VideoSurface`、`PlayerChrome` 已移除散落的颜色、字体字号、间距和视觉尺寸字面量；窗口默认/最小尺寸行为保持 1280×720 / 960×540。
- 新增 `theme_tokens` 合同测试和产品 QML 硬编码回流门禁，测试总数由 42 增至 43。
- 锁定 Windows 环境实测已确认：configure、Debug build、无 warning `player_qml_lint`、**43/43 CTest PASS** 与 `Player.exe` 启动 smoke 均通过。**R5-02 正式 Complete。**

### 2026-08-13 — R5-01 Complete

- `Theme/Primitives/Controls/Surfaces` 四个公开 QML URI、公开 import 边界和最小模块加载链已稳定。
- 显式 tooling typeinfo 最终候选在锁定 Windows 环境通过 configure、Debug build、development runtime deployment、**无 warning `player_qml_lint`** 和 **42/42 CTest PASS（48.23 s）**。
- `MpvVideoItem was not found`、anchors/objectName 派生 warning、export/meta-object revision warning 均已关闭；运行时继续使用已验证的 `qmlRegisterType<MpvVideoItem>()`。
- `Player.exe` 实际启动 smoke 已确认正常：窗口正常创建，无 QML root/type registration 启动错误。**R5-01 正式 Complete。**

### 2026-08-13 — R5-01 validation fixes

- Windows Qt 6.8.3 / MSVC 已确认 configure 与 build 通过；早期 `qmltyperegistrar` Generate 阻断已消失。
- `qml_module_boundaries` 的 `QtQuick` import path 已通过测试脚本显式注入 `<Qt>/qml` 修复，并在测试后恢复原环境变量。
- 全量回归捕获的 R4-08 late-update 竞态已通过 consumer-side shutdown gate 修复；公共接口和播放状态所有权不变。
- 修复后 Windows 全量回归已达到 **42/42 PASS**；后续仅继续治理 qmllint 的 `MpvVideoItem` tooling metadata。

### 2026-08-12 — R5-01

- 建立 `Theme/Primitives/Controls/Surfaces` 四个公开 QML URI，现有 Theme 使用点改为显式模块 import。
- `Player.Presentation` 保留 App 公共入口，其余现有 Shell/Screen/Feature 类型收为 internal；播放接口、依赖版本和视觉 token 不变。
- Windows configure 暴露 Qt 6.8.3 TARGET-based QML dependency 的 deferred `qmltyperegistrar` 生成失败；已改为 URI dependency 并保留显式 backing-target 链接。
- 新增 QML public-module 最小加载测试；详细记录见 R5 Stage 文档。

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