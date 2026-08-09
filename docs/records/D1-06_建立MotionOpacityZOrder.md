# D1-06 建立 Motion / Opacity / Z-order — 完成记录

> Task ID：D1-06  
> 状态：Complete  
> Figma 文件：`Qt6 + libmpv Player — Native Blank Slate Exploration`  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Foundations Page：`01 Foundations`  
> Foundations 文档板：`41:2` — `D1-06 / Motion Opacity Z-order`

## 1. 任务目标

为第三版 Airy Glass 播放器建立唯一的 Motion / Opacity / Z-order 基础，使后续 OSC、Inspector、Popover、HUD、Toast、Dialog 不再各自维护 transition、alpha 或 overlay 层级。

本任务遵循 D1 任务书约束：

- 默认动效轻、短、无 spring / bounce；
- 支持 Reduce Motion；
- Overlay 层级不可互相争抢；
- `interaction state → motion token → component`；
- OSC / Inspector / Dialog 同时出现时必须通过 smoke 验证。

## 2. 实施前真实审计

审计 Main Player / Fullscreen / Playlist Inspector 三张已确认框架：

- 可见节点：162；
- 已存在 Prototype Reaction：0；
- Node-level `opacity != 1`：仅 1 个；
- 唯一真实 Opacity 例外：`Floating Media Header / Close` = 72%；
- Main stacking：`Video Scene → Floating Media Header → Glass OSC`；
- Fullscreen stacking：`Video Scene → Floating Title → Fullscreen OSC`；
- Playlist / Inspector stacking：`Video Scene → Glass OSC → Floating Inspector`。

结论：现有框架没有遗留第二套 Motion 逻辑，可直接建立统一规则；现有 Figma child order 已与后续语义层级基本一致。

## 3. Interaction Primitive

新增 Variable Collection：

- `V3 / Interaction Primitive`
- Mode：`Base`
- 总数：26

### 3.1 Duration Primitive

```text
duration/instant      0 ms
duration/fast       120 ms
duration/short      160 ms
duration/close      180 ms
duration/standard   200 ms
duration/deliberate 240 ms
```

### 3.2 Easing Primitive

```text
easing/out     cubic-bezier(0.2, 0, 0, 1)
easing/in      cubic-bezier(0.4, 0, 1, 1)
easing/linear  linear
```

禁止在产品动效中使用 spring / bounce 作为默认过渡。

### 3.3 Alpha Primitive

Figma `OPACITY` Scope 的 FLOAT 使用 0–100 百分比语义：

```text
alpha/hidden     0
alpha/scrim     18
alpha/disabled  38
alpha/idle      72
alpha/pressed   84
alpha/visible  100
```

### 3.4 Z Primitive

```text
z/video             0
z/atmosphere       10
z/media-content    20
z/floating-header  30
z/osc              40
z/inspector        50
z/popover          60
z/hud              70
z/toast            80
z/dialog-scrim     90
z/dialog          100
```

这些值同时作为 Qt Quick / QML `z` 层级的实现契约。

## 4. Interaction Semantic

新增 Variable Collection：

- `V3 / Interaction Semantic`
- Mode 1：`Standard`
- Mode 2：`Reduce Motion`
- 总数：42

其中：

- Motion Semantic：24；
- Opacity Semantic：7；
- Z-order Semantic：11。

## 5. Motion Semantic

### OSC

```text
motion/osc/show-duration  160 ms
motion/osc/hide-duration  120 ms
show easing               Ease Out
hide easing               Ease In
```

### Inspector

```text
motion/inspector/open-duration   240 ms
motion/inspector/close-duration  180 ms
open easing                      Ease Out
close easing                     Ease In
```

### Popover

```text
open   160 ms / Ease Out
close  120 ms / Ease In
```

### HUD

```text
show  160 ms / Ease Out
hide  120 ms / Ease In
```

### Dialog

```text
open   240 ms / Ease Out
close  180 ms / Ease In
```

### Toast

```text
show  200 ms / Ease Out
hide  160 ms / Ease In
```

## 6. Reduce Motion

