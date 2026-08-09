# D1-04 建立 Spacing / Size / Radius — 完成记录

> Task ID：D1-04  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Foundations Page：`01 Foundations`  
> Geometry 文档板节点：`30:2` — `D1-04 / Spacing Size Radius`

## 1. 任务基线

D1 阶段任务书对 D1-04 的要求为：

- 把第三版的强留白与小控件密度写成数值体系；
- 目标对象为 `spacing / size / radius variables`；
- 主链为 `primitive metrics → semantic layout`；
- 必须区分窗口/浮层级留白、Panel 内边距、Control gap、List row gap；
- Radius 必须按 Surface / Control / Pill 等职责分层；
- 完成判断：同类结构不应继续依赖无语义的随机数值。

当前 Preferences 尚未进入 D6，因此本任务没有为 Preferences 虚构具体布局；只建立未来 Preferences 可以复用的通用 Surface / Padding / Control Geometry 语义，并以已经确认的 Main Player / Fullscreen / Playlist Inspector 作为真实基线。

## 2. 真实 Geometry 审计

对三张已确认核心框架执行完整几何扫描，重点提取尺寸、圆角、相对边距和重复间距。

确认的关键几何包括：

### Player / Window

- Main Player Window：`1320 × 700 / R32`
- Fullscreen Surface：`1344 × 700 / R34`

### Floating Header

- Main Header：`H54 / R27 / top 26`
- Fullscreen Header：`H50 / R25 / top 26`
- Main Header content inset：`22`
- Compact Header content inset：`24`

### Main OSC

- Height：`124`
- Radius：`34`
- Timeline horizontal inset：`26`
- Bottom spacing：`38`
- Playback Button：`40 / R20`
- Standard icon hit frame：`22`
- Timeline / Volume track thickness：`3 / R2`
- Timeline thumb：`10`
- Volume fixed track width：`100`

### Fullscreen OSC

- Height：`106`
- Radius：`32`
- Timeline horizontal inset：`28`
- Bottom spacing：`30`
- Compact icon hit frame：`21`

### Inspector

- Width：`368`
- Radius：`32`
- Top / Bottom edge：`24`
- Right edge：`28`
- Main content inset：`22`
- Search：`H42 / R21`
- Playlist row：`H58 / R18`
- Playlist row horizontal inset：`18`
- Row gap：`10`
- Search → first row：`20`
- Last row → footer：`24`
- Footer：`H54 / R25`

### 明确不进入 Token 的数值

以下类型不纳入 Geometry Token：

- `x=410`、`x=452` 等由居中产生的构图坐标；
- Timeline Progress 当前宽度等运行时状态值；
- Vector 内部 7.33 / 9.17 等图标光学几何；
- 单次出现、没有独立复用语义的 23 / 29 等视觉间隔。

这避免把“当前截图的位置结果”误当成长期设计系统真值。

## 3. Geometry Primitive

新增 Figma Variable Collection：

- `V3 / Geometry Primitive`
- Mode：`Base`

总计 **38 个 FLOAT Primitive Variable**：

### Spacing — 17

`6 / 10 / 12 / 14 / 18 / 20 / 22 / 24 / 26 / 28 / 30 / 32 / 34 / 38 / 48 / 52 / 60`

命名形式：

```text
spacing/6
spacing/10
...
spacing/60
```

### Size — 13

`3 / 10 / 21 / 22 / 40 / 42 / 50 / 54 / 58 / 100 / 106 / 124 / 368`

命名形式：

```text
size/3
size/10
...
size/368
```

### Radius — 8

`2 / 18 / 20 / 21 / 25 / 27 / 32 / 34`

命名形式：

```text
radius/2
radius/18
...
radius/34
```

Primitive 只保存原始数值，不直接提供给成品设计选择；同时写入 WEB code syntax。

## 4. Geometry Semantic

新增 Figma Variable Collection：

- `V3 / Geometry Semantic`
- Mode：`Base`

