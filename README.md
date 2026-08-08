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
- Windows 构建后显式 `windeployqt` 与 `.player-development-root` 开发标记；
- R2-04 代码加入后测试集目标数为 8 个；其中前 7 个已在用户 Windows 环境通过，新增 `mpv_event_loop` 等待本机验收。

当前仍未进入的职责：异步 playback command、Property registry/observer、完整 event decoder、PlaybackSession、Render API、数据库、播放列表、完整播放器 UI 与安装包。这些继续按阶段任务书推进，不提前堆入现有模块。

## Product scope

首版目标是一个以 libmpv 为播放内核、Qt Quick/QML 为界面的 Windows 10/11 x64 桌面播放器，同时保持 macOS/Linux 清晰适配边界。

MVP 包括：本地文件、网络 URL、播放/暂停/停止、绝对/相对 Seek、Timeline、音量/静音、倍速、全屏、播放列表、音轨、字幕、外挂字幕、音画延迟、章节、媒体信息、错误反馈、加载/缓冲/暂停/结束状态、最近播放、恢复进度、快捷键、系统媒体键、防休眠、单实例、文件关联和 Windows 安装包。

第二阶段再做截图、A-B 循环、画面比例/裁剪/旋转、字幕样式、播放质量预设、Shader、迷你播放器、画中画、高级统计和 macOS/Linux 适配。在线站点解析、媒体服务器、DLNA/AirPlay/Chromecast、账号、云同步、在线字幕搜索、插件市场、视频剪辑/转码和 AI 字幕/画质增强不属于首版。

## Architecture boundary

```text
QML presentation
      ↓ intent / projected state
Application layer (PlaybackSession arrives in R3)
      ↓ commands / events
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

src/playback/infrastructure/mpv/events/
  MpvWakeupBridge 只把 libmpv 内部线程 wakeup 转为 Qt queued signal；
  MpvEventLoop 只在其 QObject 所属线程执行 mpv_wait_event(handle, 0) drain。
  callback 不调用普通 libmpv API，不承载业务逻辑，也不建立 PlaybackSnapshot。

src/presentation/qml/
  只负责 presentation；禁止直接 mpv_command/mpv_set_property/C 指针访问。
```

