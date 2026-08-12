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
- **D5：Complete — D5-01 ～ D5-07 全部关闭。**
- **D6：Complete — D6-01 ～ D6-06 全部关闭。**
- **D7：Complete — D7-01 ～ D7-05 全部关闭。**
- **D8：Complete — D8-01 ～ D8-07 全部关闭。**
- **D9：In Progress — D9-01 ～ D9-03 Complete；D9-04 待执行。**
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
- D4-07：Inspector Responsive & Dismissal Contract，Contract `207:2497` / Verification `209:2497` / Prototype Navigation `216:3436`。
- D5-01：Feedback Priority Matrix，Page `219:2` / Contract `220:2` / Verification `221:2`。
- D5-02：Empty / Loading，Source `223:2` / Verification `223:3` / Player Status Overlay `225:30`。
- D5-03：Playing / Paused，Contract `234:66` / Verification `234:67` / Prototype Navigation `237:66`。
- D5-04：Buffering / Seeking，Source `241:66` / Verification `241:67` / Buffering Status `242:88` / Seek Preview `243:66`。
- D5-05：Ended，Source `254:119` / Verification `254:120` / Ended Action `255:159` / Ended Status `256:144`。
- D5-06：Error Overlay，Source `262:186` / Verification `262:187` / Error Action `264:225` / Error Status `265:246`。
- D5-07：HUD / Toast / Dialog，Source `271:301` / Verification `271:302` / Prototype Navigation `278:528` / HUD `272:339` / Toast `273:318` / Dialog Action `274:317` / Dialog `274:342`。
- **D6-01：Preferences Window Shell，Page `285:19` / Source `285:20` / Verification `285:21` / Preferences Shell `286:59`。**
- **D6-02：Source List Navigation，Source `295:74` / Verification `295:75` / Source Icon `296:142` / Source Item `298:349` / Source List `299:313`。**
- **D6-03：Settings Section / Row，Source `315:1847` / Verification `315:1848` / Settings Row `336:2293` / Settings Section Header `319:1853`。**
- **D6-04：通用设置控件，Source `328:2160` / Verification `328:2161` / Toggle `329:2190` / Select `330:2187` / Slider `331:2190` / Segmented `332:2265` / Text Field `333:2194`。**
- **D6-05：真实设置内容，Source `343:2625` / Verification `343:2626` / Content Header `344:2627` / Settings Action `345:2641` / Raw Mpv Warning `346:2625` / Playback `347:2626` / Video `348:2659` / Audio `349:2683` / Subtitles `350:2695` / Interface `351:2717` / Diagnostics Export `352:2741` / Advanced `353:2742` / Real Content Windows `354:2759` / Prototype Navigation `358:3747` / Contract `360:4225`。**
- **D6-06：Shortcuts，Source `363:4225` / Verification `363:4226` / Prototype Navigation `363:4227` / Shortcut Search `365:4245` / Keycap `366:4232` / Shortcut Binding `367:4277` / Shortcut Conflict `368:4239` / Shortcut Action Row `369:4292` / Shortcuts Content `370:4471` / Real Shortcuts Windows `371:4419` / Contract `375:5777`。**
- **D7-01：Fullscreen 构图，Page `382:5777` / Source `382:5778` / Verification `382:5779` / Minimal Header `386:16` / Compact OSC Composition `386:21` / Layout Contract `386:71`。**
- **D7-02：Fullscreen 显隐交互，Source `393:2` / Verification `393:3` / Chrome Projection `394:16` / Interaction Scenarios `394:30` / Acceptance Gate `394:80` / Verification Assertions `395:367`。**
- **D7-03：Mini Player，Source `401:47` / Verification `401:48` / Geometry Contract `402:61` / Rest `402:70` / Hover `402:76` / Minimum `402:100` / Acceptance Gate `402:143` / Verification Assertions `403:151`。**
- **D7-04：窄窗口降级，Source `407:47` / Verification `407:48` / Policy Matrix `408:61` / Pressure Ladder `408:95` / Acceptance Gate `408:129` / Geometry Audit `413:203` / Verification Assertions `413:223`。**
- **D7-05：跨模式状态保持，Source `418:203` / Verification `419:203` / Prototype Navigation `420:381` / Transition Matrix `418:218` / Transition Topology `418:252` / Acceptance Gate `418:286` / Verification Assertions `419:1036`。**
- **D8-01：重复结构审计，Page `431:559` / Source `432:2` / Verification `432:175`。**
- **D8-02：Icon 与图标语言，Source `439:2` / Verification `439:4` / Preferences Playback Icon `442:44`。**
- **D8-03：Controls，Source `450:106` / Verification `450:108` / Icon Button `451:154`。**
- **D8-04：Surfaces，Source `470:308` / Verification `470:310` / OSC `474:308` / Inspector `476:308` / Popover `477:308` / HUD `478:308` / Toast `479:308` / Dialog `479:310`。**
- **D8-05：Composite，Source `491:397` / Verification `491:399` / Floating Header `494:10663` / Playlist Row `168:260` / Track Row `179:716` / Settings Row `336:2293` / Source List Item `507:537`。**
- **D8-06：全局实例回刷，Source `529:676` / Verification `529:678`。**
- **D8-07：命名/变量/层级卫生，Source `591:676` / Verification `591:677`。**
- **D9-01：核心播放原型，Page `599:3362` / Source `605:203` / Verification `606:203` / Empty `600:2` / Loading `600:24` / Playing Visible `600:43` / Playing Hidden `600:147` / Paused Persistent `600:166`。**
- **D9-02：Timeline Seek 原型，Page `599:3362` / Source `617:580` / Verification `618:580` / Rest `614:203` / Hover Preview `614:298` / Scrubbing `614:414` / Pending Seek `614:530` / Confirmed Resume `614:647`。**
- **D9-03：Inspector 原型，Page `599:3362` / Source `627:1837` / Verification `627:1891` / Standard Closed `622:939` / Playlist `622:961` / Tracks `623:1062` / Subtitles `623:1363` / Chapters `623:1611` / Narrow Overlay QA `624:3850`。**
- **下一任务：D9-04 Window Mode 原型。**

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

Visible-state母材质冻结：

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
Contract                 207:2497
Responsive Verification  209:2497
Prototype Navigation      216:3436
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
Close         → close
ESC           → close
Outside Click → close

Wide Dock
Close         → close
ESC           → close
Outside Click → keep open
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

## Stage D5 — Playback States & Feedback · Complete

### D5-01 Feedback Priority Matrix · Complete

Figma：

```text
Page                    219:2   05 States & Feedback
Contract                220:2   D5-01 / Feedback Priority Matrix
20 Event Verification   221:2   D5-01 / 20 Event Verification
```

D5-01 只冻结反馈路由规则，不提前创建 D5-02～D5-07 的正式 HUD / Toast / Dialog / Error Overlay 组件。

主路由：

```text
Decision required
  → Dialog

Current-media lifecycle / current-media failure
  → Overlay

Direct timeline seek interaction
  → Seek Preview

Mergeable playback-control feedback
  → HUD

Non-blocking action result
  → Toast

Feature-scoped validation
  → Inline

Stable Playing / Paused transport state
  → Player State / no transient surface
```

继续复用已有 z-order，不创建第二套反馈层：

```text
Overlay      z35
OSC          z40
Inspector    z50
Popover      z60
HUD          z70
Toast        z80
Dialog Scrim z90
Dialog       z100
```

20 个典型事件已全部归类，Primary Route 分布：

```text
Overlay / Error  6
Player State     3
Seek Preview     2
HUD              4
Toast            2
Inline           2
Dialog           1
Total           20 / 20
```

关键路由事实：

- First launch / no media → `Overlay · Empty`。
- Local / network media opening → `Overlay · Loading`。
- Playing → no transient feedback。
- Paused → 复用 D3 `OSC persistent`，不创建巨大暂停 Overlay/HUD。
- Buffering while playing → `Overlay · Buffering`。
- Buffering signal while paused → 只允许 secondary Inline 状态，用户 Pause 意图保持 primary。
- Timeline Scrub / Pending Seek → `Seek Preview`，不伪装成 Buffering。
- Keyboard/media-key Seek、Volume、Speed、Audio/Subtitle Track switch → HUD，连续同类事件原位合并，不排队。
- Screenshot success → Toast。
- EOF without next → Ended Overlay；next/repeat 自动切换时不闪 Ended。
- Current-media network/decode/render failure → Error Overlay。
- Non-current Playlist missing / external subtitle invalid → owning Inspector Inline；只有成为当前媒体失败时才升级为 Error Overlay。
- Settings save failure → Toast。
- Resume playback choice → Dialog。

冲突验证：

```text
Paused + Buffering                     PASS
Loading + current-media Error          PASS
Error Overlay → recovery Dialog        PASS
Repeated Volume / Seek / Speed HUD     PASS
Scoped Invalid → current-media Error   PASS
```

统一冲突规则：

- 同一事件只有一个 Primary Surface。
- Error Overlay 替换同媒体的 Loading/Buffering，不三层并发。
- Dialog 打开后成为消息与动作 primary owner；底层 Error Overlay 只可保留上下文，不重复文案/动作。
- HUD 同类高频反馈原位合并；Toast 不承载连续播放控制。
- Inline 错误保持 feature-local；只有影响当前媒体时才全局升级。

最终结构审计：

```text
05 States & Feedback sections = 2
D5-01 event rows            = 20
Primary route populated     = 20 / 20
Premature Components        = 0
New feedback variables      = 0
Generic unnamed residues    = 0
```

审计命中的 `motion/hud/*`、`motion/toast/*`、`motion/dialog/*`、`opacity/dialog/scrim` 均为 D1 已存在的基础变量，本任务没有新增或修改这些 Variable。

本任务只新增 Figma Page/Section 与根 README 事实记录；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-01：Complete。**

### D5-02 Empty / Loading · Complete

Figma：

```text
Source / Components      223:2   D5-02 / Empty & Loading
Verification             223:3   D5-02 / Verification
Open Media Action        224:24  Feedback / Open Media Action
Loading Indicator        224:25  Feedback / Loading Indicator
Player Status Overlay    225:30  Player Status Overlay
```

继续消费 D5-01 已冻结路由：

```text
no media              → Overlay · Empty
local/network loading → Overlay · Loading
```

单一 `Player Status Overlay` 只定义：

```text
State=Empty
State=Loading
```

不提前加入 Playing / Paused / Buffering / Seeking / Ended / Error，也不创建平行 Overlay。

Empty：

- 无额外大卡片；在 `surface/empty` Player 背景上只保留小型 Media Glyph、标题、说明、Open Media CTA 与拖入提示。
- `Feedback / Open Media Action`=`144×42`，State=`Default / Hover / Focus / Pressed`；Focus=`1.5px` 可见焦点环。
- Open Media 是 Empty 唯一主动作，不拥有文件对话框实现或播放器状态真值。

Loading：

- `Feedback / Loading Indicator`=`28×28`，只表达媒体准备中，不表达 Buffering 百分比。
- Loading 使用 `290×65` Compact Status Pod；Pod 背景继续绑定 `overlay/contrast-support`，通过独立背景层 opacity=`0.46` 保持语义绑定与透明度稳定，文字使用 `text/inverse`。
- 不使用中央大 Spinner 卡片，不发 Toast/Dialog，不提前定义 Buffering 进度。

真实 Host 验证：

```text
First Launch          230:13   960×700   Empty    Overlay 908×406 @ 26,106
Local Loading         230:32   960×700   Loading  Overlay 908×406 @ 26,106
Network Loading       231:34   960×700   Loading  Overlay 908×406 @ 26,106
Pure Audio Loading    231:56   960×700   Loading  Overlay 908×406 @ 26,106
Narrow Empty          231:91   720×700   Empty    Overlay 668×436 @ 26,102
```

验证事实：

- Network Loading 在高亮/暗色/暖色拼接背景上仍保持可读。
- Pure Audio 只替换媒体背景上下文；不复制第二套 Player Window 或 Status Overlay。
- 720 Narrow 的 Empty CTA、文案和提示无横向溢出。
- Header / OSC 只作为 D2 Host guide；D5-02 不拥有或修改其几何。

最终审计：

```text
Feedback / Open Media Action  1 authority
Feedback / Loading Indicator  1 authority
Player Status Overlay         1 authority
Player Status Overlay states  2 / 2 (Empty, Loading)
Unexpected D5-03+ components  0
New feedback variables        0
Generic unnamed residues      0
```

所有源组件可见 Solid Paint 均保持 Semantic-bound；D5-01 Contract / Verification 坐标与尺寸未改变。

本任务只修改 Figma 设计与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-02：Complete。**

### D5-03 Playing / Paused · Complete

Figma：

```text
Contract              234:66   D5-03 / Playing & Paused Contract
Verification          234:67   D5-03 / Verification
Prototype Navigation  237:66   D5-03 / Prototype Navigation
Prototype / Playing   237:69
Prototype / Paused    237:99
```

继续消费 D5-01 与 D3-07 已冻结状态规则：

```text
Playing → Player State · no transient feedback
Paused  → Player State · OSC persistent
```

Playing：

- Transport 使用 Pause glyph；播放状态本身不创建 Overlay/HUD/Toast。
- OSC 处于 `Rest · countdown`；由 D3-07 唯一 Visibility Controller 持有 `hideDeadline = now + 2200ms`，到期后进入 Hidden。
- Header / Video Viewport 几何与内容 ownership 不因 Playing 改变。

Paused：

- Transport 使用 Play glyph；PlayPause 几何继续保持 `40×40 / R20 / V3 Glass Control`。
- OSC 处于 `Rest · persistent`；`autoHideAllowed=false`，`hideDeadline=none`。
- 若 Pause 发生时 OSC 已 Hidden，先按既有 `160ms Show` 恢复 OSC，再保持 persistent。
- 不创建中央巨大暂停 glyph、Paused Overlay 或 Paused HUD。

Playing / Paused 共享不变项：

```text
Player Window / Video Viewport geometry unchanged
Floating Header geometry unchanged
OSC visible material = 32% fill / 48% border / V3 Glass OSC
D5-02 Player Status Overlay remains Empty / Loading only
No second transport / OSC / visibility timer owner
```

真实验证：

```text
Playing Visible      236:69   960×700   OSC 856×124 @ 52,538   Pause glyph
Playing Hidden       236:100  960×700   OSC absent
Pause from Hidden    236:110  960×700   OSC 856×124 @ 52,538   Play glyph
Paused Persistent    236:140  960×700   OSC 856×124 @ 52,538   Play glyph
Resume Playing       236:170  960×700   OSC 856×124 @ 52,538   Pause glyph
Narrow Paused        236:201  720×700   OSC 616×106 @ 52,564   Play glyph
```

快速切换 Prototype：

- `237:69 Playing ↔ 237:99 Paused`。
- PlayPause click 与 Space 键均双向切换。
- 切换使用 `SMART_ANIMATE 120ms Ease Out`；只验证 transport state，不创建 D5-03 timer。
- D5-03 `AFTER_TIMEOUT owner = 0`；2200ms 自动隐藏仍只属于 D3-07。

实施中发现并修复：

- Paused 首轮 Play polygon 因 Figma rotation 基准产生肉眼可见的光学偏移；已替换为居中的 `16×18` SVG Play glyph，并保持 40×40 PlayPause 几何不变。
- Prototype hit 区 semantic-bound 后 Paint alpha 被 Figma 归一化；通过绑定 `surface/glass` + node opacity=`0.001` 保持不可见命中区。
- 清理 SVG 默认 `Vector` 命名和 Figma Section 默认未绑定 outline stroke。

最终审计：

```text
D5-03 new Product Components   0
D5-03 new Variables            0
D5-03 AFTER_TIMEOUT owners     0
Generic unnamed residues       0
Visible unbound UI paints      0
Player Status Overlay states   Empty / Loading only
```

D5-01 Contract / Verification 与 D5-02 Source / Verification 坐标、尺寸全部保持不变。

本任务只修改 Figma 设计与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-03：Complete。**

### D5-04 Buffering / Seeking · Complete

Figma：

```text
Source / Contract          241:66   D5-04 / Buffering & Seeking
Verification               241:67   D5-04 / Verification
Buffering Status           242:88   Feedback / Buffering Status
Seek Preview               243:66   Feedback / Seek Preview
Player Status Overlay      225:30   existing authority, extended with State=Buffering
Buffering Overlay Variant  244:66   State=Buffering
```

继续消费 D5-01 与 D3-03 已冻结路由和时间线真值：

```text
Buffering while Playing       → Overlay · Buffering
Buffering while user Paused   → Inline · secondary only
Timeline Scrub                → Seek Preview · Scrub
Released Seek Pending         → Seek Preview · Pending
```

Buffering：

- `Feedback / Buffering Status` 只有 `Mode=Known / Unknown` 两种语义 Variant，均为 `296×66` Compact Contrast Glass Pod。
- Known 显示真实动态 percentage + 3px progress；源组件默认以 `10%` 作为样例。Percentage 是运行时数据，不建立 10/50/100 等离散 Variant。
- Unknown 使用 indeterminate activity cue，不显示伪造百分比。
- Buffering ring 只作为 activity cue，不编码百分比；真实缓冲进度由文字与 3px progress 表达。
- Buffering 进入唯一 `Player Status Overlay`；Overlay 保持 z35，OSC 保持 z40，因此状态反馈不会吞掉控制系统。

Seeking：

- `Feedback / Seek Preview`=`64×28 / R14 / V3 Glass Control`，正式暴露 `Time` 文本属性。
- Seek Preview 只拥有 target time；不拥有 Timeline Progress、Thumb、Pending Range 或 PlaybackSnapshot。
- Scrubbing：Progress/Thumb 临时投影到 pointer target，并保留弱化 Committed Marker 作为拖动前 confirmed origin。
- Pending Seek：confirmed Progress/Thumb 恢复/保持真实位置；请求目标使用 Pending Range + Hollow Pending Target，直到 backend position confirmation 后才 commit。
- 不创建中央 Seeking Overlay，也不把 Pending Seek 伪装成 network Buffering。

Paused + Buffering 冲突：

