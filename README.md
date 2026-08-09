# 第三版 Airy Glass UI 设计任务书包

此分支用于第三版 Airy Glass UI/UX 设计任务书、Figma 设计执行与 Atomic Task 完成记录。

不整合旧 R2–R14 源码开发任务；播放器产品结构按 Window / OSC / Inspector / Overlay / Preferences / Window Modes / Design System 组织。

入口：
- `docs/plans/Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md`
- `docs/plans/stages/00_INDEX.md`
- `docs/records/`：每个已完成 Design Atomic Task 的独立事实记录；详细实施、异常、修复与验证以这里为准。

## Current design status

- 第三版核心框架：Main Player / Fullscreen / Playlist Inspector 已确认。
- **D1：Complete — D1-01 ～ D1-06 全部关闭。**
- **D2：Complete — D2-01 ～ D2-05 全部关闭。**
- **D3：Complete — D3-01 ～ D3-07 全部关闭。**
- D3-01：OSC Surface & Internal Grid，Board `90:3`。
- D3-02：Timeline Basic Geometry，Board `95:8`。
- D3-03：Timeline Interaction States，Board `104:2`。
- D3-04：Transport Cluster，Board `117:2`。
- D3-05：Volume Cluster，Board `127:2`。
- D3-06：Utility Action Cluster，Board `134:2`。
- **D3-07：OSC Visibility Lifecycle，Board `142:2`。**
- **下一任务：D4-01 Inspector Shell。**

## Stage D1 — Foundations · Complete

- D1-01：建立浅雾 Primitive Color。
- D1-02：建立 Semantic Color，并回刷核心框架；框架 Visible Solid Paint=`142/142` Semantic-bound。
- D1-03：建立 Typography System；框架文本=`43/43` 正式 Text Style。
- D1-04：建立 Spacing / Size / Radius Geometry；核心框架 Geometry target=`55/55`。
- D1-05：建立 Glass / Blur / Shadow；核心框架 Effect target=`25/25`。
- D1-06：建立 Motion / Opacity / Z-order 与 Reduce Motion；Stage D1 关闭。

## Stage D2 — Player Window System · Complete

- D2-01：Player Window Shell；Standard=`1320×700 / R32 / border70% / clipping`，Maximized=`R0 / no elevation`。
- D2-02：Video Viewport；冻结 `Fit + preserve aspect + centered + no stretch + no crop`。
- D2-03：Floating Header；采用 `Media Info Pod + Window Actions Pod`，Playback State 不进入 Header。
- D2-04：Host Spatial Contract；冻结 Overlay z35 / OSC z40 / Inspector z50 及空间碰撞规则。
- D2-05：Responsive Window Skeleton；Breakpoint：Narrow=`0–839`、Standard=`840–1199`、Wide=`>=1200`；冻结 Inspector overlay/dock、OSC compact/standard、Volume 与 Utility 降级策略。

## Stage D3 — OSC System · Complete

### D3-01 OSC Surface & Internal Grid

- OSC width=`min(hostWidth - 52, 880)`。
- Narrow=`106px`，Standard/Wide=`124px`。
- 固定 `Timeline Lane → Control Lane` 两层内部网格。

记录：`docs/records/D3-01_OSCSurface与内部网格.md`。

### D3-02 Timeline Basic Geometry

- Visual Track=`3px`。
- Hit Target=`16px`。
- Thumb=`10px`。
- `0 / 50 / 100% / Unknown / Non-seekable` 与不同宽度均验证。

记录：`docs/records/D3-02_Timeline基础几何.md`。

### D3-03 Timeline Interaction States

- 冻结 Rest / Hover Preview / Scrubbing / Pending Seek / Commit / Cancel / Chapter Hover。
- PlaybackSnapshot 保持 confirmed position 唯一真值；Preview/Scrub/Pending 不提前改写真实位置。

记录：`docs/records/D3-03_Timeline交互状态.md`。

### D3-04 Transport Cluster

- Transport=`116×40 / gap6`。
- Previous/Next=`32×32 hit / 22×22 visual / R16`。
- PlayPause=`40×40 / R20 / V3 Glass Control`。
- Previous≈`+0.9px`、Play≈`+0.6px`、Next≈`-0.9px` 光学补偿。

记录：`docs/records/D3-04_TransportCluster.md`。

### D3-05 Volume Cluster

- `volume` 与 `mute` 独立；0% 不等于 Muted。
- Wide Inline=`138×32 = 32 trigger + 6 gap + 100 slider`。
- Narrow/Standard=`Popover`，Wide=`Inline`。
- 本地 Slider/Mute 不叠 HUD；键盘/媒体键反馈进入 D5 HUD z70。

