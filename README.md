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

### 2026-08-15 — R6-16 HUD Coalescing Complete / Stage R6 Complete

- 在既有 R6-12 `HudMessageQueue` 上补强成熟 HUD 合并策略，继续由该对象独占当前消息、pending queue 与**唯一一个 C++ single-shot `QTimer`**；没有新增第二套 HUD queue/Timer、Playback 真值或新的生产依赖。
- 原隐式 `MessageKind` 收敛为明确 `CoalescingKey + Priority`：Volume/Mute 共用 Volume key；Seek 连续结果共用 Seek key；Speed 连续 +/- 共用 Speed key；Track change 使用独立 Track key，因此 Track 不会与 Volume 合并。相同 key 更新当前或 pending 的最新值，不追加历史重复消息。
- `seekFailed` 作为当前已有的重要 HUD 反馈提升为 Important priority：它可以立即抢占 transient HUD，并清理已经排队的低优先级旧反馈；Important 正在显示时，后续 Volume/Seek/Speed/Track 只能进入 bounded pending，不能覆盖当前重要反馈。Fatal Playback Error 继续由 R6-11 Status Overlay 单一持有；`PlayerScreen` 只把既有 `errorOverlayVisible` 传给 `PlayerHudOverlay.suppressed`，Error 生效时 HUD 立即停止渲染而不是等待淡出，避免 z70 HUD 覆盖更重要的 Error。
- pending queue 保持硬上限 **3**；满队列时优先淘汰最旧 transient，低优先级新消息不能挤掉重要 pending，从策略上禁止无限积压。`clear()` 仍统一停止唯一 hold timer 并清空 current/pending，composition shutdown 语义不变。
- `PlayerHudOverlay` 补齐 `speed` / `track` 的展示 label 并增加 Error suppression，继续复用既有 `Surfaces.Hud`、Typography、Motion 与 z70；没有新增图标资产、QML Timer、Toast/Dialog 或业务命令。当前真实 producer 仍只有成功提交后的 Volume/Mute、Seek 与 Seek failure；Speed 虽有 Domain command 但 R6 尚无 Presentation action，Track selection 属于 R8，因此 `PlaybackComposition` 明确不接入 `showSpeed/showTrackChange`，不伪造未来功能。
- 新增独立 `player_hud_coalescing_policy` CTest，覆盖 Speed burst 合并、Track 与 Volume 分离、Important 抢占与低优先级不可覆盖、bounded pending 淘汰最旧 transient、空未来消息忽略；既有 `player_hud_message_queue` 与 `player_hud_overlay` 继续作为 R6-12 回归门禁。Windows 锁定环境最终验收：`configure.ps1` **PASS**（Configuring **4.7 s** / Generating **2.4 s**）；Debug `build.ps1` 完成 **124/124**，development runtime root marker 与 Qt runtime deployment PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **76/76 CTest PASS，0 failed，71.23 s**。其中 `player_hud_overlay` **1.18 s PASS**、`player_hud_message_queue` **6.38 s PASS**、`player_hud_coalescing_policy` **3.87 s PASS**，既有 Playback/Render/Presentation 回归全部保持通过。
- 随后执行 `Player.exe` startup/exit smoke；`player-20260815-201501.log` 仅包含 INFO，记录 libmpv 0.41.0 / FFmpeg 8.0.3、NVIDIA OpenGL 4.6 初始化，并以 `Application stopping with exit code 0` 正常结束，未见 WARN/ERROR/CRITICAL 或 QML runtime error。验证前 `git status --short` 仍显示受保护的 `.gitignore` 修改与 `r4-04-qml-diagnostics/` 未跟踪目录；本轮验证记录中没有执行 reset/clean。
- 产品媒体打开入口仍属于 R7，因此正式产品 UI 以真实媒体触发 Volume/Seek HUD 的手工矩阵继续在 R7 后回归；Speed 的真实 presentation producer 与 Track change producer 分别留给其所属后续功能阶段，不伪装为 R6 已存在。R6-16 的 Queue/Overlay/priority/bounded backlog/Error suppression 自动化与 startup/runtime 门禁均已通过。**R6-16 正式 Complete；R6-01~R6-16 全部完成，Stage R6 重新关闭为 Complete；下一阶段按任务书进入 R7。**

### 2026-08-15 — R6-15 Cursor Visibility Policy Complete

- 在既有 R6-10 `PlayerCursorVisibilityController` 上补强成熟 Cursor Policy，继续让 Cursor 独立消费 R6-14 的 `oscVisible` 结果而**不拥有 inactivity Timer**；控制栏 inactivity 仍只有 `PlayerChromeVisibilityController` 的单一 Timer，Cursor 没有复制 2200/1600 ms 超时或建立第二套 Playback 真值。
- Cursor 隐藏条件收敛为：Playing + OSC Hidden + Window active + pointer 位于播放器窗口内，并且不存在 Scrub/Drag、Popup/Menu/Drawer/Modal、Error、显式 suppression 或窗口恢复锁。窗口失焦、重新激活、pointer leave/re-enter 都先恢复可见，并要求后续真实 pointer activity 释放恢复锁，避免窗口重新获得焦点时瞬时重新隐藏。
- `PlayerChromeActivityLayer` 继续是窗口级 pointer sensor 和 `Qt.BlankCursor : undefined` 的唯一应用点，只新增只读 `pointerInside` 投影；pointer activity 先通知 Chrome 使控制栏恢复/刷新 inactivity，再通知 Cursor 释放 restore latch，没有全局 `QGuiApplication::setOverrideCursor()` 或 shutdown 时“恢复系统全局鼠标”的副作用路径。
- Timeline drag 继续复用既有 `timelineInteractionActive`；当前已经存在的 Volume Slider drag 不再只靠未来占位输入，`VolumeControls` 仅把既有 `volumeSlider.pressed` 暴露为只读 `interactionActive`，`PlayerScreen.controlsDragActive` 将其与显式外部 `dragActive` 合并后交给同一 Cursor Policy。没有把 Slider 状态机复制进 Cursor controller。
- R6-14 已建立的 `popupOpen/menuOpen/drawerOpen/modalActive` presentation 边界继续复用；当前 R6 尚无真实 Menu/Inspector Drawer/Modal producer，本任务不创建假弹层或第二套 opened state。Focus within controls 仍通过 R6-14 保持 OSC visible，从而自然阻止 Cursor Hidden；R6-15 不复制 Focus owner。
- 新增独立 `player_cursor_visibility_policy` CTest，覆盖 Scrub/Drag/Popup/Menu/Drawer/Modal/Error/suppression 强制可见、pointer leave/re-enter、focus loss/activation restore、快速 10 次 activation 循环、单 Timer owner、window-local cursor application 与 Volume live drag route；既有 `player_cursor_visibility` 继续作为 R6-10 回归门禁。Windows 锁定环境最终验收：`configure.ps1` **PASS**（Configuring **4.8 s** / Generating **2.3 s**）；Debug `build.ps1` 完成 **112/112**，development runtime root marker 与 Qt runtime deployment PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **75/75 CTest PASS，0 failed，66.75 s**。其中 `player_chrome_visibility` **0.27 s PASS**、`player_controls_visibility_policy` **0.28 s PASS**、`player_cursor_visibility` **0.22 s PASS**、`player_cursor_visibility_policy` **0.30 s PASS**，既有 Playback/Render/Presentation 回归全部保持通过。
- 随后执行 `Player.exe` startup/exit smoke；`player-20260815-185721.log` 仅包含 INFO，记录 libmpv 0.41.0 / FFmpeg 8.0.3、NVIDIA OpenGL 4.6 初始化、window deactivation/activation 以及 pointer enter/move/leave 的 R6-15 cursor restore 状态，并以 `Application stopping with exit code 0` 正常结束，未见 WARN/ERROR/CRITICAL。Build 仍出现 Qt 6.8.3 `qjsprimitivevalue.h/qjsengine.h/qvariant.h` 的 MSVC C4702 system-header warning，没有新增 suppression/白名单，也未形成 build/test/runtime 阻断。
- 产品媒体打开入口仍属于 R7，因此真实 Playing 下的 idle hide / window leave / focus restore / scrub / volume drag / popup 交叉手工矩阵当前仍不可执行；该项继续保留到 R7 媒体入口可用后回归，不伪装为已手工覆盖。验证前后本地受保护 `.gitignore` 修改与 `r4-04-qml-diagnostics/` 保持存在，没有 reset/clean 或覆盖。**R6-15 正式 Complete；Stage R6 仍为 Supplemental In Progress；下一补充任务 R6-16 HUD Coalescing。**