总计 **49 个 Semantic Geometry Variable**：

- Spacing：23
- Size：14
- Radius：12

所有 Semantic Variable 通过 alias 指向 Geometry Primitive。

### 4.1 主要 Spacing Role

```text
spacing/control/tight           → 6
spacing/control/adjacent        → 12
spacing/control/group           → 32
spacing/surface/padding-sm      → 18
spacing/surface/padding         → 22
spacing/surface/padding-lg      → 24
spacing/floating/top            → 26
spacing/floating/edge           → 28
spacing/header/content          → 22
spacing/header/content-compact  → 24
spacing/osc/inset               → 26
spacing/osc/inset-compact       → 28
spacing/osc/bottom              → 38
spacing/osc/bottom-compact      → 30
spacing/inspector/edge          → 24
spacing/inspector/right         → 28
spacing/inspector/content       → 22
spacing/inspector/row-inset     → 18
spacing/list/gap                → 10
spacing/list/search-to-first    → 20
spacing/list/row-index          → 14
spacing/list/row-content        → 52
spacing/footer/content          → 18
```

Spacing Semantic 使用 `GAP` scope。

### 4.2 主要 Size Role

```text
size/control/icon               → 22
size/control/icon-compact       → 21
size/control/playback           → 40
size/track/thickness            → 3
size/timeline/thumb             → 10
size/volume/track-width         → 100
size/header/height              → 54
size/header/height-compact      → 50
size/osc/height                 → 124
size/osc/height-compact         → 106
size/inspector/width            → 368
size/inspector/search-height    → 42
size/list/row-height            → 58
size/inspector/footer-height    → 54
```

Size Semantic 使用 `WIDTH_HEIGHT` scope。

### 4.3 主要 Radius Role

```text
radius/window/player            → 32
radius/window/fullscreen        → 34
radius/surface/osc              → 34
radius/surface/osc-compact      → 32
radius/surface/inspector        → 32
radius/header                   → 27
radius/header-compact           → 25
radius/control/playback         → 20
radius/control/search           → 21
radius/list/row                 → 18
radius/track                    → 2
radius/inspector/footer         → 25
```

Radius Semantic 使用 `CORNER_RADIUS` scope。

## 5. 核心框架回刷

已将 Main Player / Fullscreen / Playlist Inspector 中适合直接变量化的 Size / Radius 接入 Semantic Geometry。

共覆盖 **55 个产品节点**，包括：

- Player Window / Video Scene；
- Fullscreen Surface / Video Scene；
- Main / Fullscreen Floating Header；
- Main / Fullscreen / Inspector OSC；
- Playback Button；
- Standard / Compact icon hit frame；
- Timeline Track / Progress / Thumb；
- Volume Track / Value；
- Floating Inspector；
- Search；
- 6 个 Playlist Row；
- Inspector Footer。

最终审计：

```text
Product target nodes:          55
Nodes with Geometry binding:   55
Missing Geometry binding:       0
```

## 6. Spacing 为什么没有强制回刷现有绝对布局

当前三张核心框架仍属于 D0 确认后的探索框架，内部大部分位置采用绝对布局。

D1-04 没有为了追求“Token 覆盖率”而把现有 Main / Fullscreen / Inspector 整体强制转换为 Auto Layout，因为这样会同时改变：

- 已确认构图；
- 中央 Header 的真实居中；
- OSC 的悬浮位置；
- Inspector 与视频画面的叠放关系；
- 图标的光学微调。

本任务采取的边界为：

- 当前框架中的 Size / Radius 立即绑定；
- Spacing 建立正式 Semantic；
- Foundations 文档使用真实 Auto Layout `itemSpacing` 消费 Spacing Variable；
- 从 D2 / D3 / D4 开始建立正式 Player Window / OSC / Inspector 组件时，必须让 Auto Layout 的 Gap / Padding 消费这些 Semantic Spacing。

因此没有为了数字上的“100% 绑定”而破坏已确认视觉。

