# 第三版 Airy Glass UI 设计任务书包

此分支用于第三版 Airy Glass UI/UX 设计任务书、Figma 设计执行与 Atomic Task 完成记录。

不整合旧 R2–R14 源码开发任务；播放器产品结构按 Window / OSC / Inspector / Overlay / Preferences / Window Modes / Design System 组织。

入口：
- `docs/plans/Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md`
- `docs/plans/stages/00_INDEX.md`
- 根 `README.md`：当前分支唯一 canonical change record。
- `docs/records/`：保留已存在的历史详细事实记录；不再替代根 README 的变更记录职责。

## Current design status

- 第三版核心框架：Main Player / Fullscreen / Playlist Inspector 已确认。
- **D1：Complete — D1-01 ～ D1-06 全部关闭。**
- **D2：Complete — D2-01 ～ D2-05 全部关闭。**
- **D3：Complete — D3-01 ～ D3-07 全部关闭。**
- **D4：Complete — D4-01 ～ D4-07 全部关闭。**
- D3-01：OSC Surface & Internal Grid，Board `90:3`。
- D3-02：Timeline Basic Geometry，Board `95:8`。
- D3-03：Timeline Interaction States，Board `104:2`。
- D3-04：Transport Cluster，Board `117:2`。
- D3-05：Volume Cluster，Board `127:2`。
- D3-06：Utility Action Cluster，Board `134:2`。
- D3-07：OSC Visibility Lifecycle，Board `142:2`。
- D4-01：Inspector Shell，唯一 Shell Component `152:4`。
- D4-02：Inspector Navigation，唯一 Mode Switch Component Set `160:88`。
- D4-03：Playlist Content，唯一 Content Component Set `172:284`。
- D4-04：Tracks Content，唯一 Content Component Set `183:823`。
- D4-05：Subtitles Content，唯一 Content Component Set `191:1565`。
- D4-06：Chapters Content，唯一 Content Component Set `200:2168`。
- **D4-07：Inspector Responsive & Dismissal Contract，Contract `207:2497` / Verification `209:2497` / Prototype Navigation `216:3436`。**
- **下一任务：D5-01 Feedback Priority Matrix。**

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

## Stage D4 — Inspector System · Complete

### D4-01 ～ D4-06 Source Contracts

- `Inspector / Shell` 是唯一 Drawer/Inspector 几何 owner；Header、Mode Switch、Content Host、Footer Host 通过 Slot 组合。
- `Inspector / Mode Switch` 只拥有 Playlist / Tracks / Subtitles / Chapters 单选导航，不拥有 Shell 尺寸。
- Playlist / Tracks / Subtitles / Chapters 均保持独立 Content Component Set，只负责各自内容与局部状态，不重建 Shell。
- D4-04 Track Selection Row 统一 Audio / Video / Subtitle 的 Default / Selected / Pending / Off / Focus 表达。
- D4-05 外挂字幕文件 Pending/Invalid 生命周期与已加载 Subtitle Track selection 分离。
- D4-06 Chapters 明确 Current 与 Pending Jump 可同时存在，避免把 Seek 请求目标伪装成已确认当前章节。

### D4-07 Inspector Responsive & Dismissal Contract

Figma：

```text
Contract              207:2497
Responsive Verification 209:2497
Prototype Navigation   216:3436
```

继续消费 D2-05 既有 Responsive Variable，不新增第二套 breakpoint：

```text
Narrow    0–839     overlay   width 320
Standard  840–1199 overlay   width 368
Wide      >=1200    dock      width 368
```

单一 Shell 适配真实 Player Host：

```text
720×700   Shell 320×450   Content 276×214
960×700   Shell 368×420   Content 324×184
1280×700  Shell 368×584   Content 324×348
```

`Content Host`（`152:11`）统一启用 Vertical Scrolling；Header=`54`、Mode Switch=`42`、Footer=`54` 保持固定，窗口高度收缩只改变 Content 可视区，不反向修改 Window/Header/Video Viewport。

覆盖关系：

```text
720 Overlay Open
Overlay 322×436
OSC     668×106
Closed → Overlay 668×436，OSC 不变

960 Overlay Open
Overlay 512×406
OSC     908×124
Closed → Overlay 908×406，OSC 不变

1280 Dock Open
Overlay 832×406
OSC     832×124
Closed → Overlay 1228×406，OSC 1228×124
```

关闭策略：

```text
Narrow / Standard Overlay
Close        → close
ESC          → close
Outside Click→ close

Wide Dock
Close        → close
ESC          → close
Outside Click→ keep open
```

Mode retention Prototype：

- Playlist / Tracks / Subtitles / Chapters 四个 960×700 screen 的 Shell 均为 `x564 / y92 / 368×420`。
- 四个 Mode 的 Content Host 均为 `324×184 / Vertical`，Footer 均为 `324×54`。
- Mode click 使用 `SMART_ANIMATE 160ms`，只切换 Content/选中态；不改变 open state、Shell x/y/w/h 或 Footer slot。
- Close/ESC/Outside 使用已提交的 page-level Prototype destination；Figma `NAVIGATE` 的 destination 必须是同页顶层 Frame，因此 `D4-07 / Prototype Navigation` 只负责视觉分区，不接管 Prototype screen parent。

最终审计：

```text
Authoritative source count
Inspector / Shell        1
Inspector / Mode Switch  1
Playlist / Content       1
Tracks / Content         1
Subtitles / Content      1
Chapters / Content       1

Responsive token duplicates
breakpoint/narrow-min             1
breakpoint/standard-min           1
breakpoint/wide-min               1
inspector/presentation            1
inspector/affects-osc-range       1
inspector/affects-overlay-range   1
inspector/width                   1
inspector/top                     1

D4-07 generic unnamed residues = 0
```

视觉验证已覆盖：720 Overlay open/closed、960 四 Mode、1280 Dock open/closed；Host guide 使用低透明工程范围，不伪装成真实产品 Surface。

本任务只修改 Figma 设计与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**Stage D4：Complete。**

## Next

**D5-01 — Feedback Priority Matrix**

下一步按 `docs/plans/stages/D5_播放状态与反馈系统.md` 建立：

```text
state / error
  → severity / recoverability
  → Overlay / HUD / Toast / Dialog
```

先冻结反馈层级与唯一主入口，再进入 Empty / Loading / Playing / Paused / Buffering / Seeking / Ended / Error 的具体视觉设计。
