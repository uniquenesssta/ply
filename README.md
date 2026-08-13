# Modular Qt 6 + libmpv Player

Windows-first、跨平台预留的 Qt 6 + libmpv 桌面播放器。播放核心使用 libmpv，UI 使用 Qt 6 / Qt Quick/QML，核心代码采用 C++20，构建系统为 CMake + Ninja。

README 只维护**项目入口、当前状态、关键架构边界和简短变更记录**。Atomic Task 的目标、实现边界、验收矩阵与详细结果统一查看 `docs/plans/stages/`；README 不再重复完整实施过程。

## Current status

| Stage | 状态 | 结果 |
|---|---|---|
| R2 — 无 UI libmpv 播放核心 | Complete | 真实 load/play/pause/seek/stop、事件/属性/命令与 headless probe 主链完成 |
| R3 — 领域状态与 PlaybackSession | Complete | PlaybackSnapshot、Reducer、Generation、RequestTracker、Supersession、Session 生命周期完成 |
| R4 — libmpv OpenGL Render API | Complete | 视频进入 Qt Quick，Render 生命周期、DPI/visibility/shutdown 与 1080p/4K 基线完成 |
| R5 — UI 设计系统 | In Progress | **R5-01 / R5-02 / R5-03 / R5-04 / R5-05 Complete**；R5-06 Button controls 已完成候选实现，等待 Windows 47-test 与 `Player.exe` 启动验证 |

R0/R1 属于现有项目基线，R2–R14 快速任务书不重新定义其历史状态。R4 后置 `PlaybackSession` 职责边界优化属于独立可选任务，仅在明确调用时执行，不阻断 R5。

当前保留的已知非阻塞事项：R2-01 的仓库根 `player.log` 落盘缺口仍未关闭。

## Technical baseline

- Qt **6.8.3** / C++20 / CMake / Ninja。
- Windows 首发工具链：MSVC 2022 x64。
- 固定 libmpv **0.41.0** sibling package。
- Qt Quick 图形后端固定 OpenGL；视频使用 libmpv OpenGL Render API + `QQuickFramebufferObject`。
- QML 不直接调用 libmpv。
- `PlaybackSession` 是播放状态与媒体代际的唯一权威 owner。
- libmpv 字符串命令、属性、事件和 C API 类型限制在 `src/playback/infrastructure/mpv/`。
- Renderer 只拥有 Render/OpenGL 资源，不拥有播放业务状态。
- Playlist、Persistence、Platform、Presentation 各自保持独立职责边界。

## Core architecture

```text
QML / Presentation
        ↓ intent / read-only state
Application / PlaybackSession
        ↓ PlaybackCommand / PlaybackEvent
Playback Domain
        ↓
Infrastructure / libmpv
        ↓
libmpv

libmpv redraw callback
        ↓
Render update bridge
        ↓
MpvVideoItem / MpvVideoRenderer
        ↓
Qt Quick Scene Graph
```

核心目录职责：

```text
src/app/                         应用启动、composition、顶层生命周期
src/foundation/                  通用基础设施与稳定值类型
src/playback/domain/             后端无关的播放命令、事件、状态与规则
src/playback/application/        PlaybackSession、请求生命周期与应用编排
src/playback/infrastructure/mpv/ libmpv client/event/property/command/render 适配
src/presentation/                QML 与 presentation 层
src/platform/                    平台能力
src/persistence/                 持久化边界
```

## Taskbooks

执行开发任务时以以下文档为准：

- [完整模块化开发任务书](docs/plans/Qt6-libmpv播放器-完整模块化开发任务书.md)
- [R2–R14 Stage 索引](docs/plans/stages/00_INDEX.md)
- [成熟播放器行为补强与验收矩阵](docs/plans/stages/成熟播放器行为补强与验收矩阵.md)
- [R2 — 无 UI libmpv 播放核心](docs/plans/stages/R2_无%20UI%20libmpv%20播放核心.md)
- [R3 — 领域状态与 PlaybackSession](docs/plans/stages/R3_领域状态与%20PlaybackSession.md)
- [R4 — libmpv OpenGL Render API](docs/plans/stages/R4_libmpv%20OpenGL%20Render%20API.md)
- [R5 — UI 设计系统](docs/plans/stages/R5_UI%20设计系统.md)
- [R4 后置 — PlaybackSession 职责边界优化](docs/plans/stages/R4后置_PlaybackSession职责边界优化.md)

R6–R14 的任务书继续由 [Stage 索引](docs/plans/stages/00_INDEX.md) 统一导航。

## Build and test

Windows 开发构建使用项目脚本：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