记录：`docs/records/D3-05_VolumeCluster.md`。

### D3-06 Utility Action Cluster

入口所有权：

```text
Subtitles    → Inspector / Subtitles    → z50
Audio Tracks → Inspector / Audio Tracks → z50
Chapters     → Inspector / Chapters     → z50
Playlist     → Inspector / Playlist     → z50
More         → Popover / More            → z60
Fullscreen   → Window Mode Toggle
```

四类 Inspector Action 共享同一个 D4 Inspector Shell；More 不创建第二个 Inspector；Fullscreen 不消费 Panel Open selection。

Responsive：

```text
Narrow   essential+more  = 108×32
Standard mixed+more      = 146×32
Wide     full            = 222×32
```

记录：`docs/records/D3-06_UtilityActionCluster.md`。

### D3-07 OSC Visibility Lifecycle

Figma：`D3-07 / OSC Visibility Lifecycle`（`142:2`）。

唯一状态解析顺序：

```text
lockReasons > 0          → LockedVisible
directInteraction        → Active
autoHideAllowed = false  → Rest · persistent
now >= hideDeadline      → Hidden
otherwise                → Rest · countdown
```

Lock reasons：

```text
Timeline Scrubbing
FocusWithinOSC
Volume Popover Open
More Popover Open
Inspector Open
```

Paused / Error 不创建 lock，而是关闭 auto-hide，保持 `Rest · persistent`。

唯一新增行为 Token：

```text
motion/osc/hide-delay
Standard      = 2200ms
Reduce Motion = 2200ms
```

Show/Hide 继续复用 D1-06：

```text
Show 160ms · Ease Out
Hide 120ms · Ease In
Reduce Motion = 0ms
```

Visible-state 母材质冻结：

```text
REST = ACTIVE = LOCKEDVISIBLE
OSC Fill   = 32%
OSC Border = 48%
Effect     = V3 / Glass / OSC
```

Hover/Pressed/Focus 只由子控件表达，避免整条 OSC 因 pointer 横穿控件而闪烁。

Keyboard / Media Key 默认不唤醒 OSC，继续走 D3-05 → D5 HUD；只有 Playback 变 Paused 或 Keyboard Focus 进入 OSC 时才由状态机自然改变可见性。

Smoke Prototype：

```text
143:2   REST PLAYING
143:15  HIDDEN
143:23  ACTIVE HOVER
143:38  LOCKED SCRUB
143:53  LOCKED POPOVER
143:69  REST PAUSED
143:82  REST ERROR
143:95  FULLSCREEN SAME POLICY
```

实际 `AFTER_TIMEOUT` owner 只有 `143:2 REST PLAYING`：`2.2s → 143:15 HIDDEN`。Fullscreen Smoke 不创建第二个 timeout，只用显式 trigger 验证同一 policy。

实施修复：

- 修正 Smoke OSC 被 Viewport 裁剪；
- `OSC Surface.clipsContent=false`，允许 z60 Popover 正常浮出；
- Semantic Paint Binding 再次把新材质 opacity 写回 100%，已独立恢复 `182` 个节点目标 alpha，同时保留 Color / Text / Effect / Reaction / Variable / Geometry binding。

最终验证：

```text
D3-07 Board
Visible Solid Paint 316 / 316 Semantic-bound
Text Style          176 / 176
Unbound             0

D3-07 Smoke
Visible Solid Paint 169 / 169 Semantic-bound
Text Style           37 / 37
Unbound              0

Single AFTER_TIMEOUT owner = 1
hide-delay Standard / Reduce Motion = 2200 / 2200ms

D1 Regression
Color       142 / 142
Typography   43 / 43
Geometry     55 / 55
Effect       25 / 25

D2-01 ～ D2-05 保持
D3-01 ～ D3-06 保持
```

记录：`docs/records/D3-07_OSCVisibilityLifecycle.md`。

## Stage D3 Closing Result

任务书要求的 OSC / Timeline / Transport / Volume / Utility / Visibility 已形成稳定 Component Contract；Timeline 响应式与 Popup/Inspector visibility lock 已验证；Main/Fullscreen 不复制第二套生命周期；未回归大蓝主按钮或厚重底栏。

正式 published Component Set / Variables / Surfaces 全局收口仍按原计划在 D8 完成。

**Stage D3：Complete。**

## Next

**D4-01 — Inspector Shell**

下一步建立单一 Inspector Shell，直接消费：

- D2-04：Inspector z50 / Host Spatial Contract；
- D2-05：Narrow/Standard overlay、Wide dock；
- D3-06：Subtitles / Audio / Chapters / Playlist 单一 destination 语义；
- D3-07：Inspector Open → LockedVisible。
