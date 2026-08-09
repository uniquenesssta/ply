# D2-03 建立 Floating Header — 完成记录

> Task ID：D2-03  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`02 Player Window` — `48:11`  
> D2-03 Board：`65:2` — `D2-03 / Floating Header Contract`

## 1. 任务目标

把已确认第三版框架中的顶部媒体信息从探索态推进成正式 Floating Header Contract，明确：

- Media Title；
- Technical Metadata；
- Window Actions；
- 长标题优先级；
- 无标题行为；
- 720-width Header 自身 Compact 行为；
- Bright media 下 D2-02 Contrast Support 的真实使用边界。

本任务不提前实现 D2-04 Host 空间契约、D2-05 全局响应式断点、D3 OSC 或 D5 Player State。

## 2. 正式 Header 结构

D2-03 不继续沿用探索稿中“一条 500px 胶囊同时塞 Title / Meta / PLAYING / Close”的结构。

正式结构冻结为：

```text
Floating Header
├─ Media Info Pod
│  ├─ Media Title
│  └─ Technical Metadata
└─ Window Actions Pod
   ├─ Minimize
   ├─ Maximize / Restore
   └─ Close
```

设计理由与职责边界：

- Media Info 与 Window Actions 分离，长标题不会推动或挤压窗口按钮；
- Header 不再承载 `PLAYING / PAUSED / BUFFERING`，播放状态归 D3 / D5；
- Window Actions 独立存在，标题缺失时仍保持窗口控制可用；
- Header 仍然是附着在视频上的轻量浮层，不形成传统 full-width title bar。

## 3. Header 优先级

正式优先级：

```text
Window Actions
    ↓ highest
Media Title
    ↓
Technical Metadata
    ↓ lowest
```

行为冻结：

1. Window Actions 永远不能因为长标题移动或消失；
2. Media Title 保持单行；
3. 长标题使用 `ENDING` ellipsis；
4. Metadata 在空间不足时先于 Title 隐藏；
5. Title 缺失时 Media Info Pod 整体不渲染，禁止保留空白玻璃胶囊。

## 4. Geometry Token 补强

D2-03 只新增当前 Header Contract 实际需要的 5 个 Geometry Primitive 和 5 个 Semantic Geometry。

新增 Primitive：

- `size/110`
- `size/360`
- `size/420`
- `spacing/2`
- `spacing/16`

新增 Semantic：

- `size/header/info-width = 420`
- `size/header/info-width-compact = 360`
- `size/header/actions-width = 110`
- `spacing/header/title-meta = 2`
- `spacing/header/actions-inset = 16`

因此 Geometry 数量变化：

```text
V3 / Geometry Primitive
41 → 46

V3 / Geometry Semantic
53 → 58
```

继续复用 D1 已有：

- `size/header/height = 54`
- `size/header/height-compact = 50`
- `radius/header = 27`
- `radius/header-compact = 25`
- `spacing/header/content = 22`
- `spacing/control/tight = 6`
- `size/control/icon = 22`
- `size/control/icon-compact = 21`
- `opacity/control/idle = 72%`
- `z/floating-header = 30`
- `V3 / Glass / Header`
- `V3 / Glass / Header Compact`

没有建立第二套 Header 材质或字体系统。

## 5. Figma 设计场景

在 `02 Player Window` 新增 `D2-03 / Floating Header Contract`（`65:2`），包含：

### 5.1 Standard Window

- Reference Window：`1320×700`；
- Media Info：`420×54 / R27`；
- Window Actions：`110×54 / R27`；
- Top safe edge：`26`；
- Media Info 与 Window Actions 为两个独立 Pod；
- 不显示 PLAYING / PAUSED。

### 5.2 Long Title + Bright Media

- Media Info 尺寸不变；
- 长标题 TextNode 实际 `textTruncation = ENDING`；
- Title content width：`376`；
- Window Actions 位置保持不变；
- Bright media 使用 D2-02 `overlay/contrast-support`，但只放在两个 Header Pod 后方。

最终 Local Support：

