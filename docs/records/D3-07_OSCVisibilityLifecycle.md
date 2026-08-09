# D3-07 OSC Visibility Lifecycle — 完成记录

> Task ID：D3-07  
> 状态：Complete  
> Figma File Key：`KIOxfwTvQJlcVLinkeJAxY`  
> Figma Page：`03 OSC` — `90:2`  
> D3-07 Board：`142:2` — `D3-07 / OSC Visibility Lifecycle`

## 1. 任务目标

按照 `docs/plans/stages/D3_OSC与时间轴控制系统.md` 的 D3-07 定义，建立可直接交付开发的 OSC 显隐生命周期，而不是只做一段 fade 动画。

任务覆盖：

- Hidden / Rest / Active / LockedVisible 四态；
- 唯一 Visibility Controller；
- 唯一 hide deadline；
- Playing 静置自动隐藏；
- Hover / Press / Timeline Preview；
- Timeline Scrub；
- Volume Popover / More Popover；
- Inspector；
- Paused / Error；
- Keyboard / Media Key；
- Main / Fullscreen 一致性；
- Reduce Motion；
- 可运行的 Figma Smoke Prototype。

本任务不提前实现：

- D4 Inspector 业务内容；
- D5 最终 Error Overlay / HUD / Toast / Dialog；
- D7 Fullscreen Cursor Hide 与 Mini Player 最终几何；
- D8 正式 Component Set / Library 收口。

## 2. 状态机

唯一解析顺序：

```text
1. lockReasons > 0
   → LockedVisible

2. directInteraction = true
   → Active

3. autoHideAllowed = false
   → Rest · persistent

4. now >= hideDeadline
   → Hidden

5. otherwise
   → Rest · countdown
```

状态语义：

### Hidden

```text
OSC parent opacity = 0
OSC hit-test        = off
Reveal owner        = Video Viewport / Player Viewport host
```

隐藏后 OSC 本身不能再负责“Hover 自己把自己显示出来”，否则隐藏层会继续抢 pointer。Reveal 必须来自 Viewport host。

### Rest

OSC 可见但当前没有直接控制交互。

分两种：

```text
Rest · countdown
Playing + no lock + no direct interaction
→ hideDeadline 存在

Rest · persistent
Paused / Error
→ hideDeadline 不存在
```

### Active

直接控制交互：

- pointer over OSC；
- control hover / press；
- Timeline hover preview；
- 其他短时直接交互。

Active 不修改 OSC 母 Surface 的整体亮度；Hover/Pressed/Focus 仍由子控件自己表达。

### LockedVisible

用于必须保证 OSC 不消失的显式锁：

```text
Timeline Scrubbing
FocusWithinOSC
Volume Popover Open
More Popover Open
Inspector Open
```

LockedVisible 暂停 hide deadline；锁释放后如果仍在 Playing 且没有直接交互，则重新进入 Rest · countdown，并由唯一 owner 重新生成 deadline。

## 3. 单一 Timer Owner

正式规则：

```text
Visibility Controller owns:
- visibilityState
- hideDeadline
- reveal()
- scheduleHide()
- cancel/suspend deadline

Child controls emit:
- activity
- lock acquired
- lock released

Child controls NEVER own:
- setTimeout
- hide timer
- OSC fade timer
```

因此不会出现：

- Timeline 一个 timer；
- Volume Popover 一个 timer；
- More 一个 timer；
- Inspector 一个 timer；
- Fullscreen 再复制一套 timer。

## 4. Hide Deadline

新增唯一语义变量：

```text
motion/osc/hide-delay = 2200ms
```

Figma Variable ID：`VariableID:141:2`。

Mode：

```text
Standard      = 2200ms
Reduce Motion = 2200ms
```

Reduce Motion 只移除过渡动画，不改变用户静置多久后自动隐藏的行为时间。

Deadline 只由 Visibility Controller 在以下时机创建或重置：

- Hidden 被 Viewport activity 唤醒；
- Playback 从 Paused 恢复 Playing；
- Direct interaction 结束；
- LockedVisible 的最后一个 lock 释放。

Active / LockedVisible 期间 deadline 不允许到期触发隐藏。

## 5. Motion

完全复用 D1-06：

```text
Hidden → Rest
Show 160ms
Ease Out
motion/osc/show-duration
motion/osc/show-easing

Rest → Hidden
Hide 120ms
Ease In
motion/osc/hide-duration
motion/osc/hide-easing

Reduce Motion
Show = 0ms
Hide = 0ms
hide-delay = 2200ms unchanged
```

