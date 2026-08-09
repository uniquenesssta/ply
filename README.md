# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器工程。Stage R2：无 UI libmpv 播放核心已经完成；当前进入 Stage R3：领域状态与 PlaybackSession。R3-01 Command/Event、R3-02 PlaybackSnapshot、R3-03 Reducer 与 R3-04 Invariants 已完成。

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
- Windows 构建后显式 `windeployqt` 与 `.player-development-root` 开发标记；
- R2-08 已由用户 Windows 环境完成最终验收：11/11 CTest 全绿（1.47 秒），真实媒体主链 PASS，非法媒体错误路径返回明确加载失败诊断；
- R2-09 已由用户 Windows 环境完成最终验收：12/12 CTest 全绿（2.31 秒），新增 `mpv_event_semantics` 通过；
- R2-10 已由用户 Windows 环境完成最终验收：13/13 CTest 全绿（2.40 秒），`mpv_properties` 与新增 `mpv_property_baseline` 均通过；
- R2-11 已由用户 Windows 环境完成最终验收：14/14 CTest 全绿（2.22 秒），新增 `playback_probe_matrix` 通过（0.62 秒）；
- R3-01 已由用户 Windows 环境完成最终验收：16/16 CTest 全绿（3.31 秒），新增 `playback_commands`（0.11 秒）与 `playback_events`（0.12 秒）均通过；
- R3-02 已由用户确认验收成功；该次对话未提供逐项 CTest 输出或总耗时，因此 README 不补写未提供的数值；
- R3-03 已由用户 Windows 环境完成最终验收：18/18 CTest 全绿（3.40 秒），`playback_reducer` 0.11 秒通过；首次构建因测试 helper `event(...)` 被 `QObject::event(QEvent*)` 名字隐藏而出现 C2664，重命名为 `makePlaybackEvent(...)` 后构建和完整回归通过；
- R3-04 已由用户 Windows 环境完成最终验收：19/19 CTest 全绿（3.44 秒），`playback_invariants` 0.11 秒通过，开发 runtime marker 校验正常。

R2-02~R2-11 均已完成并通过对应阶段验收；R2-01 的仓库根 `player.log` 落盘问题继续作为用户明确允许延期的非阻塞诊断缺口保留。当前 **Stage R3：In Progress**；**R3-01：Complete**；**R3-02：Complete**；**R3-03：Complete**；**R3-04：Complete**。PlaybackSession、RequestTracker、Generation stale gate 和 supersession 仍按后续 Atomic Task 引入；R3-04 只报告 invariant violation，不自动修复状态、不丢弃事件。

## Product scope

首版目标是一个以 libmpv 为播放内核、Qt Quick/QML 为界面的 Windows 10/11 x64 桌面播放器，同时保持 macOS/Linux 清晰适配边界。

MVP 包括：本地文件、网络 URL、播放/暂停/停止、绝对/相对 Seek、Timeline、音量/静音、倍速、全屏、播放列表、音轨、字幕、外挂字幕、音画延迟、章节、媒体信息、错误反馈、加载、缓冲、暂停、播放结束状态、最近播放、恢复进度、快捷键、系统媒体键、防休眠、单实例、文件关联和 Windows 安装包。

第二阶段再做截图、A-B 循环、画面比例、裁剪、旋转、字幕样式、播放质量预设、Shader、迷你播放器、画中画、高级播放统计和 macOS/Linux 适配。在线站点解析、媒体服务器、DLNA/AirPlay/Chromecast、账号、云同步、在线字幕搜索、插件市场、视频剪辑、转码和 AI 字幕、画质增强不属于首版。

## Architecture boundary

```text
QML presentation
      ↓ intent / projected state
Application layer (PlaybackSession arrives later in R3)
      ↓ PlaybackCommand / PlaybackEvent
Playback domain
      ↓ PlaybackSnapshot
publisher / ViewModel (arrives later in R3/R6)
      ↓
QML

Playback domain
      ↓ adapter boundary
libmpv infrastructure
      ↓ public C API
libmpv
```

当前关键职责：

```text
src/app/
  main/bootstrap/composition
  启动顺序、RuntimePaths、日志、图形探针、QML 与顶层生命周期。

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
  MediaGeneration 只定义不可复用媒体代际的强类型值语义，0 为 invalid；R3-02~R3-04 不负责 generation 分配或 stale event gate；
  PlaybackLifecycleState 独立表达 Empty/Opening/Ready/Ended/Failed/Closing；
  PlaybackTransportState 独立表达 Idle/Playing/Paused/Stopped，禁止用 buffering 覆盖用户暂停意图；
  PlaybackMediaState 持有 source/title/path 媒体 identity，后续 typed tracks/chapters 在该媒体状态边界扩展，不引入 generic QVariant 列表；
  PlaybackTimelineState 持有 position/duration/seekable/seeking；PlaybackBufferingState 持有 active/progress；PlaybackControlsState 持有 volume/mute/speed；
  PlaybackSnapshot 私有拥有上述状态并只暴露 const 访问器；`PlaybackSnapshotState` 作为生成新值的复制载体；
  `reducePlaybackSnapshot()` 是纯 Domain reducer：复制当前 SnapshotState，按 PlaybackEvent 更新拥有的状态轴，再生成新 PlaybackSnapshot；不调用 libmpv、不访问线程/IO、不持有请求表；
  MediaLoadStarted 清旧 title/path/timeline/buffering/failure 但保留当前 source/generation 与会话级 controls；EOF 进入 Ended 并保留媒体 identity/timeline；显式 Stop/Shutdown 清媒体级状态但保留 controls；MediaFailed 保留 source 供错误展示并清旧媒体详细状态；
  Pause 与 Buffering 独立更新，buffering 结束不会把 Paused 猜回 Playing；CoreIdle/EofReached/CommandReply 不在 R3-03 抢 Snapshot 所有权，后续由 Session/RequestTracker 策略消费；
  `checkPlaybackSnapshotInvariants()` 只检查单个 Snapshot 的高价值结构一致性；`checkPlaybackTransitionInvariants()` 只检查跨快照时间不变量；两者返回 typed violation 列表，不修改 Snapshot、不抛业务决策；
  当前 invariants 覆盖 Empty 不得残留 media/timeline/buffering、Opening/Ready/Ended/Failed 必须有有效 generation/source、lifecycle/transport 组合、Failed 必须有 failure、buffering 只能处于 Opening/Ready、seeking 只能处于 Ready 且不能与明确 `seekable=false` 冲突，以及有效 generation 不能回退或重新变 invalid。

src/playback/infrastructure/mpv/runtime/
  libmpv manifest 与真实已加载 DLL/client API 身份探测。

src/playback/infrastructure/mpv/client/
  MpvHandle 唯一拥有 mpv_handle，负责 create/initialize-state/close/destroy。

src/playback/infrastructure/mpv/initialization/
  产品 option profile、option 校验/应用、受控 mpv_initialize 入口。

src/playback/infrastructure/mpv/commands/
  mpv_command_request.h 只定义 load/play/pause/stop/seek/volume/mute/speed typed request；
  MpvCommandEncoder 只把 typed request 转成拥有自身字符串生命周期的 mpv argv；
  MpvCommandExecutor 只在其 Qt owner thread 调用 mpv_command_async，不保存播放真值、不追踪 reply 生命周期。

src/playback/infrastructure/mpv/errors/
  MpvErrorMapper 把 libmpv 原始错误码收敛为内部 MpvError，同时保留 raw code 与诊断文本。

src/playback/infrastructure/mpv/events/
  MpvWakeupBridge 只把 libmpv 内部线程 wakeup 转为 Qt queued signal；
  MpvEventLoop 只在其 QObject 所属线程执行 mpv_wait_event(handle, 0) drain；
  MpvEventDecoder 在下一次 mpv_wait_event 前把 raw event/payload 深拷贝成内部 MpvEvent；
  MpvPlaybackEventMapper 是 R3-01 新增的 infrastructure→domain 适配器，只把明确支持的 typed MpvEvent 转成 PlaybackEvent；
  raw mpv_event、mpv_event_*、mpv_node 指针不离开 infrastructure/mpv。

src/playback/infrastructure/mpv/properties/
  MpvPropertyRegistry 集中拥有 R2 baseline property 名、稳定 observation id 与期望格式；
  MpvPropertyObserver 负责 observe/unobserve 与 Qt owner-thread 生命周期；
  MpvPropertyDecoder 统一负责 FLAG/DOUBLE/STRING/NODE typed decode，并由 Observer 兼容入口与同步 Reader 复用；
  MpvPropertyReader 只在受控 Playback Thread 后端链按 registry 同步读取当前值，STRING/NODE 的 libmpv 分配内存在完成 Qt-owned 深拷贝后立即释放；
  MpvNodeDecoder 把 track/chapter/cache/track-selection/video/audio 等 MPV_FORMAT_NODE 深拷贝为 QVariant/QVariantList/QVariantMap/QByteArray。

tools/playback_probe/
  main.cpp 只处理 CLI 模式选择和进程退出；
  PlaybackProbeRunner 保留 R2-08 单媒体基本链，不与 R2-11 行为矩阵混写；
  PlaybackProbeMatrixRunner 只串行调度 8 个脚本化场景；
  scenarios/ 定义场景数据与通用脚本执行器；
  fixtures/ 只生成 probe-local 短 PCM WAV A/B/EOF 与缺失媒体路径；
  trace/ 只记录 typed command/event/property 顺序并输出有界 trace；
  runtime/PlaybackProbeRuntime 仍是 MpvHandle/Observer/EventLoop/Executor 唯一组合与销毁入口；
  全部探针采用 config=no + vo=null + ao=null，不建立产品 PlaybackSession。

src/presentation/qml/
  只负责 presentation；禁止直接 mpv_command/mpv_set_property/C 指针访问。
```

R2-04 的关闭顺序仍是硬约束：先令 wakeup bridge inactive，再注销 libmpv wakeup callback，等待已经进入 callback 的短路径退出，最后才允许事件循环目标对象销毁；停止后的 queued drain 直接 no-op。

R2-05 保留 `reply_userdata` 作为 R3 RequestId 的后端关联值，但 executor 不建立 pending map、不保存播放状态真值。R3-01 只把 reply userdata 映射为强类型 RequestId；真正的 pending tracking、generation 绑定、duplicate/unknown reply 处理仍归 R3-06/R3-09/R3-12。

R2-06/R2-07/R2-10 的 property 数据链当前为：registry 定义观察身份与格式，observer 只管理 observe/unobserve 生命周期，`MpvPropertyDecoder` 统一执行 typed decode；event decoder 继续通过兼容入口消费 observed property，R3-09 的 `MpvPropertyReader` 则复用同一 decoder 做 FileLoaded 后当前媒体值重同步。FLAG/DOUBLE/STRING/None 都在 infrastructure 内收敛，Node 通过 `MpvNodeDecoder` 深拷贝，STRING/NODE 的同步读取分配在返回上层前释放。

