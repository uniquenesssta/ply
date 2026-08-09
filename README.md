# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器工程。Stage R2：无 UI libmpv 播放核心已经完成；当前进入 Stage R3：领域状态与 PlaybackSession。R3-01 Command/Event、R3-02 PlaybackSnapshot、R3-03 Reducer 与 R3-04 Invariants 已完成；R3-05 PlaybackSession thread 主链已经实现，等待 Windows build/CTest 验收。

## Current baseline

当前仓库已经具备：

- Qt 6.8.3 + C++20 + CMake + Ninja 的模块化工程基线；
- 固定 OpenGL 的 Qt Quick 启动链与真实 OpenGL 探针；
- `RuntimePaths`、文件日志、QML Bootstrap、`ApplicationContainer` 等启动/生命周期边界；
- 模块化 QML 壳：`App.qml -> MainWindow.qml -> PlayerScreen.qml`，QML 不直接调用 libmpv；
- 固定 sibling 路径 `../libmpv/0.41.0/windows-x64` 的 `LibMpv::LibMpv` imported target；
- libmpv 0.41.0 + FFmpeg 8.0.3 + libplacebo 7.351.0 + libass 0.17.4 + FreeType 2.13.3 + FriBidi 1.0.16 + HarfBuzz 10.2.0 的受控源码构建与 manifest/hash 验证；
- R2-01 runtime identity probe；
- R2-02 `MpvHandle` RAII：`mpv_handle` 单一所有权、初始化状态、幂等 close、正确 destroy/terminate-destroy；
- R2-03 `initialization/`：产品 option profile、`config=no`、option 校验、初始化失败清理；
- R2-04 `events/`：wakeup callback 与 Qt 事件循环桥接、所属 Qt 线程 drain、关闭时 callback 失效/注销与在途 callback 收敛；
- R2-05 `commands/`：typed request、命令编码与 `mpv_command_async` 提交边界，request id 原样进入 `reply_userdata`；
- R2-06 `properties/`：核心 property registry、observe/unobserve 生命周期、FLAG/DOUBLE/None 安全 typed decode；
- R2-07 `events/ + errors/`：typed `MpvEvent`、libmpv error mapper、完整核心事件 decode，以及 track/chapter Node 深拷贝为 Qt-owned 数据；
- R2-08 `tools/playback_probe/`：无 QML 的真实 libmpv 控制台探针，复用现有 typed command/event/property 主链；
- R2-09 `events/`：保留完整 end-file typed/raw reason、error 与 playlist 边界元数据，并新增独立事件语义回归矩阵；
- R2-10 `properties/`：Property Baseline 从 11 项扩展到 22 项，统一覆盖 seek/buffering/cache、媒体 identity、当前轨道选择和最小 video/audio 参数入口；
- R2-11 `tools/playback_probe/`：脚本化 8 场景 headless 行为矩阵、运行时生成媒体、typed trace 与自动 CTest 门禁；
- R3-01 `domain/commands + domain/events`：新增强类型 `RequestId`、封闭 PlaybackCommand variant、命令范围校验、PlaybackEvent variant，以及 `MpvPlaybackEventMapper` 后端适配边界；
- R3-02 `domain/state`：新增只读 `PlaybackSnapshot`、`MediaGeneration` 值类型以及 lifecycle/transport/media/timeline/buffering/controls 独立状态轴；共享 `PlaybackFailure` 已从 event payload 中提取到 `domain/errors`；
- R3-03 `domain/state/playback_reducer.*`：新增纯 `Snapshot + Event -> Snapshot` 状态迁移，覆盖媒体加载/结束/失败、transport、timeline、buffering、controls 与 backend shutdown；
- R3-04 `domain/state/playback_invariants.*`：新增无副作用 Snapshot/transition invariant checker，集中检测媒体状态残留、identity 缺失、lifecycle/transport 冲突、失败状态缺失错误、buffering/seeking 非法组合与 generation 回退；
- R3-05 `playback/application/`：新增 `PlaybackSession`、`PlaybackSessionThread`、`PlaybackCommandBus` 与内部 `PlaybackSessionBackend`；所有 Session backend 初始化、mpv command、event drain、Reducer 和 Snapshot commit 均归专用 Playback Thread 主链；
- R3-05 新增 `MpvPlaybackCommandMapper`，只把 Domain `PlaybackCommandPayload` 映射为 R2 typed mpv request，不让 infrastructure 反向链接 Domain 实现；
- Windows 构建后显式 `windeployqt` 与 `.player-development-root` 开发标记；
- R2-08 已由用户 Windows 环境完成最终验收：11/11 CTest 全绿（1.47 秒），真实媒体主链 PASS，非法媒体错误路径返回明确加载失败诊断；
- R2-09 已由用户 Windows 环境完成最终验收：12/12 CTest 全绿（2.31 秒），新增 `mpv_event_semantics` 通过；
- R2-10 已由用户 Windows 环境完成最终验收：13/13 CTest 全绿（2.40 秒），`mpv_properties` 与新增 `mpv_property_baseline` 均通过；
- R2-11 已由用户 Windows 环境完成最终验收：14/14 CTest 全绿（2.22 秒），新增 `playback_probe_matrix` 通过（0.62 秒）；
- R3-01 已由用户 Windows 环境完成最终验收：16/16 CTest 全绿（3.31 秒），新增 `playback_commands`（0.11 秒）与 `playback_events`（0.12 秒）均通过；
- R3-02 已由用户确认验收成功；该次对话未提供逐项 CTest 输出或总耗时，因此 README 不补写未提供的数值；
- R3-03 已由用户 Windows 环境完成最终验收：18/18 CTest 全绿（3.40 秒），`playback_reducer` 0.11 秒通过；首次构建因测试 helper `event(...)` 被 `QObject::event(QEvent*)` 名字隐藏而出现 C2664，重命名为 `makePlaybackEvent(...)` 后构建和完整回归通过；
- R3-04 已由用户 Windows 环境完成最终验收：19/19 CTest 全绿（3.44 秒），`playback_invariants` 0.11 秒通过，开发 runtime marker 正常。

