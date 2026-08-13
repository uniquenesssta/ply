# Qt6 + libmpv 播放器阶段实施任务书

> 版本定位：快速框架实施版  
> 范围：R2–R14  
> 原则：先把正确的模块框架和真实主链跑起来，再集中修复、补边界和做强验证。  
> 不包含：R0、R1；现有 R0/R1 状态不在本包中重新定义。

## 统一执行策略

本任务书不要求每个 Atomic Task 都先写成“工程合同”。普通任务只要明确目标、模块、主链、实现重点和基本验证即可直接开工。

整个项目按四个节奏推进：

1. **骨架优先**：先建立真正会被使用的目录、CMake、接口、对象所有权和最小实现，不创建大批空文件。
2. **主链优先**：优先让真实链路工作，例如 `libmpv → PlaybackSession → Render → ViewModel → QML`，先看到结果。
3. **边做边修**：编译错误、接口不顺、职责耦合、明显竞态、真实运行问题出现后立即在影响边界修复；普通边缘问题可记录到阶段修复清单，不要求每个小任务都阻断后续。
4. **集中硬化**：R12 专门负责压力、故障、性能、泄漏、长时间运行和最终架构审查；R13 再做真正发布门禁。

只有以下问题默认视为“必须先停下来解决”的硬阻断：

- 项目无法 configure/build；
- 主链完全不可运行；
- 明确的状态双重拥有、循环依赖或错误模块边界；
- crash、deadlock、use-after-free、线程越界等高风险生命周期问题；
- 数据损坏、不可逆迁移错误；
- Render/mpv 销毁顺序错误；
- 发布阶段安装包无法在干净机器启动。

普通 UI 细节、非主路径边缘场景、尚未覆盖的手工矩阵不需要阻止每个 Atomic Task 继续，但必须在阶段修复清单或 R12/R13 前关闭。

## 软件可用性里程碑

| 里程碑 | 达到的实际形态 |
|---|---|
| R2 | 无 UI 的 libmpv 播放核心可真实加载/播放/Seek/关闭 |
| R3 | 播放状态、线程、命令/事件稳定，形成真正“播放器内核” |
| R4 | 视频真正进入 Qt Quick，开始像一个播放器 |
| R6 | 主窗口、播放/暂停、Timeline、音量、全屏可用，形成基础桌面播放器 |
| R8 | 文件/URL/播放列表/音轨/字幕/章节完整，达到主流播放器核心功能形态 |
| R10 | 历史/设置/快捷键/媒体键/单实例/文件关联齐全，Windows 日常使用基本完整 |
| R11 | 高级能力增强 |
| R12 | 稳定性和性能收敛 |
| R13 | 可发给别人安装使用的 Windows 正式桌面软件 |
| R14 | macOS/Linux 平台适配 |

## 模块化总原则

- 一个模块只拥有一个清晰职责；新能力有独立状态、IO、生命周期、错误或测试边界时再拆文件/目录。
- 不为了“文件多”而拆；也不为了“改得少”把不同职责继续塞进同一个文件。
- QML 不直接调用 libmpv；PlaybackSession 是播放真值；Playlist 只拥有队列；DatabaseWorker 只拥有数据库连接；Renderer 只拥有 Render/OpenGL 资源。
- 所有源码/配置/行为变化继续同步记录根 `README.md`。
- R0-06 已按用户要求跳过，因此依赖合法可分发媒体 fixture 的验证，如果只有本地合法样本，必须写成“本地验证”，不能伪装成完整 fixture 体系已经闭环。

# Stage R5：UI 设计系统

## 1. 本阶段最终要得到什么

尽快得到一套够 R6-R10 使用的现代播放器设计系统，不要求在 R5 就把所有未来控件一次性设计完。

## 2. 本阶段怎么快速推进

先把 Token + Icon + Button + Slider + Surface 搭起来，让 R6 能立即开主界面；后续在真实页面中发现缺失的 primitive/control，再回到对应目录补，不建立“大而全组件库”。

## 3. 目标模块结构

```text
src/presentation/qml/
├─ theme/
├─ primitives/
├─ controls/
├─ surfaces/
├─ feedback/
└─ assets/icons/
```

## 4. 阶段主链

```text
Theme/Tokens
  ↓
Primitives
  ↓
Controls
  ↓
Surfaces / Feedback
  ↓
Feature
  ↓
Screen
```

## 5. 关键状态/资源归属

- 视觉 token：Theme。
- 控件临时交互：控件自身。
- 播放业务状态：绝不进入 Design System。

## 6. Atomic Tasks

### R5-01 QML module 和 import 边界

**目的**  
先把 theme/primitives/controls/surfaces 的模块入口搭出来。

**主要模块 / 文件**  
QML module CMake/qmldir

**主要链路**  
`Feature → public controls/theme`

**实施重点**
建立公开 URI 和最小 import 规则，避免深路径引用。

**基本验证**
qmllint / 最小加载。

**完成判断**  
后续 UI 有稳定公共组件入口。

### R5-02 Color/Typography/Spacing tokens