### 2026-08-15 — R6-14 Controls Visibility Policy Complete

- 在既有 R6-09 `PlayerChromeVisibilityController` 上补强成熟显隐策略，继续由它独占 `Hidden / Rest / Active` 与**唯一一个** `oscInactivityTimer`；没有建立第二套 controls visibility state、第二个 inactivity Timer 或新的 Playback 真值。R6-09 已验证的同步决策继续直接读取 `isVisibilityLocked()` primitive inputs，避免重新引入派生 binding 求值顺序竞态。
- visibility lock 现在显式覆盖：无媒体/Paused（由 `!playing` 表达）、Timeline Scrubbing/Pending、鼠标位于实际 OSC Surface、播放器控件键盘 focus、Popup、Menu、Drawer、Modal 与 Error。任一 lock 激活都会把 Hidden 恢复为可见并停止 Timer；lock 释放后重新按当前 Windowed/Fullscreen delay 计算 inactivity，而不是沿用已经过期的 timeout。
- `PlayerOscLayout` 升级为 `FocusScope`，只暴露 `controlsFocused` 与实际 `OscSurface` 范围内的 `controlsHovered`；hover 使用被动 `HoverHandler`，没有 MouseArea、Timer 或业务命令。`PlayerTopRegion` 同样只通过 `FocusScope.activeFocus` 暴露 Header/Window Actions 的键盘 focus。显隐规则仍全部集中在 Chrome controller，布局/Host 不自行决定何时隐藏。
- `PlayerScreen` 继续作为组合层：把 OSC hover、Top/OSC focus、既有 `popupOpen` 以及新增的 `menuOpen/drawerOpen/modalActive` presentation 输入传给同一 controller。当前 R6 尚没有真实 Menu/Inspector Drawer/Modal producer，因此这些只是后续 Feature 的显式接入边界，本任务不创建静态假控件、假 opened state 或第二套 Overlay owner。
- Pointer move/enter 仍复用既有 `PlayerChromeActivityLayer → notifyActivity()`；有效活动会进入 Active 并刷新同一 inactivity window。Windowed 仍为 **2200 ms**、Fullscreen **1600 ms**，切换窗口模式只改变该 Timer 的 interval。新增回归还要求快速 pause/play 往返 10 次始终只有同一个 Timer owner。
- R6-15 Cursor Policy 未提前实现：R6-14 只通过保持 `oscVisible=true` 间接阻止 cursor 进入 Hidden，Menu/Drawer/Modal/focus-loss 等 cursor 自身的完整恢复策略继续归 R6-15 收口。
- 新增独立 `player_controls_visibility_policy` CTest，覆盖 Scrub、OSC hover、control focus、Popup/Menu/Drawer/Modal、Error、Paused lock；lock release 重算 timeout；pointer wake；Windowed/Fullscreen 单 Timer；快速 pause/play 单 owner；以及 PlayerScreen/OSC/Header 的模块边界。既有 `player_chrome_visibility` 与 `player_cursor_visibility` 继续作为 R6-09/R6-10 回归门禁。
- Windows 锁定环境最终验收：`configure.ps1` **PASS**（Configuring **4.7 s** / Generating **2.4 s**）；Debug `build.ps1` 完成 **107/107**；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **74/74 CTest PASS，0 failed，66.46 s**，其中 `player_controls_visibility_policy` **0.46 s PASS**、`player_chrome_visibility` **0.28 s PASS**、`player_cursor_visibility` **0.12 s PASS**，既有 Playback/Render/Presentation 回归全部保持通过。
- 随后执行 `Player.exe` startup/exit smoke；`player-20260815-183234.log` 仅包含 INFO，记录 libmpv 0.41.0 / FFmpeg 8.0.3、NVIDIA OpenGL 4.6 初始化、`R6-14 OSC visibility Active reason=pointer-enter` 以及 `Application stopping with exit code 0`，未见 WARN/ERROR/CRITICAL。验证前本地受保护 `.gitignore` 修改与 `r4-04-qml-diagnostics/` 保持存在，没有 reset/clean 或覆盖。
- 产品媒体打开入口仍属于 R7，因此真实 Playing 媒体下的 inactivity/hover/focus/Popup/Drawer 手工矩阵当前仍不可执行；该项继续保留到 R7 媒体入口可用后回归，不伪装为已手工覆盖。**R6-14 正式 Complete；Stage R6 仍为 Supplemental In Progress；下一补充任务 R6-15。**

### 2026-08-15 — R6-13 Timeline Interaction State Machine Complete

