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

# Stage R10：快捷键与 Windows 平台能力

## 1. 本阶段最终要得到什么

把播放器从“Qt 窗口程序”提升成真正融入 Windows 的桌面软件：统一动作、快捷键、媒体键、原生窗口、休眠、单实例和文件关联。

## 2. 本阶段怎么快速推进

先搭 Action Registry 和 shortcuts；再做窗口/媒体键/休眠；单实例和文件关联最后接 MediaOpenCoordinator。平台细节出现真实问题时在 `platform/windows` 内修，不污染播放核心。

## 3. 目标模块结构

```text
src/shortcuts/
├─ domain/
├─ application/
└─ presentation/
src/platform/
├─ ports/
└─ windows/
   ├─ window/
   ├─ media_keys/
   ├─ power/
   ├─ single_instance/
   └─ file_association/
```

## 4. 阶段主链

```text
UI / Shortcut / Media Key
  ↓
ActionId / Action Registry
  ↓
Application actions / PlaybackCommand

Windows native events
  ↓
Platform ports
  ↓
Window / MediaOpen / Sleep policies
```

## 5. 关键状态/资源归属

- 动作身份：Action Registry。
- 播放真值：仍是 PlaybackSession。
- 平台句柄/IPC/Win32 生命周期：`platform/windows`。
- 文件打开：仍统一 MediaOpenCoordinator。

## 6. Atomic Tasks

### R10-01 Action Registry

**目的**  
统一 UI/快捷键/媒体键的 ActionId。

**主要模块 / 文件**  
`shortcuts/domain/action_id.h`、`action_registry.*`

**主要链路**  
`UI/key/media key → ActionId → handler`

**实施重点**
先注册已有主操作；不要求一次覆盖未来全部动作。

**基本验证**
play/pause/seek/fullscreen actions。

**完成判断**  
相同行为只有一个 action identity。

### R10-02 Shortcut Dispatcher

**目的**  
按焦点上下文分发快捷键。

**主要模块 / 文件**  
shortcut_dispatcher/context

**主要链路**  
`key event + context → ActionId`

**实施重点**
先播放器全局 + 文本输入保护；复杂列表上下文后补。

**基本验证**
输入框/播放器主界面。

**完成判断**  
不会输入文字时误 Seek。

### R10-03 Shortcut Editor

**目的**  
提供冲突检测和恢复默认。

**主要模块 / 文件**  
shortcut_editor/conflict + QML

**主要链路**  
`binding edit → validate → settings → dispatcher`

**实施重点**
先基本编辑/恢复；高级多 profile 后续。

**基本验证**
冲突/非法/恢复。

**完成判断**  
用户可修改快捷键。

### R10-04 无边框窗口与 hit test

**目的**  
让 Windows 窗口拖动、resize、最大化、Snap 正常。

**主要模块 / 文件**  
platform/windows/window/

**主要链路**  
`Win32 native events → WindowIntegration → Qt/QML`

**实施重点**
先主窗口 hit-test；DPI/Snap 问题边跑边修。

**基本验证**
drag/resize/maximize/Snap。

**完成判断**  
Windows 窗口行为像正常桌面软件。

### R10-05 DWM 材质

**目的**  
支持时启用 Windows 材质并安全 fallback。

**主要模块 / 文件**  
windows_dwm_material.*

**主要链路**  
`system capability/theme → DWM attrs`

**实施重点**
先做当前支持的主材质；高对比度/旧系统 fallback。

**基本验证**
Win10/Win11 支持矩阵。

**完成判断**  
不支持时仍正常显示。

### R10-06 媒体键

**目的**  
把系统媒体键接到 Action Registry。

**主要模块 / 文件**  
platform/windows/media_keys/

**主要链路**  
`OS media key → ActionId → handler`

**实施重点**
先 play/pause；next/prev 随 Playlist。

**基本验证**
前台/后台/关闭。

**完成判断**  
后台也能正常控制。

### R10-07 休眠抑制

**目的**  
播放视频时阻止系统自动休眠并正确释放。

**主要模块 / 文件**  
platform/windows/power/

**主要链路**  
`Snapshot playing → policy → SleepInhibitor token`

**实施重点**
先 playing video acquire，pause/stop/end/close release。

**基本验证**
完整状态切换。

**完成判断**  
退出后无残留请求。

### R10-08 单实例

**目的**  
第二实例把文件/URL 交给主实例。

**主要模块 / 文件**  
platform/windows/single_instance/

**主要链路**  
`secondary process → IPC → primary → MediaOpenCoordinator`

**实施重点**
先可靠传一个/多个参数；主实例未 ready 时简单 queue。

**基本验证**
并发启动、多文件、关闭。

**完成判断**  
不会出现两个播放器抢同一应用状态。

**高风险约束**
IPC payload 必须校验；关闭时停止监听；外部参数仍必须进入 MediaOpenCoordinator，不能直达 mpv。

### R10-09 文件关联

**目的**  
让安装后的媒体文件可以双击打开。

**主要模块 / 文件**  
platform/windows/file_association + packaging hook

**主要链路**  
`Windows shell → argv/IPC → MediaOpenCoordinator`

**实施重点**
先定义批准扩展名；安装器阶段再完成最终注册/卸载。

**基本验证**
注册/打开/清理。

**完成判断**  
文件双击进入统一播放链。

## 7. 框架打通后优先修复

- text input 快捷键冲突。
- frameless hit-test/DPI/Snap。
- media key 重复回调。
- sleep token 未释放。
- single-instance 启动竞态。

## 8. 本阶段最小可运行里程碑

应用在 Windows 上具备正常无边框窗口行为、快捷键/媒体键、播放时休眠抑制、单实例和文件双击打开。

## 9. 阶段关闭条件

Windows 日常使用链基本完整；平台资源关闭正确；通用 playback/domain 不出现 Win32 API。最终安装/卸载注册在 R13 再做硬验收。

## 10. 本阶段暂不要求

macOS/Linux 实现不在 R10；自动更新不在 ApplicationBootstrap 里提前做。

## 11. 后续扩展位置

R14 使用相同 platform ports 做 macOS/Linux；以后新增系统通知/Now Playing 也继续放平台 capability。
