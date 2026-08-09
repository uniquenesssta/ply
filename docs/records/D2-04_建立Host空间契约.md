# D2-04 建立 Host 空间契约 — 完成记录

> Task ID：D2-04  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`02 Player Window` — `48:11`  
> D2-04 Board：`70:2` — `D2-04 / Host Spatial Contract`

## 1. 任务目标

按照 `D2_播放器窗口与视频视口.md` 的 D2-04 定义，只建立 OSC Host / Inspector Host / Overlay Host 的：

- 安全区；
- 可用范围；
- Host 间碰撞规则；
- Layer / z-order；
- Inspector 开关时的 Host range 变化；
- Error / Status Overlay 与 OSC / Inspector 同时存在时的空间关系。

本任务不提前实现：

- D3 OSC 内部 Timeline / Transport / Volume / Actions；
- D4 Inspector Shell 内部 Header / List / Playlist / Tracks / Subtitles / Chapters；
- D5 Error / Loading / Buffering 等真实业务内容。

D2-04 中出现的 Error Overlay 只是 D5 content placeholder，用于验证 Overlay Host 是否有可用空间。

## 2. 审计发现：探索稿 Inspector 与正式 Window Actions 存在碰撞

现有第三版探索稿中：

```text
Window = 1320×700
Floating Header = y26 / h54
Window Actions = top 26 / bottom 80
Floating Inspector = x924 / y24 / w368 / h652
```

如果直接把探索稿 Inspector 的 `y=24` 继续作为正式 Host，它会占用 D2-03 已冻结的右上 Window Actions 区域。

D2-04 不允许通过：

- 临时把 Window Actions 向左挪；
- 提高单个 Window Action z；
- 给 Inspector 挖异形缺口；
- 每个场景手工移动 Inspector；

来掩盖这个碰撞。

因此正式 Inspector Host 改为从 Floating Header 下方开始。

## 3. 正式 Host 几何契约

Reference Window：

```text
1320×700
```

### 3.1 Floating Header 已冻结区域

来自 D2-03：

```text
Header top = 26
Header height = 54
Header bottom = 80
Window Actions right edge = 26
Window Actions = 110×54
```

D2-04 不移动 Window Actions。

### 3.2 Inspector Host

正式公式：

```text
Inspector top
= Header top 26 + Header height 54 + separation 12
= 92

Inspector right = 28
Inspector bottom = 24
Inspector width = 368

Inspector x
= 1320 - 28 - 368
= 924

Inspector height
= 700 - 92 - 24
= 584
```

正式几何：

```text
x924 / y92 / w368 / h584
```

与 Window Actions 的垂直硬间隔：

```text
92 - 80 = 12 px
```

Inspector 内容高度减少后由 D4 自己处理滚动，不反向修改 Window / Header 几何。

### 3.3 OSC Host

继续消费既有 D1 Geometry：

```text
spacing/osc/bottom = 38
size/osc/height = 124
window safe-min = 26
```

Standard Reference：

```text
OSC bottom anchor = 700 - 38 = 662
OSC top = 662 - 124 = 538
```

Inspector Closed：

```text
OSC Host
x26 / y538 / w1268 / h124
```

Inspector Open：

```text
OSC right bound
= Inspector left 924 - safe-min 26
= 898

OSC Host
x26 / y538 / w872 / h124
```

因此 OSC 与 Inspector 保持：

```text
26 px hard separation
```

D2-04 只定义 Host envelope；D3 决定真实 OSC Surface 的最终宽度、内部网格和控件布局。

### 3.4 Overlay Host / Content Safe Rect

新增正式 Overlay 层级：

```text
z/overlay = 35
```

Overlay Root 可以覆盖 Player Window，但可见状态内容使用 unobstructed content rect，避免 Header / OSC / Inspector 冲突。

Vertical：

```text
Overlay top
= Header bottom 80 + safe-min 26
= 106

Overlay bottom
= OSC top 538 - safe-min 26
= 512

Overlay safe height
= 406
```

Inspector Closed：

```text
x26 / y106 / w1268 / h406
```

Inspector Open：

```text
Overlay right bound
= Inspector left 924 - safe-min 26
= 898

x26 / y106 / w872 / h406
```

Error / Loading / Buffering 等 D5 状态可以挂入该 content safe rect，但 D5 可以根据状态类型决定视觉内容尺寸；D2-04 不提前定义 Error Card 业务布局。

## 4. Z-order 契约

D2-04 新增 `z/overlay=35` 后，播放器关键顺序为：

```text
Media Content      z20
Floating Header    z30
Overlay Host       z35
OSC                z40
Inspector          z50
Popover            z60
HUD                z70
Toast              z80
Dialog Scrim       z90
Dialog             z100
```

因此任务书要求的同时存在状态满足：

```text
Error / Status Overlay z35
< OSC z40
< Inspector z50
```

Overlay 不会因为创建时间更晚而意外盖住 OSC / Inspector。

## 5. Token 补强

本任务只增加两个真实职责及其 Primitive：

### Geometry

- `spacing/92 = 92`
- `spacing/inspector/top-with-header → spacing/92`