- 依据跨 Stage 强制补充任务书 `成熟播放器行为补强与验收矩阵.md` 重新打开 R6；R6-13 只补强既有 R6-06 Timeline 主链，不建立第二套 Timeline 或 Playback 真值，也未提前进入 R6-14 Controls Visibility、R6-15 Cursor Visibility 或 R6-16 HUD Coalescing。
- Timeline 状态职责重新拆清：`PlaybackSnapshot.timeline.positionSeconds` 仍是只读 actual truth；`TimelineScrubSession` 只拥有 pointer `Idle/Scrubbing` 与拖动 preview；`TimelineSeekProjection` 单独拥有 commit/relative Seek 的 pending target 与 MediaGeneration。拖动期间 actual update 可以进入 Snapshot，但不能抢回 thumb；cancel、generation change、non-seekable/unknown-duration、提交拒绝或已识别的 backend seek failure 都会回到最新 Snapshot truth。
- `PlayerTimelineViewModel` 提供 relative Seek 主链与固定 **5 s** presentation step；`TimelineRelativeSeekCoalescer` 以**单一 100 ms single-shot QTimer**合并键盘/滚轮高频输入。短 burst 合成一次请求，持续输入按固定窗口形成有限批次；投影 target 限制在 `0..duration`，最终状态仍由 Snapshot acknowledgement 收敛。
- `TimelineControls.qml` 保持 generic `Slider` 作为 pointer drag owner，并通过通用 `Slider.keyboardEnabled` 关闭 Timeline 内部 normalized 键盘步进；Timeline Feature 自己将 Left/Right/Up/Down 与 Wheel 转成 relative ±5 s intent。窗口失焦通过 `MainWindow.active → PlayerScreen.windowActive → TimelineControls` 取消正在进行的 Scrub 并恢复 actual position。QML 仍不接触 PlaybackSession、CommandBus、SeekCommand 或 libmpv。
- Playing、Paused、Buffering 都允许真实 seekable Timeline 拖动；Scrubbing 时 Buffering/actual position 更新不夺取 preview。`seekable=false`、无有效 MediaGeneration、unknown/zero/non-finite duration 不能进入有效 Scrubbing/relative Seek。绝对拖动 commit 只提交一个 final Absolute Seek；键盘/滚轮走 Relative Seek；二者复用同一个 `PlaybackComposition::submitSeek(seconds, mode)`、共享 RequestId generator 与单一 PlaybackCommandBus。
- `Ended → seek earlier` 不使用 QML replay hack：产品 mpv profile 使用 `keep-open=yes` 保留 EOF 后媒体；Reducer 消费 `eof-reached`，Ready+EOF → Ended/Stopped，随后 seek earlier 导致 EOF=false 时恢复 Ready/Paused，MediaGeneration 与 media identity 不重建。真实 libmpv `playback_ended_seek` CTest 使用生成 WAV 覆盖 load → play → EOF Ended → Absolute Seek earlier → same-generation Ready/Paused。
- Seek 同步提交拒绝以及 tracked backend command failure 都清理 Timeline pending projection并复用既有 R6-12 `HudMessageQueue` 显示 `seekFailed`，没有创建第二套 HUD Timer/queue。通用 request timeout 仍由既有 30 s timeout monitor 管理；timeout-specific request type 回传不属于本次已验证范围，继续作为后续成熟行为矩阵风险项保留。
- Windows 锁定环境最终验证：Debug build 完成 **257/257**，development runtime root marker 与 Qt runtime deployment PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **73/73 CTest PASS，0 failed，71.58 s**。其中真实 `playback_ended_seek` **2.98 s PASS**、`timeline_interaction_matrix` **1.47 s PASS**，Timeline/Session/Reducer/selector/mpv initializer/HUD 以及既有 Render/Presentation 回归全部保持通过。
- 随后执行 `Player.exe` startup/exit smoke；`player-20260815-175627.log` 仅包含 INFO，记录 libmpv 0.41.0 / FFmpeg 8.0.3、NVIDIA OpenGL 4.6 初始化以及 `Application stopping with exit code 0`，没有 WARN/ERROR/CRITICAL 或 QML runtime error。构建中仍出现已记录的 Qt 6.8.3 `qjsengine.h/qvariant.h` MSVC C4702 system-header warning，未新增 suppression，也未形成 build/test/runtime 阻断。
- Chapter producer 属于 R8 Chapter Feature，正式产品媒体打开入口属于 R7，因此真实产品 UI 下的鼠标 drag/键盘/Wheel/Chapter 手工矩阵当前仍不可执行；Chapter 后续必须走现有统一 Seek action/CommandBus 路径，完整交叉矩阵继续由 R8/R12-13 收口，不伪装为本阶段已手工覆盖。
- **R6-13 正式 Complete；Stage R6 仍为 Supplemental In Progress；下一补充任务 R6-14。**

### 2026-08-15 — Per-launch session logging

- 保持 `RuntimePaths → LoggingBootstrap → LogFileSink` 单一日志主链和现有 development / Installed / Portable 目录解析，不创建第二套 logger。新增职责独立的 `log_session_file_name.*`，只负责启动会话文件名与同秒冲突消解；`LogFileSink` 继续负责实际文件写入、4 MiB 单会话大小轮转、线程安全与脱敏。
- 固定追加 `player.log` 改为每次启动创建 `player-YYYYMMDD-HHmmss.log`；同一秒存在同名文件时使用 `-02`、`-03` 等后缀。旧 `player.log` 不自动删除或迁移，避免破坏用户已有诊断记录。
- Windows 实机连续启动/退出后实际生成多个独立 session 文件；用户上传的 `player-20260815-163309.log` 与 `player-20260815-163312.log` 各自只包含一次完整启动到 `Application stopping with exit code 0`，没有跨会话追加，也无运行时 WARN/ERROR/CRITICAL。日志目录仍为开发工作区上一级 `F:\QT6-PLAYER\logs\`。

### 2026-08-15 — R6-12 HUD message queue / Stage R6 Complete

- 新增职责独立的 `viewmodels/player/hud/HudMessageQueue`，只拥有短反馈的当前消息、有限 pending queue 与**唯一一个 C++ single-shot QTimer**。默认 hold policy 为 1200 ms；同类型 Volume/Mute 或 Seek 高频更新直接替换当前/待处理同类并重启当前 hold，异类型只保留有限最新待处理值，避免连续滚轮/拖动产生历史 HUD 堆积。Queue 不拥有 Playback 状态、OSC inactivity、Status Overlay 或 libmpv 生命周期。
- HUD 生产点放在 `PlaybackComposition` 的真实 CommandBus 提交结果之后：Seek、SetVolume、SetMuted 只有 `submit(...) == true` 才进入 HUD；立即拒绝继续走既有 Timeline/Volume pending rollback，**不会显示伪成功反馈**。composition stop 会清空 HUD lifecycle。Volume/Mute 复用 `PlayerVolumeViewModel` 当前 pending/ack 语义，Seek 复用 `PlayerTimelineViewModel.positionText()`，没有复制时间格式化或 Playback 真值。
- `screens/player/overlays/hud/PlayerHudOverlay.qml` 复用 R5 已冻结 `Surfaces.Hud` / `ZOrderTokens.hud=70`，作为 `PlayerScreen` 的直接 sibling 而不是 z35 `PlayerOverlayStack` 子项，因此保持 `Overlay 35 < OSC 40 < HUD 70 < Toast 80 < Dialog 100`。root `enabled:false` 且没有 PointerHandler；QML **没有 Timer**，短反馈不阻断控制。
- 初始 candidate 的 Windows qmllint/runtime 暴露 Overlay 误用不存在的 `MotionTokens.resolvedDuration` / `commonEasing`。收口修复改为消费既有 `hudShowDuration/hudHideDuration`、enter/exit easing 与 bezier token，没有新增兼容 facade、裸动画参数或 suppression；`player_hud_overlay` 同步增加回归，最终 QML lint 输出中这两个无效 API 已完全消失。
- 当前 R6 有真实 producer 的只接入 Volume/Mute 与 Seek；虽然 Domain 已有 speed command，但 R6 尚无真实 Speed presentation action，Track 切换归 R8，因此本任务不伪造 Speed/Track 控件或假 command。Queue/Overlay 保留后续真实 Feature 的明确接入边界。
- 新增 `player_hud_message_queue` 与 `player_hud_overlay` 两个独立 CTest，并扩展 `application_container` / CMake wiring；测试覆盖 volume clamp/round、同类合并、mute update、Volume↔Seek 有限排队、clear/timeout、HUD semantic mapping、非阻断/z-order/Motion、QML 无 Timer以及 Bootstrap→MainWindow→PlayerScreen 单一 queue route。全量测试数 **69 → 71**。
- 最终 Windows 锁定环境复验：`configure.ps1` **PASS**（Configuring **4.7 s** / Generating **2.3 s**）、Debug build **PASS**、`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **71/71 CTest PASS，0 failed，67.05 s**，其中 `player_hud_overlay` **0.73 s PASS**、`player_hud_message_queue` **6.27 s PASS**，logging/application_container/Status/Chrome/Cursor 以及既有 Playback/Render/Presentation 回归全部保持通过。
- 随后多次 `Player.exe` startup/exit smoke 均正常；两份最终上传日志均以 exit code 0 结束，进一步确认 HUD Motion 修复没有留下启动期 QML runtime warning，并验证“一次启动 = 一份日志”的诊断行为。
- 产品媒体打开入口仍属于 R7，因此正式产品 UI 中以真实媒体触发 Volume/Seek HUD 的完整手工矩阵尚不可执行；该项保留到 R7 后回归，不伪装为已手工覆盖。没有新增生产依赖，没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期、配置、持久化或公共播放接口。**R6-12 正式 Complete；当时 R6-01~R6-12 核心阶段按既有任务书关闭。随后识别到跨 Stage 强制补充的 R6-13~R6-16，因此 Stage R6 当前重新处于 Supplemental In Progress；R7 仍未开始。**