- Paused Player State 继续为 primary，Play glyph 与 `Rest · persistent` OSC 不变。
- Buffering 只允许 OSC 内 secondary inline cue；验证场景中的 Player Status Overlay Count=`0`。
- 因此 backend buffering signal 不会覆盖用户明确 Pause 意图。

真实验证：

```text
Buffering 10%       247:76    960×700   Overlay 1   Seek Preview 0   OSC 856×124
Buffering 50%       247:118   960×700   Overlay 1   Seek Preview 0   OSC 856×124
Buffering Unknown   247:161   960×700   Overlay 1   Seek Preview 0   OSC 856×124
Paused + Buffering  248:108   960×700   Overlay 0   Inline secondary   OSC 856×124
Seeking Scrubbing   249:108   960×700   Overlay 0   Seek Preview 1    OSC 856×124
Seeking Pending     249:143   960×700   Overlay 0   Seek Preview 1    OSC 856×124
```

50% 验证说明：Figma Instance 允许文字 override，但不能覆盖内部 progress bar geometry；因此 50% 只在 Verification 层增加动态进度扩展，不把百分比离散化为产品 Variant。产品源仍严格只有 Known / Unknown。

Seek 数值验证：

```text
Confirmed Progress            357.28
Scrub target Progress         552.16
Scrub Committed Marker x      379.28
Scrub target Thumb x          569.16

Pending confirmed Progress    357.28
Pending Range x / width       379.28 / 194.88
Confirmed Thumb x             374.28
Hollow Pending Target x       569.16
Seek Preview                  64×28
```

所有 Seek 真值继续归 D3-03：

```text
PlaybackSnapshot → confirmed playback position
Pointer Drag     → temporary scrub target
Seek Request     → pending target
Backend confirm  → commit actual Progress / Thumb
```

D5-04 没有复制第二套 Timeline 状态机、Reaction 或 Timer；D3-03 Board/Smoke 保持原样。

最终审计：

```text
Feedback / Buffering Status authorities   1
Feedback / Seek Preview authorities       1
Player Status Overlay authorities         1
Player Status Overlay states              Empty / Loading / Buffering
D5-04 Reactions                            0
D5-04 AFTER_TIMEOUT owners                 0
New D5-04 Variables                        0
Generic unnamed residues                   0
Visible unbound source/product paints      0
D5-02 source section fit after extension   PASS
D3-03 authority regression                 PASS
```

D5-01～D5-03 的既有 Section 坐标和尺寸全部保持不变；D5-02 的 Empty/Loading Variant 位置与 `512×406` 尺寸保持不变，扩展后的 `Player Status Overlay` 仍完整落在 D5-02 Source Section 内。

本任务只修改 Figma 设计与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-04：Complete。**

### D5-05 Ended · Complete

Figma：

```text
Source / Contract          254:119  D5-05 / Ended
Verification               254:120  D5-05 / Verification
Ended Action               255:159  Feedback / Ended Action
Ended Status               256:144  Feedback / Ended Status
Player Status Overlay      225:30   existing authority, extended with State=Ended
Ended Overlay Variant      257:131  State=Ended
```

继续消费 D5-01 已冻结 EOF 路由：

```text
EOF / no automatic handoff + no next  → Overlay · Ended → Replay
EOF / manual Next available           → Overlay · Ended → Replay + Next
Repeat active                         → Player State · no transient Ended
Auto-next active                      → Player State · no transient Ended
```

Ended Action：

- `Feedback / Ended Action` 定义 `Action=Replay/Next × State=Default/Hover/Focus/Pressed`，共 8 个 Variant；每个为 `118×42`。
- Focus stroke=`1.5px`，继续复用既有 Glass Control / Focus / Selection Token；不使用高饱和主按钮。
- Replay / Next 只发出 playback intent，不拥有 Playlist 顺序、repeat mode、自动 handoff 或 playback state 真值。
- `Feedback / Open Media Action` 继续保持 Empty-only 语义，没有被误复用为 EOF 动作。

Ended Status：

- `Feedback / Ended Status` 只有 `Next=No / Yes` 两态，均为 `336×150` Compact Contrast Glass Pod。
- `Next=No` 只显示 Replay；`Next=Yes` 只增加一个手动 Next，不增加第三个动作、菜单或 Dialog。
- 文案为 `播放结束 / 已到达媒体末尾`，使用 neutral contrast glass + cyan information marker；不进入 Error / Warning 色彩路由。

Player Status Overlay：

- 唯一 `Player Status Overlay` 现为 `Empty / Loading / Buffering / Ended` 四态。
- Component Set 保持 `1044×832`；原 Empty / Loading / Buffering 的 `512×406` 尺寸和位置保持不变，Ended 填入第二行右侧 `x532 / y426`。
- 扩展后仍完整落在 D5-02 Source Section：`right=1464 / bottom=1192`，均在 `1480×1500` 内。
- Ended 只叠加在现有媒体最后一帧/背景上，不替换 Video Viewport 或 Player Window 几何。

真实验证：

```text
Ended Single Item       258:141  960×700  Overlay 1  Replay          final frame preserved
Ended Next Available    258:175  960×700  Overlay 1  Replay + Next   final frame preserved
Auto Handoff / Repeat   258:223  960×700  Overlay 0                  no Ended flash
Narrow Ended            258:233  720×700  Overlay 668×436 @ 26,102   Replay
```

验证中的 Ended 场景保持 OSC hidden，用于确认最小 EOF surface；D5-05 不创建新的 OSC persistent lifecycle。实际交互仍可通过 D3 既有 visibility rules 唤醒 OSC，D3 继续是唯一 OSC 生命周期 owner。

最终审计：

```text
Feedback / Ended Action authorities        1
Feedback / Ended Status authorities        1
Player Status Overlay authorities          1
Player Status Overlay states               Empty / Loading / Buffering / Ended
D5-05 Reactions                             0
D5-05 AFTER_TIMEOUT owners                  0
New D5-05 Variables                         0
Generic unnamed residues                    0
Visible unbound source/product paints       0
D5-02 source section fit                    PASS
D5-01～D5-04 protected sections            PASS
```

所有 8 个 EOF Action Variant 均保持 `118×42`，两种 Focus Variant 均为 `1.5px` Focus stroke；Single/Next/Auto/Narrow 四条路径已完成结构和视觉验证。

本任务只修改 Figma 设计与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-05：Complete。**

### D5-06 Error Overlay · Complete

Figma：

```text
Source / Contract          262:186  D5-06 / Error Overlay
Verification               262:187  D5-06 / Verification
Error Action               264:225  Feedback / Error Action
Error Status               265:246  Feedback / Error Status
Player Status Overlay      225:30   existing authority, extended with State=Error
Error Overlay Variant      266:201  State=Error
```

继续消费 D5-01 与 D3-07 已冻结规则：

```text
retryable current-media failure       → Overlay · Error → Retry
non-recoverable decode/render failure → Overlay · Error → Open Media
unknown current-media failure         → Overlay · Error → Retry + Open Media
Loading / Buffering → Error           → replace previous state, never stack
Error + decision required             → D5-07 Dialog becomes Primary
Error OSC policy                      → D3-07 Rest · persistent
```

Error semantic token：

- 新增且仅新增一个 `feedback/error` semantic color variable：`VariableID:262:185`。
- WEB code syntax=`var(--v3-feedback-error)`；scope=`FRAME_FILL / SHAPE_FILL / TEXT_FILL / STROKE_COLOR`。
- 初始 alias 到既有 `color/warm/400`（与 warning 当前共享 warm primitive），没有新增红色 Primitive 或第二套 Error palette。
- Error 与 Warning 语义 owner 已分离，后续 D8 可独立调整而无需改写 Error 组件。

Error Action：

- `Feedback / Error Action` 定义 `Action=Retry/OpenMedia × State=Default/Hover/Focus/Pressed`，共 8 个 Variant。
- 控件高度统一 `42px`；Focus stroke=`1.5px`，继续复用 Glass Control / Focus / Selection language。
- Retry / Open Media 只发恢复 intent，不拥有错误分类、播放状态、文件对话框或 Dialog routing。
- 按钮保持浅色玻璃，错误色只用于局部 icon，不创建红色实心主按钮。

Error Status：

- `Feedback / Error Status` 只有 `Recovery=Retryable / NonRecoverable / Unknown` 三态，均为 `360×190` Compact Contrast Glass。
- Retryable：`无法加载媒体 / 连接或资源暂时不可用 / 检查网络后重试` → Retry。
- NonRecoverable：`无法播放此媒体 / 当前格式、解码或渲染路径不可用 / 请尝试打开其他媒体` → Open Media。
- Unknown：`播放出现问题 / 当前媒体无法继续播放 / 可先重试，若仍失败请打开其他媒体` → Retry + Open Media。
- 不展示后端错误码、堆栈或内部异常字符串；产品层只表达用户能理解的失败与恢复路径。
- Pod 本体继续使用 neutral contrast glass；warm error 只落在 `28×28` marker / title hierarchy / action icon，不使用整圈红框或大红底。

Player Status Overlay：

- 唯一 `Player Status Overlay` 现为 `Empty / Loading / Buffering / Ended / Error` 五态。
- Error Variant=`266:201`，保持 `512×406`，位于第三行 `x0 / y852`；原四态位置与尺寸未改变。
- Component Set 从 `1044×832` 扩展为 `1044×1258`。
- 为容纳唯一 Overlay authority 的第三行，D5-02 Source Section 只做必要高度扩容：`1480×1500 → 1480×1660`；Player Status Overlay bottom=`1618`，仍完整落在该 Section 内。

真实验证：

```text
Retryable Error       267:219  960×700  Overlay 908×406 @ 26,106  Recovery=Retryable      OSC 856×124
NonRecoverable Error  267:278  960×700  Overlay 908×406 @ 26,106  Recovery=NonRecoverable OSC 856×124
Unknown Error         267:352  960×700  Overlay 908×406 @ 26,106  Recovery=Unknown        OSC 856×124
Narrow Error          267:431  720×700  Overlay 668×436 @ 26,102  Recovery=Retryable      OSC 616×106
```

Collision / ownership 验证：

- Loading → Error：Error 替换 Loading；current-media Overlay count 保持 1。
- Buffering → Error：Error 替换 Buffering；不叠两个 status pod。
- Error → Decision：如果恢复需要用户选择，D5-07 Dialog 成为 Primary；Error Overlay 仅保留上下文，不能重复同一文案/动作。
- Error + Toast：同一个媒体失败不得再触发重复 failure Toast。
- D5-06 不创建正式 Dialog、Scrim、Escape policy、HUD 或 Toast Component。

D3-07 回归：

- Error 继续关闭 OSC auto-hide，使用 `Rest · persistent`。
- D5-06 只消费该 visibility policy，不新增 lock reason、hide deadline、Reaction 或 timer owner。
- 960 Error 验证 OSC=`856×124`；720 Narrow Error OSC=`616×106`。

最终审计：

```text
Feedback / Error Action authorities         1
Feedback / Error Status authorities         1
Player Status Overlay authorities           1
Player Status Overlay states                Empty / Loading / Buffering / Ended / Error
feedback/error semantic variables           1
D5-06 Reactions                              0
D5-06 AFTER_TIMEOUT owners                  0
Premature formal HUD/Toast/Dialog comps      0
Generic unnamed residues                    0
Visible unbound source/product paints        0
D5-02 source section fit after extension     PASS
D5-01～D5-05 protected sections              PASS
```

D5-01、D5-02 Verification、D5-03、D5-04、D5-05 的既有 Section 坐标和尺寸全部保持不变；唯一历史区结构变化是 D5-02 Source Section 高度 `1500 → 1660`，原因是扩展同一个 `Player Status Overlay` authority，而不是建立平行 Error Overlay。

本任务只修改 Figma 设计、一个 Figma semantic token 与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-06：Complete。**

### D5-07 HUD / Toast / Dialog · Complete

Figma：

```text
Source / Contract          271:301  D5-07 / HUD Toast Dialog
Verification               271:302  D5-07 / Verification
Prototype Navigation       278:528  D5-07 / Prototype Navigation
HUD                        272:339  Feedback / HUD
Toast                      273:318  Feedback / Toast
Dialog Action              274:317  Feedback / Dialog Action
Dialog                     274:342  Feedback / Dialog
```

正式反馈 authority：

- `Feedback / HUD`：`Kind=Volume/Seek/Speed/Track` 共 4 个 Variant，每个 `272×72`。同类事件原位更新 value/content 并重置 hide deadline；不同 Kind 直接替换当前 HUD；不建立可见历史或垂直队列。
- Volume / Seek 使用 3px value/progress；Speed / Track 保持信息型，不强塞无意义进度。
- `Feedback / Toast`：`Tone=Success/Failure` 两态，每个 `320×64`。Toast 只有一个 slot，新结果 replace/merge，不建立右上角垂直通知列表；current-media Error 不进入 Toast。
- `Feedback / Dialog Action`：`Role=Primary/Secondary × State=Default/Hover/Focus/Pressed` 共 8 个 Variant，每个 `116×42`，Focus stroke=`1.5px`。
- `Feedback / Dialog`：`Kind=Resume/ErrorRecovery` 两态，每个 `420×226`；Dialog 无自动 timeout，ESC 走 secondary/cancel path，动作只发 intent。

Motion / z-order 继续完全复用 D1 既有基础：

```text
HUD    show 160ms Ease Out / hide 120ms Ease In / z70
Toast  show 200ms Ease Out / hide 160ms Ease In / z80
Dialog open 240ms Ease Out / close 180ms Ease In
Scrim  18% / z90
Dialog z100
Reduce Motion = 0ms transitions
```

D5-07 没有新增 z-order、motion、color、radius 或 dwell timeout Variable。当前 Foundation 没有 HUD/Toast dwell token，因此 Prototype 只使用 QA 样例：HUD=`1200ms after last event`、Toast=`2800ms`；这两个数值不是 Foundation token，后续如需正式 token 化统一在 D8 收口。

真实验证：

```text
HUD Volume           276:306  960×700  one HUD slot
HUD Seek             276:365  960×700  one HUD slot
HUD Speed            276:433  960×700  one HUD slot
HUD Track            276:500  960×700  one HUD slot
Toast Success        276:566  960×700  screenshot success
Toast Failure        276:622  960×700  settings save failure
Dialog Resume        276:686  960×700  scrim 1 / dialog 1
Dialog ErrorRecovery 276:745  960×700  error context + scrim + dialog primary
Narrow HUD           276:814  720×700  HUD 272px fits
Narrow Toast         276:873  720×700  Toast 320px fits
Narrow Dialog        276:929  720×700  Dialog 420px fits
```

ErrorRecovery ownership：

- Dialog 打开后成为 primary decision owner；底层 Error Overlay 只保留上下文，经过 18% scrim 降级且底层恢复动作不可交互。
- Dialog 不复读 Error Overlay 的同一失败文案/动作；只询问真正需要决策的下一步。
- 同一 current-media failure 不再产生 duplicate failure Toast。

Prototype：

```text
Idle                 278:529
HUD Volume 40        278:570
HUD Volume 50        278:611
HUD Volume 60        278:652
Toast Success        278:693
Dialog Resume        278:724
Dialog ErrorRecovery 278:756
Error Context        279:517
```

- Idle 的 ArrowUp 进入 Volume 40；连续 ArrowUp 推进 `40 → 50 → 60`，用于验证同一 HUD slot 原位更新和 deadline 重置。
- HUD 40/50/60 的 QA sample dwell=`1.2s`，随后按既有 Hide `120ms Ease In` 回到 Idle。
- Toast QA sample dwell=`2.8s`，Hide=`160ms Ease In`。
- Resume / ErrorRecovery Dialog 的 `AFTER_TIMEOUT=0`；ESC 与 Secondary 使用 close `180ms Ease In`。ErrorRecovery ESC/Secondary 返回 Error Context，Primary 完成决策。
- Prototype destination 均为 Page-level Frame，满足 Figma `NAVIGATE` 顶层 destination 约束；`D5-07 / Prototype Navigation` 只承担视觉组织。

最终审计：

```text
Feedback / HUD authorities                  1
Feedback / Toast authorities                1
Feedback / Dialog Action authorities        1
Feedback / Dialog authorities               1
HUD variants                                4
Toast variants                              2
Dialog Action variants                      8
Dialog variants                             2
Dialog AFTER_TIMEOUT owners                 0
New D5-07 Variables                         0
Generic unnamed residues                    0
Visible unbound source/product paints       0
720 Narrow HUD / Toast / Dialog fit         PASS
D5-01～D5-06 protected sections             PASS
Player Status Overlay states                Empty / Loading / Buffering / Ended / Error
```

实施收口：

- Prototype 首轮因 Instance 文本 override 前未加载 `Noto Sans SC Medium` 被 Figma 原子拒绝；没有遗留半成品，补齐字体后重新创建并通过。
- 清理 18 个 SVG 默认 `Vector` 子节点，按 HUD/Toast/Dialog 语义改名，并绑定现有 `border/glass / surface/glass / feedback/info / feedback/warning / accent/strong / feedback/error`。
- 4 个不可见 Dialog Prototype 命中层绑定 `surface/glass`，node opacity=`0.001`，既保持不可见又不遗留硬编码 Paint。

本任务只修改 Figma 设计与根 README；没有源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-07：Complete。**

## Stage D5 Closing Result

任务书要求的 Empty / Loading / Playing / Paused / Buffering / Seeking / Ended / Error，以及 Seek Preview / HUD / Toast / Dialog 已全部形成单一职责 authority。Feedback Priority、current-media Overlay ownership、Seek 真值、OSC persistent、单槽 HUD/Toast、Dialog 决策 ownership 与复杂背景可读性均已完成验证；没有新增第五种反馈层，也没有把暂停、错误或恢复设计成第二套播放器页面。

**Stage D5：Complete。**

## Stage D6 — Preferences & Shortcuts · Complete

### D6-01 Preferences Window Shell · Complete

Figma：