Geometry 计数：

```text
Primitive 46 → 47
Semantic  58 → 59
```

### Interaction / Z-order

- `z/35 = 35`
- `z/overlay → z/35`

Interaction 计数：

```text
Primitive 27 → 28
Semantic  43 → 44
```

没有为 `x=924`、`w872`、`h406` 等公式结果创建无意义 Token；这些值由 Window / Header / Inspector width / safe-min / OSC size 共同计算得到。

## 6. Figma 实现

在 `02 Player Window` 新增：

```text
D2-04 / Host Spatial Contract
├─ Contract cards
├─ Stage / Inspector Closed
│  └─ Window Surface
│     ├─ Header fixed regions
│     ├─ Overlay Host / Content Safe Rect
│     └─ OSC Host / Envelope
├─ Stage / Inspector Open
│  └─ Window Surface
│     ├─ Header fixed regions
│     ├─ Overlay Host / Content Safe Rect
│     ├─ OSC Host / Envelope
│     └─ Inspector Host
├─ Stage / Simultaneous Hosts
│  └─ Window Surface
│     ├─ Error Overlay Placeholder
│     ├─ OSC Host
│     └─ Inspector Host
├─ D2-04 / Z Ladder
└─ D2-04 / Ownership Rules
```

Host 使用极淡填充 + dashed/soft border 表达“可用范围”，不伪装成真实 UI Surface。

真实产品内容只有 D2-03 已冻结的 Header ghost region；Error Overlay 仅为 D5 占位验证。

## 7. 实施中发现并修复的问题

### 7.1 第一轮 Host 填充视觉过重

第一轮截图中：

- Overlay Host 使用青色低透明填充；
- OSC Host 使用紫色低透明填充；
- Inspector Host 使用暖色低透明填充；

虽然几何正确，但在强紫蓝 Synthetic Media 背景上仍然看起来像三块真实产品面板，容易误导后续 D3/D4/D5 直接照抄 Host Fill。

已修复：

- Overlay Fill 收敛到 `2.5%`；
- OSC Fill 收敛到 `3%`；
- Inspector Fill 收敛到 `4.5%`；
- 保留明确 outline / dashed host boundary；
- 新增显式 Z Ladder；
- Error Placeholder 保留真实 Glass，仅用于 simultaneous-state 验证。

第二轮截图通过，Host 明确表现为工程范围，不再像产品 Surface。

## 8. Geometry / Collision 最终验证

最终审计：

```text
Inspector Closed
Overlay  x26 y106 w1268 h406
OSC      x26 y538 w1268 h124

Inspector Open
Overlay  x26 y106 w872 h406
OSC      x26 y538 w872 h124
Inspector x924 y92 w368 h584

Simultaneous
Overlay  x26 y106 w872 h406
OSC      x26 y538 w872 h124
Inspector x924 y92 w368 h584
```

硬间隔：

```text
Header → Inspector      12 px
Overlay → OSC           26 px
OSC → Inspector         26 px
Overlay → Inspector     26 px
```

全部达到预期，无 overlap 后再手工 nudging。

## 9. Binding / Layer 验证

```text
OSC Host height Variable binding       3 / 3
Inspector Host width Variable binding  2 / 2
OSC shared z-role                      3 / 3 = z/osc
Inspector shared z-role                2 / 2 = z/inspector
Overlay shared z-role                  3 / 3 = z/overlay
```

D2-04 Board 可见 Solid Paint：

```text
Visible Solid Paints       95
Semantic-bound             95
Unbound                     0
```

Synthetic media gradient 只表达任意视频像素，不属于应用主题色。

## 10. D1 / D2 回归

D1 原始三张核心框架保持：

```text
Semantic Color    142 / 142
Typography         43 / 43
Geometry           55 / 55
Effect             25 / 25
```

D2-01 Standard Window 保持：

```text
1320×700
R32
clipsContent=true
border=70%
V3 / Elevation / Window
```

D2-02 `Video Viewport Contract` 存在且未修改。

D2-03 `Floating Header Contract` 存在且未修改。

## 11. 正式 Ownership 结论

D2-04 后 Window System 对 Host 的唯一职责冻结为：

```text
Window
├─ Header Host / D2-03
├─ OSC Host / D3 content slot
├─ Inspector Host / D4 content slot
└─ Overlay Host / D5 content slot
```

规则：

- Inspector 开关不得 resize Video Viewport；
- Inspector 只改变右侧 floating host 可用宽度；
- OSC 保持 bottom anchor，不因 Inspector 开关上下漂移；
- Overlay content 自动使用 unobstructed rect；
- Header Window Actions 不得被 Inspector 遮挡；
- D3/D4/D5 不得自行修改 Window Shell 来解决局部碰撞；
- Popover / HUD / Dialog 使用 D1 更高层级，不占用 D2-04 的普通 Overlay z35。

## 12. 下一任务

`D2-05 响应式窗口骨架`

下一步将正式冻结 Wide / Standard / Narrow 的窗口布局模式与规则驱动的 Host constraints，并至少验证 720 / 960 / 1280 / 1600 宽度。