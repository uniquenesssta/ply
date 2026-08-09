# D1-05 建立 Glass / Blur / Shadow — 完成记录

> Task ID：D1-05  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Foundations Page：`01 Foundations`  
> Material 文档板节点：`35:2` — `D1-05 / Glass Blur Shadow`

## 1. 任务目标

D1-05 的目标是把第三版已经成立的 Airy Glass 材质从散落的 Blur / Shadow 数值收敛成唯一设计来源，并明确 Window、Header、OSC、Control、Inspector、Field、Footer、HUD、Dialog 与 Atmosphere 的材质职责。

任务书要求：

- 定义浮动玻璃、强玻璃、Inspector、HUD、Dialog 等材质层级；
- 主链为 `video/background → glass material → content`；
- 优先半透明亮色玻璃与柔和 Blur；
- Shadow 只做空间分离，不制造黑色卡片；
- 在明亮、暗部、暖色高光背景上验证可读性；
- 完成后复杂背景中的玻璃必须稳定可读。

## 2. 真实材质审计

对 Main Player、Fullscreen、Playlist Inspector 的现有可见材质执行逐节点审计。

确认当前设计真实存在的效果层级：

### Window Elevation

- Main Player Window：Drop Shadow `radius 70 / y 24 / plum 13%`
- Player + Inspector：Drop Shadow `radius 70 / y 24 / plum 12%`
- Fullscreen Surface：Drop Shadow `radius 65 / y 22 / plum 11%`

本任务把标准 Window 收敛为 `70 / y24 / 12%`，Fullscreen 保留独立 Immersive 层级 `65 / y22 / 11%`。

### Floating Glass

- Main Header：Background Blur `28` + Shadow `42 / y12 / 12%`
- Fullscreen Header：Background Blur `30` + Shadow `42 / y12 / 12%`
- Main / Inspector OSC：Background Blur `36` + Shadow `42 / y12 / 12%`
- Fullscreen OSC：Background Blur `38` + Shadow `42 / y12 / 12%`
- Playback Control：Background Blur `18` + Shadow `16 / y5 / 8%`
- Inspector：Background Blur `42` + Shadow `42 / y12 / 12%`
- Search Field：Background Blur `18` + Shadow `42 / y12 / 12%`
- Inspector Footer：Background Blur `20` + Shadow `42 / y12 / 12%`

### Atmosphere Layer Blur

- Soft Field：Layer Blur `24`
- Lavender Glow：Layer Blur `90`
- Warm Glow：Layer Blur `95`
- Cyan Glow：Layer Blur `100`

### Playlist Row

普通 Playlist Row 当前没有 Blur / Shadow；Selected Row 也只使用淡紫填充与柔和描边。

本任务明确保持这一规则，没有为了“所有东西都玻璃化”给每行增加 Blur 或 Shadow。

## 3. Effect Primitive

新增 Variable Collection：

```text
V3 / Effect Primitive
Mode: Base
```

共 38 个 FLOAT Variable。

### Blur Primitive

- `blur/18`
- `blur/20`
- `blur/24`
- `blur/28`
- `blur/30`
- `blur/36`
- `blur/38`
- `blur/42`
- `blur/90`
- `blur/95`
- `blur/100`

### Shadow Primitive

Radius：

- `shadow/radius/16`
- `shadow/radius/42`
- `shadow/radius/65`
- `shadow/radius/70`

Y Offset：

- `shadow/y/5`
- `shadow/y/12`
- `shadow/y/22`
- `shadow/y/24`

Spread：

- `shadow/spread/0`

### Material Alpha Primitive

建立框架当前真实使用的 alpha primitive：

- `alpha/03`
- `alpha/07`
- `alpha/08`
- `alpha/11`
- `alpha/12`
- `alpha/13`
- `alpha/22`
- `alpha/26`
- `alpha/28`
- `alpha/32`
- `alpha/34`
- `alpha/36`
- `alpha/38`
- `alpha/42`
- `alpha/48`
- `alpha/52`
- `alpha/66`
- `alpha/100`

## 4. Material Semantic

新增 Variable Collection：

```text
V3 / Material Semantic
Mode: Light Mist
```

共 47 个 Semantic FLOAT Variable。

### Glass Blur

- `blur/glass/header`
- `blur/glass/header-compact`
- `blur/glass/osc`
- `blur/glass/osc-compact`
- `blur/glass/control`
- `blur/glass/inspector`
- `blur/glass/field`
- `blur/glass/footer`
- `blur/glass/hud`
- `blur/glass/dialog`

### Atmosphere Blur

- `blur/atmosphere/field`
- `blur/atmosphere/lavender`
- `blur/atmosphere/warm`
- `blur/atmosphere/cyan`

### Shadow Semantic

Floating：

- `shadow/floating/radius`
- `shadow/floating/y`
- `shadow/floating/spread`
- `shadow/floating/alpha`

Control：

- `shadow/control/radius`
- `shadow/control/y`
- `shadow/control/spread`
- `shadow/control/alpha`

Window：

- `shadow/window/radius`
- `shadow/window/y`
- `shadow/window/spread`
- `shadow/window/alpha`

Immersive Window：

- `shadow/window-immersive/radius`
- `shadow/window-immersive/y`
- `shadow/window-immersive/spread`
- `shadow/window-immersive/alpha`

### Material Alpha Semantic