Rest ↔ Active、Active ↔ LockedVisible 不做 OSC parent 级亮度动画。

## 6. Visible-State Material Contract

正式冻结：

```text
OSC parent material
REST = ACTIVE = LOCKEDVISIBLE

Fill   = 32%
Border = 48%
Effect = V3 / Glass / OSC
```

理由：避免 pointer 横穿多个控件时整条 OSC 不断闪亮/变暗。

只有子级发生变化：

- Timeline hover/scrub；
- Button Hover/Pressed/Focus；
- Utility Panel Open；
- Popover；
- Inspector。

Hidden 才会把整个 OSC parent opacity 变成 0。

## 7. 场景策略

### Playing + interaction ends

```text
→ Rest · countdown
hideDeadline = now + 2200ms
```

### Deadline expires

```text
→ Hidden
Hide 120ms Ease In
```

### Viewport pointer activity while Hidden

```text
→ Rest · countdown
Show 160ms Ease Out
hideDeadline = now + 2200ms
```

Viewport host 只负责发 reveal activity，不拥有 timer。

### Pointer over OSC / Control Hover

```text
→ Active
parent material unchanged
```

### Timeline Scrub

```text
→ LockedVisible
until commit/cancel
```

Scrub pointer capture 是 lock reason，不创建新的 hide timer。

### Volume Popover / More Popover

```text
→ LockedVisible
Popover owns z60 motion
OSC stays visible until close
```

关闭后释放 lock，由 Visibility Controller 决定 Rest/countdown。

### Inspector

```text
→ LockedVisible
Inspector owns z50 motion/content
OSC stays visible until close
```

### Paused

```text
→ Rest · persistent
autoHideAllowed = false
hideDeadline = none
```

如果之前为 Hidden，Pause 需要先 Show OSC。

### Error

```text
→ Rest · persistent
autoHideAllowed = false
hideDeadline = none
```

D5 后续拥有 Error Overlay；D3-07 只保证 OSC 不在错误状态下自行消失。

### Keyboard / Media Key

默认：

```text
visibilityState unchanged
hideDeadline unchanged
feedback → D5 HUD
```

继续遵守 D3-05 的反馈路由，避免每次音量键/媒体键都把整条 OSC 重新唤醒。

例外：

- Playback 由 Playing 变 Paused 时，状态机自然解析到 Rest · persistent；
- Keyboard focus 真正进入 OSC 时，`FocusWithinOSC` 成为 lock reason → LockedVisible。

## 8. Main / Fullscreen / Mini

### Main Player

- 使用同一个 resolver；
- 使用同一个 2200ms deadline；
- Viewport host 负责 Hidden reveal；
- Window chrome 不拥有 visibility timer。

### Fullscreen

- 使用同一个 resolver；
- 使用同一个 2200ms deadline；
- 整个 fullscreen viewport 是 reveal host；
- D3-07 不拥有 cursor hide，cursor hide 保留给 D7。

### Mini Player

D7 后续只允许改变呈现几何，不允许复制第二套 OSC lifecycle 或 timer。

## 9. Figma Board

新增：

```text
D3-07 / OSC Visibility Lifecycle
├─ One Owner / Visual Stability / Auto Hide Rules
├─ Strict Priority State Resolver
├─ Hidden / Rest / Active / LockedVisible
├─ Lock Reasons
├─ Scenario Policy Matrix
├─ Main / Fullscreen / Mini Continuity
├─ Visible-State Visual Contract
├─ Motion / Deadline Contract
└─ Stage D3 Closing Gate
```

Board Node：`142:2`。

## 10. Smoke Prototype

新增 8 个 Page 顶层 Smoke Frame：

```text
143:2   D3-07 Smoke / 1 REST PLAYING
143:15  D3-07 Smoke / 2 HIDDEN
143:23  D3-07 Smoke / 3 ACTIVE HOVER
143:38  D3-07 Smoke / 4 LOCKED SCRUB
143:53  D3-07 Smoke / 5 LOCKED POPOVER
143:69  D3-07 Smoke / 6 REST PAUSED
143:82  D3-07 Smoke / 7 REST ERROR
143:95  D3-07 Smoke / 8 FULLSCREEN SAME POLICY
```

### 唯一 After Timeout Owner

最终审计：

```text
143:2 REST PLAYING
AFTER_TIMEOUT = 2.2s
Destination   = 143:15 HIDDEN
Transition    = 120ms Ease In
```

其余 Smoke 顶层状态均没有 `AFTER_TIMEOUT`。

### 显式 Smoke Routes

