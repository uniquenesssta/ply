# D1-03 建立 Typography System — 完成记录

> Task ID：D1-03  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Foundations Page：`01 Foundations`  
> Typography 文档板节点：`21:2` — `D1-03 / Typography System`

## 1. 任务基线

D1 阶段任务书对 D1-03 的要求为：

- 定义 `Display / Title / Body / Label / Caption / Timecode / Keycap` 等层级；
- 主链为 `type scale → semantic styles → product`；
- 大标题允许较强 Grotesk 气质；
- 播放器内控件文字保持克制；
- Timecode 需要数字宽度稳定；
- 必须验证长媒体标题、中英混排、`00:00:00` 时间码；
- 完成判断：文字层级不依赖随意字号差异维持。

本次严格从已确认 Main Player / Fullscreen / Playlist Inspector 三张框架的实际文本使用情况反推 Typography，而不是先套通用 type scale。

## 2. 真实框架审计

共审计三个已确认框架：

- `Framework / Main Player` — `4:2`
- `Framework / Fullscreen` — `4:48`
- `Framework / Playlist Inspector` — `4:88`

实际文本节点：`43`

D1-03 开始前：

- 43 个文本节点均未绑定本地 Text Style；
- 当前真实字体集中为 `Inter` 与 `Noto Sans SC`；
- 真实字号集中为 `32 / 22 / 14 / 13 / 12 / 11 / 10`；
- 所有现有文本 letter spacing 均为 `0%`；
- 所有现有文本 line-height 均为 Figma `Auto`。

原框架职责分布：

- `Inter`：英文说明标题、序号、状态、时间码、计数、技术 metadata；
- `Noto Sans SC`：中文说明、媒体标题、Inspector 标题、搜索文案、列表标题与提示。

## 3. 字体职责决策

第三版 Typography 最终固定三套字体职责：

### Inter

用于：

- 英文 UI；
- 状态；
- Index；
- Count；
- Technical Metadata；
- Figma Docs 层英文标题。

### Noto Sans SC

用于：

- 中文正文；
- 多语言媒体标题；
- Inspector 标题；
- Playlist title；
- Search / Hint / Meta。

选择原因：媒体标题和应用正文必须稳定支持中文及中英混排，不能为了追求 Grotesk 外观把中文依赖不可控 fallback。

### Geist Mono

**仅用于 Timecode。**

原因：D1 任务书明确要求时间码数字稳定。原 Inter 比例数字下相同 8 字符时间码宽度存在差异；切换 `Geist Mono` 后，M / S / XS 三档时间码在真实框架中的等长字符串宽度差全部变为 `0 px`。

Geist Mono 不进入标题、正文、Label 或普通 Metadata，避免播放器变成硬核科技风。

## 4. Typography Primitive Variables

新增 Variable Collection：

- `V3 / Type Primitive`
- Mode：`Base`

新增 14 个 Primitive Variable：

### Font Family

- `font/family/latin` = `Inter`
- `font/family/cjk` = `Noto Sans SC`
- `font/family/mono` = `Geist Mono`

Scopes：`FONT_FAMILY`

### Font Style

- `font/style/regular` = `Regular`
- `font/style/medium` = `Medium`
- `font/style/semibold` = `Semi Bold`

Scopes：`FONT_STYLE`

### Font Size

- `font/size/10` = `10`
- `font/size/11` = `11`
- `font/size/12` = `12`
- `font/size/13` = `13`
- `font/size/14` = `14`
- `font/size/22` = `22`
- `font/size/32` = `32`

Scopes：`FONT_SIZE`

### Tracking

- `font/tracking/0` = `0`

Scope：`LETTER_SPACING`

所有变量均设置稳定 WEB code syntax。

## 5. 正式 Text Styles

共建立 17 个本地 V3 Text Style。

### Figma Documentation Only

- `V3 / Docs / Display` — Inter Semi Bold 32
- `V3 / Docs / Eyebrow` — Inter Semi Bold 12
- `V3 / Docs / Supporting` — Noto Sans SC Regular 14

`Docs/*` 只用于 Figma 说明层，不作为播放器产品实现样式。

### Product Titles

- `V3 / Title / Inspector` — Noto Sans SC Medium 22
- `V3 / Title / Media` — Noto Sans SC Medium 14
- `V3 / Title / Media Compact` — Noto Sans SC Medium 13

### Product Body / Metadata

- `V3 / Body / Control` — Noto Sans SC Regular 12
- `V3 / Body / Meta` — Noto Sans SC Regular 11
- `V3 / Metadata / Technical` — Inter Regular 10

### Labels / Keycap

- `V3 / Label / Micro Strong` — Inter Semi Bold 10
- `V3 / Label / Compact Strong` — Inter Semi Bold 11
- `V3 / Keycap / Compact` — Inter Medium 11

### Timecode

