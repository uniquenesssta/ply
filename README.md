# 第三版 Airy Glass UI 设计任务书包

此分支用于第三版 Airy Glass UI/UX 设计任务书、Figma 设计执行与 Atomic Task 完成记录。

不整合旧 R2–R14 源码开发任务；播放器产品结构按 Window / OSC / Inspector / Overlay / Preferences / Window Modes / Design System 组织。

入口：
- `docs/plans/Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md`
- `docs/plans/stages/00_INDEX.md`
- `docs/records/`：每个已完成 Design Atomic Task 的独立事实记录

## Current design status

- 第三版核心框架：Main Player / Fullscreen / Playlist Inspector 已确认。
- **D1：Complete — D1-01 ～ D1-06 全部关闭。**
- **D2：Complete — D2-01 ～ D2-05 全部关闭。**
- D2 最终产物：Window Surface、Video Viewport、Floating Header、OSC / Inspector / Overlay Host Contract、Narrow / Standard / Wide Responsive Contract。
- D2-05 Breakpoint：Narrow=`0–839`、Standard=`840–1199`、Wide=`>=1200`。
- **D3：In Progress。**
- D3-01：Complete — 已建立正式 `03 OSC` 与 `D3-01 / OSC Surface & Internal Grid`（`90:3`）；OSC width=`min(host-52,880)`，Narrow=`106px`、Standard/Wide=`124px`。
- D3-02：Complete — 已建立 `D3-02 / Timeline Basic Geometry`（`95:8`）；Timeline=`3px Track / 16px Hit / 10px Thumb / R2`。
- **D3-03：Complete — 已建立 `D3-03 / Timeline Interaction States`（`104:2`）。**
- D3-03 Ownership：Preview≠Playback Position；Scrub 只拥有 temporary target；Pending Seek 在 backend confirmation 前不改写 confirmed position。
- D3-03 States：Rest / Hover Preview / Scrubbing / Pending Seek / Committed / Chapter Hover；另有 ESC Cancel → Rest。
- D3-03 Preview：Cyan Preview Marker=`6×6 / 92%`；Seek Preview Bubble=`64×28 / R14 / 52% glass`。
- D3-03 Chapter：Marker=`1×7`，Idle=`34%`，Hover/Target=`90%`；不创建第二条 progress track。
- D3-03 Motion：Preview=`120ms`、Scrub update=`0ms`、Pending/Commit=`160ms`、Cancel=`120ms`；Reduce Motion 全部=`0ms`。
- D3-03 Prototype：6 个 Page 顶层 Smoke Frame、6 条主链 Reaction、1 条 ESC Cancel Reaction。
- D3-03 最终审计：Visible Solid Paint=`302/302` Semantic-bound、Unbound=`0`；D1 回归 `142/142 · 43/43 · 55/55 · 25/25`；D2-01～05、D3-01/02 保持。
- **下一任务：D3-04 Transport Cluster。**

## Completed design tasks

### D1 Foundations

- D1-01：建立浅雾基础色谱。
- D1-02：建立 Semantic Color，并回刷三张核心框架。
- D1-03：建立 Typography System，框架文本 `43/43` 使用正式 Text Style。
- D1-04：建立 Spacing / Size / Radius Geometry System。
- D1-05：建立 Glass / Blur / Shadow Material System。
- D1-06：建立 Motion / Opacity / Z-order；Stage D1 关闭。

### D2 Player Window System

- D2-01：定义 Player Window 外壳；Standard=`1320×700 / R32 / border70% / clipping`，Maximized=`R0 / no elevation`。
- D2-02：定义 Video Viewport；冻结 `Fit + preserve aspect + centered + no stretch + no crop`，覆盖 16:9 / 21:9 / 4:3 / 9:16 / Audio / Empty。
- D2-03：建立 Floating Header；采用 `Media Info Pod + Window Actions Pod`，Playback State 不进入 Header。
- D2-04：建立 Host 空间契约；冻结 Overlay z35 / OSC z40 / Inspector z50 及 12px / 26px 硬间隔。
- D2-05：建立响应式窗口骨架；冻结 Narrow / Standard / Wide breakpoint 与 Inspector overlay/dock、OSC compact/standard、Utility/Volume 降级策略。

### D3 OSC System

- D3-01：建立 OSC Surface 与内部网格；冻结 Max Width、两层 Vertical Grid 与 Narrow / Standard / Wide 响应式几何。
- D3-02：建立 Timeline 基础几何；冻结 3px Track、16px Hit Target、10px Thumb、Buffer/Progress/Disabled 语义与端点公式。
- D3-03：建立 Timeline 交互状态；冻结 Preview / Scrub / Pending / Commit / Cancel / Chapter Marker 的所有权、视觉状态与 Prototype 链。