## 7. Figma Foundations 文档板

在 `01 Foundations` 新增：

```text
D1-04 / Spacing Size Radius
├─ Semantic Spacing
│  ├─ OSC inset
│  ├─ OSC inset compact
│  ├─ Inspector content
│  ├─ Inspector row inset
│  ├─ List row gap
│  ├─ Search → first row
│  ├─ Control group
│  └─ Inspector edge
├─ Semantic Size
│  ├─ Icon hit frame
│  ├─ Icon compact
│  ├─ Playback button
│  ├─ Search height
│  ├─ Header height
│  ├─ Playlist row
│  ├─ OSC height
│  ├─ OSC compact
│  └─ Inspector width
├─ Semantic Radius
│  ├─ List row
│  ├─ Playback
│  ├─ Search pill
│  ├─ Header compact
│  ├─ Header
│  ├─ Inspector
│  ├─ OSC compact
│  └─ OSC
├─ Product Geometry Map
│  ├─ Main OSC
│  ├─ Fullscreen OSC
│  ├─ Inspector
│  └─ Playlist Row
└─ Geometry Rules
```

Spacing 区的 8 个演示不是静态数字图，而是实际 Auto Layout specimen；其 `itemSpacing` 已绑定对应 Semantic Spacing Variable。

## 8. 实施中发现并修复的问题

### 8.1 文档板第一次创建原子回滚

第一次生成 D1-04 Foundations Board 时，代码直接修改新建 Frame 的只读 `width / height` 属性，Figma 抛出：

```text
TypeError: node.height: read-only property on FRAME node
```

Figma 原子回滚，本次失败没有留下半成品。

修复：

- 所有新建 Frame 尺寸统一改为 `resize(width, height)`；
- 重新完整生成文档板；
- 第二次创建成功并通过截图复核。

### 8.2 Geometry binding 最小验证

批量回刷前，先只对 Main OSC 测试：

- `height` → `size/osc/height`
- `cornerRadius` → `radius/surface/osc`

绑定后仍为：

```text
Height: 124
Radius: 34
```

确认没有视觉漂移后才批量推进。

## 9. 最终验证

已执行并通过：

### 视觉回归

- Main Player 高分辨率截图；
- Fullscreen 高分辨率截图；
- Playlist / Inspector 高分辨率截图；
- D1-04 Foundations Board 截图。

所有画面在 Geometry Variable 回刷后保持原构图。

### Geometry 审计

```text
Geometry Primitive:             38
Geometry Semantic:              49
  Spacing:                      23
  Size:                         14
  Radius:                       12

Product target nodes:           55
Geometry-bound product nodes:   55
Missing product bindings:        0

Spacing specimen:                8
GAP-bound specimen:              8

Random spacing 13 / 15 / 17:     0
```

### D1-02 / D1-03 回归

```text
Framework text nodes:           43
V3 styled text nodes:           43
Unstyled text:                   0

Visible Solid Paints:          142
Semantic-bound Solid Paints:   142
Visible unbound Solid Paints:    0
```

说明 D1-04 没有破坏已完成的 Typography 和 Semantic Color 链路。

## 10. 保持不变

本任务没有：

- 改变 Main Player / Fullscreen / Inspector 的已确认构图；
- 将探索框架强制改造成 Auto Layout；
- 建立正式 OSC / Inspector Component；
- 创建 Preferences 具体布局；
- 修改 Glass / Blur / Shadow；
- 修改 Motion / Opacity / Z-order；
- 变量化 Timeline Progress 动态宽度或视频场景装饰坐标。

## 11. 下一任务

`D1-05 建立 Glass / Blur / Shadow`

目标：从当前三张框架的真实背景模糊、透明玻璃、边界与阴影中提取材质层级，建立 Floating Control Glass / Inspector Glass / Popover / HUD / Dialog 等材质语义，并在明亮、暗部和暖色高光背景上验证可读性。