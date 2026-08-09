# 第三版 Airy Glass UI 设计任务书包

此包是**独立 UI/UX 任务书**，仅学习现有 `docs/plans` 与 `docs/plans/stages` 的写作结构和 Atomic Task 组织方式。

不包含旧 R2–R14 开发任务整合，不包含当前源码开发进度，不包含 Screenbox 补强任务。

入口：
- `docs/plans/Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md`
- `docs/plans/stages/00_INDEX.md`
- `docs/records/`：每个已完成 Design Atomic Task 的独立事实记录

## Current design status

- 第三版核心框架：Main Player / Fullscreen / Playlist Inspector 已确认。
- **D1：Complete — D1-01 ～ D1-06 全部关闭。**
- D1-01：Complete — 初始建立 `V3 / Primitive` 15 个基础色；D1-02 在真实语义映射时补充 `ink-800` 与 `violet/glow`，当前 Primitive Color 共 17 个。
- D1-02：Complete — 已建立 `V3 / Semantic` / `Light Mist`，共 38 个 Semantic Color，并回刷 Main Player / Fullscreen / Playlist Inspector 的全部可见 Solid Paint。
- D1-03：Complete — 已建立 `V3 / Type Primitive` 14 个 Typography Variable 与 17 个 V3 Text Style；三张核心框架 `43 / 43` 个文本节点已绑定正式 Text Style。
- D1-04：Complete — 已建立 `V3 / Geometry Primitive` 38 个 FLOAT Variable 与 `V3 / Geometry Semantic` 49 个 Semantic Geometry；三张核心框架共 55 个目标节点已绑定 Size / Radius Geometry Variable。
- D1-04 Spacing：23 个 Semantic Spacing 已建立；当前探索框架保持绝对布局，不强制转换 Auto Layout；Foundations 中 8 / 8 个 Spacing specimen 已真实绑定 GAP Variable，正式 Window / OSC / Inspector 组件从 D2 / D3 / D4 开始消费 Gap / Padding Token。
- D1-05：Complete — 已建立 `V3 / Effect Primitive` 38 个 FLOAT Variable、`V3 / Material Semantic` 47 个 Semantic Material Variable，以及 16 个本地 V3 Effect Style；当前三张核心框架 25 / 25 个真实 Effect 节点已绑定正式 Effect Style。
- D1-06：Complete — 已建立 `V3 / Interaction Primitive` 26 个 Variable 与 `V3 / Interaction Semantic` 42 个 Semantic Interaction Variable，包含 `Standard / Reduce Motion` 两个 Mode。
- D1-06 Motion：12 / 12 个 Semantic Duration 在 `Reduce Motion` 下解析为 `0 ms`；建立 4-step Smoke Prototype 与 3 段真实 Smart Animate Reaction。
- D1-06 Opacity / Z-order：Close 72% 已绑定 `opacity/control/idle`；建立 11 级 `z/*` 契约，并给当前 9 个关键产品层写入共享 Z-role 注记。
- D1 最终回归：Semantic Color `142 / 142`；Typography `43 / 43`；Geometry `55 / 55`；Effect `25 / 25`；三张核心框架截图均通过。
- **D2：In Progress。**
- D2-01：Complete — 已建立正式 `02 Player Window` 与 Window Surface Contract；Standard=`1320×700 / R32 / Window Elevation / border 70% / clipping`，Maximized=`R0 / no elevation / no outer border / clipping`。
- D2-01 Token 补强：Geometry Primitive `38 → 41`、Geometry Semantic `49 → 53`、Effect Primitive `38 → 39`、Material Semantic `47 → 48`；新增 reference size、safe-min、maximized radius 与 window border alpha 语义。
- D2-01 验证：Product Solid Paint `26 / 26` Semantic-bound；D1 回归 Semantic Color `142 / 142`、Typography `43 / 43`、Geometry `55 / 55`、Effect `25 / 25`。
- D2-02：Complete — 已建立 `D2-02 / Video Viewport Contract`（`57:2`），冻结默认 `Fit + preserve aspect + centered + no stretch + no crop`，并验证 16:9 / 21:9 / 4:3 / 9:16 / Audio-only / Empty。
- D2-02 Token 补强：Semantic Color `38 → 42`；Effect Primitive `39 → 40`；Material Semantic `48 → 49`；Interaction Primitive `26 → 27`；Interaction Semantic `42 → 43`；新增 letterbox/audio/empty/contrast-support、16% contrast alpha 与 `z/contrast-support=25`。
- D2-02 验证：四种视频比例 Fit 最大误差 `<0.000011 px`；D2-02 Visible Solid Paint `126 / 126` Semantic-bound，Synthetic Media Gradient `6` 个故意不主题化；Contrast Support OFF=`0` node，ON=`16% / z25`；D1 回归仍为 Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`；D2-01 Standard Window 保持不变。
- D2-03：Complete — 已建立 `D2-03 / Floating Header Contract`（`65:2`），正式采用 `Media Info Pod + Window Actions Pod` 双浮层结构；Playback State 从 Header 移出，归 D3 / D5。
- D2-03 Token 补强：Geometry Primitive `41 → 46`、Geometry Semantic `53 → 58`；新增 Header info/action 宽度、Title-Meta gap 与 Action inset。
- D2-03 验证：Long Title=`ENDING` ellipsis；No-title Info Pod=`0`；720-width Metadata=`0`；4 / 4 Close glyph 正确；Local Contrast Support=`16% / z25`；D2-03 Visible Solid Paint `85 / 85` Semantic-bound；D1 回归仍为 Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`。
- D2-04：Complete — 已建立 `D2-04 / Host Spatial Contract`（`70:2`），冻结 OSC / Inspector / Overlay 的安全区、碰撞与层级规则；Inspector Open 不 resize Video，只缩小 OSC / Overlay 的 floating range。
- D2-04 Token 补强：Geometry Primitive `46 → 47`、Geometry Semantic `58 → 59`；Interaction Primitive `27 → 28`、Interaction Semantic `43 → 44`；新增 `spacing/inspector/top-with-header=92` 与 `z/overlay=35`。
- D2-04 验证：Header→Inspector=`12px`、Overlay→OSC=`26px`、OSC→Inspector=`26px`、Overlay→Inspector=`26px`；OSC Height binding=`3/3`、Inspector Width binding=`2/2`；D2-04 Visible Solid Paint `95/95` Semantic-bound；D1 回归仍为 Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`；D2-01～03 保持不变。
- **下一任务：D2-05 响应式窗口骨架。**

## Change Log

### 2026-08-09 — D2-04 建立 Host 空间契约

- 结构：在 `02 Player Window` 新增 `D2-04 / Host Spatial Contract`（`70:2`），只定义 OSC Host / Inspector Host / Overlay Host 的位置、可用范围、碰撞和层级，不提前实现 D3 / D4 / D5 Feature 内容。
- 审计：发现探索稿 Inspector `y24 / h652` 会与 D2-03 固定 Window Actions `y26 / h54` 发生真实碰撞；禁止通过临时移动 Window Actions、提高单节点 z 或挖异形 Inspector 缺口规避。
- Inspector Contract：正式改为 `x924 / y92 / w368 / h584`；`top=26+54+12=92`，right=`28`，bottom=`24`，与 Window Actions 保持 `12px` 硬间隔。
- OSC Contract：bottom=`38`、height reference=`124`、top=`538`；Inspector Closed 为 `x26 / w1268`，Open 为 `x26 / w872`，右边界始终停在 Inspector left 前 `26px`。
- Overlay Contract：新增 `z/overlay=35`；content safe rect vertical=`y106…512`，Closed=`x26 / w1268`，Inspector Open=`x26 / w872`；位于 Header 下方、OSC 上方、Inspector 左侧。
- Layer：`Media z20 → Header z30 → Overlay z35 → OSC z40 → Inspector z50 → Popover z60 → HUD z70 → Dialog z100`。
- Geometry Token：新增 `spacing/92` 与 `spacing/inspector/top-with-header → 92`；Geometry Primitive `46→47`、Semantic `58→59`。
- Interaction Token：新增 `z/35` 与 `z/overlay → z35`；Interaction Primitive `27→28`、Semantic `43→44`。
- Simultaneous State：真实摆放 D5 Error Placeholder 于 Overlay z35，同时保留 OSC z40 和 Inspector z50，验证三层共存且 Window / Video 不改变尺寸。
- 修复：第一轮 Host Fill 虽几何正确但青/紫/橙色块过重，容易被误认成真实产品 Surface；第二轮收敛为 Overlay `2.5%`、OSC `3%`、Inspector `4.5%` 的极淡填充 + outline，并新增显式 Z Ladder。
- Binding：3/3 OSC Host height 绑定 `size/osc/height`；2/2 Inspector Host width 绑定 `size/inspector/width`；所有 Host 写入共享 `z-role`。
- 验证：硬间隔 Header→Inspector=`12`、Overlay→OSC=`26`、OSC→Inspector=`26`、Overlay→Inspector=`26`；D2-04 Board Visible Solid Paint=`95/95` Semantic-bound、Unbound=`0`。
- 回归：D1 Semantic Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`；D2-01 Standard Window 保持 `1320×700 / R32 / clipping / border70% / Window Elevation`，D2-02 / D2-03 Contract 均存在且未修改。
- 记录：`docs/records/D2-04_建立Host空间契约.md`。
- 下一任务：`D2-05 响应式窗口骨架`。

