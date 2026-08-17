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
| R6 — 播放器主界面与基础交互 | Complete | **R6-01 ~ R6-16 全部 Complete**；最终 Windows Debug build **124/124**、QML lint **6/6**、**76/76 CTest PASS（71.23 s）**、`Player.exe` startup/exit smoke PASS |
| R7 — 媒体打开与播放列表 | In Progress | **R7-01 / R7-02 / R7-03 / R7-04 / R7-05 / R7-06 Complete**；Playlist Controller、mutation boundary 与 readonly list model 已接通统一 MediaOpenCoordinator/Playback load 主链。Windows Debug configure/build PASS、Quick **86/86 PASS**、完整 **93/93 CTest PASS（73.31 s）**，其中 7 个 `windowed-render` 测试合计 **35.14 s**。R7-07 及之后任务尚未开始 |

R0/R1 属于现有项目基线，R2–R14 快速任务书不重新定义其历史状态。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，仅在明确调用时执行，不阻断后续 Stage。`成熟播放器行为补强与验收矩阵.md` 是跨 Stage 强制补充基线；其中追加的 R6-13 ~ R6-16 已全部完成，Stage R6 已重新关闭。

R2-01 原日志落盘缺口已经由 Pre-R6-09 development diagnostics 正式关闭；当前进一步按启动会话分文件：开发态日志目录仍为项目根上一级 `logs/`，每次 `Player.exe` 启动创建独立 `player-YYYYMMDD-HHmmss.log`，同秒冲突追加 `-02` 等后缀。普通 Installed/Portable 的日志目录规则保持原有语义；单次运行内部仍保留大小轮转，旧 `player.log` 不自动迁移或删除。

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
src/media/domain/                规范化媒体来源值对象
src/media/application/           媒体打开校验与统一 workflow 编排
src/playlist/domain/             播放队列、Entry/current/repeat/shuffle 领域真值
src/playlist/application/        Playlist Controller、结构 mutation 与 load 编排
src/playlist/presentation/       Playlist readonly model 投影
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

日常迭代可使用快速验证；它仍执行依赖检查、测试构建和所有非 `windowed-render` CTest，但跳过会真实创建/显示 `QQuickWindow` 的 R4 Render/DPI/visibility/shutdown 回归。若刚刚已经单独执行并通过 `build.ps1`，可同时使用 `-SkipBuild` 避免重复增量构建：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Quick
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Quick -SkipBuild
```

无参数 `scripts\test.ps1` 的完整验收语义保持不变；`-Quick` **不能**替代 Atomic Task / Stage 收口、Render 相关修改或发布前的完整回归。

Release 构建：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1 -Preset windows-msvc-release
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Preset windows-msvc-release
```

不得把未执行、被阻塞或失败的验证描述为通过；具体 Stage 的验收数字记录在对应 Stage 文档中。

## Change log

### 2026-08-17 — R7-06 Playlist Controller Complete