```text
Media Info support
460×82 / opacity 16% / z25

Window Actions support
140×82 / opacity 16% / z25
```

默认场景仍为 OFF，不把 contrast support 作为视觉装饰。

### 5.3 No Title

- `Media Info Pod = 0`；
- 只保留 Window Actions；
- 不显示空白 420px Glass Pod。

### 5.4 720-width Header Stress

该场景只验证 Header 自身 Compact 行为，不冻结 D2-05 全局 breakpoint。

- Media Info Compact：`360×50 / R25`；
- Window Actions Compact：`110×50 / R25`；
- Metadata nodes：`0`；
- Title 仍使用 ENDING ellipsis；
- Window Actions 保留。

## 6. Window Actions

正式展示三个窗口动作：

- Minimize；
- Maximize / Restore；
- Close。

动作视觉：

- Standard visual slot：`22×22`；
- Compact visual slot：`21×21`；
- action gap：`6`；
- actions inset：`16`；
- idle opacity：`72%`，绑定 `opacity/control/idle`；
- icon color：绑定 `icon/secondary`。

D2-03 只定义结构和视觉职责；平台相关的具体窗口行为和 hover/pressed 控件实现后续由正式 Component / Qt Quick 实现承接。

## 7. 实施中发现并修复的问题

### 7.1 首次 Contract 创建脚本语法错误

第一次创建时 synthetic media gradient helper 的 `map()` 少一个右括号，Figma 原子回滚。

结果：

- 没有留下半成品；
- 修正语法后重新完整创建 Board。

### 7.2 Close glyph 光学错误

第一轮截图中，Close 使用两条 Line 直接旋转，因旋转基点导致视觉更接近尖括号，不是标准 `×`。

已修复：

- 4 / 4 个 Close Action 全部替换成正式 X path；
- stroke 重新绑定 `icon/secondary`；
- idle opacity 保持 72%。

### 7.3 Contrast Support 范围过大

第一轮 Bright-media 示例使用整条顶部 16% support band，虽然透明度正确，但不符合 D2-02 “local only” 契约。

已修复为：

- `Contrast Support / Media Info`；
- `Contrast Support / Window Actions`；
- 两个局部区域均为 `16% / z25`；
- 不再横跨整个 Video Viewport。

## 8. 最终验证

最终结构审计：

```text
Standard Info Pods       2
Compact Info Pods        1
Standard Action Pods     3
Compact Action Pods      1

Actions per Pod          3
Close glyph fixed        4 / 4
Close idle opacity       72% + Semantic binding

Long title truncation    ENDING
Long title width         376
No-title Info Pods       0
Narrow Metadata nodes    0

Local contrast support
Media Info               16% / z25
Window Actions           16% / z25
```

D2-03 Board 可见 Solid Paint 审计：

```text
Visible Solid Paint      85
Semantic-bound           85
Unbound                   0
```

D1 回归：

```text
Semantic Color    142 / 142
Typography         43 / 43
Geometry           55 / 55
Effect             25 / 25
```

D2-01 Standard Window 仍保持：

```text
1320×700
R32
clipsContent = true
V3 / Elevation / Window
border = 70%
```

D2-02 Contract 保持存在且未修改。

视觉截图验证通过：

- Standard Header；
- Long Title + Bright Media；
- No Title；
- 720-width Compact；
- 第二轮修复后的 Close glyph 与 local contrast support。

## 9. 保持不变

本任务没有：

- 修改 `00 Framework Preview`；
- 修改 D2-01 Window Contract；
- 修改 D2-02 Viewport Fit / Letterbox / Audio / Empty 规则；
- 创建正式 Figma Component Set；
- 创建 Header hover / pressed / window drag 行为；
- 定义 D2-05 全局响应式断点；
- 把 Playback State 重新塞进 Header。

## 10. 下一任务

`D2-04 建立 Host 空间契约`

目标：正式定义 OSC Host / Inspector Host / Overlay Host 在 Player Window 内的安全区、碰撞和层级，让后续 D3 / D4 / D5 可以挂内容而不再临时挪位置。