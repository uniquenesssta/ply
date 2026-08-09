# D3-02 Timeline 基础几何 — 完成记录

> Task ID：D3-02  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-02 Board：`95:8` — `D3-02 / Timeline Basic Geometry`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-02 定义，建立 Timeline 的静态基础几何：

- Track；
- Buffered Range；
- Progress；
- Thumb；
- Interaction Hit Target；
- 0% / 50% / 100%；
- unknown duration；
- non-seekable；
- 不同 Timeline Lane 宽度下的几何稳定性。

本任务只冻结静态几何，不提前实现 D3-03 的 Hover Preview、Scrub、Pending Seek、Chapter Marker 或 Seek Preview。

## 2. 正式 Timeline 几何

Timeline 的视觉轨道与交互命中区域正式分离：

```text
Visual Track     = 3px
Hit Target       = 16px
Thumb            = 10px
Track Radius     = 2px
```

核心端点公式：

```text
thumbDiameter = 10
trackX         = thumbDiameter / 2 = 5
trackWidth     = componentWidth - thumbDiameter
thumbX         = positionRatio × trackWidth
```

因此 Thumb 本身始终完整落在 Timeline Component 内：

```text
0%   → thumbX = 0
50%  → thumbX = trackWidth / 2
100% → thumbX = trackWidth
```

不需要通过 clipping 或额外 endpoint padding 修补 Thumb 溢出。

## 3. 新增 / 补强 Token

### Geometry Primitive

- `size/16 = 16`

### Geometry Semantic

- `size/timeline/hit-height = 16`
- `spacing/timeline/control-offset = 12`
- `spacing/timeline/control-offset-compact = 10`

### Responsive Semantic

- `timeline/control-offset`
  - Narrow → `10`
  - Standard → `12`
  - Wide → `12`

### Color Semantic

- `control/buffered` → `color/neutral/ink-600`
- `control/thumb-border` → `color/violet/400`

继续复用既有 Token：

- `size/track/thickness = 3`
- `size/timeline/thumb = 10`
- `radius/track = 2`
- `control/track`
- `control/progress`
- `control/thumb`
- `control/disabled`

没有重新建立第二套 Timeline 尺寸体系。

## 4. Static State Matrix

D3-02 Board 中已建立：

### 0%

- Progress width = `0`
- Buffered Range 可存在；
- Thumb left = `0`；
- Thumb 完整位于组件内部。

### 50%

- `252px` 样本：Visual Track width = `242px`
- Progress width = `121px`
- Thumb left = `121px`

### 100%

- `252px` 样本：Visual Track width = `242px`
- Thumb left = `242px`
- Thumb right = `252px`
- 右端误差 = `0`

### Unknown Duration

- 保留同一 3px Base Track；
- Duration = `--:--`；
- 不显示 Progress；
- 不显示 Buffered Range；
- 不显示 Thumb；
- 不创建“未知时长专用轨道”。

### Non-seekable

- 保留同一 3px Base Track；
- 使用 `control/disabled`；
- 不显示 Progress / Buffer / Thumb；
- Hit Target 仍保留几何契约，但交互禁用；
- 不创建第二套 non-seekable 几何。

## 5. Hit Target 规则

产品实际视觉中 Hit Target 完全透明。

D3-02 文档板只有 Anatomy 样本保留极淡虚线边界，以说明 `16px` 命中高度；State Matrix 与 Width Stress 中的 Hit Target 全部不可见。

因此：

> 易点击不等于画粗。

Timeline 的 Track / Buffer / Progress 始终只有 `3px`。

## 6. Thumb 明亮背景可读性补强

第一轮视觉收敛后发现：白色 10px Thumb 在浅色卡片或未来明亮视频背景上容易消失。

本任务新增：

```text
control/thumb-border
```

规则：

- 1px inside stroke；
- 柔紫语义色；
- Stroke opacity = `28%`；
- Thumb Fill 仍使用 `control/thumb`；
- Thumb Diameter 仍为 `10px`；
- 不扩大 Thumb，不做高饱和圆点。

最终检查：7 个 seekable Thumb 的 Stroke opacity 全部为 `28%`。

## 7. Width Stress

使用真实 D3-01 Timeline Lane 宽度验证：

```text
560px · Narrow
728px · Wide + Inspector Dock
828px · Max Width Lane
```

三种宽度全部使用同一公式：

