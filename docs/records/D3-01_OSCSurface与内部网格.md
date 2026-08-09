# D3-01 OSC Surface 与内部网格 — 完成记录

> Task ID：D3-01  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-01 Board：`90:3` — `D3-01 / OSC Surface & Internal Grid`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-01 定义，把已确认的悬浮玻璃控制带升级为稳定的 OSC Layout System。

本任务只负责：

- OSC Glass Surface；
- OSC width rule；
- Timeline Lane；
- Control Lane；
- Narrow / Standard / Wide 下的几何密度；
- 与 D2-05 Host / Inspector 响应式范围的连接。

本任务明确不提前实现：

- D3-02 Timeline track/progress/buffer/thumb 的正式几何；
- D3-03 Timeline hover/scrub/pending/chapter 行为；
- D3-04 Transport 控件状态；
- D3-05 Volume 交互；
- D3-06 Utility Action 具体组成；
- D3-07 OSC visibility lifecycle。

## 2. 正式 OSC Width Rule

冻结：

```text
OSC surface width
= min(
    OSC host width - 2 × spacing/osc/surface-margin,
    size/osc/max-width
  )

spacing/osc/surface-margin = 26
size/osc/max-width = 880
```

结果：

```text
720   host 668  → surface 616
960   host 908  → surface 856
1280  host 832  → surface 780
1600  host 1152 → surface 880
```

四个样本 `width error = 0`。

意义：

- OSC 永远是浮在视频上的控制器，不允许自动铺满整个 Window；
- 1600px Wide Window 即使拥有 1152px Host，也会在 880px 停止增长；
- Inspector Dock 只改变 Host 可用范围，不改变 OSC 内部布局模型；
- 当前 880px Max Width 直接复现已确认 Main Player OSC 的实际宽度。

## 3. 正式纵向网格

OSC 始终只有两个结构 Lane：

```text
OSC Surface
├─ Timeline Lane
└─ Control Lane
```

### Standard / Wide

```text
padding top    18
Timeline Lane  28
section gap    14
Control Lane   40
padding bottom 24
-----------------
total          124
```

校验：

```text
18 + 28 + 14 + 40 + 24 = 124
```

### Narrow / Compact

```text
padding top    12
Timeline Lane  26
section gap     6
Control Lane   40
padding bottom 22
-----------------
total          106
```

校验：

```text
12 + 26 + 6 + 40 + 22 = 106
```

Primary Playback 的 40px 基础尺寸因此不需要为 Narrow 模式缩小。

## 4. Responsive Variable 接入

D3-01 扩展 `V3 / Responsive Semantic`，新增 7 个 OSC 语义：

```text
osc/inset
osc/radius
osc/timeline-lane-height
osc/control-lane-height
osc/section-gap
osc/padding-top
osc/padding-bottom
```

当前 `V3 / Responsive Semantic`：

```text
16 → 23 variables
Modes:
- Narrow
- Standard
- Wide
```

四个正式测试 Window 均使用同一 OSC node structure，通过 Window 上的 Responsive Variable Mode 解析不同几何。

## 5. Geometry Token 补强

新增 Geometry Primitive：

```text
size/26  = 26
size/28  = 28
size/880 = 880
```

新增 Geometry Semantic：

```text
size/osc/max-width
size/osc/timeline-lane
size/osc/timeline-lane-compact
size/osc/control-lane
spacing/osc/surface-margin
spacing/osc/section-gap
spacing/osc/section-gap-compact
spacing/osc/padding-top
spacing/osc/padding-top-compact
spacing/osc/padding-bottom
spacing/osc/padding-bottom-compact
```

当前 Geometry Semantic 总数：`73`。

## 6. Internal Control Grid 职责

D3-01 只冻结 Control Lane 的结构方向：

```text
Control Lane
├─ Left Anchor
│  └─ Transport + Adaptive Secondary
├─ Flexible Space
└─ Right Anchor
   └─ Utility Actions
```

响应式策略继续服从 D2-05：

- Narrow：`essential + More`；
- Standard：`mixed + More`；
- Wide：`full utility`；
- Volume 在 Narrow / Standard 为 Popover，Wide 才允许 Inline。

D3-01 Board 中的 Anchor 只是 Grid Slot，不是正式 Button Group Component；D3-04 ～ D3-06 再决定内容和交互。

## 7. Figma 实际节点

新增 Page：

```text
03 OSC
```

新增 Board：

```text
D3-01 / OSC Surface & Internal Grid
Node: 90:3
```

Anatomy OSC：

```text
Node: 90:21
880 × 124
V3 / Glass / OSC
```

响应式 OSC：

