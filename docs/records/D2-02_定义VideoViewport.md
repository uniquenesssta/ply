# D2-02 定义 Video Viewport — 完成记录

> Task ID：D2-02  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`02 Player Window` — `48:11`  
> D2-02 Board：`57:2` — `D2-02 / Video Viewport Contract`

## 1. 任务目标

把 D2-01 预留的 `Viewport Host / D2-02` 推进成正式 Video Viewport Contract，明确：

- 视频默认 Fit 行为；
- Letterbox / Pillarbox 所有权；
- 16:9 / 21:9 / 4:3 / 9:16 的比例适配；
- Audio-only Viewport 背景；
- Empty Viewport 背景；
- Contrast Support 的启用边界、透明度和层级。

本任务不提前实现 D2-03 Floating Header、D3 OSC、D4 Inspector，也不定义 D5 的 Empty CTA / Error / Loading 业务反馈。

## 2. 正式 Viewport Fit 契约

默认视频策略冻结为：

```text
media aspect
  ↓
Fit
  ↓
preserve aspect
  ↓
centered
  ↓
no stretch
  ↓
no crop by default
```

播放器不允许为了填满窗口默认拉伸或裁切媒体。以后如果产品加入 crop / zoom / aspect override，必须作为显式视频功能，不修改 D2-02 默认 Fit 真值。

D2-02 文档板以 `1320×700` Reference Viewport 的 50% 缩放，即 `660×350`，验证四类比例。

### 16:9

```text
660 × 350 host
media = 622.2222 × 350
x = 18.8889
pillarbox ≈ 18.8889 each side
```

映射到 `1320×700` 实际 Reference Viewport：

```text
media ≈ 1244.4444 × 700
pillarbox ≈ 37.7778 each side
```

### 21:9

```text
660 × 350 host
media = 660 × 282.8571
y = 33.5714
letterbox ≈ 33.5714 top/bottom
```

映射到实际 Reference Viewport：

```text
media ≈ 1320 × 565.7143
letterbox ≈ 67.1429 top/bottom
```

### 4:3

```text
660 × 350 host
media = 466.6667 × 350
x = 96.6667
pillarbox ≈ 96.6667 each side
```

映射到实际 Reference Viewport：

```text
media ≈ 933.3333 × 700
pillarbox ≈ 193.3333 each side
```

### 9:16 Vertical

```text
660 × 350 host
media = 196.875 × 350
x = 231.5625
pillarbox = 231.5625 each side
```

映射到实际 Reference Viewport：

```text
media = 393.75 × 700
pillarbox = 463.125 each side
```

最终 Geometry Audit 的最大误差小于 `0.000011 px`，仅来自 Figma FLOAT 精度。

## 3. Letterbox / Pillarbox 所有权

新增 Semantic Color：

- `surface/letterbox` → `color/neutral/ink-950`

正式职责：

```text
Video Viewport owns
├─ media fit
├─ letterbox
└─ pillarbox
```

`Window Surface` 不拥有 Bars；D3 OSC / D4 Inspector 也不能通过改变媒体尺寸来制造自己的安全区。

Letterbox 使用深 Ink 只代表媒体适配区域，不等于播放器整体回到深色 UI。

## 4. Audio-only Viewport

新增：

- `surface/audio` → `color/neutral/mist-100`

Audio-only 不创建假视频内容，使用轻雾背景，只保留非常弱的 Atmosphere：

- Lavender node opacity：`16%`
- Cyan node opacity：`10%`

文档板中的音频波形只作为 Viewport 背景职责示意，不代表 D2-02 已定义完整音频元数据 UI。

## 5. Empty Viewport

新增：

- `surface/empty` → `color/neutral/mist-50`

Empty 只定义安静的 Viewport 背景。

最终 Empty Lavender Atmosphere node opacity 收敛到 `4.5%`，避免无媒体时装饰本身成为视觉主体。

明确：

- D2-02 不实现 Open File CTA；
- 不实现 Loading；
- 不实现 Error；
- 不实现 Resume；
- 上述播放状态全部留给 D5。

## 6. Contrast Support

新增 Semantic Color：

- `overlay/contrast-support` → `color/neutral/ink-950`

新增 Effect / Material Token：

- `alpha/16`
- `alpha/viewport/contrast-support` → `alpha/16`

