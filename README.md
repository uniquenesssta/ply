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
- D2-05 验证宽度：`720 / 960 / 1280 / 1600`；边界验证：`839 / 840 / 1199 / 1200`。
- D2-05 Responsive Semantic：16 个 Variable；四个测试 Window 已真实消费 Header / OSC / Inspector / hit-target 响应式尺寸。
- D2-05 最终审计：Visible Solid Paint `118/118` Semantic-bound，Unbound=`0`；D1 回归 Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`。
- **下一任务：D3-01 OSC Surface 与内部网格。**

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

## Change Log

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