R2-08 只消费上述既有接口。probe runtime 自己拥有 headless option profile 与 mpv 对象生命周期，runner 只根据 typed event/reply 推进 `load -> pause -> play -> relative seek -> stop -> end-file`，每一步都有 15 秒超时；成功/失败通过 queued finalization 等当前 event drain 返回后再关闭 runtime，避免在 `MpvEventLoop::drainPendingEvents()` 栈内销毁 EventLoop；失败路径返回非零退出码并打印 libmpv typed diagnostic。

R2-09 仍只增强 infrastructure 事件契约，不建立媒体真值或业务决策。`MpvEndFileData` 同时保留稳定的 `MpvEndFileReason` 与 libmpv 原始 `rawReason`；已知 reason 继续映射为 EOF/Stop/Quit/Error/Redirect，未知 future reason 保留原始整数值供诊断和后续 R3 生命周期边界判断。decoder 仍不做自动下一项，也不持有当前媒体、generation 或 PlaybackSnapshot。

R2-10 仍只建立 Property Baseline。当前选中的 `aid/sid/vid` 使用 `MPV_FORMAT_NODE`，以保留数字 track id 与 `no` 等原生值形态；`cache-buffering-state` 同样使用 Node，避免依赖显示字符串或不必要的格式假设。`media-title` 与 `path` 使用 `MPV_FORMAT_STRING` 并立即复制到 QString。`demuxer-cache-state`、`video-params`、`audio-params` 使用官方支持的 Node 结构并在 infrastructure 内深拷贝完成生命周期。

R2-11 不新增产品状态。`playback_probe --matrix` 运行时生成三段无第三方版权依赖的短 silent PCM WAV，并通过 `PlaybackProbeScenarioCatalog` 定义 8 个真实 libmpv 场景。`PlaybackProbeScenarioRunner` 只解释 submit/reply/event/property/barrier/shutdown 步骤；所有事件继续来自现有 typed infrastructure。每个场景结束或失败都通过 queued finalization 关闭 runtime，确保不会在 `MpvEventLoop::drainPendingEvents()` 调用栈内销毁事件循环。Trace 最多保留 512 条记录，避免异常事件风暴导致无界增长。R2-11 probe 子目录的头文件统一以 `tools/playback_probe/` 为模块 include root，由 target 私有 include path 提供；不向其他产品 target 暴露该工具内部路径。

R3-01 建立的 Domain 契约不直接复用 `MpvCommandRequest` 或 `MpvEvent` 作为产品模型。`PlaybackCommand` 采用封闭 variant，当前覆盖 load、play/pause/stop、absolute/relative seek、volume/mute、speed、initialize/shutdown；`validatePlaybackCommand()` 只做产品入口的基本值域保护。`PlaybackEvent` 采用按职责拆分的 typed payload；`MpvPlaybackEventMapper` 将 start/file-loaded/end/reply/shutdown、position/duration/pause/seekable/seeking、volume/mute/speed、buffering 和媒体 title/path 映射到 Domain。R3-01 尚不把 track-list/chapter-list/cache/video/audio Node 强行塞入 generic event，而是留给后续明确的 domain model。

R3-02 只建立状态载体，不建立状态迁移副作用。Snapshot 将媒体生命周期、transport、buffering 分成独立轴；`Ready + Paused + buffering=true` 可合法表达，避免缓冲覆盖用户暂停。position/duration/seekable/seeking 与 volume/mute/speed 的未观测值使用 optional，不假定后端尚未确认的真值。MediaGeneration 已作为 Snapshot 字段建立比较和值语义，为后续 invariant/gate 提供类型边界，但本任务没有 generation allocator、RequestTracker、late event filter 或 reducer 分支。

R3-03 只负责纯状态转换。Reducer 不接收 mpv 类型、不执行命令、不决定自动下一项，也不消费 RequestId 副作用；同一个输入 Snapshot/Event 始终产生新的 Snapshot 值。媒体切换和 Stop 清理只清媒体级状态，volume/mute/speed 继续作为会话级控制真值保留。Protocol failure 可记录到 Snapshot failure，但不会伪造 MediaFailed 生命周期；真正的生命周期失败由 MediaFailedEvent 驱动。Generation 仍不参与事件接受/拒绝，避免提前实现 R3-09 stale gate。

R3-04 只负责发现非法状态组合，不负责修复。单 Snapshot 检查与跨 Snapshot generation 检查使用同一 typed violation 枚举，但职责分离：结构检查只看当前值，transition 检查只看 previous/next generation 是否回退。该层不访问 libmpv、不执行命令、不记录日志、不发布 Snapshot；R3-05 PlaybackSession 后续决定如何消费 violation。R3-09 仍负责真正的 stale-event/generation gate，本任务不把“检测 generation 回退”扩展成事件过滤。

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

libmpv 包包含：

```text
../libmpv/0.41.0/windows-x64/
├─ include/mpv/
├─ lib/mpv.lib
├─ bin/libmpv-2.dll + 实际动态依赖
├─ licenses/
└─ dependency-manifest.json
```

`scripts/libmpv/build-package.ps1` 负责受控源码构建，`scripts/libmpv/verify-package.ps1` 独立验证 package identity 与 manifest 中的 artifact SHA-256。用户环境已经验证最终包的 17 个记录 artifact hash 全部通过。

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

`build.ps1` 在成功生成 Player 后写入并验证：

```text
build/<preset>/.player-development-root
../..
```

随后显式执行匹配配置的 `windeployqt`。`test.ps1` 会在运行 CTest 前再次验证开发 marker，避免旧产物伪造绿色结果。`playback_probe` 自己在 POST_BUILD 阶段复制匹配配置的 Qt6 Core runtime，并复用 `player_stage_libmpv_runtime()` 把固定 libmpv runtime/manifest 放到 probe 输出目录，因此可从普通开发终端直接启动。

## Atomic Task status

### R0

- R0-01 ~ R0-05：Complete。
- R0-06：按用户明确要求 Skipped；完整媒体 fixture 合法性闭环未声明完成。

### R1

R1-01 ~ R1-06：Complete。用户 Windows 环境已经验证 configure/build/test、Qt Quick 壳、OpenGL probe、ApplicationContainer 生命周期等 R1 基线。

### R2-01 — Implemented; file-log acceptance deferred

固定 libmpv target、受控源码构建、manifest/hash、runtime probe、Qt runtime staging 与当时 5 个 CTest 均已在用户 Windows 环境验证。`Player.exe` 可以启动并保持响应，Qt/MSVC Debug DLL 从 `build/windows-msvc-debug` 加载。

仍存在一个明确记录的非阻塞缺口：开发模式预期的仓库根目录 `player.log` 没有生成。用户已明确要求不让该问题继续阻塞 R2 普通框架任务。它必须在后续相关诊断、发布门禁前重新关闭，但不计入 R2-02 之后普通 Atomic Task 的局部验收，也不阻塞当前 Stage R2 关闭。

### R2-02 — Complete

`src/playback/infrastructure/mpv/client/` 已实现 `MpvHandle` RAII：

- `mpv_handle` unique ownership；wrapper 不可复制/移动；
- 未初始化 handle 使用 `mpv_destroy`；已初始化 handle 使用 `mpv_terminate_destroy`；
- `close()` 与重复 initialize 安全幂等；
- 初始化失败立即清理，不保留半初始化 owner；
- raw handle 只提供给 mpv infrastructure 后续模块。

用户在 Windows `54f6b16` 运行正常 build/test，6/6 CTest 通过，包括 `mpv_handle` 和 100 次 create/destroy 循环。

### R2-03 — Complete

`src/playback/infrastructure/mpv/initialization/` 已实现：

- `MpvOptionProfile` 集中拥有初始化 option；
- 当前产品默认仅加入确有需要的 `config=no`，避免用户系统 `mpv.conf` 干扰；
- `MpvInitializer` 在 `mpv_initialize` 前逐项调用 `mpv_set_option_string`；
- option name 不能为空，name/value 不允许嵌入 NUL；
- option 应用失败返回明确诊断并关闭未初始化 handle；
- 已初始化/已关闭两类重复调用路径均有确定行为。

用户在 Windows `41bbd7e` 运行正常 build/test，7/7 CTest 全部通过，新增 `mpv_initialization` 通过；总测试时间 1.17 秒。

### R2-04 — Complete

`src/playback/infrastructure/mpv/events/` 已实现 wakeup/event-loop 生命周期：

- `MpvWakeupBridge` 注册 `mpv_set_wakeup_callback`；callback 本身只发出 Qt wakeup signal，不调用普通 libmpv API、不解析事件、不做业务决策；
- wakeup signal 强制 `Qt::QueuedConnection` 投递到 `MpvEventLoop` 所属 Qt 线程；
- `MpvEventLoop::start()` 要求 handle 已初始化并从自身 Qt 所属线程启动；
- 所有 `mpv_wait_event(handle, 0.0)` drain 都发生在该所属线程；
- stop/destructor 保持 callback 注销和在途 callback 收敛顺序。

第 8 个 CTest `mpv_event_loop` 覆盖未初始化拒绝、真实 wakeup、owner-thread drain、重复 start/stop、stop 后抑制、析构关闭顺序。用户在 Windows `56fc1b4` 运行正常 build/test，8/8 CTest 全部通过；总测试时间 1.63 秒。

### R2-05 — Complete

`src/playback/infrastructure/mpv/commands/` 已实现：

- `mpv_command_request.h` 定义 load/play/pause/stop/seek/volume/mute/speed 八类 typed request；
- `MpvCommandEncoder` 统一生成 mpv argv，不把命令字符串散到上层；
- load source、seek、volume、speed 均有边界校验；
- `MpvCommandExecutor` 要求 initialized handle 和自身 Qt owner thread；
- 所有提交使用 `mpv_command_async`，`requestId` 原样进入 `reply_userdata`；
- executor 不维护 pending map、不保存播放状态、不解释 command reply。

第 9 个 CTest `mpv_commands` 验证八类命令编码、非法输入、未初始化、跨线程拒绝和真实异步 reply。用户在 Windows `edbc549` 运行正常 build/test，9/9 CTest 全部通过；总测试时间 1.63 秒。

### R2-06 — Complete

`src/playback/infrastructure/mpv/properties/` 初始实现：

- `MpvPropertyRegistry` 集中拥有首批 11 个核心 property：position、duration、pause、volume、mute、speed、seekable、core-idle、eof-reached、track-list、chapter-list；
- 每个 property 具有稳定 observation id、唯一 mpv name 与明确格式；
- `MpvPropertyObserver` 要求 initialized handle，并要求 start/stop 在自身 Qt owner thread 执行；
- observe 中途失败回滚已注册 observation；start/stop 幂等；
- `MPV_FORMAT_NONE`/null 明确转成 unavailable `std::monostate`；FLAG 解码 bool，DOUBLE 解码 double；
- 错误格式和未知 observation id 返回明确诊断，不崩溃；
- track/chapter `MPV_FORMAT_NODE` 的注册入口由本任务建立，真实深拷贝 decode 在 R2-07 接通。