- 新增独立 `src/playlist/application/` 与 `src/playlist/presentation/`。`PlaylistMutation` 负责结构 mutation 与批量追加回滚；`PlaylistController` 是所有 Playlist 修改和 load 编排的 application boundary；`PlaylistListModel` 只从 controller/domain 读取 snapshot 并通过 Qt roles 投影，未提供 mutation API。Playlist Domain 继续唯一拥有队列顺序/current/repeat/shuffle，PlaybackSession 继续唯一拥有实际已加载媒体状态。
- `MediaOpenCoordinator` 新增批量 source/URL 入口并在提交前完成全部 validation；生产 composition 的单/批媒体提交都改为进入 `PlaylistController`，再复用既有 `PlaybackComposition::submitMediaLoad()`，没有重新引入 UI/Media workflow 直达 Playback 的旁路。多文件 Drop 与多 argv 由此前 deferred 改为真实批量入队，保持输入顺序并选择/加载首个新条目。
- `openSources()` 对新批次采用事务语义：先保存 previous current，再批量 append、选择首个新条目并提交 load；若即时 load submission 被拒绝，则整批回滚并恢复 previous current（无 previous 时清空），避免 queue/current 与实际 load 主链立即分叉。选择其他条目时同样在即时提交失败后恢复 previous current；选择已是 current 的条目不重复 load。
- R7-06 按任务边界只允许删除非 current 条目；删除 current 明确拒绝，避免提前决定 next/stop UX，正式策略继续归 R7-09。稳定 Entry ID 下的 reorder 已由 Domain `move()` + Controller action 接通。R7-07 Playlist QML、R7-08 EOF auto-advance 与 R7-09 current deletion policy 均未提前实现。
- ApplicationContainer 现在显式拥有 Playlist Domain → Mutation → Controller → readonly Model，并把 `playlistController` / `playlistModel` 作为 QML initial properties 暴露；QML 本轮只建立注入边界，没有创建播放列表抽屉。shutdown 顺序保持 consumer 先销毁、其依赖后销毁，未改变 PlaybackSession/Render 生命周期 owner。
- 新增/扩展 `playlist_domain`、`playlist_application`、`playlist_list_model`、`media_open_coordinator`、`media_drop_handler`、`media_argument_open_workflow`、`application_container` 等定向回归，覆盖添加/删除/重排/选择、readonly model、批量顺序、submission rejection rollback 与 current 恢复。验证期间发现 `scripts/test.ps1` 的 Quick 参数数组改造不再包含项目既有 layout verifier 要求的字面 `& $ctest --preset $Preset`，导致 configure 在静态门禁提前失败；修复恢复普通完整分支的该调用，同时 Quick 分支仍显式使用 `--label-exclude "windowed-render"`，没有删除、skip 或弱化任何测试。
- Windows 锁定环境最终验证：`configure.ps1` PASS（CMake Configuring **4.1 s** / Generating **2.8 s**）；Debug `build.ps1` PASS，development runtime marker 与 Qt runtime deployment PASS；`scripts/test.ps1 -Quick -SkipBuild` **86/86 CTest PASS，0 failed，39.79 s**；最终 `scripts/test.ps1 -SkipBuild` **93/93 CTest PASS，0 failed，73.31 s**，其中 7 个真实 `windowed-render` 测试合计 **35.14 s**。R7-06 直接相关 Playlist/Media tests 与既有 Playback/Render/Presentation 全量回归均保持 PASS。没有新增生产依赖。**R7-06 正式 Complete；Stage R7 继续 In Progress；下一任务 R7-07 Playlist QML。**

### 2026-08-17 — R7-05 Playlist Domain Complete

- 新增独立 `src/playlist/domain/` 与 `player_playlist_domain`。`Playlist` 是队列顺序、current entry、repeat mode 与 shuffle-enabled 的唯一领域 owner；`PlaylistEntry` 只绑定稳定 `PlaylistEntryId + MediaSource`，Domain 不依赖 QML、PlaybackSession、libmpv 或 Controller。
- `append()` 只接受有效 `MediaSource` 并分配单调、不回收的稳定 Entry ID；相同媒体允许重复入队但 ID 独立。追加不会隐式选择 current；`select()` 只接受已存在 ID；删除 current 时清空 current，保证没有 dangling current ID；`clear()` 清队列/current 但不回收已发出的 ID，也不重置 repeat/shuffle policy。
- Repeat 领域状态限定为 `Off / One / All`，Shuffle 本轮只建立 enabled 真值；EOF 导航、随机顺序、删除 current 后 next/stop policy、Controller/readonly model/QML 均未提前实现，分别继续归 R7-06/R7-08/R7-09。
- 新增独立 `playlist_domain` CTest，覆盖空列表、追加顺序、稳定/重复 ID、无效来源、选择与删除 current、clear、repeat/shuffle 状态。Windows 锁定环境验证：Debug build PASS；Quick **84/84 PASS**；最终 `scripts/test.ps1 -SkipBuild` **91/91 CTest PASS，0 failed，72.72 s**，其中 `playlist_domain` **0.02 s PASS**、7 个真实 `windowed-render` 测试合计 **34.83 s**，既有 Media/Open、Playback、Render 与 Presentation 回归全部保持 PASS。没有新增生产依赖。**R7-05 正式 Complete；Stage R7 继续 In Progress；下一任务 R7-06 Playlist Controller。**

### 2026-08-16 — R7-04 Command-line and file-association entry Complete