```text
REST → ACTIVE
REST → LOCKED SCRUB
REST → LOCKED POPOVER
REST → PAUSED
REST → ERROR

HIDDEN → REST via Viewport Wake
ACTIVE → REST via Leave OSC
LOCKED SCRUB → REST via Commit / Cancel
LOCKED POPOVER → REST via Close Popover
PAUSED → REST via Resume
ERROR → REST via Retry
FULLSCREEN → HIDDEN via Simulate Idle
```

Fullscreen 的 Smoke Frame 不拥有第二个 timeout；它只用显式 trigger 验证同一 policy。

## 11. 实施中发现并修复的问题

### 11.1 Smoke OSC 被 Viewport 裁剪

初始 Prototype 把 OSC 放在 Viewport 下边界之外，截图中只剩背景和 trigger。

修复：

- 将 7 个可见态 `OSC Surface` 上移到 `y=146`；
- 保留 Frame/Reaction/Timer 不变。

### 11.2 z60 Popover 被 OSC clipsContent 截断

Popover 位于 OSC 之外，但 Smoke `OSC Surface` 初始开启 clipping。

修复：

- 所有可见态 Smoke `OSC Surface.clipsContent=false`；
- Popover 正常浮出；
- 不修改 z60 ownership。

### 11.3 Semantic Paint Binding 再次把 Alpha 写回 100%

第一轮结构/Reaction 正确，但变量绑定后以下材质 opacity 被重置：

- OSC；
- Popover；
- Viewport；
- Trigger；
- Open/Hover；
- Track/Progress；
- Board 的文档层辅助 Surface。

最终独立恢复 `182` 个节点目标 alpha，不改变：

- Semantic Color binding；
- Effect Style；
- Text Style；
- Reaction；
- Variable；
- Geometry。

## 12. 最终验证

### Timing Variable

```text
motion/osc/hide-delay
Standard      = 2200ms
Reduce Motion = 2200ms
```

### Single Timer Owner

```text
AFTER_TIMEOUT owners = 1
Owner = 143:2 REST PLAYING
Timeout = 2.2s
```

### Visible OSC Material

所有可见 Smoke 状态：

```text
Rest       32% / 48%
Active     32% / 48%
Scrub      32% / 48%
Popover    32% / 48%
Paused     32% / 48%
Error      32% / 48%
Fullscreen 32% / 48%
Effect     V3 / Glass / OSC
clipsContent = false
```

### Key Material

```text
More Popover  52% / 48%
Trigger       30% / 42%
Hover Action  16% / 18%
Open Action   66% / 28%
Timeline Track 16%
Progress       82%
```

### D3-07 Board Semantic Audit

```text
Visible Solid Paint = 316
Semantic-bound       = 316
Unbound              = 0

Text Nodes           = 176
Text Styled          = 176
```

### Smoke Semantic Audit

```text
Visible Solid Paint = 169
Semantic-bound       = 169
Unbound              = 0

Text Nodes           = 37
Text Styled          = 37
```

### D1 回归

```text
Semantic Color  142 / 142
Typography       43 / 43
Geometry         55 / 55
Effect           25 / 25
```

### 上游保持

以下 Board 全部保持存在且未被 D3-07 修改：

- D2-01 Player Window Shell；
- D2-02 Video Viewport；
- D2-03 Floating Header；
- D2-04 Host Spatial Contract；
- D2-05 Responsive Window Skeleton；
- D3-01 OSC Surface & Internal Grid；
- D3-02 Timeline Basic Geometry；
- D3-03 Timeline Interaction States；
- D3-04 Transport Cluster；
- D3-05 Volume Cluster；
- D3-06 Utility Action Cluster。

## 13. Stage D3 关闭检查

任务书要求：

- OSC / Timeline / Volume / Transport / Utility / Visibility 完整；
- Timeline 响应式成立；
- 显隐规则明确；
- Popup 打开后 OSC 不误消失；
- Main/Fullscreen 不复制第二套 Timeline/OSC 语义；
- 不回归大蓝主按钮与厚重底栏。

当前结果：通过。

D3 的控制对象已经形成稳定的 Component Contract。正式 published Component Set / Variables / Surfaces 全局收口仍按原计划在 D8 完成，不在 D3-07 提前扩展组件库。

**Stage D3：Complete。**

## 14. 下一阶段

`Stage D4 — Inspector 系统与媒体内部结构`

下一 Atomic Task 应从 D4-01 Inspector Shell 开始，消费 D2-04/D2-05/D3-06 已冻结的 Inspector z50、overlay/dock responsive policy、单一 Inspector destination 语义与 D3-07 LockedVisible 规则。