### 2026-08-15 — R6-11 Status Overlay Complete

- 新增独立 `viewmodels/player/status/`：`PlayerStatusKind` selector 只把 `PlaybackSnapshot` 投影为媒体状态，不拥有第二套 Playback 真值；优先规则为 `Failed → Error`、`Opening → Loading`、`Ended → Ended`、`Ready + buffering.active + transport != Paused → Buffering`，其余为 None。这样 Opening 的 Buffering 信号不会覆盖 Loading，用户主动 Paused 也不会被误显示成 Buffering。
- `PlayerStatusViewModel` 仅发布 `statusKey/visible/errorVisible/bufferingPercent`；Buffering percent 只在真实 Buffering 状态读取并限制到 0..100，未知值为 -1。该 VM 接入既有 `StatePublisher::snapshotPublished`，由 `ApplicationBootstrap → MainWindow → PlayerScreen` 显式传递，没有 command 提交、service locator 或 libmpv 依赖。
- `PlayerScreen` 原 R6-09/R6-10 `errorOverlayVisible` 占位值已收敛为 `statusViewModel.errorVisible` 的只读投影，因此 Error Overlay、OSC auto-hide lock 与 Cursor restore 使用同一真实状态来源，不再形成重复 Error state owner。
- 新增 `screens/player/overlays/status/PlayerStatusOverlay.qml`，只组合当前媒体状态；复用既有轻量 `LoadingFeedback/ErrorFeedback`，并在 `Player.Presentation.Feedback` 增加同职责的 `BufferingFeedback/EndedFeedback`。状态层不创建大型 Spinner 卡片、红色错误框或第二套 Surface；Error 只显示用户可理解的通用文案，不把 backend diagnostic/error code 暴露到 UI。
- Timeline `isScrubbing || seekPending` 时仅抑制 Buffering Overlay，确保 Seeking preview 不被 Buffering 覆盖；Loading/Ended/Error 不受该 suppression。R6-11 不创建 Timer、Toast、Dialog、Replay/Retry command 或 HUD queue，**R6-12 HUD owner 未提前实现**。
- 新增独立 `player_status_view_model`、`player_status_overlay` 两个 CTest，扩展 `feedback_controls` 与 `application_container`；全量测试 **67 → 69**。首轮 Windows 回归为 **67/69**：旧 `player_cursor_visibility` 仍禁止 Screen 组合新的 Status Overlay，且 `player_status_overlay` 把脱离 Window/Scene 的 root `visible` 当成业务可见性；两处测试契约修正后第二轮为 **68/69**。剩余失败来自测试 helper 在 `component.create()` 后依次写入 `viewModel` 与 `suppressBuffering`，导致 BufferingFeedback 曾瞬时实例化；最终改为 `createWithInitialProperties()` 原子注入初始属性，保留“suppression 生效时 BufferingFeedback 不应实例化”的强断言，生产 Overlay/selector/VM 均未为测试放宽。
- 最终 Windows 复验：Debug `build.ps1` **PASS**，development marker 与 Qt runtime deployment PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **69/69 CTest PASS，0 failed，54.71 s**，其中 `player_status_overlay`、`player_status_view_model`、R6-09 `player_chrome_visibility` 与 R6-10 `player_cursor_visibility` 均 PASS。随后 `Player.exe` startup smoke **PASS**，窗口正常打开。测试环境仍会出现已记录的字体目录 warning，未新增 suppression 或字体依赖。
- 产品媒体打开入口仍属于 R7，因此真实媒体 Loading/Buffering/Ended/Error 的产品手工切换尚不可执行；该项保留到媒体入口可用后回归，不伪装为已覆盖。**R6-11 正式 Complete；R6-12 尚未开始。**

### 2026-08-15 — R6-10 Cursor hiding Complete

