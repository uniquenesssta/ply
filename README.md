# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器工程。当前处于 R2：无 UI libmpv 播放核心阶段。

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
- Windows 构建后显式 `windeployqt` 与 `.player-development-root` 开发标记；
- R2-07 已由用户 Windows 环境完成 11/11 CTest 验收，总测试时间 2.18 秒。

R2 的源码职责已经全部落地；当前只剩 R2-08 在用户 Windows 环境用本地合法媒体完成真实 `load -> pause -> play -> seek -> stop/end-file -> close` 验收。PlaybackSession、Render API、数据库、播放列表、完整播放器 UI 与安装包继续在 R3 及后续阶段实现，不提前堆入当前模块。

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
  MpvPropertyRegistry 集中拥有核心 property 名、稳定 observation id 与期望格式；
  MpvPropertyObserver 负责 observe/unobserve、Qt owner-thread 约束和 property payload decode；
  MpvNodeDecoder 把 track/chapter 等 MPV_FORMAT_NODE 深拷贝成 QVariant/QVariantList/QVariantMap/QByteArray。

tools/playback_probe/
  main.cpp 只处理 CLI/进程退出；PlaybackProbeRunner 只拥有探针步骤状态机和超时；
  runtime/PlaybackProbeRuntime 只组合并按正确顺序销毁 MpvHandle/Observer/EventLoop/Executor；
  探针采用 config=no + vo=null + ao=null，不建立 PlaybackSnapshot、不复制未来 R3 业务状态机。

src/presentation/qml/
  只负责 presentation；禁止直接 mpv_command/mpv_set_property/C 指针访问。
```

R2-04 的关闭顺序仍是硬约束：先令 wakeup bridge inactive，再注销 libmpv wakeup callback，等待已经进入 callback 的短路径退出，最后才允许事件循环目标对象销毁；停止后的 queued drain 直接 no-op。

R2-05 保留 `reply_userdata` 作为后续 R3 command tracker 的关联键，但 executor 不建立 pending map、不保存播放状态真值。

R2-06/R2-07 的 property 数据链现在是：registry 定义观察身份与格式，observer 管理观察生命周期，event decoder 在消费 raw property event 时调用 property decode；FLAG/DOUBLE/None 直接转成内部值，Node 通过独立 `MpvNodeDecoder` 深拷贝为 Qt-owned 数据。因此事件循环发出 `eventDecoded(const MpvEvent&)` 后，上层无需再解释或持有 libmpv C payload。

R2-08 只消费上述既有接口。probe runtime 自己拥有 headless option profile 与 mpv 对象生命周期，runner 只根据 typed event/reply 推进 `load -> pause -> play -> relative seek -> stop -> end-file`，每一步都有 15 秒超时；成功/失败通过 queued finalization 等当前 event drain 返回后再关闭 runtime，避免在 `MpvEventLoop::drainPendingEvents()` 栈内销毁 EventLoop；失败路径返回非零退出码并打印 libmpv typed diagnostic。

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
build/windows-msvc-debug/tools/playback_probe/playback_probe.exe
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

`src/playback/infrastructure/mpv/properties/` 已实现：

- `MpvPropertyRegistry` 集中拥有 11 个核心 property：position、duration、pause、volume、mute、speed、seekable、core-idle、eof-reached、track-list、chapter-list；
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

### R2-08 — Implemented; local-media runtime acceptance pending

新增 `tools/playback_probe/`，按职责拆分为：

- `main.cpp`：只负责 `playback_probe <local-media-or-url>` 参数、进程事件循环和退出码；
- `PlaybackProbeRunner`：只负责 load/pause/play/seek/stop/end-file 的异步步骤状态机、request id 关联、逐步输出、15 秒步骤超时，以及 queued finalization；
- `runtime/PlaybackProbeRuntime`：只负责 probe 的 headless option profile、MpvHandle/Observer/EventLoop/Executor 组合、提交入口和关闭顺序。

probe 不访问 raw `mpv_event`/`mpv_node`，不调用散落的 `mpv_command_*`，不保存产品播放真值；它完全复用 R2-02~R2-07 已建立的 typed infrastructure。probe profile 使用 `config=no + vo=null + ao=null`，避免用户 mpv.conf、视频窗口和本机音频设备成为 R2 headless 验收前提。

CMake 新增顶层 `tools/` 构建入口；`playback_probe` 链接 `player_mpv_infrastructure + LibMpv::LibMpv + Qt6::Core`，复用固定 libmpv runtime staging，并额外复制匹配配置的 Qt6 Core DLL，保证用户可直接从 build 输出目录运行探针。

当前连接环境不能执行用户 Windows Qt/MSVC/libmpv 二进制，因此 R2-08 源码/CMake/边界已实现，但不能在这里声明本机合法媒体主链已经通过。R2-08 的完成门禁是：正常 build、既有 11/11 CTest 不回归、一个本地合法短媒体完整输出 PASS，以及一个不存在/不可加载媒体返回非零并打印明确错误。

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
- `Player.exe` 可正常启动并保持响应；
- `player.log` 根目录落盘问题仍为单独已知缺口，不阻塞当前普通 R2 Atomic Task。

R2-08 尚未在用户 Windows 环境执行合法媒体 probe；该真实运行门禁通过前不标记 R2-08 Complete，也不进入 R3。

## Change Log

### 2026-08-08

- Accepted R2-07 after the user confirmed the real Windows environment passed all 11 CTests, including `mpv_event_decoder`, in 2.18 seconds total.
- Implemented R2-08 modular console playback probe with separate CLI, orchestration state machine and mpv runtime-lifecycle ownership.
- Added the real headless control sequence `load -> pause -> play -> relative seek -> stop -> end-file -> close`, per-step diagnostics and 15-second timeouts without adding PlaybackSession or product playback state.
- Deferred probe runtime teardown through Qt queued finalization so an `eventDecoded` callback never destroys `MpvEventLoop` while its drain stack is still active.
- Added a probe-only `config=no + vo=null + ao=null` profile and direct Qt6 Core/libmpv runtime staging so the console probe does not depend on a QML window, user mpv.conf or physical audio output.
- Kept R2-08 completion pending until the user runs one local legal media success path plus one invalid-media error path on Windows; existing 11 CTests remain the regression gate.
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

## Current R2 local verification

R2-08 不需要重新关闭已延后的 `player.log`。从仓库根目录执行：

```powershell
git pull --ff-only origin agent/r2-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r2.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

上面必须保持 **11/11 CTest** 全绿。然后用一个你本机合法、时长至少数秒的媒体文件运行：

```powershell
.\build\windows-msvc-debug\tools\playback_probe\playback_probe.exe "F:\path\to\sample.mp4"
```

成功路径必须最终打印：

```text
[playback_probe] PASS: load -> pause -> play -> seek -> stop -> end-file -> close
```

再验证一个错误路径：

```powershell
.\build\windows-msvc-debug\tools\playback_probe\playback_probe.exe "F:\definitely-not-exist\missing.mp4"
```

错误路径必须返回非零退出码并打印明确 `FAIL`/libmpv 诊断，不能 crash、deadlock 或超过步骤超时。两条 probe 路径都满足后，R2-08 才可标记 Complete，并进入 R3。