- 新增职责独立的 `media/application/arguments/MediaArgumentParser` 与 `MediaArgumentOpenWorkflow`。Parser 只解析当前进程 `argv`，跳过 `argv[0]`，将本地绝对/相对路径按启动 working directory 规范化为 local `QUrl`，保留 HTTP/HTTPS URL，并支持 `--` 后以 `-` 开头的合法文件名；普通 option-like token 不当作媒体来源。
- `MediaArgumentOpenWorkflow` 不建立第二套 load：单本地路径复用 `MediaOpenCoordinator::openLocalFile()`，单 HTTP/HTTPS 参数复用 R7-03 `UrlOpenWorkflow`；多个来源只保留输入顺序并返回 deferred，在 R7-05 Playlist authoritative queue 建立前不连续 `loadfile`、不让“最后一个覆盖前一个”冒充多文件支持。
- 新增 `StartupMediaOpenScheduler`，只负责在 QML root、VideoOutput/Render binding 与 Qt Quick 首个 `frameSwapped` 已完成后一次性执行 startup argv open；避免在 R7-01 已修复的 RenderContext 建立前提前提交媒体 load。Parser/Workflow 不依赖 Render，后续 R10 第二实例转发可复用 open workflow 而无需复制 Playback load 主链。
- 本任务按任务书只实现**当前进程 argv / 外部路径被交给已启动进程时的消费入口**；没有新增 Windows 注册表文件关联安装、installer association、单实例锁或第二实例 IPC/转发，这些继续归后续 Platform/R10 范围。无新增生产依赖，PlaybackSession、CommandBus、MediaGeneration、Renderer、配置与持久化语义保持不变。
- 新增 `media_argument_parser`、`media_argument_open_workflow`、`startup_media_open_scheduler` 三个独立 CTest。Windows 锁定环境验证：Debug build 与 runtime deployment PASS；Quick **83/83 PASS**。实机以 `Player.exe "F:\081.mp4"` 启动后无需文件选择即自动进入 `playing=true`，无参数启动保持 Empty/`playing=false`，两类运行均正常 exit code 0 且未见 WARN/ERROR/CRITICAL。HTTP/HTTPS argv 的路由由 workflow 定向测试覆盖；本轮未把额外 HTTPS argv smoke 伪报为手工通过。
- 最终 `scripts/test.ps1 -SkipBuild` 完成全量 **90/90 CTest PASS，0 failed，72.49 s**，其中 7 个真实 `windowed-render` 测试合计 **34.78 s**；R7-04 新增三项、既有 R7-01~03 Media/Open、Playback、Render 与 Presentation 回归全部保持 PASS。**R7-04 正式 Complete；Stage R7 继续 In Progress；下一任务 R7-05 Playlist Domain。**

### 2026-08-16 — R7-03 URL Open Complete