- `features/player/chrome/PlayerCursorVisibilityController.qml` 作为独立 cursor visibility policy owner，只消费 R6-09 的 `oscVisible` 结果，不拥有 inactivity Timer；R6-09 继续是“何时进入沉浸态”的唯一 Timer/state owner，R6-10 没有复制 2200/1600ms delay。
- Cursor 仅在 **Playing + OSC Hidden** 且不存在 Scrubbing/Pending、Popup、Error、显式 `cursorHideSuppressed` 例外时隐藏；Paused 或任一锁定场景均保持可见。`cursorHideSuppressed` 是 presentation 层显式 suppression 输入，默认不改变现有行为。
- `PlayerChromeActivityLayer` 继续拥有 pointer activity sensor，并承担当前窗口 cursor application：隐藏态使用 `Qt.BlankCursor`，可见态恢复为 `undefined`，避免覆盖 Button/其他子控件自己的 cursor semantic。Pointer move/enter 仍先通知 R6-09 Activity policy，OSC 回到 Active 后 Cursor policy 随 `oscVisible` 自动恢复可见。
- `PlayerScreen` 仅组合 Chrome/Cursor policy：共享 `playbackPlaying`、`timelineInteractionActive`、`popupOpen`、`errorOverlayVisible` 输入，并把 R6-09 `oscVisible` 单向传给 Cursor controller；没有反向依赖、第二套状态真值、PlaybackSession/libmpv 或 R6-11/R6-12 逻辑。
- 独立 `player_cursor_visibility` CTest 覆盖 Playing+OSC Hidden、OSC wake、Scrub、Popup、Error、suppression、Paused；静态锁定 Cursor controller 无 Timer、R6-09 仍只有一个 Timer、Activity Layer 使用 `Qt.BlankCursor : undefined`。`player_chrome_visibility` 继续独立守住 R6-09 owner 与 Timer contract。
- 用户锁定 Windows 复验：`configure.ps1` **PASS**（Configuring 4.8 s / Generating 2.2 s）；Debug `build.ps1` 完成 **144/144**，development marker 与 Qt runtime deployment 均 PASS；`scripts/test.ps1` 完成 **6/6 QML lint**；全量 **67/67 CTest PASS，0 failed，54.47 s**，其中 `player_chrome_visibility` **0.37 s PASS**、`player_cursor_visibility` **0.22 s PASS**，既有 Playback/Render/Presentation 回归全部保持通过。随后执行 `Player.exe` startup smoke，PowerShell 未输出 QML/runtime 启动错误。
- 构建继续出现已记录的 Qt 6.8.3 `qjsengine.h/qvariant.h` MSVC C4702 system-header warning，没有新增 suppression/白名单，也未形成 build/test 阻断。验证前本地受保护 `.gitignore` 修改与 `r4-04-qml-diagnostics/` 仍保持存在，pull/configure/build/test 未清理或覆盖。
- 当前产品仍没有 R7 媒体打开入口，因此不能从正式产品 UI 进入真实 Playing 完整手工验证“静置 → OSC Hidden → Cursor Hidden → pointer move → OSC/Cursor restore”；该真实媒体手工矩阵保留到 R7 媒体入口可用后回归，不伪装成已执行。自动化 policy/边界与 startup 已满足本 Atomic Task 收口条件。**R6-10 正式 Complete；R6-11 开始。**

### 2026-08-15 — R6-09 OSC auto-hide Complete

- `PlayerChromeVisibilityController.qml` 是 OSC visibility / inactivity timer 的单一 owner；状态为 `Hidden / Rest / Active`，只消费 playing、scrubbing、popup、error、fullscreen 等 presentation 输入，不接触 PlaybackSession、CommandBus 或 libmpv。Paused、Scrubbing/Pending Seek、Popup、Error 均属于 visibility lock。
- `PlayerChromeActivityLayer.qml` 使用 `HoverHandler` 把 pointer enter/move 转成 activity intent；Windowed 使用 `MotionTokens.oscHideDelay=2200ms`，Fullscreen 使用 `oscFullscreenHideDelay=1600ms`。Reduce Motion 只改变 enter/exit transition duration，不改变语义 inactivity delay。
- 首轮 Windows 验证为 **65/66**，唯一失败集中在 `player_chrome_visibility`。后续诊断明确显示 `playing=true` 已进入 Active，但 Timer `running=false`；这排除了“只是等待时间不足”的解释。真实根因是 `onPlayingChanged → notifyActivity → scheduleHide()` 同步路径读取派生 `visibilityLocked` binding，而 QML 不保证派生 binding 与 change handler 的求值先后顺序。
- 最终修复将命令路径改为 `isVisibilityLocked()` 直接读取当前 primitive inputs；`visibilityLocked` 只保留只读观测值，不再用于同步 schedule/hide/reevaluate 决策。测试增加 `playing=true` 后 `visibilityLocked=false + Timer.running=true` 回归守卫；offscreen CTest 对 Timer 采用“running/interval + triggered handler”确定性验证，不再把 offscreen animation clock 当 Windows wall-clock 测量。**2200/1600ms 产品参数从未因测试失败而放宽。**
- 用户最终确认修复后全链复验通过，R6-09 按 **66/66 CTest PASS** 收口；R6-09 的单一 Timer、Paused/Scrub/Popup/Error lock、Fullscreen shorter timeout 与 pointer wake contract 保持成立。产品仍无 R7 媒体打开入口，因此真实媒体 Playing 下的长期手工 inactivity 回归继续在 R7 后补，不伪装为本阶段已具备的产品入口。
- **R6-09 正式 Complete。**

### 2026-08-14 — Pre-R6-09 runtime file diagnostics Complete

- 复用既有 `RuntimePaths → LoggingBootstrap → LogFileSink` 单一日志主链，没有创建第二套 logger、QML 专用日志系统或新生产依赖。现有 Qt message handler 自动收集 C++ `qDebug/qInfo/qWarning/qCritical`、logging category 与 QML/Qt runtime message；日志由 `Player.exe` 启动时自主创建/追加，PowerShell `Get-Item/Get-Content` 仅用于查看，不是触发条件。
- 开发态固定输出到项目根上一级 `logs/player.log`；用户工作区实际路径已验证为 `F:\QT6-PLAYER\logs\player.log`，从项目根看即 **`..\logs\player.log`**。`LogFileSink` 继续自动建目录，保留 4 MiB active file + 3 archive、线程安全写入与敏感信息 redaction。普通 Installed 仍写 `AppLocalDataLocation/logs`，Portable 仍写可执行文件旁 `logs/`。
- 首轮修正测试后全量曾恢复 **65/65 PASS**，但直接启动产品仍未生成目标日志。最终根因确认是 build/runtime development marker 契约错位：构建脚本 canonical marker 位于 `build/windows-msvc-debug/cmake/.player-development-root` 且内容 `../../..`，而旧 RuntimePaths 只查 executable directory 的 legacy `../..` marker。产品现已优先解析 canonical marker；legacy marker 仅在 canonical 不存在时兼容回退，存在但非法的 canonical marker不会被 legacy 静默覆盖。
- `runtime_paths` 与 `application_container` 测试同步改用真实 canonical build layout，避免测试构造产品实际不会生成的 marker。构建脚本本身未改，未恢复旧 marker；配置、截图、持久化路径与公共播放接口不变。
- 用户最终锁定 Windows 复验：Debug build PASS；`scripts/test.ps1` **65/65 CTest PASS，0 failed，53.73 s**；随后启动 `Player.exe`，无需额外日志触发命令即自主生成 `F:\QT6-PLAYER\logs\player.log`。实际日志包含 `File logging active`、`Pre-GUI bootstrap starting`、`Application starting`、libmpv 0.41.0 / FFmpeg 8.0.3 runtime validation、NVIDIA OpenGL 4.6 backend validation以及正常 `Application stopping with exit code 0`。**Pre-R6-09 runtime diagnostics 正式 Complete，R2-01 原日志落盘缺口关闭。**