第 10 个 CTest `mpv_properties` 覆盖 registry 唯一性、未初始化、跨线程拒绝、真实 pause/volume/mute/speed observation、None/null、错误格式、未知 id、Node 入口和重复 start/stop。

用户在 Windows `a0aeff3` 运行正常 build/test，**10/10 CTest 全部通过**，新增 `mpv_properties` 通过；总测试时间 **1.88 秒**。

### R2-07 — Complete

新增并收敛以下职责：

- `src/playback/infrastructure/mpv/errors/` 新增 `MpvErrorMapper`，把 libmpv 公开错误码映射为内部 `MpvErrorCode`，同时保留 raw code 和 `mpv_error_string` 诊断；未知负错误码安全保留为 `Unknown`；
- `events/mpv_event.h` 定义内部 `MpvEvent`、event type、end-file reason 及 start/end/log/unknown/decode-failure payload；
- `MpvEventDecoder` 覆盖 `START_FILE`、`FILE_LOADED`、`END_FILE`、`COMMAND_REPLY`、`PROPERTY_CHANGE`、`LOG_MESSAGE`、`SHUTDOWN`；
- 未知 event 统一变成 `MpvEventType::Unknown`，已知 event 缺少必需 payload 时变成 `DecodeFailure`，均不崩溃；
- `MpvEventLoop` 不再向外发出 `event id/reply userdata/error` 三元 raw metadata，而是在下一次 `mpv_wait_event` 前调用 decoder，发出 Qt-owned `MpvEvent`；
- `MpvPropertyChange` 已提取为独立数据契约，避免 observer QObject 与 typed event 数据相互绑死；
- `MpvNodeDecoder` 支持 NONE/STRING/FLAG/INT64/DOUBLE/NODE_ARRAY/NODE_MAP/BYTE_ARRAY，递归深拷贝到 QVariant 系列并校验无效 list/null pointer/递归深度；
- track-list/chapter-list Node 因此不再持有 libmpv 瞬时内存，后续 R3 可以直接消费内部 event；
- R2-04/R2-05 既有事件循环和命令测试已迁移到 `eventDecoded(const MpvEvent&)`，不再依赖 raw event metadata；
- 没有建立 PlaybackSnapshot、CommandTracker 或 R3 PlaybackSession，也没有新增生产依赖。

第 11 个 CTest `mpv_event_decoder` 覆盖：

- 已知、未知 libmpv error mapping；
- synthetic start/file-loaded/end/command-reply/log/shutdown 解码；
- synthetic track-list Node array/map 深拷贝；
- 未知 event 与 malformed known event 安全路径；
- 测试运行时自行生成一个极短 silent PCM WAV，并使用 test-only `config=no + ao=null + vo=null` 初始化 profile，验证真实 `load -> command reply -> start-file -> file-loaded -> property-change -> end-file/EOF` typed event 链。

该 WAV 由测试代码即时生成，仅用于验证 R2-07 事件链，不引入第三方媒体文件，也不代表被跳过的 R0-06 完整媒体 fixture 策略已经恢复或闭环。

用户在 Windows `9c9f58c` 基线完成 R2-07 本机验收：`scripts/test.ps1` 报告 **11/11 CTest 全部通过**，新增 `mpv_event_decoder` 通过，总测试时间 **2.18 秒**。用户同时在此之前执行了 `scripts/build.ps1`；构建输出被重定向到 `build-r2.log`，本对话未单独读取该日志内容。

### R2-08 — Complete

新增 `tools/playback_probe/`，按职责拆分为：

- `main.cpp`：只负责 `playback_probe <local-media-or-url>` 参数、进程事件循环和退出码；
- `PlaybackProbeRunner`：只负责 load/pause/play/seek/stop/end-file 的异步步骤状态机、request id 关联、逐步输出、15 秒步骤超时，以及 queued finalization；
- `runtime/PlaybackProbeRuntime`：只负责 probe 的 headless option profile、MpvHandle/Observer/EventLoop/Executor 组合、提交入口和关闭顺序。

probe 不访问 raw `mpv_event`/`mpv_node`，不调用散落的 `mpv_command_*`，不保存产品播放真值；它完全复用 R2-02~R2-07 已建立的 typed infrastructure。probe profile 使用 `config=no + vo=null + ao=null`，避免用户 mpv.conf、视频窗口和本机音频设备成为 R2 headless 验收前提。

CMake 新增顶层 `tools/` 构建入口；`playback_probe` 链接 `player_mpv_infrastructure + LibMpv::LibMpv + Qt6::Core`，复用固定 libmpv runtime staging，并额外复制匹配配置的 Qt6 Core DLL，保证用户可直接从 build 输出目录运行探针。

用户在 Windows `0d2fc38` 基线完成 R2-08 本机验收：`scripts/test.ps1` 报告 **11/11 CTest 全部通过**、0 failed，总测试时间 **1.47 秒**；实际 probe 产物位于 `build/windows-msvc-debug/cmake/playback_probe.exe`。本地合法媒体完整通过 `load -> pause -> file-loaded -> play -> seek -> stop -> end-file(reason=stop) -> close` 并输出 PASS。不存在媒体进入 `end-file(reason=error)`，输出 `FAIL: media ended with an error: loading failed`；runner 的失败路径使用默认退出码 2，`main.cpp` 将该退出码传给 `QCoreApplication::exit()`，因此错误路径为非零退出。未观察到 crash、deadlock 或步骤超时。

### R2-09 — Complete

R2-09 只补强 `src/playback/infrastructure/mpv/events/` 的产品无关事件语义，不引入 PlaybackSession、MediaGeneration、自动下一项或其他 R3 业务状态：

- `MpvEndFileData` 新增 `rawReason`，在 typed `MpvEndFileReason` 之外保留 libmpv 原始 end-file reason；未来 libmpv 出现当前代码未知的 reason 时，仍映射为 `Unknown`，但原始整数不会丢失；
- EOF、Stop、Quit、Error、Redirect 继续维持稳定 typed mapping；`endFile->error` 继续独立映射到内部 `MpvError`，playlist entry/insert 元数据保持不变；
- `start-file.playlist_entry_id`、`file-loaded`、`shutdown`、`command-reply.reply_userdata/error`、`property-change` 的既有 typed 信息保持不变；
- null/none property 继续收敛成 `std::monostate`，raw Node 继续在 infrastructure 内深拷贝并结束生命周期；
- decoder 不持有当前媒体真值，也不根据 end-file reason 产生自动下一项等业务决策。

为避免继续膨胀已有 decoder 单测与真实短媒体链，R2-09 新增独立 `mpv_event_semantics_test.cpp` 与第 12 个 CTest `mpv_event_semantics`。该矩阵覆盖 EOF/Stop/Quit/Error/Redirect、未知 future reason、null/none property、command success/error、unknown/malformed event 与 null log string。

用户在 Windows `d6810f4` 基线执行正常 build/test，**12/12 CTest 全部通过，0 failed，总测试时间 2.31 秒**；新增 `mpv_event_semantics` 通过。R2-09 因此正式验收完成。

### R2-10 — Complete

R2-10 在 R2-06 的统一 registry/observer 上补齐后续 R3/R6/R8 明确依赖的 Property Baseline，不建立完整产品媒体模型：

- 保留原 11 个 property 与 observation id `2001~2011` 不变；
- 新增 `seeking`、`paused-for-cache`、`cache-buffering-state`、`demuxer-cache-state`；
- 新增 `media-title`、`path` 作为当前媒体 display/identity 基础入口；
- 新增当前 `aid`、`sid`、`vid` 选择状态；
- 新增 `video-params`、`audio-params`，为后续最小 video/audio 信息和 audio-only 判断提供统一输入；
- 新增 observation id `2012~2022`，registry 总数为 22；
- `MpvPropertyFormat` 新增 `String`，`MpvPropertyValue` 新增 Qt-owned `QString`；STRING event data 按 libmpv `char**` 语义立即深拷贝，null string 安全收敛为 unavailable；
- `aid/sid/vid`、`cache-buffering-state`、`demuxer-cache-state`、`video-params`、`audio-params` 统一使用 `MPV_FORMAT_NODE`，保留原生 int/string/map 类型并通过既有 `MpvNodeDecoder` 深拷贝；
- observer 注册、注销、owner-thread、rollback 和 None/null 语义保持原样；
- 不在 ViewModel/QML/Playlist 新增任何 mpv property 字符串，也不建立 PlaybackSnapshot。

新增独立 `mpv_property_baseline_test.cpp` 与第 13 个 CTest `mpv_property_baseline`，避免继续扩大已有 R2-06 observer 测试职责。该测试锁定 22 项 registry 名称/format/observation id，验证 STRING 深拷贝与 null、数字/`no` 两种 track selection native shape、cache buffering scalar Node、demuxer cache map、video/audio params map。已有 `mpv_properties` 同时会在真实初始化 handle 上注册全部 22 项，因此 Windows 回归可直接验证新 property 集合是否被 libmpv 0.41.0 接受。

用户在 Windows `a67a0ec` 基线执行正常 build/test，**13/13 CTest 全部通过，0 failed，总测试时间 2.40 秒**；`mpv_properties` 和新增 `mpv_property_baseline` 均通过。R2-10 因此正式验收完成。

### R2-11 — Complete

R2-11 在不改变 R2-08 原单媒体探针行为的前提下新增 `playback_probe --matrix`：

- `PlaybackProbeMatrixRunner` 只负责依次调度 8 个场景并汇总 PASS/FAIL；
- `scenarios/playback_probe_scenario.h` 只定义 submit/reply/event/property/barrier/shutdown 脚本步骤数据；
- `PlaybackProbeScenarioCatalog` 只拥有 R2-11 八个场景定义；
- `PlaybackProbeScenarioRunner` 只执行脚本、等待 typed event/reply/property、执行 10 秒步骤超时并按 queued finalization 收尾；
- `fixtures/PlaybackProbeMediaSet` 在临时目录生成 4 秒 A、5 秒 B、350 ms EOF silent PCM WAV，并提供一个保证不存在的 error path；不提交第三方媒体 fixture；
- `trace/PlaybackProbeTrace` 记录 command submit、start/file-loaded/end/reply/property/log/shutdown/unknown/decode-failure 顺序，单场景最多 512 项；
- 原 `PlaybackProbeRunner` 和 `PlaybackProbeRuntime` 职责保持不变，`playback_probe <local-media-or-url>` 的 R2-08 CLI 继续兼容。

八个真实场景严格对应补强任务书：

1. `load -> play -> pause -> seek -> play -> stop`；
2. pause property 已确认 true 时 seek；
3. 不等待中间 reply 连续提交两次不同 seek；
4. load A 后立即 load B，记录 command/event/path replacement 顺序；
5. 同一场景先自然 EOF，再显式 stop，分别要求 `EndFileReason::Eof` 与 `EndFileReason::Stop`；
6. 缺失媒体要求 `EndFileReason::Error`；
7. 收到 `start-file` 后触发 loading 期间 queued shutdown；
8. `file-loaded + play reply` 后触发 playback 期间 queued shutdown。