- `MediaSource` 增加 `RemoteUrl` 来源类型；新增职责独立的 `UrlMediaValidator` 与 `UrlOpenWorkflow`。Validator 仅负责 trim、严格绝对 URL、host 与 HTTP/HTTPS scheme 校验及规范化，不执行网站解析、可达性探测、阻塞网络请求或重试；Workflow 只执行 `validate → MediaOpenCoordinator::openSource()`，网络媒体继续复用同一 Playback load 主链和既有 Playback Error/Status 失败语义。
- `MediaOpenCoordinator` 增加已验证 `MediaSource` 的统一提交边界，原 `openLocalFile()` 仍先走 `LocalMediaValidator` 后再汇入同一 source submission；没有复制 CommandBus、RequestId、MediaGeneration 或 PlaybackSession owner。R7-02 `MediaDropHandler` 的单个 HTTP/HTTPS URL 已改为进入 `UrlOpenWorkflow`；多个本地/远程来源仍只保序并 deferred，不连续 `loadfile`，Playlist 权威队列继续留给 R7-05。
- 新增 internal `UrlMediaOpenDialog.qml` 与 Empty/Error 状态的独立 `Open URL` intent；`MainWindow` 持有 workflow 注入和 dialog lifecycle，URL dialog 与既有 FileDialog 一起进入 `modalActive`，继续复用 R6 OSC/Cursor lock。Feature QML 只依赖 R5 已冻结的公共 `Theme + Controls` 边界；没有放宽 `qml_module_boundaries`，没有把 Primitives/Surfaces、PlaybackSession 或 backend 类型泄漏进 Feature。URL Dialog 已改用可定制的 Qt Quick Controls Basic `TextField`，先前 native Controls style warning 已关闭。
- 真实远程媒体验证补齐两类故障。第一类是加载悬挂：既有 30 s `RequestTimeoutMonitor` 原先只在 `RequestTracker` 内将过期请求标记 Timeout，不会结束 Snapshot 的 Opening；现已返回完整过期 request record，并只对**当前 MediaGeneration + LoadMedia + Opening**提交 Media failure、关闭旧 generation gate。Loading 提供 `Cancel`，严格复用既有 `PlayerTransportViewModel.requestStop() → TransportAction::Stop → Empty` 主链；Stop/Shutdown 会取消该 generation 未完成的媒体请求，避免迟到 event/reply 重新污染 Empty；Error 同时提供 `Open media` / `Open URL` recovery。新增 `playback_request_timeout_recovery`，并扩展 `player_url_open` / `player_status_overlay` 覆盖 timeout metadata、Cancel 与 Error recovery。
- 第二类是合法 HTTPS MP4 直链被立即判定不可播放。根因定位到 audited FFmpeg recipe 使用 `--disable-autodetect` 却未显式启用 Windows TLS backend；现固定加入 `--enable-schannel`，FFmpeg configure 后必须真实产生 `CONFIG_SCHANNEL 1`，package manifest、`FindLibMpv.cmake` 与 `mpv_runtime_probe` 同步拒绝旧的无 Schannel package。重建后的 FFmpeg 明确启用 `schannel`、`http/https/tls` 并编译 `libavformat/tls_schannel.o`；没有引入 OpenSSL/GnuTLS 等额外生产依赖。LoadMedia backend failure 追加永久诊断，但不记录 URL 本身，避免 query/token 进入日志。
- Windows 锁定环境最终验收：audited libmpv package 重建/验证 PASS；Debug build 与 runtime deployment PASS；Quick **80/80 PASS**；用户以先前同一个 HTTPS MP4 直链实机确认**可以正常打开并播放**。随后最新 `scripts/test.ps1 -SkipBuild` 完成全量 **87/87 CTest PASS，0 failed，73.24 s**，其中 7 个真实 `windowed-render` 测试合计 **35.49 s**，`mpv_runtime_probe`、`url_media_validator`、`url_open_workflow`、`playback_request_timeout_recovery`、`player_url_open`、`player_status_overlay` 以及既有 Playback/Render/Presentation 回归全部保持 PASS。**R7-03 正式 Complete；Stage R7 继续 In Progress；R7-04 尚未开始。**

### 2026-08-16 — R7-02 Drag and Drop Complete

- 新增职责独立的 `MediaDropHandler` 与 screen-local `PlayerDropOverlay`。Drop payload 只负责分类、顺序保留和入口转发；单本地文件继续复用 R7-01 `MediaOpenCoordinator → LocalMediaValidator → MediaSource → Playback`，QML 不直接调用 PlaybackSession/libmpv。
- 多本地文件严格保留原始拖入顺序但不连续提交 load；在 R7-05 Playlist Domain 尚未建立前不制造临时队列或“最后一项覆盖前一项”的假播放列表。目录与不支持 scheme 明确拒绝；R7-02 当时对 URL 只分类/deferred，实际 URL 打开能力归 R7-03。Drop hover 复用既有 `controlsDragActive` / Cursor visibility lock，没有创建第二套 OSC/Cursor owner。
- Windows 锁定环境验证：Debug build 与 runtime deployment PASS；Quick 模式执行 **76/76 PASS**。随后完整 `scripts/test.ps1 -SkipBuild` 执行 **83/83 CTest PASS，0 failed，70.87 s**，其中 7 个真实 `windowed-render` 测试合计 **34.91 s**，`media_drop_handler`、`player_media_drop`、R7-01 Media/Open、Playback、Render 与 Presentation 回归全部保持 PASS。**R7-02 正式 Complete；Stage R7 继续 In Progress。**

