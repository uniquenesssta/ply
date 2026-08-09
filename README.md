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
- **D3：In Progress。**
- D3-01：Complete — OSC Surface & Internal Grid，Board `90:3`。
- D3-02：Complete — Timeline Basic Geometry，Board `95:8`。
- D3-03：Complete — Timeline Interaction States，Board `104:2`。
- D3-04：Complete — Transport Cluster，Board `117:2`。
- D3-05：Complete — Volume Cluster，Board `127:2`。
- **D3-06：Complete — Utility Action Cluster，Board `134:2`。**
- **下一任务：D3-07 OSC 显隐生命周期。**

## Stage D1 — Foundations · Complete

- D1-01：建立浅雾 Primitive Color。
- D1-02：建立 Semantic Color，并回刷核心框架；框架 Visible Solid Paint=`142/142` Semantic-bound。
- D1-03：建立 Typography System；框架文本=`43/43` 正式 Text Style。
- D1-04：建立 Spacing / Size / Radius Geometry；核心框架 Geometry target=`55/55`。
- D1-05：建立 Glass / Blur / Shadow；核心框架 Effect target=`25/25`。
- D1-06：建立 Motion / Opacity / Z-order 与 Reduce Motion；Stage D1 关闭。

## Stage D2 — Player Window System · Complete

- D2-01：Player Window Shell；Standard=`1320×700 / R32 / border70% / clipping`，Maximized=`R0 / no elevation`。
- D2-02：Video Viewport；冻结 `Fit + preserve aspect + centered + no stretch + no crop`，覆盖 16:9 / 21:9 / 4:3 / 9:16 / Audio / Empty。
- D2-03：Floating Header；采用 `Media Info Pod + Window Actions Pod`，Playback State 不进入 Header。
- D2-04：Host Spatial Contract；冻结 Overlay z35 / OSC z40 / Inspector z50，以及 Header/OSC/Inspector 的硬间隔与碰撞规则。
- D2-05：Responsive Window Skeleton；Breakpoint：Narrow=`0–839`、Standard=`840–1199`、Wide=`>=1200`；冻结 Inspector overlay/dock、OSC compact/standard、Volume 与 Utility 的降级策略。

## Stage D3 — OSC System · In Progress

### D3-01 OSC Surface & Internal Grid · Complete

- OSC width=`min(hostWidth - 52, 880)`。
- Narrow=`106px`，Standard/Wide=`124px`。
- 固定 `Timeline Lane → Control Lane` 两层内部网格。

记录：`docs/records/D3-01_OSCSurface与内部网格.md`。

### D3-02 Timeline Basic Geometry · Complete

- Visual Track=`3px`。
- Hit Target=`16px`。
- Thumb=`10px`。
- `0 / 50 / 100% / Unknown / Non-seekable` 与不同宽度均验证。

记录：`docs/records/D3-02_Timeline基础几何.md`。

### D3-03 Timeline Interaction States · Complete

- 冻结 Rest / Hover Preview / Scrubbing / Pending Seek / Commit / Cancel / Chapter Hover。
- PlaybackSnapshot 保持 confirmed position 唯一真值；Preview/Scrub/Pending 不提前改写真实位置。
- Reduce Motion 下 Timeline interaction duration 全部解析到 0ms。

记录：`docs/records/D3-03_Timeline交互状态.md`。

### D3-04 Transport Cluster · Complete

- Transport=`116×40 / gap6`。
- Previous/Next=`32×32 hit / 22×22 visual / R16`。
- PlayPause=`40×40 / R20 / V3 Glass Control`。
- Primary Rest/Hover/Pressed=`48/52/42%`；Focus ring=`82%`；Disabled=`38%`。
- Previous≈`+0.9px`、Play≈`+0.6px`、Next≈`-0.9px` 光学补偿。

记录：`docs/records/D3-04_TransportCluster.md`。

### D3-05 Volume Cluster · Complete

- `volume` 与 `mute` 独立；0% 不等于 Muted。
- Wide Inline=`138×32 = 32 trigger + 6 gap + 100 slider`。
- Slider=`100×16 / 90×3 visual track / 10 thumb`；0/100% endpoint error=`0`。
- Narrow/Standard=`Popover`，Wide=`Inline`。
- Popover=`170×52 / R20 / z60 / V3 Glass Popover`。
- 本地 Slider/Mute 不叠 HUD；键盘/媒体键反馈进入 D5 HUD z70。

记录：`docs/records/D3-05_VolumeCluster.md`。

### D3-06 Utility Action Cluster · Complete

Figma：`D3-06 / Utility Action Cluster`（`134:2`）。

正式入口所有权：

```text
Subtitles    → Inspector / Subtitles    → z50
Audio Tracks → Inspector / Audio Tracks → z50
Chapters     → Inspector / Chapters     → z50
Playlist     → Inspector / Playlist     → z50
More         → Popover / More            → z60
Fullscreen   → Window Mode Toggle        → Enter / Exit Fullscreen
```

四类 Inspector Action 共享同一个 D4 Inspector Shell；同一时间只有一个 Inspector destination active。More 只拥有 overflow popover，不创建第二个 Inspector。Fullscreen 只切换 Window Mode，不消费 Panel Open selection。

Geometry：

```text
Utility Hit Target = 32×32 / R16
Standard/Wide Icon = 22×22
Narrow Icon        = 21×21
Gap                = 6

Narrow Cluster     = 108×32
Standard Cluster   = 146×32
Wide Cluster       = 222×32
```

Responsive：

```text
Narrow / essential+more
Subtitles · More · Fullscreen
More → Audio Tracks / Chapters / Playlist

Standard / mixed+more
Subtitles · Playlist · More · Fullscreen
More → Audio Tracks / Chapters

Wide / full
Subtitles · Audio Tracks · Chapters · Playlist · More · Fullscreen
```

State：

```text
Rest       72%
Hover      100%
Pressed    84%
Focus      82% focus ring
Disabled   38%
Panel Open 66% selection fill / 28% border / icon primary 100%
```

More Popover：

```text
250×176 / R20
Fill 52% / Border 48%
V3 / Glass / Popover
z60
```

Token：D3-06 没有新增 Color / Geometry / Material / Motion Token，完全复用 D1～D3 已冻结系统。

实施问题与修复：Semantic Paint Binding 再次把 Focus / Panel Open / More Popover 及文档辅助材质 opacity 写回 100%；已独立恢复 50 个节点目标透明度，不改变 Semantic Color、Geometry、Route Metadata 或 Effect Style。

最终验证：

```text
Visible Solid Paint 396 / 396 Semantic-bound
Text Style          155 / 155
Unbound             0

D1 Regression
Color       142 / 142
Typography   43 / 43
Geometry     55 / 55
Effect       25 / 25

D2-01 ～ D2-05 保持
D3-01 ～ D3-05 保持
```

记录：`docs/records/D3-06_UtilityActionCluster.md`。

## Next

**D3-07 — OSC 显隐生命周期**

下一任务需要建立 `Hidden / Rest / Active / LockedVisible` 的唯一可交付生命周期，统一 hover、scrub、Volume Popover、More Popover、Inspector、paused、error、keyboard action 与 Fullscreen 下的 OSC 显隐 ownership。D3-07 完成后执行 Stage D3 关闭检查。