R2-02~R2-11 均已完成并通过对应阶段验收；R2-01 的仓库根 `player.log` 落盘问题继续作为用户明确允许延期的非阻塞诊断缺口保留。当前 **Stage R3：In Progress**；**R3-01：Complete**；**R3-02：Complete**；**R3-03：Complete**；**R3-04：Complete**；**R3-05：Implemented，Windows verification pending**。RequestTracker、StatePublisher、完整 Session shutdown hardening、Generation stale gate 和 supersession 仍按后续 Atomic Task 引入。

## Product scope

首版目标是一个以 libmpv 为播放内核、Qt Quick/QML 为界面的 Windows 10/11 x64 桌面播放器，同时保持 macOS/Linux 清晰适配边界。

MVP 包括：本地文件、网络 URL、播放/暂停/停止、绝对/相对 Seek、Timeline、音量、静音、倍速、全屏、播放列表、音轨、字幕、外挂字幕、音画延迟、章节、媒体信息、错误反馈、加载、缓冲、暂停、播放结束状态、最近播放、恢复进度、快捷键、系统媒体键、防休眠、单实例、文件关联和 Windows 安装包。

第二阶段再做截图、A-B 循环、画面比例、裁剪、旋转、字幕样式、播放质量预设、Shader、迷你播放器、画中画、高级统计和 macOS/Linux 适配。在线站点解析、媒体服务器、DLNA/AirPlay/Chromecast、账号、云同步、在线字幕搜索、插件市场、视频剪辑、转码和 AI 字幕、画质增强不属于首版。

## Architecture boundary

```text
QML presentation
      ↓ intent / projected state
Application layer / PlaybackCommandBus
      ↓ queued command
PlaybackSession (Playback Thread)
      ↓ typed request
libmpv infrastructure
      ↓ public C API
libmpv

libmpv event/property
      ↓
MpvEventLoop / MpvPlaybackEventMapper
      ↓ PlaybackEvent
PlaybackSession → Reducer → PlaybackSnapshot
      ↓
StatePublisher / ViewModel (arrives later in R3/R6)
      ↓
QML
```

当前关键职责：