`playback_probe_matrix` 已直接注册为第 14 个 CTest，并设置 90 秒 CTest 总超时；内部每一步单独 10 秒超时。首次用户 Windows 构建在 CTest 之前失败：MSVC C1083 分别报告 `scenarios/playback_probe_scenario_catalog.cpp` 无法找到 `fixtures/playback_probe_media_set.h`，以及 AutoMOC 编译 `playback_probe_scenario_runner.h` 时无法找到 `runtime/playback_probe_runtime.h`。两者属于同一 CMake target include-root 缺口；随后在 `tools/playback_probe/CMakeLists.txt` 为 `playback_probe` 增加私有 `${CMAKE_CURRENT_SOURCE_DIR}` include root。用户在修复后重新执行标准 build/test，开发 runtime marker 恢复正常，`playback_probe_matrix` 通过，最终 **14/14 CTest 全部通过，0 failed，总测试时间 2.22 秒**，其中行为矩阵耗时 **0.62 秒**。R2-11 因此正式验收完成。

### R3-01 — Complete

R3-01 建立产品语义的 command/event 边界，不提前建立 PlaybackSnapshot、Reducer、PlaybackSession、MediaGeneration 或 RequestTracker：

- `foundation/ids/RequestId` 提供强类型异步请求身份，0 为 invalid；
- `domain/commands/` 按 load、transport、seek、volume/mute、speed、lifecycle 分模块，`PlaybackCommand` 仅拥有封闭 variant 与 RequestId；
- `validatePlaybackCommand()` 覆盖 invalid RequestId、空/NUL source、非有限 seek、负/NaN volume、非正/NaN speed，并与当前 R2 encoder 的后端边界保持一致；
- `domain/events/` 按 lifecycle、media、position、buffering、property、failure、command-reply 分模块，unavailable property 使用 optional 表达；
- `MpvPlaybackEventMapper` 位于 infrastructure/mpv/events，负责把 R2 typed MpvEvent 映射为 backend-neutral PlaybackEvent；Domain 不包含 mpv 类型、property id、property name 或 C API；
- end-file 的 EOF/Stop/Quit/Redirect 转成领域结束原因，Error 转成 MediaFailedEvent；command reply 保留强类型 RequestId 与失败诊断；malformed typed property 转为 protocol failure；log/unknown 与尚未建立 domain model 的 track/chapter/cache/video/audio Node 不伪造 generic domain event；
- 新增两个独立 CTest：`playback_commands` 与 `playback_events`。前者验证命令构造和值域，后者用 synthetic MpvEvent 验证 lifecycle/end/error/reply/core-property/unavailable/malformed/ignored 映射边界。

用户在 Windows 工作区执行标准 build/test，开发 runtime marker 校验通过，`playback_commands` 0.11 秒通过、`playback_events` 0.12 秒通过；连同全部 R2 回归，最终 **16/16 CTest 全部通过，0 failed，总测试时间 3.31 秒**。R3-01 因此正式验收完成。

### R3-02 — Complete

R3-02 建立产品只读播放快照和值类型，不实现 reducer/session 副作用：

- `domain/state/MediaGeneration` 提供单调代际所需的强类型可比较值；0 为 invalid，本任务不负责生成新 generation；
- lifecycle 与 transport 独立：`PlaybackLifecycleState` 覆盖 Empty/Opening/Ready/Ended/Failed/Closing，`PlaybackTransportState` 覆盖 Idle/Playing/Paused/Stopped；
- `PlaybackMediaState` 保存 source/title/path；timeline 保存 position/duration/seekable/seeking；buffering 保存 active/progress；controls 保存 volume/mute/speed；未确认的后端属性用 optional 表示；
- `PlaybackSnapshot` 私有持有 `PlaybackSnapshotState`，只暴露 const 读取接口；默认、opening、stopped 三类基础快照可直接构造；
- stopped 快照保留 generation，但清空媒体 identity、timeline、buffering 和 failure，从而给后续 late-event gate 保留代际边界而不把旧媒体数据继续暴露给 UI；
- `PlaybackFailure` 从 `events/failure_event.h` 提取到独立 `domain/errors/playback_failure.h`，事件 include 路径仍兼容，避免 state 依赖 event 模块；
- track/chapter 不使用 placeholder QVariant/generic list；后续 typed descriptor 会沿 `PlaybackMediaState` 责任边界扩展；
- 新增独立 CTest `playback_snapshot`，覆盖默认/Opening/Stopped、独立 Pause+Buffering 轴、媒体/timeline/control 字段和 typed failure。

用户明确确认 R3-02 验收成功。由于对应回合没有提供该次 Windows CTest 的逐项输出、总数或耗时，本 README 只记录验收结论，不补写未提供的数值。

### R3-03 — Complete

R3-03 新增纯 `reducePlaybackSnapshot(current, event)`，只负责 Domain 状态转换：

- `MediaLoadStartedEvent` 将 lifecycle 置为 Opening，清除旧 title/path/timeline/buffering/failure，同时保留当前 source、generation 与 volume/mute/speed；
- `MediaLoadedEvent` 将 lifecycle 置为 Ready；Pause property 独立决定 Playing/Paused，不与 buffering 合并；
- position/duration/seekable/seeking、title/path、volume/mute/speed 各自只更新所属状态轴；不可用的 pause/buffering 不猜测新真值；
- buffering 结束只关闭 buffering 并清 progress，不改变 transport，因此 Paused + Buffering 的成熟播放器语义保持成立；
- EOF/Unknown end 进入 Ended + Stopped 并保留媒体 identity/timeline；显式 Stop/Shutdown end 清空媒体级状态但保留会话 controls；Redirect 回到 Opening 并丢弃旧媒体详细状态；
- `MediaFailedEvent` 进入 Failed + Stopped，保留 source 供错误展示，清除旧 title/path/timeline/buffering 并保存 typed failure；
- backend shutdown 进入 Closing + Stopped；Protocol `PlaybackFailureEvent` 记录诊断但不伪造 MediaFailed 生命周期；
- CoreIdle/EofReached/CommandReply 在 R3-03 不直接改变 Snapshot，避免 reducer 抢占 Session/RequestTracker 的后续职责；
- 不实现 generation stale event gate、request supersession、invariant 修复或 Session 副作用。

新增独立 CTest `playback_reducer`，覆盖 load/file-loaded、play/pause、seek/timeline、buffering 与 pause 独立、EOF/stop/redirect/error、shutdown、unavailable property、非 Snapshot-owner event 与 protocol failure。

首次 Windows build 在编译 `playback_reducer_test.cpp` 时失败：测试类继承 `QObject`，命名空间辅助函数也命名为 `event(...)`，MSVC 在成员函数调用处优先解析到 `QObject::event(QEvent*)`，导致 Domain event payload 全部报 C2664。修复仅将 test helper 重命名为 `makePlaybackEvent(...)`，未修改 Reducer、Domain API 或产品行为。用户随后重新执行标准 build/test，开发 runtime marker 正常，最终 **18/18 CTest 全部通过，0 failed，总测试时间 3.40 秒**，其中 `playback_reducer` **0.11 秒**。R3-03 因此正式验收完成。

### R3-04 — Complete

R3-04 集中建立少量高价值 Snapshot invariants，不自动修复状态：

- 新增 `PlaybackInvariantViolation` typed enum 与 `PlaybackInvariantViolations` 结果集合；
- `checkPlaybackSnapshotInvariants(snapshot)` 检查当前 Snapshot 结构一致性；
- Empty lifecycle 不允许残留 media identity、timeline 或 buffering；
- Opening/Ready/Ended/Failed 必须拥有有效 `MediaGeneration` 与非空 source；
- lifecycle 与 transport 必须落在允许组合：Empty=Idle/Stopped、Opening=Idle、Ready=Idle/Playing/Paused、Ended/Failed/Closing=Stopped；
- Failed lifecycle 必须携带 failure；buffering active 只能位于 Opening/Ready；seeking=true 只能位于 Ready，且不能与明确 `seekable=false` 同时存在；
- `checkPlaybackTransitionInvariants(previous, next)` 单独检测 generation 回退：一旦 previous generation 有效，next 不得变 invalid，也不得数值下降；
- checker 只返回 violation，不修改 Snapshot、不抛业务决策、不接触 libmpv/线程/IO；真正的 stale-event filtering 仍归 R3-09；
- 新增独立 CTest `playback_invariants`，同时验证 canonical Snapshot、Reducer 主路径输出、非法结构组合与 generation same/increase/regression。

用户在 Windows 工作区完成标准 build/test，开发 runtime marker 校验通过，`playback_invariants` **0.11 秒**通过；连同此前全部回归，最终 **19/19 CTest 全部通过，0 failed，总测试时间 3.44 秒**。R3-04 因此正式验收完成。

## Validation record

已由用户 Windows 工作区真实确认：

- CMake 3.30.5、Ninja 1.12.1、Qt 6.8.3、VS 2022 17.14、MSVC 19.44/14.44、Windows SDK 10.0.26100.0 基线通过；
- 完整 libmpv 依赖源码构建成功，FFmpeg 8.0.3 官方签名验证成功，最终 package 17 个 manifest artifact hash 全部通过；
- 自动 Qt runtime deployment 与开发 marker 链通过；
- R2-02：6/6 CTest 通过；
- R2-03：7/7 CTest 通过；
- R2-04：8/8 CTest 通过；
- R2-05：9/9 CTest 通过；
- R2-06：10/10 CTest 通过，`mpv_properties` 通过，总测试时间 1.88 秒；
- R2-07：11/11 CTest 通过，`mpv_event_decoder` 通过，总测试时间 2.18 秒；
- R2-08：11/11 CTest 回归通过，总测试时间 1.47 秒；合法媒体 probe PASS；不存在媒体得到 `end-file(reason=error)` + `loading failed`，失败链返回非零退出码；
- R2-09：12/12 CTest 通过，`mpv_event_semantics` 通过，总测试时间 2.31 秒；
- R2-10：13/13 CTest 通过，`mpv_property_baseline` 通过，总测试时间 2.40 秒；
- R2-11：首次 Windows build 的 C1083 include-root 问题修复后，标准 build/test 成功进入 CTest，`playback_probe_matrix` 0.62 秒通过，最终 **14/14 CTest 全部通过，0 failed，总测试时间 2.22 秒**；
- R3-01：标准 Windows build/test 通过；`playback_commands` 0.11 秒、`playback_events` 0.12 秒，最终 **16/16 CTest 全部通过，0 failed，总测试时间 3.31 秒**；
- R3-02：用户确认验收成功；对应回合未提供逐项 CTest 输出/总耗时，因此不补写数值；
- R3-03：首次 build 因 reducer test helper `event(...)` 与 `QObject::event` 名字隐藏产生 C2664；重命名 helper 后标准 build/test 通过，开发 runtime marker 正常，`playback_reducer` 0.11 秒，最终 **18/18 CTest 全部通过，0 failed，总测试时间 3.40 秒**；
- R3-04：标准 Windows build/test 通过，开发 runtime marker 正常，`playback_invariants` 0.11 秒，最终 **19/19 CTest 全部通过，0 failed，总测试时间 3.44 秒**；
- `Player.exe` 可正常启动并保持响应；
- `player.log` 根目录落盘问题仍为单独已知非阻塞缺口，保留到后续相关诊断、发布门禁关闭。