### 2026-08-16 — Developer quick validation path

- `scripts/test.ps1` 新增显式 `-Quick` / `-SkipBuild`，默认无参数路径仍执行原完整 CTest gate。`-Quick` 仅通过 CTest label 排除 R4 已稳定且会真实创建/显示窗口的 `mpv_video_renderer`、4 档 `mpv_video_resize_dpi_*`、`mpv_video_visibility`、`mpv_video_shutdown`；Playback、Reducer、Media/R7、Presentation/ViewModel/QML contract、HUD 等其余测试仍执行。没有删除、skip 或放宽任何测试本身，也没有改用 offscreen 平台冒充真实 OpenGL windowed 验证。
- `-SkipBuild` 只在调用者明确指定时跳过 `test.ps1` 内部的重复 `cmake --build`，用于已经刚执行过 `scripts/build.ps1` 的迭代链；脚本仍要求 development runtime marker 和现有测试产物。Windows 实测确认 `-Quick -SkipBuild` 会跳过 7 个 `windowed-render` 测试并执行其余 74 项；迭代过程中曾由真实过期测试契约暴露单项失败，相关契约随后修复，最终无参数完整门禁达到 **81/81 PASS**。最终未单独再执行一次 Quick 74/74，因此不伪报该数字；正式收口仍以无参数完整 `test.ps1` 为准。

### 2026-08-15 — R7-01 Local File Open Complete

