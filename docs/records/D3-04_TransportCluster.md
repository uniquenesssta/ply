# D3-04 Transport Cluster — 完成记录

> Task ID：D3-04  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-04 Board：`117:2` — `D3-04 / Transport Cluster`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-04 定义，完成 Previous / PlayPause / Next 核心 Transport Controls 的正式视觉与交互契约。

本任务覆盖：

- Previous；
- Play / Pause；
- Next；
- Playing / Paused；
- Rest / Hover / Pressed / Focus / Disabled；
- 命中尺寸与视觉尺寸分离；
- 图标光学中心；
- Narrow / Standard / Wide 控制区压力测试。

本任务不提前实现：

- D3-05 Volume Cluster；
- D3-06 Utility Action Cluster；
- D3-07 OSC 显隐生命周期；
- D8 正式 Component Set 收口。

## 2. 来源与基线审计

D3-04 开始前先读取现有 Main Player / Fullscreen / Playlist Inspector Transport：

- Main Previous / Next visual frame：`22×22`；
- Main Play：`40×40 / R20 / Glass Control`；
- Fullscreen transport visual frame：`21×21`；
- Main Play triangle 已存在约 `+0.6px` 向右光学补偿；
- Previous / Next 已存在约 `+0.9px / -0.9px` 的镜像光学补偿。

确认 D1 已有可复用基础：

```text
size/control/hit-min       = 32
size/control/icon          = 22
size/control/icon-compact  = 21
size/control/playback      = 40
radius/control/playback    = 20
spacing/control/tight      = 6

opacity/control/idle       = 72%
opacity/control/hover      = 100%
opacity/control/pressed    = 84%
opacity/control/disabled   = 38%

focus/ring
V3 / Glass / Control
```

设计系统搜索未发现已发布 Transport Component，因此 D3-04 继续消费当前文件本地 Variable / Effect Style，不引入外部 UI Kit，也不提前制造正式组件库。

## 3. 正式 Transport Geometry

Transport Cluster 冻结为：

```text
Transport Cluster
├─ Previous   32 hit / 22 visual
├─ gap         6
├─ PlayPause  40 hit / R20
├─ gap         6
└─ Next       32 hit / 22 visual
```

Cluster 使用 Auto Layout / Hug Contents：

```text
32 + 6 + 40 + 6 + 32 = 116px
```

最终：

```text
Cluster Width  = 116
Cluster Height = 40
Gap            = 6
```

没有创建 `cluster-width=116` 这种多余固定 Token；宽度由真实子节点与 `spacing/control/tight` 自动计算。

## 4. Primary Play / Pause

Primary 使用同一个 40px 白色 Glass Control，不使用品牌色实心圆。

### Playback State

```text
Paused  → Play glyph
Playing → Pause glyph
```

只替换 glyph，以下属性保持不变：

- 40×40 hit / visual surface；
- R20；
- Glass Control blur / shadow；
- Control lane 占位；
- 不发生 layout shift。

### Pointer / Focus State

```text
Rest     fill 48% / border 48%
Hover    fill 52% / border 48%
Pressed  fill 42% / border 48%
Focus    fill 48% / focus ring 82%
Disabled fill 48% / whole control opacity 38%
```

Hover 只比 Rest 增强 4%，Pressed 只降低到 42%，保证主按钮明确但不成为视频上的视觉主体。

## 5. Previous / Next

Previous / Next 不增加 Rest/Hover 圆形 Surface，只使用 icon opacity 表达优先级。

```text
Rest      icon 72%
Hover     icon 100%
Pressed   icon 84%
Focus     icon 100% + 1px focus ring 82%
Disabled  icon 38%
```

Hit target 始终保持：

```text
32×32 / R16
```

因此视觉图标保持轻量，同时 desktop hit target 不因窗口变窄而缩小。

## 6. 光学中心规则

正式保留第三版框架已经成立的光学补偿：

```text
Previous ≈ +0.9px
Play     ≈ +0.6px
Next     ≈ -0.9px
```

说明：

- Hit target 几何中心保持绝对对齐；
- Triangle / skip glyph 自身允许光学偏移；
- Previous / Next 使用镜像补偿；
- 禁止为了数学中心把 glyph 调回肉眼偏斜状态。

Figma `OPTICAL CENTER QA` 已用 geometric crosshair 独立验证上述规则。

## 7. Responsive / Context Stress

D3-04 使用 D3-01 已冻结 Control Lane 进行三档压力测试：

```text
Narrow   560px lane
Standard 804px lane
Wide     828px lane
```

三档均保持：

```text
Transport Cluster = 116×40
Previous / Next hit = 32
PlayPause = 40
Gap = 6
```

