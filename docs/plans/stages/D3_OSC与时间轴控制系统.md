# Qt6 + libmpv 播放器第三版 UI 阶段设计任务书

> 版本定位：第三版 Airy Glass 独立 UI/UX 设计任务书  
> 设计基准：已确认的浅雾、柔紫蓝、柔焦、半透明玻璃、强留白、轻悬浮视觉方向  
> 设计对象：Qt6 + libmpv 桌面播放器  
> 文档性质：设计执行基线，不替代源码开发 R2–R14 任务书  
> 组织原则：按窗口、控制系统、Inspector、状态、Preferences、设计系统、原型与交付组织，不把播放器功能误写成网页“页面”  
> 设计阶段编号：D0–D9

## 统一执行策略

本任务书学习现有 `docs/plans` 与 `docs/plans/stages` 的写法：阶段目标明确、职责边界明确、Atomic Task 可独立验收、先搭正确骨架再逐步收口；但**内容完全独立于旧 R2–R14 开发任务书**。

整个第三版 UI 设计按以下节奏推进：

1. **视觉母语优先**：先锁定已经确认的 Airy Glass 基准，不再在执行中回到深色黑蓝播放器、传统后台面板或旧 IINA 复刻。
2. **产品结构优先**：先明确 Player Window、Video Viewport、Floating Header、OSC、Inspector、Overlay、Preferences、Mini/Fullscreen 的空间关系，再设计局部控件。
3. **真实场景优先**：所有组件都必须来自真实播放器场景；不为了“组件库看起来完整”提前创建没有产品落点的组件。
4. **源组件优先**：一旦发现跨场景重复的视觉或交互，必须回到 Token / Primitive / Component / Surface 源处理，不在多个成品画面分别打补丁。
5. **全局回刷**：圆角、玻璃材质、间距、图标、时间轴、按钮尺寸、显隐逻辑发生基准变化时，必须全局同步，不允许只改一个窗口模式。
6. **最后文档化**：Handoff、变量统计、组件清单、交互说明必须从最终设计反推，不允许先写过期规格再强迫产品稿适配。

### 默认硬阻断

出现以下任一情况时，不得继续铺开后续设计：

- 主播放器框架仍与前两版视觉骨架高度相似；
- 同一控件在主窗口、Fullscreen、Mini 中出现三套独立实现；
- Playlist / Tracks / Subtitles / Chapters 形成四套不同的侧板语言；
- 窗口、OSC、Inspector、Preferences 之间明显不像同一个产品；
- 关键控件存在肉眼可见的未对齐、非光学居中、行高漂移、间距不一致；
- Figma 组件源和成品实例出现无法解释的硬编码颜色、圆角、阴影、尺寸分叉；
- 状态展示依赖“大色块、大圆按钮、大红框”掩盖层级问题；
- 为了追求玻璃效果牺牲可读性、焦点可见性或复杂视频背景下的辨识度。

## 设计可评审里程碑

| 里程碑 | 达到的实际形态 |
|---|---|
| D0 | 视觉基准、产品结构、禁用旧视觉规则、设计文件组织锁定 |
| D1 | Color / Type / Spacing / Radius / Glass / Motion 等设计基础可支撑产品 |
| D2 | 主播放器窗口框架定稿：视频、Floating Header、OSC 空间关系成立 |
| D3 | OSC、Timeline、Volume、Transport、显隐生命周期完整 |
| D4 | 单一 Inspector Shell 承载 Playlist / Tracks / Subtitles / Chapters |
| D5 | Empty / Loading / Playing / Paused / Buffering / Seeking / Ended / Error 与 HUD/Toast/Dialog 完整 |
| D6 | Preferences / Shortcuts 与独立设置窗口结构完成 |
| D7 | Mini Player / Fullscreen / 窄窗口等窗口模式完成 |
| D8 | Components / Surfaces / Variables 完整收口，所有成品回归源组件 |
| D9 | Prototype / Handoff / 全局 QA 完成，可交给 Qt Quick/QML 实现 |

## 模块化总原则

- Figma 的 Page 只是设计文件分类容器，不代表产品“页面”。
- 产品结构必须按 **Window / Region / Overlay / Inspector / State / Component** 理解。
- 主播放器窗口只拥有布局组合，不拥有各 Feature 的内部组件实现。
- OSC 只负责播放器控制系统，不承载 Playlist/Tracks/Settings 的业务内容。
- Inspector Shell 只负责容器、标题区、尺寸、材质、开合和切换；Playlist/Tracks/Subtitles/Chapters 各自只负责内容模块。
- HUD / Toast / Dialog / Error Overlay 是不同反馈层级，禁止混成一个万能浮层。
- Mini Player / Fullscreen 是同一播放器的不同窗口模式，不复制第二套播放状态。
- Token 是视觉真值；Component 是交互/几何真值；成品画面只消费它们。
- 任何一个 Figma Component 一旦需要承担第二项可独立演进职责，应升级为 Component Set 或拆成子组件，不继续在单组件内部堆 Variant 例外。

# Stage D3：OSC 与时间轴控制系统

## 1. 本阶段最终要得到什么

建立第三版最核心的播放器控制语言：悬浮玻璃 OSC、Transport、Timeline、Volume、Action Cluster、显隐生命周期。D3 完成后，播放器“怎么被控制”应当已经稳定。

## 2. 本阶段怎么快速推进