### 2026-08-09 — D2-03 建立 Floating Header

- 结构：在 `02 Player Window` 新增 `D2-03 / Floating Header Contract`（`65:2`）；正式 Header 拆为 `Media Info Pod + Window Actions Pod`，不形成 full-width title bar。
- Ownership：Header 只拥有 Media Title / Technical Metadata / Window Actions；探索稿中的 `PLAYING` 不进入正式 Header，Playback State 归 D3 / D5。
- Geometry：新增 5 个 Primitive 与 5 个 Semantic Geometry：`size/header/info-width=420`、`size/header/info-width-compact=360`、`size/header/actions-width=110`、`spacing/header/title-meta=2`、`spacing/header/actions-inset=16`；Geometry Primitive `41→46`、Semantic `53→58`。
- Standard：Media Info=`420×54 / R27`，Window Actions=`110×54 / R27`，Top safe edge=`26`；两个 Pod 独立占位。
- Long title：Media Title 固定单行并实际 `textTruncation=ENDING`，content width=`376`；Window Actions 不位移。
- Priority：`Window Actions > Media Title > Technical Metadata`；空间不足时 Metadata 先隐藏，Title 继续截断。
- No-title：Media Info Pod=`0`，只保留 Window Actions，禁止显示空白玻璃胶囊。
- Narrow stress：720-width 只验证 Header 自身 Compact，不冻结 D2-05 breakpoint；Info=`360×50 / R25`，Actions=`110×50 / R25`，Metadata nodes=`0`。
- Window Actions：Minimize / Maximize-Restore / Close；Standard visual slot=`22`、Compact=`21`、gap=`6`、inset=`16`、idle opacity=`72%`，颜色绑定 `icon/secondary`。
- Contrast Support：Bright-media 初版整条 Top Band 不符合 D2-02 local-only 契约，已改为 Media Info `460×82` 与 Window Actions `140×82` 两个局部区域，均为 `16% / z25`。
- 修复：第一次 Board 创建因 synthetic gradient helper 少一个右括号被 Figma 原子回滚；无半成品残留。第一轮截图还发现 Close glyph 因 Line 旋转基点错误呈尖括号，4/4 已替换为标准 X path 并重新绑定 `icon/secondary`。
- 验证：D2-03 Visible Solid Paint `85/85` Semantic-bound、Unbound=`0`；Long Title ENDING；No-title Info Pod=`0`；Narrow Metadata=`0`；4 个 Close 均正确且 idle=`72%`；D1 回归 `142/142 · 43/43 · 55/55 · 25/25`；D2-01 Standard Window 保持不变，D2-02 Contract 未修改。
- 记录：`docs/records/D2-03_建立FloatingHeader.md`。
- 下一任务：`D2-04 建立 Host 空间契约`。

