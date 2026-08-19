# Qt6 + libmpv 播放器 UI v3 · Figma 节点映射

生成日期：2026-08-19  
Figma 文件：[Qt6 + libmpv 播放器 UI · 第三版完全重构](https://www.figma.com/design/KIOxfwTvQJlcVLinkeJAxY/Qt6---libmpv-%E6%92%AD%E6%94%BE%E5%99%A8-UI-%C2%B7-%E7%AC%AC%E4%B8%89%E7%89%88%E5%AE%8C%E5%85%A8%E9%87%8D%E6%9E%84)  
文件 Key：`KIOxfwTvQJlcVLinkeJAxY`

## 1. 映射范围

本表是面向需求分析、设计核对和开发实现的结构节点映射，覆盖：

- 全部 10 个 Figma Page；
- D0–D9 各阶段的顶层 Section、Frame 和主要验证入口；
- 56 个关键 Component Set；
- 主要原型与状态入口。

没有展开文字层、图标路径、装饰图层和每个组件实例。执行具体开发任务时，应先定位本表中的阶段节点，再读取该节点及其必要子节点。

打开任意节点时，将节点 ID 中的 `:` 改成 `-`，追加到文件 URL：

```text
https://www.figma.com/design/KIOxfwTvQJlcVLinkeJAxY/Qt6---libmpv-播放器-UI-第三版完全重构?node-id=<节点ID>
```

例如：`166:195` → `node-id=166-195`。

## 2. 页面总览

| 阶段 | Page | Page ID | 顶层节点 | 结构节点 | Component | Component Set |
|---|---|---:|---:|---:|---:|---:|
| D0 / D1 / D8 | 00 Framework Preview | `0:1` | 6 | 160 | 1 | 0 |
| D1 | 01 Foundations | `8:2` | 10 | 276 | 0 | 0 |
| D2 | 02 Player Window | `48:11` | 5 | 181 | 0 | 0 |
| D3 | 03 OSC | `90:2` | 21 | 568 | 0 | 0 |
| D4 | 04 Inspector | `151:2` | 33 | 2,271 | 80 | 22 |
| D5 | 05 States & Feedback | `219:2` | 34 | 2,018 | 50 | 11 |
| D6 | 06 Preferences | `285:19` | 45 | 3,576 | 108 | 15 |
| D7 | 07 Window Modes | `382:5777` | 17 | 1,351 | 0 | 0 |
| D8 | 08 Design System | `431:559` | 14 | 957 | 92 | 8 |
| D9 | 09 Prototype & Handoff | `599:3362` | 43 | 1,630 | 0 | 0 |

结构节点总数：`12,988`。

## 3. D0–D9 快速路由

| 阶段 | 默认设计入口 | 验证入口 | 实现主题 |
|---|---|---|---|
| D0 | `712:2` D0 / Direction Board · Frozen Baseline | `4:2`、`4:48`、`4:88` 三个框架预览 | 视觉方向与冻结基线 |
| D1 | `8:2` 01 Foundations | `758:2` D1 / Global Theme Modes | 色彩、字体、尺寸、材质、动效、层级 |
| D2 | `48:11` 02 Player Window | 各 D2 节点内的 Contract / Stage | 播放器壳体、视频视口、浮动标题栏、Host 与响应式骨架 |
| D3 | `90:2` 03 OSC | D3-03 与 D3-07 Smoke / Prototype 节点 | OSC、时间轴、播放控制、音量、工具动作、显隐生命周期 |
| D4 | `151:2` 04 Inspector | 各功能 Section 的同级 Verification Section | Playlist、Tracks、Subtitles、Chapters、响应式与关闭策略 |
| D5 | `219:2` 05 States & Feedback | 各状态 Section 的同级 Verification Section | 空、加载、播放、暂停、缓冲、跳转、结束、错误、HUD、Toast、Dialog |
| D6 | `285:19` 06 Preferences | 各功能 Section 的同级 Verification / Prototype | 设置窗口、分类导航、控件、真实设置、快捷键 |
| D7 | `382:5777` 07 Window Modes | D7-01 至 D7-05 的 Verification Section | 全屏、Mini、窄屏、显隐和模式迁移 |
| D8 | `431:559` 08 Design System | D8-01 至 D8-07 的 Verification Section | 图标、控件、Surface、Composite、实例回填和命名治理 |
| D9 | `599:3362` 09 Prototype & Handoff | D9-01 至 D9-08 的 Verification / QA Section | 核心原型、交互链路、交付规格和最终验收 |

## 4. 详细节点映射

### 00 Framework Preview — `0:1`

| 节点 ID | 节点名称 | 用途 |
|---|---|---|
| `4:2` | Framework / Main Player | 主播放器视觉框架 |
| `4:48` | Framework / Fullscreen | 全屏视觉框架 |
| `4:88` | Framework / Playlist Inspector | 播放器 + Inspector 框架 |
| `712:2` | D0 / Direction Board · Frozen Baseline | D0 冻结视觉基线 |
| `745:3` | D8 / Canonical Transport Assets | 规范化 Transport 资源；含 `746:2` Stop 组件 |
| `758:2` | D1 / Global Theme Modes | 三个框架的深色主题对照 |

### 01 Foundations — `8:2`

| 节点 ID | 节点名称 | 用途 |
|---|---|---|
| `8:3` | D1-01 / Primitive Color Palette | 原始色板 |
| `14:2` | D1-02 / Semantic Color Roles | Surface、文字、图标、边框、反馈、Accent、Control、Inspector 语义色 |
| `21:2` | D1-03 / Typography System | 字体系统 |
| `30:2` | D1-04 / Spacing Size Radius | 间距、尺寸、圆角与产品几何映射 |
| `35:2` | D1-05 / Glass Blur Shadow | Window、Header、OSC、Control、Inspector、Field 材质 |
| `41:2` | D1-06 / Motion Opacity Z-order | 动效、不透明度与层级规则 |
| `41:155`–`41:200` | D1-06 Smoke / 1–4 | OSC、Inspector、HUD、Dialog 层级冒烟验证 |

### 02 Player Window — `48:11`

| 节点 ID | 节点名称 | 用途 |
|---|---|---|
| `50:2` | D2-01 / Player Window Shell Contract | Windowed、Maximized、安全边缘和所有权 |
| `57:2` | D2-02 / Video Viewport Contract | 16:9、21:9、4:3、9:16、纯音频、空背景、对比支持 |
| `65:2` | D2-03 / Floating Header Contract | 标题、长标题、无标题、窄屏和优先级 |
| `70:2` | D2-04 / Host Spatial Contract | OSC Host、Inspector Host、Overlay Host、Z Ladder |
| `79:2` | D2-05 / Responsive Window Skeleton | 720、960、1280、1600 宽度布局 |

### 03 OSC — `90:2`

| 节点 ID | 节点名称 | 用途 |
|---|---|---|
| `90:3` | D3-01 / OSC Surface & Internal Grid | OSC Surface、内部网格和 720–1600 宽度布局 |
| `95:8` | D3-02 / Timeline Basic Geometry | 时间轴视觉区、命中区、端点、未知时长、不可 Seek |
| `104:2` | D3-03 / Timeline Interaction States | Rest、Hover Preview、Scrubbing、Pending、Committed、Chapter Hover |
| `105:2`–`105:88` | D3-03 Smoke / 1–6 | 时间轴六类状态冒烟入口 |
| `117:2` | D3-04 / Transport Cluster | 主次播放控制、交互状态和响应式上下文 |
| `127:2` | D3-05 / Volume Cluster | Zero、Low、Medium、High、Muted、窄屏 Popover |
| `134:2` | D3-06 / Utility Action Cluster | 工具动作簇 |
| `142:2` | D3-07 / OSC Visibility Lifecycle | OSC 显隐生命周期 |
| `143:2`–`143:95` | D3-07 Smoke / 1–8 | 播放、隐藏、Hover、Scrub、Popover、暂停、错误、全屏策略验证 |

### 04 Inspector — `151:2`

| 子阶段 | 设计入口 | 验证入口 | 关键组件入口 |
|---|---|---|---|
| D4-01 Shell | `151:3` | `151:4` | `152:4` Inspector / Shell |
| D4-02 Navigation | `159:50` | `159:51` | `160:88` Inspector / Mode Switch |
| D4-03 Playlist | `166:195` | `166:196` | `168:260` Entry Row、`172:284` Content、`175:368` Status Footer |
| D4-04 Tracks | `178:582` | `178:583` | `179:716` Selection Row、`183:823` Content |
| D4-05 Subtitles | `188:1206` | `188:1207` | `190:1241` External File Row、`191:1313` Delay、`191:1565` Content |
| D4-06 Chapters | `197:2098` | `197:2099` | `198:2135` Entry Row、`199:2125` Footer、`200:2168` Content |
| D4-07 Responsive & Dismissal | `207:2497` | `209:2497` | `216:3436` Prototype Navigation |

Inspector Component Set：

| 节点 ID | 组件集 |
|---|---|
| `160:88` | Inspector / Mode Switch |
| `168:260` | Playlist / Entry Row |
| `172:284` | Playlist / Content |
| `175:368` | Playlist / Status Footer |
| `179:716` | Track / Selection Row |
| `181:594` | Tracks / Section Header |
| `181:607` | Tracks / Section Empty |
| `182:619` | Tracks / Audio Section |
| `182:657` | Tracks / Video Section |
| `183:670` | Tracks / Audio Delay Control |
| `183:823` | Tracks / Content |
| `189:1220` | Subtitles / Section Header |
| `189:1233` | Subtitles / Section Empty |
| `190:1224` | Subtitles / Add External Action |
| `190:1241` | Subtitles / External File Row |
| `190:1279` | Subtitles / Embedded Section |
| `190:1374` | Subtitles / External Section |
| `191:1313` | Subtitles / Delay Control |
| `191:1565` | Subtitles / Content |
| `198:2135` | Chapters / Entry Row |
| `199:2125` | Chapters / Status Footer |
| `200:2168` | Chapters / Content |

### 05 States & Feedback — `219:2`

| 子阶段 | 设计入口 | 验证入口 | 关键组件集 |
|---|---|---|---|
| D5-01 Priority Matrix | `220:2` | `221:2` | HUD / Toast / Overlay / Dialog 路由矩阵 |
| D5-02 Empty & Loading | `223:2` | `223:3` | `224:24` Open Media Action、`225:30` Player Status Overlay |
| D5-03 Playing & Paused | `234:66` | `234:67` | `237:69` Playing、`237:99` Paused 原型 |
| D5-04 Buffering & Seeking | `241:66` | `241:67` | `242:88` Buffering Status、`243:66` Seek Preview |
| D5-05 Ended | `254:119` | `254:120` | `255:159` Ended Action、`256:144` Ended Status |
| D5-06 Error Overlay | `262:186` | `262:187` | `264:225` Error Action、`265:246` Error Status |
| D5-07 HUD Toast Dialog | `271:301` | `271:302` | `272:339` HUD、`273:318` Toast、`274:342` Dialog |

状态反馈 Component Set：`224:24`、`225:30`、`242:88`、`255:159`、`256:144`、`264:225`、`265:246`、`272:339`、`273:318`、`274:317`、`274:342`。

### 06 Preferences — `285:19`

| 子阶段 | 设计入口 | 验证入口 | 原型 / 组件入口 |
|---|---|---|---|
| D6-01 Window Shell | `285:20` | `285:21` | `286:59` Preferences / Shell |
| D6-02 Source List | `295:74` | `295:75` | `299:313` Source List；`308:1077`–`308:1952` 分类和键盘原型 |
| D6-03 Settings Section & Row | `315:1847` | `315:1848` | `319:1853` Section Header、`336:2293` Settings Row |
| D6-04 Controls | `328:2160` | `328:2161` | `340:2601` Prototype Navigation |
| D6-05 Real Settings | `343:2625` | `343:2626` | `358:3747` Prototype Navigation |
| D6-06 Shortcuts | `363:4225` | `363:4226` | `363:4227` Prototype Navigation |

Preferences Component Set：

| 节点 ID | 组件集 |
|---|---|
| `286:59` | Preferences / Shell |
| `299:313` | Preferences / Source List |
| `319:1853` | Preferences / Settings Section Header |
| `336:2293` | Preferences / Settings Row |
| `329:2190` | Control / Toggle |
| `330:2187` | Control / Select |
| `331:2190` | Control / Slider |
| `332:2265` | Control / Segmented |
| `333:2194` | Preferences / Text Field |
| `345:2641` | Preferences / Settings Action |
| `365:4245` | Control / Search |
| `366:4232` | Control / Keycap |
| `367:4277` | Preferences / Shortcut Binding |
| `369:4292` | Preferences / Shortcut Action Row |
| `370:4471` | Preferences / Shortcuts Content |

真实设置分类原型：

- `358:3748` Playback
- `358:3753` Video
- `358:3758` Audio
- `358:3763` Subtitles
- `358:3768` Interface
- `358:3773` Advanced
- `358:3778` Shortcuts

快捷键状态原型：`373:5228` Default、`373:5366` Search、`373:5467` Capturing、`373:5568` Conflict、`373:5685` Custom。

### 07 Window Modes — `382:5777`

| 子阶段 | 设计入口 | 验证入口 | 主题 |
|---|---|---|---|
| D7-01 | `382:5778` | `382:5779` | Fullscreen Composition |
| D7-02 | `393:2` | `393:3` | Fullscreen Visibility Contract |
| D7-03 | `401:47` | `401:48` | Mini Player |
| D7-04 | `407:47` | `407:48` | Narrow Responsive Policy |
| D7-05 | `418:203` | `419:203` | Mode Transition Contract |
| D7-05 Prototype | `420:381` | `420:386`–`420:651` | Windowed ↔ Fullscreen ↔ Mini 与 Inspector 恢复 |

### 08 Design System — `431:559`

| 子阶段 | 设计入口 | 验证入口 | 主题 |
|---|---|---|---|
| D8-01 | `432:2` | `432:175` | Duplicate Structure Audit |
| D8-02 | `439:2` | `439:4` | Icon Language |
| D8-03 | `450:106` | `450:108` | Controls |
| D8-04 | `470:308` | `470:310` | Surfaces |
| D8-05 | `491:397` | `491:399` | Composite |
| D8-06 | `529:676` | `529:678` | Product Instance Backfill |
| D8-07 | `591:676` | `591:677` | Naming Variable Hierarchy Hygiene |

Design System Component Set：

| 节点 ID | 组件集 |
|---|---|
| `451:154` | Control / Icon Button |
| `452:148` | Control / Playback Button |
| `453:341` | Control / Timeline |
| `474:308` | Surface / OSC |
| `494:10663` | Composite / Floating Header |
| `506:10735` | __Composite / Source Icon · Default |
| `506:10777` | __Composite / Source Icon · Selected |
| `507:537` | Composite / Source List Item |

### 09 Prototype & Handoff — `599:3362`

| 子阶段 | 原型 / 规格入口 | 验证入口 | 关键状态 |
|---|---|---|---|
| D9-01 Core Playback | `605:203` | `606:203` | `600:2` Empty、`600:24` Loading、`600:43` Playing Visible、`600:147` Hidden、`600:166` Paused |
| D9-02 Timeline Seek | `617:580` | `618:580` | `614:203` Rest、`614:298` Hover、`614:414` Scrubbing、`614:530` Pending、`614:647` Resume |
| D9-03 Inspector | `627:1837` | `627:1891` | Standard Closed / Playlist / Tracks / Subtitles / Chapters；720 Narrow |
| D9-04 Window Mode | `638:2454` | `638:2508` | Windowed、Fullscreen、Mini、Inspector Suspend / Restore |
| D9-05 Error Recovery | `648:2769` | `648:2823` | `641:2454` Retryable、`641:2583` NonRecoverable、`641:2697` Decision Dialog |
| D9-06 Handoff | `660:2769` | `662:2769` | 交付规格 |
| D9-07 Visual QA | `680:2795` | `681:2795` | 全局视觉验收 |
| D9-08 Structural QA | `715:2795` | `715:2842` | 结构与设计系统验收 |

D9-03 Inspector 原型入口：

- `622:939` Standard Closed
- `622:961` Standard Inspector / Playlist
- `623:1062` Standard Inspector / Tracks
- `623:1363` Standard Inspector / Subtitles
- `623:1611` Standard Inspector / Chapters
- `624:3827` Narrow Closed / 720
- `624:3850` Narrow Inspector / Playlist

D9-04 模式原型入口：

- `634:1837` Windowed Clean
- `634:1934` Fullscreen Active
- `634:2028` Mini Hover
- `636:1993` Windowed Inspector Before
- `636:2193` Fullscreen Inspector Suspended
- `636:2270` Windowed Inspector Restored
- `636:2464` Windowed More Open QA

## 5. 后续任务的节点选择规则

1. 需求分析或实现某个 Atomic Task 时，优先读取对应 `D?-?? / 功能名` 的设计入口。
2. 同步读取其同级 `Verification` 节点作为验收依据。
3. 涉及交互链路时，再读取对应 `Prototype` 或 `Smoke` 节点；不要用原型帧替代组件规范。
4. 涉及组件状态、尺寸或变体时，读取对应 Component Set，而不是只读取页面上的某个 Instance。
5. 涉及跨页面复用时，先以 `08 Design System` 为规范源，再核对业务页面实例。
6. 涉及最终行为链路或交付检查时，以 `09 Prototype & Handoff` 为整体验收入口。
7. 后续提示词可直接写：`Figma 目标：Page 04 Inspector；设计节点 166:195；验证节点 166:196；组件节点 168:260、172:284、175:368。`

## 6. 已知边界

- 本映射来自只读结构扫描，没有修改 Figma 文件。
- 统计中的“结构节点”包括 Section、Frame、Group、Component、Component Set 和 Instance。
- 节点名称和 ID 以 2026-08-19 扫描结果为准；Figma 中复制、删除或重建节点后，ID 可能变化。
- 具体开发前仍需读取目标节点的布局、样式、变量、文本、交互和必要子节点，不能仅凭本表还原界面。
