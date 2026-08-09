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
- D1-02 最终审计：`142 / 142` 个可见 Solid Paint 已绑定 Semantic Color，Visible Unbound = `0`；23 个隐藏占位 Fill 忽略，6 个 Gradient 留待材质阶段。
- 下一任务：D1-03 建立 Typography System。

## Change Log

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