### 2026-08-09 — D2-02 定义 Video Viewport

- 结构：在 `02 Player Window` 新增 `D2-02 / Video Viewport Contract`（`57:2`），不修改已确认 Framework，也不把 D2-01 Window Contract 改成成品播放器。
- Fit：冻结 `preserve aspect / centered / no stretch / no crop by default`；50% Reference Viewport `660×350` 验证 16:9=`622.2222×350`、21:9=`660×282.8571`、4:3=`466.6667×350`、9:16=`196.875×350`，最大 Figma FLOAT 误差 `<0.000011 px`。
- Letterbox：新增 `surface/letterbox → color/neutral/ink-950`；Bars 明确由 Video Viewport 拥有，Window / OSC / Inspector 不得通过改变媒体 Fit 制造空间。
- Audio：新增 `surface/audio → mist-100`；Audio-only 不创建假视频像素，Lavender / Cyan atmosphere 最终 node opacity=`16% / 10%`。
- Empty：新增 `surface/empty → mist-50`；只定义安静背景，Empty CTA / Loading / Error 留给 D5；Empty Lavender atmosphere 最终收敛为 `4.5%`。
- Contrast Support：新增 `overlay/contrast-support → ink-950`、`alpha/viewport/contrast-support=16%`、`z/contrast-support=25`；默认 OFF，只有未来 Header / OSC 在具体画面可读性不足时局部开启。
- Layer contract：更新为 `video z0 → atmosphere z10 → media content z20 → contrast support z25 → floating controls z30+`。
- Media pixels：6 个 Synthetic Video Gradient 只用于表达任意媒体内容，故意不绑定产品主题 Token；应用自身拥有的 D2-02 Visible Solid Paint `126/126` Semantic-bound、Unbound=`0`。
- 修复：第一轮创建再次遇到 `setBoundVariableForPaint()` 把 Paint opacity 写回 `1.0`，导致 Empty Glow、Contrast Support、Audio Glow 与测试 pill 过强；第二轮独立恢复 44 个 Paint / Node opacity 后重新截图通过。
- 验证：Aspect Fit、Letterbox/Pillarbox、Audio/Empty、Contrast OFF/ON 与 Ownership Contract 全部截图通过；Contrast OFF support nodes=`0`，ON Top/Bottom=`16%` 且 shared z-role=`z/contrast-support`。
- 回归：D1 Semantic Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`；D2-01 Standard Window 仍为 `1320×700 / R32 / clipsContent / Window Elevation / border 70%`。
- 记录：`docs/records/D2-02_定义VideoViewport.md`。
- 下一任务：`D2-03 建立 Floating Header`。

### 2026-08-09 — D2-01 定义 Player Window 外壳

- 审计：从已确认 Main Player 读取真实 Window Shell：`1320×700 / R32 / clipsContent=true / border 70% / V3 Elevation Window`；确认 Window 本体无实体 Fill，Video Scene 贴满外壳。
- 结构：新增 Figma Page `02 Player Window`（`48:11`）与 `D2-01 / Player Window Shell Contract`（`50:2`）。
- Standard：`50:22` 真实绑定 `size/window/reference-width`、`size/window/reference-height`、`radius/window/player`、`border/glass` 与 `V3 / Elevation / Window`。
- Maximized：`50:34` 使用 `radius/window/maximized=0`，无 outer border、无 outer elevation，保留 clipping；明确 Maximized 不是 Fullscreen。
- Safe Area：新增 `spacing/window/safe-min=26`；`50:17` 四边 padding 真实绑定同一 Variable；它只表示 Floating Host minimum edge，不等于整个窗口统一 content padding。
- Geometry Token：新增 `radius/0`、`size/700`、`size/1320`、`spacing/window/safe-min`、`size/window/reference-width`、`size/window/reference-height`、`radius/window/maximized`。
- Material Token：新增 `alpha/70` 与 `alpha/window/border`，记录 Window Border 70% 的实现真值。
- Ownership：Window Surface 只拥有 radius / border / elevation / clipping / safe minimum；D2-02 拥有 Video Viewport，D2-03 拥有 Header，D3 拥有 OSC，D4 拥有 Inspector。
- 修复：首次生成时 `setBoundVariableForPaint()` 将 Paint opacity 重置为 1.0，导致占位柔光和 Window Border 过强；恢复 22 个 Fill/Stroke 透明度后第二轮截图通过。
- 验证：Standard、Maximized、Background Contrast 与完整 D2-01 Board 截图通过；D2-01 产品 Solid Paint `26/26` Semantic-bound、Unbound=`0`。
- D1 回归：Semantic Color `142/142`、Typography `43/43`、Geometry `55/55`、Effect `25/25`。
- 记录：`docs/records/D2-01_定义PlayerWindow外壳.md`。
- 下一任务：`D2-02 定义 Video Viewport`。

### 2026-08-09 — D1-06 建立 Motion / Opacity / Z-order；Stage D1 关闭

- 审计：三张核心框架可见节点 162 个、Prototype Reaction 0 个；唯一 Node opacity 例外为 Floating Header Close = 72%；现有 child order 已形成 `Video → Header/OSC → Inspector` 基本层级。
- 实现：新增 `V3 / Interaction Primitive`，共 26 个 Variable，覆盖 duration / easing / alpha / z。
- 实现：新增 `V3 / Interaction Semantic`，共 42 个 Semantic Variable：Motion 24、Opacity 7、Z-order 11；Collection Mode 为 `Standard / Reduce Motion`。
- Motion：OSC 160/120ms、Inspector 240/180ms、Popover 160/120ms、HUD 160/120ms、Dialog 240/180ms、Toast 200/160ms；进入 Ease Out、退出 Ease In；禁止默认 spring / bounce。
- Reduce Motion：12 / 12 个 Motion Duration Semantic 在 Reduce Motion Mode 下全部解析为 `0 ms`，不存在第二套组件动效。
- Opacity：建立 hidden 0%、dialog scrim 18%、disabled 38%、idle 72%、pressed 84%、hover/visible 100%；Close 72% 已真实绑定 `opacity/control/idle`。
- Z-order：建立 `Video 0 → Atmosphere 10 → Media 20 → Header 30 → OSC 40 → Inspector 50 → Popover 60 → HUD 70 → Toast 80 → Dialog Scrim 90 → Dialog 100`；Qt Quick/QML 后续使用同一 `z/*` 数值契约。
- Figma：在 `01 Foundations` 新增 `D1-06 / Motion Opacity Z-order`（Node `41:2`）。
- Prototype：创建 `41:155 / 41:170 / 41:185 / 41:200` 四个 Smoke Frame，并建立 3 段 `ON_CLICK + SMART_ANIMATE + EASE_OUT` Reaction；最终态同时保留 OSC / Inspector / HUD / Dialog。
- 修复：普通 `setPluginData` 在当前 Host Runtime 不支持，原子回滚后改用 `setSharedPluginData(openai.v3player, z-role, ...)`。
- 修复：Figma `OPACITY` Scope FLOAT 使用 0–100；初始 `0.72` 绑定后错误变成 `0.72%`，已统一修正 Primitive 为 0/18/38/72/84/100，Close 实际恢复 72%。
- 修复：Smoke Final 初始 HUD 被 Dialog 完全遮挡；仅调整测试稿光学位置，不改变 Z-order，最终四层均可辨认。
- 验证：Interaction Primitive 26、Semantic 42、Modes 2；Reduce Motion 12/12 = 0ms；Smoke Frame 4、Reaction 3、Mismatch 0；Close 72%；Z-role 产品节点 9。
- D1 最终回归：Semantic Color `142 / 142`、Typography `43 / 43`、Geometry `55 / 55`、Effect `25 / 25`；Main / Fullscreen / Playlist Inspector / Foundations / Smoke Final 截图通过。
- 边界：D1 只定义 transition duration/easing/opacity/z；OSC 自动隐藏 timer 生命周期留到 D3，不在 Foundations 建立第二套 timer 规则。
- 记录：`docs/records/D1-06_建立MotionOpacityZOrder.md`。
- 结论：Stage D1 关闭；下一任务为 `D2-01 定义 Player Window 外壳`。

### 2026-08-09 — D1-05 建立 Glass / Blur / Shadow

- 审计：从 Main Player / Fullscreen / Playlist Inspector 提取真实 Window Elevation、Floating Glass、Inspector 与 Atmosphere Blur，确认现有材质集中为少数稳定层级，而不是随机效果堆叠。
- 实现：新增 `V3 / Effect Primitive`，共 38 个 FLOAT Variable，覆盖 Blur、Shadow radius/y/spread 与框架当前真实 Material Alpha。
- 实现：新增 `V3 / Material Semantic`，共 47 个 Semantic Material Variable，覆盖 Glass Blur、Atmosphere Blur、Window/Floating/Control Shadow 及 Material Alpha。
- 实现：新增 16 个本地 Effect Style：2 个 Elevation、10 个 Glass、4 个 Atmosphere。
- 决策：HUD / Dialog 已建立独立语义 Effect Style，但暂时分别复用 Control / Inspector 的已验证效果强度，不在 D5 真实状态画面出现前虚构新的材质层级。
- 回刷：25 / 25 个当前真实 Effect 节点已接入正式 Effect Style；Assignment Issues = `0`；相关 Effect 节点未样式化残留 = `0`。
- 修复：Inspector 场景 Play Button 描边从异常 `100%` 统一回同职责 Main Play Button 的 `48%`；标准 Window Shadow 从 Main 的 `13%` / Inspector 的 `12%` 收敛为统一 `12%`，Fullscreen 保留 `11%` Immersive 层级。
- 边界：Playlist 普通 Row 继续保持无 Blur / 无 Shadow；Material Alpha 作为设计与实现真值记录，但不错误绑定整个 Frame opacity；状态级 Opacity 留给 D1-06。
- Figma：在 `01 Foundations` 新增 `D1-05 / Glass Blur Shadow`（Node `35:2`），包含 Material/Elevation Roles、Atmosphere Layer Blur、Material Alpha Map、Bright/Dark/Warm 背景压力测试、Product Material Map 与规则说明。
- 验证：Main / Fullscreen / Playlist Inspector 高分辨率截图回归通过；Bright / Dark / Warm 三种复杂背景下玻璃边界与文字可读性通过；D1-05 Foundations 文档板截图无裁切。
- 回归：Semantic Color 保持 `142 / 142`；Typography 保持 `43 / 43`；Geometry 保持 `55 / 55`。
- 记录：`docs/records/D1-05_建立GlassBlurShadow.md`。

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
