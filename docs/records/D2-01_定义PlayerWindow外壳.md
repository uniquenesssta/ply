# D2-01 定义 Player Window 外壳 — 完成记录

> Task ID：D2-01  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`02 Player Window` — `48:11`  
> D2-01 Board：`50:2` — `D2-01 / Player Window Shell Contract`

## 1. 任务目标

把已确认第三版 Main Player 中的窗口外壳从探索构图推进成正式 Player Window Contract，只冻结：

- Window Surface 边界；
- Windowed / Maximized 圆角规则；
- Window Elevation；
- 外壳玻璃边界；
- `clipsContent`；
- minimum safe edge；
- Window Surface 与后续 Video / Header / OSC / Inspector 的职责边界。

本任务不提前实现 D2-02 Video Viewport、D2-03 Floating Header、D3 OSC 或 D4 Inspector。

## 2. 实施前真实审计

从已确认 `Framework / Main Player` 读取当前真实外壳：

```text
Player Window
1320 × 700
R32
clipsContent = true
border = border/glass @ 70%
effect = V3 / Elevation / Window
```

当前 Window 本体无实体 Fill；`Video Scene` 贴满整个窗口，因此正式结论为：

> Window Surface 只拥有边界、Elevation 和 Clipping，不再添加一层厚背景。真实媒体内容由 D2-02 Video Viewport 填充。

当前 Header 顶边距为 26，OSC 底边距为 38。这两个数值分别属于 Header/OSC Host 布局，不被误写成整个窗口统一 padding。

## 3. D2-01 新增 Geometry Contract Token

D2-01 首次出现了正式 Window reference size、最大化 R0 与 safe-min，因此在 D1 Geometry 基础上补充真实 Token。

### Geometry Primitive

新增：

- `radius/0` = `0`
- `size/700` = `700`
- `size/1320` = `1320`

`V3 / Geometry Primitive`：`38 → 41`

### Geometry Semantic

新增：

- `spacing/window/safe-min` → `spacing/26`
- `size/window/reference-width` → `size/1320`
- `size/window/reference-height` → `size/700`
- `radius/window/maximized` → `radius/0`

`V3 / Geometry Semantic`：`49 → 53`

这些 Token 表达正式 Window Contract；没有把居中坐标或当前文档画布尺寸 Token 化。

## 4. Window Border Material 补强

D1-05 已有 `48%` soft border 和 `100%` strong border，但当前 Player Window 的真实玻璃边界是 `70%`。

为避免用近似值替代，新增：

### Effect Primitive

- `alpha/70` = `0.70`

`V3 / Effect Primitive`：`38 → 39`

### Material Semantic

- `alpha/window/border` → `alpha/70`

`V3 / Material Semantic`：`47 → 48`

Figma 当前不能把 Variable 直接绑定 Paint opacity，因此产品节点继续保留实际 `70%` Paint alpha；该 Semantic 作为设计与 Qt Quick/QML 实现真值。

## 5. 正式 Window Surface Contract

### 5.1 Standard Window

Figma：`50:22` — `Window Surface / Standard`

```text
reference width  = 1320
reference height = 700
radius           = 32
border           = border/glass @ 70%
elevation        = V3 / Elevation / Window
clipsContent     = true
mode             = windowed
```

真实 Variable binding：

- width → `size/window/reference-width`
- height → `size/window/reference-height`
- four corners → `radius/window/player`
- stroke color → `border/glass`

Standard Window 的宽高是当前视觉基准 reference size，不等于未来唯一允许窗口尺寸；D2-05 才负责响应式宽度规则。

### 5.2 Maximized Window

Figma：`50:34` — `Window Surface / Maximized`

```text
size             = available window area
radius           = 0
outer border     = none
outer elevation  = none
clipsContent     = true
mode             = maximized
```

四角真实绑定：

- `radius/window/maximized`

正式规则：

> Maximized 仍是 Windowed 模式，不等于 Fullscreen；只移除位于屏幕外缘无意义的 Window radius / outer elevation，内部 Video / Header / OSC 仍沿用同一产品系统。

### 5.3 Background Contrast

Figma：

- Test Stage：`50:45`
- Window Surface：`50:48`

同一个 Standard Window Shell 放置到：

- 深色桌面背景；
- 暖色桌面背景。

验证 `70%` glass border + Window Elevation 不依赖浅灰设计画布才能成立。

## 6. Minimum Safe Edge

新增 `spacing/window/safe-min = 26`。

