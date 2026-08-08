# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器工程。当前仍处于 R2：无 UI libmpv 播放核心阶段；R2-10 已完成 Windows 验收，R2-11 已实现。R2-11 首次 Windows 构建因 `playback_probe` target 未声明模块 include root 而在编译阶段失败，该构建缺口已修复，当前等待重新执行本机 build/CTest 行为矩阵验收。

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
- R2-11 `tools/playback_probe/`：新增脚本化 8 场景 headless 行为矩阵、运行时生成媒体、typed trace 与自动 CTest 门禁；
- Windows 构建后显式 `windeployqt` 与 `.player-development-root` 开发标记；
- R2-08 已由用户 Windows 环境完成最终验收：11/11 CTest 全绿（1.47 秒），真实媒体主链 PASS，非法媒体错误路径返回明确加载失败诊断；
- R2-09 已由用户 Windows 环境完成最终验收：12/12 CTest 全绿（2.31 秒），新增 `mpv_event_semantics` 通过；
- R2-10 已由用户 Windows 环境完成最终验收：13/13 CTest 全绿（2.40 秒），`mpv_properties` 与新增 `mpv_property_baseline` 均通过。

R2-01~R2-10 已完成或按既有记录验收；R2-11 源码与自动行为矩阵已实现。首次 Windows build 在进入 CTest 前因 probe 子模块 include root 缺失而失败，现已补上 `target_include_directories(playback_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})`，仍需用户本机重新 build 并达到 **14/14 CTest** 才能验收。PlaybackSession 仍从 R3 开始；Render API、数据库、播放列表、完整播放器 UI 与安装包继续在后续阶段实现，不提前堆入 R2。

## Product scope

首版目标是一个以 libmpv 为播放内核、Qt Quick/QML 为界面的 Windows 10/11 x64 桌面播放器，同时保持 macOS/Linux 清晰适配边界。

MVP 包括：本地文件、网络 URL、播放/暂停/停止、绝对/相对 Seek、Timeline、音量/静音、倍速、全屏、播放列表、音轨、字幕、外挂字幕、音画延迟、章节、媒体信息、错误反馈、加载/缓冲/暂停/结束状态、最近播放、恢复进度、快捷键、系统媒体键、防休眠、单实例、文件关联和 Windows 安装包。

第二阶段再做截图、A-B 循环、画面比例/裁剪/旋转、字幕样式、播放质量预设、Shader、迷你播放器、画中画、高级统计和 macOS/Linux 适配。在线站点解析、媒体服务器、DLNA/AirPlay/Chromecast、账号、云同步、在线字幕搜索、插件市场、视频剪辑/转码和 AI 字幕/画质增强不属于首版。

## Architecture boundary

```text
QML presentation
      ↓ intent / projected state
Application layer (PlaybackSession arrives in R3)
      ↓ commands / typed events
libmpv infrastructure
      ↓ public C API
libmpv
```

当前关键职责：

```text
src/app/
  main/bootstrap/composition
  启动顺序、RuntimePaths、日志、图形探针、QML 与顶层生命周期。

src/foundation/logging/
  日志分类、文件 sink、轮转、脱敏、flush。

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
  raw mpv_event、mpv_event_*、mpv_node 指针不离开 infrastructure/mpv。

src/playback/infrastructure/mpv/properties/
  MpvPropertyRegistry 集中拥有 R2 baseline property 名、稳定 observation id 与期望格式；
  MpvPropertyObserver 负责 observe/unobserve、Qt owner-thread 约束和 FLAG/DOUBLE/STRING/NODE payload decode；
  MPV_FORMAT_STRING 在 observer 内立即复制为 Qt-owned QString；
  MpvNodeDecoder 把 track/chapter/cache/track-selection/video/audio 等 MPV_FORMAT_NODE 深拷贝为 QVariant/QVariantList/QVariantMap/QByteArray。

tools/playback_probe/
  main.cpp 只处理 CLI 模式选择和进程退出；
  PlaybackProbeRunner 保留 R2-08 单媒体基本链，不与 R2-11 行为矩阵混写；
  PlaybackProbeMatrixRunner 只串行调度 8 个脚本化场景；
  scenarios/ 定义场景数据与通用脚本执行器；
  fixtures/ 只生成 probe-local 短 PCM WAV A/B/EOF 与缺失媒体路径；
  trace/ 只记录 typed command/event/property 顺序并输出有界 trace；
  runtime/PlaybackProbeRuntime 仍是 MpvHandle/Observer/EventLoop/Executor 唯一组合与销毁入口；
  全部探针采用 config=no + vo=null + ao=null，不建立 PlaybackSnapshot、不复制未来 R3 业务状态机。

src/presentation/qml/
  只负责 presentation；禁止直接 mpv_command/mpv_set_property/C 指针访问。
```