R2-09~R2-11 跨阶段补强任务均已完成；Stage R2 正式 Complete。Stage R3 当前为 In Progress，R3-01、R3-02、R3-03、R3-04 正式 Complete。

## Change Log

### 2026-08-09

- Accepted R3-04 after the user completed the standard Windows build/test flow: the development runtime marker passed, `playback_invariants` passed in 0.11 seconds, and all 19 CTests passed with 0 failures in 3.44 seconds total.
- Accepted R3-03 after the user reran the standard Windows build/test flow: the development runtime marker passed, `playback_reducer` passed in 0.11 seconds, and all 18 CTests passed with 0 failures in 3.40 seconds total.
- Recorded the first R3-03 Windows build failure accurately: reducer test helper `event(...)` was hidden by inherited `QObject::event(QEvent*)`, causing C2664 for every Domain event payload; the fix renamed only the test helper to `makePlaybackEvent(...)` and did not change product code.
- Implemented R3-04 pure Snapshot/transition invariant checking with typed violations for stale media state, active-media identity, lifecycle/transport consistency, failure presence, buffering/seeking combinations and generation regression.
- Added independent `playback_invariants` CTest including canonical-state checks, reducer-output cross-checks and illegal snapshot/transition cases.
- Accepted R3-02 from the user's explicit acceptance confirmation; the user did not provide the corresponding CTest detail/timing in that turn, so no unprovided test counts or elapsed time were added.
- Implemented R3-03 pure `PlaybackSnapshot + PlaybackEvent -> PlaybackSnapshot` reducer with media lifecycle cleanup, independent transport/buffering axes, timeline/control updates and typed failure handling.
- Preserved session-level volume/mute/speed across media replacement/stop while clearing media-scoped identity details and timeline where required.
- Kept CommandReply/CoreIdle/EofReached outside Snapshot ownership and did not implement R3-05 Session, generation stale filtering or request supersession early.
- Implemented R3-02 immutable `PlaybackSnapshot` value model with separate lifecycle, transport, media, timeline, buffering and controls state axes; no reducer or Session side effects were introduced.
- Added strong `MediaGeneration` value semantics to the Snapshot boundary without generation allocation or stale-event filtering; those remain later R3 tasks.
- Promoted `PlaybackFailure` to a shared Domain error value so both events and Snapshot can use it without a state→event dependency.
- Added the independent `playback_snapshot` CTest for default/opening/stopped snapshots, pause+buffering coexistence, scalar state preservation and typed failure.
- Accepted R3-01 after the user completed the standard Windows build/test flow: the development runtime marker passed, `playback_commands` passed in 0.11 seconds, `playback_events` passed in 0.12 seconds, and all 16 CTests passed with 0 failures in 3.31 seconds total.

### 2026-08-08

- Started Stage R3 on a dedicated `agent/r3-stage` branch from the accepted R2 head.
- Implemented R3-01 product-semantic playback command contracts with a strong `RequestId`, responsibility-separated load/transport/seek/volume/speed/lifecycle payloads and centralized validation matching the accepted R2 backend bounds.
- Implemented R3-01 typed PlaybackEvent contracts for lifecycle, media, position, buffering, core property, failure and command-reply semantics without leaking mpv property strings or C types into Domain.
- Added `MpvPlaybackEventMapper` as the infrastructure-to-domain adapter for typed R2 events；unsupported future track/chapter/cache/video/audio domain models remain intentionally unmapped instead of entering a generic string/property container.
- Added independent `playback_commands` and `playback_events` CTests；Windows suite size is now 16 tests.
- Accepted R2-11 after the user reran the standard Windows build/test flow: the development runtime marker recovered, `playback_probe_matrix` passed in 0.62 seconds, and all 14 CTests passed with 0 failures in 2.22 seconds total.
- Closed Stage R2 after R2-09~R2-11 mature-player behavior supplements were all implemented and verified；the previously recorded repository-root `player.log` issue remains an explicit non-blocking diagnostic gap for a later gate.
- Accepted R2-10 after the user confirmed the real Windows environment passed all 13 CTests, including `mpv_property_baseline`, in 2.40 seconds total.
- Implemented R2-11 as a modular scripted `playback_probe --matrix` path while preserving the accepted R2-08 `playback_probe <source>` behavior.
- Added separate generated-media fixture, scenario catalog/executor, matrix orchestration and bounded typed trace responsibilities instead of extending the existing R2-08 runner into a multi-purpose state machine.
- Added all eight required R2-11 real libmpv scenarios: transport sequence, paused seek, consecutive seek, immediate A->B replacement, EOF/stop distinction, load error, shutdown during loading and shutdown during playback.
- Kept shutdown scenarios on queued teardown so the runtime is never destroyed from inside the synchronous `MpvEventLoop` drain signal stack.
- Registered `playback_probe --matrix` as the 14th CTest `playback_probe_matrix`；Windows regression gate is 14/14.
- Kept generated WAV files temporary and probe-local；R0-06 remains skipped and no distributable external fixture policy is falsely claimed complete.
- Recorded the first R2-11 Windows build failure before CTest：MSVC C1083 could not resolve probe-root `fixtures/...` and AutoMOC `runtime/...` includes because the target had no module include root.
- Fixed that single build-system root cause by adding `${CMAKE_CURRENT_SOURCE_DIR}` as a PRIVATE include directory of `playback_probe`；no scenario logic or production interface changed.
- Accepted R2-09 after the user confirmed the real Windows environment passed all 12 CTests, including `mpv_event_semantics`, in 2.31 seconds total.
- Implemented R2-10 Property Baseline expansion from 11 to 22 centralized registry entries while preserving the original observation IDs and all existing observer lifecycle behavior.
- Added centralized seeking/cache/media identity/current track/video/audio property entries；flexible native values remain Node-backed and raw property strings do not escape `infrastructure/mpv`.
- Added STRING property decoding into a Qt-owned `QString` value and explicit null-string unavailable handling.
- Added the independent `mpv_property_baseline` CTest for the R2-10 registry/format/deep-copy contract.
- Kept R2-10 inside infrastructure only: no PlaybackSnapshot, ViewModel property lookup, Playlist state, new production dependency, configuration change or user-visible playback behavior was introduced.
- Implemented R2-09 event-semantics hardening: `MpvEndFileData` now preserves the raw libmpv end-file reason alongside the stable typed reason, error and playlist boundary metadata.
- Added a separate `mpv_event_semantics` CTest instead of further growing the existing decoder/integration test；it covers all known end reasons plus unknown raw reason retention, null/none property semantics, command success/error identity, unknown/malformed event and null log strings.
- Kept R2-09 strictly inside the mpv infrastructure boundary: no PlaybackSession, MediaGeneration, auto-advance decision, new production dependency, configuration change or user-visible playback behavior was introduced.
- Accepted R2-07 after the user confirmed the real Windows environment passed all 11 CTests, including `mpv_event_decoder`, in 2.18 seconds total.
- Implemented R2-08 modular console playback probe with separate CLI, orchestration state machine and mpv runtime-lifecycle ownership.
- Added the real headless control sequence `load -> pause -> play -> relative seek -> stop -> end-file -> close`, per-step diagnostics and 15-second timeouts without adding PlaybackSession or product playback state.
- Deferred probe runtime teardown through Qt queued finalization so an `eventDecoded` callback never destroys `MpvEventLoop` while its drain stack is still active.
- Added a probe-only `config=no + vo=null + ao=null` profile and direct Qt6 Core/libmpv runtime staging so the console probe does not depend on a QML window, user mpv.conf or physical audio output.
- Accepted R2-08 after the user confirmed all 11 CTests still passed in 1.47 seconds, a real local media probe completed the full headless control chain with PASS, and a missing-media probe produced `end-file(reason=error)` with `loading failed` and a non-zero failure exit path.
- Corrected the documented playback probe output path to `build/windows-msvc-debug/cmake/playback_probe.exe`；no source, public-interface, dependency or runtime behavior change was required for this correction.
- Corrected the premature R2 closure: the mandatory mature-player behavior supplement adds R2-09 through R2-11, so Stage R2 remained open until those tasks were verified.
- Kept the accepted R2-08 Windows results unchanged；this correction changed documentation/status only and did not modify source, dependencies, interfaces or runtime behavior.
- Accepted R2-06 after the user confirmed the real Windows build and all ten CTests passed, including `mpv_properties`, in 1.88 seconds total.
- Implemented R2-07 typed event boundary: raw `mpv_event` is decoded inside `infrastructure/mpv` into Qt-owned `MpvEvent` before the next `mpv_wait_event` call.
- Added `errors/MpvErrorMapper` with known/unknown libmpv error mapping and preserved raw diagnostics.
- Added recursive Node deep-copy for track/chapter payloads, eliminating libmpv node-pointer lifetime from the outward event contract.
- Migrated the R2-04 event loop and R2-05 command-reply tests from raw metadata signals to typed `MpvEvent` delivery.
- Added the eleventh `mpv_event_decoder` CTest with synthetic event/error/Node cases plus a runtime-generated silent PCM WAV for a real short-media typed event sequence.
- Kept the generated WAV strictly test-local；R0-06 remains skipped and no complete external media-fixture policy is claimed.
- Accepted R2-05 after the user confirmed all nine CTests passed, including `mpv_commands`, in 1.63 seconds total.
- Implemented R2-06 centralized property registry/observer with owner-thread lifecycle, rollback, None/null safety and typed FLAG/DOUBLE decoding.
- Accepted R2-04 after the user confirmed all eight CTests passed, including `mpv_event_loop`, in 1.63 seconds total.
- Implemented R2-05 typed async command request/encoder/executor boundaries.
- Accepted R2-03 after the user confirmed all seven CTests passed, including `mpv_initialization`.
- Implemented R2-04 wakeup bridge/event-loop ownership and shutdown protection.
- Accepted R2-02 after the user confirmed six CTests passed, including the `MpvHandle` lifecycle/100-cycle regression.
- Implemented R2-03 initialization profile with centralized `config=no`, pre-initialize option application, validation and failure cleanup.
- Recorded the unresolved R2-01 repository-root `player.log` as a non-blocking validation gap by explicit user direction.
- Completed the controlled libmpv source-build/package verification chain and explicit Qt runtime deployment path described above.

### 2026-08-07

