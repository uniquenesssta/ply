# D3-05 Volume Cluster — 完成记录

> Task ID：D3-05  
> 状态：Complete  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-05 Board：`127:2` — `D3-05 / Volume Cluster`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-05 定义，完成 mute、volume slider、responsive popover 与 HUD 入口关系的正式 Volume Contract。

本任务覆盖：

- `volume` 与 `mute` 的独立状态所有权；
- `0 / Low / Medium / High / Muted`；
- Wide Inline Volume；
- Narrow / Standard Volume Popover；
- Volume Slider 的 visual track / hit target / thumb；
- 键盘与媒体键的 D5 HUD 反馈入口；
- Main / Fullscreen / Mini 的共享状态语义；
- D3-07 OSC Visibility 的后续锁定要求。

本任务不提前实现：

- D3-06 Utility Action Cluster；
- D3-07 OSC Hidden / Rest / Active / LockedVisible 全生命周期；
- D5 最终 HUD Surface；
- D7 Fullscreen / Mini 的最终窗口几何；
- D8 正式 Component Set。

## 2. 基线审计

D3-05 开始前读取已确认框架：

```text
Main Player / Playlist Inspector
Volume visual icon = 22
Volume Track       = 100×3

Fullscreen
Volume visual icon = 21
```

D2-05 已经冻结 Responsive Volume Presentation：

```text
Narrow   = popover
Standard = popover
Wide     = inline
```

已有可复用基础：

```text
size/control/hit-min       = 32
size/control/icon          = 22
size/control/icon-compact  = 21
size/volume/track-width    = 100
size/track/thickness       = 3
radius/track               = 2

control/track
control/progress
control/thumb
control/thumb-border
control/disabled

opacity/control/idle       = 72%
opacity/control/hover      = 100%
opacity/control/pressed    = 84%
opacity/control/disabled   = 38%

z/popover = 60
z/hud     = 70
```

现有文件没有可复用的已发布 Volume / Popover Component，因此本任务继续使用当前文件的 Variable / Text Style / Effect Style，不引入外部 UI Kit，也不提前创建 D8 Component Set。

## 3. 状态所有权

D3-05 正式冻结：

```text
numeric volume ≠ mute flag
```

也就是：

```text
volume=0, mute=false
```

和：

```text
volume=64, mute=true
```

是两个完全不同的状态。

### 3.1 Volume 0

`volume=0 / mute=false`：

- 使用 zero speaker glyph；
- Slider Thumb 真正停在起点；
- 不显示 Volume Progress；
- 后续增加 volume 时直接恢复声音，不需要先解除 mute。

### 3.2 Muted

`mute=true` 不改写 numeric volume。

示例：

```text
volume=64
mute=true
```

视觉规则：

- 使用 muted speaker glyph；
- Slider 仍保留 64% remembered position；
- remembered progress 与 thumb 使用 `opacity/control/disabled=38%`；
- unmute 后恢复到原 volume value。

因此 Muted 不会伪装成 Volume 0。

## 4. 音量级别映射

正式阈值：

```text
0%       → Zero
1–33%    → Low
34–66%   → Medium
67–100%  → High
mute=true→ Muted override
```

D3-05 Board 实际样本：

```text
Zero    0%
Low    24%
Medium 55%
High   85%
Muted  64% stored
```

Mute glyph 的优先级高于 numeric volume glyph。

## 5. Wide Inline Volume

正式结构：

```text
Volume Cluster
├─ Volume Trigger  32
├─ Gap               6
└─ Volume Slider    100
```

最终：

```text
Cluster = 138×32
```

没有为了 Volume 改变 D3-04 Transport：

```text
Transport = 116×40
```

也没有扩张 OSC Surface Max Width。

## 6. Volume Slider 几何

Volume Slider 继承 D3-02 Timeline 的“视觉轨道与交互命中区分离”原则。

正式几何：

```text
Component Width = 100
Hit Height      = 16
Visual Track    = 90×3
Track X         = 5
Thumb           = 10×10
```

公式：

```text
thumbX = volumeRatio × 90
```

端点验证：

```text
0%:
thumbX = 0

100%:
thumbX = 90
thumbRight = 90 + 10 = 100
endpoint error = 0
```