**目的**  
集中颜色、字体和间距语义。

**主要模块 / 文件**  
`theme/*Tokens.qml`

**主要链路**  
`Theme → semantic tokens → controls`

**实施重点**
先覆盖播放器当前要用的 token，不追求一次列完。

**基本验证**
核心控件不再散落常用硬编码。

**完成判断**  
R6 可以直接复用。

### R5-03 Motion/Radius/Elevation tokens

**目的**  
统一动画时长、圆角、阴影层级。

**主要模块 / 文件**  
`theme/MotionTokens.qml` 等

**主要链路**  
`tokens → controls/surfaces`

**实施重点**
先做一套主视觉；保留 reduce motion 开关。

**基本验证**
同类组件视觉一致。

**完成判断**  
动效/圆角不各写各的。

### R5-04 图标管线

**目的**  
把 SVG 资源、命名、着色和打包路径固定。

**主要模块 / 文件**  
`assets/icons/`、`Icon.qml`

**主要链路**  
`icon id → resource → semantic color`

**实施重点**
先覆盖播放器主操作图标；缺资源直接可诊断。

**基本验证**
开发构建可加载全部当前图标。

**完成判断**  
R6 不再临时画图标。

### R5-05 Typography primitives

**目的**  
建立标题/正文/说明/Timecode 基础文本。

**主要模块 / 文件**  
`primitives/text/`

**主要链路**  
`semantic text style → feature`

**实施重点**
先解决字号、字重、ellipsis、等宽数字。

**基本验证**
长标题/时间码显示。

**完成判断**  
文字层级统一。

### R5-06 Button controls

**目的**  
建立 IconButton/TextButton/ToggleButton。

**主要模块 / 文件**  
`controls/buttons/`

**主要链路**  
`input → control signal → feature action`

**实施重点**
hover/press/disabled/focus/tooltip 主状态先齐。

**基本验证**
Qt Quick Test 基本交互。

**完成判断**  
R6 不重复造按钮。

### R5-07 Slider controls

**目的**  
建立业务无关 Slider。

**主要模块 / 文件**  
`controls/sliders/`

**主要链路**  
`pointer/key/wheel → normalized value`

**实施重点**
不直接 Seek/Volume；只发 value/interaction 信号。

**基本验证**
鼠标、键盘、滚轮。

**完成判断**  
Timeline/Volume 可以二次封装。

### R5-08 Surface controls

**目的**  
建立 Panel/Drawer/Popover/HUD 的视觉底座。

**主要模块 / 文件**  
`surfaces/`

**主要链路**  
`feature content → surface → overlay stack`

**实施重点**
先统一背景、圆角、shadow、z 层级。

**基本验证**
并排/叠层 smoke。

**完成判断**  
R6/R7 弹层外观一致。

### R5-09 Feedback controls

**目的**  
区分 Toast/Error/Loading/Empty。

**主要模块 / 文件**  
`feedback/`

**主要链路**  
`feedback model → presentation`

**实施重点**
只做展示语义，不判断业务原因。

**基本验证**
四类组件加载。

**完成判断**  
后续错误/空态不会混用。

### R5-10 可访问性基线

**目的**  
在基础控件上补 accessible name、focus visible 和键盘可达。

**主要模块 / 文件**  
controls + accessibility tests

**主要链路**  
`keyboard/accessibility → control`

**实施重点**
先覆盖所有主操作组件；高级 screen reader 验证后续补。

**基本验证**
键盘遍历和 Accessible.name。

**完成判断**  
基础可访问性不欠账。

## 7. 框架打通后优先修复

- 硬编码颜色/尺寸回流。
- Button/Slider 在不同 feature 被重复实现。
- Popover/Drawer z 层级冲突。
- 键盘 focus 不明显。

## 8. 本阶段最小可运行里程碑

做一个最小组件展示页或测试场景，可看到按钮、Slider、Panel、Popover、HUD、Toast，并能用键盘操作。

## 9. 阶段关闭条件

R6 所需基础组件齐全；qmllint/加载正常；不包含任何 libmpv/playback 逻辑。未来设置页专用控件可后补。

## 10. 本阶段暂不要求

不追求一次性覆盖所有高级设置控件、PiP、mini player 或未来插件 UI。

## 11. 后续扩展位置

后续缺少的控件按真实 Feature 需求补入对应层；若只是某 Feature 专用，不强行提升为全局 Control。

## 12. R5-01 实施记录（2026-08-12）

状态：**R5-01 Complete（2026-08-13）。**

### 已实施