```text
Page                    285:19  06 Preferences
Source / Contract       285:20  D6-01 / Preferences Window Shell
Verification            285:21  D6-01 / Verification
Preferences Shell       286:59  Preferences / Shell
Standard Variant        286:2   Size=Standard
Narrow Variant          286:21  Size=Narrow
Minimum Variant         286:40  Size=Minimum
```

D6-01 只建立独立 Preferences Window 的几何与窗口材质 owner，不提前创建 Source List Item、Settings Row、Toggle/Select/Slider/TextField 或 Shortcuts 内容。

唯一 `Preferences / Shell` 定义三档逻辑尺寸：

```text
Standard  980×680   Source Host 220   Content Host 759   Titlebar 58
Narrow    820×620   Source Host 190   Content Host 629   Titlebar 58
Minimum   760×560   Source Host 190   Content Host 569   Titlebar 58
```

Shell ownership：

- Window Surface。
- Titlebar / `偏好设置`。
- Window Actions Host；当前只作为 Shell 私有 chrome，不把播放器 Window Actions 复制成第二套业务组件。
- Source List Host。
- Content Host。
- Source / Content 单一 divider。
- Minimum size 与 resize contract。

D6-02+ 只能向 Source List Host / Content Host 填入各自内容，不得反向拥有或重新定义窗口尺寸、Titlebar 或两个 Host 的几何。

Foundation / Token：

- 新增 8 个 Size Primitive：`size/980 / 680 / 820 / 620 / 760 / 560 / 220 / 190`。
- 新增 9 个 Preferences Semantic Geometry：
  - `size/preferences/window-width-standard`
  - `size/preferences/window-height-standard`
  - `size/preferences/window-width-narrow`
  - `size/preferences/window-height-narrow`
  - `size/preferences/window-min-width`
  - `size/preferences/window-min-height`
  - `size/preferences/titlebar-height`
  - `size/preferences/source-width-standard`
  - `size/preferences/source-width-compact`
- 所有 semantic geometry 均在既有 Geometry Semantic collection，scope=`WIDTH_HEIGHT`，并设置 WEB code syntax。
- 新增 Color / Effect / Typography / Radius / Motion Token=`0`；材质继续复用 `surface/glass / surface/glass-subtle / surface/canvas / border/glass / V3 Elevation Window`。

High-DPI contract：

- High-DPI 不创建第四个 Variant，也不改变逻辑尺寸。
- Verification 使用 Standard Variant 的 `2×` render：实例 `289:116 = 1960×1360`，仅验证边界、字体、图标、阴影在 2× 下的视觉清晰度。

真实验证：

```text
Standard  289:4    stage 1060×770   instance 289:8    980×680
Narrow    289:28   stage 900×710    instance 289:32   820×620
Minimum   289:70   stage 840×650    instance 289:74   760×560
High DPI  289:112  stage 2040×1450  instance 289:116  1960×1360 (2× Standard)
```

四个 Verification 实例全部直接消费 `Preferences / Shell`（`286:59`）；没有复制第二套 Window、Player Overlay、OSC 或 Inspector Shell。

实施收口：

- 首次 Component 创建脚本因 async 函数声明遗漏而语法失败；该调用没有成功提交产品组件，修正执行结构后重新创建。
- 第一轮 Source 视觉复核发现 Component Set Variant 重叠与 Close glyph 旋转基准偏斜；已将 Variant 按两行组织，并将 Close 改为居中 SVG X，子路径职责化命名并绑定 `feedback/neutral`。

最终审计：

```text
Preferences / Shell authorities          1
Preferences Shell variants               3
Preferences semantic geometry variables  9
New primitive size variables             8
New Color / Effect / Text / Motion       0
Generic unnamed residues                 0
Visible unbound source/product paints    0
Standard / Narrow / Minimum instances    PASS
High-DPI 2× visual                       PASS
D1～D5 page structure regression         PASS
Player Status Overlay states             Empty / Loading / Buffering / Ended / Error
```

本任务只修改 Figma 设计、Figma geometry variables 与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D6-01：Complete。**

### D6-02 Source List Navigation · Complete

Figma：

```text
Source / Contract       295:74   D6-02 / Source List Navigation
Verification            295:75   D6-02 / Verification
Source Icon             296:142  Preferences / Source Icon
Source Item             298:349  Preferences / Source Item
Source List             299:313  Preferences / Source List
Preferences Shell       286:59   existing authority, integrated with nested Source List
```

分类结构：

```text
Playback   播放
Video      视频
Audio      音频
Subtitles  字幕
Interface  界面
Advanced   高级
Shortcuts  快捷键
```

Source ownership：

- `Preferences / Shell` 继续唯一拥有窗口尺寸、Titlebar、Source List Host 宽度与 Content Host；D6-02 没有重新定义窗口几何。
- `Preferences / Source List` 只拥有 7 类分类顺序、列表 padding/gap 与 Standard/Compact 横向密度。
- `Preferences / Source Item` 只拥有 Category + interaction/selection state。
- `Preferences / Source Icon` 只拥有分类 glyph 与 Default/Selected tone。
- Content Host 仍属于后续 D6-03+；D6-02 的 Verification 只使用轻量 content target placeholder，不提前实现 Settings Section/Row。

Source Icon：

- `Preferences / Source Icon` 共 `7 Category × 2 Tone = 14` Variant。
- 每个 icon=`18×18`；Default 使用现有 `icon/secondary`，Selected 使用 `selection/indicator`。
- Category glyph 不因选中状态改变几何，只改变语义 tone。

Source Item：

- `Preferences / Source Item` 共 `7 Category × 5 State = 35` Variant。
- Row height=`42px`，icon=`18×18`，左右 inset=`12px`，icon-label gap=`10px`。
- State contract：

```text
Default       → no wash
Hover         → soft selection wash only
Selected      → soft full-row wash + slim indicator
Focus         → 1.5px focus ring only, selection unchanged
SelectedFocus → selected wash + indicator + focus ring
```

- Focus 与 Selected 独立表达；键盘焦点可以移动而不改变当前选中分类。
- 整行 Selection 使用既有 `selection/background / selection/indicator / focus/ring`，没有新建 Preferences 私有选中色，也没有高饱和后台式左栏。

Source List：

```text
Standard  220×390   padding 16   item 188×42
Compact   190×390   padding 12   item 166×42
```

- Compact 同时供 Narrow / Minimum Shell 使用。
- 7 项保持快速扫读；Shortcuts 仅做轻微视觉间隔，不形成第二分组面板或第二窗口。
- 新增 D6-02 Variable / Text Style / Effect / Motion=`0`。

D6-01 集成：

- Figma Plugin API 当前没有可创建的原生 Slot API，因此没有用视觉叠加假装 Slot，也没有创建平行 Preferences Shell。
- Source List 是常驻 Preferences chrome，不是随内容切换的动态 host；因此把共享 `Preferences / Source List` 作为嵌套实例直接放入 D6-01 的 `Source List Host`。
- 三档 Shell source 均只嵌套一个 Source List：

```text
Standard  286:2   Host 220×621  → Source List Standard 220×390  instance 302:295
Narrow    286:21  Host 190×561  → Source List Compact  190×390  instance 302:347
Minimum   286:40  Host 190×501  → Source List Compact  190×390  instance 302:399
```

- Shell 仍是 geometry owner；Source List 仍是 navigation owner。该集成没有改变 980/820/760 窗口尺寸、Titlebar 58 或 Source Host 220/190 真值。

真实验证：

```text
Standard Playback Selected         303:658
Standard Playback + Video Hover    304:726
Standard Interface Focus           305:797
Standard Advanced SelectedFocus    305:877
Narrow Shortcuts Selected          307:937   Shell 820×620 / List 190×390
Minimum Audio SelectedFocus        307:1028  Shell 760×560 / List 190×390
```

- Standard / Narrow / Minimum 均直接消费同一个 `Preferences / Shell` 与嵌套 `Preferences / Source List`。
- Narrow/Minimum 的 item=`166×42`，最长分类标签“快捷键”无挤压、无裁切、无横向溢出。
- Focus ring 在浅色窗口背景中清晰；SelectedFocus 同时保留当前选择与键盘焦点。

Prototype：

7 个 page-level 分类 Selected screen：

```text
Playback   308:1077
Video      308:1162
Audio      308:1247
Subtitles  308:1336
Interface  308:1422
Advanced   308:1508
Shortcuts  308:1597
```

- 每个 Selected screen 对其余 6 个分类均有 `ON_CLICK → NAVIGATE`，当前已选分类不创建自跳转。
- 分类 click 使用 `SMART_ANIMATE 160ms`，只改变 Source selection 与 Content target；Preferences Window 保持打开，Shell geometry 不变。

Keyboard Prototype：

```text
Playback SelectedFocus  308:1685
Video Focus             308:1770
Audio Focus             308:1859
Video SelectedFocus     308:1952
```

行为冻结为：

```text
Arrow Up / Down → move Focus only
Enter / Space   → commit focused category as Selected
Focus move      → current Content unchanged
Selection commit→ Content target changes
```

- `Playback SelectedFocus → ArrowDown → Video Focus`：证明焦点移动不会改选中分类。
- Video Focus 可 ArrowUp/Down 在相邻项移动；Enter/Space 后才进入 Video SelectedFocus。
- Audio Focus Enter/Space 进入 Audio selected category screen。
- D6-02 `AFTER_TIMEOUT owner=0`。

最终审计：

```text
Preferences / Shell authorities       1
Preferences / Source Icon authorities 1
Preferences / Source Item authorities 1
Preferences / Source List authorities 1
Shell variants                         3
Source Icon variants                  14
Source Item variants                  35
Source List variants                   2
D6-02 new navigation variables         0
Prototype AFTER_TIMEOUT owners         0
Generic unnamed residues               0
Visible unbound source/product paints  0
Standard / Narrow / Minimum fit        PASS
D6-01 geometry regression              PASS
D1～D5 protection regression           PASS
Player Status Overlay states           Empty / Loading / Buffering / Ended / Error
```

已在 Figma Source 写入 Source Ownership / Selection State / Keyboard Navigation / D6-03+ Non-goal Contract，并在 Verification 建立 12 条 Acceptance Assertions，全部 PASS。

本任务只修改 Figma 设计、D6-01 Shell 的嵌套组件组合与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D6-02：Complete。**

### D6-03 Settings Section / Row · Complete

Figma：

```text
Source / Contract          315:1847  D6-03 / Settings Section & Row
Verification               315:1848  D6-03 / Verification
Settings Row               336:2293  Preferences / Settings Row
Settings Section Header    319:1853  Preferences / Settings Section Header
Standard Density           320:1847  D6-03 / Standard Density
Long Description Boundary  321:1956  D6-03 / Long Description Boundary
Narrow Density             324:1963  D6-03 / Narrow Density
Minimum Density            324:2117  D6-03 / Minimum Density
```

Section composition：

```text
Settings Section
  = Settings Section Header
  + N × Settings Row
```

- `Preferences / Settings Section Header` 只拥有 section title 与可选 supporting text，不固定 Row 数量，也不拥有真实设置内容。
- Header Variant=`Supporting=No / Yes`，共 2 个；暴露 `Title / Supporting Text` TEXT properties。
- Section 本体不使用卡片背景；依靠标题、留白与连续 Row divider 建立层级，避免 Preferences 变成后台卡片系统。

Settings Row：

- `Preferences / Settings Row` 只表示一个设置概念；复杂工作流必须升级独立组件/Dialog，不能继续向 Row 内增加无关字段。
- Variant 轴保持正交：

```text
Body    = Compact / Description
Status  = Normal / Disabled / RestartRequired
Total   = 6 variants
```

- 暴露 `Title / Description / Auxiliary` TEXT properties。
- D6-04 为建立真实模块化控件接缝，将旧 `318:1907` 无损迁移为当前唯一 authority `336:2293`；Row 几何、6 个 Variant、文字属性和所有 D6-03 Verification 内容保持不变。
- 当前 Row 暴露 `Control` INSTANCE_SWAP property，默认使用内部不可见 `Preferences / Control Placeholder`（`336:2160`）；接口尺寸固定 `180×32`。
- D6-04 的 Toggle/Select/Slider/Segmented/Text Field 必须通过该 `Control` property 进入 Row，不允许视觉叠加或在各分类复制控件实现。
- 每个 Variant 均只有一个 Control instance 与一个底部 Divider。

Density：

```text
Compact      min-height 56px
Description  min-height 72px
```

- 长描述不是第三个 Variant；Description 使用真实 Hug Content。
- 3+ 行边界验证中，Description 文本高度增长后 Row 从 `72 → 87px`，divider 不覆盖正文。
- RestartRequired 使用局部 `feedback/warning` marker + `重启播放器后生效` auxiliary text；不使用黄底整行或警告卡片。
- Disabled 使用既有 muted/disabled hierarchy，不复制第二套控件实现。

真实窗口宽度验证：

```text
Standard
Content Host   759
Reading width  695
Text width     491
Control        180×32

Narrow
Content Host   629
Reading width  573
Text width     369
Control        180×32

Minimum
Content Host   569
Reading width  513
Text width     309
Control        180×32
```

- Narrow 4 个 Row 分别验证 Compact / Description / RestartRequired / Disabled；最高 Row=`76px`。
- Minimum 中普通多行 Description=`73px`；RestartRequired + 多行描述自动增长到 `90px`。
- 最小窗口仍维持“设置概念 + 右侧 Control”的桌面 Preferences 结构，没有退化为网页表单式纵向控件堆叠。

Ownership / escalation：

- Row 只拥有 Title、Description、Auxiliary、Divider、Control placement。
- 若设置需要多个独立字段、文件选择、校验流程、向导或破坏性确认，必须升级为独立 Component/Dialog。
- D6-03 Verification 的 Content composition 只验证 Section/Row 密度；D6-05 才拥有各分类真实 Content，D6-04 拥有真实设置控件。

D6-04 integration migration：

- 由于已经位于 Component Set 内的 Variant 无法事后新增 `INSTANCE_SWAP` property，D6-04 没有保留旧 Row 再叠一层兼容壳。
- 实施方式为原子无损迁移：将 6 个旧 Row Variant 转换为带 `Control INSTANCE_SWAP` 的新组件，再组合为新的唯一 Component Set。
- 13 个既有 Standard/Narrow/Minimum/Long Description Verification Row 实例全部按同名 Variant 自动 `swapComponent`，并保留 Title/Description/Auxiliary overrides 与原实例尺寸。
- 旧 authority `318:1907` 已删除；当前 authority 只有 `336:2293`。

最终审计：

```text
Preferences / Settings Row authorities            1
Preferences / Settings Section Header authorities 1
Settings Row variants                              6
Settings Section Header variants                   2
Settings Row Control INSTANCE_SWAP                 1 per variant / 180×32
D6-03 Reactions                                    0
D6-03 AFTER_TIMEOUT owners                         0
New D6-03 Variables                                0
New D6-03 Text Styles                              0
Generic unnamed residues                           0
Visible unbound source/product paints              0
Long description Hug                               PASS (87px)
Standard / Narrow / Minimum density                PASS
D6-01 / D6-02 protected sections                   PASS
D6-02 Prototype screens                            11, unchanged
Player Status Overlay states                       Empty / Loading / Buffering / Ended / Error
```

Figma Source 已写入 Section Composition / Row Ownership / Density & Status / Escalation Contract 与 9/9 Acceptance Gate。

本任务只修改 Figma 设计与根 README；D6-04 的 Row authority migration 仅改变 Figma 组件接缝，不改变播放器源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D6-03：Complete。**

### D6-04 通用设置控件 · Complete

Figma：

```text
Source / Contract             328:2160  D6-04 / Preferences Controls
Verification                  328:2161  D6-04 / Verification
Toggle                        329:2190  Preferences / Toggle
Select                        330:2187  Preferences / Select
Slider                        331:2190  Preferences / Slider
Segmented                     332:2265  Preferences / Segmented
Text Field                    333:2194  Preferences / Text Field
Control Placeholder           336:2160  Preferences / Control Placeholder (internal)
Settings Row                  336:2293  existing authority, migrated for Control INSTANCE_SWAP
State Matrix                  337:2160  D6-04 / State Matrix
True Row Integration          337:2299  D6-04 / Row Integration
Responsive Window Verify      338:2310  D6-04 / Responsive Window Verification
Prototype Navigation          340:2601  D6-04 / Prototype Navigation
Acceptance Gate               341:2645  D6-04 / Acceptance Gate
```

本任务继续消费 D6-03 `setting concept → row → control` 边界：控件只负责本地 value/state 呈现与交互，不拥有 Settings Row、Preferences Shell、持久化、restart routing 或播放器状态。

Toggle：

```text
Value = Off / On
State = Default / Hover / Pressed / Focus / Disabled
Total = 10 variants
Outer = 48×32
```

- On 使用柔和 `selection/background`，不是高饱和网页开关。
- Pressed 只做轻微几何压缩：track `44×26 → 42×24`，thumb `20 → 18`。
- Focus 使用 `1.5px focus/ring`；Disabled 使用既有 `opacity/control/disabled`。
- Toggle 不建立 Error Variant；布尔值的无效/不可用语义归 owning setting/row feedback。

Select：

```text
State = Default / Hover / Pressed / Focus / Disabled / Error
Total = 6 variants
Size  = 180×32
Value = TEXT property
```

- 继续使用 `V3 / Glass / Field`。
- Error 只使用局部 warm border + `6px` marker，不使用红底/错误面板。
- Select 只展示当前 value 和下拉 affordance；option model、popover 内容和值提交归 owning setting feature。

Slider：

```text
State = Default / Hover / Pressed / Focus / Disabled
Total = 5 variants
Size  = 180×32
```

- 直接复用 D3-05 Slider anatomy：`16px hit target / 3px visual track / 10px resting thumb`。
- Hover thumb=`12px`，Pressed thumb=`14px`；Focus 不修改 committed value。
- Preferences 只扩展可用横向空间，不建立第二套 slider state/value model。
- 示例 Value=`60%` 为 TEXT property；真实数值/progress geometry 由 setting value owner 驱动。

Segmented：

```text
Selection = First / Second / Third
State     = Default / Hover / Pressed / Focus / Disabled
Total     = 15 variants
Size      = 180×32
```