R2-04 的关闭顺序要求是硬约束：先将 wakeup bridge 标记为 inactive，再通过 `mpv_set_wakeup_callback(handle, nullptr, nullptr)` 注销 callback，等待已进入 callback 的短路径退出，最后才允许目标事件循环对象销毁。已排队到 Qt 的 drain 在 `running_ == false` 时直接 no-op。

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
```

`build.ps1` 在成功生成 Player 后写入并验证：

```text
build/<preset>/.player-development-root
../..
```

随后显式执行匹配配置的 `windeployqt`。`test.ps1` 会在运行 CTest 前再次验证开发 marker，避免旧产物伪造绿色结果。

## Atomic Task status

### R0

- R0-01 ~ R0-05：Complete。
- R0-06：按用户明确要求 Skipped；媒体 fixture 合法性完整闭环未声明完成。

### R1

R1-01 ~ R1-06：Complete。用户 Windows 环境已经验证 configure/build/test、Qt Quick 壳、OpenGL probe、ApplicationContainer 生命周期等 R1 基线。

### R2-01 — Implemented; file-log acceptance deferred

固定 libmpv target、受控源码构建、manifest/hash、runtime probe、Qt runtime staging 与 5 个当时 CTest 均已在用户 Windows 环境验证。`Player.exe` 可以启动并保持响应，Qt/MSVC Debug DLL 从 `build/windows-msvc-debug` 加载。

仍存在一个明确记录的非阻塞缺口：开发模式预期的仓库根目录 `player.log` 没有生成。用户已明确要求不让该问题继续阻塞 R2 普通框架任务。它必须在后续相关诊断/发布门禁前重新关闭，但不计入 R2-02/R2-03/R2-04 的局部验收。

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

### R2-04 — Implemented; Windows build/test acceptance pending

新增 `src/playback/infrastructure/mpv/events/`：

- `MpvWakeupBridge` 注册 `mpv_set_wakeup_callback`；callback 本身只发出 Qt wakeup signal，不调用普通 libmpv API、不解析事件、不做业务决策；
- wakeup signal 强制 `Qt::QueuedConnection` 投递到 `MpvEventLoop` 所属 Qt 线程；
- `MpvEventLoop::start()` 要求 handle 已初始化并要求从自身 Qt 所属线程启动；
- 所有 `mpv_wait_event(handle, 0.0)` drain 都发生在该所属线程；
- event loop 当前只向 infrastructure 测试/后续模块暴露复制后的 event id、reply userdata、error 三项轻量 metadata，不把 raw `mpv_event*` 或 raw node 传出模块；
- `stop()` 先令 `running_` 失效，再注销 wakeup callback，并等待已经进入 callback 的短路径退出；重复 stop 安全；
- destructor 复用同一停止路径，已排队 drain 在停止后 no-op；
- 没有提前实现 R2-05 command encoder/executor、R2-06 property registry、R2-07 event decoder 或 R3 PlaybackSession。

新增第 8 个 CTest `mpv_event_loop`，覆盖：未初始化 handle 拒绝启动、真实 property observation 触发 wakeup、事件在 Qt owner thread drain、重复 start/stop、stop 后不再 drain、析构先停用 callback 再销毁 target。

当前连接环境无法运行用户的 Windows Qt/MSVC/libmpv 二进制，因此 R2-04 的本机 build 与第 8 个 CTest 仍待用户验证。

## Validation record

已由用户 Windows 工作区真实确认：

- CMake 3.30.5、Ninja 1.12.1、Qt 6.8.3、VS 2022 17.14、MSVC 19.44/14.44、Windows SDK 10.0.26100.0 基线通过；
- 完整 libmpv 依赖源码构建成功，FFmpeg 8.0.3 官方签名验证成功，最终 package 17 个 manifest artifact hash 全部通过；
- 自动 Qt runtime deployment 与开发 marker 链通过；
- R2-02：6/6 CTest 通过；
- R2-03：7/7 CTest 通过，`mpv_initialization` 通过；
- `Player.exe` 可正常启动并保持响应；
- `player.log` 根目录落盘问题仍为单独已知缺口，不阻塞当前普通 R2 Atomic Task。

R2-04 当前尚未在连接环境执行真实 build/test。下一次本机验收应报告 8 个 CTest，其中新增 `mpv_event_loop` 必须通过。

## Change Log

### 2026-08-08

- Accepted R2-03 after the user confirmed the real Windows build and all seven CTests passed, including `mpv_initialization`.
- Implemented R2-04 `events/` module with `MpvWakeupBridge` and `MpvEventLoop`: libmpv internal-thread wakeups are marshalled through a queued Qt signal, while all `mpv_wait_event(0)` draining remains on the event loop's owning Qt thread.
- Added shutdown protection for the wakeup bridge: deactivate first, unregister the libmpv wakeup callback, wait for callbacks already in flight, and make queued drains no-op after stop.
- Added the eighth `mpv_event_loop` CTest covering real wakeup delivery, owner-thread draining, idempotent start/stop, post-stop suppression, and destructor shutdown ordering.
- Kept R2-04 limited to event transport/lifecycle; command encoding, property registry, full event decoding, PlaybackSession and UI behavior remain in later Atomic Tasks.
- Accepted R2-02 after the user confirmed six CTests passed, including the `MpvHandle` lifecycle/100-cycle regression.
- Implemented R2-03 initialization profile with centralized `config=no`, pre-initialize option application, validation and failure cleanup.
- Recorded the unresolved R2-01 repository-root `player.log` as a non-blocking validation gap by explicit user direction.
- Completed the controlled libmpv source-build/package verification chain and explicit Qt runtime deployment path described above.

### 2026-08-07

- Started Stage R2 after accepted R1-06.
- Added the project-controlled MSYS2 CLANG64 libmpv source-build pipeline and fixed source/archive acquisition boundaries.
- Kept all third-party binaries/build trees outside Git and preserved repository-parent-relative dependency paths.

## Current R2 local verification

For R2-04, do not retest the deferred `player.log` issue. Pull, build and run the normal suite:

```powershell
git pull --ff-only origin agent/r2-stage
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 > build-r2.log 2>&1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

R2-04 acceptance requires a successful build and **8/8 CTest** with `mpv_event_loop` passing. If this gate is green, the next Atomic Task is R2-05 async command encoder/executor.