- 固定四个公开 QML URI：`Player.Presentation.Theme`、`Player.Presentation.Primitives`、`Player.Presentation.Controls`、`Player.Presentation.Surfaces`。
- `Theme.qml` 从应用大模块拆入独立 Theme backing module；文件物理路径和现有 token 值不变。
- `MainWindow`、`PlayerScreen`、`VideoSurface`、`PlayerChrome` 收为 `Player.Presentation` internal type；`App` 继续作为应用模块公开入口，`QmlBootstrap::loadFromModule("Player.Presentation", "App")` 不变。
- Theme 真实使用点改为显式 `import Player.Presentation.Theme`，不再依赖应用模块内隐式同域可见性；公共设计系统不使用相对/深路径 import。
- Design System 模块未引入 Playback/libmpv 依赖；R5-01 不提前实现 R5-02 的 Airy Glass token 改造。
- 统一 QML tooling 输出到 `${CMAKE_BINARY_DIR}/qml`；模块 `qmldir` 继续由 Qt CMake API 生成，不维护重复手写清单。
- 新增 `qml_module_boundaries` 最小加载测试：同时导入四个公开 URI，并实例化读取 Theme singleton；测试允许 `QQmlComponent` 按 Qt 语义异步完成，但 Error/timeout 仍硬失败并输出 status/progress/QQmlError。
- `MpvVideoItem` 的运行时 QML 暴露恢复由 `presentation/qml/types/presentation_type_registration.*` 独立负责，继续使用已验证的 `qmlRegisterType<MpvVideoItem>("Player.Presentation", 1, 0, "MpvVideoItem")`；Playback render 类不承担 Presentation URI 语义。
- `MpvVideoItem` 的静态 tooling 元数据由 `presentation/qml/types/player_presentation.qmltypes` 独立负责。`Player.Presentation` 使用 `NO_GENERATE_QMLTYPES + TYPEINFO player_app.qmltypes`，并通过 `qt_query_qml_module(TYPEINFO ...)` 取得 Qt 实际模块输出路径后在 configure 阶段复制该 typeinfo；Qt 继续生成 `qmldir`、QML 资源和 lint target。
- `Player.Presentation` 新增 URI dependency `QtQuick`，与 `MpvVideoItem` 的 `QQuickItem` 基类 tooling 契约一致；Theme 仍保持 URI dependency + backing-target 链接。

### 影响与兼容性

- C++ 公共接口、PlaybackSession、libmpv、依赖版本、配置和 `Player.Presentation/App` 启动入口不变。
- QML 类型名和 URI 保持 `Player.Presentation 1.0 / MpvVideoItem`；运行时注册语义恢复到此前已验证路径，新增的 `.qmltypes` 仅供 qmllint/qmlls/Qt tooling 使用，不承担运行时对象所有权。
- `MpvVideoItem` 本体、Renderer、Render context、Playback 状态和所有权均未修改；R4-04 为 `MpvVideoItem` 建立的手工 moc 构建链保持不变；未引入新的生产依赖。
- QML 内部资源归属变化：Theme 必须经公开模块导入；现有 Shell/Screen/Feature 类型不再作为公共 QML API 暴露。
- Windows 测试脚本只新增 Qt 安装 `qml` 路径的进程级 `QML_IMPORT_PATH`，执行结束后恢复原环境变量；不改变正式应用运行配置。
- R5-01 全量回归暴露的 R4-08 late-update 竞态采用局部修复：delivery gate 在 queued signal 消费时再次检查既有 shutdown coordinator，不引入第二 shutdown 状态、不改变 Render/mpv 所有权或公共接口。

### Windows 验收事实

- Qt 6.8.3 / MSVC 19.44 / Ninja 1.12.1 环境已确认 configure **PASS**：`Configuring done`、`Generating done`，早期 `qmltyperegistrar` Generate 阻断已消失。
- 随后 Debug build **PASS**，`qml_module_boundary_tests.exe` 成功链接，development runtime marker 与 Qt runtime deployment 均成功。
- 第一轮 42-test 回归：**41/42 PASS**；唯一失败 `qml_module_boundaries` 当时没有诊断，因为 `QQmlComponent` 尚在 `Loading` 即被同步断言。测试已修正为等待 `Ready/Error`。
- 第二轮 42-test 回归：**40/42 PASS**。`qml_module_boundaries` 给出真实根因 `module "QtQuick" is not installed`，后确认 QtQuick 文件实际存在，故障是测试进程缺少 Qt QML import path；`scripts/test.ps1` 已修复为向 CTest 进程显式提供 `<Qt>/qml`。
- 同轮回归 `mpv_render_update_bridge::shutdownCoordinatorSuppressesLateRequests` 出现 `deliveredCount=6 / countAfterShutdown=5`；delivery gate 已增加 shutdown-time consumer check，确保 late queued request no-op。
- 第三轮本机复测已确认上述两个硬失败关闭：**42/42 CTest PASS，0 failed**；`qml_module_boundaries` 与 `mpv_render_update_bridge` 均 PASS。
- 同一轮 build 仍有 6 条 `VideoSurface.qml` qmllint warning，根 warning 为 `MpvVideoItem was not found`。用户提供的文件清单已确认 QtQuick 模块、`MpvVideoItem` 源文件及既有注册文件均实际存在，因此该问题定性为静态 QML type metadata 缺口，而非缺文件。
- 第四轮（2026-08-13）在自动 `QML_FOREIGN` type registration 候选上重新 configure **PASS**，但 Debug build 在 `Automatic QML type registration for target player_app` 阶段硬失败：`qmltyperegistrar` 读取 `qt6player_app_debug_metatypes.json` 返回 `Failed to parse JSON: 5 illegal value`。因此该候选没有进入 qmllint/CTest，随后 `test.ps1` 的 development runtime marker 缺失只是 build 未完成的连锁结果，不作为独立故障处理。
- 第五轮显式 tooling typeinfo 候选验证：configure **PASS**、Debug build **PASS**、development runtime deployment **PASS**；原 `MpvVideoItem was not found` 与其 anchors/objectName 派生 warning 均消失，全量 **42/42 CTest PASS**。
- 修正 `exportMetaObjectRevisions` 的 1.0 编码后再次验证：configure **PASS**、Debug build **PASS**、`player_qml_lint` **无 warning**、全量 **42/42 CTest PASS，0 failed，48.23 s**；`qml_module_boundaries` 与 Render shutdown 相关回归继续 PASS。
- `Player.exe` 实际启动 smoke 已由用户确认 **PASS**：窗口正常启动，无 QML root/type registration 启动错误。