Narrow 模式只允许 D3-05 / D3-06 后续内容降级：

- Volume → Popover；
- Utility Actions → collapse / More；
- Transport 几何不缩小。

## 8. Token 补强

D3-04 只增加真实缺口。

### Geometry

新增：

```text
radius/16
radius/control/transport-secondary → radius/16
```

已有 `32 / 22 / 40 / R20 / gap6` 全部直接复用，没有重复创建。

### Material Semantic

复用已有 Effect Primitive：

```text
alpha/42
alpha/48
alpha/52
```

新增产品语义：

```text
alpha/transport/primary/rest    → 48%
alpha/transport/primary/hover   → 52%
alpha/transport/primary/pressed → 42%
```

### Interaction Semantic

新增：

```text
motion/control/state-duration
Standard = 120ms
Reduce Motion = 0ms

motion/control/press-duration
Standard = 0ms
Reduce Motion = 0ms

motion/control/state-easing
Standard = cubic-bezier(0.2, 0, 0, 1)
Reduce Motion = linear
```

Press 反馈保持即时，Hover / Focus 使用现有短时 Ease Out 节奏。

## 9. Figma Board

新增：

```text
D3-04 / Transport Cluster
├─ Primary Hierarchy Rule
├─ Secondary Hierarchy Rule
├─ Optical Center Rule
├─ Transport Anatomy
├─ Primary Play / Pause States
├─ Previous / Next States
├─ Narrow / Standard / Wide Context Stress
├─ Optical Center QA
├─ Interaction Contract
└─ Closing Rule
```

Board Node：`117:2`。

## 10. 实施中发现并修复的问题

### 10.1 Semantic Paint Binding 重置透明度

第一轮 Board 创建后，结构与 Color Variable binding 正确，但 Figma 再次把部分 Paint opacity 写回 `100%`。

实际影响：

- Primary Rest / Hover / Pressed 的 `48 / 52 / 42%` 全部变成 100%；
- Primary Border 变成 100%；
- Focus Ring 从目标 82% 变成 100%；
- 文档 Card 的 24–36% Glass 也被放大。

已执行独立第二阶段材质恢复：

- 恢复 54 个节点的目标 Fill / Stroke opacity；
- 保留所有 Semantic Color binding；
- 保留 Geometry binding；
- 保留 Effect Style；
- 保留 Node Opacity Variable binding。

第二轮截图重新验收通过。

## 11. 最终验证

### Geometry

```text
Transport Cluster  116×40 / gap6
Primary            40×40 / R20
Secondary Hit      32×32 / R16
Secondary Visual   22×22
```

所有 Cluster 的 `itemSpacing` 均真实绑定 `spacing/control/tight`。

### Primary Material

```text
Rest / Playing  48%
Hover           52%
Pressed         42%
Focus Fill      48%
Focus Ring      82%
Disabled Node   38%
```

所有 Primary 均真实挂载：

```text
V3 / Glass / Control
```

### Secondary State

```text
Rest      72%
Hover     100%
Pressed   84%
Focus     100% + 82% ring
Disabled  38%
```

### Semantic Paint Audit

```text
Visible Solid Paint = 255
Semantic-bound       = 255
Unbound              = 0
```

### Motion

```text
motion/control/state-duration  Standard=120ms / Reduce=0ms
motion/control/press-duration  Standard=0ms   / Reduce=0ms
motion/control/state-easing    Standard=Ease Out / Reduce=linear
```

### D1 回归

```text
Semantic Color  142 / 142
Typography       43 / 43
Geometry         55 / 55
Effect           25 / 25
```

### 上游保持

以下正式 Board 均保持存在且未被 D3-04 修改：

- D2-01 Player Window Shell；
- D2-02 Video Viewport；
- D2-03 Floating Header；
- D2-04 Host Spatial Contract；
- D2-05 Responsive Window Skeleton；
- D3-01 OSC Surface & Internal Grid；
- D3-02 Timeline Basic Geometry；
- D3-03 Timeline Interaction States。

## 12. 保持不变

D3-04 没有：

- 修改 Timeline 3px / 16px Hit / 10px Thumb；
- 修改 OSC 116 之外的 Control Lane 结构；
- 新增 Stop / A-B Loop / Screenshot 等高级 Action；
- 设计 Volume；
- 设计 Utility Actions；
- 建立正式 Figma Component Set；
- 修改 Main / Fullscreen / Inspector 已确认框架。

## 13. 下一任务

`D3-05 Volume Cluster`

目标：定义 mute、volume slider、popover / HUD 入口关系，并确保 Narrow 模式可以折叠 Volume，而不让主 OSC 塞入过长 slider。