```text
720 Narrow
Window  90:35
Surface 90:42
616 × 106

960 Standard
Window  90:55
Surface 90:62
856 × 124

1280 Wide + Inspector Dock
Window  90:75
Surface 90:82
780 × 124

1600 Wide + Inspector Dock
Window  90:95
Surface 90:102
880 × 124
```

每个 Surface：

- `layoutMode = VERTICAL`；
- children 顺序固定为 `Timeline Lane → Control Lane`；
- `height / radius / padding / itemSpacing / lane heights` 均绑定 Responsive/Geometry Variable；
- shared `z-role = z/osc`；
- shared `width-rule = min(hostWidth - 2*spacing/osc/surface-margin, size/osc/max-width)`。

## 8. Material

正式保持 D1-05 已验证的 Glass Material：

```text
Narrow / Compact
fill   32%
stroke 48%
V3 / Glass / OSC Compact
R32

Standard / Wide
fill   34%
stroke 48%
V3 / Glass / OSC
R34
```

本任务没有创建新的 Glass Effect Style。

## 9. 实施中发现并修复的问题

### 9.1 Inspector Host Node Type 错误

D3-01 Board 第一次创建时，把 `Inspector Host Context` 建成 Rectangle 后又试图将 Label 作为其子节点。

Figma 报错：Rectangle 不支持 `appendChild`。

结果：

- 本轮 mutation 原子回滚；
- 没有留下半完成 Board；
- 改用 Frame 后重新完整生成。

### 9.2 Semantic Paint Binding 再次把 Paint Alpha 写成 100%

第一轮截图发现：

- OSC Surface 变成近乎白色实体卡片；
- Timeline / Control Guide 过重；
- Inspector Host Context 变成大面积橙色实体区。

读取节点确认：OSC fill/stroke 均被写成 `opacity = 1.0`。

已统一恢复 55 个 D3-01 节点的目标材质 alpha，并保留所有 Color Variable binding。

同时把 Inspector Host Context 改为：

```text
fill   none
stroke 30%
```

确保它只表达 Host range，不伪装成 D4 的正式 Inspector Surface。

## 10. 最终视觉验证

第二轮 D3-01 Board 截图通过：

- OSC 恢复 Airy Glass，而不是白色底栏；
- Inspector Context 退回轻量 outline；
- 720 / 960 / 1280 / 1600 四档视觉保持同一控制语言；
- 1600 Wide 下 OSC 在 880px 停止增长；
- Inspector Dock 时只缩 Host，不重做 OSC；
- Narrow 保持完整两层网格和 40px Control Lane。

## 11. 最终结构审计

```text
720 Narrow
Host                 668
Surface              616
Expected             616
Width error            0
Height                106
Radius                 32
Padding L/R            28
Padding T/B         12/22
Section gap             6
Timeline Lane          26
Control Lane           40
Vertical sum          106
Glass fill             32%
Glass stroke           48%

960 Standard
Host                 908
Surface              856
Expected             856
Width error            0
Height                124
Radius                 34
Padding L/R            26
Padding T/B         18/24
Section gap            14
Timeline Lane          28
Control Lane           40
Vertical sum          124
Glass fill             34%
Glass stroke           48%

1280 Wide + Dock
Host                 832
Surface              780
Expected             780
Width error            0
Vertical sum          124

1600 Wide + Dock
Host                1152
Surface              880
Expected             880
Width error            0
Vertical sum          124
```

D3-01 Board Paint Audit：

```text
Visible Solid Paints       141
Semantic-bound             141
Visible Unbound              0
```

## 12. D1 / D2 回归

D1：

```text
Semantic Color   142 / 142
Typography        43 / 43
Geometry          55 / 55
Effect            25 / 25
```

D2 Contract：

```text
D2-01 exists
D2-02 exists
D2-03 exists
D2-04 exists
D2-05 exists
```

没有修改已确认 Framework，也没有重做 D2 Window / Viewport / Header / Host / Responsive Contract。

## 13. 完成判断

D3-01 达成任务书要求：

- OSC 不铺满整窗；
- Glass Surface 视觉轻；
- Timeline / Controls 两层结构稳定；
- Narrow / Standard / Wide 由同一 Variable-driven grid 解析；
- Inspector Dock 只改变 Host 可用宽度；
- 后续 D3-02 可以直接在 Timeline Lane 内建立正式 Timeline Geometry，不需要再改 OSC 外壳。

## 14. 下一任务

`D3-02 Timeline 基础几何`

下一步只在已经冻结的 Timeline Lane 中建立：

- visual track；
- buffered range；
- progress；
- thumb；
- interaction hit target；
- 0 / 50 / 100%；
- unknown duration；
- non-seekable。

不会回头重新设计 OSC Surface。