- `First Label / Second Label / Third Label` 均为 TEXT properties。
- Selection 表示 committed value；Hover/Pressed 只作用于一个未选 candidate segment，同时保留当前 selection。
- 仅用于短小的互斥值；更大 option set 使用 Select，避免 Preferences 出现大量网页式 tab/form 控件。

Text Field：

```text
Content = Placeholder / Filled
State   = Default / Hover / Pressed / Focus / Disabled / Error
Total   = 12 variants
Size    = 180×32
```

- 暴露 `Text / Placeholder` TEXT properties。
- Placeholder 与 Filled 是内容呈现语义，不是业务状态；两者保持不同文本层级。
- Focus 使用 `1.5px focus/ring` + local caret；Error 使用局部 warm border + marker。
- Validation meaning 归 owning setting；Text Field 只负责本地输入/错误 affordance。

Settings Row integration：

- D6-03 `Preferences / Settings Row` 现暴露唯一 `Control#336:27` INSTANCE_SWAP property，默认内部占位组件=`336:2160`。
- 每个 Row Variant 只有 1 个 `Control` instance，几何始终 `180×32`。
- D6-04 Verification 已将五种控件全部通过真实 INSTANCE_SWAP 注入 Row：Toggle / Select / Slider / Segmented / Text Field；没有视觉叠加。
- Internal `Preferences / Control Placeholder` 无 fill/stroke，仅用于 Figma property plumbing，不是第六个可见产品控件。

State Matrix：

- 五类控件统一验证 Default / Hover / Pressed / Focus / Disabled / Error。
- Toggle / Slider / Segmented 的 Error 明确为 N/A，不为满足矩阵机械增加无语义 Variant。
- Select / Text Field 的 Error 为 local validation presentation，不升级成 D5 Error Overlay。

真实窗口验证：

```text
Standard
Shell 980×680
Rows  695 wide
Control 180×32

Narrow
Shell 820×620
Rows  573 wide
Control 180×32

Minimum
Shell 760×560
Rows  513 wide
Control 180×32
```

- 三档都消费同一个 Preferences Shell、Settings Row 与五个 control authority。
- Minimum 下仍保持横向“setting concept + control”阅读型结构，没有变成网页表单的上下堆叠。

Prototype：

```text
Toggle Off        340:2604
Toggle On         340:2611
Segment First     340:2618
Segment Second    340:2629
Select Default    340:2640
Select Focus      340:2648
Text Field Focus  340:2656
```

行为：

- Toggle：Click / Space 双向 Off ↔ On。
- Segmented：Click / Right Arrow 将 First → Second；Left Arrow 将 Second → First。
- Select：Click / Tab 从 Default → Focus。
- Select Focus：Tab → Text Field Focus。
- Text Field Focus：ESC → Select Focus。
- 所有 destination 都是 Page-level Frame，使用 `SMART_ANIMATE 120ms Ease Out`。
- D6-04 `AFTER_TIMEOUT owner=0`。

Foundation / style：

- 新增全局 Color / Primitive / Effect / Text / Motion Variable/Style=`0`。
- 继续复用 `surface/glass-subtle / selection/background / border/glass / border/selection / focus/ring / feedback/error / control/track / control/progress / control/thumb / opacity/control/disabled`。
- Effect 继续复用 `V3 / Glass / Control` 与 `V3 / Glass / Field`。
- Slider 不可见 hit target 已从硬编码透明白收口为 semantic-bound `surface/glass + node opacity=0.001`。
- Select SVG Chevron 外层默认白 fill 已清空；内部 Chevron Path 使用 semantic-bound stroke。

最终审计：

```text
Preferences / Toggle authorities            1
Preferences / Select authorities            1
Preferences / Slider authorities            1
Preferences / Segmented authorities         1
Preferences / Text Field authorities        1
Preferences / Settings Row authorities      1
Preferences / Control Placeholder           1 internal
Toggle variants                              10
Select variants                               6
Slider variants                               5
Segmented variants                           15
Text Field variants                          12
Settings Row variants                         6
Settings Row Control INSTANCE_SWAP            1 per variant / 180×32
D6-04 Prototype screens                       7
D6-04 Prototype reactions                    12
D6-04 AFTER_TIMEOUT owners                    0
Non-page Prototype destinations               0
New D6-04 global Variables / Styles           0
Generic unnamed residues                      0
Visible unbound source/product paints         0
Standard / Narrow / Minimum fit               PASS
D6-01 / D6-02 / D6-03 section regression     PASS
D6-02 Prototype screens                       11, unchanged
Player Status Overlay states                  Empty / Loading / Buffering / Ended / Error
```

Figma Source 已写入 Value Ownership / State Responsibility / Row Integration / Slider Continuity / D6-05 Boundary / Avoid Web Form Contract；Verification 为 **12/12 Acceptance PASS**。

本任务只修改 Figma 设计、D6-03 Row 的组件接缝与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D6-04：Complete。**

### D6-05 填充 Playback / Video / Audio / Subtitles / Interface / Advanced · Complete

Figma：

```text
Source / Contract            343:2625  D6-05 / Real Settings Content
Verification                 343:2626  D6-05 / Verification
Content Header               344:2627  Preferences / Content Header
Settings Action              345:2641  Preferences / Settings Action
Raw Mpv Warning              346:2625  Preferences / Raw Mpv Warning
Playback Content             347:2626  Preferences / Playback Content
Video Content                348:2659  Preferences / Video Content
Audio Content                349:2683  Preferences / Audio Content
Subtitles Content            350:2695  Preferences / Subtitles Content
Interface Content            351:2717  Preferences / Interface Content
Diagnostics Export           352:2741  Preferences / Diagnostics Export
Advanced Content             353:2742  Preferences / Advanced Content
Real Content Windows         354:2759  D6-05 / Real Content Windows
Prototype Navigation         358:3747  D6-05 / Prototype Navigation
Capability / Ownership       360:4225  D6-05 / Contract
Acceptance Gate              360:4238  D6-05 / Acceptance Gate
```

D6-05 不再使用漂亮空壳验证 Preferences，而是以任务书与项目既定模块为边界，把六个普通设置分类填入真实内容。六个分类分别拥有独立 Content authority；Content 只负责组合，不重新拥有 Shell、Source List、Settings Row 或 D6-04 control 几何。

真实设置范围：

```text
Playback
  Hardware Decode           → Select · 自动
  Resume Playback           → Toggle · On
  Cache                     → Select · 自动

Video
  Scaling                   → Select · 自动
  Render Quality            → Disabled · future enhancement

Audio
  Preferred Audio Language  → Text Field · zh, en

Subtitles
  Preferred Subtitle Lang   → Text Field · zh, en
  Subtitle Style            → Disabled · future enhancement

Interface
  Theme                     → Select · 默认
  Control Visibility        → Select · 自动隐藏

Advanced
  Raw mpv options           → warning + action entry
  Diagnostics export        → dedicated action workflow
```

能力边界：

- Playback 只放 `Hardware Decode / Resume Playback / Cache`，不把现场播放控制塞入 Preferences。
- Video 的 `Render Quality` 与 Subtitles 的 `Subtitle Style` 属于项目已定义的第二阶段增强能力；D6-05 用 `Disabled Row + Disabled Select` 明确保留位置，不伪装成当前已经可配置。
- Audio 只提供首选音频语言，不复制 D3 Volume/Mute，也不复制 D4 Audio Delay。
- Subtitles 的实时 Track selection、外挂字幕加载与 delay 继续归 D4 Inspector；Preferences 只保存首选语言/未来样式偏好。
- Interface 的 `ThemeSetting` 在项目资料中没有冻结具体主题枚举，因此本稿只使用中性示例值 `默认`，不自行扩展 Light/Dark/System 等未确认枚举。
- `Control Visibility` 直接消费 D3 已冻结的 OSC auto-hide 生命周期；D6-05 不创建第二个 visibility owner。
- Shortcuts 不属于 D6-05 内容填充范围，继续留给 D6-06。

共享 Content Header：

- `Preferences / Content Header`=`695×53`，只拥有分类标题和一段简介。
- 使用既有 `V3 / Title / Inspector` 与 `V3 / Body / Control`；不形成第二个窗口标题，也不拥有 Settings Section / Row。

Settings Action：

```text
Tone  = Neutral / Warning
State = Default / Hover / Pressed / Focus
Total = 8 variants
Size  = 116×32
```

- 只用于触发工作流，不用于编辑 setting value。
- Neutral 用于 Diagnostics Export；Warning 用于 Raw mpv 入口。
- Warning 保持浅玻璃控件，仅使用 warm border/text 表达风险，不使用大红底或破坏性主按钮视觉。

Raw mpv：

- `Preferences / Raw Mpv Warning`=`695×85`。
- 独立承担“底层选项可能导致播放失败、渲染异常或与产品设置冲突”的风险说明。
- Raw mpv 选项不直接以普通 Settings Row 形式铺开，也不在 D6-05 内创建原始参数编辑器。
- Warning 组件只拥有风险 copy、局部 marker 与“打开原始选项”入口；真实 option model、validation、persistence 与 mpv command execution 仍属于实现层。

Diagnostics：

- `Preferences / Diagnostics Export`=`695×73`。
- Diagnostics export 是 action workflow，不是 setting value；因此没有强塞入 D6-03 `180×32 Control` 接口。
- 组件只拥有说明与“导出”动作组合；诊断数据生成、隐私脱敏、文件 I/O 与结果反馈不由 Figma 组件持有。
- 导出成功/失败的非阻塞结果继续路由到 D5 Toast。

Feedback / persistence ownership：

```text
Local Select/Text Field invalid → D6-04 local Error
Restart required                → D6-03 Row Status
Settings save failure           → D5 Toast
Current-media playback failure  → D5 Error Overlay
Persistence success/state       → implementation setting model
```

D6-05 Content 只展示当前 setting value 与状态，不将 Figma 控件实例当作持久化真值。

真实窗口验证：

```text
Playback Standard    Shell 980×680   Content 695×353
Video Standard       Shell 980×680   Content 695×281
Audio Standard       Shell 980×680   Content 695×209
Subtitles Standard   Shell 980×680   Content 695×281
Interface Standard   Shell 980×680   Content 695×281
Advanced Standard    Shell 980×680   Content 695×307

Playback Narrow      Shell 820×620   Content 573×353
Playback Minimum     Shell 760×560   Content 513×353
```

- 六个 Standard 分类都在原 Preferences Shell 内成立，没有改变 Titlebar / Source List / Window geometry。
- Narrow Playback 三个 Row 均为 `573px`，Minimum 均为 `513px`；所有 Row 的 Control interface 始终为 `180×32`。
- Minimum 仍保持桌面式“setting concept + right control”结构，没有退化成网页表单纵向堆叠。
- Advanced 真窗口确认 raw mpv warning 与 Diagnostics 两条职责可同时存在，且不形成后台式大警告面板。

D6-02 Prototype 保护与 D6-05 Prototype：

- 首轮曾尝试把 D6-05 真实 Content instance 直接插入旧 D6-02 分类 Prototype Frame。
- 结构审计显示新 Content instance 自身几何与 Verification 完全一致，但旧 D6-02 交互容器中出现大面积异常白层与内部布局漂移；该视觉 QA 失败，因此没有将其描述为通过。
- 为避免强行兼容旧验证容器，已完整恢复 D6-02 原 7 个分类 Prototype：六个被尝试修改的 screen 均恢复 `2` 个 Content Target placeholder，每屏保留原 `6` 个 category Reaction，`realD605=0`；Shortcuts 本来就未修改。
- D6-05 另建独立 page-level Prototype destinations，并直接复用视觉验证已经通过的 Real Content Window 组合。

D6-05 Prototype：

```text
Prototype Navigation  358:3747
Playback              358:3748
Video                 358:3753
Audio                  358:3758
Subtitles             358:3763
Interface             358:3768
Advanced              358:3773
Shortcuts placeholder 358:3778
```

- 7 个 destination 全部是 Page-level Frame。
- 每个 screen 对其他 6 个分类提供 `ON_CLICK → NAVIGATE`；总计 `42` 个分类 Reaction。
- 使用 `SMART_ANIMATE 160ms Ease Out`。
- D6-05 `AFTER_TIMEOUT owner=0`。
- Shortcuts screen 仍只承载 D6-06 placeholder，不提前创建搜索、Keycap、冲突或编辑状态。
- Prototype hit target 使用 semantic-bound `surface/glass + node opacity=0.001`，没有不可见硬编码 Paint。

Foundation / style：

- D6-05 新增全局 Variable=`0`。
- D6-05 新增 Text Style=`0`。
- D6-05 新增 Effect Style=`0`。
- D6-05 新增 Motion Token=`0`。
- 全部继续消费既有 Airy Glass semantic colors、Typography 与 Glass Control/Field effect。

最终审计：

```text
Preferences / Shell authorities                    1
Preferences / Source List authorities              1
Preferences / Settings Section Header authorities  1
Preferences / Settings Row authorities              1
Preferences / Toggle authorities                    1
Preferences / Select authorities                    1
Preferences / Slider authorities                    1
Preferences / Segmented authorities                 1
Preferences / Text Field authorities                1

Preferences / Content Header authorities            1
Preferences / Settings Action authorities           1
Preferences / Raw Mpv Warning authorities           1
Preferences / Diagnostics Export authorities        1
Preferences / Playback Content authorities          1
Preferences / Video Content authorities             1
Preferences / Audio Content authorities             1
Preferences / Subtitles Content authorities         1
Preferences / Interface Content authorities         1
Preferences / Advanced Content authorities          1

Render Quality future state                         Disabled / PASS
Subtitle Style future state                         Disabled / PASS
Visible unbound source/product paints               0
Generic unnamed residues                            0
New D6-05 global Variables / Text / Effect / Motion 0
D6-05 Prototype screens                             7
D6-05 Prototype reactions                          42
D6-05 AFTER_TIMEOUT owners                          0
Non-page Prototype destinations                     0
D6-02 protected prototypes                          PASS
D6-01～D6-04 protected section geometry             PASS
Player Status Overlay states                        Empty / Loading / Buffering / Ended / Error
Acceptance Gate                                     12 / 12 PASS
```

D6-01～D6-04 Source/Verification Section 坐标和尺寸均保持不变；D5 唯一 `Player Status Overlay` 仍只有 `Empty / Loading / Buffering / Ended / Error` 五态。

本任务只修改 Figma 设计与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D6-05：Complete。**

### D6-06 Shortcuts · Complete

Figma：

```text
Source / Contract            363:4225  D6-06 / Shortcuts
Verification                 363:4226  D6-06 / Verification
Prototype Navigation         363:4227  D6-06 / Prototype Navigation
Shortcut Search              365:4245  Preferences / Shortcut Search
Keycap                       366:4232  Preferences / Keycap
Shortcut Binding             367:4277  Preferences / Shortcut Binding
Shortcut Conflict            368:4239  Preferences / Shortcut Conflict
Shortcut Action Row          369:4292  Preferences / Shortcut Action Row
Shortcuts Content            370:4471  Preferences / Shortcuts Content
Real Shortcuts Windows       371:4419  D6-06 / Real Shortcuts Windows
Contract                     375:5777  D6-06 / Contract
Acceptance Gate              375:5790  D6-06 / Acceptance Gate
```

D6-06 只建立快捷键搜索、动作列表、绑定显示/捕获、冲突和恢复默认体验，不创建第二个 Preferences Window，也不把快捷键直接定义成 mpv command 字符串。未来实现真值继续遵守 R10 的单一 Action identity：

```text
Key event
  → Shortcut Dispatcher / focus context
  → ActionId / Action Registry
  → Application action / PlaybackCommand
```

当前源码仍处于 pre-R10 阶段，因此 D6-06 不把 Figma 演示按键声明为已经实现的 runtime 默认值。

Shortcut Search：

```text
Content = Placeholder / Filled
State   = Default / Focus
Total   = 4 variants
Size    = 320×36
```

- Search 只拥有 query presentation 与 focus。
- Search/Text Field Focus 是受保护文本输入上下文；输入字符不得同时触发播放器普通 shortcut dispatch。
- Search 不拥有 Action Registry、过滤实现、binding 或持久化。

Keycap：

```text
Tone  = Default / Active / Conflict
Total = 3 variants
Height= 28
```

- 一个 Keycap 只表示一个键位 token。
- Conflict 除 warning border 外还有独立 `!` 标记，因此即使去色仍能理解异常状态。
- Chord、Capture、Conflict 对象与持久化均由更高层 owner 管理。

Shortcut Binding：

```text
Chord = Single / Combo
State = Default / Custom / Capturing / Conflict
Total = 8 variants
Size  = 180×32
```

- Single/Combo 都复用同一个 `Preferences / Keycap`。
- Capturing 使用 focus/capture 状态，并明确 `按键中…`；此时键盘输入被解释为 candidate binding，不执行播放器 Action。
- Conflict 同时具备 Conflict Keycap、`冲突`文字和外部 Conflict explanation，不依赖颜色。
- Combo Variant 作为组合键能力独立存在，不为了冲突样例强行制造组合键。

Shortcut Conflict：

- `Preferences / Shortcut Conflict`=`360×34`。
- 默认文案样例：`与「全屏」使用相同按键，请更换其中一个绑定。`
- 组件只解释冲突对象与处理意图，不拥有 validation algorithm 或 persistence。
- QA 时发现首轮 Conflict 样例为 `Ctrl+F ↔ F`，与“相同按键”文案不一致；已修正为真正的 `F ↔ F`，Combo Variant 继续独立保留。

Shortcut Action Row：

```text
State = Default / Custom / Capturing / Conflict
Total = 4 variants
Width = responsive 695 / 573 / 513
```

- Row 只拥有动作名称、说明、一个共享 Shortcut Binding、divider 与可选 Conflict explanation。
- 不复用普通 Settings Row 的 RestartRequired / Disabled 业务状态来伪装快捷键编辑。
- Runtime ActionId 仍是动作身份 owner；Row 不是 Action Registry。

Shortcuts Content：