- Started Stage R2 after accepted R1-06.
- Added the project-controlled MSYS2 CLANG64 libmpv source-build pipeline and fixed source/archive acquisition boundaries.
- Kept all third-party binaries/build trees outside Git and preserved repository-parent-relative dependency paths。

## R2 final Windows verification

R2-11 行为矩阵已接入标准 CTest，不需要手工准备第二个媒体文件。最终 Windows 验收使用：

```powershell
git pull --ff-only origin agent/r2-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r2.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

实际结果：

```text
playback_probe_matrix ............ Passed    0.62 sec
100% tests passed, 0 tests failed out of 14
Total Test time (real) = 2.22 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

R2-01 记录的仓库根 `player.log` 落盘问题仍是明确的非阻塞缺口；它没有被本次验收伪装为已解决，后续仍需在相关诊断、发布门禁前单独关闭。

## R3-01 Windows verification

用户在 `agent/r3-stage` 执行标准 Windows build/test，实际结果：

```text
playback_commands ................ Passed    0.11 sec
playback_events .................. Passed    0.12 sec
100% tests passed, 0 tests failed out of 16
Total Test time (real) = 3.31 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

## R3-02 acceptance

用户确认 R3-02 验收成功；对应回合未提供该次 CTest 的逐项输出或总耗时，因此只记录验收结论。

## R3-03 Windows verification

R3-03 首次 build 在 CTest 前被 reducer test helper 命名冲突阻断；修复 `event(...)` → `makePlaybackEvent(...)` 后，用户重新执行标准 build/test，实际结果：

```text
playback_reducer ................. Passed    0.11 sec
100% tests passed, 0 tests failed out of 18
Total Test time (real) = 3.40 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

## R3-04 Windows verification

用户在 `agent/r3-stage` 执行标准 Windows build/test，实际结果：

```text
playback_invariants .............. Passed    0.11 sec
100% tests passed, 0 tests failed out of 19
Total Test time (real) = 3.44 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

R2-01 的 `player.log` 落盘缺口继续作为非阻塞诊断事项保留。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。**

## R3-05 implementation status — current

> 本节是当前 Stage R3 状态的权威追加记录，取代上文在 R3-04 收口时写下的“PlaybackSession 仍待引入”和阶段状态摘要；上文原文保留作为已完成阶段的历史事实记录。

R3-05 已实现 `PlaybackSession thread` 主链，当前状态为 **Implemented; Windows verification pending**。本任务没有接入 UI，也没有提前实现 R3-06 RequestTracker、R3-07 StatePublisher、R3-08 完整 shutdown hardening 或 R3-09 stale-generation gate。

实际新增与职责边界：

- `src/playback/application/session/PlaybackSession`：PlaybackSnapshot 单一 owner；只在所属 Playback Thread 初始化/关闭 backend、接受命令、消费 PlaybackEvent、调用 Reducer 与 R3-04 invariants 并 commit 新 Snapshot。
- `PlaybackSessionThread`：只拥有 QThread 与 Session worker 生命周期，以及基础 5 秒 stop wait；完整关闭竞态、pending cancellation、late callback/reply 仍归 R3-08。
- `PlaybackCommandBus`：只做 Domain command validation 与 Qt queued ingress，不直接调用 libmpv、不保存播放真值。
- `session/backend/PlaybackSessionBackend`：只组合既有 R2 `MpvHandle`、`MpvPropertyObserver`、`MpvEventLoop`、`MpvCommandExecutor` 并把 MpvEvent 映射成 PlaybackEvent；不保存第二份 Snapshot。
- `MpvPlaybackCommandMapper`：只把 Domain `PlaybackCommandPayload` 映射为 R2 typed mpv request，覆盖 load/play/pause/stop/absolute-relative seek/volume/mute/speed；Lifecycle command 仍由 Session 拥有。
- 每次新的 Load 由 Session 分配单调递增 MediaGeneration，先建立 Opening/source identity 再提交 backend；事件/reply 尚未绑定 generation，因此 A→B stale event/reply 过滤仍未实现。
- Session 对基础 property event 做 lifecycle acceptance：Empty/Failed/Closing 等不接受媒体/timeline property，pause/seeking 只在 Ready 接受，buffering 只在 Opening/Ready 接受；这是防止初始化 property observation 制造非法 Snapshot，不是 generation gate。
- `snapshotCommitted` 目前只作为 Playback Thread 内部原始 commit 信号供测试与后续 R3-07 接入，不直接暴露给 QML/GUI，也不做 position throttle。

新增真实 CTest `playback_session`：测试运行时生成 3 秒静音 PCM WAV，不使用 mock 替代 libmpv 主链；覆盖 `PlaybackCommandBus → PlaybackSession(QThread) → R2 libmpv infrastructure → PlaybackEvent → Reducer → Snapshot` 的 load/pause/play/pause/absolute-seek/stop，以及 Snapshot callback thread、0 invariant violation、`PlaybackSessionThread` start/stop smoke。

当前连接环境未执行 Windows Qt/MSVC 构建，因此 R3-05 不声明 Complete。标准门禁预计从 19 项增加到 **20/20**：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_session
```

当前准确状态：**Stage R3：In Progress；R3-01~R3-04：Complete；R3-05：Implemented，Windows verification pending。** R2-01 的 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留。

## R3-05 Windows verification and acceptance — current

> 本节取代上一个 `R3-05 implementation status — current` 中的待验证状态；实现说明继续保留为历史事实。

用户在 `agent/r3-stage` 的 Windows 工作区提供了标准门禁输出：开发工具与 R2 产品依赖可用，开发 runtime root marker 校验成功，Ninja 报告 `no work to do`，随后完整 CTest 通过。

实际结果：

```text
playback_session ................. Passed    0.24 sec
100% tests passed, 0 tests failed out of 20
Total Test time (real) = 3.39 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

R3-05 的真实 Session 主链因此正式验收：`PlaybackCommandBus → PlaybackSession(QThread) → R2 libmpv infrastructure → PlaybackEvent → Reducer → PlaybackSnapshot` 的集成测试通过，并与其余 19 项既有回归共同保持全绿。本次验收不扩大 R3-05 范围；RequestTracker、StatePublisher、完整 shutdown hardening 与 stale-generation gate 仍分别属于后续 R3-06、R3-07、R3-08、R3-09。

R2-01 的仓库根 `player.log` 落盘缺口仍是明确的非阻塞诊断事项，未被本次验收视为解决。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Complete。下一 Atomic Task：R3-06 Request tracker。**

### 2026-08-09 — R3-05 acceptance addendum

- Accepted R3-05 from the user's Windows validation output: development runtime marker passed, `playback_session` passed in 0.24 seconds, and all 20 CTests passed with 0 failures in 3.39 seconds total.
- Kept R3-06 RequestTracker, R3-07 StatePublisher, R3-08 shutdown hardening and R3-09 stale-generation filtering out of the R3-05 acceptance scope.

## R3-06 implementation status — current

R3-06 已实现 generation-aware `RequestTracker` 并接入 `PlaybackSession`，当前状态为 **Implemented; Windows verification pending**。

- 新增 `src/playback/application/requests/`：`playback_request.h` 只定义 request type/state/cancellation reason/record/resolution/diagnostics；`request_tracker.*` 是 RequestId、pending/completed/cancelled、submittedAt、generation 与 reply resolve 的唯一 owner；`request_timeout_monitor.*` 只负责 Playback Thread 上的超时调度，不把 QTimer 责任塞回 Session。
- Load/play/pause/stop/seek 始终作为 media-scoped request 记录提交时的 `MediaGeneration`，包括 generation=0 的“当前无媒体”上下文；因此无媒体阶段的 pending 也会在下一次 Load 建立新 generation 时失效。volume/mute/speed 作为 session-scoped control request 不因媒体替换取消，保持 R3-03 已冻结的 controls 跨媒体语义。
- 新 Load B 先登记 B，再取消非 B generation 的 media-scoped pending，因此 A 的 late reply 只得到 Cancelled/StaleGeneration disposition，不进入 Reducer；B reply 仍可正常完成。
- duplicate RequestId、unknown reply、duplicate reply、cancelled reply、stale-generation reply 均不产生新的播放状态副作用；有效失败 reply 才进入错误路径，其中 Load failure 映射到 `MediaFailedEvent`，其他命令 failure 记录为 command failure。
- backend 同步提交失败会把已登记 request 标记为 `SubmissionFailed` cancellation，避免遗留 Pending；Session shutdown 会把剩余 pending 标记为 Shutdown cancellation。
- RequestTracker 提供 30 秒 timeout cancellation；`RequestTimeoutMonitor` 在 Playback Thread 以 1 秒 coarse timer 调用 expired scan。timeout/unknown/duplicate/cancelled/stale reply 诊断计数保留在 tracker 内，不引入新的日志依赖。
- R3-06 只解决 async reply identity/cancellation，不给 start-file/file-loaded/property/end-file 等媒体事件打 generation；完整 stale-event gate 仍归 R3-09。连续 seek/track selection 等同类请求 supersession 仍归 R3-12。
- 新增独立 CTest `playback_request_tracker`，覆盖 request type/generation/submittedAt、duplicate/lifecycle rejection、无媒体 generation、A→B cancellation、session-scoped control 保留、reply exactly-once、stale/cancelled/unknown reply、timeout 与 shutdown cancel-all；现有真实 `playback_session` CTest 继续作为接入后的 libmpv 主链回归。

当前连接环境没有执行 Windows Qt/MSVC 构建。标准门禁预计从 20 项增加到 **21/21**：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_request_tracker
```

当前准确状态：**Stage R3：In Progress；R3-01~R3-05：Complete；R3-06：Implemented，Windows verification pending。** R2-01 的 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留。

### 2026-08-09 — R3-06 implementation addendum

- Implemented generation-aware RequestTracker ownership for async command replies, cancellation, timeout, unknown/duplicate handling and shutdown cleanup without moving playback truth out of PlaybackSession.
- Extracted request timeout scheduling into `RequestTimeoutMonitor`, kept media requests generation-bound even before a media is active, and added the independent `playback_request_tracker` CTest.
- Kept full stale media-event filtering and same-kind request supersession outside R3-06.

## R3-06 Windows verification and acceptance — current

> 本节取代上一个 `R3-06 implementation status — current` 中的待验证状态；实现说明继续保留为历史事实。

用户在 `agent/r3-stage` 的 Windows 工作区完成标准 build/test。开发 runtime root marker 校验成功，Ninja 报告 `no work to do`，随后完整 CTest 全部通过。

实际结果：