### qmllint metadata 治理

- 未采用 `QT_QML_SKIP_QMLLINT`、warning suppression、降低门禁或伪造占位 QML 类型。
- 手写 `qmlRegisterType<MpvVideoItem>(...)` 只在运行时执行，qmllint 不执行应用 bootstrap，因此本身不能提供静态类型元数据；这正是原始 `MpvVideoItem was not found` 及 anchors/objectName 派生 warning 的来源。
- 首次治理尝试使用 Qt 6.8 `QML_FOREIGN` descriptor + `qt_add_qml_module()` 自动 `.qmltypes` / C++ 注册，但当前 executable-backed `player_app` 的 metatypes JSON 生成链在锁定 Windows 环境产生不可解析输入并阻断 build，因此该方案已撤销，不把失败路径保留为兼容层。
- 最终治理采用 Qt 6.8 支持的 fallback：运行时继续由明确的 Presentation registration module 注册；tooling 使用项目维护的 `.qmltypes`，并由 `TYPEINFO` 写入 Qt 生成的 `qmldir`。两份信息职责不同：前者是运行时行为，后者是静态工具契约；名称/URI 保持一致。
- `.qmltypes` 的 `Player.Presentation/MpvVideoItem 1.0` 使用 `exportMetaObjectRevisions: [256]`，与 Qt `QTypeRevision` 1.0 编码一致；最终 qmllint 已无 warning。

### R5-01 关闭结论

- 后续 UI 已具有稳定公开 QML module 入口和明确 import 边界。
- qmllint、最小模块加载、全量 42-test 回归和实际应用启动均通过。
- R5-01 不包含 R5-02 token 实施，也未将 playback/libmpv 逻辑引入 Design System。
- **R5-01 正式 Complete。R5-02 尚未开始，等待单独任务指令。**

### 早期 configure 阻断与修复记录

- 2026-08-13 本机 Qt 6.8.3 / MSVC configure 两次在 Generate 阶段失败，错误均为 `$<TARGET_FILE:::qmltyperegistrar>` / `No target "::qmltyperegistrar"`；因此当时 build 的 `rules.ninja` 缺失与 test 的 development runtime marker 缺失均属于 configure 未完成后的连锁结果，不作为独立故障处理。
- 第一轮曾把问题误判为三个空 QML 模块的 typeinfo 生成，并对 `Primitives/Controls/Surfaces` 添加 `NO_GENERATE_QMLTYPES`；第二次本机复测证明该假设无效，三个参数已全部撤销，未保留无效绕过。
- 对照 Qt 6.8.3 `Qt6QmlMacros.cmake` 后确认真正触发点是 executable QML module `player_app` 使用 `DEPENDENCIES TARGET player_presentation_theme`：Qt 会为 TARGET-based dependency 在 `PROJECT_SOURCE_DIR` deferred finalizer 中合并 build-tree `qt.conf`，该路径依赖 `QT_CMAKE_EXPORT_NAMESPACE`；本项目 Qt package 在 `cmake/` 子目录作用域加载，defer 回项目根后该内部变量不可用，最终把工具目标展开成 `::qmltyperegistrar`。
- 修复改用 Qt 支持的 URI 依赖 `DEPENDENCIES Player.Presentation.Theme`，保留现有 `target_link_libraries(player_app PRIVATE player_presentation_theme)` 作为真实链接关系；不移动 `find_package(Qt6)`、不改变项目 CMake 分层、不引入 Qt 内部变量补丁。

## 13. R5-02 实施记录（2026-08-13）

状态：**R5-02 Complete（2026-08-13）。**

### 设计来源与实施边界

- 实现值来自第三版 Airy Glass Figma 文件 `KIOxfwTvQJlcVLinkeJAxY` 的最终本地 Variables / Text Styles，而不是根据旧 Theme 或截图猜值。
- 只推进 R5-02 的 Color / Typography / Spacing 语义；为满足根任务书“核心页面无散落硬编码颜色和尺寸”，额外建立 `SizePrimitives/LayoutTokens` 作为纯几何语义边界；R5-02 本身不实现 Motion / Radius / Elevation 或 Surface 材质组件。
- 未新增生产依赖；未修改 PlaybackSession、libmpv、Render context、Renderer、线程/生命周期、播放接口或数据结构。

