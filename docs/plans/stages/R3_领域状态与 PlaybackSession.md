# Qt6 + libmpv 播放器阶段实施任务书

> 版本定位：快速框架实施版  
> 范围：R2–R14  
> 原则：先把正确的模块框架和真实主链跑起来，再集中修复、补边界和做强验证。  
> 不包含：R0、R1；现有 R0/R1 状态不在本包中重新定义。

## 统一执行策略

本任务书不要求每个 Atomic Task 都先写成“工程合同”。普通任务只要明确目标、模块、主链、实现重点和基本验证即可直接开工。

整个项目按四个节奏推进：

1. **骨架优先**：先建立真正会被使用的目录、CMake、接口、对象所有权和最小实现，不创建大批空文件。
2. **主链优先**：优先让真实链路工作，例如 `libmpv → PlaybackSession → Render → ViewModel → QML`，先看到结果。
3. **边做边修**：编译错误、接口不顺、职责耦合、明显竞态、真实运行问题出现后立即在影响边界修复；普通边缘问题可记录到阶段修复清单，不要求每个小任务都阻断后续。
4. **集中硬化**：R12 专门负责压力、故障、性能、泄漏、长时间运行和最终架构审查；R13 再做真正发布门禁。

只有以下问题默认视为“必须先停下来解决”的硬阻断：

- 项目无法 configure/build；
- 主链完全不可运行；
- 明确的状态双重拥有、循环依赖或错误模块边界；
- crash、deadlock、use-after-free、线程越界等高风险生命周期问题；
- 数据损坏、不可逆迁移错误；
- Render/mpv 销毁顺序错误；
- 发布阶段安装包无法在干净机器启动。

普通 UI 细节、非主路径边缘场景、尚未覆盖的手工矩阵不需要阻止每个 Atomic Task 继续，但必须在阶段修复清单或 R12/R13 前关闭。

## 软件可用性里程碑

| 里程碑 | 达到的实际形态 |
|---|---|
| R2 | 无 UI 的 libmpv 播放核心可真实加载/播放/Seek/关闭 |
| R3 | 播放状态、线程、命令/事件稳定，形成真正“播放器内核” |
| R4 | 视频真正进入 Qt Quick，开始像一个播放器 |
| R6 | 主窗口、播放/暂停、Timeline、音量、全屏可用，形成基础桌面播放器 |
| R8 | 文件/URL/播放列表/音轨/字幕/章节完整，达到主流播放器核心功能形态 |
| R10 | 历史/设置/快捷键/媒体键/单实例/文件关联齐全，Windows 日常使用基本完整 |
| R11 | 高级能力增强 |
| R12 | 稳定性和性能收敛 |
| R13 | 可发给别人安装使用的 Windows 正式桌面软件 |
| R14 | macOS/Linux 平台适配 |

## 模块化总原则

- 一个模块只拥有一个清晰职责；新能力有独立状态、IO、生命周期、错误或测试边界时再拆文件/目录。
- 不为了“文件多”而拆；也不为了“改得少”把不同职责继续塞进同一个文件。
- QML 不直接调用 libmpv；PlaybackSession 是播放真值；Playlist 只拥有队列；DatabaseWorker 只拥有数据库连接；Renderer 只拥有 Render/OpenGL 资源。
- 所有源码/配置/行为变化继续同步记录根 `README.md`。
- R0-06 已按用户要求跳过，因此依赖合法可分发媒体 fixture 的验证，如果只有本地合法样本，必须写成“本地验证”，不能伪装成完整 fixture 体系已经闭环。

# Stage R3：领域状态与 PlaybackSession

## 1. 本阶段最终要得到什么

把 R2 的 libmpv 基础设施提升成产品自己的播放器内核：有类型化命令/事件、唯一 PlaybackSnapshot、纯 Reducer、独立 PlaybackSession 线程和安全状态发布。

## 2. 本阶段怎么快速推进

先把 `Command/Event → Snapshot → Reducer → PlaybackSession Thread` 骨架搭起来并真实播放；随后再补 request tracker、position 节流和 shutdown 压测。不要在第一轮就追求完整状态机所有边角。

## 3. 目标模块结构

```text
src/playback/
├─ domain/
│  ├─ commands/
│  ├─ events/
│  └─ state/
└─ application/
   ├─ session/
   ├─ command_bus/
   ├─ requests/
   └─ state_publisher/
```

## 4. 阶段主链

```text
QML / future ViewModel
  ↓ PlaybackCommand
PlaybackSession (Playback Thread)
  ↓
R2 libmpv infrastructure
  ↑
PlaybackEvent → Reducer → PlaybackSnapshot
  ↓
StatePublisher → GUI
```

## 5. 关键状态/资源归属

- 播放真值：`PlaybackSession` / `PlaybackSnapshot`。
- 请求关联：`RequestTracker`。
- GUI 只接收只读快照。
- QML 临时交互状态不进入 PlaybackSnapshot。

## 6. Atomic Tasks

### R3-01 Command/Event 类型

**目的**  
建立产品语义的播放命令和事件，让 domain 不认识 mpv 字符串。

**主要模块 / 文件**  
`domain/commands/`、`domain/events/`