所以 Volume Slider 在两端都不会溢出 100px Component。

## 7. Slider Visual Material

最终材质：

```text
Base Track     = control/track    / 16%
Volume Value   = control/progress / 70%
Thumb Fill     = control/thumb    / 96%
Thumb Border   = control/thumb-border / 28%
```

Muted remembered state：

```text
Stored Volume Value node opacity = 38%
Stored Volume Thumb node opacity = 38%
```

颜色仍然使用同一套 Timeline / Control Semantic Color，没有新增“Volume 紫色”或另一套 Slider Theme。

## 8. Narrow / Standard Popover

正式 Responsive Policy：

```text
Narrow   → Trigger only in OSC + z60 Popover
Standard → Trigger only in OSC + z60 Popover
Wide     → Inline Volume
```

因此 Narrow / Standard 的 Control Lane 只消耗：

```text
32px Volume Trigger
```

100px Slider 不继续占用 OSC 横向空间。

### 8.1 Popover Surface

正式 Volume Popover：

```text
170×52
R20
Fill 52%
Border 48%
z60
V3 / Glass / Popover
```

内部：

```text
100px Volume Slider
+ percentage value
```

Narrow Trigger visual=`21px`，Standard Trigger visual=`22px`；两者 Hit Target 都保持 `32px`。

## 9. 新增 Popover Material Semantic

D3-05 发现 D1 已有 Popover z-order / Motion，但没有独立 Popover Material 名称。

因此新增：

```text
V3 / Glass / Popover
```

其 blur / shadow 复用既有 HUD / Control 轻悬浮材质：

```text
Background Blur = 18
Soft Shadow Radius = 16
Shadow Offset Y = 5
```

新增 Material Semantic：

```text
alpha/glass/popover-fill → 52%
```

这样 D3-06 后续轻量 Utility Popover 可以消费明确的 Popover 责任，不需要借用名不副实的 HUD Style。

## 10. 其他新增 Semantic Alias

D3-05 只补真实职责：

```text
size/volume/hit-height = 16
size/volume/thumb      = 10
radius/surface/popover = 20
alpha/glass/popover-fill = 52%
```

没有新增：

- Track Color；
- Progress Color；
- Thumb Color；
- Thumb Border Color；
- 新 Volume Accent；
- 新 Responsive breakpoint。

## 11. 输入 / Mutation Contract

### Speaker click

```text
toggle mute
preserve numeric volume
```

### Slider drag > 0

```text
set volume(target)
if muted:
    clear mute
```

### Slider drag → 0

```text
volume = 0
mute = false
```

因此“拖到 0”与“点击 Mute”不会进入同一个状态。

## 12. Feedback Routing

### 12.1 OSC 内直接交互

Wide Inline Slider 或已打开 Popover 内的 Slider 本身已经提供足够的本地反馈。

因此：

```text
pointer drag / mute click
→ local feedback only
→ no extra HUD
```

避免一边拖 Slider 一边再出现第二层 HUD。

### 12.2 Keyboard / Media Key

当 volume / mute 由键盘或系统媒体键改变，并且本地 Volume Control 没有处于可见交互态时：

```text
volume/mute mutation
→ D5 HUD payload
→ z70 HUD
```

D3-05 只冻结 payload/entry route，不提前定义 D5 HUD 的最终组件和生命周期。

D3-05 Board 中的 `D5 HUD Entry Stub` 仅用于验证反馈链，尺寸：

```text
122×48
Fill 52%
Border 48%
V3 / Glass / HUD
```

不是 D5 最终 HUD Contract。

## 13. OSC Visibility 依赖

当 Volume Popover 打开：

```text
z60 Popover open
→ D3-07 OSC visibility must be LockedVisible
```

Popover 关闭以后：

```text
release D3-07 visibility lock
```

D3-05 只声明这一跨任务契约，不提前实现 D3-07 的完整状态机。

## 14. Window Mode Continuity

Main / Fullscreen / Mini 必须消费同一个 Volume State Model：

```text
numeric volume
mute flag
zero/low/medium/high/muted thresholds
speaker toggle semantics
slider mutation semantics
```

Main Player 的 Presentation 由 D2-05 Responsive Policy 决定。