```text
trackX     = 5
trackWidth = W - 10
thumbX     = ratio × trackWidth
```

没有 Narrow Timeline、Wide Timeline、Fullscreen Timeline 三套几何。

Narrow 只改变 control offset：

```text
Narrow          10
Standard / Wide 12
```

Track thickness / Thumb / Hit Target 不变。

## 8. Figma 可视化结构

`D3-02 / Timeline Basic Geometry` 包含：

```text
Rule Cards
├─ Visual vs Hit
├─ Endpoint Rule
└─ Ownership

Timeline Anatomy

Static State Matrix
├─ 0%
├─ 50%
├─ 100%
├─ Unknown Duration
└─ Non-seekable

Width Stress
├─ 560 Narrow Lane
├─ 728 Wide + Inspector Dock
└─ 828 Max Width Lane

Geometry Map
└─ Closing Rule
```

## 9. 实施中发现并修复的问题

### 9.1 Hit Target 工程层过强

第一次截图中 Hit Target 的青色辅助 Fill/Stroke 在 State Matrix 与 Width Stress 仍然可见，导致视觉上像第二条粗轨道。

已修复：

- Anatomy：仅保留极淡虚线 outline；
- 其余 8 个产品/状态样本：Hit Target Fill/Stroke 全部隐藏；
- Timeline Visual Track 重新成为唯一可见轨道。

### 9.2 白色 Thumb 在亮背景上辨识度不足

第二轮截图发现 10px 白色 Thumb 在白底上接近消失。

已修复：

- 新增 `control/thumb-border`；
- 1px / 28% 柔紫 inside stroke；
- 没有改变 Thumb 尺寸或高饱和度。

### 9.3 Semantic Paint Binding 再次重置 Stroke opacity

首次绑定 `control/thumb-border` 后，Figma 将 Stroke opacity 写回 `100%`。

已单独执行第二轮 opacity restore；最终 7 个 Thumb 均为：

```text
Stroke opacity = 28%
```

## 10. 最终验证

### Timeline 几何

共 9 个 Timeline 样本：

- Hit Target height：全部 `16px`；
- Track height：全部 `3px`；
- Track radius：全部 `2px`；
- Seekable Thumb：全部 `10×10`；
- Visual Track：全部 `x=5 / width=W-10`。

端点验证：

```text
252px Timeline
Track x      = 5
Track width  = 242

0%   Thumb x/right = 0 / 10
50%  Thumb x/right = 121 / 131
100% Thumb x/right = 242 / 252
```

100% 右端误差：`0`。

浮点计算样本最大 Thumb X 误差仅约 `0.00001px`。

### State Structure

- Unknown：Progress=`0`、Buffer=`0`、Thumb=`0`；
- Non-seekable：Progress=`0`、Buffer=`0`、Thumb=`0`；
- 0/50/100：共享同一 Track / Thumb 几何。

### Semantic Paint Audit

```text
D3-02 Visible Solid Paint
144 / 144 Semantic-bound
Unbound = 0
```

### D1 Regression

```text
Semantic Color 142 / 142
Typography      43 / 43
Geometry        55 / 55
Effect          25 / 25
```

### D2 / D3-01 Regression

- D2-01 ～ D2-05 五个 Contract Board 全部存在；
- D3-01 OSC Grid 保持；
- D3-01 四档 Surface 仍为：
  - 616×106 / fill32% / stroke48%；
  - 856×124 / fill34% / stroke48%；
  - 780×124 / fill34% / stroke48%；
  - 880×124 / fill34% / stroke48%。

## 11. 保持不变

本任务没有：

- 修改 D3-01 OSC Surface / Internal Grid；
- 修改 D2 Window / Viewport / Header / Host / Responsive Contract；
- 创建 Hover Preview；
- 创建 Scrub / Pending Seek；
- 创建 Chapter Marker；
- 创建 Seek Preview；
- 创建正式 Timeline Component Set；
- 修改 Playback 状态语义。

## 12. 下一任务

`D3-03 Timeline 交互状态`

下一步在本次冻结的同一 Timeline 几何上增加：

- Hover Preview；
- Scrubbing；
- Pending Seek；
- Cancel / Commit；
- Chapter Marker；
- Seek Preview；

但不得改变 D3-02 已冻结的 3px Track / 16px Hit Target / 10px Thumb / endpoint formula。