### 2026-08-14 — R6-08 Fullscreen Complete

- Fullscreen 是窗口模式，不建立第二套 Playback 真值。新增 `shell/window/FullscreenWindowController.qml`，直接以 Qt `Window.visibility === Window.FullScreen` 作为权威状态；进入前只记录 Windowed/Maximized 恢复语义，退出时分别 `showNormal()` / `showMaximized()`。`MainWindow → PlayerScreen` 只显式传递 `fullScreen` 并转发 fullscreen intent，没有把 window-state owner 塞进 Playback ViewModel。
- 新增职责独立的 `features/player/fullscreen/FullscreenControls.qml` 与 `FullscreenGestureLayer.qml`：前者只拥有 OSC Utility 的 Fullscreen action，后者只用 `TapHandler.onDoubleTapped` 把视频双击转换为同一 toggle intent；Esc 由 Window Controller 的 `Shortcut` + `Qt.ApplicationShortcut` 统一处理，因此按钮、双击、Esc 最终共用一个进入/退出 owner。
- 重新读取最终 Figma `4:48 Framework / Fullscreen`：Fullscreen Header `4:60` 为 **440×50**，使用 Header Compact 材质/R25/MediaCompact title + `ESC` keycap；Fullscreen OSC `4:63` 为 **828×106**、R32、Compact material，Timeline 使用 11px timecode/28px inset，Transport/Volume/Utility visual glyph 为 **21px**，Volume slider 不显示。新增 `SizePrimitives.size440/size828` 与对应 semantic Layout token，没有把裸尺寸散回业务 QML。
- Fullscreen 没有复制 Timeline/Transport/Volume：`PlayerScreen` 仍组合现有 `PlayerOscLayout` 与同一三类 Feature，只把 `compact` 的模式来源改为 `fullScreen`。`PlayerOscLayout/OscSurface` 在 Fullscreen 使用 828px max width；`TransportControls` 在 compact 下复用同一 intent/VM，但把 Play 从 40px Primary glass 降为 Secondary，并用新增的通用 `IconButton.iconSizeOverride` 输出 21px visual；32px Secondary hit target 继续保持。`VolumeControls` 在 compact 下继续隐藏 Slider 并把 Volume visual 降为 21px。
- 最终 Figma 目前只有 canonical `fullscreen` glyph，没有独立 Exit-Fullscreen glyph；实现不手绘/伪造资产，进入/退出使用同一 canonical glyph，仅切换 Tooltip/Accessible description。Subtitles/Playlist 尚属后续真实 Feature，因此 R6-08 不用静态假按钮占位。
- Fullscreen Header 在当前无媒体 identity 时不渲染空玻璃块；真正媒体标题绑定仍由后续媒体入口/presentation 数据链提供。R6-08 没有提前实现 R6-09 的 inactivity Timer/OSC 自动隐藏，也没有提前实现 R10 的 FramelessWindowHint、Win32 hit-test、DWM、Snap 或 native resize。
- 新增独立 `player_fullscreen_controls` CTest，并同步修正 R6-04 Bottom Region contract：`compact` 明确代表 Fullscreen mode，而不是普通 Main Window 宽度断点。Qt 6.8 API 静态核对确认 `Shortcut/Qt.ApplicationShortcut`、`TapHandler.doubleTapped` 与 `Window.showFullScreen()/showMaximized()/showNormal()` 均属于锁定版本支持能力。
- 首轮 Windows 锁定环境验证：`configure.ps1` **PASS**（CMake Configuring 4.6 s / Generating 2.1 s）；Debug `build.ps1` **PASS**；首轮 CTest 为 **63/65 PASS、2 failed，56.34 s**。两个失败均为 Presentation contract：无视觉意义的 controller `width:0/height:0` 被 raw-metric 门禁识别，以及旧 Transport test 错误要求所有模式 Play 都固定 Primary。两处同源修复只删除零尺寸声明，并同时锁定 Main Standard Primary 与 Fullscreen compact Secondary + 21px visual，未改变 Fullscreen/Transport 生产语义。
- 最终 Windows 复验：Debug `build.ps1` **PASS**；`scripts/test.ps1` 完成 QML lint；**65/65 CTest PASS，0 failed，53.75 s**，`player_fullscreen_controls` **0.02 s PASS**，Playback/Render/Presentation 全量回归保持通过；随后 `Player.exe` 实际启动。用户明确确认手工 Fullscreen 验收“通过”，覆盖按钮进入/退出、Esc、视频区域双击，以及 Windowed/Maximized 进入 Fullscreen 后恢复语义。**R6-08 正式 Complete。**

### 2026-08-14 — R6-07 Volume Complete