```text
View = Default / Search / Capturing / Conflict / Custom
Total = 5 variants
Source size = 695×520
Action List overflow = Vertical
```

Default 只用当前规划/源码能支持的核心动作做设计验证：

```text
播放 / 暂停
后退 Seek
前进 Seek
全屏
```

Figma 中显示的 `Space / Left / Right / F / K` **全部只是 QA sample bindings**，用于验证 Keycap、搜索、Capture、Conflict、Custom 与 Restore Default；它们不是 D6-06 冻结的产品默认键位。真正默认值由未来 R10 Action Registry / shortcut settings implementation 决定。

任务书五态验证：

```text
无搜索     → View=Default
搜索       → View=Search · query=全屏
冲突       → View=Conflict · F ↔ F + explicit copy
自定义     → View=Custom · QA sample K
恢复默认   → Custom → Default
附加编辑态 → View=Capturing
```

冲突/提交规则：

- candidate binding 必须先 validate，再 commit。
- 同一活动上下文发生 duplicate binding 时，旧 committed binding 保持不变，新 candidate 进入 local Conflict。
- 非法/不支持输入保持 Shortcut Editor local feedback，不升级成 D5 current-media Error Overlay。
- Restore Default 只发送 reset intent；实际 default binding 由 settings/registry owner 提供。
- D6-06 不扩展 macros、profiles、scripts 或第二套 Action Registry。

真实 Preferences Window 验证：

```text
Standard Default     Shell 980×680   Content 695×520   Row 695   Binding 180×32
Standard Search      Shell 980×680   Content 695×520
Standard Capturing   Shell 980×680   Content 695×520
Standard Conflict    Shell 980×680   Content 695×520
Standard Custom      Shell 980×680   Content 695×520
Narrow Default       Shell 820×620   Content 573×462   Row 573   Binding 180×32
Minimum Default      Shell 760×560   Content 513×402   Row 513   Binding 180×32
```

- 三档窗口全部直接复用唯一 `Preferences / Shell` 与嵌套 `Preferences / Source List`。
- Narrow/Minimum 只缩小 Content viewport；Shortcut Action List 使用 Vertical scrolling，交互目标与 Binding 高度不缩小。
- Minimum 验证已正确将 Source selection 从 Audio SelectedFocus 切为 Shortcuts Selected，不修改 Source List authority。

D6-06 Prototype：

```text
Default    373:5228
Search     373:5366
Capturing  373:5467
Conflict   373:5568
Custom     373:5685
```

交互主链：

```text
Default → Search
Default → Capturing
Capturing + F → Conflict
Capturing + K → Custom
Conflict + K → Custom
Custom → Restore Default → Default
Search / Capturing / Conflict + ESC → cancel/default
```

- 每个 destination 都是 Page-level Frame。
- 分类导航继续使用既有 160ms Smart Animate；Shortcut edit/search flow 使用 120ms Smart Animate。
- 总计 `41` 个 Prototype Reaction。
- `AFTER_TIMEOUT owner=0`。
- Non-page Prototype destination=`0`。
- 不可见 hit target 继续使用 semantic-bound `surface/glass + node opacity=0.001`。

D6-05 / D6-02 集成：

- D6-05 的 Playback / Video / Audio / Subtitles / Interface / Advanced 六个真实分类 screen，其 Shortcuts destination 已统一切到 D6-06 Default `373:5228`；每屏原 6 个分类 Reaction 保持不变。
- D6-05 历史 Shortcuts placeholder `358:3778` 保留为 D6-05 阶段事实，不再作为六个普通分类的真实 Shortcuts destination。
- D6-02 七个历史分类 Prototype 完全未改：每屏仍为 6 reactions，D6-06 residue=`0`。

Foundation / style：

- 新增 D6-06 全局 Variable=`0`。
- 新增 D6-06 Text Style=`0`。
- 新增 D6-06 Effect Style=`0`。
- 新增 D6-06 Motion Token=`0`。
- 继续消费 `surface/glass / surface/glass-subtle / border/glass / selection/background / focus/ring / feedback/warning / text/* / V3 Glass Control / V3 Glass Field`。

最终审计：

```text
Preferences / Shortcut Search authorities       1   variants 4
Preferences / Keycap authorities                1   variants 3
Preferences / Shortcut Binding authorities      1   variants 8
Preferences / Shortcut Conflict authorities     1
Preferences / Shortcut Action Row authorities   1   variants 4
Preferences / Shortcuts Content authorities     1   variants 5

Visible unbound source/product paints           0
Generic unnamed residues                        0
New D6-06 Variable / Text / Effect / Motion     0
Standard / Narrow / Minimum fit                 PASS
Conflict non-color communication                PASS
D6-06 Prototype reactions                       41
D6-06 AFTER_TIMEOUT owners                      0
Non-page Prototype destinations                 0
D6-05 category → Shortcuts retarget              6 / 6 PASS
D6-02 protected prototypes                      PASS
D6-01～D6-05 protected section geometry          PASS
Existing Shell / Source List / Settings Row /
D6-04 control authorities                       1 each
Player Status Overlay states                    Empty / Loading / Buffering / Ended / Error
Acceptance Gate                                 14 / 14 PASS
```

本任务只修改 Figma 设计、D6-05 Prototype 的 Shortcuts destination 与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D6-06：Complete。**

## Stage D6 Closing Result

Preferences Shell、Source List、Settings Section/Row、通用设置控件、六类真实设置内容以及 Shortcuts 的搜索/编辑/冲突/恢复默认已经形成完整且单一职责的设计系统。Standard/Narrow/Minimum 均保持桌面 Preferences 信息结构；快捷键没有绕过未来 Action Registry 直接绑定 mpv command，也没有为了冲突和编辑状态复制第二套窗口或普通 Settings Row。

**Stage D6：Complete。**

## Stage D7 — Window Modes & Responsive · Complete

### D7-01 Fullscreen 构图 · Complete

Figma：

```text
Page                       382:5777  07 Window Modes
Source / Contract          382:5778  D7-01 / Fullscreen Composition
Verification               382:5779  D7-01 / Verification
Minimal Header             386:16    Fullscreen / Minimal Header
Minimal Header Reference   386:19    D2 Ref / Minimal Header
Compact OSC Composition    386:21    Fullscreen / Compact OSC Composition
Compact OSC Reference      386:24    D3 Ref / Compact OSC Composition
Layout Contract            386:71    Fullscreen / Layout Contract · 1600×900
Acceptance Gate            386:133   D7-01 / Acceptance Gate
Verification Assertions    387:208   D7-01 / Verification Assertions
```

D7-01 只冻结 Fullscreen 的 Host composition 与 information density，不创建第二套播放状态、Timeline、Transport、Volume、Utility 或 OSC lifecycle。当前 `agent/r4-stage` 的 Qt/QML 仍是 `PlayerScreen → VideoSurface + PlayerChrome` 的早期骨架，Fullscreen 尚未形成独立 runtime owner；因此本记录只描述已完成的 Figma 设计契约，不将其描述为运行时已实现功能。

Fullscreen composition ownership：

```text
Fullscreen Host
  → Video Viewport absolute priority
  → Minimal Header · top-center
  → Compact Floating OSC · bottom-center
```

- Fullscreen 不保留桌面 Window Chrome；四个真实 host 中 `Window Actions count=0`。
- 顶部只保留 title-only Minimal Header，不放 minimize / maximize / close，也不另造 top-right Exit Fullscreen。
- Exit Fullscreen 继续属于 D3 Utility action；D7 reference 中每个 OSC 都只有一个 `Utility / exitfullscreen / rest`，旧 `Utility / fullscreen` residue=`0`。
- Fullscreen 只改变 placement / density；播放状态、Seek 真值、Volume、Utility 入口与反馈层级继续消费 D2/D3/D5 既有 contract。

Minimal Header：

- Fullscreen 使用 D2 Compact Header 几何与材质：`360×50 / R25 / V3 Glass Header Compact`。
- 只保留媒体标题；不展示 desktop metadata/action pod。
- Source reference=`386:19`；1600×900 Layout 中为 `x620 / y26`，保持水平居中。

Compact Floating OSC：

- D7 source reference=`386:24`，固定 `880×106 / R32`。
- 材质继续复用 `V3 / Glass / OSC Compact`；最终 QA 恢复并确认 `Fill=32% / Border=48%`。
- 内部只组合 D3 已冻结的引用：Timeline Lane、Playing Transport、Compact Volume Trigger、`Subtitles + More + Exit Fullscreen` Utility。
- Timeline 仍遵守 D3-02 的 `3px visual track / 16px hit target / 10px resting thumb`；Transport/Volume/Utility hit target 不因 Fullscreen 变小。
- 1600×900 Layout 中 OSC=`x360 / y764 / 880×106`，底部 inset=`30`。
- 21:9 host 只改变可用屏幕范围，OSC 仍保持 `880×106`，不会横向拉成全屏宽底栏。

D3-07 lifecycle reuse：

- D7-01 不创建正式 Reaction、Prototype Navigation、hide deadline、lock reason 或 timer。
- `Hidden / Rest / Active / LockedVisible` 继续由 D3-07 唯一 resolver 解释；Fullscreen 仍使用同一个 `2200ms` hide-delay policy。
- D3-07 Fullscreen smoke `143:95` 保持原位置与结构；D7-01 不把已有 same-policy smoke 复制成第二套生命周期。
- 鼠标移动、键盘、Paused、Popup、Error 下 Header/OSC 的具体显隐 Prototype 留给 D7-02。

真实 Fullscreen 验证：

```text
16:9 Dark    387:6    1600×900   Header 360×50 centered   OSC 880×106 centered   bottom 30
16:9 Bright  387:56   1600×900   Header 360×50 centered   OSC 880×106 centered   bottom 30
21:9 Dark    387:108  1680×720   Header 360×50 centered   OSC 880×106 centered   bottom 30
21:9 Bright  387:158  1680×720   Header 360×50 centered   OSC 880×106 centered   bottom 30
```

Bright / Dark contrast：

- Dark 两个 host 不增加额外 contrast-support。
- Bright 两个 host 各只有 Top / Bottom 两个局部 `overlay/contrast-support` zone，均保持 semantic-bound，最终 alpha=`16%`。
- contrast-support 只保护顶部标题和底部控制区可读性，不给整屏视频盖统一灰蒙层。
- 视觉 QA 已确认亮/暗画面下 Header 与 OSC 均可读，同时视频仍是第一视觉层级。

实施中发现并修复：

- 第一次 Source 写入因正式 Timecode Style 使用 `Geist Mono Medium` 且未预加载字体被 Figma 原子拒绝；没有遗留半成品。随后改为从本地 Text Style 自动收集并加载实际 fontName 后重试。
- 第二次 Source 写入仅在返回 created-ID 审计时对 TEXT 错误调用 `findAll`，再次被 Figma 原子回滚；改为从 Source Section 统一收集 descendants 后成功提交。
- Semantic Paint binding 将新 D7 OSC 与 Bright contrast-support Paint alpha 归一到 100%；视觉 QA 捕获后显式恢复 OSC `32%/48%` 与 contrast-support `16%`，同时保留 semantic color binding。
- 从 D3 复制的图标 SVG 带入 90 个默认 `Vector` 子路径；仅在 D7 新副本中按 Previous/Pause/Next/Volume/Subtitles/More/Exit Fullscreen 职责重命名，D3 source 不改。最终 generic residue=`0`。

Foundation / style：

- 新增 D7-01 Color / Geometry / Text / Effect / Motion Variable/Style=`0`。
- 继续消费既有 Compact Header/OSC geometry、Airy Glass material、semantic colors 与 D3 control geometry。
- D7-01 Page 正式 Component / Component Set=`0`；本阶段冻结 composition，不提前抢 D8 的 published component/library 收口职责。

最终审计：

```text
Fullscreen / Minimal Header source count       1
Fullscreen / Compact OSC Composition count     1
Fullscreen / Layout Contract screen count      1

Visible unbound D7 product paints              0
Generic unnamed residues                       0
New D7-01 Variable / Text / Effect / Motion    0
D7-01 formal Components                        0
D7-01 Reactions                                0
D7-01 AFTER_TIMEOUT owners                     0
Section outline residue                        0

Compact OSC copies                             6
Each OSC geometry                              880×106
Each OSC fill / border                         32% / 48%
Each OSC exitfullscreen                        1
Old fullscreen action residue                  0

16:9 / 21:9 × Dark / Bright                    PASS
Header centered                                PASS
OSC centered / bottom 30                       PASS
Desktop Window Actions                         0
Bright contrast support                        2 × 16% per bright host
Dark contrast support                          0
No full-width bottom control bar               PASS

D2 / D3 source protection                      PASS
D3-07 Fullscreen smoke protection              PASS
D6-01～D6-06 section geometry                   PASS
Acceptance Gate                                PASS
```

本任务只修改 Figma 设计与根 README；没有修改播放器源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D7-01：Complete。**

### D7-02 Fullscreen 显隐交互 · Complete

Figma：

```text
Source / Contract        393:2    D7-02 / Fullscreen Visibility Contract
Verification             393:3    D7-02 / Verification
Chrome Projection        394:16   Fullscreen / Chrome Projection
Interaction Scenarios    394:30   Fullscreen / Interaction Scenarios
Visibility Ownership     394:73   Fullscreen / Visibility Ownership
Acceptance Gate          394:80   D7-02 / Acceptance Gate
Verification Assertions  395:367  D7-02 / Verification Assertions
```

D7-02 不创建第二套 Fullscreen 生命周期。Fullscreen Header 和 Compact OSC 只投影 D3-07 已解析的唯一可见性状态：

```text
Hidden            → Header 0 / OSC 0
Active            → Header 1 / OSC 1
Rest · countdown  → Header 1 / OSC 1
Rest · persistent → Header 1 / OSC 1
LockedVisible     → Header 1 / OSC 1
```

Timer 与 lock ownership 保持不变：

- `motion/osc/hide-delay` 仍只由 D3-07 Visibility Controller 持有。
- Standard / Reduce Motion 均为 `2200ms`。
- D7-02 不新增 `hideDeadline`、lock reason、visibility controller、Reaction 或 timer。
- `FocusWithinOSC / Volume Popover Open / More Popover Open / Timeline Scrubbing / Inspector Open` 继续使用 D3-07 原 lock reason；Paused / Error 继续通过 `autoHideAllowed=false` 进入 `Rest · persistent`，不伪装成 lock。

键盘策略：

- 普通 Seek / Volume / Media Key 默认不直接唤醒 Fullscreen Header/OSC。
- D5 HUD 可以独立出现；验证中使用既有 `Feedback / HUD · Kind=Volume`，不复制第二套键盘反馈。
- 如果键盘动作本身把播放状态切为 Paused，则由 playback state 自然进入 `Rest · persistent`，此时 Header/OSC 可见。
- 真正的快捷键绑定仍归未来 Action Registry；D7-02 不把 D6 的 QA sample binding 冻结为运行时默认值。

退出 Fullscreen：

- 不增加第二个退出按钮。
- 不增加专用 Fullscreen HUD、Toast 或 onboarding hint。
- Chrome 可见时仍只有 D3 Utility 中的 `Exit Fullscreen` action；实际快捷键绑定由实现层决定。

六条真实 Fullscreen 验证均为 `1600×900`：

```text
Idle Hidden           395:4    Header 0  OSC 0
Pointer Active        395:54   Header 1  OSC 1
Keyboard HUD No Wake  395:104  Header 0  OSC 0  HUD 1
Paused Persistent     395:165  Header 1  OSC 1  Paused Transport 1
Menu LockedVisible    395:227  Header 1  OSC 1  More Popover 1
Error Persistent      395:281  Header 1  OSC 1  Error Overlay 1
```

所有可见 Chrome 场景继续严格使用 D7-01 几何：

```text
Minimal Header  360×50  @ 620,26
Compact OSC     880×106 @ 360,764
OSC bottom      30
Window Actions  0
```

Menu 验证复用 D3 `More Popover`；Error 验证复用唯一 D5 `Player Status Overlay · Error`；Paused 验证复用 D3 paused transport；没有复制对应 authority。

跨页回归：

```text
D3-07 Board                         142:2 unchanged
D3-07 Fullscreen same-policy smoke 143:95 unchanged
Single AFTER_TIMEOUT owner         143:2 REST PLAYING
AFTER_TIMEOUT                      2.2s → 143:15 HIDDEN
motion/osc/hide-delay              2200 / 2200ms
D7-02 Reactions                    0
D7-02 AFTER_TIMEOUT owners         0
```

最终审计：

```text
Required Fullscreen cases                   6 / 6
D7-02 formal Components                     0
New D7-02 Variable / Text / Effect / Motion 0
Visible unbound D7 product paints           0
Generic unnamed residues                    0
Exit hint/onboarding surfaces               0
Section outline residue                     0
Source / Verification section fit           PASS
D7-01 section geometry                      PASS
D3-07 single lifecycle owner                PASS
Acceptance Gate / Assertions                 8 / 8 · 10 / 10 PASS
```

当前 `agent/r4-stage` Qt/QML runtime 仍处于早期 `PlayerScreen → VideoSurface + PlayerChrome` 骨架，尚未实现 Fullscreen visibility owner；因此本任务完成的是 Figma 设计契约，不把运行时功能描述为已实现。本任务没有修改播放器源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D7-02：Complete。**

### D7-03 Mini Player · Complete

Figma：

```text
Source / Contract        401:47   D7-03 / Mini Player
Verification             401:48   D7-03 / Verification
Geometry Contract        402:61   Mini / Geometry Contract
Rest                      402:70   Mini / Rest · 420×236
Hover                     402:76   Mini / Hover · 420×236
Minimum Hover             402:100  Mini / Minimum Hover · 320×180
Hover Density Contract    402:124  Mini / Hover Density Contract
Shared Authority          402:134  Mini / Shared Authority
Acceptance Gate           402:143  D7-03 / Acceptance Gate
Verification Rest         403:49
Verification Hover        403:58
Verification Minimum      403:85
Always-on-top Context     403:112
Verification Assertions   403:151
Detailed Record           docs/records/D7-03_MiniPlayer.md
```