```text
playback_request_tracker ......... Passed    0.16 sec
playback_session ................. Passed    0.55 sec
100% tests passed, 0 tests failed out of 21
Total Test time (real) = 10.67 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

R3-06 的异步请求关联链因此正式验收：RequestId/generation 绑定、A→B 旧请求取消、unknown/duplicate/cancelled/stale reply no-op、timeout 与 shutdown cancellation 的独立 tracker 测试通过；接入后的真实 `playback_session` libmpv 主链也保持通过。完整 stale media-event gate 仍归 R3-09，同类请求 supersession 仍归 R3-12，本次验收不扩大任务范围。

R2-01 的仓库根 `player.log` 落盘缺口仍是明确的非阻塞诊断事项，未被本次验收视为解决。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Complete。R3-06：Complete。下一 Atomic Task：R3-07 State publisher。**

### 2026-08-09 — R3-06 acceptance addendum

- Accepted R3-06 from the user's Windows validation output: development runtime marker passed, `playback_request_tracker` passed in 0.16 seconds, `playback_session` passed in 0.55 seconds, and all 21 CTests passed with 0 failures in 10.67 seconds total.
- Kept R3-07 StatePublisher, R3-08 shutdown hardening, R3-09 stale media-event filtering and R3-12 supersession outside the R3-06 acceptance scope.

## R3-07 implementation status — current

R3-07 已实现 `StatePublisher` GUI/consumer-thread 发布边界，当前状态为 **Implemented; Windows verification pending**。

- 新增 `src/playback/application/state_publisher/`，`StatePublisher` 只拥有发布缓存与节流时序，不拥有或修改播放真值；权威 `PlaybackSnapshot` 仍只由 `PlaybackSession` 持有。
- `PlaybackSnapshot` 增加 Qt metatype 声明，`StatePublisher` 注册该类型；`PlaybackSessionThread` 以 `Qt::QueuedConnection` 将 Playback Thread 的 `snapshotCommitted` 投递给创建 `PlaybackSessionThread` 的 consumer/GUI thread 上的 Publisher，未来 ViewModel 不需要跨线程访问 Session 内部状态。
- 首个 Snapshot 立即发布；完全重复 Snapshot 不重复发布。只有“除 `timeline.positionSeconds` 外所有字段均相同”的 position-only 更新进入节流，目标频率为 **20 Hz / 50 ms**，窗口内只保留最新 pending Snapshot。
- lifecycle、transport、media identity、duration、seekable/seeking、buffering、controls、failure 或 generation 任一变化都绕过 position throttle 立即发布；关键变化到来时会取消尚未 flush 的旧 position-only pending，因此 Pause/Error 等状态不会被位置节流延迟。
- Publisher 使用所属 consumer thread 的 single-shot precise timer；不新增线程、不调用 libmpv、不执行 Reducer、不修改 Snapshot，也不实现 R3-09 generation stale-event filtering。
- `PlaybackSession::snapshotCommitted` 继续保留为 Playback Thread 内部原始 commit 信号，现有 Session 测试仍可验证播放真值提交线程；GUI 侧应消费 `StatePublisher::snapshotPublished`。
- 新增独立 CTest `playback_state_publisher`，覆盖首帧即时、重复抑制、position burst 合并到最新值、Pause/Failure 绕过节流、queued producer→consumer thread handoff；现有 `playback_session` CTest 追加真实短媒体 `PlaybackSessionThread → StatePublisher` consumer-thread 接线验证。

当前连接环境没有执行 Windows Qt/MSVC 构建，因此 R3-07 尚不能标记 Complete。标准门禁预计从 21 项增加到 **22/22**：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_state_publisher
```

当前准确状态：**Stage R3：In Progress；R3-01~R3-06：Complete；R3-07：Implemented，Windows verification pending。** R2-01 的 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留。R3-08 shutdown hardening、R3-09 stale media-event gate 与 R3-12 supersession 均未提前实现。

### 2026-08-09 — R3-07 implementation addendum

- Implemented GUI/consumer-thread StatePublisher handoff with queued `PlaybackSnapshot` delivery and 20 Hz position-only coalescing while keeping all non-position state changes immediate.
- Added `playback_state_publisher` CTest plus a real PlaybackSessionThread-to-StatePublisher short-media handoff check；Windows build/test remains pending before acceptance.
- Kept shutdown hardening, stale media-event filtering, supersession and full ViewModel/QML integration outside R3-07.

## R3-07 Windows verification and acceptance — current

> 本节取代上一个 `R3-07 implementation status — current` 中的待验证状态；实现说明继续保留为历史事实。

用户在 `agent/r3-stage` 的 Windows 工作区完成标准 build/test。依赖检查报告 libmpv import SHA-256 为 `6c5e98ad4f5b53dbb847c522f3aaa2fc4dd8d1df1b4153af85fd2db4fa65296b`，开发 runtime root marker 校验成功，Ninja 报告 `no work to do`，随后完整 CTest 全部通过。

实际结果：

```text
playback_state_publisher ......... Passed    0.66 sec
playback_session ................. Passed    0.37 sec
100% tests passed, 0 tests failed out of 22
Total Test time (real) = 5.03 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

R3-07 的 GUI/consumer-thread 状态发布边界因此正式验收：独立 `playback_state_publisher` 测试通过，真实 `PlaybackSessionThread → StatePublisher` 短媒体接线继续由 `playback_session` 回归覆盖；其余 20 项既有测试同时保持全绿。本次验收不扩大范围，R3-08 Session shutdown、R3-09 stale media-event gate 与 R3-12 supersession 仍按后续 Atomic Task 单独实施。

R2-01 的仓库根 `player.log` 落盘缺口仍是明确的非阻塞诊断事项，未被本次验收视为解决。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Complete。R3-06：Complete。R3-07：Complete。下一 Atomic Task：R3-08 Session shutdown。**

### 2026-08-09 — R3-07 acceptance addendum

- Accepted R3-07 from the user's Windows validation output: development runtime marker passed, `playback_state_publisher` passed in 0.66 seconds, `playback_session` passed in 0.37 seconds, and all 22 CTests passed with 0 failures in 5.03 seconds total.
- Kept R3-08 shutdown hardening, R3-09 stale media-event filtering, R3-12 supersession and full ViewModel/QML integration outside the R3-07 acceptance scope.

## R3-08 implementation status — current

R3-08 已实现 PlaybackSession 有界关闭协议，当前状态为 **Implemented; Windows verification pending**。

- `PlaybackCommandBus` 新增线程安全 close gate；`PlaybackSessionThread::stop()` 开始时先关闭入口，之后 submit 明确失败，不再把新命令排入正在关闭的 Playback Thread。
- 新增 `application/session/playback_shutdown.*`，集中执行 Session 级关闭顺序：停止 request timeout 调度 → `RequestTracker` 将全部 pending 标记为 Shutdown cancellation → backend teardown。
- `PlaybackSessionBackend::shutdown()` 先清空上层 event handler，再停止 `MpvEventLoop`；其 stop 会 deactivate/unregister libmpv wakeup callback 并收敛在途 callback，随后才 unobserve properties、释放 executor/observer/event loop、关闭 mpv handle，因此普通 libmpv client API 与 teardown 都仍发生在 Playback Thread。
- `PlaybackSession` 在 `stopping_` 后拒绝命令并忽略 backend event/reply，资源关闭完成后提交最终 Closing + Stopped Snapshot；late backend callback/reply 不再进入 Reducer。
- `PlaybackSessionThread::stop()` 保持 5 秒有界等待并验证 worker 已释放；析构不再退化为无期限 `QThread::wait()`，若安全有界关闭无法完成则 fail-fast，不使用不安全的 `QThread::terminate()`。
- 现有 `PlaybackSessionTestHarness` 同步移除无限 wait 兜底，避免测试基础设施掩盖 shutdown hang。
- backend/session 析构不再提供跨线程 libmpv teardown 兜底；正常关闭必须先在 Playback Thread 完成资源释放，若带着 live backend 资源越界析构则 fail-fast。
- 新增独立 CTest `playback_shutdown`：覆盖 command bus close rejection、loading 中 stop、playing 中 stop + final Closing 后无 late Snapshot、同一 `PlaybackSessionThread` 100 次 start/stop 生命周期；CTest 总超时 120 秒。
- 本任务不实现 R3-09 MediaGeneration stale event gate，也不实现 R3-12 request supersession。

当前连接环境未执行 Windows Qt/MSVC build/test。标准门禁预计从 22 项增加到 **23/23**：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_shutdown
```

当前准确状态：**Stage R3：In Progress；R3-01~R3-07：Complete；R3-08：Implemented，Windows verification pending。** R2-01 的 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留；后续仍需执行 R3-09~R3-12 补强任务。

### 2026-08-09 — R3-08 implementation addendum

- Implemented bounded PlaybackSession shutdown ordering with a closed command ingress, pending-request cancellation, wakeup/event-loop teardown before handle destruction, and no unbounded QThread wait.
- Added independent `playback_shutdown` coverage for loading/playback shutdown and 100 Session lifecycle cycles；Windows build/test remains pending before acceptance.
- Kept R3-09 stale media-event filtering and R3-12 supersession outside R3-08.

## R3-08 Windows verification and acceptance — current

> 本节取代上一个 `R3-08 implementation status — current` 中的待验证状态；实现说明继续保留为历史事实。

R3-08 第一次 Windows 门禁实际结果为 **22/23**：开发 runtime root marker 通过，`playback_shutdown` 在 `stopDuringPlaybackSuppressesLateSnapshots()` 中因 `playingSnapshotSeen == false` 失败；同一测试中的 command-bus close、loading 中关闭与 100 次 Session 生命周期循环均已通过。该次完整 CTest 总耗时 **29.11 秒**，因此没有被标记为验收通过。

失败根因确认是测试前置条件而非 shutdown 生产链：测试在 Ready 后直接重复发送 Play，并错误假设 libmpv 必然再次产生 `pause=false` property change；当前 transport Snapshot 只由实际 PauseChangedEvent 更新。修复仅调整 `playback_shutdown_test.cpp`：改为 `Ready → Pause → 确认 Paused → Play → 确认 Playing → shutdown`，并把测试媒体从 5 秒延长到 30 秒以排除自然 EOF 竞争；生产源码、公共接口和运行时行为均未修改。

用户随后在修正版 `agent/r3-stage` 上重新执行标准 Windows build/test，实际结果：

```text
playback_shutdown ................ Passed    6.99 sec
100% tests passed, 0 tests failed out of 23
Total Test time (real) = 9.84 sec
```

测试前开发 runtime marker 校验成功：

```text
[OK] Development runtime root marker -> build/windows-msvc-debug/.player-development-root
```

依赖检查继续确认 libmpv 0.41.0 / FFmpeg 8.0.3 / libplacebo 7.351.0 / libass 0.17.4 包可用，libmpv runtime SHA-256 为 `e4edeadd3daf7ca36c2da31a06534a273c61ad4a0f05bb2e9c3c851dfd482acc`，import SHA-256 为 `6c5e98ad4f5b53dbb847c522f3aaa2fc4dd8d1df1b4153af85fd2db4fa65296b`。

R3-08 因此正式验收：关闭期间命令入口关闭、pending request cancellation、loading/playing shutdown、final Closing 后 late Snapshot 抑制以及 100 次基础 Session 生命周期均由当前测试链覆盖并通过；完整 23 项回归保持全绿。本次验收不扩大范围，R3-09 MediaGeneration stale-event gate、R3-10~R3-12 补强仍按后续 Atomic Task 单独实施。