- 既有 Domain/libmpv 主链已经提供 `SetVolumeCommand` / `SetMutedCommand`、mpv volume/mute request、property event 与 Reducer→`PlaybackSnapshot.controls` 回写，因此本轮没有改动 libmpv、Reducer、播放命令数据结构、配置或持久化。
- 新增职责独立的 `PlayerVolumeViewModel`：Snapshot 仍是 volume/mute 真值；ViewModel 只持有短暂 pending target。连续 volume 编辑期间 stale backend echo 不覆盖最新 UI target，实际回写达到目标 ±0.5% 后释放 pending；CommandBus 立即拒绝可显式回退。Mute 在前一个 toggle 获得 Snapshot acknowledgement 前阻止第二次 toggle，避免未确认 optimistic state 上再次反转。
- `PlaybackComposition` 将 Volume VM 接入与 Transport/Timeline 相同的 StatePublisher、共享 `PlaybackRequestIdGenerator` 与唯一 `PlaybackCommandBus`，分别提交 `SetVolumeCommand` / `SetMutedCommand`；`ApplicationBootstrap → MainWindow → PlayerScreen → VolumeControls` 继续通过显式 root property 注入，没有引入 QML singleton/service locator。
- `VolumeControls.qml` 只 import Theme/Controls 并复用既有 `IconButton + Slider`；Main Standard Figma `4:20` 使用 22px Volume glyph + **100px / 3px** slider。重新核对 Figma 后确认 `4:63` 的正式语义是 **Fullscreen OSC（828×106）**，不是窄窗口 Compact；该 variant 只保留 Volume icon、不显示 Slider，真正的 Fullscreen mode 切换属于 R6-08，不在 R6-07 通过普通 Main Window resize 提前触发。
- 最终 Figma 目前只有 canonical `volume` glyph，没有独立 Mute glyph；本任务不手绘/伪造 `mute.svg`，静音状态通过同一 glyph + Tooltip/Accessible description 表达。该资产缺口按当前设计基线接受；后续若设计侧新增 Mute glyph，应更新统一 Icon source，而不是在 Volume Feature 内自造资产。
- R6-12 明确拥有 `HudMessageQueue + PlayerHudOverlay` 以及连续 volume/seek 消息合并策略，所以本任务没有提前创建第二套 HUD Timer/queue；R6-07 只建立后续 HUD consumer 所需的 Volume 状态/intent 主链。
- 新增独立 `player_volume_view_model`、`player_volume_controls` 两个 CTest target，并扩展 `application_container` 的直接 composition 链接/生命周期边界；全量测试数 **62 → 64**。
- 用户锁定 Windows Qt 6.8.3 / MSVC / libmpv 环境最终验证：`configure.ps1` **PASS**（Configuring 4.6 s / Generating 2.1 s）；Debug `build.ps1` **PASS**；`scripts/test.ps1` 完成 QML lint；全量 **64/64 CTest PASS，0 failed，53.71 s**，其中 `player_volume_controls` **0.11 s PASS**、`player_volume_view_model` **0.11 s PASS**，Transport/Timeline/Render/Playback 回归全部保持通过。
- `Player.exe` 实际启动 **PASS**。用户提供的两张实机截图覆盖 960px 最小 Main Window 与宽窗口：Main Standard OSC 在两档宽度下 Timeline/Transport/Volume 均无重叠或错位，100px Volume Slider 正常可见；Slider 获得焦点后的紫色外框来自 R5 已冻结的 focus-visible contract。Main Window resize 不应以“隐藏 Slider”作为 R6-07 验收条件；Fullscreen no-slider variant 的运行时切换留给 R6-08。
- 当前产品媒体打开入口仍属于 R7，因此无法在产品 UI 中加载真实媒体后手工验证 backend volume/mute acknowledgement；该未执行项由现有真实 playback backend 能力链与本轮 VM/QML 定向测试覆盖，没有伪装成已手工通过。**R6-07 正式 Complete；下一项 R6-08。**

### 2026-08-14 — R6-06 Timeline Complete

- 新增职责独立的 `TimelineScrubSession` 与 `PlayerTimelineViewModel`。Scrub session 只拥有 `Idle / Scrubbing / PendingCommit` 交互状态、MediaGeneration 与 normalized preview；真实 position/duration/seekable/seeking 继续来自 `PlaybackSnapshot.timeline`，没有建立第二套 Playback 真值。
- Timeline VM 增加 `canSeek/isScrubbing/seekPending/backendSeeking/displayedNormalized/durationSeconds/positionText/durationText` 投影；拖动时 preview 屏蔽后台旧 position，commit 只发一次 absolute target。commit 后冻结 absolute target seconds，即使同媒体 duration 刷新也不会重算成错误目标；现有 mpv mapper/encoder 的 Absolute Seek 已确认走 `absolute+exact`，PendingCommit 只在实际 position 到达冻结 target ±0.75s 后释放，避免 `seeking:true→false` 事件先到造成 thumb 回弹。
- `PlaybackComposition` 继续复用 R6-05 的同一个 `PlaybackSessionThread`、共享 `PlaybackRequestIdGenerator` 与 `PlaybackCommandBus`，新增 `SeekCommand{absoluteSeconds, SeekMode::Absolute}` 提交；CommandBus 立即拒绝时显式释放 pending preview。具体 SeekCommand 依赖保持在 composition `.cpp`，没有扩散到公共头。
- `ApplicationBootstrap → MainWindow → PlayerScreen → TimelineControls` 通过 initial properties 显式传递 Timeline VM；新增 `features/player/timeline/TimelineControls.qml` 只 import Theme/Controls，复用既有通用 `Slider` pointer/keyboard 状态机，不复制 `MouseArea` 或直接接触 PlaybackSession/libmpv。
- Timeline 几何重新核对最终 Figma Standard `4:20` 与 Compact `4:63`：继续复用 12/11px Geist Mono timecode、3px track、10px rest thumb、16px hit target 与 26/28px OSC inset。wrapper 的半像素纵向补偿由既有 1px thumb-border token 的一半推导；R6-04 timeline slot 改为 `clip:false` 允许 canonical thumb 略超出 28/26px lane，外层 OSC Surface 仍是裁切 owner，其他 Feature slot 不变。
- Domain `playback_selectors` 新增 `canSeek()`；新增 `timeline_scrub_session`、`player_timeline_view_model`、`player_timeline_controls` 三个 CTest target，并扩展 selector 与 R6-04 slot contract，测试总数 **59 → 62**。覆盖 drag/cancel/one-commit、stale position、pending target、duration refresh、generation change、non-seekable、unknown duration、seeking event ordering 与立即提交失败。
- 首轮 Windows 锁定环境验证：`configure.ps1` **PASS**（CMake Configuring 4.5 s / Generating 2.0 s）；Debug `build.ps1` 在 `application_container_tests.exe` 链接阶段 **FAILED**，根因是测试 target 遗漏 `player_presentation_timeline`。同源修复 commit `23f593ed8f26e42dd85bc75aa51e04b4ae6de6b8` 只补测试链接，生产 `player_app` 原本已正确链接 Timeline。
- 第二轮 Windows 复验：`configure.ps1` **PASS**（Configuring 4.0 s / Generating 2.0 s）、Debug `build.ps1` **PASS**、QML lint 阶段完成，全量 **60/62 PASS，57.54 s**。两项失败均为 Presentation contract：两个冗余裸 `z: 2` 触发 R5-03 effect-token 门禁，以及旧测试错误要求 Timeline wrapper 自己读取 generic Slider 的 track-height token。修复删除冗余 z，并把 3px track 断言移回 `Slider.qml` 真正 owner；Seek/VM/CommandBus 和 Figma geometry 均未改变。
- 用户最终 Windows 复验确认：修复后 **62/62 CTest PASS**，`Player.exe` **正常启动**。产品媒体打开入口仍属于 R7，所以 live-media 产品手工 scrub 尚不可执行；没有把该未执行项写成通过。按 R6 快速框架策略，R6-06 自动化主链、状态机与 startup 已满足本 Atomic Task 收口条件。**R6-06 正式 Complete；下一项 R6-07。**

### 2026-08-14 — R6-05 Transport Complete