R2-04 的关闭顺序仍是硬约束：先令 wakeup bridge inactive，再注销 libmpv wakeup callback，等待已经进入 callback 的短路径退出，最后才允许事件循环目标对象销毁；停止后的 queued drain 直接 no-op。

R2-05 保留 `reply_userdata` 作为后续 R3 command tracker 的关联键，但 executor 不建立 pending map、不保存播放状态真值。

R2-06/R2-07/R2-10 的 property 数据链现在是：registry 定义观察身份与格式，observer 管理观察生命周期和标量复制，event decoder 在消费 raw property event 时调用 property decode；FLAG/DOUBLE/STRING/None 直接转成 Qt-owned/内部值，Node 通过独立 `MpvNodeDecoder` 深拷贝。因此事件循环发出 `eventDecoded(const MpvEvent&)` 后，上层无需解释或持有 libmpv C payload，也不需要散写 mpv property 字符串。

R2-08 只消费上述既有接口。probe runtime 自己拥有 headless option profile 与 mpv 对象生命周期，runner 只根据 typed event/reply 推进 `load -> pause -> play -> relative seek -> stop -> end-file`，每一步都有 15 秒超时；成功/失败通过 queued finalization 等当前 event drain 返回后再关闭 runtime，避免在 `MpvEventLoop::drainPendingEvents()` 栈内销毁 EventLoop；失败路径返回非零退出码并打印 libmpv typed diagnostic。

R2-09 仍只增强 infrastructure 事件契约，不建立媒体真值或业务决策。`MpvEndFileData` 同时保留稳定的 `MpvEndFileReason` 与 libmpv 原始 `rawReason`；已知 reason 继续映射为 EOF/Stop/Quit/Error/Redirect，未知 future reason 保留原始整数值供诊断和后续 R3 生命周期边界判断。decoder 仍不做自动下一项，也不持有当前媒体、generation 或 PlaybackSnapshot。

R2-10 仍只建立 Property Baseline，不建立 R3 Snapshot。当前选中的 `aid/sid/vid` 使用 `MPV_FORMAT_NODE`，以保留数字 track id 与 `no` 等原生值形态；`cache-buffering-state` 同样使用 Node，避免依赖显示字符串或不必要的格式假设。`media-title` 与 `path` 使用 `MPV_FORMAT_STRING` 并立即复制到 QString。`demuxer-cache-state`、`video-params`、`audio-params` 使用官方支持的 Node 结构并在 infrastructure 内深拷贝完成生命周期。

R2-11 不新增产品状态。`playback_probe --matrix` 运行时生成三段无第三方版权依赖的短 silent PCM WAV，并通过 `PlaybackProbeScenarioCatalog` 定义 8 个真实 libmpv 场景。`PlaybackProbeScenarioRunner` 只解释 submit/reply/event/property/barrier/shutdown 步骤；所有事件继续来自现有 typed infrastructure。每个场景结束或失败都通过 queued finalization 关闭 runtime，确保不会在 `MpvEventLoop::drainPendingEvents()` 调用栈内销毁事件循环。Trace 最多保留 512 条记录，避免异常事件风暴导致无界增长。R2-11 probe 子目录的头文件统一以 `tools/playback_probe/` 为模块 include root，由 target 私有 include path 提供；不向其他产品 target 暴露该工具内部路径。

## Module growth rule

模块按职责拆分，不按行数拆分。一个已有文件出现第二项可独立描述、独立测试、独立演进的职责时，升级为职责目录；不允许把页面/播放核心全部堆入一个文件，也不允许 `old/new/v2/final/copy` 源码历史副本。Git 负责历史。

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

仍存在一个明确记录的非阻塞缺口：开发模式预期的仓库根目录 `player.log` 没有生成。用户已明确要求不让该问题继续阻塞 R2 普通框架任务。它必须在后续相关诊断/发布门禁前重新关闭，但不计入 R2-02 之后普通 Atomic Task 的局部验收。

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

第 9 个 CTest `mpv_commands` 验证八类命令编码、非法输入、未初始化/跨线程拒绝和真实异步 reply。用户在 Windows `edbc549` 运行正常 build/test，9/9 CTest 全部通过；总测试时间 1.63 秒。

### R2-06 — Complete

`src/playback/infrastructure/mpv/properties/` 初始实现：