先做 OSC Shell + Timeline，再接 Transport/Volume/Actions，最后做 Rest/Active/Hidden 和 Hover/Pressed/Focus。禁止先给每个按钮做炫酷 Variant，再回来拼整体。

## 3. 目标模块结构

```text
OSC
├─ Control Surface
├─ Timeline
│  ├─ Track
│  ├─ Buffered Range
│  ├─ Progress
│  ├─ Thumb
│  ├─ Chapter Markers
│  └─ Hover / Seek Preview
├─ Transport Cluster
├─ Volume Cluster
├─ Utility Actions
└─ Visibility State
```

## 4. 阶段主链

```text
Playback / Interaction State
  ↓
OSC Visibility
  ↓
Timeline + Controls
  ↓
Hover / Press / Scrub / Popup
```

## 5. 关键状态 / 资源归属

- 播放真值：开发侧 PlaybackSnapshot，设计稿只展示投影。
- Scrub preview：Timeline interaction state。
- Hover/Pressed/Focus：控件自身。
- OSC Visible/Rest/Active/Hidden：OSC visibility state。
- Popup 打开时 OSC 是否保持：visibility policy。
- 成品 Main/Fullscreen/Mini 只消费同一套 Control Components。

## 6. Atomic Tasks
### D3-01 OSC Surface 与内部网格

**目的**  
把已确认的悬浮玻璃控制带升级为稳定布局系统。

**主要模块 / 设计对象**  
OSC / Control Surface

**主要链路**  
`host width → osc layout grid → controls`

**实施重点**  
不铺满整窗；玻璃边界柔和；上下两层或单层结构必须有明确间距规则。

**基本验证**  
标准/窄/宽窗口。

**完成判断**  
OSC 像漂浮控件，不像底栏。
### D3-02 Timeline 基础几何

**目的**  
建立极细轨道、progress、buffer、thumb、hit target。

**主要模块 / 设计对象**  
Timeline component

**主要链路**  
`position/duration → visual progress`

**实施重点**  
视觉轨道与交互命中区分离；不要为了易点击把轨道画粗。

**基本验证**  
0/50/100%、unknown duration、non-seekable。

**完成判断**  
几何在不同宽度下保持精致。
### D3-03 Timeline 交互状态

**目的**  
覆盖 hover preview、scrub、pending seek、chapter marker。

**主要模块 / 设计对象**  
Timeline interaction variants

**主要链路**  
`pointer → preview/scrub → commit`

**实施重点**  
Preview 和真实 position 必须视觉可区分；chapter marker 不抢进度主线。

**基本验证**  
hover、drag、cancel、commit、chapter seek。

**完成判断**  
Timeline 行为可以被 Prototype 明确表达。
### D3-04 Transport Cluster

**目的**  
设计 Previous / PlayPause / Next 等核心操作。

**主要模块 / 设计对象**  
Transport controls

**主要链路**  
`transport state → icon/button state`

**实施重点**  
播放键允许稍强但不能回到大蓝圆；按钮尺寸、图标光学中心统一。

**基本验证**  
playing/paused/disabled/hover/press/focus。

**完成判断**  
主操作明确但不抢视频视觉。
### D3-05 Volume Cluster

**目的**  
定义 mute、volume slider、popover/hud 入口关系。

**主要模块 / 设计对象**  
Volume controls

**主要链路**  
`volume/mute → control → feedback`

**实施重点**  
窄窗口可折叠成 Popover；主 OSC 不塞过长 slider。

**基本验证**  
0/低/中/高/muted。

**完成判断**  
音量信息在 Main/Fullscreen/Mini 规则一致。
### D3-06 Utility Action Cluster

**目的**  
统一字幕、音轨、章节、播放列表、更多、全屏入口。

**主要模块 / 设计对象**  
Utility action controls

**主要链路**  
`action → inspector/popover/window mode`

**实施重点**  
功能入口图标视觉权重低于 transport；选中/面板打开状态可见但克制。

**基本验证**  
Inspector opening、disabled、focus。

**完成判断**  
同一动作在不同窗口模式使用同一组件语义。
### D3-07 OSC 显隐生命周期

**目的**  
建立 Hidden / Rest / Active / LockedVisible 等状态规则。

**主要模块 / 设计对象**  
OSC visibility prototype

**主要链路**  
`interaction snapshot → visibility state`

**实施重点**  
hover、scrub、popup、paused、error、keyboard action 的可见策略明确；不靠多个 timer owner。

**基本验证**  
播放静置、暂停、拖动、Popup、错误、全屏。

**完成判断**  
显隐规则可交付开发，不只是一个 fade 动画。

## 7. 框架打通后优先修复

- 图标光学中心和按钮几何中心不一致。
- Timeline 在窄窗口挤压时间码。
- Popup 打开后 OSC 误消失。
- Main/Fullscreen/Mini 分别画了不同 Timeline。
- 鼠标 Hover 状态过强导致整条 OSC 闪烁。

## 8. 本阶段最小可评审里程碑

Main Player 上 OSC 可完整展示播放、暂停、Seek、音量和 Inspector 入口，并有 Rest/Active/Hidden 原型。

## 9. 阶段关闭条件

所有核心控制已组件化；Timeline 响应式；显隐规则明确；没有大蓝主按钮和厚重底栏回归。

## 10. 本阶段暂不要求

不完成 Playlist 内容、设置、复杂快捷键提示。

## 11. 后续扩展位置

Speed、A-B Loop、截图等高级 Action 后续作为 Utility Action 插入，不修改 OSC 主结构。