- 建立 `PlaybackSnapshot → playback_selectors → PlayerTransportViewModel → QML intent → PlaybackComposition → PlaybackCommandBus → PlaybackSession` 真实 Transport 主链；ViewModel 不拥有播放真值，QML 不直接访问 PlaybackSession/libmpv。`PlaybackComposition` 同时引入共享 `PlaybackRequestIdGenerator`，为后续 Timeline/Volume 共用 RequestId 所有权。
- 常驻 OSC Transport 按用户确认的最终定义保持 **Previous / Primary Play-Pause Toggle / Next**；Previous/Next 在 R7 Playlist 提供真实 navigation state/command 前保持禁用。Stop 已完整存在于 selector/VM/CommandBus/PlaybackSession 能力链，但不作为常驻 OSC 按钮。
- 为关闭设计资产缺口，最终 Figma 新增 `D8 / Canonical Transport Assets` Section `745:3` 和唯一 `Icon / Transport / Stop` Component `746:2`（内部 glyph `746:3`）：22×22 canvas、中心 8×8 R1 filled glyph，绑定既有 `icon/primary`；主 OSC `4:20` 的 Previous `4:26` / Play Button `4:28` / Next `4:31` geometry 未改。当前产品没有 Stop 可见 consumer，因此仓库没有新增无使用方的 `stop.svg`。
- 首轮 Windows `configure.ps1` **PASS**（CMake Configuring 4.4 s / Generating 1.9 s），随后 Debug build 因 `playback_composition.h` 错误前置声明 `TransportAction` 与 canonical `TransportAction : quint8` 冲突而 **FAILED**；`test.ps1` 因 build 未生成 development marker 被正确阻断，没有伪报测试通过。
- 修复 commit `2509e25f01481e31efd94a9e1bb6cd7dd65d4e8c` 改为直接引用 canonical `transport_command.h` 并删除重复枚举前置声明，不改变 TransportAction、PlaybackCommand、CommandBus 或公共播放语义。
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
- 用户 Windows 锁定环境实测：`configure.ps1` **PASS**（CMake Configuring 4.5 s / Generating 1.8 s）；Debug `build.ps1` **PASS**，新增 `player_video_viewport_tests.exe` 正常编译链接并完成 Qt runtime deployment；`scripts/test.ps1` 的 QML lint 门禁完成，全量 **53/53 CTest PASS，0 failed，53.38 s**，新增 `player_video_viewport` PASS，既有 R2–R6-01 全部回归保持 PASS。构建日志中的 `/showIncludes` 中文乱码是已知控制台编码显示问题；`WrapVulkanHeaders` 未找到在当前固定 OpenGL backend 下未形成阻断。
- 本地验证前工作区已有受保护内容：`.gitignore` 修改，以及 `r4-04-qml-diagnostics/`、R4-09 1080p/4K JSON 文件；本任务 pull/build/test 均未覆盖、删除或清理这些文件。**R6-02 正式 Complete；R6-03 未开始。**

### 2026-08-14 — R6-01 PlayerScreen composition Complete

- 从已正式关闭的 R5 HEAD `470ea1a58739be59dc4f24ff910a90dfb8c6c506` 创建独立 `agent/r6-stage`，R6-01 只推进播放器页面组合骨架，不提前进入 R6-02 的媒体状态/aspect fit 或 R6-03/R6-04 的真实 Header/OSC 内容。
- 按 R6-01 与第三版 D2 Window System 契约，将 `PlayerScreen` 从旧的 `VideoSurface + PlayerChrome` 两层占位结构拆为职责独立的 `VideoViewport`、`PlayerTopRegion`、`PlayerBottomRegion`、`PlayerOverlayStack`、`PlayerDrawerHost`。`PlayerScreen.qml` 现在只负责 Host 组合、安全边距、尺寸约束和层级，不包含按钮、文字、播放业务或 libmpv 调用。
- `VideoViewport.qml` 继续包裹现有 `VideoSurface`，因此 R4 已验证的 `MpvVideoItem` Render 主链没有被复制或改写；本任务不把 `MpvVideoItem` 直接移入 Screen，也不改变 Renderer/Render context 生命周期。
- Top/Bottom/Overlay/Drawer 四个 Host 都只提供 replaceable content slot，并直接消费 R5 已冻结的 `LayoutTokens` / `SpacingTokens` / `ZOrderTokens`。DrawerHost 作为 z50 Inspector overlay 覆盖在视频之上，不通过改变 VideoViewport 宽度挤压视频，符合最终 Figma Player+Inspector 组合规则。
- 已删除被新结构完整替代的 `features/player/chrome/PlayerChrome.qml`。该旧文件同时拥有顶部和底部占位 UI，继续保留会形成重复路径；删除不改变任何已实现业务操作，因为其中只有框架提示文字和玻璃占位矩形。
- 目标结构中的 `states/` 本轮没有创建空文件：R6-01 尚无独立状态 owner，Loading/Buffering/Ended/Error selector 属于后续 R6-11；不为了目录形式制造透明转发或推测状态抽象。当前仓库也尚无 `PlayerViewModel`，因此 R6-01 只建立 `PlayerViewModel → PlayerScreen → child features` 链中的 Screen/Host 边界，不把尚未存在的 VM 伪装为已接通。
- 新增独立 `player_screen_structure` CTest，静态验证 PlayerScreen 只组合五类 Host、Host 使用正确 z-order/slot、VideoViewport 只包装 VideoSurface、Screen/Host 不出现 PlaybackSession/libmpv/mpv_ 业务词，并强制旧 `PlayerChrome.qml` 不再存在。全量测试数由 **51 → 52**。
- 首轮 Windows 验证锁定代码 HEAD `630bc2249b5024408590b7b9fd803daf82bfb5c5`：`configure.ps1` 在真正调用 CMake 前被旧 scaffold 校验阻断，原因是旧 `verify-project-layout.ps1` 仍把已删除的 `PlayerChrome.qml` 当作必需文件；随后 `build.ps1` 的 CMake 自动重跑正常，Debug build PASS，`scripts/test.ps1` 的 QML lint 门禁完成且无新增 warning/error，全量 **52/52 CTest PASS，0 failed，52.79 s**，新增 `player_screen_structure` PASS；`Player.exe` 实际启动 PASS。
- 同源修复更新 `scripts/verify-project-layout.ps1`：required-files 真值改为 R6-01 五个 Screen Host + `VideoSurface`，旧 `PlayerChrome.qml` 改为 obsolete path；Presentation CMake 校验同步要求五个 Host 已进入 QML module；PlayerScreen composition 校验同步改为新五层结构。没有恢复旧 Chrome，也没有弱化校验。
- 修复后 `configure.ps1` 在锁定 Windows 环境复验 **PASS**：layout verifier 明确识别 R6-01 PlayerScreen composition 完整，CMake **Configuring 3.7 s / Generating 1.7 s**；此前 build/QML lint/52-test 结果保持有效，因为修复只涉及 verifier 与文档。
- 最终手工 basic resize 验证 **PASS**：窗口缩小、放大均正常，内容始终铺满，没有错位或运行时报错；`Player.exe` 再次启动正常。未修改 PlaybackSession、libmpv、Renderer、Render 生命周期、公共播放接口、配置、数据结构或持久化。**R6-01 正式 Complete；R6-02 未开始。**
