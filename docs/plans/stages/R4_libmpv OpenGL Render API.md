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

# Stage R4：libmpv OpenGL Render API

## 1. 本阶段最终要得到什么

让真实视频通过 libmpv OpenGL Render API 进入 Qt Quick，并把 Render 生命周期与 Playback 生命周期接正确。R4 完成后，项目第一次成为“看得见视频”的播放器。

## 2. 本阶段怎么快速推进

不要先造完整 Renderer 框架再验证。优先顺序是：OpenGL resolver → RenderContext → update callback → QQuickFramebufferObject → 画面显示。画面出来以后再集中修 DPI、最小化、关闭竞态和性能。

## 3. 目标模块结构

```text
src/playback/infrastructure/mpv/render/
├─ opengl_proc_resolver.*
├─ mpv_render_context.*
├─ mpv_render_update_bridge.*
├─ mpv_video_item.*
└─ mpv_video_renderer.*
src/presentation/qml/features/player/video/
```

## 4. 阶段主链

```text
Playback mpv core
  ↓ controlled render handle
MpvRenderContext
  ↕ update callback
MpvVideoRenderer (Qt Quick Render Thread)
  ↓ FBO
Qt Quick Scene Graph
  ↓
QML overlays
```

## 5. 关键状态/资源归属

- `mpv_render_context`：Render infrastructure 唯一拥有。
- FBO/OpenGL：Qt Quick Render Thread。
- `MpvVideoItem`：GUI Thread，只同步 presentation state。
- `mpv_handle` 仍属于 Playback Thread/R3。

## 6. Atomic Tasks

### R4-01 OpenGL proc resolver

**目的**  
先确认 Qt 当前 OpenGL context 可以满足 libmpv Render API。

**主要模块 / 文件**  
`render/opengl_proc_resolver.*`

**主要链路**  
`Qt current GL → getProcAddress → libmpv`

**实施重点**
用 `QOpenGLContext::getProcAddress`；缺函数明确失败。

**基本验证**
离屏 context 能解析；失败路径可见。

**完成判断**  
Render context 创建前就能判断是否可用。

### R4-02 MpvRenderContext RAII

**目的**  
建立 render context 的唯一所有者和正确 create/free。

**主要模块 / 文件**  
`render/mpv_render_context.*`

**主要链路**  
`mpv core + current GL → create → render → free`

**实施重点**
先做最小 create/free，再接 update/render；明确 context 所属线程。

**基本验证**
create/free 循环；错误 API 类型。

**完成判断**  
render context 生命周期可独立验证。

**高风险约束**
所有 `mpv_render_*` 必须串行；free 必须发生在 mpv core destroy 前，并在合适 GL context current 时执行。

### R4-03 Render update bridge

**目的**  
把 libmpv 的 redraw 通知转成 Qt Quick update。

**主要模块 / 文件**  
`render/mpv_render_update_bridge.*`

**主要链路**  
`mpv callback → signal/update request → Qt render thread`

**实施重点**
先实现 signal-only callback 和 alive 标记；重复 update 后续再合并优化。

**基本验证**
连续播放持续触发更新；关闭不访问死对象。

**完成判断**  
回调不会直接渲染或访问 QML。

### R4-04 MpvVideoItem

**目的**  
建立 QML 可放置的视频 Item。

**主要模块 / 文件**  
`render/mpv_video_item.*`、`VideoSurface.qml`

**主要链路**  
`QML Item state → synchronize → Renderer`

**实施重点**
先只同步尺寸/DPR/visible 和 renderer 创建。

**基本验证**
QML 可实例化；resize 时 state 同步。

**完成判断**  
视频区域在 QML 中有稳定边界。

### R4-05 MpvVideoRenderer

**目的**  
真正把视频渲染进 FBO。

**主要模块 / 文件**  
`render/mpv_video_renderer.*`

**主要链路**  
`FBO → mpv_render_context_render → Qt Quick composite`

**实施重点**
先把画面显示出来；再修 Y flip、physical pixel size 和 GL state restore。

**基本验证**
真实视频可见且方向正确。

**完成判断**  
播放器第一次真正显示视频画面。

### R4-06 Resize 和 DPI

**目的**  
修正窗口尺寸、DPR、多屏和全屏时的视频尺寸。

**主要模块 / 文件**  
Item/Renderer size sync

**主要链路**  
`logical size × DPR → physical FBO`

**实施重点**
先覆盖当前开发屏幕，再补 125%/150%/200% DPI。

**基本验证**
resize/fullscreen/DPR smoke。

**完成判断**  
画面不拉伸、不糊、不越界。

### R4-07 最小化/恢复与不可见

**目的**  
避免不可见时 render storm 和恢复黑屏。

**主要模块 / 文件**  
render visibility policy

**主要链路**  
`visibility → suppress/restore update`

**实施重点**
先用简单 visible/minimized 策略；真实问题出现后再优化。

**基本验证**
最小化数十秒再恢复。

**完成判断**  
恢复后能继续出画面。

### R4-08 Render shutdown race

**目的**  
在真正有视频后解决 callback/render/free/core destroy 的关闭竞态。

**主要模块 / 文件**  
`shutdown_coordinator.*` + render lifecycle

**主要链路**  
`invalidate callback → wait render quiescence → free render context → stop Session/core`

**实施重点**
先实现严格顺序；再用播放中关窗、resize 中关窗、最小化关窗找问题。

**基本验证**
压力关闭 + 带 renderer 的循环生命周期。

**完成判断**  
关闭不崩、不黑住、不死锁。

**高风险约束**
render context 必须先于 mpv_handle；活动 render 未退出时不能 free；late callback 必须安全 no-op。

### R4-09 Render 性能基线

**目的**  
先记录 1080p/4K 数据，不急着微优化。

**主要模块 / 文件**  
简单 diagnostics/render probe

**主要链路**  
`playback → frame/drop/timing metrics`

**实施重点**
记录开发机硬件、媒体、窗口/全屏数据。

**基本验证**
1080p/4K 各一组。

**完成判断**  
后续 R12 有可对比基线。

## 7. 框架打通后优先修复

- Y flip/尺寸/DPR。
- GL state restore。
- hidden/minimized update storm。
- callback/render/free 与关窗竞态。
- 4K 主观卡顿先测再优化。

## 8. 本阶段最小可运行里程碑

打开本地视频后能在 QML 视频区域连续播放，控制层能叠在视频上，resize/fullscreen 基本正常。

## 9. 阶段关闭条件

视频主链稳定；无 `wid` fallback；关闭顺序正确；常见 resize/DPI 不明显错误。性能极限和各种 GPU 组合留 R12。

## 10. 本阶段暂不要求

不在 Renderer 放播放按钮、Timeline、Playlist、历史和业务状态。

## 11. 后续扩展位置

未来若迁移 RHI/Metal/Vulkan，必须单独 ADR/阶段，不在 R4 悄悄加第二渲染架构。