`V3 / Interaction Semantic` 使用 Figma Variable Mode 实现 Reduce Motion，而不是维护第二套组件。

最终验证：12 / 12 个 Motion Duration Semantic 在 `Reduce Motion` Mode 下全部解析为：

```text
0 ms
```

Easing 在 Reduce Motion Mode 下解析到 `linear`，但由于 duration 为 0，不产生视觉运动。

## 7. Semantic Opacity

```text
opacity/hidden            0%
opacity/dialog/scrim     18%
opacity/control/disabled 38%
opacity/control/idle     72%
opacity/control/pressed  84%
opacity/control/hover   100%
opacity/visible         100%
```

边界：

- D1-06 Opacity 只表达交互状态、显隐和 Dialog Scrim；
- D1-05 Material Paint alpha 继续负责玻璃材质；
- 禁止把整个玻璃 Frame 通过状态 opacity 统一变淡，导致内部文字、图标一起失真。

当前框架真实回刷：

- `4:18 Floating Media Header / Close`
- 72% 已绑定到 `opacity/control/idle`；
- 最终实际 Node opacity = `0.72`。

## 8. Z-order Contract

正式层级：

```text
Dialog          z100
Dialog Scrim     z90
Toast            z80
HUD              z70
Popover          z60
Inspector        z50
OSC              z40
Floating Header  z30
Media Content    z20
Atmosphere       z10
Video             z0
```

当前三个框架的关键层均写入共享 Figma 注记 namespace：

```text
openai.v3player / z-role
```

已注记节点：9 个。

当前 Figma child order 与 Z-order 语义一致：

- Main：Video < Header < OSC；
- Fullscreen：Video < Header < OSC；
- Inspector：Video < OSC < Inspector。

Figma 负责 child-order 视觉验证；Qt Quick / QML 实现使用 `z/*` 数值契约，不依赖 QML 对象创建顺序。

## 9. Foundations 文档板

在 `01 Foundations` 新增：

```text
D1-06 / Motion Opacity Z-order
├─ Semantic Motion Matrix
├─ Semantic Opacity
├─ Z-order / Overlay Contract
├─ Simultaneous Overlay Smoke
├─ Product Interaction Map
└─ D1-06 Rules
```

Node：`41:2`

文档板明确展示：

- Standard / Reduce Motion 差异；
- OSC / Inspector / Popover / HUD / Dialog / Toast 的 enter / exit duration；
- Opacity 状态；
- 11 级 Overlay stack；
- Dialog > HUD > Inspector > OSC 的最终重叠验证；
- D1 与 D3 的职责边界。

## 10. Smoke Prototype

创建 4 个独立 Prototype Frame：

```text
41:155  D1-06 Smoke / 1 OSC
41:170  D1-06 Smoke / 2 Inspector
41:185  D1-06 Smoke / 3 HUD
41:200  D1-06 Smoke / 4 Dialog
```

实际 Prototype Reaction：

```text
1 OSC
  ↓ ON_CLICK / SMART_ANIMATE / EASE_OUT / 240 ms
2 Inspector
  ↓ ON_CLICK / SMART_ANIMATE / EASE_OUT / 160 ms
3 HUD
  ↓ ON_CLICK / SMART_ANIMATE / EASE_OUT / 240 ms
4 Dialog
```

说明：

- 1 → 2 的 240 ms 对应 Inspector Open；
- 2 → 3 的 160 ms 对应 HUD Show；
- 3 → 4 的 240 ms 对应 Dialog Open；
- 最终态同时保留 OSC / Inspector / HUD / Dialog；
- Dialog Scrim 位于 HUD / Inspector / OSC 上方，Dialog 位于 Scrim 上方。

最终截图中四层均可辨认：

- OSC 底部可见；
- Inspector 右侧可见；
- HUD 上方可见并被 Scrim 正常压暗；
- Dialog 最上层。

## 11. 实施中发现并修复的问题

### 11.1 `setPluginData` Host Runtime 不支持

首次给产品节点写 Z-role 时调用普通 `setPluginData`，当前 Figma Host Runtime 拒绝：

