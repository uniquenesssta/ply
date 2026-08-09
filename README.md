# 第三版 Airy Glass UI 设计任务书包

此包是**独立 UI/UX 任务书**，仅学习现有 `docs/plans` 与 `docs/plans/stages` 的写作结构和 Atomic Task 组织方式。

不包含旧 R2–R14 开发任务整合，不包含当前源码开发进度，不包含 Screenbox 补强任务。

入口：
- `docs/plans/Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md`
- `docs/plans/stages/00_INDEX.md`
- `docs/records/`：每个已完成 Design Atomic Task 的独立事实记录

## Current design status

- 第三版核心框架：Main Player / Fullscreen / Playlist Inspector 已确认。
- D1：In Progress。
- D1-01：Complete — 初始建立 `V3 / Primitive` 15 个基础色；D1-02 在真实语义映射时补充 `ink-800` 与 `violet/glow`，当前 Primitive Color 共 17 个。
- D1-02：Complete — 已建立 `V3 / Semantic` / `Light Mist`，共 38 个 Semantic Color，并回刷 Main Player / Fullscreen / Playlist Inspector 的全部可见 Solid Paint。
- D1-03：Complete — 已建立 `V3 / Type Primitive` 14 个 Typography Variable 与 17 个 V3 Text Style；三张核心框架 `43 / 43` 个文本节点已绑定正式 Text Style。
- D1-04：Complete — 已建立 `V3 / Geometry Primitive` 38 个 FLOAT Variable 与 `V3 / Geometry Semantic` 49 个 Semantic Geometry；三张核心框架共 55 个目标节点已绑定 Size / Radius Geometry Variable。
- D1-04 Spacing：23 个 Semantic Spacing 已建立；当前探索框架保持绝对布局，不强制转换 Auto Layout；Foundations 中 8 / 8 个 Spacing specimen 已真实绑定 GAP Variable，正式 Window / OSC / Inspector 组件从 D2 / D3 / D4 开始消费 Gap / Padding Token。
- D1-04 回归：Typography `43 / 43`；Semantic Color `142 / 142`；均无漏绑。
- 下一任务：D1-05 建立 Glass / Blur / Shadow。

## Change Log

### 2026-08-09 — D1-04 建立 Spacing / Size / Radius

- 审计：从 Main Player / Fullscreen / Playlist Inspector 提取真实 Window / Header / OSC / Inspector / Search / Playlist Row / Control / Timeline 几何。
- 实现：新增 `V3 / Geometry Primitive`，共 38 个 FLOAT Variable：Spacing 17、Size 13、Radius 8。
- 实现：新增 `V3 / Geometry Semantic`，共 49 个 Semantic Geometry：Spacing 23、Size 14、Radius 12。
- 边界：不把居中坐标、Timeline Progress 动态宽度、图标内部光学微调或单次出现的视觉间隔 Token 化。
- 回刷：55 / 55 个目标产品节点已绑定 Size / Radius Geometry Variable，Missing = `0`。
- Spacing：8 / 8 个 Foundations Auto Layout specimen 的 `itemSpacing` 已绑定 Semantic GAP Variable；当前三张探索框架不为追求覆盖率而强制改 Auto Layout。
- Figma：在 `01 Foundations` 新增 `D1-04 / Spacing Size Radius` 文档板，包含 Semantic Spacing / Size / Radius / Product Geometry Map / Rules。
- 修复：第一次文档板创建因直接写 Frame 只读 `width/height` 被 Figma 原子回滚；改用 `resize()` 后重新完整创建并截图通过。
- 验证：Main / Fullscreen / Playlist Inspector 截图回归通过；Geometry Primitive 38、Semantic 49；55 / 55 产品目标节点绑定；8 / 8 GAP specimen 绑定；无 `13/15/17` 无语义 spacing token。
- 回归：D1-03 Typography 保持 `43 / 43`；D1-02 Semantic Color 保持 `142 / 142`，漏绑 `0`。
- 记录：`docs/records/D1-04_建立SpacingSizeRadius.md`。

### 2026-08-09 — D1-03 建立 Typography System

- 审计：三张核心框架共 43 个文本节点；原始字体集中为 Inter / Noto Sans SC，字号集中为 32 / 22 / 14 / 13 / 12 / 11 / 10。
- 实现：新增 `V3 / Type Primitive`，共 14 个 Font Family / Font Style / Font Size / Tracking Variable。
- 实现：新增 17 个本地 V3 Text Style，区分 Docs、Inspector Title、Media Title、Body、Metadata、Label、Keycap 与 Timecode。
- 决策：Inter 负责英文 UI；Noto Sans SC 负责中文和多语言媒体标题；Geist Mono 仅用于 Timecode，以满足稳定数字宽度要求。
- 回刷：Main Player / Fullscreen / Playlist Inspector 共 `43 / 43` 文本节点已绑定 V3 Text Style，Unstyled = `0`，Non-V3 / Mixed = `0`。
- 验证：Main / Fullscreen / Playlist Inspector 高分辨率截图通过；Typography 文档板截图通过；长媒体标题、中英/韩混排通过；Timecode M/S/XS width delta 均为 `0 px`。
- 回归：D1-02 Semantic Color 保持 `142 / 142` 可见 Solid Paint 已绑定，漏绑 `0`。
- 限制：Text Style 当前保持 Auto line-height，以避免已确认框架发生无依据布局漂移；固定行高若最终需要，将在 Handoff 基于实际字体度量补充。
- 记录：`docs/records/D1-03_建立TypographySystem.md`。

### 2026-08-09 — D1-02 建立语义颜色

- 实现：建立 38 个 Semantic Color，覆盖 Surface / Text / Icon / Border / Accent / Selection / Focus / Feedback / Control / Inspector。
- 补强：发现 D1-01 缺少两个真实框架色值，补充 `color/neutral/ink-800` 与 `color/violet/glow`，Primitive Color 当前共 17 个。
- 回刷：三张已确认核心框架的可见实色已开始消费 Semantic Variable。
- 修复：解决 Semantic Paint 绑定导致原 Paint opacity 被重置为 1.0 的问题；恢复 42 个 Fill opacity 与 8 个 Stroke opacity。
- Figma：在 `01 Foundations` 新增 `D1-02 / Semantic Color Roles` 文档板，并修复 Inspector 组裁切/节点引用问题。
- 验证：Main / Fullscreen / Playlist Inspector 视觉回归截图通过；Semantic 文档板截图通过；最终 Visible Solid Paint 审计 `142/142` 已绑定、漏绑 `0`。
- 记录：`docs/records/D1-02_建立语义颜色.md`。

### 2026-08-09 — D1-01 建立浅雾基础色谱

- 实现：建立初始 15 个 Primitive Color 变量与 Foundations 色谱板。
- 影响：仅新增设计基础变量和文档记录；未改变已确认的三张核心框架。
- 验证：真实取样三张框架；Figma 变量创建成功；色谱板截图复核通过。
- 记录：`docs/records/D1-01_建立浅雾基础色谱.md`。