D2-01 中建立真实 Auto Layout specimen：`50:17`，四边：

```text
paddingLeft   = 26
paddingRight  = 26
paddingTop    = 26
paddingBottom = 26
```

四项均真实绑定同一个 `spacing/window/safe-min` Variable。

正式定义：

> safe-min 是任何 Floating Host 的最小边缘保护，不是 Window Surface 的统一 content padding。

因此：

- Header 可以使用更具体的 `spacing/floating/top = 26`；
- OSC 可以继续使用 `spacing/osc/bottom = 38`；
- Inspector / Overlay 后续可有自己的 edge contract；
- 但所有 Host 不得无规则贴到窗口裁切边缘。

## 7. 职责边界

D2-01 正式冻结：

```text
Window Surface owns
├─ radius
├─ border
├─ elevation
├─ clipping
└─ safe minimum contract

D2-02 owns Video Viewport / media fit
D2-03 owns Floating Header
D3 owns OSC
D4 owns Inspector
```

D2-01 没有创建正式 Control Component，也没有把现有 Main Player 的 Header/OSC 复制进新的 Window System。

## 8. Figma 产物

新增 Page：

- `02 Player Window` — `48:11`

新增主文档板：

- `D2-01 / Player Window Shell Contract` — `50:2`

主要验证节点：

- Standard：`50:22`
- Maximized：`50:34`
- Background Contrast Stage：`50:45`
- Contrast Window：`50:48`
- Safe Area Token Specimen：`50:17`
- Ownership Contract：`50:57`

Viewport 内部目前只存在 `Viewport Host / D2-02` 占位和低透明度柔光，用于看清外壳边界；不代表 D2-02 的媒体比例、Letterbox 或纯音频设计已经完成。

## 9. 实施中发现并修复的问题

### 9.1 Semantic Paint binding 再次重置 Paint opacity

第一次生成 D2-01 后截图发现：

- Lavender / Warm 占位光斑变成高饱和实心圆；
- Window Border 从目标 70% 变成 100%；
- Safe Area / 文档卡片边界透明度同时被放大。

根因与 D1-02 相同：

`setBoundVariableForPaint()` 绑定 Color Variable 时把新 Paint opacity 重置为 `1.0`。

已修复并保持所有 Color Variable binding，恢复 22 个 Fill / Stroke 的目标透明度，包括：

- Window border `70%`；
- documentation border `48%`；
- safe annotation `30%`；
- safe chip `78%`；
- Lavender placeholder `16%`；
- Warm placeholder `12%`；
- Viewport owner label `74%`；
- warm contrast background `64%`。

第二轮截图后恢复第三版 Airy Glass 的低饱和柔和表现。

## 10. 最终验证

已执行：

- D2-01 全文档板截图；
- Standard Window 独立截图；
- Maximized Window 独立截图；
- Background Contrast Stage 独立截图；
- Standard / Maximized 结构属性审计；
- safe-min Variable binding 审计；
- D2-01 产品节点 Semantic Color 审计；
- D1 Color / Typography / Geometry / Effect 回归审计。

最终结果：

```text
Standard
  size                  1320 × 700
  radius                32
  clipping              true
  border alpha          70%
  effect                V3 / Elevation / Window

Maximized
  size                  1440 × 900 test canvas / available-size rule
  radius                0
  clipping              true
  outer border          none
  outer elevation       none

Safe Area specimen
  26 / 26 / 26 / 26
  Variable-bound        4 / 4

D2-01 Product Solid Paints
  Semantic-bound        26 / 26
  Unbound               0

D1 regression
  Semantic Color        142 / 142
  Typography             43 / 43
  Geometry               55 / 55
  Effect                 25 / 25
```

## 11. 保持不变 / 未提前实施

本任务没有：

- 修改 `00 Framework Preview` 三张已确认框架；
- 决定视频 aspect fit / letterbox；
- 设计纯音频或空媒体 Viewport；
- 重做 Floating Header；
- 重做 OSC；
- 创建 Inspector Host 内容；
- 建立响应式窗口断点；
- 把 Maximized 当成 Fullscreen；
- 创建正式 Component Library。

## 12. 下一任务

`D2-02 定义 Video Viewport`

下一步将把当前 `Viewport Host / D2-02` 占位替换成正式 Video Viewport，验证：

- 16:9；
- 21:9；
- 4:3；
- 竖视频；
- 纯音频；
- Letterbox / Background；
- 只有为控件可读性服务时才允许出现 Contrast Support Layer。