### 已实施

- Theme 模块按职责拆成：`ColorPrimitives → ColorTokens`、`TypographyPrimitives → TypographyTokens`、`SpacingPrimitives → SpacingTokens`、`SizePrimitives → LayoutTokens`。
- `Theme.qml` 保留为 R5-01 兼容 facade，仅把旧 `windowBackground/videoBackground/chromeBackground/primaryText/secondaryText` 映射到新的 semantic color token；后续新 UI 不再往 `Theme.qml` 聚合新职责。
- 颜色使用第三版最终值与语义映射，包括 `surface/canvas #F7F7FC`、`surface/video #EDEEF7`、`text/primary #1A1720`、`accent/primary #7B2CFF`，并覆盖当前 R6 将需要的 text/icon/border/accent/selection/focus/feedback/control 基础语义。
- Typography 映射最终 Figma 字体层级：Inter 用于 Latin/UI 技术标签，Noto Sans SC 用于中文媒体/控件文本，Geist Mono 用于 timecode；当前实现只声明字体契约，不捆绑字体文件、不新增字体依赖。
- Spacing/Size 按 Figma primitive → semantic 映射实现；现有窗口默认/最小尺寸继续保持 1280×720 / 960×540，以避免 R5-02 无关的启动几何行为变化，同时暴露最终设计参考窗口 1320×700 给后续响应式阶段使用。
- `MainWindow.qml`、`VideoSurface.qml`、`PlayerChrome.qml` 改为直接消费 semantic token；原 `#...`、56/72/20、15/13 等散落视觉值不再留在核心 QML。当前占位 Header/OSC 几何分别映射最终语义 54 / 124，文字使用 Media Title 14 Medium 与 Control Body 12 Regular。
- Theme CMake 统一注册所有 token singleton，不手写 `qmldir`，继续使用 R5-01 已稳定的 `Player.Presentation.Theme` 公共 URI。

### 验证与关闭

- `theme_tokens` CTest 独立验证代表性 Color/Typography/Spacing/Layout token、Theme 兼容 alias 与最终 Figma 值，并静态扫描 shell/screens/features，阻止 raw hex color、`font.pixelSize` 和常见 visual metric 数值字面量回流。
- 字体 contract 验证声明的 family 名称与字号/字重，不要求验证机已经安装 Noto Sans SC 或 Geist Mono，避免把字体部署问题伪装成 token 定义失败；正式字体资源/发布可用性后续按实际 UI/发布任务处理。
- 用户在锁定 Windows 环境完成实测并确认 **OK**：configure **PASS**、Debug build **PASS**、`player_qml_lint` **无 warning**、全量 **43/43 CTest PASS**、`Player.exe` 启动 smoke **PASS**。
- **R5-02 正式 Complete。**

### 当前限制

- 当前代码未捆绑 Inter/Noto Sans SC/Geist Mono 字体文件，也未新增生产字体依赖；若目标机器缺少指定字体，Qt 仍可能使用系统 fallback。这不改变 R5-02 的 token API，但正式交付前必须由后续字体/发布阶段明确解决。
- Radius/Material/Elevation/Motion 等基础 token 已转入 R5-03；真正 Panel/Popover/HUD 等 Surface 对这些 token 的视觉消费仍属于 R5-08。

## 14. R5-03 实施记录（2026-08-13）

状态：**R5-03 Complete（2026-08-13）。**

### 设计来源与职责边界

- 实现值直接读取第三版 Airy Glass Figma 最终 `V3 / Geometry Primitive/Semantic`、`V3 / Effect Primitive/Material Semantic`、`V3 / Interaction Primitive/Semantic` 与 Effect Styles；不根据截图或旧 UI 猜测 Radius、Blur、Shadow、Motion、Opacity、Z-order。
- R5-03 只建立设计基础真值和 Reduce Motion 契约；不提前创建 R5-08 的 Panel/Drawer/Popover/HUD/Dialog surface，不修改播放业务或窗口模式行为。
- 为避免一个 singleton 混合多种独立视觉职责，按原因变化拆成 Radius、Blur/Material、Elevation、Motion、Opacity、Z-order 各自 primitive/semantic 模块；`Theme.qml` 继续只承担兼容 facade，不吸收新职责。
- 未新增生产依赖；未修改 PlaybackSession、libmpv、MpvVideoItem、Renderer、Render context、线程/生命周期、持久化结构或公共播放接口。

### 已实施

