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

# Stage D6：Preferences 与快捷键窗口

## 1. 本阶段最终要得到什么

建立独立 Preferences Window，承载 Playback、Video、Audio、Subtitles、Interface、Advanced 与 Shortcuts。它应与播放器共享同一 Airy Glass 视觉语言，但不能把主播放器的 OSC/Inspector 直接复制进设置窗口。

## 2. 本阶段怎么快速推进

先锁定 Window Shell + Source List + Content Region，再做通用 Settings Row / Section / Field / Toggle / Select / Slider / Keycap。最后填真实设置内容。

## 3. 目标模块结构

```text
Preferences Window
├─ Preferences Shell
├─ Source List
├─ Content Header
├─ Settings Sections
│  ├─ Playback
│  ├─ Video
│  ├─ Audio
│  ├─ Subtitles
│  ├─ Interface
│  └─ Advanced
└─ Shortcuts Content
```

## 4. 阶段主链

```text
Preferences Navigation
  ↓
Content Mode
  ↓
Setting Rows / Controls
  ↓
Validation / Restart / Feedback
```

## 5. 关键状态 / 资源归属

- Window geometry：Preferences Shell。
- 当前分类：Source List。
- 单项设置状态：对应 Setting Row。
- restart required / invalid / reset：Settings feedback。
- Shortcuts：与普通设置共享 Shell，不另造第二窗口。

## 6. Atomic Tasks
### D6-01 Preferences Window Shell

**目的**  
设计独立设置窗口的材质、标题、尺寸和最小宽高。

**主要模块 / 设计对象**  
Preferences Shell

**主要链路**  
`window → source list + content`

**实施重点**  
比主播放器更平静、更阅读型；仍保留浅雾玻璃和柔和边界。

**基本验证**  
标准/窄/高 DPI 视觉。

**完成判断**  
一眼属于同一产品但不是播放器 Overlay。
### D6-02 Source List Navigation

**目的**  
设计分类导航、Selected、Hover、Focus。

**主要模块 / 设计对象**  
Source List

**主要链路**  
`category → selection → content`

**实施重点**  
整行 Selection 柔和；图标/文字对齐；避免后台左栏厚重块。

**基本验证**  
6+分类、键盘 focus。

**完成判断**  
分类可快速扫读。
### D6-03 Settings Section / Row

**目的**  
建立设置分组标题、行、描述、辅助文本和分割规则。

**主要模块 / 设计对象**  
Settings primitives/components

**主要链路**  
`setting concept → row → control`

**实施重点**  
一个 row 一个概念；复杂项升级独立组件；不做卡片套卡片。

**基本验证**  
长描述、禁用、restart required。

**完成判断**  
设置内容密度统一。
### D6-04 通用设置控件

**目的**  
设计 Toggle/Select/Slider/Segmented/TextField 等 Preferences 变体。

**主要模块 / 设计对象**  
Settings controls

**主要链路**  
`value/state → control`

**实施重点**  
与播放器控件共享基础语言，但允许更阅读型尺寸；Focus 清晰。

**基本验证**  
hover/press/focus/disabled/error。

**完成判断**  
不出现网页表单感。
### D6-05 填充 Playback/Video/Audio/Subtitles/Interface/Advanced

**目的**  
用真实播放器设置验证 Shell。

**主要模块 / 设计对象**  
Settings contents

**主要链路**  
`category → real settings → controls`

**实施重点**  
只放 MVP/明确能力；高级原始 mpv 入口必须有高风险警告样式。

**基本验证**  
每类至少一屏真实内容。

**完成判断**  
结构能容纳真实设置，不只是漂亮空壳。
### D6-06 Shortcuts

**目的**  
设计搜索、动作分类、命令行、Keycap、冲突、重置。

**主要模块 / 设计对象**  
Shortcuts content

**主要链路**  
`actions → filter → binding/edit/conflict`

**实施重点**  
命令与按键视觉层级清楚；冲突不能只靠红色。

**基本验证**  
无搜索/搜索/冲突/自定义/恢复默认。

**完成判断**  
快捷键可读、可扫、可编辑。

## 7. 框架打通后优先修复

- Preferences 变成后台管理系统。
- 左栏太宽挤压内容。
- 设置行高不一致。
- Toggle/Select 与播放器内同名控件视觉割裂。
- Shortcuts 只画键帽没有编辑/冲突状态。

## 8. 本阶段最小可评审里程碑

一个完整 Preferences Window，可在多个分类和 Shortcuts 之间切换，保持一致的浅雾、留白和阅读密度。

## 9. 阶段关闭条件

设置窗口骨架稳定；通用 Settings Components 成立；所有主要分类有真实示例。

## 10. 本阶段暂不要求

不设计账号、云同步、插件市场等不在播放器范围的设置。

## 11. 后续扩展位置

新增设置只增加对应 Section/Row；若出现独立复杂工作流，应建立独立 Dialog/Window，不无限堆入 Advanced。