```text
setPluginData is not supported in this host runtime
```

该调用原子回滚，没有留下半完成状态。

修复：

- 改用 `setSharedPluginData`；
- namespace 固定为 `openai.v3player`；
- key 固定为 `z-role`。

### 11.2 Figma OPACITY Variable 单位不是 0–1

第一次把 `alpha/idle = 0.72` 绑定给 Close 后，实际 Node opacity 变成：

```text
0.0072
```

定位：Figma `OPACITY` Scope FLOAT 使用 0–100，而 Node 属性读取显示为 0–1。

修复：

```text
0.72 → 72
0.18 → 18
0.38 → 38
0.84 → 84
1.00 → 100
```

修正后 Close 实际 opacity 恢复为：

```text
0.7200000286
```

### 11.3 Smoke 最终态 HUD 被 Dialog 完全遮住

第一次最终态虽然 HUD 节点真实存在，但 Dialog 完全覆盖 HUD，肉眼无法验证“同时出现”。

修复：

- 只调整 Smoke Test 内 HUD / Dialog 的光学位置；
- 不改变任何 Z-order；
- 不修改产品框架；
- 最终截图中 HUD、Inspector、OSC、Dialog 均可辨认。

### 11.4 `get_motion_context` 不读取 Prototype Reaction

对 Smoke Frame 调用 Motion Context 返回空数组。

原因：该工具用于 Timeline / Keyframe Motion，不代表 Prototype Reaction 不存在。

最终改为直接审计 Figma `reactions` 属性，确认三段 `ON_CLICK + SMART_ANIMATE` 真实存在，duration / easing 与设计定义一致。

## 12. 最终 D1-06 验证

```text
Interaction Primitive         26
Interaction Semantic          42
Semantic Modes                 2

Motion Semantic               24
Opacity Semantic               7
Z-order Semantic              11

Reduce Motion durations      12 / 12 = 0 ms
Smoke Prototype Frames         4
Smart Animate Reactions        3
Reaction mismatch              0

Close idle opacity            72%
Z-role annotated product nodes 9
```

## 13. Stage D1 全量回归

D1-06 完成后重新验证整个 Foundation 链：

```text
Primitive Color                17
Semantic Color                 38
Type Primitive                 14
Geometry Primitive             38
Geometry Semantic              49
Effect Primitive               38
Material Semantic              47
Interaction Primitive          26
Interaction Semantic           42
```

核心框架回归：

```text
Visible Solid Paints          142
Semantic Color bound          142
Color unbound                   0

Framework Text Nodes           43
V3 Text Style bound            43
Typography unstyled             0

Geometry-bound target nodes    55

V3 Effect Style nodes          25
```

并再次截图检查：

- Main Player：通过；
- Fullscreen：通过；
- Playlist / Inspector：通过；
- D1-06 Foundations：通过；
- D1-06 Smoke Final：通过。

## 14. D1 与后续阶段边界

D1-06 不定义 OSC 自动隐藏的实际 timer 生命周期。

原因：

- D1 只定义 transition duration / easing / opacity / z；
- “多久无操作后隐藏”“鼠标移动如何重置 timer”“Seeking / Menu Open 时是否暂停隐藏”等生命周期属于 D3 OSC 行为系统；
- 这样避免出现两套 timer / transition 规则。

## 15. Stage D1 关闭结论

D1-01 ～ D1-06 已全部完成。

当前 Foundations 已覆盖：

- Color；
- Semantic Color；
- Typography；
- Spacing / Size / Radius；
- Glass / Blur / Shadow；
- Motion / Opacity / Z-order；
- Reduce Motion。

Stage D1 可关闭，后续 D2–D7 可以直接消费这些 Token，不需要重新发明基础视觉规则。

## 16. 下一任务

`D2-01 定义 Player Window 外壳`

目标：把当前确认的 Main Player 探索框架推进为正式 Player Window / Window Surface，建立窗口圆角、边界、Elevation、内容裁切和基础安全区；验证常规、最大化与不同背景，使其一眼仍然是播放器窗口而不是网页大卡片。