Fullscreen / Mini 的最终几何留 D7，但 D7 不允许重新创造第二套 mute/volume 语义、阈值或状态所有权。

## 15. Motion

复用 D1 已有 Popover / HUD Motion：

```text
Popover Open  = 160ms
Popover Close = 120ms
HUD Show      = 160ms
HUD Hide      = 120ms
```

Reduce Motion：

```text
Popover Open  = 0ms
Popover Close = 0ms
HUD Show      = 0ms
HUD Hide      = 0ms
```

没有为 Volume 单独创建新的动画时长系统。

## 16. Figma Board

新增：

```text
D3-05 / Volume Cluster
├─ State Ownership Rule
├─ Responsive Rule
├─ Feedback Routing Rule
├─ Wide Inline Anatomy
├─ 0 / Low / Medium / High / Muted State Matrix
├─ Narrow / Standard Popover Presentation
├─ Local Control vs D5 HUD Feedback Routing
├─ Narrow / Standard / Wide Context Stress
├─ Main / Fullscreen / Mini Window Mode Continuity
├─ Interaction / Ownership Contract
└─ Closing Rule
```

Board Node：`127:2`。

## 17. 实施中修正

### 17.1 Figma Plugin API 节点类型

第一次 Board 创建使用了不存在的 `figma.createRoundedRectangle()`。

该次 Board mutation 失败，没有作为 D3-05 成品保留。随后改为：

```text
figma.createRectangle()
+ cornerRadius
```

并重新完整创建 Board。

### 17.2 Semantic Paint opacity 重置

第一次完整 Board 审计发现 Variable Color Binding 再次把多类 Paint opacity 写回 `100%`：

- Track；
- Progress；
- Thumb；
- Thumb Border；
- Popover；
- HUD Stub；
- Board Card / Outline。

已执行独立材质恢复：

```text
92 nodes restored
```

只恢复 Paint opacity，不改变：

- Semantic Color binding；
- Geometry binding；
- Text Style；
- Effect Style；
- Responsive Policy；
- Muted node opacity binding。

## 18. 最终验证

### Geometry

```text
Inline Volume = 138×32 / gap6
Trigger       = 32×32
Slider        = 100×16
Visual Track  = 90×3
Thumb         = 10×10
```

Endpoint：

```text
0% x=0
100% x=90
right=100
error=0
```

### Muted State

最终审计：

```text
Stored Value position = 57.6px @ 64%
Stored Value opacity  = 38%
Stored Thumb position = 57.6px
Stored Thumb opacity  = 38%
```

### Material

```text
Track        16%
Progress     70%
Thumb        96%
Thumb Border 28%

Popover Fill   52%
Popover Border 48%
HUD Fill       52%
HUD Border     48%
```

Popover 实际 Effect Style ID 与 `V3 / Glass / Popover` 一致；HUD Stub 实际 Effect Style ID 与 `V3 / Glass / HUD` 一致。

### Semantic Paint / Typography

```text
Visible Solid Paint = 315
Semantic-bound       = 315
Unbound              = 0

Text Nodes           = 133
Text Style applied   = 133
```

### Responsive

```text
Narrow   = popover
Standard = popover
Wide     = inline
```

### Motion

```text
Popover Open/Close = 160 / 120ms
HUD Show/Hide      = 160 / 120ms
Reduce Motion      = 0ms for all above
```

### D1 回归

```text
Semantic Color  142 / 142
Typography       43 / 43
Geometry         55 / 55
Effect           25 / 25
```

### 上游保持

以下正式 Board 均存在且未被 D3-05 修改：

- D2-01 Player Window Shell；
- D2-02 Video Viewport；
- D2-03 Floating Header；
- D2-04 Host Spatial Contract；
- D2-05 Responsive Window Skeleton；
- D3-01 OSC Surface & Internal Grid；
- D3-02 Timeline Basic Geometry；
- D3-03 Timeline Interaction States；
- D3-04 Transport Cluster。

## 19. 下一任务

`D3-06 Utility Action Cluster`

目标：统一字幕、音轨、章节、播放列表、更多、全屏等 Utility Action 的图标优先级、Inspector / Popover / Window Mode 路由、Disabled / Focus / Open 状态，并保持视觉权重低于 Transport。