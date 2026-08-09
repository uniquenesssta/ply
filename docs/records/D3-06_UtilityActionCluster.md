# D3-06 Utility Action Cluster — 完成记录

> Task ID：D3-06  
> 状态：Complete  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-06 Board：`134:2` — `D3-06 / Utility Action Cluster`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-06 定义，统一 Subtitles、Audio Tracks、Chapters、Playlist、More、Fullscreen 六类 Utility Action 的入口语言、状态、响应式降级和目标 Surface 所有权。

本任务覆盖：

- Utility Action 的统一 `32px` desktop hit geometry；
- Standard/Wide `22px` visual icon 与 Narrow `21px` compact visual icon；
- Rest / Hover / Pressed / Focus / Disabled / Panel Open；
- Inspector destination、More Popover 与 Fullscreen Window Mode 的责任划分；
- Narrow / Standard / Wide 的 `utility/policy`；
- Inspector destination 切换、More Popover 与 Fullscreen Enter/Exit 的交付契约；
- D3-07 OSC Visibility 的后续锁定要求。

本任务不提前实现：

- D4 Inspector 内部 Playlist / Tracks / Subtitles / Chapters 内容；
- D3-07 Hidden / Rest / Active / LockedVisible 生命周期；
- Speed / A-B Loop / Screenshot 等后续高级 Utility Action；
- D8 正式 Figma Component Set 收口。

## 2. 现状与设计系统审计

已确认第三版框架当前真实使用：

- Main Player：Subtitles / Playlist / Fullscreen；
- Fullscreen：同一 Utility 语言的 compact visual；
- Audio Tracks / Chapters / More 在 D3-06 首次正式进入 Utility 系统。

设计系统搜索未发现可直接复用的已发布 Utility Component，因此本任务继续复用本文件现有 Variable / Text Style / Effect Style，不引入外部 UI Kit，也不提前建立正式 Component Library。

D1/D2/D3 已存在且直接复用的基础：

```text
size/control/hit-min                 = 32
size/control/icon                    = 22
size/control/icon-compact            = 21
radius/control/transport-secondary   = 16
spacing/control/tight                = 6

opacity/control/idle                 = 72%
opacity/control/hover                = 100%
opacity/control/pressed              = 84%
opacity/control/disabled             = 38%
focus/ring

surface/selection
border/selection
icon/secondary
icon/primary
alpha/glass/selection-fill           = 66%
alpha/glass/selection-border         = 28%

z/inspector                          = 50
z/popover                            = 60

motion/control/state-duration        = 120ms
motion/control/press-duration        = 0ms
motion/popover/open-duration         = 160ms
motion/popover/close-duration        = 120ms
Reduce Motion                        = 0ms
```

结论：D3-06 **没有新增 Color / Geometry / Material / Motion Token**；任务重点是产品入口语义与编排。

## 3. Utility Action 几何

统一几何：

```text
Hit Target       = 32×32
Radius           = R16
Visual Icon      = 22×22 Standard / Wide
Compact Visual   = 21×21 Narrow
Adjacent Gap     = 6
```

Utility 的视觉权重保持低于 Transport：

- 不使用 Primary Play/Pause 的 Glass Control surface；
- Rest/Hover/Pressed 仅通过 secondary icon opacity 变化；
- 只有 Focus 或 Panel Open 才出现必要的 ring / selection anchor。

## 4. Action Routing / Ownership

正式所有权：

```text
Subtitles    → Inspector / Subtitles    → z50
Audio Tracks → Inspector / Audio Tracks → z50
Chapters     → Inspector / Chapters     → z50
Playlist     → Inspector / Playlist     → z50
More         → Popover / More            → z60
Fullscreen   → Window Mode Toggle        → Enter / Exit Fullscreen
```

### 4.1 Inspector Destination

Subtitles / Audio Tracks / Chapters / Playlist **共享同一个 D4 Inspector Shell**。

规则：

- 同时只能存在一个 active Inspector destination；
- 从 Playlist 切到 Subtitles 时只替换同一个 Shell 的内容；
- 不创建第二个 Drawer / Inspector；
- Action 的 Panel Open 状态只是当前 Inspector destination 的入口锚点。

### 4.2 More

More 只拥有 `z60` Popover：

- 不创建第二个 Inspector；
- 可以和 Inspector z50 同时存在；
- Esc 应优先关闭最上层 More Popover；
- D3-07 后续在 More 打开期间将 OSC 视为 visibility lock；
- Speed / A-B Loop / Screenshot 等后续高级 Action 可插入 More，不修改 OSC 主结构。

### 4.3 Fullscreen

Fullscreen 只拥有 Window Mode：

```text
Windowed    → Enter Fullscreen glyph
Fullscreen  → Exit Fullscreen glyph
```

Fullscreen 不使用 Inspector 的 `Panel Open` selection 状态，也不创建任何 z50/z60 surface。

## 5. Generic Utility States

正式状态：

```text
Rest       icon/secondary 72%
Hover      icon/secondary 100%
Pressed    icon/secondary 84%
Focus      icon 100% + focus ring 82%
Disabled   icon 38% / geometry unchanged
Panel Open selection fill 66% / border 28% / icon/primary 100%
```

Panel Open 使用克制的 selection anchor，而不是大蓝色/大紫色按钮。

## 6. Responsive Policy

D3-06 完全消费 D2-05 已冻结的：