- `MpvPropertyRegistry` 集中拥有首批 11 个核心 property：position、duration、pause、volume、mute、speed、seekable、core-idle、eof-reached、track-list、chapter-list；
- 每个 property 具有稳定 observation id、唯一 mpv name 与明确格式；
- `MpvPropertyObserver` 要求 initialized handle，并要求 start/stop 在自身 Qt owner thread 执行；
- observe 中途失败回滚已注册 observation；start/stop 幂等；
- `MPV_FORMAT_NONE`/null 明确转成 unavailable `std::monostate`；FLAG 解码 bool，DOUBLE 解码 double；
- 错误格式和未知 observation id 返回明确诊断，不崩溃；
- track/chapter `MPV_FORMAT_NODE` 的注册入口由本任务建立，真实深拷贝 decode 在 R2-07 接通。

第 10 个 CTest `mpv_properties` 覆盖 registry 唯一性、未初始化/跨线程拒绝、真实 pause/volume/mute/speed observation、None/null、错误格式、未知 id、Node 入口和重复 start/stop。

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

- 已知/未知 libmpv error mapping；
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

### R2-11 — Implemented; Windows verification pending

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

`playback_probe_matrix` 已直接注册为第 14 个 CTest，并设置 90 秒 CTest 总超时；内部每一步单独 10 秒超时。首次用户 Windows 构建在 CTest 之前失败：MSVC C1083 分别报告 `scenarios/playback_probe_scenario_catalog.cpp` 无法找到 `fixtures/playback_probe_media_set.h`，以及 AutoMOC 编译 `playback_probe_scenario_runner.h` 时无法找到 `runtime/playback_probe_runtime.h`。两者属于同一 CMake target include-root 缺口；现已在 `tools/playback_probe/CMakeLists.txt` 为 `playback_probe` 增加私有 `${CMAKE_CURRENT_SOURCE_DIR}` include root。该修复不修改场景逻辑、公共接口、生产依赖或用户可观察播放行为。修复后的 Windows build 与 **14/14 CTest 尚未执行，不声明通过**。

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
- R2-11：首次 Windows build 在进入 CTest 前因 probe target 缺少模块 include root 失败；C1083 日志已确认 `fixtures/...` 与 AutoMOC 下的 `runtime/...` 两条同源路径。CMake 修复已提交，**修复后的 build 与 14/14 Windows CTest 尚未执行，不声明通过**；
- `Player.exe` 可正常启动并保持响应；
- `player.log` 根目录落盘问题仍为单独已知缺口，不阻塞当前普通 R2 Atomic Task。

R2-10 已完成；R2-11 当前为 Implemented、build fix committed、待 Windows 重新验收。R2-11 通过后，跨阶段补强矩阵中的 R2-09~R2-11 即全部关闭，Stage R2 才可进入最终关闭判断。已知的仓库根 `player.log` 落盘缺口继续按既有记录作为非阻塞事项保留，后续在相关诊断/发布门禁前关闭。

## Change Log

### 2026-08-08