```text
src/app/
  main/bootstrap/composition
  启动顺序、RuntimePaths、日志、图形探针、QML 与顶层生命周期。
  R3-05 尚未把 PlaybackSessionThread 接入 ApplicationContainer；该 composition wiring 留到真正需要 UI/Application 入口时处理。

src/foundation/ids/
  RequestId 是异步请求的强类型稳定身份；0 保留为 invalid，不承载 generation 或业务状态。

src/foundation/logging/
  日志分类、文件 sink、轮转、脱敏、flush。

src/playback/domain/commands/
  按 load / transport / seek / volume / speed / lifecycle 拆分产品语义命令；
  playback_command.h 只拥有封闭 variant 与 RequestId 公共元数据；
  validation 与 R2 backend 的已接受边界保持一致，不包含 mpv 命令字符串、属性名、QObject 或后端类型。

src/playback/domain/errors/
  PlaybackFailure 是后端无关的领域错误值，可同时被 event payload 和 PlaybackSnapshot 持有；
  event 模块不再拥有错误值类型本身。

src/playback/domain/events/
  按 lifecycle / media / position / buffering / property / failure / command-reply 拆分产品语义事件；
  unavailable property 使用 optional 表达，不使用 mpv 字符串或 generic string property container；
  当前只覆盖 R3-01 所需核心事件，track/chapter/video/audio domain model 留给后续对应任务。

src/playback/domain/state/
  MediaGeneration 定义不可复用媒体代际的强类型值语义，0 为 invalid；R3-05 在 Session 接受新的 Load 时仅负责产生单调递增 generation 并建立当前 source identity；事件/reply 仍未绑定 generation，stale event gate 仍属于 R3-09；
  PlaybackLifecycleState 独立表达 Empty/Opening/Ready/Ended/Failed/Closing；
  PlaybackTransportState 独立表达 Idle/Playing/Paused/Stopped，禁止用 buffering 覆盖用户暂停意图；
  PlaybackMediaState 持有 source/title/path 媒体 identity；PlaybackTimelineState 持有 position/duration/seekable/seeking；PlaybackBufferingState 持有 active/progress；PlaybackControlsState 持有 volume/mute/speed；
  PlaybackSnapshot 私有拥有上述状态并只暴露 const 访问器；`PlaybackSnapshotState` 作为生成新值的复制载体；
  `reducePlaybackSnapshot()` 是纯 Domain reducer；`checkPlaybackSnapshotInvariants()` / `checkPlaybackTransitionInvariants()` 只报告 typed violation，不自动修复。

src/playback/application/command_bus/
  `PlaybackCommandBus` 是跨线程命令入口；先执行 Domain validation，再使用 Qt queued invoke 把命令投递给 Session；不直接调用 mpv、不保存播放真值、不等待 command reply。

src/playback/application/session/
  `PlaybackSession` 是 R3-05 开始的唯一播放 Snapshot owner；它只在所属 Playback Thread 初始化/关闭 backend、接受 queued command、消费 backend PlaybackEvent、调用 Reducer 并 commit Snapshot；
  新 Load 在提交 backend 前先分配新的 MediaGeneration、写入 source 并形成 Opening Snapshot，保证当前媒体 identity 在 mpv 事件到达前已经建立；
  Session 会忽略在当前 lifecycle 下明显无效的基础 mpv property（例如 Empty 状态下的 media/timeline property、非 Ready 状态下的 pause/seeking、非 Opening/Ready 状态下的 buffering），防止 backend 初始 property observation 制造非法 Snapshot；这不是 generation stale filtering；
  `snapshotCommitted` 目前只是 Playback Thread 内部原始信号，供测试和后续 R3-07 StatePublisher 接入；R3-05 不把它直接暴露给 QML/GUI，不做 position throttle；
  `PlaybackSessionThread` 只拥有 QThread/worker 生命周期和 command bus 门面，不暴露 Session 可写状态；当前提供基础 5 秒 stop wait，R3-08 仍负责完整 reject-new-command、pending cancellation、late callback/reply、循环关闭等 shutdown hardening。

src/playback/application/session/backend/
  `PlaybackSessionBackend` 只组合既有 R2 `MpvHandle` / `MpvPropertyObserver` / `MpvEventLoop` / `MpvCommandExecutor`，使用产品初始化 profile，并把 MpvEvent 通过既有 mapper 转成 PlaybackEvent；它不保存 PlaybackSnapshot，不成为第二份播放真值。

src/playback/infrastructure/mpv/commands/
  `MpvPlaybackCommandMapper` 把 Domain command payload 映射到 R2 typed mpv request；它不读取 RequestTracker、不保存 Session 状态；
  `MpvCommandExecutor` 继续只在 QObject owner thread 调用 `mpv_command_async`，request id 原样进入 `reply_userdata`。

src/playback/infrastructure/mpv/events/
  `MpvWakeupBridge` 只把 libmpv 内部线程 wakeup 转为 Qt queued signal；
  `MpvEventLoop` 只在其 QObject 所属线程执行 `mpv_wait_event(handle, 0)` drain；
  `MpvEventDecoder` 在下一次 mpv_wait_event 前把 raw event/payload 深拷贝成内部 MpvEvent；
  `MpvPlaybackEventMapper` 只把明确支持的 typed MpvEvent 转成 PlaybackEvent；raw C payload 不离开 infrastructure。

tools/playback_probe/
  R2 headless probe/8 场景矩阵继续保留并作为 infrastructure 回归；R3-05 新 Session test 不替代这些既有探针。

src/presentation/qml/
  只负责 presentation；禁止直接 mpv_command/mpv_set_property/C 指针访问。
```