新增 Interaction Z Token：

- `z/25`
- `z/contrast-support` → `z/25`

正式规则：

```text
Contrast Support = OFF by default
```

只有未来 Header / OSC 在具体媒体画面上达不到可读性时，才允许在对应控件附近局部开启。

D2-02 Test：

- `SUPPORT OFF`：Contrast Support node count = `0`
- `SUPPORT ON`：只在未来 Header / OSC 附近存在 Top / Bottom 两个 local support region
- node opacity = `16%`
- shared z-role = `z/contrast-support`

层级契约更新为：

```text
video              z0
atmosphere         z10
media content      z20
contrast support   z25
floating controls  z30+
```

因此 Contrast Support 只能压媒体，不能压 Header / OSC / Inspector。

## 7. Figma 结构

新增：

```text
02 Player Window
└─ D2-02 / Video Viewport Contract  [57:2]
   ├─ Contract cards
   ├─ 16:9 Video
   ├─ 21:9 Video
   ├─ 4:3 Video
   ├─ 9:16 Vertical
   ├─ Audio Only
   ├─ Empty Media
   ├─ Contrast Support OFF
   ├─ Contrast Support ON
   └─ Viewport Ownership Contract
```

Synthetic Video Content 使用 6 个 Gradient Paint 表达“任意媒体像素”。这些 Gradient **故意不绑定产品主题 Token**，因为真实视频像素不属于应用 Design System。

所有应用自身拥有的可见 Solid Paint 均消费 Semantic Color。

## 8. 实施中发现并修复的问题

### 8.1 Semantic Paint binding 再次重置 Paint opacity

第一轮 D2-02 Board 截图发现：

- Empty Lavender Glow 看起来像高饱和紫色主体；
- Contrast Support ON 变成近乎实体黑色带；
- Audio Glow、测试 pill 等半透明材质也偏重。

只读检查确认：`setBoundVariableForPaint()` 在同一创建调用中再次把目标 Paint opacity 写回 `1.0`。

例如：

```text
Empty Glow target = 4.5% / 6% class
actual first pass = 100%

Contrast Support target = 16%
actual first pass = 100%
```

修复：

- 保留 Semantic Color binding；
- 第二轮独立 mutation 恢复 44 个相关 Paint / Node opacity；
- Contrast Support Top / Bottom 改为 node opacity `16%`；
- Empty Glow 收敛为 node opacity `4.5%`；
- Audio Lavender / Cyan 分别恢复为 `16% / 10%`；
- 重新截图后通过。

## 9. 最终验证

### Aspect Fit

四类媒体 Geometry 均按公式居中 Fit。

Figma 与理论值最大差值：

```text
< 0.000011 px
```

### D2-02 Paint Audit

```text
Visible Solid Paints:          126
Semantic-bound Visible Solids: 126
Visible Unbound Solids:          0
Synthetic Media Gradients:       6
```

### Contrast Support

```text
OFF support nodes: 0
ON Top opacity:    16%
ON Bottom opacity: 16%
z-role:            z/contrast-support
```

### Fallback Background

```text
Audio role:        audio
Empty role:        empty
Audio Lavender:    16%
Audio Cyan:        10%
Empty Lavender:     4.5%
```

### D1 Regression

```text
Semantic Color: 142 / 142
Typography:      43 / 43
Geometry:        55 / 55
Effect:          25 / 25
```

### D2-01 Regression

Standard Player Window 保持：

```text
1320 × 700
R32
clipsContent = true
V3 / Elevation / Window
border = 70%
```

## 10. 明确未实现

D2-02 不包含：

- crop / fill / stretch 模式；
- zoom / pan；
- 手动 aspect ratio override；
- rotate；
- HDR / SDR tone mapping；
- 视频色彩管理；
- 真实缩略图或真实视频帧；
- Floating Header；
- OSC；
- Inspector；
- Empty / Error / Loading 的业务反馈组件。

这些能力不得因为 Viewport 已完成就在后续实现中被默认视为已设计。

## 11. 下一任务

`D2-03 建立 Floating Header`

目标：在已经稳定的 Window Surface + Video Viewport 之上设计媒体标题、轻量元数据与窗口动作；验证长标题、无标题和窄窗口，同时保证 Header 不改变媒体 Fit，也不压迫视频主体。