- `RadiusPrimitives/RadiusTokens` 映射最终几何语义：Player 32、Fullscreen 34、Maximized 0、OSC 34、Inspector 32、Popover 20、Toast 22、HUD 24、Dialog 30，并覆盖 Header/Control/List/Timeline 等已确认半径。
- `BlurPrimitives/MaterialAlphaPrimitives/MaterialTokens` 映射 Airy Glass 材质基础：Header blur 28、OSC 36、Inspector/Dialog 42、Control/HUD/Popover 18；同时集中 header/OSC/inspector/control/field/footer/selection/atmosphere 等 alpha 真值。
- `ShadowPrimitives/ElevationTokens` 映射最终四级空间阴影：Floating `42/12/0/.12`、Control `16/5/0/.08`、Window `70/24/0/.12`、Immersive Window `65/22/0/.11`；阴影颜色复用既有 `ColorPrimitives.shadowPlum #2E1F47`，不建立第二颜色真值。
- `MotionPrimitives/MotionTokens` 映射 duration `0/120/160/180/200/240 ms` 与 Figma easing；CSS cubic-bezier 转为 Qt `Easing.BezierSpline` 控制点数组，Reduce Motion 时统一切为 `Easing.Linear` 且 transition duration 归零。
- `MotionTokens.reduceMotionEnabled` 是 Presentation 层的运行时配置入口，不创建第二播放状态或持久化 owner。OSC `hideDelay = 2200 ms` 属于语义性 inactivity delay，在 Standard/Reduce Motion 均保持 2200，不因“减少动态效果”改变交互时序。
- `OpacityPrimitives/OpacityTokens` 将 Figma 交互透明度百分比在单点转换为 QML `Item.opacity` 的 `0.0–1.0`：hidden 0、scrim .18、disabled .38、idle .72、pressed .84、visible 1；消费者不再重复换算。
- `ZOrderPrimitives/ZOrderTokens` 固定层级：Video 0 → Atmosphere 10 → Media 20 → Contrast 25 → Header 30 → Overlay 35 → OSC 40 → Inspector 50 → Popover 60 → HUD 70 → Toast 80 → Dialog Scrim 90 → Dialog 100。
- Theme CMake 将上述 singleton 纳入既有 `Player.Presentation.Theme` URI；不新增公开 URI、不手写 qmldir。

### 验证与关闭

- `theme_effect_tokens` CTest 独立验证代表性 Radius、Blur/Material、Elevation、Opacity、Z-order、Motion 真值与标准模式 easing 控制点。
- Reduce Motion 测试运行时开启 `MotionTokens.reduceMotionEnabled`，确认 OSC/Inspector/Popover/Timeline/Control 等 transition duration 均变为 0、Bezier 数据移除，同时 `oscHideDelay` 保持 **2200 ms**。
- 静态扫描 shell/screens/features 中的 QML，阻止裸 `radius/z/opacity/duration` 数值字面量回流；token 定义自身不在扫描范围。
- 锁定 Windows 环境最终实测：configure **PASS**、Debug build **PASS**、`player_qml_lint` **无 warning**、全量 **44/44 CTest PASS，0 failed，49.52 s**；`theme_tokens`、`theme_effect_tokens`、`qml_module_boundaries` 均 PASS；`Player.exe` 启动 smoke **PASS**。
- **R5-03 正式 Complete。**

### 当前限制

- R5-03 建立的是材质/动效基础真值，并没有把 Qt Quick Surface 控件提前实现出来；因此当前应用不会仅因新增 Blur/Shadow token 就自动出现最终玻璃模糊和阴影，真实消费在 R5-08。
- `MotionTokens.reduceMotionEnabled` 当前是可注入的运行时 Presentation 配置点，尚未接 Preferences/系统辅助功能持久化；该集成属于后续真实设置/可访问性链路，不在 R5-03 制造额外状态 owner。

## 15. R5-04 实施记录（2026-08-13）

状态：**R5-04 Complete（2026-08-13）。**

### 设计来源与职责边界

- 图标资源直接来自第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 当前已存在的主操作 glyph，不根据截图手绘或猜测缺失资产。
- 当前纳入 `previous/play/next/volume/subtitles/playlist/fullscreen/close/search` 九个 glyph；Figma 当前没有独立 Pause/Mute/Exit-Fullscreen 等最终 glyph，因此本任务不伪造这些资源。
- `IconCatalog` 只拥有 `iconId → qrc resource / 默认 semantic color role`；公开 `Icon` primitive 只负责加载、尺寸、semantic tint、ready/error 状态和诊断。Playback/libmpv 状态不进入 Design System。

### 已实施

- SVG 统一放入 `src/presentation/qml/assets/icons/`，由 `player_presentation_primitives` 以固定 qrc alias 打包；资源 path 数据保持 Figma 导出真值。
- `Icon.qml` 公开 `iconId`、semantic `color`、`known/source/loadStatus/ready/diagnostic` 与 intrinsic size 契约；未知 id 明确返回 `Unknown icon id: ...`，不静默吞掉缺失资源。
- `IconCatalog.qml` 保持模块内部实现。首轮候选曾同时标记 singleton/internal，Qt 6.8 configure 明确拒绝；最终修复为仅 internal 的普通 QML type，由每个 `Icon` 内部实例化，不扩大公共 API。
- primary/secondary 默认 tint 复用既有 `ColorTokens.iconPrimary/iconSecondary`；着色使用锁定 Qt 6.8.3 自带 `QtQuick.Effects.MultiEffect`。没有新增外部包或第三方许可证，但 primitives 模块新增对既有 `QtQuick.Effects` runtime module 的依赖，Windows 部署需包含现有 `effectsplugin.dll`。
- `player_qml_lint` 覆盖 primitives；产品 QML 静态门禁禁止绕过 `Icon` primitive 直接引用 `assets/icons/`。
- 未修改 PlaybackSession、libmpv、Renderer、Render 生命周期、播放接口或当前 Player 骨架行为。