## Module growth rule

模块按职责拆分，不按行数拆分。一个已有文件出现第二项可独立描述、独立测试、独立演进的职责时，升级为职责目录；不允许把页面、播放核心全部堆入一个文件，也不允许 `old/new/v2/final/copy` 源码历史副本。Git 负责历史。

## Toolchain and controlled dependency baseline

| Component | Current baseline |
|---|---|
| Windows minimum | Windows 10 22H2 build `10.0.19045` |
| Primary validation family | Windows 11 24H2 / SDK 10.0.26100 family |
| Visual Studio | VS 2022 17.14 family |
| MSVC | v143 14.44 / compiler 19.44, x64 |
| Qt | exact 6.8.3, MSVC 2022 64-bit |
| CMake | minimum 3.30.5 |
| Ninja | minimum 1.12.1 |
| mpv/libmpv | 0.41.0, tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413` |
| FFmpeg | 8.0.3, LGPL-oriented shared build, GPL/nonfree/autodetect disabled |
| libplacebo | 7.351.0 |
| libass | 0.17.4 |
| FreeType | 2.13.3 |
| FriBidi | 1.0.16 |
| HarfBuzz | 10.2.0 |
| C++ | C++20 |

MSYS2 CLANG64 仅作为 libmpv 依赖链的构建工具，不作为 Player 生产运行时目录。最终发布仍需要 R13 的干净机器依赖和许可证扫描。

## Parent-workspace policy

```text
<parent>/
├─ cache/
│  ├─ cmake/fetchcontent/
│  └─ libmpv-build/
├─ downloads/
│  └─ libmpv/
├─ libmpv/
│  └─ 0.41.0/windows-x64/
├─ Qt/
└─ <repository>/
```

提交到仓库的配置不得包含机器专属盘符路径。Qt、libmpv、下载和缓存都通过 `../Qt/...`、`../libmpv/...`、`../downloads/...`、`../cache/...` 解析。

## Build and test

标准 Windows 开发链：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\verify-project-layout.ps1
powershell -ExecutionPolicy Bypass -File scripts\verify-dependencies.ps1
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

已有配置树在普通 Atomic Task 后通常只需：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

Debug 可执行文件：

```text
build/windows-msvc-debug/Player.exe
build/windows-msvc-debug/cmake/playback_probe.exe
```

## Atomic Task status

### R0
- R0-01 ~ R0-05：Complete。
- R0-06：按用户明确要求 Skipped；完整媒体 fixture 合法性闭环未声明完成。

### R1
R1-01 ~ R1-06：Complete。

### R2
R2-02 ~ R2-11：Complete。R2-01 的仓库根 `player.log` 落盘问题继续作为明确的非阻塞诊断缺口保留。

### R3-01 — Complete
用户 Windows 标准 build/test：16/16 CTest 全绿，0 failed，总测试时间 3.31 秒；`playback_commands` 0.11 秒，`playback_events` 0.12 秒。

### R3-02 — Complete
用户明确确认验收成功；对应回合未提供逐项 CTest 输出、总数或耗时，因此不补写未提供数值。

### R3-03 — Complete
首次 Windows build 因 reducer test helper `event(...)` 与 `QObject::event(QEvent*)` 名字隐藏产生 C2664；仅重命名 test helper 后，用户标准 build/test **18/18** 全绿，0 failed，总测试时间 **3.40 秒**，`playback_reducer` **0.11 秒**。

### R3-04 — Complete
用户标准 Windows build/test **19/19** 全绿，0 failed，总测试时间 **3.44 秒**，`playback_invariants` **0.11 秒**，development runtime marker 正常。

### R3-05 — Implemented; Windows verification pending

R3-05 新增真正的 Application 播放主链，但尚未接 UI：

- `PlaybackSession` 是 Snapshot 单一 owner，backend events 只通过它进入 Reducer；
- `PlaybackSessionThread` 创建独立 `PlaybackThread`，Session worker moveToThread 后才初始化 libmpv backend；
- `PlaybackCommandBus` 只做 validation + queued command ingress，GUI/Application caller 不直接调用 Session 内部状态或 mpv；
- `PlaybackSessionBackend` 复用 R2 Handle/Initializer/Observer/EventLoop/Executor，不复制另一套 mpv 核心；
- `MpvPlaybackCommandMapper` 覆盖 load/play/pause/stop/absolute-relative seek/volume/mute/speed；Lifecycle command 仍由 Session 拥有；
- Load 请求在 Session 内分配 generation 并建立 Opening/source 后再提交 backend；generation event/reply tagging、stale event discard 仍未实现；
- 基础 lifecycle property acceptance 避免 Empty/Failed/Closing 等状态被无媒体 property observation 写出明显非法 Snapshot；
- Session 每次 commit 调用 R3-04 invariant checker；发现 violation 只发内部诊断信号，不自动修状态；
- 当前 shutdown 只完成基础 observer/event-loop/handle 关闭和 5 秒 thread wait；R3-08 的完整关闭门禁仍未声明完成；
- 新增真实 `playback_session` CTest，测试运行时生成 3 秒静音 PCM WAV，验证 Playback Thread 初始化、load/pause/play/pause/absolute seek/stop、Snapshot callback thread、0 invariant violation，以及 `PlaybackSessionThread` start/stop smoke；不使用 mock 替代 libmpv 主链。

当前连接环境无法执行用户 Windows Qt/MSVC 构建，因此 R3-05 不声明 Complete。标准 CTest 门禁预计从 19 项增加到 **20/20**。

## Validation record

已由用户 Windows 工作区真实确认：

- CMake 3.30.5、Ninja 1.12.1、Qt 6.8.3、VS 2022 17.14、MSVC 19.44/14.44、Windows SDK 10.0.26100.0 基线通过；
- 完整 libmpv 依赖源码构建成功，最终 package manifest/hash 链通过；
- 自动 Qt runtime deployment 与开发 marker 链通过；
- R2 最终：14/14 CTest 通过；
- R3-01：16/16 CTest 通过，总 3.31 秒；
- R3-02：用户确认验收成功，未提供该次逐项数值；
- R3-03：18/18 CTest 通过，总 3.40 秒，`playback_reducer` 0.11 秒；
- R3-04：19/19 CTest 通过，总 3.44 秒，`playback_invariants` 0.11 秒；
- R3-05：源码/CMake/真实 `playback_session` CTest 已实现；**Windows build 与预期 20/20 CTest 尚未执行，不声明通过**；
- `player.log` 根目录落盘问题仍为单独已知非阻塞缺口，保留到后续相关诊断、发布门禁关闭。

## Change Log

### 2026-08-09

- Accepted R3-04 after the user completed the standard Windows build/test flow: development runtime marker passed, `playback_invariants` passed in 0.11 seconds, and all 19 CTests passed with 0 failures in 3.44 seconds total.
- Implemented R3-05 PlaybackSession application layer with responsibility-separated Session state ownership, QThread host, queued command bus and R2 backend composition.
- Added Domain-command-to-typed-mpv adapter without introducing an infrastructure link dependency on Domain implementation methods.
- Added minimal Session-owned MediaGeneration allocation for new Load identity while explicitly leaving request/event generation tagging and stale-event filtering to R3-06/R3-09.
- Added lifecycle-aware acceptance of backend media properties so initial no-media observations do not create invalid Empty/Opening/Failed/Closing Snapshot combinations.
- Added real `playback_session` CTest using a generated silent PCM WAV to exercise load/pause/play/pause/seek/stop through the complete R3-05 thread/backend/event/reducer chain; expected suite size is 20 tests, pending Windows execution.
- Kept R3-06 RequestTracker, R3-07 StatePublisher, R3-08 shutdown hardening and R3-09 stale generation gate out of R3-05.

## R3-05 local verification

当前 Windows 工作区继续使用 `agent/r3-stage`：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_session
```

标准门禁预计为 **20/20 passed, 0 failed**。当前连接环境未执行该 Windows 门禁，因此这里只记录预期结果。

R2-01 的 `player.log` 落盘缺口继续作为非阻塞诊断事项保留。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Implemented; Windows verification pending。**