Mini 是同一播放器的独立 Window Mode，不是第二播放器，也不是主窗口等比缩放。默认逻辑尺寸=`420×236`，最小逻辑尺寸=`320×180`；Video 继续消费 D2 的 Fit / preserve aspect / centered 语义。

信息密度冻结为：

```text
Rest  → Video only
Hover → Title + Expand + Close + PlayPause + Timeline
```

Mini 明确不承载 Inspector / Playlist / Tracks / Subtitles / Chapters / Volume / More；完整能力需先 Expand 回 Windowed。`always-on-top` 是 Window flag，不增加 Pin/Lock 按钮或 badge。

共享几何保持：PlayPause=`40×40`；Timeline=`16px hit / 3px track / 10px thumb`。不通过缩小交互目标解决密度，也不建立 Mini 私有 PlaybackSession、seek truth 或 OSC lifecycle。Mode transition、Focus 与 Popup cleanup 继续留给 D7-05。

最终验证：

```text
Rest 420×236          Chrome 0
Hover 420×236         Title 1 / Actions 1 / Controls 1
Minimum 320×180       long title truncation / no overlap
Always-on-top          overlap PASS / Mini above Windowed PASS

Expand / Close         1 / 1
Minimize / Pin         0 / 0
Inspector-related      0
Visible unbound UI     0
Generic residues       0
Formal Components      0
Reactions              0
AFTER_TIMEOUT owners   0
New Mini Variables     0
Section outline        0
D7-01 / D7-02 geometry PASS
Acceptance             9 / 9 · 10 / 10 PASS
```

一次最终只读审计因 JavaScript 局部变量误用保留字产生 SyntaxError，被 Figma 原子拒绝，没有提交任何画布变化；修正后同一审计通过。D7-03 本地复制的 Pause SVG 默认 Vector 子路径已职责化命名，D3 source 未修改。

当前 `agent/r4-stage` runtime 仍只有 `PlayerScreen → VideoSurface + PlayerChrome` 早期骨架，没有 Mini Window runtime owner 或 always-on-top 实现。因此本任务完成的是 Figma 设计契约；没有修改播放器源码、配置、依赖、数据格式或运行时接口，也没有构建、单元测试或运行时测试项。

**D7-03：Complete。**

### D7-04 窄窗口降级 · Complete

Figma：

```text
Source / Contract        407:47   D7-04 / Narrow Responsive Policy
Verification             407:48   D7-04 / Verification
Policy Matrix            408:61   Responsive Policy Matrix
Pressure Ladder          408:95   Narrow Pressure Ladder
Inspector Contract       408:119
Overflow Contract        408:124
Acceptance Gate          408:129  D7-04 / Acceptance Gate
Geometry Audit           413:203  Geometry / Ownership Audit
Verification Assertions  413:223  D7-04 / Verification Assertions
Detailed Record           docs/records/D7-04_窄窗口降级.md
```

D7-04 不建立第二套全局 breakpoint。D2-05 继续是唯一响应式断点 owner：

```text
Narrow     0–839
Standard   840–1199
Wide       >=1200
```

`820 / 720 / 640 / 560` 仅作为 Narrow 内的 QA 压力宽度，不创建第四种 Responsive Mode 或新的 breakpoint Variable。

降级顺序继续消费 D3 已冻结的密度：

```text
Wide
  Utility full 222×32
  Volume inline 138×32
  Current + Duration
  Inspector dock 368

Standard
  Utility mixed 146×32
  Volume trigger 32 + z60 Popover
  Current + Duration
  Inspector overlay 368

Narrow
  Utility essential+More 108×32
  Volume trigger 32 + z60 Popover
  Inspector overlay 320
  Timeline information yields before geometry
```

Narrow 直接可见动作保持 `Subtitles + More + Fullscreen`；Audio Tracks / Chapters / Playlist 继续由既有 More Popover 提供，不把 Volume 塞进 More，也不建立第二个 Inspector。

交互目标不随窗口变窄而缩小：

```text
Transport        116×40
PlayPause         40×40
Volume Trigger    32×32
Narrow Utility   108×32
Timeline Hit      16px
Timeline Track     3px
Timeline Thumb    10×10
```

时间码使用局部 lane-pressure 规则，而不是新建全局断点：

```text
Timeline Lane >= 480px → Current + Duration
Timeline Lane < 480px  → Current only; Duration yields first
```

真实验证：

```text
820 Narrow Upper       410:49   window 820×700   OSC 716×106   lane 660   Current + Duration
720 Narrow Reference   410:108  window 720×700   OSC 616×106   lane 560   Current + Duration
640 Narrow Floor       410:167  window 640×700   OSC 536×106   lane 480   Current + Duration
560 Narrow Stress      411:203  window 560×700   OSC 456×106   lane 400   Current only
560 More Open          411:261  More 250×176 @ 284,360
560 Inspector Open     411:323  Inspector 320×450 @ 214,88
```

560 Header 保持 Compact Media Info=`360×50`、Window Actions=`110×50`，只改变 placement：Title=`x26`、Actions=`x424`、gap=`38`，不缩小窗口动作 hit target。

560 More 直接复用 D3-06 `More Popover`：`250×176 @ 284,360`，完整落在 560 窗口内且位于 OSC 上方；Audio / Chapters / Playlist 均保持可达。

560 Inspector 直接复用 D4-07 `Inspector / Shell` 实例：`320×450 @ 214,88`，bottom=`538 < OSC y564`；Inspector overlay 停在 OSC 上方，OSC 仍为 `456×106`，Video Viewport 也不因 open/close 改变。

最终审计：

```text
Source Section fit                    PASS
Verification Section fit              PASS
Visible unbound product paints        0
Generic unnamed residues              0
D7-04 formal Components               0
D7-04 Reactions                       0
D7-04 AFTER_TIMEOUT owners            0
New D7-04 global Variables            0
Section outline residue               0

All cases Timeline hit                16
All cases Timeline track               3
All cases Timeline thumb              10×10
All cases Transport                   116×40
All cases PlayPause                    40×40
All cases Volume                       32×32
All cases Narrow Utility              108×32

560 More inside host                  PASS
560 More above OSC                    PASS
560 Inspector main                    Inspector / Shell
560 Inspector stops above OSC         PASS
560 Inspector above OSC z-order       PASS

D2-05 authority regression            PASS
D3-02 authority regression            PASS
D3-05 authority regression            PASS
D3-06 authority regression            PASS
D4-07 authority regression            PASS
D7-01 / D7-02 / D7-03 geometry        PASS
Acceptance Gate                       10 / 10 PASS
Verification Assertions               12 / 12 PASS
```

D7-04 verification 中复制的 Utility SVG 默认 `Vector` 子路径仅在新副本内职责化命名，D3 source 未修改。最终 `generic residue=0`。

当前 `agent/r4-stage` 的 Qt/QML runtime 仍只有 `PlayerScreen → VideoSurface + PlayerChrome` 早期骨架，尚无 Narrow responsive runtime owner、真实 collapse resolver 或 Inspector/More 窄窗运行时布局。因此本任务完成的是 Figma 设计契约；没有修改播放器源码、配置、依赖、数据格式或运行时接口，也没有构建、单元测试或运行时测试项。

**D7-04：Complete。**

### D7-05 跨模式状态保持 · Complete

Figma：

```text
Page                       382:5777  07 Window Modes
Source / Contract          418:203   D7-05 / Mode Transition Contract
Mode Transition Matrix     418:218
Mode Transition Topology   418:252
Inspector Suspension       418:276
Focus Migration            418:281
Acceptance Gate            418:286   D7-05 / Acceptance Gate
Verification               419:203   D7-05 / Verification
Inspector Roundtrip        419:701   Flow A
Mini Cleanup Roundtrip     419:877   Flow B
Transition State Audit     419:1005
Verification Assertions    419:1036  D7-05 / Verification Assertions
Prototype Navigation       420:381   D7-05 / Prototype Navigation
```

D7-05 冻结的是跨 Window Mode 的状态映射与清理职责，不创建第二个 PlaybackSession、Seek 真值、OSC lifecycle 或 Inspector authority。当前产品拓扑只包含：

```text
Windowed ↔ Fullscreen
Windowed ↔ Mini
Fullscreen ↔ Mini  direct product edge = none
```

Windowed 是当前模式拓扑 hub。D7-03 已冻结 Mini 的 Expand 返回 Windowed；现有 Fullscreen action 也从 Windowed 进入 Fullscreen。由于当前 UI 没有冻结 Fullscreen ↔ Mini 的直接产品入口，D7-05 不凭空新增一个按钮或快捷路径。

全局播放真值持续：

- 同一媒体、Playing/Paused/Error、confirmed playback position、duration、volume/mute 与 committed track selection 都在 Window Mode 切换中持续。
- Window Mode 只改变 Host composition / information density，不重建播放会话。
- D3-03 继续唯一拥有 confirmed position；若 mode transition 打断 direct scrub / preview，先取消临时 target，不将它伪装为 commit。

Inspector suspension：

- Inspector visible surface 只属于 Windowed context；进入 Fullscreen 或 Mini 时立即隐藏。
- 可以保留 `Windowed Inspector open flag + selected Inspector mode` 作为可恢复 context。
- 返回 Windowed 时，仅当同一 player context 仍有效才恢复一个 Inspector Shell 与原 selected mode。
- Inspector focus ring、Popover、pointer hover、scroll drag 等 transient interaction 不参与恢复。
- 如果 current-media lifecycle 已使旧 Inspector context 失效，新的 primary state 优先，不能机械恢复陈旧 surface。

Transient cleanup：

```text
More Popover        → close before mode commit
Volume Popover      → close before mode commit
Timeline hover      → clear
Timeline direct drag→ cancel transient target
Seek Preview        → clear with canceled direct interaction
stale Focus         → clear / remap
```

Popup/hover/drag 都是 mode-local transient state，不跨模式泄漏。清理 transient 不等于改写 playback truth。

Focus mapping：

- Keyboard-driven Windowed Fullscreen action 与 Fullscreen Exit Fullscreen 在 destination chrome 可见时可以映射到语义 counterpart。
- 目标模式不存在 counterpart 时，focus 回到 destination window root；Mini Expand 返回 Windowed root / 后续 Tab traversal。
- Pointer-driven transition 不人工制造 keyboard focus ring。
- 一个在 destination 中不存在的 node 永远不能继续持有 focus；Inspector focus 不做 suspend/restore。

OSC / Chrome：

- Windowed ↔ Fullscreen 的明确 mode action 使 destination chrome 从 Active presentation 开始，然后继续交给 D3-07 唯一 Visibility Controller。
- D7-05 不创建新的 hide deadline、lock reason 或 `AFTER_TIMEOUT`。
- Paused / Error 仍按 D3-07 `autoHideAllowed=false → Rest · persistent`，不新增模式专用 persistent state。
- Mini 不继承 Windowed/Fullscreen 的 OSC visible/hidden 值；Mini 继续按 D7-03 自己的 Rest / Hover information-density contract 投影共享 playback truth。

真实往返验证：

```text
Flow A — Inspector context
Windowed Inspector Before      419:704
Fullscreen Active              419:764
Windowed Inspector Restored    419:812

Flow B — transient cleanup
Windowed More Before           419:880
Mini Rest After Commit         419:939
Windowed Clean Return          419:945
```

两条 flow 的 QA media 均使用 `夜间列车 · demo.mp4`；Windowed / Fullscreen 场景 confirmed time 均为 `04:12`。Flow A 验证 Inspector `1 → 0 → 1`，且返回时仍是同一个有效 Playlist context；Flow B 验证 More `1 → 0 → 0`，Mini 不携带 Inspector/More/OSC，返回 Windowed 后 More 不复活。

Prototype destination 全部为同页 top-level Frame：

```text
420:386  Windowed · Inspector Context
420:449  Fullscreen · Active
420:499  Windowed · Restored Inspector
420:562  Windowed · More Open
420:624  Mini · Hover
420:651  Windowed · Clean Return
```

Prototype reactions 共 5 条：

```text
Windowed Fullscreen Action   420:440 → 420:449  SMART_ANIMATE 160ms
Fullscreen Exit Action       420:496 → 420:499  SMART_ANIMATE 160ms
Restored Windowed Fullscreen 420:553 → 420:449  SMART_ANIMATE 160ms
QA Enter Mini                420:709 → 420:624  DISSOLVE 160ms
Mini Expand                  420:635 → 420:651  DISSOLVE 160ms
```

`QA / Enter Mini`（`420:709`）是明确标注的工程验证入口，因为 D7-03 只冻结了 Mini 内的 Expand/Close，并没有冻结 Windowed 侧的产品 Mini entry。D7-05 不把这个 QA hit target 描述成真实产品按钮，也不据此扩大 Action Registry 或运行时范围。

最终审计：

```text
Source Section fit                         PASS
Verification Section fit                   PASS
Prototype Section fit                      PASS
Prototype destinations page-level          6 / 6 PASS
Visible unbound product/UI paints           0
Intentional media-test artwork unbound     20
Generic unnamed residues                    0
D7-05 formal Components                     0
New D7-05 global Variables                  0
D7-05 Prototype reactions                   5
D7-05 AFTER_TIMEOUT owners                  0
Direct Fullscreen ↔ Mini product edge       0
Acceptance Gate                            10 / 10 PASS
Verification Assertions                    12 / 12 PASS
D7-01 ～ D7-04 protected geometry           PASS
```

20 个 unbound paint 全部来自 Verification / Prototype 的 `Media Test` glow/artwork，用于模拟视频内容，不属于产品 UI Surface；产品 UI unbound paint=`0`。D7-05 Source / Verification / Prototype Section 均清除默认 Section fill/stroke；没有新 Component、Variable、breakpoint、PlaybackSession、seek owner 或 timer owner。

实施中 Source 第一次写入因清空旧 Frame 内容后仍尝试 clone 已删除的 Text exemplar，被 Figma 原子拒绝；该调用没有提交半成品。修正 exemplar 生命周期后重新执行并完成。Prototype 的真实 NAVIGATE destination 按 Figma 约束拆成同页 top-level Frame，再在后续调用中绑定 Reaction。

当前 `agent/r4-stage` 的 Qt/QML runtime 仍只有 `PlayerScreen → VideoSurface + PlayerChrome` 早期骨架，没有 Window Mode coordinator、Fullscreen/Mini window owner、Inspector suspension 或 transient cleanup runtime 实现。因此 D7-05 完成的是 Figma 设计契约；没有修改播放器源码、配置、依赖、数据格式或运行时接口，也没有构建、单元测试或运行时测试项。

**D7-05：Complete。**

## Stage D7 Closing Result

Fullscreen 构图与显隐、Mini Player、Narrow collapse，以及 Windowed ↔ Fullscreen ↔ Mini 的状态迁移契约已经形成完整窗口模式系统。共享 Playback/Seek/OSC/Inspector authority 保持唯一；模式切换只改变 Host 与 information density，并清理不适配的 transient Popup/Focus/drag，不复制第二播放器，也不通过缩小交互目标解决响应式问题。

**Stage D7：Complete。**

## Stage D8 — Design System & Global Convergence · Complete

### D8-01 重复结构审计 · Complete

Figma：

```text
Page          431:559  08 Design System
Source        432:2    D8-01 / Duplicate Structure Audit
Verification  432:175  D8-01 / Verification
```

- 扫描 D2–D7 共 `19,641` 个节点、`2,091` 个 Instance、`50` 个 Component Set、`265` 个 Variant。
- 收敛出 Window Actions、Transport、OSC Surface、Volume、Utility、Timeline、Search/Text Entry、Feedback/Settings Action、List Row Internals、Preferences Source Item、Icon Language、Radius/Binding Drift 共 12 类重复结构。
- 明确 Playlist / Tracks / Subtitles / Chapters 不合并成万能 Row；HUD / Toast / Dialog 继续保持不同反馈 surface。
- D8-01 只审计，不创建 Component / Component Set；D2–D7 source 行为不变。
- D8-01 Audit/Verification semantic paint 最终无未绑定残留。

**D8-01：Complete。**

### D8-02 Icon 与图标语言 · Complete

Figma：

```text
Source        439:2
Verification  439:4
Preferences Playback Icon  442:44
```

- 建立 D8 统一 icon geometry / optical box / stroke language，并将重复的 16/18/21/22/24/28 容器漂移收敛为明确图标尺寸职责。
- Window / Preferences 等图标只拥有 geometry；颜色与交互 tone 继续由上层 Control/Composite state 持有。
- D8-02 不提前承担 D8-03 Control 或 D8-05 Composite 业务职责。

**D8-02：Complete。**

### D8-03 Controls · Complete

Figma：

```text
Source        450:106
Verification  450:108
Icon Button   451:154  Control / Icon Button
```

- 建立 D8 控件层 authority，并将图标选择与控件交互状态解耦；Icon Button 使用 D8-02 geometry icon 作为可替换输入。
- Control 只拥有本地 interaction/value presentation，不拥有 Player/Settings backend truth，也不提前组合成 Feature。
- D6 已存在的 Preferences Toggle / Select / Slider 等继续保留原 ownership；D8-03 不机械复制已有稳定 authority。

**D8-03：Complete。**

### D8-04 Surfaces · Complete

Figma：

```text
Source / Contract   470:308  D8-04 / Surfaces
Verification        470:310  D8-04 / Verification
Surface / OSC       474:308
Surface / Inspector 476:308
Surface / Popover   477:308
Surface / HUD       478:308
Surface / Toast     479:308
Surface / Dialog    479:310
```

六类 Surface 形成唯一材质 authority：

```text
OSC        2 variants  Compact / Standard
Inspector  1 component
Popover    1 component
HUD        1 component
Toast      1 component
Dialog     1 component
Total      6 authorities / 7 product components / 12 real Figma Slots
```

