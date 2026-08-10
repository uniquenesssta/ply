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
- **D6：In Progress — D6-01 Complete。**
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
- **下一任务：D6-02 Source List Navigation。**

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
retryable current-media failure      → Overlay · Error → Retry
non-recoverable decode/render failure→ Overlay · Error → Open Media
unknown current-media failure        → Overlay · Error → Retry + Open Media
Loading / Buffering → Error          → replace previous state, never stack
Error + decision required            → D5-07 Dialog becomes Primary
Error OSC policy                     → D3-07 Rest · persistent
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
Generic unnamed residues                     0
Visible unbound source/product paints        0
D5-02 source section fit after extension     PASS
D5-01～D5-05 protected sections              PASS
```

D5-01、D5-02 Verification、D5-03、D5-04、D5-05 的既有 Section 坐标和尺寸全部保持不变；唯一历史区结构变化是 D5-02 Source Section 高度 `1500 → 1660`，原因是扩展同一个 `Player Status Overlay` authority，而不是建立平行 Error Overlay。

本任务只修改 Figma 设计、一个 Figma semantic token 与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

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

本任务只修改 Figma 设计与根 README；没有播放器源码、配置、依赖、数据格式或运行时接口变化，因此没有构建、单元测试或运行时测试项。

**D5-07：Complete。**

## Stage D5 Closing Result

任务书要求的 Empty / Loading / Playing / Paused / Buffering / Seeking / Ended / Error，以及 Seek Preview / HUD / Toast / Dialog 已全部形成单一职责 authority。Feedback Priority、current-media Overlay ownership、Seek 真值、OSC persistent、单槽 HUD/Toast、Dialog 决策 ownership 与复杂背景可读性均已完成验证；没有新增第五种反馈层，也没有把暂停、错误或恢复设计成第二套播放器页面。

**Stage D5：Complete。**

## Stage D6 — Preferences & Shortcuts · In Progress

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

## Next

**D6-02 — Source List Navigation**

下一步按 `docs/plans/stages/D6_Preferences与快捷键窗口.md` 建立 Preferences 分类导航：

```text
category
  → selection
  → content
```

重点完成 Playback / Video / Audio / Subtitles / Interface / Advanced / Shortcuts 的快速扫读结构，以及 Default / Hover / Selected / Focus；继续消费唯一 `Preferences / Shell`，不让 Source List 重新拥有窗口几何。