Release 构建：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\configure.ps1 -Preset windows-msvc-release
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Preset windows-msvc-release
```

不得把未执行、被阻塞或失败的验证描述为通过；具体 Stage 的验收数字记录在对应 Stage 文档中。

## Change log

### 2026-08-13 — R5-06 Button controls candidate

- 重新读取第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 的 D3-04 Transport 与 D3-06 Utility 设计上下文：Secondary/Utility 固定 **32px hit / 22px visual / R16**，Primary Playback 固定 **40px / 24px visual / R20**；交互权重使用 rest 72%、hover 100%、pressed 84%、disabled 38%，focus ring 82%，Primary glass alpha 使用 48/52/42%，hover/focus 使用既有 120ms Ease Out、pressed 0ms，Reduce Motion 继续降为 0ms。
- 新增 `controls/buttons/ButtonBase.qml`、`IconButton.qml`、`TextButton.qml`、`ToggleButton.qml`。内部 `ButtonBase` 统一拥有 pointer hover/press、Space/Enter/Return 键盘激活、focus、disabled gate、toggle/checked 与 tooltip-request 状态；三个公开控件只消费该输入契约并负责各自视觉语义，不复制输入状态机、不持有播放业务状态。
- `IconButton` 支持 Secondary / Primary 两级 emphasis，并暴露 `opticalOffsetX` 给后续 Transport consumer 应用已冻结的 Prev/Play/Next 光学修正；`ToggleButton` 复用 D3-06 open-selection 语义，selection fill 66% 与 selection border 28% 分层实现；`TextButton` 采用低权重 semantic text 状态，不凭空新增品牌色实心按钮变体。
- R5-06 首次真实消费暴露两个基础 token 缺口：补充 `SizePrimitives.size24 → LayoutTokens.playbackIcon` 与 `OpacityPrimitives.focus → OpacityTokens.focusRing`，分别承载 Figma 的 24px Primary glyph 与 82% focus ring；未改写既有 72/84/38/100% 或 Radius/Material/Motion 真值。
- `Player.Presentation.Controls` 现在显式依赖 Theme + Primitives；Feature import 规则保持 R5-01 冻结状态，Feature 仍只直接消费 Theme/Controls，不允许重新出现 Feature→Primitives 深依赖。`player_qml_lint` 已纳入 controls lint。
- Tooltip 本轮提供 `toolTipText + toolTipVisible` 请求/状态契约，没有伪造 Figma 尚未冻结的 Tooltip Surface；真正 overlay 材质与宿主仍由后续 Surface/Feedback 层负责。Primary 控件当前消费玻璃填充/边框/交互 alpha，不在通用 Button 内复制依赖背景捕获的 backdrop-blur 实现。
- 收口静态审查发现并修正 Primary `IconButton` 的 Focus 视觉偏差：`activeFocus` 不再触发 hover glass alpha，键盘 Focus 保持 Figma 冻结的 **48% rest glass + 82% focus ring**；同时 `button_controls` 新增真实 pointer hover/press、Primary hover 52% 与 Focus 48% 的直接断言，避免只测 click 而遗漏视觉状态契约。
- 新增独立 `button_controls` CTest，覆盖公开类型加载与 32/22、40/24、R16/R20、72/48/82 等设计契约，真实 pointer hover/press/click、Space 键 toggle、disabled no-op、focus tooltip-request 以及 Reduce Motion 传播；全量 Windows CTest 预计由 46 增至 **47**。本轮修改了 QML module/CMake，因此关闭前必须重新 configure/build/qmllint/47-test 并完成 `Player.exe` smoke。当前容器没有锁定 Windows Qt/libmpv sibling 环境，未把这些未执行验证写成 PASS。**R5-06 当前为 Candidate；R5-07 未开始。**

### 2026-08-13 — R5-05 Typography primitives Complete

- 重新读取第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 本地 Text Styles；现有 `TypographyTokens` 的 Inter / Noto Sans SC / Geist Mono、字号和字重已经与最终设计一致，因此 R5-05 不复制或改写字体真值。
- 新增 `primitives/text/TitleText.qml`、`BodyText.qml`、`CaptionText.qml`、`TimecodeText.qml`。四类 primitive 分别消费既有 semantic typography/color token，并覆盖 Inspector/Media 标题、Supporting/Control 正文、Meta/Technical/Strong 说明和 M/S/XS Timecode 变体；标题和说明默认单行右侧 ellipsis，Timecode 固定单行并使用 Geist Mono 语义字体。
- 首轮 Windows configure/build 均 PASS；`typography_primitives` 自身 PASS，但全量回归为 **45/46 PASS**。唯一失败是既有 `qml_module_boundaries`：候选把 `PlayerChrome` 直接改为 `import Player.Presentation.Primitives`，违反 R5-01 已冻结的 Feature 仅直接消费 Theme/Controls 的边界。
- 修复没有放宽 R5-01 门禁：撤回 `PlayerChrome → Primitives` 直接依赖并恢复既有 Theme/TypographyTokens 消费，同时删除错误新增的“所有产品 QML 必须直接使用 typography primitive”扫描；`typography_primitives` 继续独立验证 semantic style、长标题 ellipsis 与 Geist Mono Timecode 契约，供后续 Controls/Surfaces 组合使用。
- 修复后 Windows Debug build PASS，`scripts/test.ps1` 的 QML lint 门禁完成且无 warning/error；全量 **46/46 CTest PASS，0 failed，49.49 s**，其中 `qml_module_boundaries` 与 `typography_primitives` 均 PASS；`Player.exe` 实际启动 smoke 无 QML/运行时错误输出。没有新增生产依赖，也没有修改 PlaybackSession、libmpv、Renderer、Render 生命周期或播放接口。字体文件仍未随应用捆绑，沿用 R5-02 已记录的系统 fallback 限制。**R5-05 正式 Complete。**

### 2026-08-13 — R5-04 Complete

- 从第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 直接导出并纳入当前已存在的主操作 glyph：`previous/play/next/volume/subtitles/playlist/fullscreen/close/search`；SVG path 数据未手工重绘。Figma 当前没有独立 Pause/Mute/Exit-Fullscreen 等最终 glyph，因此本任务不伪造缺失资产，未知 `iconId` 由 `Icon.diagnostic` 明确暴露。
- 新增 `Player.Presentation.Primitives/Icon.qml` 与内部 `IconCatalog.qml`：Catalog 只拥有 `icon id → qrc resource / 默认色角色`，Icon 只负责加载、intrinsic size、semantic color 与状态诊断。默认 primary/secondary 颜色映射既有 `ColorTokens.iconPrimary/iconSecondary`。
- 首轮 Windows configure 在 `daf6f17` 暴露 Qt 6.8 硬约束：QML 类型不能同时标记 `QT_QML_SINGLETON_TYPE` 与 `QT_QML_INTERNAL_TYPE`。后续 `rules.ninja` 和 development marker 缺失均是 configure 未完成的连锁结果，不作为独立故障。修复后 `IconCatalog` 改为仅 internal 的普通 QML type，由 `Icon` 内部实例化；公共 `Icon` API、iconId、SVG 资源和着色语义均不变，也没有把 Catalog 扩大为公共 QML API。
- SVG 统一放入 `src/presentation/qml/assets/icons/`，由 `player_presentation_primitives` QML module 以固定 resource alias 打包；Icon semantic tint 使用锁定 Qt **6.8.3** 已包含的 `QtQuick.Effects.MultiEffect`。没有新增外部包或第三方许可证，但 primitives 模块新增对现有 `QtQuick.Effects` QML runtime module 的依赖，Windows 部署需包含既有 `effectsplugin.dll`；用户当前 Qt 安装已确认该模块存在。相比引入 QtSvg/C++ image provider 或复制多套预着色 SVG，该方案保持单一 Figma SVG 真值且只对 22/24 px 图标增加轻量 colorization pass。
- `player_qml_lint` 包含 primitives lint；`icon_pipeline` CTest 验证九个当前 Figma glyph 全部从 qrc 加载、Figma intrinsic 22/24 尺寸、primary/secondary semantic tint、未知 id 诊断、资产清单和产品 QML 不绕过 Icon primitive。
- 锁定 Windows 环境最终实测：全量 **45/45 CTest PASS，0 failed，49.83 s**；`qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline` 均 PASS；`Player.exe` 实际启动无报错。未修改 PlaybackSession、libmpv、Renderer、Render 生命周期或播放接口。**R5-04 正式 Complete。**

### 2026-08-13 — R5-03 Complete

- Radius、Blur/Material、Elevation、Motion、Opacity、Z-order primitive/semantic token 已建立；Reduce Motion 统一将 transition duration 降为 0 并切换 Linear easing，OSC inactivity hide-delay 继续保持 **2200 ms**。
- 新增 `theme_effect_tokens` 合同测试与产品 QML raw radius/z/opacity/duration 回流门禁。
- 锁定 Windows 环境实测已确认：configure、Debug build、无 warning `player_qml_lint`、**44/44 CTest PASS（49.52 s）** 与 `Player.exe` 启动 smoke 均通过。当前仍是播放器骨架界面；Panel/Popover/HUD 等实际 Surface 消费属于 R5-08。**R5-03 正式 Complete。**

### 2026-08-13 — R5-02 Complete

- 按第三版 Airy Glass 最终 Figma 变量/文本样式建立 `ColorPrimitives/ColorTokens`、`TypographyPrimitives/TypographyTokens`、`SpacingPrimitives/SpacingTokens`；为清除现有核心 QML 的裸尺寸，补充职责独立的 `SizePrimitives/LayoutTokens`。
- `Theme.qml` 收敛为兼容 facade；`MainWindow`、`VideoSurface`、`PlayerChrome` 已移除散落的颜色、字体字号、间距和视觉尺寸字面量；窗口默认/最小尺寸行为保持 1280×720 / 960×540。
- 新增 `theme_tokens` 合同测试和产品 QML 硬编码回流门禁，测试总数由 42 增至 43。
- 锁定 Windows 环境实测已确认：configure、Debug build、无 warning `player_qml_lint`、**43/43 CTest PASS** 与 `Player.exe` 启动 smoke 均通过。**R5-02 正式 Complete。**

### 2026-08-13 — R5-01 Complete

- `Theme/Primitives/Controls/Surfaces` 四个公开 QML URI、公开 import 边界和最小模块加载链已稳定。
- 显式 tooling typeinfo 最终候选在锁定 Windows 环境通过 configure、Debug build、development runtime deployment、**无 warning `player_qml_lint`** 和 **42/42 CTest PASS（48.23 s）**。
- `MpvVideoItem was not found`、anchors/objectName 派生 warning、export/meta-object revision warning 均已关闭；运行时继续使用已验证的 `qmlRegisterType<MpvVideoItem>()`。
- `Player.exe` 实际启动 smoke 已确认正常：窗口正常创建，无 QML root/type registration 启动错误。**R5-01 正式 Complete。**

### 2026-08-13 — R5-01 validation fixes

- Windows Qt 6.8.3 / MSVC 已确认 configure 与 build 通过；早期 `qmltyperegistrar` Generate 阻断已消失。
- `qml_module_boundaries` 的 `QtQuick` import path 已通过测试脚本显式注入 `<Qt>/qml` 修复，并在测试后恢复原环境。
- 全量回归捕获的 R4-08 late-update 竞态已通过 consumer-side shutdown gate 修复；公共接口和播放状态所有权不变。
- 修复后 Windows 全量回归已达到 **42/42 PASS**；后续仅继续治理 qmllint 的 `MpvVideoItem` tooling metadata。

### 2026-08-12 — R5-01

- 建立 `Theme/Primitives/Controls/Surfaces` 四个公开 QML URI，现有 Theme 使用点改为显式模块 import。
- `Player.Presentation` 保留 App 公共入口，其余现有 Shell/Screen/Feature 类型收为 internal；播放接口、依赖版本和视觉 token 不变。
- Windows configure 暴露 Qt 6.8.3 TARGET-based QML dependency 的 deferred `qmltyperegistrar` 生成失败；已改为 URI dependency 并保留显式 backing-target 链接。
- 新增 QML public-module 最小加载测试；详细记录见 R5 Stage 文档。

### 2026-08-11 — README documentation policy

- README 收敛为项目入口、当前状态、架构边界和简短变更记录；不再复制 Atomic Task 的完整实施过程。
- 详细任务、实现边界、测试矩阵与阶段验收记录由 `docs/plans/stages/` 承担；历史 README 内容仍保留在 Git 历史中。
- 文档职责调整，不改变源码、公共接口、配置、依赖或运行行为。

### 2026-08-11 — R4-09 / Stage R4

- 修复默认 libmpv Render 模式下错误的 update gate，解决 1080p windowed 稳态停帧。
- Windows Debug：**41/41 CTest PASS，0 failed，58.78 s**。
- Release probe：1080p/4K 的 windowed/fullscreen 均约 **300 frames / 10 s**，新增 frame/decoder/delayed drop 均为 **0**。
- **R4-01~R4-09 Complete；Stage R4 Complete。** 详细数据见 R4 Stage 文档。

### Stage R3

- **R3-01~R3-12 Complete；Stage R3 Complete。**
- 建立后端无关播放领域状态、PlaybackSession 单一真值、MediaGeneration stale gate、RequestTracker、cleanup matrix 与 supersession/cancellation。

### Stage R2

- **R2-01~R2-11 完成 Stage 主链；Stage R2 Complete。**
- 建立固定 libmpv 运行链、RAII client、初始化、命令、属性、事件、错误映射与 headless playback probe。
- R2-01 的根 `player.log` 落盘缺口继续作为已知非阻塞诊断事项保留。