R2-01 的仓库根 `player.log` 落盘缺口仍是明确的非阻塞诊断事项，未被本次验收视为解决。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Complete。R3-06：Complete。R3-07：Complete。R3-08：Complete。下一 Atomic Task：R3-09 MediaGeneration 与 Stale Event Gate。**

### 2026-08-09 — R3-08 acceptance addendum

- Recorded the first R3-08 Windows gate accurately as 22/23 with `playback_shutdown` failing only its direct-Play precondition; loading shutdown and the 100-cycle Session lifecycle path already passed.
- Stabilized that test without product-code changes by requiring a confirmed Pause before Play and extending the generated media duration, then accepted R3-08 from the user's rerun: `playback_shutdown` passed in 6.99 seconds and all 23 CTests passed with 0 failures in 9.84 seconds total.
- Kept R3-09 stale media-event filtering and the remaining R3-10~R3-12 supplement tasks outside the R3-08 acceptance scope.

## R3-09 implementation status — current

R3-09 已实现 MediaGeneration stale-event gate，当前状态为 **Implemented; Windows verification pending**。

- `PlaybackEvent` 新增后端无关的 `MediaGeneration` 标签；0/invalid 保留为“未归属”，mpv playlist-entry id 不进入 Domain 或 Snapshot。
- 新增 `application/session/backend/MpvMediaGenerationAttributor`，只负责把 Load RequestId/generation 与 libmpv `start-file/end-file` 的 playlist-entry identity、FileLoaded 顺序和 property replacement fence 对应起来；它不决定事件是否可进入 Reducer。
- replacement fence：Session 接受 Load B 时立即切到 B generation；在 B 自己的 FileLoaded 之前，媒体 property 仍归属此前已建立 generation，因此 A 的晚到 position/title/path/track/chapter property 会被当前 B gate 拒绝，而不是因为 Start B 已出现就误标成 B。
- 新增 Session 内部 `MediaGenerationGate`；媒体级事件必须带有效且等于当前 generation 的标签才允许继续进入 lifecycle acceptance / Reducer。missing/stale generation 只累计 diagnostics，不改 Snapshot。
- `start-file/file-loaded/end-file/error/position/duration/seekable/seeking/pause/buffering/media identity/core-idle/eof` 走 generation gate；Volume/Mute/Speed 保持 session-scoped；CommandReply 仍由 R3-06 RequestTracker 负责 RequestId + generation 的 exactly-once 关联。
- Redirect 产生的插入 playlist entry 继承同一 generation；backend shutdown 会 reset attribution state。
- 当前 `MpvPlaybackEventMapper` 仍不把 track-list/chapter-list Node 转成 Domain state，因此 R3-09 只建立这些 raw property 的 generation attribution边界；真正的 typed tracks/chapters 与 Snapshot 多轴扩展仍归 R3-10。本任务没有提前实现 R3-10、R3-11 或 R3-12 supersession。
- 新增独立 CTest `playback_media_generation`，覆盖 synthetic A→B late position/file-loaded/end/property、missing generation fail-closed、session-scoped control、failed load cancellation 与 redirect generation inheritance；现有真实 `playback_session` 追加 250ms A → 5s B 的 immediate replacement 回归，并在 A 足以结束后仍要求 B generation/source/lifecycle 不被污染。
- 真实 libmpv 0.41.0 的 immediate A→B event ordering 由新增 Session 回归作为本任务硬门禁；当前连接环境未执行 Windows Qt/MSVC，因此尚未证明该实际时序通过。

当前连接环境未执行 Windows Qt/MSVC build/test。新增一个 CTest 后，标准门禁**预计**从 23 项增加到 **24/24**；这是待验证目标，不是已通过结果：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期新增：

```text
playback_media_generation
```

当前准确状态：**Stage R3：In Progress；R3-01~R3-08：Complete；R3-09：Implemented，Windows verification pending。** R2-01 的 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留；R3-10~R3-12 尚未实施。

### 2026-08-09 — R3-09 implementation addendum

- Implemented backend media-generation attribution plus a Session stale-event gate before Reducer, including a replacement property fence that does not relabel queued A properties as B.
- Added independent synthetic generation-gate coverage and a real immediate A→B libmpv Session regression；Windows build/test remains pending.
- Kept track/chapter Snapshot expansion, reducer cleanup matrix and generalized supersession in R3-10~R3-12.

## R3-09 first Windows gate and property-refresh fix — current

> 本节取代上一节“Windows verification pending”的初始状态；R3-09 仍未验收完成，直到修正版标准 Windows 门禁全绿。

用户在 `fb74019990d6dbdbb7c442949a415d285c104aaa` 上完成第一次标准 Windows build/test。工具链、依赖、libmpv manifest/hash 与开发 runtime root marker 均通过，CTest 实际结果为 **23/24**：

```text
playback_session .................***Failed    7.41 sec
playback_shutdown ................ Passed    6.96 sec
playback_media_generation ........ Passed    0.17 sec
96% tests passed, 1 tests failed out of 24
Total Test time (real) = 18.94 sec
```

失败集中在 `PlaybackSessionTest::rapidReplacementKeepsLatestGeneration()`：A=250ms、B=5s 的 immediate replacement 中，7 秒内没有得到同时满足 generation=2、Ready、source=B、`path` 指向 `rapid-b.wav` 且 `duration≈5s` 的 Snapshot。该失败不被视为测试误报，也没有通过放宽断言处理。

根因确认是 R3-09 replacement fence 过度保守但缺少 current-media resync：B `FileLoaded` 前，无法可靠区分 A/B 的 property change，继续 fail-closed 是必要的；但 B 自己的 `path/duration` 等 property 若只在 fence 打开前通知一次，会被按旧 generation 丢弃，而 B `FileLoaded` 后原实现没有重新读取当前媒体值，因此 B 可以进入 Ready 却永久缺少静态状态。

修复已提交到 `agent/r3-stage`：

- 新增 `MpvPropertyDecoder`，把 FLAG/DOUBLE/STRING/NODE typed decode 从 Observer 的第二职责中提取；`MpvPropertyObserver::decode()` 保留为兼容转发，不改变既有调用接口。
- 新增 `MpvPropertyReader`，只通过 registry 读取当前 property；STRING 使用 `mpv_free`，NODE 在完成 Qt-owned 深拷贝后使用 `mpv_free_node_contents`，`MPV_ERROR_PROPERTY_UNAVAILABLE` 继续收敛为 typed unavailable，而不是伪造硬失败。
- `MpvMediaGenerationAttributor` 只在与当前 active playlist entry 对应的 `FileLoaded` 上发出一次 property-refresh generation；无法再与 loading entry 对应的 FileLoaded 不猜 active generation，保持 fail-closed。
- `PlaybackSessionBackend` 先正常投递当前 generation 的 FileLoaded，让 Session 进入 Ready，再在同一 Playback Thread 回调中同步刷新 Snapshot 当前拥有的 9 个媒体属性：position、duration、pause、seekable、seeking、paused-for-cache、cache-buffering-state、media-title、path。Volume/Mute/Speed 保持 session-scoped；track/chapter 等仍留给 R3-10。
- 原 `rapidReplacementKeepsLatestGeneration()` 的 `path + duration≈5s` 硬断言保持不变；新增独立 CTest `mpv_property_reader` 验证未初始化拒绝以及真实 pause/volume 当前值读取。修复后标准 CTest 数预计由 24 增至 **25**。

当前代码头包含上述修复，但本连接环境不能执行用户 Windows Qt/MSVC 门禁，因此此处只记录 **fix implemented; Windows rerun pending**，不写成通过。请重新执行：

```powershell
git pull --ff-only origin agent/r3-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r3.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

下一次验收至少要求 `mpv_property_reader`、`playback_session`、`playback_media_generation`、`playback_shutdown` 全部通过，且完整标准套件为 **25/25，0 failed**。若仍失败，以实际输出继续定位，不降低测试强度。

**当前准确状态：Stage R3：In Progress；R3-01~R3-08：Complete；R3-09：Implemented / first Windows gate failed / fix submitted / Windows rerun pending。R3-10~R3-12 尚未实施。** R2-01 的仓库根 `player.log` 缺口仍为已知非阻塞诊断事项。

## R3-09 Windows verification and acceptance — current

> 本节取代上一节“Windows rerun pending”的状态；R3-09 现已完成真实 Windows 验收。

用户在修正版 `12bea42ddfd6aca4d54b46d6bef1f12ffd0c5f85` 上重新执行标准 Windows build/test。工具链、依赖与 libmpv 包校验通过，开发 runtime root marker 正常，Ninja 报告 `no work to do`。

实际结果：

```text
mpv_property_reader .............. Passed    0.14 sec
playback_session ................. Passed    1.10 sec
playback_shutdown ................ Passed    6.93 sec
playback_media_generation ........ Passed    0.16 sec
100% tests passed, 0 tests failed out of 25
Total Test time (real) = 12.36 sec
```

依赖校验继续确认 libmpv runtime SHA-256 为 `e4edeadd3daf7ca36c2da31a06534a273c61ad4a0f05bb2e9c3c851dfd482acc`，import SHA-256 为 `6c5e98ad4f5b53dbb847c522f3aaa2fc4dd8d1df1b4153af85fd2db4fa65296b`；开发 runtime marker 为 `build/windows-msvc-debug/.player-development-root`。

原始失败路径 `PlaybackSessionTest::rapidReplacementKeepsLatestGeneration()` 已在未降低断言强度的情况下通过；`MpvPropertyReader` 的独立读取/线程所有权覆盖、真实 Session A→B replacement、shutdown 回归与 generation gate 同时保持全绿。R3-09 因此正式验收完成。

本次验收不提前实现 R3-10 PlaybackSnapshot 多轴扩展、R3-11 reducer cleanup matrix 或 R3-12 supersession；这些任务继续独立推进。R2-01 的仓库根 `player.log` 落盘缺口仍作为已知非阻塞诊断事项保留。

**Stage R2：Complete。Stage R3：In Progress。R3-01：Complete。R3-02：Complete。R3-03：Complete。R3-04：Complete。R3-05：Complete。R3-06：Complete。R3-07：Complete。R3-08：Complete。R3-09：Complete。下一 Atomic Task：R3-10 PlaybackSnapshot 多轴状态。**

### 2026-08-09 — R3-09 acceptance addendum

- Accepted R3-09 from the user's Windows rerun: `mpv_property_reader` 0.14 seconds, `playback_session` 1.10 seconds, `playback_shutdown` 6.93 seconds, `playback_media_generation` 0.16 seconds, and all 25 CTests passed with 0 failures in 12.36 seconds total.
- Confirmed the original immediate A→B replacement hard regression now passes without weakening the `path + duration≈5s` acceptance condition.
- Kept R3-10~R3-12 outside the R3-09 acceptance scope; the existing R2-01 `player.log` diagnostic gap remains open.