### 验证与关闭

- 新增独立 `icon_pipeline` CTest：验证九个 Figma glyph 从 qrc 加载成功、22/24 intrinsic size、primary/secondary semantic tint、未知 id 诊断、资产清单与产品 QML 不绕过 primitive。
- 首轮 Windows configure 在 `daf6f17` 因 Qt 6.8 `singleton + internal` 冲突硬失败；当轮 build 的 `rules.ninja` 缺失与 test development marker 缺失均为 configure 未完成后的连锁结果，不作为独立故障。
- 修复后锁定 Windows 环境最终实测：全量 **45/45 CTest PASS，0 failed，49.83 s**；`qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline` 均 PASS。
- `Player.exe` 实际启动 smoke 已由用户确认 **PASS**：启动无报错。
- **R5-04 正式 Complete。R5-05 尚未开始。**

## 16. R5-05 实施记录（2026-08-13）

状态：**R5-05 Complete（2026-08-13）。**

### 设计来源与职责边界

- 重新读取第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 的本地 Text Styles；最终样式仍以 Inter / Noto Sans SC / Geist Mono 为三类字体，并包含 Inspector/Media、Control/Meta/Technical、Strong label 与 M/S/XS Timecode 层级。R5-02 `TypographyTokens` 已与这些值一致，因此 R5-05 不建立第二套字号、字重或字体真值。
- 本任务只建立 Stage R5-05 明确要求的标题、正文、说明、Timecode primitive；R5-01 已冻结的 Feature import 规则仍是 `Feature → public Controls/Theme`，因此 typography primitive 作为 Design System 基础能力供 Controls/Surfaces 等上层公共组件组合，不通过 R5-05 私自扩大 Feature 的直接 import 边界。
- 第三版设计任务书中 Display/Keycap 等样式已经存在于 Figma/`TypographyTokens`，但当前 R5-05 不为没有真实 consumer 的样式创建额外透明 wrapper，后续按实际 Feature/Control 需求提升。

### 已实施

- 新增 `primitives/text/TitleText.qml`：公开 Inspector / Media / MediaCompact 三种 title variant，默认使用 primary text color、PlainText、单行 NoWrap + `ElideRight`。
- 新增 `BodyText.qml`：公开 Supporting / Control 正文 variant，默认使用 secondary text color和 WordWrap。
- 新增 `CaptionText.qml`：公开 Meta / Technical / MicroStrong / CompactStrong variant，默认使用 muted text color、单行 `ElideRight`。
- 新增 `TimecodeText.qml`：公开 MediumPrimary / MediumSecondary / SmallPrimary / SmallSecondary / ExtraSmall variant，统一消费 Geist Mono semantic token、单行 NoWrap；primary/secondary 颜色随时间码层级映射。
- 四个文字 primitive 纳入既有 `Player.Presentation.Primitives` QML module；没有新增 URI、生产依赖或业务状态。
- 首轮候选曾把 `PlayerChrome.qml` 直接迁移到 `Player.Presentation.Primitives`，随后被既有 `qml_module_boundaries` 正确拒绝；该越界迁移已完整撤回，现有 Feature 继续使用 R5-02 已验证的 Theme/TypographyTokens 消费方式，产品可观察骨架行为不变。

### 验证与关闭

- 新增独立 `typography_primitives` CTest，避免把 R5-05 继续堆入 R5-02 `theme_tokens` 测试。
- Contract smoke 验证 Inspector/Media/Supporting/Control/Meta/Technical/Strong semantic 字号和字重映射、默认文字色、长标题 `truncated + lineCount=1`、Timecode `Geist Mono / 12px / Medium / lineCount=1` 契约。
- 首轮 Windows configure **PASS**、Debug build **PASS**；`typography_primitives` 自身 PASS，但全量为 **45/46 PASS**，唯一失败 `qml_module_boundaries` 明确指出 `PlayerChrome → Player.Presentation.Primitives` 违反 R5-01 Feature import 边界。
- 修复没有放宽或删除 `qml_module_boundaries`：撤回 Feature→Primitives 直接依赖，并删除错误新增的“产品 QML 必须直接创建 typography primitive”全局扫描，保留 primitive 自身的真实 semantic/ellipsis/Timecode contract。
- 修复没有修改 CMake，因此不要求无意义地重复 configure；随后 Windows Debug build **PASS**，`scripts/test.ps1` 的 QML lint 门禁完成且无 warning/error，全量 **46/46 CTest PASS，0 failed，49.49 s**。`qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline`、`typography_primitives` 均 PASS。
- `Player.exe` 实际启动 smoke 已执行，命令无 QML/运行时错误输出。
- **R5-05 正式 Complete。R5-06 尚未开始。**