- Surface 只拥有材质、border/radius/effect 与可替换结构 Slot，不拥有业务文案、player command、feedback kind 或 backend truth。
- 新增并仅新增 D5 历史硬编码收口所需的 radius/stroke/Inspector gap foundations；产品视觉值保持不变。
- Dark/Bright 双背景、真实 Slot replacement、OSC/Inspector/Popover resize、HUD `0.46` / Toast `0.88` opacity 全部通过。
- D4 Inspector / D5 Feedback 既有产品 Composite 未在 D8-04 被替换；product replacement=`0`。
- Source / Verification visible solid paint unbound=`0`；generic residue=`0`。

**D8-04：Complete。**

### D8-05 Composite · Complete

Figma：

```text
Source / Contract             491:397  D8-05 / Composite
Verification                  491:399  D8-05 / Verification
Floating Header               494:10663  Composite / Floating Header
Playlist Row                  168:260    Playlist / Entry Row
Track Row                     179:716    Track / Selection Row
Settings Row                  336:2293   Preferences / Settings Row
Source List Item              507:537    Composite / Source List Item
Source Icon Helper · Default  506:10735  private
Source Icon Helper · Selected 506:10777  private
Acceptance Assertions         516:676
```

Authority 策略：

```text
2 new
  Floating Header
  Source List Item

2 enhanced in place
  Playlist Row
  Track Row

1 retained
  Settings Row
```

Floating Header：

- `Size=Compact / Standard` 两态；Compact=`668×50`，Standard source=`1268×54`，验证额外覆盖 resized Standard=`908×54`。
- 使用 `110 spacer + centered Media Info + 110 Window Actions` 的平衡结构保持媒体信息真实居中。
- `Title / Meta` 为数据输入；Window Actions 真实消费 D8-03 `Control / Icon Button` 和 D8-02 Minimize/Maximize/Close geometry。
- 不拥有 playback state、OSC visibility 或窗口 command truth。

Playlist / Track Row：

- `Playlist / Entry Row` 原 ID `168:260` 保持，6 个真实 State 不变；新增 `Index / Title / Meta` TEXT properties。
- `Track / Selection Row` 原 ID `179:716` 保持，`Type=Audio/Video/Subtitle × State=5` 共 15 Variant 不变；新增 `Type Label / Title / Meta` TEXT properties。
- Playlist 与 Track 继续分开，未合并为万能 List Row。

Settings Row：

- `Preferences / Settings Row` `336:2293` 保留原 authority 与原 API；只补 canonical ownership 描述。
- Verification 真实 swap 到 D6 既有正式 Control：Toggle `329:2175` / Select `330:2160` / Slider `331:2160`，nested main component ID 全部一致。

Source List Item：

- legacy D6 `Preferences / Source Item` `298:349` 的 `7 Category × 5 State = 35` Variant 保留到 D8-06，不在 D8-05 破坏性迁移。
- 新 canonical `507:537` 顶层只保留 5 个 Row State；Category 移入 private helper property。
- private helper 分成 Default / Selected 两套 tone owner，各有 7 个 Category geometry Variant，并直接消费 D8-02 Preferences icon geometry。
- Category QA 覆盖 Playback / Video / Audio / Shortcuts；Default 使用 `icon/secondary`，Selected 使用 `selection/indicator`，geometry 与 tone 均保持正确。
- 因此 Row 级 Variant 从 `35 → 5`，同时避免 Category swap 重置 selected tone。

多实例 Verification：

```text
Floating Header  668 / 908 / 1268 widths         PASS
Playlist         Default / Current / Invalid     PASS
Track            Audio Selected / Video Pending /
                 Subtitle Off                    PASS
Settings         Toggle / Select / Slider swap   PASS
Source Item      4 State × 4 Category samples    PASS
```

受保护产品页实例使用 `authority → variant → instance → PAGE` 的严格口径重新核验：

```text
Playlist Row      D4  = 5
Track Row         D4  = 8
Settings Row      D6  = 37
legacy Source Item D6 = 14
Protected total        = 64
```

D8-05 新建 QA/Registry 实例只存在于 `08 Design System`；Floating Header、Source List Item 和两个 private helper 在 D8 页面外实例均为 `0`。因此 D8-05 product replacement=`0`，真正全局回刷继续由 D8-06 负责。

最终 machine gate：

```text
Authority API                         PASS
Protected product-page live baseline  5 / 8 / 37 / 14 PASS
New authority external instances      0
Product replacement                   0
Source visible solid paints           161 / unbound 0
Verification visible solid paints     184 / unbound 0
Generic residue                       0
Foundation Δ                          0
D2 frozen Header anchors              8 / 8 PASS
Acceptance Assertions                 10 / 10 PASS
Source / Verification section fit     PASS
Section gap                           100px
```

D8-05 只修改 Figma Component/Component Set/API 与根 README；没有播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D8-05：Complete。**

### D8-06 全局实例回刷 · Complete

Figma：

```text
Source / Contract  529:676  D8-06 / Product Instance Backfill
Verification       529:678  D8-06 / Verification
```

D8-06 将 D8-02～D8-05 已验证的正式 authority 真正回刷到 D2～D7 成品链。原则保持为：

```text
source authority
  → product instances
  → state/data/geometry/reaction restoration
  → visual QA
  → machine regression gate
```

未使用 detach，也没有为保留旧视觉建立无退出计划的兼容壳；发现 D7 响应式能力缺口时回到正式 source component 补能力后再继续实例迁移。

产品回刷范围：

```text
D2 Responsive Header                4
D4 Playlist Search                  1 source migration
D4 Inspector Shell                 46 live instances via source material migration
D5 Floating Header                 41
D5 OSC root                         36
D5 HUD / Toast / Dialog             3 business authorities → D8 Surfaces
D6 Source List                     57 instances / 399 derived rows
D6 Source List direct rows          14
D6 Preferences Window Actions        9
D7 Windowed Header                  14
D7 product OSC                      24
D7 Mini control compositions         6
D7 More Popover                      4
```

D2 / D5 Header：

- D2-05 的 720 / 960 / 1280 / 1600 四套 product Header 全部替换为 `Composite / Floating Header`；D2-03 三套历史 contract example 不作为 product residual 处理。
- D5 41 套 Verification / Prototype Header 全部切到同一 `Composite / Floating Header`；旧直接 `Floating Header / Media Info + Window Actions` product Frame=`0`。
- Title / Meta、位置、窗口 inset 与 Compact/Standard 高度均保持。

D4 Search / Inspector：

- Playlist Toolbar 唯一 Search 已从旧 `Playlist / Search` 切到 `Control / Search`（`365:4245`），保持 `270×42` 与 `搜索播放列表`。
- `Inspector / Shell` `152:4` 保持原 `Title / Meta / Content Host / Footer Host` 公开 API。
- Figma 的 Slot-inside-Slot 会改变原 Slot identity，因此 D8-06 不强行把原 Content/Footer Slot 嵌套进 Surface Slot；最终结构为 canonical `Surface / Inspector` 作为唯一 material layer，原 Header / Mode Switch / Content Host / Footer Host 继续承担业务组合 API。
- Shell live instances=`46`；320×450、368×420、368×584、368×652 等真实 resize 与 Content/Footer overrides 均验证不变。

D5 OSC / Feedback：

- 36 个 product OSC root 全部使用 `Surface / OSC + Control / Timeline + Control / Playback Button + Control / Icon Button`。
- Timeline 状态映射保持：Default、Scrubbing、PendingSeek；Paused 使用 Play，Playing 使用 Pause。
- D5 Standard Utility 继续遵守 D3-06：`Subtitles / Playlist / More / Fullscreen`；不新增第二套路由。
- `Feedback / HUD / Toast / Dialog` 保持原业务 Component Set 与 Variant ID，只把材质 owner 分别迁到 `Surface / HUD / Toast / Dialog`；已有业务内容、Action、marker 与 live instance 保持。
- D5 product old Header / old Timeline-Control lane / primitive transport control residual=`0`。

D6 Source List：

- 迁移前严格快照 `57 × 7 = 399` 个派生 row 的 Category / State / width。
- 14 个 Source List source row 从旧 35-Variant Source Item 迁到 `Composite / Source List Item` 后，对 57 个 Source List instance 逐行显式恢复；`399/399` 恢复，mismatch=`0`。
- D6-02 7 个 category screen 仍各 `6` reactions；D6-05 category Prototype 总 `42`；D6-06 Shortcuts Prototype 总 `41`，全部保持。
- Preferences Shell Standard / Narrow / Minimum 共 9 个手绘窗口动作已切 D8 Icon Button。

D7 响应式 source capability fix：

`Control / Timeline` 新增两个正交 BOOLEAN property：

```text
Show Current#550:0   default = true
Show Duration#550:15 default = true
```

- 14 个 Timeline Variant 全部绑定 timecode visibility。
- 已有 Timeline instance 默认仍为 `true / true`，D5 不回归。
- 560 Narrow 只设置 `Show Duration=false`，继续保持 `Current only`。
- Mini 设置 `Show Current=false / Show Duration=false`，不伪造新的 Mini Timeline variant。

`Composite / Floating Header` 新增：

```text
Center Media#563:0  BOOLEAN  default = true
```

- 只控制 Balance Spacer visibility，不增加 `Size × Alignment` Variant 轴。
- D2 / D5 / D8 已有 48 个 Header instance 默认保持 `true`。
- D7 820/720 使用 Center；640/560 使用 Leading。14 个 Windowed Header 回刷后 Media Info / Window Actions 的绝对 x 与旧成品逐个比对为 `0` 偏差。

D7 OSC / Mini / Popover：

- 24 个真实 product Compact OSC 全部回刷；D7-01 的 3 个 source/layout historical fixture 保留为阶段历史，不计入 product residual。
- Fullscreen Utility 保持 `Subtitles / More / ExitFullscreen`；Windowed Narrow 保持 `Subtitles / More / Fullscreen`；More/Open 使用 Icon Button Open state。
- 6 套 Mini Hover / Minimum / Always-on-top / Prototype control composition 全部回刷：Expand / Close → Icon Button，PlayPause → Playback Button，Timeline → canonical Timeline。
- Mini Timeline 通过 y `14→10` 的 canonical geometry placement 保持旧 global Hit Target y 与 Track y，逐实例偏差=`0`。
- 4 个 D7 product More Popover 均使用 `Surface / Popover`，P1/P2/P3 业务内容保持。
- Fullscreen `D2 Ref / Minimal Header` 共 12 个属于 title-only feature composition，没有 Window Actions，因此不强行合并为 Windowed Floating Header。

Prototype / Reaction 回归：

- 显式迁移 D7 OSC Fullscreen / ExitFullscreen control reaction 共 `3` 条；Mini Expand reaction `1` 条，before/after Reaction JSON 完全一致。
- D7-05 mode transition Prototype 总 reaction 仍为 `5`。
- D6-02 / D6-05 / D6-06 reaction 总量保持 `6×7 / 42 / 41`。

旧 authority 退休：

```text
Preferences / Source Item  298:349  → retired / fresh-index absent
Preferences / Source Icon  296:142  → retired / fresh-index absent
Playlist / Search          167:197  → retired / fresh-index absent
```

- Source Item 删除前 product live usage=`0`。
- Source Icon 的 35 个实例全部仅嵌在已退休的旧 Source Item source 内，external usage=`0`；Source Item 退休后归零并删除。
- Playlist Search product live usage=`0` 后删除；Toolbar canonical Search 保持。
- Figma Component Set 删除索引存在事务内延迟刷新，最终验收使用新事务 fresh-index 确认三个 legacy source 均已不存在。

视觉 QA 实际覆盖：

```text
D4  Inspector 320×450 / 368×584
D5  Playing / Scrubbing / Pending / Narrow Paused
    HUD / Toast / Resume Dialog / ErrorRecovery Dialog
D6  Source List / Advanced SelectedFocus / Narrow Shortcuts Selected
D7  Fullscreen Active / Menu Open
    820 centered Header
    560 Current-only / More Open
    Windowed Inspector Prototype
    Mini Hover / Minimum / Prototype
```

最终 machine gate：

```text
D2  canonical Header                         4 / 4 PASS
D4  Search main set                          365:4245 PASS
D4  Inspector Shell live                     46 PASS
D5  Header / OSC                            41 / 36 PASS
D5  old product Header / lane/control        0
D6  Source List direct canonical rows        14 PASS
D6  Preferences Window Icon Buttons           9 PASS
D6  Prototype reactions             6×7 / 42 / 41 PASS
D7  Header / OSC / Mini / Popover       14 / 24 / 6 / 4 PASS
D7  product old Header / OSC lane / Mini       0
D7-05 Prototype reactions                       5 PASS
Legacy authority nodes                          0
D8-06 Source visible solids              62 / unbound 0
D8-06 Verification visible solids        50 / unbound 0
D8-06 Source / Verification generic residue     0
D8-06 formal Components created                  0
Source / Verification section fit                PASS
Section gap                                     100px
Foundation Variable Δ                             0
```

D8-06 没有新增 Color / Geometry / Effect / Motion Variable；仅为现有 Component 增加 3 个响应式/信息密度 BOOLEAN property。没有修改播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D8-06：Complete。**

### D8-07 命名/变量/层级卫生 · Complete

Figma：

```text
Source / Contract  591:676  D8-07 / Naming Variable Hierarchy Hygiene
Verification       591:677  D8-07 / Verification
```

D8-07 对 9 个 Figma Page 做最终文件级 hygiene，只处理命名、变量绑定、Alias、Slot 默认材质与 authority namespace；不重新设计 D8-06 已验证通过的 Header / OSC / Inspector / Preferences / Window Mode 行为。

命名收口：

- `00 Framework Preview` 39 个、D2 本地 4 个、D3 本地 229 个、D8-02 authoritative Icon source 38 个默认 `Vector/Frame` 改为职责化层名，共 `310` 个 source/local layer。
- D8-02 source 的图标路径按 Close / Transport / Volume / Utility 等职责命名，实例派生层自动继承；optical geometry 未改变。
- 最终 9 个 Page 的 semantically meaningless default `Vector / Frame / Rectangle / ...` residue=`0`。

Section 默认材质：

```text
D4  15
D5  11
D6  15
D7   6
Total 47
```

以上只清除 Figma Section 容器自带的默认白 Fill / 10% 黑 Stroke；Section 内产品 Frame、Instance、geometry、reaction 均未修改。

Semantic Paint binding：

- D1 历史文档层 `214` 个未绑定 Paint 全部回到既有 semantic color variable；Primitive swatch 本身未被错误重定义。
- D4 精确回刷 `82` 个可一一解释的产品/文档 Paint。
- D5 精确回刷 `15` 个可一一解释的产品/文档 Paint。
- 没有为测试视频或复杂背景创建伪产品 Token；D8-07 新增 Color / Geometry / Effect / Motion Variable=`0`。

剩余 intentional QA/Test solid 共 `325`，全部有明确测试用途：

```text
D2   1    Test Background / Dark
D4 126    Verification background / atmosphere / QA annotation
D5  88    Synthetic Media + Network Loading contrast artwork
D7 110    Media Test / Dark & Bright scenes
```

- D4 剩余 `126/126` 全部位于 Verification Section，`0` 个进入任何真实 Component Instance 子树，`0` 个位于 Verification 外。
- D5 剩余 `88/88` 全部为 Synthetic Media / Network Loading 测试艺术层，`inInstances=0`、unexpected=`0`。
- D7 剩余 `110/110` 全部位于明确命名的 `Media Test / …` 测试艺术层。
- 因此 taskbook 口径下最终 **unexplained hardcoded product/UI solid=`0`**；保留的 325 个测试色不是产品 UI hardcode。

Variable / Alias 完整性：

```text
Variables             456
Collections            11
Alias edges            360
Broken Alias             0
Bad collection refs      0
Empty collections        0
```

11 组 `z/*` 同名项已逐项确认是合法的 `V3 / Interaction Primitive → V3 / Interaction Semantic` collection-scoped Alias；现有 API / code syntax 已使用，因此保持不改名。

Hierarchy / authority hygiene：

- Broken Instance main component=`0`。
- Dirty Slot background/stroke/effect=`0`。
- canonical namespace detached-copy suspect=`0`。
- D5-01 Priority Matrix 中四个说明卡原 `Surface / HUD|TOAST|OVERLAY|DIALOG` 改为 `Feedback Route / …`，避免文档层冒充 D8 正式 Surface authority；只改名称，不改视觉与内容。
- D8-06 已退休的 `Preferences / Source Item 298:349`、`Preferences / Source Icon 296:142`、`Playlist / Search 167:197` 继续保持 fresh-index absent。

D8-06 产品回归：

```text
D2  Responsive Header                     4 / 4 PASS
D4  canonical Search                      1 / 1 PASS
D4  Inspector Shell live                     46 PASS
D5  Header / OSC                         41 / 36 PASS
D5  OSC inner Timeline/Playback/Icon  36 / 36 / 252 PASS
D6  Source List direct canonical rows         14 PASS
D6  Preferences Window Icon Buttons            9 PASS
D6  Prototype reactions              6×7 / 42 / 41 PASS
D7  Header / OSC / Mini / Popover        14 / 24 / 6 / 4 PASS
D7  OSC inner Timeline/Playback/Icon 24 / 24 / 144 PASS
D7-05 Prototype reactions                        5 PASS
Legacy authority nodes                           0
```

视觉 QA 实际覆盖：

```text
D4  154:3    Standard Inspector
D5  236:69   Playing / Header / OSC
D6  303:658  Standard Playback Selected
D7  411:261  560 More Open
D8  D8-07 Source / Verification full-board
```

上述代表场景均通过；D7 560 仍保持 Leading Header、Current-only Timeline 与 More Popover contract。

D8-07 最终 machine gate：

```text
D8-07 Source visible solids          74 / 74 semantic-bound
D8-07 Verification visible solids   138 / 138 semantic-bound
D8-07 Source / Verification generic residue     0
D8-07 dirty Slot / broken Instance               0
D8-07 formal Components / Component Sets         0
08 Design System Page generic / unbound / dirty  0 / 0 / 0
D8-02 authoritative Icon source generic          0
Variables                                      456
Foundation Variable Δ                            0
Source / Verification section fit               PASS
Section gap                                    100px
```

D8-07 只修改 Figma 设计卫生与根 README；没有修改播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D8-07：Complete。**

