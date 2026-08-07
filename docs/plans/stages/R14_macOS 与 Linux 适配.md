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

# Stage R14：macOS 与 Linux 适配

## 1. 本阶段最终要得到什么

在 Windows 正式版之后，用已经建立好的 ports 和通用播放核心扩展 macOS/Linux。目标不是“到处加 ifdef”，而是尽量保持同一个 Playback/Media/UI 主体，只替换平台实现和打包。

## 2. 本阶段怎么快速推进

macOS 和 Linux 分开推进。每个平台先做到“能 build → 能显示视频 → 能打开文件 → 能全屏/控制 → 能打包”，再修平台特有视觉和系统集成。

## 3. 目标模块结构

```text
src/platform/
├─ ports/
├─ windows/
├─ macos/
└─ linux/
packaging/
├─ windows/
├─ macos/
└─ linux/
```

## 4. 阶段主链

```text
Shared Playback / Media / Settings / QML
  ↓
Platform Ports
  ├─ Windows implementation
  ├─ macOS implementation
  └─ Linux implementation
  ↓
Platform packaging
```

## 5. 关键状态/资源归属

- 播放状态继续由 PlaybackSession/Snapshot 拥有。
- 平台对象只拥有窗口、媒体键、休眠、单实例/外部打开等系统资源。
- 通用 QML 不复制整页平台版本。

## 6. Atomic Tasks

### R14-M01 macOS 工具链与依赖

**目的**  
固定 macOS/Xcode/Qt/libmpv 构建矩阵。

**主要模块 / 文件**  
CMake presets + dependency manifest

**主要链路**  
`macOS toolchain → build → runtime probe`

**实施重点**
先单架构跑通，再决定 universal。

**基本验证**
真实 macOS build。

**完成判断**  
不影响 Windows 基线。

### R14-M02 AppKit 窗口与 Retina

**目的**  
实现 macOS NativeWindowPort。

**主要模块 / 文件**  
platform/macos/window/

**主要链路**  
`QML/window VM → port → AppKit/Qt`

**实施重点**
fullscreen、traffic lights、Retina 分职责实现。

**基本验证**
resize/fullscreen/multi-DPI。

**完成判断**  
窗口正常。

### R14-M03 macOS 媒体键/休眠/文件打开

**目的**  
补齐平台能力。

**主要模块 / 文件**  
platform/macos/media_keys|power|open_events

**主要链路**  
`OS → ports/ActionId/MediaOpenCoordinator`

**实施重点**
复用 R10 通用 policy。

**基本验证**
前后台/关闭。

**完成判断**  
不复制业务状态。

### R14-M04 macOS 打包/签名/公证

**目的**  
生成可分发 app bundle。

**主要模块 / 文件**  
packaging/macos/

**主要链路**  
`bundle → deploy → codesign → notarize → smoke`

**实施重点**
真实机器验证。

**基本验证**
Gatekeeper/播放。

**完成判断**  
macOS 可安装使用。

### R14-L01 Linux 工具链与 X11/Wayland

**目的**  
固定目标发行环境和 X11/Wayland 矩阵。

**主要模块 / 文件**  
presets + platform/linux/window

**主要链路**  
`Linux build → window/render smoke`

**实施重点**
先选主发行基线，再测 X11/Wayland。

**基本验证**
两种 session 启动/播放。

**完成判断**  
通用层无 XID/wl_surface。

### R14-L02 MPRIS/休眠

**目的**  
实现 Linux media control 和 SleepInhibitor。

**主要模块 / 文件**  
platform/linux/mpris|power

**主要链路**  
`MPRIS/system inhibit → ports → ActionId/policy`

**实施重点**
选一个主实现，必要时 fallback。

**基本验证**
后台控制/关闭释放。

**完成判断**  
Linux 桌面集成可用。

### R14-L03 desktop/MIME/文件入口

**目的**  
接 .desktop、MIME 和外部打开。

**主要模块 / 文件**  
platform/linux/file_association

**主要链路**  
`desktop shell → argv/IPC → MediaOpenCoordinator`

**实施重点**
入口不直达 mpv。

**基本验证**
双击/多个文件。

**完成判断**  
文件打开统一。

### R14-L04 Linux 打包策略

**目的**  
在 AppImage/Flatpak/发行包中选主路线并做 clean smoke。

**主要模块 / 文件**  
packaging/linux/

**主要链路**  
`chosen package → clean env → launch/play`

**实施重点**
不要同时维护三套半成品。

**基本验证**
目标发行环境。

**完成判断**  
Linux 包可用。

## 7. 框架打通后优先修复

- AppKit/X11/Wayland API 泄漏进通用模块。
- Retina/DPI 与 Render 尺寸。
- sandbox/portal 路径。
- 打包和系统运行库。

## 8. 本阶段最小可运行里程碑

macOS 和 Linux 各自至少有一个目标环境可以构建、启动、播放、全屏、关闭，并具备对应安装/包形式。

## 9. 阶段关闭条件

每个平台有真实验证记录；通用 playback/domain/persistence 无平台 API；Render 主链实际跑过。未支持的桌面/发行版明确列为范围外。

## 10. 本阶段暂不要求

不要求第一轮同时支持所有 macOS 版本、所有 Linux 发行版和所有打包格式。

## 11. 后续扩展位置

未来如切换 Metal/Vulkan/RHI，必须单独做 Renderer 迁移阶段；PipeWire/portal 等继续做独立 Linux capability。
