# D3-03 Timeline 交互状态 — 完成记录

> Task ID：D3-03  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-03 Board：`104:2` — `D3-03 / Timeline Interaction States`

## 1. 任务目标

按照 `D3_OSC与时间轴控制系统.md` 的 D3-03 定义，在不改变 D3-02 静态几何的前提下，建立 Timeline 的完整交互状态：

- Hover Preview；
- Scrubbing；
- Pending Seek；
- Commit；
- Cancel；
- Chapter Marker / Chapter Target；
- Seek Preview；
- 实际播放位置与 Pointer / Seek Target 的视觉区分。

本任务不提前实现 Transport、Volume、Utility Actions 或 OSC 整体显隐生命周期。

## 2. 交互所有权

正式规则：

```text
PlaybackSnapshot
└─ owns confirmed playback position

Pointer Hover
└─ owns preview target only

Pointer Drag
└─ owns temporary scrub target

Seek Request
└─ owns pending target until backend confirmation
```

关键约束：

> Preview 不是 Playback Position；Pending Seek 不是已确认 Position。

只有后端位置确认后，真实 Progress / Thumb 才进入新位置。

## 3. 正式状态链

```text
Rest
→ Hover Preview
→ Scrubbing
→ Pending Seek
→ Committed
```

另有：

```text
Scrubbing → Cancel → Rest
Rest → Chapter Hover / Chapter Seek Intent
```

### Rest

- 只显示已确认 Playback Position；
- Violet Progress + White Thumb；
- 不显示 Preview / Pending Target。

### Hover Preview

- 实际 Progress / Thumb 保持原位；
- Pointer Target 使用 Cyan Preview Marker；
- 显示 Seek Preview Bubble；
- 不改写实际播放位置。

### Scrubbing

- Thumb 跟随 Pointer Target；
- Progress 暂时投影到 Scrub Target；
- 保留弱化 `Committed Marker`，标记拖动开始前的已确认播放位置；
- Scrub 更新使用 0ms / Linear。

### Pending Seek

- Pointer Release 后发出 Seek Intent；
- 实际 Progress 仍保持原位置；
- Pending Range 表示实际位置与目标之间的请求区间；
- Hollow Pending Target 标记目标；
- 等待 backend position confirmation。

### Committed

只有确认后：

- 实际 Progress 更新到目标；
- 实际 Thumb 更新到目标；
- Pending Range / Target 消失。

### Cancel

ESC / drag cancel：

- 删除 temporary target；
- 返回原始 Rest；
- 实际位置不变。

## 4. Chapter Marker

Chapter Marker 不创建第二条进度轨道。

正式几何：

```text
Chapter Marker
1 × 7px
Idle opacity = 34%
Hover / target = 90%
```

Idle 使用 `control/chapter-marker`；Hover / target 使用 `control/preview`。

Chapter Marker 只表达时间线地标，Chapter 内容和列表仍归 D4 / Chapters Content。

## 5. Seek Preview

新增几何：

```text
Preview Marker       6 × 6
Preview Bubble      64 × 28
Preview Bubble R14
```

材质：

```text
Preview Bubble Fill   52%
Preview Bubble Border 48%
V3 / Glass / Control
```

Preview 使用 Cyan 语义，与真实 Violet Progress 明确区分。

## 6. Pending Seek

正式视觉：

```text
Pending Range opacity       72%
Pending Target fill         16%
Pending Target stroke       80%
```

Pending Target 为 Hollow Target，不伪装成已确认 Thumb。

## 7. Token 补强

### Geometry Primitive

按真实缺口补充：

- `size/1`
- `size/6`
- `size/7`
- `size/64`
- `size/28`（不存在时补齐）
- `radius/14`

### Geometry Semantic

- `size/timeline/preview-marker`
- `size/timeline/chapter-marker-width`
- `size/timeline/chapter-marker-height`
- `size/timeline/preview-bubble-width`
- `size/timeline/preview-bubble-height`
- `radius/timeline/preview-bubble`

### Color Semantic

- `control/preview`
- `control/pending-target`
- `control/chapter-marker`
- `surface/seek-preview`

### Motion Semantic

- `motion/timeline/preview-show-duration`
- `motion/timeline/preview-hide-duration`
- `motion/timeline/scrub-update-duration`
- `motion/timeline/pending-enter-duration`
- `motion/timeline/commit-duration`
- `motion/timeline/cancel-duration`
- `motion/timeline/easing`
- `motion/timeline/scrub-easing`

