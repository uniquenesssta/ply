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

# Stage R2：无 UI libmpv 播放核心

## 1. 本阶段最终要得到什么

先得到一个真正能工作的 headless 播放核心。R2 的价值不是“把 libmpv 包装得完美”，而是尽快让固定版本 libmpv 在项目里可靠创建、初始化、接收异步命令、产生事件和属性，并由控制台 probe 证明主链。

## 2. 本阶段怎么快速推进

第一轮先把 `FindLibMpv → MpvHandle → initialize → event loop → async command → property/event decoder → playback_probe` 全部搭起来。只要主链能跑并且没有明显生命周期错误，就继续推进；再回头补错误映射、负向路径、事件量和关闭细节。

## 3. 目标模块结构

```text
src/playback/infrastructure/mpv/
├─ client/
├─ initialization/
├─ commands/
├─ properties/
├─ events/
└─ errors/
tools/playback_probe/
cmake/FindLibMpv.cmake
```

## 4. 阶段主链

```text
libmpv DLL
  ↓
MpvHandle / Initializer
  ↓
Async Commands ↔ Event Loop
  ↓
Property / Event Decoder
  ↓
playback_probe
```

## 5. 关键状态/资源归属

- `mpv_handle`：`MpvHandle` 唯一拥有。
- 普通 libmpv client API：Playback Thread。
- raw mpv event/node：只留在 `infrastructure/mpv`。
- 本阶段不建立 PlaybackSnapshot 真值。

## 6. Atomic Tasks

### R2-01 FindLibMpv 与运行时探测

**目的**  
把固定版本 libmpv 变成项目唯一可链接目标，并能知道实际加载的是哪个 DLL/版本。

**主要模块 / 文件**  
`cmake/FindLibMpv.cmake` 或等价模块；`mpv_runtime_probe.*`

**主要链路**  
`CMake 固定依赖路径 → LibMpv::LibMpv → runtime probe`

**实施重点**
- 只使用 R0-04 固定 sibling 路径，不扫描随机系统 mpv。
- imported target 统一 include/link/runtime 信息。
- 启动时记录版本、DLL 路径、manifest 身份。

**基本验证**
configure 成功；DLL 缺失/版本不符时有明确错误。

**完成判断**  
开发目录和后续 staging 都能解析同一个已审计 libmpv。

### R2-02 MpvHandle RAII

**目的**  
让 `mpv_handle` 只有一个所有者，创建和销毁路径先稳定。

**主要模块 / 文件**  
`src/playback/infrastructure/mpv/client/mpv_handle.*`

**主要链路**  
`create → initialize → use → close/destroy`

**实施重点**
- 不可复制。
- raw `mpv_handle*` 不离开 mpv infrastructure。
- close 幂等，初始化失败也能安全清理。

**基本验证**
创建失败、初始化失败、重复 close、循环 create/destroy。

**完成判断**  
不存在双重析构和明显泄漏。

### R2-03 初始化 profile

**目的**  
让产品默认行为不受用户系统 `mpv.conf` 干扰。

**主要模块 / 文件**  
`initialization/mpv_initializer.*`、`mpv_option_profile.*`

**主要链路**  
`product profile → set options → mpv_initialize`

**实施重点**
- 先采用独立/no-config 路线。
- option 按职责分组，不把字符串散到上层。
- 只先放 MVP 真正需要的默认值。

**基本验证**
有/无用户 mpv.conf 下主行为一致；非法 option 能报错。

**完成判断**  
初始化结果稳定且可诊断。

### R2-04 Wakeup bridge 和事件循环

**目的**  
先把 mpv 事件真正送进 Qt 的 Playback Thread，形成可持续运行的事件循环。

**主要模块 / 文件**  
`events/mpv_wakeup_bridge.*`、`mpv_event_loop.*`

**主要链路**  
`mpv internal thread → wakeup callback → Playback Thread → mpv_wait_event(0) → decoder`

**实施重点**
- callback 只负责唤醒，不执行业务。
- Playback Thread drain events。
- 先把主路径跑起来，再根据真实事件量调节 drain/合并。

**基本验证**
持续播放时事件持续到达；关闭后不再访问已销毁对象。

**完成判断**  
播放数分钟事件队列不堵死，关闭不崩。

**高风险约束**
wakeup callback 不能调用普通 libmpv API；关闭时先让 callback 失效，再销毁目标对象。

### R2-05 命令 encoder/executor

**目的**  
先支持 load/play/pause/stop/seek/volume/mute/speed 的异步提交。

**主要模块 / 文件**  
`commands/mpv_command_encoder.*`、`mpv_command_executor.*`

**主要链路**  
`typed request → encoder → mpv_command_async → command reply`

**实施重点**
- 主链全部走 async。
- executor 只提交，不保存播放真值。
- request userdata 保留给 R3 tracker。

**基本验证**
每类命令能真实提交并收到 reply。

**完成判断**  
控制台可完成基本播放控制。

### R2-06 Property registry/observer

**目的**  
把 position/duration/pause 等核心属性集中注册和安全解码。

**主要模块 / 文件**  
`properties/mpv_property_registry.*`、`observer.*`

**主要链路**  
`property definitions → observe → PROPERTY_CHANGE → typed value`

**实施重点**
- 先注册 R3/R6 确实需要的核心属性。
- `data==nullptr`/none 明确处理。
- track/chapter node 先保留解码入口。

**基本验证**
核心属性能持续变化；错误格式不崩。

**完成判断**  
没有散落 property 名，后续可统一扩展。

### R2-07 Event decoder

**目的**  
把 raw mpv event 收敛成内部事件。

**主要模块 / 文件**  
`events/mpv_event_decoder.*`、`errors/mpv_error_mapper.*`

**主要链路**  
`raw mpv_event → decoder → internal MpvEvent`

**实施重点**
- 先覆盖 start/file-loaded/end/reply/property/log/shutdown。
- 未知 event 记录，不崩。

**基本验证**
真实短媒体事件序列可观察；未知事件安全。

**完成判断**  
上层无需直接解析 `mpv_event`。

### R2-08 控制台 playback probe

**目的**  
尽快证明无 UI 的真实播放主链已经通。

**主要模块 / 文件**  
`tools/playback_probe/`

**主要链路**  
`CLI media → mpv core → load → pause/play → seek → stop/EOF → close`

**实施重点**
- 只做探针，不复制完整业务状态机。
- 每步打印结果，设超时。
- 没有合法 fixture 时允许使用本地合法样本并明确记录。

**基本验证**
正常短媒体 + 一个错误路径。

**完成判断**  
无需 QML 就能真实播放、控制、结束和关闭。

## 7. 框架打通后优先修复

- DLL 路径与版本漂移。
- 用户 mpv.conf 干扰。
- event callback 与关闭竞态。
- async reply 关联不完整。
- property 空值/格式异常。

## 8. 本阶段最小可运行里程碑

运行 `playback_probe <本地合法媒体>`，能够加载、播放、暂停、Seek、停止/结束并正常退出。

## 9. 阶段关闭条件

主链可真实运行；无明确 double-destroy/UAF/deadlock；基础错误可诊断；项目正常 build。其余非关键边缘问题允许记录到 R2 修复清单或 R12。

## 10. 本阶段暂不要求

Render API、PlaybackSnapshot、QML、播放列表、数据库都不在 R2 提前做。

## 11. 后续扩展位置

R2 的 typed infrastructure 事件/命令直接成为 R3 PlaybackSession 的底座；runtime probe 后续进入诊断包和发布 manifest。