```text
utility/policy
Narrow   = essential+more
Standard = mixed+more
Wide     = full
```

正式可见入口：

### Narrow — `108px`

```text
Subtitles / More / Fullscreen
```

More 内：

```text
Audio Tracks / Chapters / Playlist
```

几何：

```text
3×32 + 2×6 = 108
```

### Standard — `146px`

```text
Subtitles / Playlist / More / Fullscreen
```

More 内：

```text
Audio Tracks / Chapters
```

几何：

```text
4×32 + 3×6 = 146
```

### Wide — `222px`

```text
Subtitles / Audio Tracks / Chapters / Playlist / More / Fullscreen
```

More 保留为高级 overflow 入口。

几何：

```text
6×32 + 5×6 = 222
```

## 7. Control Lane Stress

D3-06 与 D3-04 / D3-05 的现有尺寸联合验证：

```text
Transport = 116×40
Volume Narrow / Standard = 32 trigger + Popover
Volume Wide = 138×32 Inline
Utility Narrow = 108×32
Utility Standard = 146×32
Utility Wide = 222×32
```

验证对象：

```text
Narrow   560px control lane
Standard 804px control lane
Wide     828px control lane
```

结果：

- Transport 不缩；
- Volume 不缩 hit target；
- Utility 不缩 `32px` hit target；
- 中间自由空间承担宽度变化；
- Wide 同时存在 Inline Volume + Full Utility 仍不要求把 OSC 变成满宽底栏。

## 8. Figma Board

新增：

```text
D3-06 / Utility Action Cluster
├─ Routing / Hierarchy / Responsive Rules
├─ Anatomy · Wide Full Set
├─ Action Routing / Ownership
├─ Generic Utility States
├─ Responsive Composition
│  ├─ Narrow
│  ├─ Standard
│  └─ Wide
├─ Open Surface Contract
│  ├─ Inspector Destination Switch
│  ├─ More z60 Popover
│  └─ Fullscreen Enter / Exit
├─ Control Lane Stress
├─ Interaction / Visibility Contract
└─ Closing Rule
```

Board Node：`134:2`。

关键 Action Frame 使用 `setSharedPluginData('openai.v3player', 'utility-route', ...)` 标注目标 ownership，供后续 Handoff/QA 追溯。

## 9. 实施中发现并修复的问题

### 9.1 Semantic Paint Binding 再次把透明度写回 100%

第一轮结构与 Color binding 均正确，但终审发现：

- Focus ring 目标 `82%` 被写成 `100%`；
- Panel Open fill 目标 `66%` 被写成 `100%`；
- Panel Open border 目标 `28%` 被写成 `100%`；
- More Popover fill 目标 `52%` 被写成 `100%`；
- More Popover border 目标 `48%` 被写成 `100%`；
- 部分文档 Card / Divider 的低透明度也被同步放大。

修复方式：

- 不重新绑定颜色；
- 不修改 geometry / routes / responsive；
- 单独第二阶段恢复 50 个节点的目标 Fill / Stroke opacity；
- 保留 Semantic Color、Text Style、Effect Style、Shared Route Metadata。

第二轮截图重新验收通过。

## 10. 最终验证

### Geometry / Responsive

```text
Narrow Utility Cluster    = 108×32 / gap6 / icon21
Standard Utility Cluster  = 146×32 / gap6 / icon22
Wide Utility Cluster      = 222×32 / gap6 / icon22
Every Utility Hit Target  = 32×32 / R16
```

### State

```text
Rest       72%
Hover      100%
Pressed    84%
Focus      82% ring
Disabled   38%
Panel Open 66% fill / 28% border / primary icon
```

### More Popover

```text
250×176 / R20
Fill   52%
Border 48%
Effect V3 / Glass / Popover
z60
```

### Figma Semantic Audit

```text
Visible Solid Paint = 396
Semantic-bound       = 396
Unbound              = 0

Text Nodes           = 155
Text Styled          = 155
```

### Responsive Variable

```text
utility/policy
Narrow   = essential+more
Standard = mixed+more
Wide     = full
```

### D1 回归

```text
Semantic Color  142 / 142
Typography       43 / 43
Geometry         55 / 55
Effect           25 / 25
```

### 上游保持

以下 Board 全部保持存在且未被 D3-06 修改：

- D2-01 Player Window Shell；
- D2-02 Video Viewport；
- D2-03 Floating Header；
- D2-04 Host Spatial Contract；
- D2-05 Responsive Window Skeleton；
- D3-01 OSC Surface & Internal Grid；
- D3-02 Timeline Basic Geometry；
- D3-03 Timeline Interaction States；
- D3-04 Transport Cluster；
- D3-05 Volume Cluster。

## 11. 保持不变

D3-06 没有：

- 修改 Timeline；
- 修改 Transport；
- 修改 Volume state / slider；
- 实现 Inspector 业务内容；
- 将 Fullscreen 伪装成 Panel Open；
- 创建第二套 Inspector；
- 新增生产级组件库；
- 修改已确认 Main / Fullscreen / Playlist Inspector 框架。

## 12. 下一任务

`D3-07 OSC 显隐生命周期`

目标：建立 Hidden / Rest / Active / LockedVisible 的完整可交付规则，统一 hover、scrub、Inspector、Popover、paused、error、keyboard action 与 Fullscreen 下 OSC 的显隐 ownership，并关闭 Stage D3。