- 从已验收 R6 HEAD 创建独立 `agent/r7-stage`。新增 `src/media/domain/MediaSource` 作为规范化媒体来源值对象，并按任务书把 `LocalMediaValidator` 与 `MediaOpenCoordinator` 分离：Validator 只负责本地 URL、存在性、regular-file、可读性与 canonical path 校验；Coordinator 只负责 `validate → MediaSource → submit` workflow，不持有 PlaybackSession、CommandBus、MediaGeneration 或 libmpv 类型。
- `PlaybackComposition` 只增加窄口 `submitMediaLoad(canonicalSource)`，继续复用既有唯一 `PlaybackCommandBus` 与 `PlaybackRequestIdGenerator` 提交 `LoadMediaCommand`；新 `MediaGeneration`、旧请求 supersession/cancellation 与 stale-event gate 仍完全由 `PlaybackSession::beginMediaLoad()` 负责，没有在 R7 MediaOpen 层复制第二套代际或请求仲裁。
- 新增 internal `LocalMediaOpenDialog.qml`，使用 Qt 6.8 自带 `QtQuick.Dialogs FileDialog` 的单文件 `OpenFile` 模式；只有 `accepted` 才发出本地 URL，取消没有 handler、没有副作用。`MainWindow` 只打开 picker 并把选中 URL 交给 Coordinator，同时把 `FileDialog.visible` 传入既有 `PlayerScreen.modalActive`，复用 R6 Controls/Cursor visibility lock；没有新增第二套 modal/OSC owner。`PlayerScreen` / Status Overlay 只转发 `openMediaRequested` intent，QML 不接触 PlaybackSession/libmpv。未引入外部生产依赖；`QtQuick.Dialogs` 已显式进入 QML module dependency。
- R7-01 同时补齐正式产品首次启动的 Empty 入口：`PlaybackLifecycleState::Empty → PlayerStatusKind::Empty → EmptyFeedback`，`PlayerStatusViewModel` 初始状态由默认 `PlaybackSnapshot{}` 经同一 selector 得出，不建立额外 `hasMedia` 真值；Empty action 显示“Open media”，Loading/Buffering/Ended/Error 既有优先级和 owner 不变。
- `PlayerMediaViewModel` 只读消费同一 `PlaybackSnapshot`，仅在 Ready/Ended 且 source 已建立时投影 `hasMedia`；`hasVideo` 最终以 reducer 从 `track-list` 维护的 `snapshot.capabilities().hasVideoTrack` 为媒体能力真值，`streams.video` 继续只表达分辨率/像素格式/旋转等运行时视频参数。`ApplicationBootstrap → MainWindow → PlayerScreen → VideoViewport` 显式传递该投影，不建立第二份播放状态真值。
- `PlayerVideoRenderBinding` 作为 composition bridge，把 Playback backend 发布的不透明 `quintptr` core address 绑定到现有 R4 `MpvVideoItem`；`src/app` 不接触 `mpv_handle` C API 类型，地址到 handle 的转换仍留在 mpv infrastructure。Binding 不拥有 mpv core、`MpvRenderContext` 或播放状态，并继续复用 R4 `MpvRenderShutdownCoordinator` 保证 `beginRenderShutdown → destroy QML/scene graph → wait/prove render release → stop PlaybackSession/core` 的关闭顺序。
- 实机“有声无画面”修复经历两层根因。第一层是 Presentation 曾把 `snapshot.streams().video` 错当 `hasVideo` 启动门槛，已改为 `capabilities().hasVideoTrack`；第二层是产品曾通过 `VideoSurface.visible = hasMedia && hasVideo` 隐藏 Render Surface，导致媒体加载前 scene graph 尚未建立视频 RenderContext。最终 `VideoViewport` 将 Render Surface 生命周期与产品视觉曝光分离：Surface 从应用启动即保持 scene-graph active，空媒体/纯音频继续由 semantic background 覆盖；媒体识别为视频后只撤掉覆盖层，不再延迟创建 Render Surface。该修复不复制 Renderer、不改变 Playback 真值、Reducer 或 libmpv 配置，定位期间加入的一次性 Render INFO 诊断已在收口前删除。
- 新增独立 `media_open_coordinator`、`media_open_playback_integration`、`player_media_open`、`player_media_view_model`、`player_video_render_binding` 五个 CTest target。覆盖 cancel/no-op、非本地、缺失、目录、空格/Unicode canonical path、submission rejection；真实 integration 使用运行时生成的短 WAV 走 `MediaOpenCoordinator → PlaybackCommandBus → PlaybackSessionThread → libmpv`；Presentation contract 锁定 picker/Coordinator/Validator/Viewport/opaque render bridge 与 shutdown order。全量测试数由 **76 → 81**。
- Windows 锁定环境最终验收：用户在 HEAD `01393e6314ba55a40781f250dfae5cfc1ac8ed1c` 拉取后执行 `scripts/build.ps1`，Debug build 完成并成功链接 `Player.exe`，development runtime root marker 与 Qt runtime deployment 均 PASS；随后无参数 `scripts/test.ps1` 完成 **6/6 QML lint**，全量 **81/81 CTest PASS，0 failed，75.25 s**。其中 7 个 `windowed-render` 测试合计 **35.21 s**，`mpv_video_visibility` **22.90 s PASS**、`mpv_video_shutdown` **3.94 s PASS**，`player_video_viewport`、`player_media_open`、`player_media_view_model`、`player_video_render_binding` 与 `media_open_playback_integration` 均 PASS。
- 最终实机本地视频验收已确认：同一入口可正常选择并加载真实视频，音频正常、视频画面真实可见，播放/暂停、Timeline、音量等既有主链保持正常；此前诊断日志确认 libmpv **0.41.0 / FFmpeg 8.0.3**、Desktop OpenGL **4.6 / NVIDIA RTX 3070** 正常初始化并以 exit code **0** 结束。R7-01 只完成单本地文件选择及其必要 Render/Presentation 衔接，没有实现 Playlist Domain、拖放、URL、argv、自动下一项或 R7-02 之后任务。**R7-01 正式 Complete；Stage R7 继续 In Progress。**

### 2026-08-15 — R6-16 HUD Coalescing Complete / Stage R6 Complete