Standard：

```text
Preview show/hide  120ms
Scrub update         0ms
Pending enter      160ms
Commit             160ms
Cancel             120ms
```

Reduce Motion：所有 Timeline duration 均解析为 `0ms`。

## 8. Prototype

建立 6 个 Page 顶层 Smoke Prototype Frame：

- `105:2` — Rest
- `105:17` — Hover Preview
- `105:35` — Scrubbing
- `105:55` — Pending Seek
- `105:73` — Committed
- `105:88` — Chapter Hover

主链共有 6 条 Reaction：

```text
Rest → Hover          120ms Ease Out
Hover → Scrub         ~0ms Linear
Scrub → Pending       160ms Ease Out
Pending → Commit      160ms Ease Out
Commit → Chapter      120ms Ease Out
Chapter → Rest        120ms Ease Out
```

另外：

- `105:53` — `ESC · CANCEL`
- 独立 Reaction：Scrubbing → Rest，120ms Ease Out。

Prototype 中使用点击只作为 smoke navigation；真实触发契约仍是 pointer hover / drag / release / backend confirmation / ESC。

## 9. 实施中发现并修复的问题

### 9.1 D3-03 文本未绑定 Semantic Color

初次 Board 使用正式 Text Style，但文字 Fill 仍是局部实色，Visible Solid Paint 审计发现 114 个未绑定项。

已统一回刷：

- Docs Display → `text/primary`
- Docs Eyebrow → `accent/strong`
- Supporting / Body Meta → `text/secondary`
- Label → `text/strong`
- Technical → `text/tertiary`
- Timecode Primary → `text/emphasis`
- Timecode Secondary → `text/tertiary`

最终 Board Visible Solid Paint `302/302` Semantic-bound。

### 9.2 Paint opacity 再次被 Variable Binding 写回 100%

Chapter / Preview / Pending 的低透明度被 Figma 绑定流程写回 100%。

已分离“颜色绑定”和“透明度恢复”步骤，最终值全部重新核验。

### 9.3 `Chapter Marker Contract` 被错误名称匹配

批量规则使用 `startsWith('Chapter Marker ')` 时，将容器 `Chapter Marker Contract` 误判成 Chapter Marker，导致容器被绑定成 `1×7px`，整块内容在截图中消失。

已修复：

- 容器恢复 `1696×250`；
- 解除错误 Width/Height Variable binding；
- Fill 恢复 `surface/glass / 36%`；
- Stroke 恢复 `border/glass / 48%`；
- Chapter Marker 后续只严格匹配 `Chapter Marker 1..4`。

## 10. 最终验证

### Board Paint Audit

```text
Visible Solid Paint   302
Semantic-bound        302
Unbound                 0
```

### Chapter Contract

```text
1696 × 250
Fill   36%
Stroke 48%
```

### Chapter Marker

```text
1 × 7
Idle  34%
Hover 90%
```

### Preview / Pending

```text
Preview Marker       6 × 6 / 92%
Preview Bubble      64 × 28 / R14
Bubble Fill         52%
Bubble Stroke       48%
Pending Range       72%
Pending Target Fill 16%
Pending Target Line 80%
```

### Motion

所有 Timeline Motion 在 Reduce Motion Mode 下均解析为 `0ms`。

### Prototype

```text
Top-level Frames      6
Main Reactions        6
Cancel Reactions      1
Missing destination   0
```

### Regression

```text
D1 Semantic Color    142 / 142
D1 Typography         43 / 43
D1 Geometry           55 / 55
D1 Effect             25 / 25

D2-01 ～ D2-05       保持
D3-01                保持
D3-02                保持
```

## 11. 保持不变

本任务没有：

- 改变 D3-02 的 3px Track / 16px Hit Target / 10px Thumb；
- 修改 Player Window / Video Viewport / Floating Header / Host / Responsive；
- 创建 Transport / Volume / Utility Action；
- 提前实现 OSC Hidden / Rest / Active / LockedVisible 生命周期；
- 创建正式可复用 Timeline Component Set（组件系统收口仍按计划留 D8）。

## 12. 下一任务

`D3-04 Transport Cluster`

目标：设计 Previous / PlayPause / Next 等核心播放操作，并验证 playing / paused / disabled / hover / press / focus，同时保持播放主操作明确但不抢视频视觉。