## Change Log

### 2026-08-09 — D3-03 Timeline 交互状态

- Figma：新增 `D3-03 / Timeline Interaction States`（`104:2`），覆盖 Rest / Hover Preview / Scrubbing / Pending Seek / Committed / Chapter Hover。
- Ownership：PlaybackSnapshot 仍是 confirmed position 唯一真值；Preview / Scrub / Pending 均不得提前改写真实 playback position。
- Geometry：新增 Preview Marker=`6×6`、Chapter Marker=`1×7`、Seek Preview Bubble=`64×28 / R14`；D3-02 的 3px Track / 16px Hit / 10px Thumb 保持不变。
- Visual：Hover 使用 Cyan Preview；Scrub 保留 weak committed marker；Pending 使用 Soft Pending Range + Hollow Target；Chapter Marker Idle=`34%`、Hover=`90%`。
- Motion：Preview show/hide=`120ms`、Scrub update=`0ms`、Pending/Commit=`160ms`、Cancel=`120ms`；Reduce Motion 下全部 duration=`0ms`。
- Prototype：新增 6 个 Page 顶层 Smoke Frame；6 条主链 Smart Animate；`105:53` 提供 ESC Cancel → Rest 独立路径。
- 修复：初次 Board 文本只有 Text Style、未绑定 Semantic Color，已全量回刷；交互低透明度被 Variable Binding 写回 100%，已恢复；`Chapter Marker Contract` 曾被过宽名称匹配误绑成 `1×7`，已恢复 `1696×250 / glass36% / border48%` 并收紧匹配规则。
- 验证：D3-03 Visible Solid Paint=`302/302` Semantic-bound、Unbound=`0`；Chapter/Preview/Pending opacity 与 Geometry binding 全部通过；D1 回归 `142/142 · 43/43 · 55/55 · 25/25`；D2-01～05、D3-01/02 保持。
- 记录：`docs/records/D3-03_Timeline交互状态.md`。
- 下一任务：`D3-04 Transport Cluster`。

### 2026-08-09 — D3-02 Timeline 基础几何

- Figma：新增 `D3-02 / Timeline Basic Geometry`（`95:8`），覆盖 Anatomy、0/50/100%、Unknown、Non-seekable 与 560/728/828 Width Stress。
- Geometry：Visual Track=`3px`、Hit Target=`16px`、Thumb=`10px`、Track Radius=`2px`；`trackX=5`、`trackWidth=W-10`、`thumbX=ratio×trackWidth`。
- Endpoint QA：252px Timeline 的 0/50/100% Thumb left=`0/121/242`，100% Thumb right=`252`，右端误差=`0`。
- Responsive：`timeline/control-offset` 为 Narrow=`10`、Standard/Wide=`12`；Track/Thumb/Hit Target 尺寸不因 Narrow 缩小。
- Color：新增 `control/buffered` 与 `control/thumb-border`；Thumb 使用 1px / 28% 柔紫 inside stroke，解决亮色视频背景可读性。
- 状态：Unknown 与 Non-seekable 保持同一 3px Base Track，不创建第二套几何；Progress/Buffer/Thumb 按状态移除或禁用。
- 修复：第一轮 Hit Target 工程辅助层过强，已改为 Anatomy 极淡 outline、其余产品态完全隐藏；Thumb Border 变量绑定后 Stroke opacity 被写回 100%，已恢复全部 7 个 Thumb 到 28%。
- 验证：D3-02 Visible Solid Paint=`144/144` Semantic-bound、Unbound=`0`；9 个 Timeline 样本 Hit Target=`16`、Track=`3`；D1 回归 `142/142 · 43/43 · 55/55 · 25/25`；D2-01～05 与 D3-01 保持。
- 记录：`docs/records/D3-02_Timeline基础几何.md`。

### 2026-08-09 — D3-01 OSC Surface 与内部网格