## Stage D8 Closing Result

D8-01～D8-07 已完成重复结构审计、Icon language、Control、Surface、Composite authority、全局成品实例回刷以及命名/变量/层级卫生收口。D2～D7 真实产品画面均消费正式 authority；旧 Source Item / Source Icon / Playlist Search 已退休，Prototype reaction 与响应式 geometry 回归保持。

最终文件级 gate 为：默认无语义层名=`0`、无解释 product/UI hardcode=`0`、Broken Alias=`0`、Broken Instance=`0`、Dirty Slot=`0`、canonical detached copy=`0`。保留的 `325` 个 unbound solid 全部属于明确命名的 QA/媒体测试艺术层，不被伪装成产品 Token。D8-07 没有新增 Variable、Component 或 Component Set，也没有改写产品行为。

**Stage D8：Complete。**

## Stage D9 — Prototype / Handoff · In Progress

### D9-01 核心播放原型 · Complete

Figma：

```text
Page                 599:3362  09 Prototype & Handoff
Source / Contract    605:203   D9-01 / Core Playback Prototype
Verification         606:203   D9-01 / Verification
Empty                600:2
Loading              600:24
Playing Visible      600:43
Playing Hidden       600:147
Paused Persistent    600:166
Open Media Hit       602:211
QA Media Ready Hit   602:213
```

D9-01 使用 D5/D8 已验证的真实 Component Instance 建立最终点击链，不创建新的播放器视觉 source、PlaybackSession、状态机或 visibility controller。

主链：

```text
Empty
  → Open Media click
Loading
  → QA / Media Ready click
Playing Visible
  ↔ PlayPause click / Space
Paused Persistent

Playing Visible
  → AFTER_TIMEOUT 2.2s
Playing Hidden
  → Mouse Enter / click fallback
Playing Visible
```

状态与组件 ownership：

- Empty / Loading 继续消费唯一 `Player Status Overlay` 与 `Feedback / Open Media Action / Loading Indicator`。
- Playing / Paused 继续消费 D8 `Composite / Floating Header`、`Surface / OSC`、`Control / Timeline`、`Control / Playback Button`、`Control / Icon Button`。
- Playing 使用 Pause glyph；Paused 使用 Play glyph；D9-01 不复制 transport source。
- D3-07 继续是 OSC inactivity policy owner；D9-01 只把已冻结的 `2.2s → Hidden / 120ms Ease In / 160ms Ease Out wake` 投影到最终 Prototype。
- Paused 继续使用 `Rest · persistent`，没有 inactivity timeout。
- `QA / Media Ready Hit` 只模拟 backend media-ready 事件，使 Loading 能进入 Playing；它不定义真实加载耗时，也不是产品按钮。

Prototype reaction：

```text
602:211  ON_CLICK          → 600:24   160ms Ease Out
602:213  ON_CLICK          → 600:43   160ms Ease Out
600:43   Space             → 600:166  120ms Ease Out
600:49   PlayPause click   → 600:166  120ms Ease Out
600:43   AFTER_TIMEOUT 2.2 → 600:147  120ms Ease In
600:147  MOUSE_ENTER       → 600:43   160ms Ease Out
600:147  ON_CLICK fallback → 600:43   160ms Ease Out
600:147  Space             → 600:166  120ms Ease Out
600:166  Space             → 600:43   120ms Ease Out
600:172  PlayPause click   → 600:43   120ms Ease Out
```

Presentation 默认起点：

```text
D9-01 Core Playback → 600:2 Empty
```

D9-01 共 5 个最终状态 destination，全部是 `09 Prototype & Handoff` Page 的顶层 Frame；D5 历史 Prototype destination residue=`0`，Non-page destination=`0`。

不可见 Hit：

- `602:211` Open Media=`144×42`。
- `602:213` QA Media Ready=`48×48`。
- 两者均使用 semantic-bound `surface/glass + node opacity=0.001`，没有引入不可见硬编码 Paint。

视觉 QA 实际覆盖：

```text
600:2    Empty
600:24   Loading
600:43   Playing Visible
600:147  Playing Hidden
600:166  Paused Persistent
605:203  Source board
606:203  Verification board
```

五个产品状态均保持 D5/D8 既有视觉。Playing Hidden 只隐藏 OSC，不改写媒体背景或窗口 chrome；Playing/Paused 的 Timeline、Pause/Play glyph、Window Actions 均正常。

最终 machine gate：

```text
Prototype states                         5 / 5 Page-level
Prototype reactions                     10 / 10
ON_CLICK                                  5
ON_KEY_DOWN                               3
AFTER_TIMEOUT                             1
MOUSE_ENTER                               1
Single timeout owner                      1 · Playing Visible · 2.2s
Non-page destinations                     0
Old D5 destinations                       0
Broken instances                          0
D9 formal Components / Component Sets     0
New Variables                             0
Variables total                         456
Foundation Variable Δ                     0
Generic default-name residue              0
Unexplained product/UI hardcode           0
Intentional Synthetic Media solids        6
Source visible solids                    46 / unbound 0
Verification visible solids              72 / unbound 0
Source / Verification fit                PASS
Source → Verification gap               100px
```

6 个 unbound solid 只来自 Playing Visible / Hidden / Paused 的 `Synthetic Media Glow / Accent` 测试艺术层，用于模拟视频内容，不属于产品 UI Surface。

Atomic boundary 保持：D9-01 未提前实现 D9-02 Timeline Seek、D9-03 Inspector、D9-04 Window Mode 或 D9-05 Error recovery。第一次 reaction 写入按 typings 携带 `MOUSE_ENTER.deprecatedVersion` 时被当前 Figma runtime schema 原子拒绝，没有提交半成品；改为运行时实际接受的 `MOUSE_ENTER { delay: 0 }` 后成功写入并通过最终 gate。

本任务只修改 Figma Prototype 设计与根 README；没有修改播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D9-01：Complete。**

### D9-02 Timeline Seek 原型 · Complete

Figma：

```text
Page                    599:3362  09 Prototype & Handoff
Source / Contract       617:580   D9-02 / Timeline Seek Prototype
Verification            618:580   D9-02 / Verification
Rest                    614:203
Hover Preview           614:298
Scrubbing               614:414
Pending Seek            614:530
Confirmed Resume        614:647
Timeline Hit · Rest     614:297
Timeline Hit · Hover    614:413
Timeline Hit · Scrub    614:529
QA Backend Confirm Hit  614:646
```

D9-02 继续使用 D9-01 最终 Playing composition，并消费 D3-03、D5-04 与 D8 已冻结 authority；没有创建第二套 Timeline、PlaybackSnapshot、Seek Preview 或 player state owner。

主链：

```text
Rest
  → Mouse Enter · 120ms Ease Out
Hover Preview
  → Mouse Down · instant
Scrubbing
  → Mouse Up / release · 160ms Ease Out
Pending Seek
  → QA Backend Confirm click · 160ms Ease Out
Confirmed Resume
```

取消链：

```text
Hover Preview → Mouse Leave → Rest · 120ms Ease Out
Scrubbing     → ESC         → Rest · 120ms Ease Out
```

position truth 继续严格属于 D3-03 `PlaybackSnapshot`：

```text
Rest               actual 38%
Hover Preview      actual 38% + preview target 66%
Scrubbing          temporary target 66% + committed marker 38%
Pending Seek       actual 38% + pending target 66%
Backend confirm    actual adopts 66%
Confirmed Resume   actual 66% / preview cleared
```

因此 pointer release 只发出 seek intent 并进入 Pending，不提前把 UI target 写成真实播放位置；真正 commit 只发生在 backend confirmation。

`Feedback / Seek Preview` 继续只拥有 preview time；Hover/Scrubbing/Pending 三个状态使用 D5 正式 `Feedback / Seek Preview` instance，Confirmed 后 Preview 消失。

D8 `Control / Timeline` 仍严格只有：

```text
Default
HoverPreview
Scrubbing
PendingSeek
ChapterHover
Unknown
NonSeekable
```

即 `2 Size × 7 State = 14 variants`，`Committed variant=0`。Committed 是运行时数据结果，不允许为了 Prototype 增加伪 Variant。

Figma Component Instance 无浮点 progress data property；因此 `614:647 Confirmed Resume` 采用 canonical Scrubbing target geometry 作为 **仅该 D9 Prototype instance 的 confirmed-data projection**，并把 Scrubbing 的临时 `Committed Marker` visibility override 为 hidden。最终视觉为 66% actual、无 Preview、无 Pending、无 Committed Marker；D8 source 与 Variant API 完全未修改，也未 detach instance。

Prototype reaction：

```text
614:297  MOUSE_ENTER       → 614:298  120ms Ease Out
614:413  MOUSE_LEAVE       → 614:203  120ms Ease Out
614:413  MOUSE_DOWN        → 614:414  instant / transition=null
614:529  MOUSE_UP          → 614:530  160ms Ease Out
614:414  ESC               → 614:203  120ms Ease Out
614:646  ON_CLICK          → 614:647  160ms Ease Out
```

第一次 Hover→Scrubbing 使用 `SMART_ANIMATE duration=0` 时，当前 Figma runtime 自动归一为 300ms；最终已改成 `transition=null`，machine audit 确认该路径为真正即时切换。

Presentation 起点：

```text
D9-01 Core Playback → 600:2
D9-02 Timeline Seek → 614:203
```

第一次增量追加第二 Flow Start 时，Figma `flowStartingPoints` setter 报 duplicate nodeIds 并原子拒绝，未提交任何 reaction；最终通过清空 setter 状态后一次性写回两个唯一 Flow Start，D9-01 起点保持不变。

视觉 QA 实际覆盖：

```text
614:203  Rest
614:298  Hover Preview
614:414  Scrubbing
614:530  Pending Seek
614:647  Confirmed Resume
617:580  Source board
618:580  Verification board
```

真实几何结果：

```text
Rest progress                    38%
Hover actual                     38%
Scrubbing progress / target      66%
Scrubbing committed marker       38%
Pending actual                   38%
Pending hollow target            66%
Confirmed actual                 66%
Confirmed preview                0
```

最终 machine gate：

```text
Prototype states                         5 / 5 Page-level
Prototype reactions                      6 / 6
MOUSE_ENTER                               1
MOUSE_LEAVE                               1
MOUSE_DOWN                                1 · instant
MOUSE_UP                                  1
ON_KEY_DOWN                               1 · ESC
ON_CLICK                                  1 · QA backend confirm
D9-02 AFTER_TIMEOUT                       0
Non-page destinations                     0
Old D3/D5 destinations                    0
Broken instances                          0
D9 formal Components / Component Sets     0
New Variables                             0
Variables total                         456
Foundation Variable Δ                     0
Generic default-name residue              0
Unexplained product/UI hardcode           0
Intentional Synthetic Media solids       10
D9-02 Source visible solids              46 / 46 semantic-bound
D9-02 Verification visible solids        72 / 72 semantic-bound
Source → Verification gap               100px
D9-01 reactions                          10 unchanged
D9-01 timeout owner                       1 · 2.2s unchanged
```

10 个 unbound solid 全部来自 5 个 Prototype screen 的 `Synthetic Media Glow / Accent` 测试艺术层，用于模拟视频内容，不属于产品 UI Surface。

跨页回归：

```text
D3-03 Board                 104:2 PASS
D3 Rest/Hover/Scrub/Pending fixtures PASS
D5 Scrubbing                249:108 → Timeline 453:270 PASS
D5 Pending                  249:143 → Timeline 453:290 PASS
D8 Timeline authority       453:341 · 14 variants PASS
D8 Committed variant        0 PASS
```

Atomic boundary 保持：D9-02 未提前实现 D9-03 Inspector、D9-04 Window Mode 或 D9-05 Error recovery；没有修改 D3/D5/D8 source、播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D9-02：Complete。**

### D9-03 Inspector 原型 · Complete

Figma：

```text
Page                   599:3362  09 Prototype & Handoff
Source / Contract      627:1837  D9-03 / Inspector Prototype
Verification           627:1891  D9-03 / Verification
Standard Closed        622:939
Playlist               622:961
Tracks                 623:1062
Subtitles              623:1363
Chapters               623:1611
Narrow Closed          624:3827
Narrow Overlay QA      624:3850
Narrow Shell           625:1926
```

D9-03 从 D9-02 已确认的 66% playback/seek 状态继续组合 Inspector，不创建第二套 Inspector、PlaybackSession、Timeline 或 visibility lifecycle。

Standard 主链：

```text
622:939 Standard Closed
  → Playlist action click · 160ms Ease Out
622:961 Playlist Inspector
  ↔ Tracks / Subtitles / Chapters · 160ms Ease Out

任一 Standard Open
  → Close / Outside / ESC · 120ms Dissolve
622:939 Standard Closed
```

四个 Standard Open 状态全部消费同一个 `Inspector / Shell` main component `152:4`：

```text
Playlist    622:983   368×420 @ 564,92
Tracks      623:1084  368×420 @ 564,92
Subtitles   623:1385  368×420 @ 564,92
Chapters    623:1633  368×420 @ 564,92
```

Mode 与 Content 继续由 D4 唯一 authority 持有：`Inspector / Mode Switch 160:88`、`Playlist / Content 172:284`、`Tracks / Content 183:823`、`Subtitles / Content 191:1565`、`Chapters / Content 200:2168`。D9-03 只负责最终 composition 与 routing。

Standard continuity：

- Closed 与四个 Open 状态都保持 D9-02 Confirmed Data Projection，Timeline actual=`66%`。
- OSC 全部保持 `856×124 @ 52,538`；Inspector bottom=`512 < OSC top=538`，不重排或挤压播放器控制区。
- Inspector Open 只消费 D3-07 既有 `Inspector Open` visibility lock；D9-03 不创建 `AFTER_TIMEOUT`、hide deadline 或新的 lock owner。

Narrow QA：

- `624:3850` 是独立 QA Presentation flow，不伪造 Narrow Playlist 产品入口。
- 直接复用 D4-07 已验证的 Narrow Playlist Shell：`625:1926 = 320×450 @ 374,88`。
- Narrow OSC=`616×106 @ 52,564`；Shell bottom=`538 < OSC top=564`，验证 overlay 停在 OSC 上方。
- Close / Outside / ESC 均以 120ms Dissolve 返回 `624:3827 Narrow Closed`。

Prototype reaction：

```text
D9-03 total                    28
ON_CLICK                       23
ON_KEY_DOWN / ESC               5
Mode switch                     12
Standard Close / Outside / ESC 12
Standard open                    1
Narrow Close / Outside / ESC    3
AFTER_TIMEOUT                    0
```

全部 28 条 NAVIGATE destination 都是 D9 Page-level Frame；Non-page destination=`0`，old D4 destination=`0`。

Presentation Flow Start：

```text
D9-01 Core Playback       → 600:2
D9-02 Timeline Seek       → 614:203
D9-03 Inspector           → 622:939
D9-03 Narrow Overlay QA   → 624:3850
```

最终 machine gate：

```text
D9-03 states                             7 / 7 Page-level
D9-03 reactions                         28 / 28
D9-03 AFTER_TIMEOUT                      0
Standard Shell main                   152:4 · 4 / 4 PASS
Narrow Shell main                     152:4 · PASS
Standard confirmed position             66% · 5 / 5 PASS
Narrow geometry                    320×450 @ 374,88 PASS
Narrow Shell bottom / OSC top          538 / 564 PASS
Non-page destinations                    0
Old D4 destinations                      0
Broken instances                         0
D9 formal Components / Component Sets    0 / 0
New Variables                            0
Variables total                        456
Foundation Variable Δ                    0
Generic default-name residue             0
Unexplained product/UI hardcode          0
Intentional Synthetic Media solids      10
Source visible solids                   46 / 46 semantic-bound
Verification visible solids             72 / 72 semantic-bound
Source → Verification gap              100px
D9-01 reactions                         10 unchanged
D9-01 timeout owner                      1 · 2.2s unchanged
D9-02 reactions                          6 unchanged
```

跨页回归：

```text
D4 Inspector / Shell                   152:4 PASS
D4 Inspector / Mode Switch            160:88 PASS
D4 four Content authorities                 PASS
D4-07 representative Prototype reactions 22 PASS
D3-07 Board                           142:2 PASS
D3-07 single AFTER_TIMEOUT owner      143:2 · 2.2s → 143:15 PASS
```

10 个 unbound solid 全部来自五个 Standard Prototype screen 的 Synthetic Media 测试艺术层，不属于产品 UI Surface；Narrow QA 继续消费 D7 已绑定的产品 UI。Source/Verification 均无 unbound paint。

Atomic boundary 保持：D9-03 未提前实现 D9-04 Window Mode 或 D9-05 Error recovery；没有修改 D3/D4/D8 source、播放器 Qt/QML/C++ 源码、配置、依赖、数据格式或运行时接口，因此没有构建、单元测试或运行时测试项。

**D9-03：Complete。**

## Stage D9 Current Result

D9-01 已建立 Empty → Loading → Playing / Hidden / Paused；D9-02 已补齐 Hover Preview → Scrub → Pending → Backend Confirm；D9-03 已从 confirmed 66% Player 打开唯一 Inspector Shell，并在同一 Shell 内切换 Playlist / Tracks / Subtitles / Chapters，同时完成 Standard 与 720 Narrow overlay dismissal 验证。Playback、confirmed position、Inspector、OSC visibility authority 仍分别归既有 D3/D4/D5/D8 owner，D9 只承担最终可点击组合与演示路由。

**Stage D9：In Progress。**

## Next

**D9-04 — Window Mode 原型**

下一步按 `docs/plans/stages/D9_原型交付与最终验收.md` 在已关闭的 D9-01 / D9-02 / D9-03 最终链上接入 Window Mode：

```text
Windowed
  → Fullscreen
  → Windowed
  → Mini
  → Windowed
```

D9-04 必须继续消费 D7-05 已冻结的 mode topology、Inspector suspension、transient cleanup 与 focus mapping；不得建立第二个 PlaybackSession、Window Mode coordinator、OSC lifecycle 或直接 Fullscreen ↔ Mini 产品边。