**主要链路**  
`UI/workflow → PlaybackCommand；MpvEvent → PlaybackEvent`

**实施重点**
先覆盖 load/transport/seek/volume/speed/lifecycle 和 file-loaded/property/eof/error/reply。

**基本验证**
类型可构造、范围可校验。

**完成判断**  
R3 后续不需要在 domain 里写 mpv property 字符串。

### R3-02 PlaybackSnapshot

**目的**  
建立一个能承载播放器主状态的只读快照。

**主要模块 / 文件**  
`domain/state/playback_snapshot.*`

**主要链路**  
`Reducer → PlaybackSnapshot → publisher/ViewModel`

**实施重点**
先放 R6 真正需要的媒体、生命周期、position/duration、seekable、pause/buffer、volume/mute/speed、error；tracks/chapters 留入口。

**基本验证**
默认/加载/停止状态可构造。

**完成判断**  
UI 以后不需要直接问 mpv。

### R3-03 Reducer

**目的**  
把事件转成快照，形成可快速迭代的纯状态机。

**主要模块 / 文件**  
`domain/state/playback_reducer.*`

**主要链路**  
`Snapshot + Event → Snapshot`

**实施重点**
先实现主路径和媒体切换清理；再补 EOF/error/乱序等。

**基本验证**
load/play/pause/seek/end/error 基本状态转换单测。

**完成判断**  
状态变化可在无 libmpv 环境独立测试。

### R3-04 Invariants

**目的**  
集中放少量真正重要的不变量，防止明显错误状态组合。

**主要模块 / 文件**  
`domain/state/playback_invariants.*`

**主要链路**  
`new snapshot → invariant check`

**实施重点**
先管高价值规则：无媒体时媒体级列表清空、generation 不倒退、互斥生命周期不冲突。

**基本验证**
构造非法组合能被发现。

**完成判断**  
不变量不散落到 QML。

### R3-05 PlaybackSession thread

**目的**  
把命令、mpv 事件、Reducer 和 Snapshot 都放到独立 Playback Thread，形成真正播放器内核。

**主要模块 / 文件**  
`application/session/`、`command_bus/`

**主要链路**  
`GUI/Application → queued command → Playback Thread Session → R2；R2 event → reducer → snapshot`

**实施重点**
- 先让 Session start/load/play/pause/seek/stop 跑起来。
- 再接 request tracker 和 publisher。
- Session 是唯一播放真值。

**基本验证**
GUI thread 不执行 mpv wait/command；真实媒体主链通过。

**完成判断**  
可以在无完整 UI 情况下稳定控制播放。

**高风险约束**
普通 libmpv client API 只在 Playback Thread；不能让 QML/GUI QObject 直接跨线程访问 Session 内部状态。

### R3-06 Request tracker

**目的**  
把异步 reply 与请求/generation 对上。

**主要模块 / 文件**  
`application/requests/`

**主要链路**  
`command → RequestId+generation → reply → match/cancel`

**实施重点**
先解决快速 load A→B 的旧 reply；再补 timeout/unknown id。

**基本验证**
A→B 后 A reply 不改变 B。

**完成判断**  
异步请求不串台。

### R3-07 State publisher

**目的**  
把 Snapshot 安全送给 GUI，position 适度节流。

**主要模块 / 文件**  
`application/state_publisher/`

**主要链路**  
`Session snapshot → immediate/throttled publish → GUI`

**实施重点**
关键状态即时，position 先用简单目标频率；后续再性能调优。

**基本验证**
持续播放 UI 更新稳定；pause/error 立即出现。

**完成判断**  
GUI 不因 position storm 过载。

### R3-08 Session shutdown

**目的**  
把 PlaybackSession 关闭顺序先做正确，再继续 Render。

**主要模块 / 文件**  
`application/session/playback_shutdown.*`

**主要链路**  
`reject new command → cancel pending → disable wakeup → stop loop → destroy handle → stop thread`

**实施重点**
先实现清晰顺序和有界等待；用循环测试找真实问题，再修。

**基本验证**
播放中/加载中关闭；循环 100 次基础 Session 生命周期。

**完成判断**  
无死锁、无残留播放线程。

**高风险约束**
关闭期间禁止新命令；late event/reply 必须 no-op；不能在 GUI 线程无限等待。

## 7. 框架打通后优先修复

- generation 过滤旧媒体事件。
- pause/error/end 等状态优先级。
- position 发布频率。
- Session shutdown 与 late callback/reply。

## 8. 本阶段最小可运行里程碑

headless Session 能持续加载和控制媒体，并持续向 GUI/测试端发布快照；快速切换媒体不明显串状态。

## 9. 阶段关闭条件

主状态只有一个 owner；GUI 不直接调 libmpv；Session 可创建/关闭；核心主路径单测和真实链路可运行。完整压力和更多非法事件组合留到 R12。

## 10. 本阶段暂不要求

Render、UI、数据库不进入 PlaybackSession；Playlist 也不成为播放真值拥有者。

## 11. 后续扩展位置

R4 只消费 Session/mpv core 的受控 Render 边界；R6 只消费 Snapshot/ViewModel。