- Accepted R2-10 after the user confirmed the real Windows environment passed all 13 CTests, including `mpv_property_baseline`, in 2.40 seconds total.
- Implemented R2-11 as a modular scripted `playback_probe --matrix` path while preserving the accepted R2-08 `playback_probe <source>` behavior.
- Added separate generated-media fixture, scenario catalog/executor, matrix orchestration and bounded typed trace responsibilities instead of extending the existing R2-08 runner into a multi-purpose state machine.
- Added all eight required R2-11 real libmpv scenarios: transport sequence, paused seek, consecutive seek, immediate A->B replacement, EOF/stop distinction, load error, shutdown during loading and shutdown during playback.
- Kept shutdown scenarios on queued teardown so the runtime is never destroyed from inside the synchronous `MpvEventLoop` drain signal stack.
- Registered `playback_probe --matrix` as the 14th CTest `playback_probe_matrix`; expected Windows regression gate is now 14/14.
- Kept generated WAV files temporary and probe-local; R0-06 remains skipped and no distributable external fixture policy is falsely claimed complete.
- Recorded the first R2-11 Windows build failure before CTest: MSVC C1083 could not resolve probe-root `fixtures/...` and AutoMOC `runtime/...` includes because the target had no module include root.
- Fixed that single build-system root cause by adding `${CMAKE_CURRENT_SOURCE_DIR}` as a PRIVATE include directory of `playback_probe`; no scenario logic or production interface changed.
- Accepted R2-09 after the user confirmed the real Windows environment passed all 12 CTests, including `mpv_event_semantics`, in 2.31 seconds total.
- Implemented R2-10 Property Baseline expansion from 11 to 22 centralized registry entries while preserving the original observation IDs and all existing observer lifecycle behavior.
- Added centralized seeking/cache/media identity/current track/video/audio property entries; flexible native values remain Node-backed and raw property strings do not escape `infrastructure/mpv`.
- Added STRING property decoding into a Qt-owned `QString` value and explicit null-string unavailable handling.
- Added the independent `mpv_property_baseline` CTest for the R2-10 registry/format/deep-copy contract.
- Kept R2-10 inside infrastructure only: no PlaybackSnapshot, ViewModel property lookup, Playlist state, new production dependency, configuration change or user-visible playback behavior was introduced.
- Implemented R2-09 event-semantics hardening: `MpvEndFileData` now preserves the raw libmpv end-file reason alongside the stable typed reason, error and playlist boundary metadata.
- Added a separate `mpv_event_semantics` CTest instead of further growing the existing decoder/integration test; it covers all known end reasons plus unknown raw reason retention, null/none property semantics, command success/error identity, unknown/malformed events and null log strings.
- Kept R2-09 strictly inside the mpv infrastructure boundary: no PlaybackSession, MediaGeneration, auto-advance decision, new production dependency, configuration change or user-visible playback behavior was introduced.
- Accepted R2-07 after the user confirmed the real Windows environment passed all 11 CTests, including `mpv_event_decoder`, in 2.18 seconds total.
- Implemented R2-08 modular console playback probe with separate CLI, orchestration state machine and mpv runtime-lifecycle ownership.
- Added the real headless control sequence `load -> pause -> play -> relative seek -> stop -> end-file -> close`, per-step diagnostics and 15-second timeouts without adding PlaybackSession or product playback state.
- Deferred probe runtime teardown through Qt queued finalization so an `eventDecoded` callback never destroys `MpvEventLoop` while its drain stack is still active.
- Added a probe-only `config=no + vo=null + ao=null` profile and direct Qt6 Core/libmpv runtime staging so the console probe does not depend on a QML window, user mpv.conf or physical audio output.
- Accepted R2-08 after the user confirmed all 11 CTests still passed in 1.47 seconds, a real local media probe completed the full headless control chain with PASS, and a missing-media probe produced `end-file(reason=error)` with `loading failed` and a non-zero failure exit path.
- Corrected the documented playback probe output path to `build/windows-msvc-debug/cmake/playback_probe.exe`; no source, public-interface, dependency or runtime behavior change was required for this correction.
- Corrected the premature R2 closure: the mandatory mature-player behavior supplement adds R2-09 through R2-11, so Stage R2 remains open.
- Kept the accepted R2-08 Windows results unchanged; this correction changes documentation/status only and does not modify source, dependencies, interfaces or runtime behavior.
- Accepted R2-06 after the user confirmed the real Windows build and all ten CTests passed, including `mpv_properties`, in 1.88 seconds total.
- Implemented R2-07 typed event boundary: raw `mpv_event` is decoded inside `infrastructure/mpv` into Qt-owned `MpvEvent` before the next `mpv_wait_event` call.
- Added `errors/MpvErrorMapper` with known/unknown libmpv error mapping and preserved raw diagnostics.
- Added recursive Node deep-copy for track/chapter payloads, eliminating libmpv node-pointer lifetime from the outward event contract.
- Migrated the R2-04 event loop and R2-05 command-reply tests from raw metadata signals to typed `MpvEvent` delivery.
- Added the eleventh `mpv_event_decoder` CTest with synthetic event/error/Node cases plus a runtime-generated silent PCM WAV for a real short-media typed event sequence.
- Kept the generated WAV strictly test-local; R0-06 remains skipped and no complete external media-fixture policy is claimed.
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

## R2-11 local verification

R2-11 已把行为矩阵接入标准 CTest，不需要手工准备第二个媒体文件。首次 Windows build 已确认因 probe target 缺少模块 include root 而在 CTest 前失败；该问题已经修复。请从仓库根目录重新执行：

```powershell
git pull --ff-only origin agent/r2-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r2.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

预期 CTest 总数从 13 增加到 **14**，新增项为：

```text
playback_probe_matrix
```

必须达到 **14/14 passed, 0 failed**。`playback_probe_matrix` 会真实启动 libmpv 并依次运行全部 8 个 R2-11 场景；失败时 `scripts/test.ps1` 会打印对应场景的 bounded typed trace。

如需主动查看完整成功 trace，可单独执行：

```powershell
.\build\windows-msvc-debug\cmake\playback_probe.exe --matrix
```

成功结尾应包含：

```text
[playback_probe] MATRIX PASS: 8/8 scripted headless scenarios
```

R2-01 记录的仓库根 `player.log` 落盘问题仍是明确的非阻塞缺口，不因 R2-11 实施而伪装为已解决；应在后续相关诊断/发布门禁前单独关闭。

**Stage R2：In Progress。R2-10：Complete。R2-11：Implemented; build fix committed; Windows verification pending。**