- 在既有 R6-12 `HudMessageQueue` 上补强成熟 HUD 合并策略，继续由该对象独占当前消息、pending queue 与**唯一一个 C++ single-shot `QTimer`**；没有新增第二套 HUD queue/Timer、Playback 真值或新的生产依赖。
- 原隐式 `MessageKind` 收敛为明确 `CoalescingKey + Priority`：Volume/Mute 共用 Volume key；Seek 连续结果共用 Seek key；Speed 连续 +/- 共用 Speed key；Track change 使用独立 Track key，因此 Track 不会与 Volume 合并。相同 key 更新当前或 pending 的最新值，不追加历史重复消息。
- `seekFailed` 作为当前已有的重要 HUD 反馈提升为 Important priority：它可以立即抢占 transient HUD，并清理已经排队的低优先级旧反馈；Important 正在显示时，后续 Volume/Seek/Speed/Track 只能进入 bounded pending，不能覆盖当前重要反馈。Fatal Playback Error 继续由 R6-11 Status Overlay 单一持有；`PlayerScreen` 只把既有 `errorOverlayVisible` 传给 `PlayerHudOverlay.suppressed`，Error 生效时 HUD 立即停止渲染而不是等待淡出，避免 z70 HUD 覆盖更重要的 Error。
- pending queue 保持硬上限 **3**；满队列时优先淘汰最旧 transient，低优先级新消息不能挤掉重要 pending，从策略上禁止无限积压。`clear()` 仍统一停止唯一 hold timer 并清空 current/pending，composition shutdown 语义不变。
- `PlayerHudOverlay` 补齐 `speed` / `track` 的展示 label 并增加 Error suppression，继续复用既有 `Surfaces.Hud`、Typography、Motion 与 z70；没有新增图标资产、QML Timer、Toast/Dialog 或业务命令。当前真实 producer 仍只有成功提交后的 Volume/Mute、Seek 与 Seek failure；Speed 虽有 Domain command 但 R6 尚无 Presentation action，Track selection 属于 R8，因此 `PlaybackComposition` 明确不接入 `showSpeed/showTrackChange`，不伪造未来功能。
- 新增独立 `player_hud_coalescing_policy` CTest，覆盖 Speed burst 合并、Track 与 Volume 分离、Important 抢占与低优先级不可覆盖、bounded pending 淘汰最旧 transient、空未来消息忽略；既有 `player_hud_message_queue` 与 `player_hud_overlay` 继续作为 R6-12 回归门禁。Windows 锁定环境最终验收：`configure.ps1` **PASS**（Configuring **4.7 s** / Generating **2.4 s**）；Debug `build.ps1` 完成 **124/124**，development runtime root marker 与 Qt runtime deployment PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **76/76 CTest PASS，0 failed，71.23 s**。其中 `player_hud_overlay` **1.18 s PASS**、`player_hud_message_queue` **6.38 s PASS**、`player_hud_coalescing_policy` **3.87 s PASS**，既有 Playback/Render/Presentation 回归全部保持通过。
- 随后执行 `Player.exe` startup/exit smoke；`player-20260815-201501.log` 仅包含 INFO，记录 libmpv 0.41.0 / FFmpeg 8.0.3、NVIDIA OpenGL 4.6 初始化，并以 `Application stopping with exit code 0` 正常结束，未见 WARN/ERROR/CRITICAL 或 QML runtime error。验证前 `git status --short` 仍显示受保护的 `.gitignore` 修改与 `r4-04-qml-diagnostics/` 未跟踪目录；本轮验证记录中没有执行 reset/clean。
- 产品媒体打开入口仍属于 R7，因此正式产品 UI 以真实媒体触发 Volume/Seek HUD 的手工矩阵继续在 R7 后回归；Speed 的真实 presentation producer 与 Track change producer 分别留给其所属后续功能阶段，不伪装为 R6 已存在。R6-16 的 Queue/Overlay/priority/bounded backlog/Error suppression 自动化与 startup/runtime 门禁均已通过。**R6-16 正式 Complete；R6-01~R6-16 全部完成，Stage R6 重新关闭为 Complete；下一阶段按任务书进入 R7。**

### Stage R3

- **R3-01~R3-12 Complete；Stage R3 Complete。**
- 建立后端无关播放领域状态、PlaybackSession 单一真值、MediaGeneration stale gate、RequestTracker、cleanup matrix 与 supersession/cancellation。

### Stage R2

- **R2-01~R2-11 完成 Stage 主链；Stage R2 Complete。**
- 建立固定 libmpv 运行链、RAII client、初始化、命令、属性、事件、错误映射与 headless playback probe。
- R2-01 原仓库根 `player.log` 落盘缺口已由 Pre-R6-09 development diagnostics 正式关闭：开发态 `Player.exe` 自主写入项目根上一级 `..\logs\player.log`，Windows 实机验证通过。