- Figma：新增 Page `03 OSC`（`90:2`）与 `D3-01 / OSC Surface & Internal Grid`（`90:3`）。
- Width：冻结 `surface=min(hostWidth-2×26,880)`；720/960/1280/1600 分别得到 `616/856/780/880`，全部 width error=`0`。
- Grid：正式结构固定为 `Timeline Lane → Control Lane`；Narrow=`106px`，Standard/Wide=`124px`，Primary Playback 40px 不因 Narrow 缩小。
- Geometry：新增 `size/26`、`size/28`、`size/880` 以及 11 个 OSC Geometry Semantic；当前 Geometry Semantic=`73`。
- Responsive：新增 `osc/inset / radius / timeline-lane-height / control-lane-height / section-gap / padding-top / padding-bottom`，Responsive Semantic=`23`。
- Material：Narrow OSC=`32% fill / 48% stroke / R32 / V3 Glass OSC Compact`；Standard/Wide=`34% / 48% / R34 / V3 Glass OSC`。
- Ownership：D3-01 只冻结 Surface + Grid Slots；Timeline 细节留 D3-02/03，Transport/Volume/Utility 留 D3-04/05/06，Visibility 留 D3-07。
- 修复：第一次 Board 创建因 Rectangle 不能 append Label 被 Figma 原子回滚；第二轮创建后又发现 Semantic Paint Binding 把 OSC/Guide/Inspector alpha 写回 100%，已恢复 55 个节点目标透明度，并将 Inspector Context 改为 outline-only。
- 验证：D3-01 Visible Solid Paint=`141/141` Semantic-bound、Unbound=`0`；四档 Surface width/vertical sum 全部精确；D1 回归 `142/142 · 43/43 · 55/55 · 25/25`；D2-01～05 全部存在且未修改。
- 记录：`docs/records/D3-01_OSCSurface与内部网格.md`。

### 2026-08-09 — D2-05 响应式窗口骨架；Stage D2 关闭

- Figma：新增 `D2-05 / Responsive Window Skeleton`（`79:2`），真实建立 `720 / 960 / 1280 / 1600` 四个 700px 高 Window 骨架。
- Responsive：新增 `V3 / Responsive Primitive` 3 个 breakpoint 与 `V3 / Responsive Semantic` 16 个 Variable，Mode=`Narrow / Standard / Wide`。
- Mode：Narrow=`0–839`、Standard=`840–1199`、Wide=`>=1200`；Inspector 分别采用 `320 overlay / 368 overlay / 368 dock`。
- Controls：Desktop utility/window action hit-min 最终定为 `32`；Primary Playback 保持 `40`；Narrow 只降低视觉密度，不缩点击目标。
- Binding：Header Info Width/Height、Window Actions Height、OSC Height、Inspector Width、12 个 Window Action hit target 均接入 Responsive Variable。
- Boundary QA：`839 / 840 / 1199 / 1200` 均无 Header / Inspector / OSC / Overlay 碰撞；关键间隔保持 `12 / 26 / 26`。
- 修复：BOOLEAN / STRING Variable 不支持强写 scopes，首次 mutation 原子回滚；Semantic Paint opacity 再次被写成 100%，已恢复 Host/Header/Window 材质透明度；删除与 Host label 重叠的 Policy Chip。
- 验证：D2-05 Visible Solid Paint=`118/118` Semantic-bound、Unbound=`0`；D1 回归 Color=`142/142`、Typography=`43/43`、Geometry=`55/55`、Effect=`25/25`；D2-01～D2-04 全部保持。
- 记录：`docs/records/D2-05_响应式窗口骨架.md`。
- 结论：Stage D2 关闭；下一任务 `D3-01 OSC Surface 与内部网格`。

### 2026-08-09 — D2-04 Host 空间契约

- 建立 `D2-04 / Host Spatial Contract`（`70:2`），冻结 OSC / Inspector / Overlay 安全区、碰撞和 z-order；记录见 `docs/records/D2-04_建立Host空间契约.md`。

### 2026-08-09 — D2-03 Floating Header

- 建立 `D2-03 / Floating Header Contract`（`65:2`），冻结 Split Pod、长标题、无标题、Compact Header 和局部 Contrast Support；记录见 `docs/records/D2-03_建立FloatingHeader.md`。

### 2026-08-09 — D2-02 Video Viewport

- 建立 `D2-02 / Video Viewport Contract`（`57:2`），冻结媒体 Fit、Letterbox、Audio/Empty 与 Contrast Support；记录见 `docs/records/D2-02_定义VideoViewport.md`。

### 2026-08-09 — D2-01 Player Window 外壳

- 建立 `D2-01 / Player Window Shell Contract`（`50:2`），冻结 Window Surface、Maximized、Safe Area 和 clipping；记录见 `docs/records/D2-01_定义PlayerWindow外壳.md`。

### 2026-08-09 — D1 Foundations

- D1-01 ～ D1-06 全部完成并关闭；各 Atomic Task 事实记录位于 `docs/records/`。