- `V3 / Timecode / M Primary` — Geist Mono Medium 12
- `V3 / Timecode / M Secondary` — Geist Mono Regular 12
- `V3 / Timecode / S Primary` — Geist Mono Medium 11
- `V3 / Timecode / S Secondary` — Geist Mono Regular 11
- `V3 / Timecode / XS` — Geist Mono Regular 10

所有 Style 当前保持：

- `letter-spacing: 0%`
- `line-height: Auto`

D1-03 没有强行将 Auto line-height 改成任意固定数值，因为三张核心框架已经确认；在没有真实 Qt 字体度量前硬改固定行高会制造无必要的布局漂移。若最终 Handoff 需要固定 line-height，应基于实际字体度量和 Qt Quick 渲染验证补充。

## 6. 三张核心框架回刷

已将全部 `43` 个文本节点映射到正式 V3 Text Style。

主要映射包括：

- Framework title / eyebrow / supporting copy → `Docs/*`
- Floating header media title → `Title / Media`
- Fullscreen / Playlist compact media title → `Title / Media Compact`
- Inspector title → `Title / Inspector`
- Search → `Body / Control`
- Inspector metadata / footer hint → `Body / Meta`
- Status / Index → `Label / Micro Strong`
- Count → `Label / Compact Strong`
- ESC → `Keycap / Compact`
- Main OSC timecodes → `Timecode / M *`
- Fullscreen timecodes → `Timecode / S *`
- Playlist duration → `Timecode / XS`

除 Timecode 字体为满足 numeric stability 从 Inter 改为 Geist Mono 外，其余字体、字号和视觉层级均保持原框架基准。

## 7. Typography Foundations 文档板

在 `01 Foundations` 新增：

```text
D1-03 / Typography System
├─ Font Responsibilities
│  ├─ Inter
│  ├─ Noto Sans SC
│  └─ Geist Mono
├─ Semantic Text Styles
│  └─ 17 个真实 Style specimen
├─ Stress Tests
│  ├─ Long Media Title / Mixed Language
│  └─ Equal-width Timecode
└─ Rule Note
```

文档板直接消费正式 Text Style 与 Semantic Color，不另建一套示意样式。

## 8. Stress Tests

### 长媒体标题 / 中英混排

验证样例：

`银翼杀手 2049 · Blade Runner 2049 — The Final Cut`

同时验证：

`字幕轨道 · Japanese / 한국어 / English Commentary`

截图检查无裁切、无异常 fallback、层级稳定。

### Timecode numeric stability

文档板验证：

- `00:00:00`
- `11:11:11`
- `22:22:22`
- `88:88:88`

真实框架最终宽度审计：

```text
M  / 12 px: width delta = 0 px
S  / 11 px: width delta = 0 px
XS / 10 px: width delta = 0 px
```

实际节点示例：

- Main Current / Total：均 `58 px`
- Fullscreen Current / Total：均 `53 px`
- Playlist 6 个 duration：均 `48 px`

## 9. 实施中发现并修复的问题

第一次创建 Typography 文档板时，辅助函数在 TextNode 尚未写入字符前调用：

`setRangeTextStyleIdAsync(0, 0, styleId)`

Figma 拒绝空范围并触发整次原子回滚：

`Empty range selected. 'end' must be greater than 'start'`

该失败没有留下半成品。

已修复创建顺序：

`load font → set font baseline → write characters → apply Text Style → bind Semantic Color`

随后完整重建并截图验证通过。

## 10. 最终验证

已执行：

- Main Player 高分辨率截图回归；
- Fullscreen 高分辨率截图回归；
- Playlist / Inspector 高分辨率截图回归；
- Typography System 文档板截图回归；
- 43 个 Framework Text Node Text Style 覆盖率审计；
- Timecode width delta 审计；
- D1-02 Semantic Color 回归审计。

最终结果：

```text
Framework Text Nodes:            43
V3 Styled Text Nodes:            43
Unstyled:                         0
Non-V3 / Mixed Style:             0
Local V3 Text Styles:            17
Type Primitive Variables:        14

Timecode width delta M:          0 px
Timecode width delta S:          0 px
Timecode width delta XS:         0 px

Visible Solid Paints:           142
Semantic-bound Visible Solids: 142
Visible Unbound Solid Paints:     0
Hidden Solid Paints ignored:     23
Gradient Paints deferred:          6
```

D1-03 没有破坏 D1-02 Semantic Color 成果。

## 11. 保持不变

本任务没有：

- 修改第三版 Airy Glass 配色；
- 改变 Main / Fullscreen / Inspector 的主构图；
- 建立 Spacing / Size / Radius Token；
- 建立 Glass / Blur / Shadow Token；
- 建立 Motion / Opacity / Z-order；
- 创建正式播放器组件；
- 引入外部 UI Kit。

## 12. 下一任务

`D1-04 建立 Spacing / Size / Radius`

目标：从三张已确认框架的实际位置、间距、控件尺寸和圆角中抽取数值，区分 Window breathing space、Surface padding、List row、Control gap、交互 hit target 与 Pill/Surface radius，并让布局从散落坐标逐步升级为可复用的几何 Token。