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

# Stage D4：Inspector 系统与媒体内部结构

## 1. 本阶段最终要得到什么

建立一个统一的 Inspector Shell，并在同一容器中承载 Playlist、Tracks、Subtitles、Chapters 四类内容。D4 的关键不是“做四个面板”，而是证明它们共享同一个外壳、密度和交互语言。

## 2. 本阶段怎么快速推进

先完成 Inspector Shell、Header、Content Host、Footer/Toolbar，再逐个插入四种内容。所有列表行、分组标题、Selection、空态、搜索、工具按钮优先复用组件。

## 3. 目标模块结构

```text
Inspector
├─ Inspector Shell
│  ├─ Header
│  ├─ Segment / Mode Switch
│  ├─ Search / Tool Area
│  ├─ Content Host
│  └─ Footer
├─ Playlist Content
├─ Tracks Content
├─ Subtitles Content
└─ Chapters Content
```

## 4. 阶段主链

```text
OSC Action
  ↓
Inspector Host
  ↓
Inspector Shell
  ↓
Content Mode
  ↓
Read-only List / Controls
```

## 5. 关键状态 / 资源归属

- Inspector 打开/关闭、尺寸、材质：Shell。
- 当前 content mode：Inspector navigation state。
- Playlist current/selected/hover：不同视觉状态。
- Track selected/pending：Track content。
- Subtitle style/delay：Subtitle content。
- Chapter current/hover：Chapter content。
- 四种内容不得修改 Shell 的几何规则。

## 6. Atomic Tasks
### D4-01 Inspector Shell

**目的**  
建立右侧/覆盖式玻璃 Inspector 的尺寸、材质、标题和内容插槽。

**主要模块 / 设计对象**  
Inspector Shell

**主要链路**  
`host → shell → content slot`

**实施重点**  
面板不能像传统黑抽屉；边界轻，背景透，关闭按钮和标题对齐。

**基本验证**  
主窗口/窄窗口/复杂视频背景。

**完成判断**  
四种内容可以无改壳层直接替换。
### D4-02 Inspector 导航

**目的**  
建立 Playlist / Tracks / Subtitles / Chapters 的切换入口。

**主要模块 / 设计对象**  
Inspector mode switch

**主要链路**  
`mode → content host`

**实施重点**  
切换控件不抢标题；状态明确；不做四个独立 Drawer。

**基本验证**  
四模式连续切换。

**完成判断**  
容器尺寸和头部位置不漂移。
### D4-03 Playlist 内容

**目的**  
设计搜索、队列列表、current/selected/hover、增删/重排入口。

**主要模块 / 设计对象**  
Playlist content

**主要链路**  
`queue model → list → actions`

**实施重点**  
列表项低卡片化；current playing 与 selected 必须可同时表达；长标题和未知时长有规则。

**基本验证**  
空/1项/20项/长标题/current≠selected。

**完成判断**  
播放列表精致且高密度，不像管理后台。
### D4-04 Tracks 内容

**目的**  
设计音频/视频轨、当前选择、音频延迟等。

**主要模块 / 设计对象**  
Tracks content

**主要链路**  
`tracks → selection → delay controls`

**实施重点**  
Section 清晰但不使用卡片套卡片；选择失败/pending 有视觉位置。

**基本验证**  
单轨/多轨/未命名/off/pending。

**完成判断**  
Track 选择与 Playlist 同一列表语言。
### D4-05 Subtitles 内容

**目的**  
设计字幕轨、外挂字幕入口、样式、位置、延迟。

**主要模块 / 设计对象**  
Subtitles content

**主要链路**  
`subtitle state → track/style/delay`

**实施重点**  
高频选择与低频样式设置分层；避免在 Inspector 内塞完整设置页。

**基本验证**  
无字幕/多字幕/off/外挂/pending。

**完成判断**  
字幕主操作在单屏内容中可理解。
### D4-06 Chapters 内容

**目的**  
设计章节时间点、当前章节、跳转。

**主要模块 / 设计对象**  
Chapters content

**主要链路**  
`chapters → current projection → seek intent`

**实施重点**  
章节更偏阅读；时间码对齐；当前章节 Selection 轻。

**基本验证**  
无章节/多章节/超长标题。

**完成判断**  
点击章节的视觉反馈与 Timeline chapter marker 同语义。
### D4-07 Inspector 响应式与覆盖关系

**目的**  
定义标准宽度、窄窗口覆盖、关闭、切换时保持。

**主要模块 / 设计对象**  
Inspector responsive state

**主要链路**  
`window width + mode → inspector layout`

**实施重点**  
窄窗口允许覆盖视频但不能改变播放器核心几何；ESC/点击外部策略在原型中明确。

**基本验证**  
720/960/1280 宽度。

**完成判断**  
没有每个 content 自己决定 Drawer 宽度。

## 7. 框架打通后优先修复

- Playlist/Tracks/Subtitles/Chapters 四套列表行高不一致。
- Selected 使用过强蓝色块。
- Inspector 过宽导致视频失去主视觉。
- 搜索、Segmented、Footer 在不同模式位置漂移。
- 低频设置塞进 Inspector 造成信息过载。

## 8. 本阶段最小可评审里程碑

一个 Inspector Shell 内可以连续切换四种内容，整体位置、材质、标题、列表密度和交互规则保持一致。

## 9. 阶段关闭条件

四类媒体内部结构均有完整正常/空/选中/错误或 pending 展示；没有重复 Shell。

## 10. 本阶段暂不要求

不把完整 Preferences 搬进 Inspector；不做在线字幕搜索、媒体库。

## 11. 后续扩展位置

未来 Media Info 可作为新的 Inspector mode；若需要完全不同生命周期，应另立 Overlay/Window，不硬塞。