- `alpha/glass/header-fill` → 52%
- `alpha/glass/header-compact-fill` → 42%
- `alpha/glass/osc-fill` → 34%
- `alpha/glass/osc-compact-fill` → 32%
- `alpha/glass/control-fill` → 48%
- `alpha/glass/inspector-fill` → 38%
- `alpha/glass/field-fill` → 36%
- `alpha/glass/footer-fill` → 26%
- `alpha/glass/row-fill` → 7%
- `alpha/glass/selection-fill` → 66%
- `alpha/glass/border-soft` → 48%
- `alpha/glass/border-strong` → 100%
- `alpha/glass/selection-border` → 28%
- `alpha/atmosphere/vignette` → 3%
- `alpha/atmosphere/lavender` → 34%
- `alpha/atmosphere/cyan` → 22%
- `alpha/atmosphere/warm` → 34%

Material Alpha 没有错误绑定到整个 Frame 的 `opacity`，因为那样会让文字、图标和内容一起透明。当前 Paint alpha 保持在 Fill / Stroke 本身；Semantic Alpha 作为设计与实现真值保留。D1-06 负责交互状态级 Opacity，不重新定义玻璃材质强度。

## 5. 正式 Effect Styles

新增 16 个本地 Effect Style：

### Elevation

- `V3 / Elevation / Window`
- `V3 / Elevation / Window Immersive`

### Glass

- `V3 / Glass / Header`
- `V3 / Glass / Header Compact`
- `V3 / Glass / OSC`
- `V3 / Glass / OSC Compact`
- `V3 / Glass / Control`
- `V3 / Glass / Inspector`
- `V3 / Glass / Field`
- `V3 / Glass / Footer`
- `V3 / Glass / HUD`
- `V3 / Glass / Dialog`

### Atmosphere

- `V3 / Atmosphere / Soft Field`
- `V3 / Atmosphere / Lavender`
- `V3 / Atmosphere / Warm`
- `V3 / Atmosphere / Cyan`

HUD / Dialog 在 D1-05 只冻结独立语义职责：

- HUD 当前复用已验证的 Control effect level；
- Dialog 当前复用已验证的 Inspector effect level；
- 不在没有 D5 真实状态场景的情况下人为创造第三套材质强度。

## 6. 已回刷框架

共 25 个当前真实 Effect 节点接入正式 Effect Style。

### Window

- `4:6` Player Window → `V3 / Elevation / Window`
- `4:52` Fullscreen Surface → `V3 / Elevation / Window Immersive`
- `4:92` Player + Inspector → `V3 / Elevation / Window`

### Glass Surface

- `4:14` Floating Media Header → `V3 / Glass / Header`
- `4:60` Floating Title → `V3 / Glass / Header Compact`
- `4:20` / `4:100` Glass OSC → `V3 / Glass / OSC`
- `4:63` Fullscreen OSC → `V3 / Glass / OSC Compact`
- `4:28` / `4:108` Play Button → `V3 / Glass / Control`
- `4:128` Floating Inspector → `V3 / Glass / Inspector`
- `4:131` Search → `V3 / Glass / Field`
- `4:161` Inspector Footer → `V3 / Glass / Footer`

### Atmosphere

三张框架中的 Soft Field / Lavender / Warm / Cyan 共 12 个节点全部接入对应 Atmosphere Effect Style。

## 7. 同源问题修复

审计发现 Inspector 场景中的 Play Button (`4:108`) 描边 opacity 为 `100%`，而同一视觉职责的 Main Play Button 使用 `48%`。

本任务将 `4:108` 统一恢复为 `48%`，避免同一 Control Material 在不同画面出现不必要的视觉分叉。

## 8. Foundations 文档板

在 `01 Foundations` 新增：

```text
D1-05 / Glass Blur Shadow
Node: 35:2
```

文档板包含：

- 12 个 Material / Elevation Role；
- 4 个 Atmosphere Layer Blur Role；
- Material Alpha Map；
- Bright / Dark / Warm 三种 Background Readability Test；
- Product Material Map；
- D1-05 边界与后续职责说明。

## 9. 背景压力测试

已使用三种设计内模拟背景验证 OSC Material：

### Bright / 花田高光

浅色背景、暖色高光、青色光斑同时存在；白玻璃仍能保留边界，时间码和标题可辨识。

### Dark / 夜景暗部

深色 Ink 背景叠加紫/青亮光；Glass OSC 没有转成黑色实体卡片，内容仍清晰。

### Warm / 夕阳高光

暖橙与紫色高光叠加；OSC 的 Stroke、Blur 和文本仍能维持层次。

## 10. 最终验证

最终结构审计：

```text
Effect Primitive              38
Material Semantic             47
D1-05 Effect Styles           16

Target effect assignments     25 / 25
Assignment issues              0
Relevant effect nodes unstyled 0

Semantic Color              142 / 142
Typography                   43 / 43
Geometry                     55 / 55

Inspector Play stroke        48%
```

已执行并通过：

- Main Player 高分辨率截图回归；
- Fullscreen 高分辨率截图回归；
- Playlist / Inspector 高分辨率截图回归；
- D1-05 Foundations 文档板截图；
- Bright / Dark / Warm 三种材质可读性目视检查；
- 25 个目标 Effect Style assignment 审计；
- 所有真实 Effect 节点是否存在未样式化残留检查；
- D1-02 Semantic Color 回归；
- D1-03 Typography 回归；
- D1-04 Geometry 回归。

## 11. 保持不变

本任务没有：

- 改变第三版整体构图；
- 给 Playlist 普通行增加 Blur / Shadow；
- 创建新的深色播放器模式；
- 创建正式组件库；
- 修改 Timeline / Inspector 业务结构；
- 定义 OSC 的 Hidden / Rest / Active 状态 opacity；
- 定义 Motion duration 或 Z-order。

## 12. 下一任务

`D1-06 建立 Motion / Opacity / Z-order`

目标：定义 OSC、Header、Popover、Inspector、HUD、Dialog 的状态级显隐 opacity、动效时长和层级关系；D1-06 不重新定义 D1-05 已冻结的玻璃材质强度。