### 当前限制

- 本任务不改变 R5-02 已记录的字体部署事实：仓库仍未捆绑 Inter / Noto Sans SC / Geist Mono 字体文件；目标系统缺字库时 Qt 仍可能 fallback。R5-05 验证的是请求字体 family、字号/字重和文本布局契约，不把字体安装状态伪装成 primitive 逻辑。
- R5-05 只建立 Typography primitives，不提前实现 R5-06 Button，也不修改播放、Render 或 libmpv 链路。

## 17. R5-06 实施记录（2026-08-13）

状态：**R5-06 Complete（2026-08-13）。**

### 设计来源与职责边界

- 实现直接对齐第三版最终 Figma `KIOxfwTvQJlcVLinkeJAxY` 的 D3-04 Transport 与 D3-06 Utility：Secondary/Utility 为 32px hit / 22px visual / R16，Primary Playback 为 40px / 24px visual / R20；交互权重为 rest 72%、hover 100%、pressed 84%、disabled 38%、focus ring 82%，Primary glass alpha 为 rest 48%、hover 52%、pressed 42%。
- R5-06 只建立业务无关的 Button controls 与其临时交互状态，不绑定 PlaybackSession、播放/暂停业务、Inspector open state 或其他 Feature action；Feature 仍通过公开 Controls/Theme 边界消费设计系统。
- Tooltip 本轮只建立 `toolTipText + toolTipVisible` 请求契约，不伪造尚未冻结的 Tooltip Surface；真正浮层材质和 overlay host 留给 Surface/Feedback 层。

### 已实施

- 新增 `controls/buttons/ButtonBase.qml`，作为 internal 输入/状态基类统一拥有 pointer hover/press、Space/Enter/Return 键盘激活、focus、disabled gate、checked/toggle 与 tooltip-request，避免三个公开按钮重复实现输入状态机。
- 新增公开 `IconButton.qml`、`TextButton.qml`、`ToggleButton.qml`。`IconButton` 提供 Secondary/Primary emphasis 与 `opticalOffsetX`；`TextButton` 复用 BodyText Control 语义；`ToggleButton` 将 selection fill 66% 与 selection border 28% 分层实现。
- 补充真实消费才暴露的两个基础 token：`SizePrimitives.size24 → LayoutTokens.playbackIcon` 与 `OpacityPrimitives.focus → OpacityTokens.focusRing`；没有改写既有 opacity/radius/material/motion 真值。
- 收口静态审查修正 Primary Focus 视觉偏差：键盘 Focus 不再被当成 hover，Primary Focus 保持 48% rest glass + 82% focus ring；hover 仍为 52%，press 为 42%。
- `Player.Presentation.Controls` 显式依赖 Theme + Primitives，`player_qml_lint` 纳入 controls lint；R5-01 的 Feature import 门禁不放宽，Feature 仍不得直接深依赖 Primitives。
- 初版 Controls QML 源位于 `buttons/` 子目录时，internal `ButtonBase` 在运行时模块解析中不可见。修复保留物理目录模块化：为 Controls QML 设置 basename `QT_RESOURCE_ALIAS` 统一到 canonical module resource root，并使用 `NO_GENERATE_EXTRA_QMLDIRS`；`ButtonBase` 继续为 internal type，没有扩大为公共 API或引入 path/deep import。

### 验证与关闭

- 新增独立 `button_controls` CTest，覆盖公开类型加载、32/22 与 40/24 几何、R16/R20、rest/hover/press/focus 视觉 contract、真实 pointer hover/press/click、Space toggle、disabled no-op、focus tooltip-request 与 Reduce Motion 传播。
- 首轮锁定 Windows configure/build 均 PASS，但全量为 **46/47 PASS**；唯一失败 `button_controls` 的 5 个用例均在组件解析阶段报 `ButtonBase is not a type`，其余 46 项全部 PASS。该失败没有通过放宽测试或公开 internal type 绕过，而是按上述 canonical resource-root 方案修复。
- 修复后重新执行完整 Windows configure/build/test：configure **PASS**（Configuring 3.6 s / Generating 1.6 s），Debug build **PASS**，controls qmllint 门禁完成；全量 **47/47 CTest PASS，0 failed，50.62 s**，其中 `qml_module_boundaries`、`theme_tokens`、`theme_effect_tokens`、`icon_pipeline`、`typography_primitives`、`button_controls` 均 PASS。
- `Player.exe` 实际启动 smoke **PASS**：命令无 QML/运行时错误输出。
- Debug build 仍在 Qt 6.8.3 `qvariant.h` 的 QML 生成代码编译路径输出 MSVC C4702 unreachable-code warning。该 warning 来自锁定 Qt system header/生成代码路径，当前不影响 configure/build/CTest/runtime；项目未增加 warning suppression、白名单或降低质量门禁。
- 未修改 PlaybackSession、libmpv、Renderer、Render 生命周期、播放接口、配置、数据结构或持久化语义。
- **R5-06 正式 Complete。R5-07 尚未开始。**
