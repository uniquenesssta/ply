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

状态：**Candidate — QML type metadata 治理后的 Windows 最终复测待执行。**

### 已实施

- 固定四个公开 QML URI：`Player.Presentation.Theme`、`Player.Presentation.Primitives`、`Player.Presentation.Controls`、`Player.Presentation.Surfaces`。
- `Theme.qml` 从应用大模块拆入独立 Theme backing module；文件物理路径和现有 token 值不变。
- `MainWindow`、`PlayerScreen`、`VideoSurface`、`PlayerChrome` 收为 `Player.Presentation` internal type；`App` 继续作为应用模块公开入口，`QmlBootstrap::loadFromModule("Player.Presentation", "App")` 不变。
- Theme 真实使用点改为显式 `import Player.Presentation.Theme`，不再依赖应用模块内隐式同域可见性；公共设计系统不使用相对/深路径 import。
- Design System 模块未引入 Playback/libmpv 依赖；R5-01 不提前实现 R5-02 的 Airy Glass token 改造。
- 统一 QML tooling 输出到 `${CMAKE_BINARY_DIR}/qml`；模块 `qmldir` 继续由 Qt CMake API 生成，不维护重复手写清单。
- 新增 `qml_module_boundaries` 最小加载测试：同时导入四个公开 URI，并实例化读取 Theme singleton；测试允许 `QQmlComponent` 按 Qt 语义异步完成，但 Error/timeout 仍硬失败并输出 status/progress/QQmlError。
- `MpvVideoItem` 的 QML 暴露职责收敛到 `presentation/qml/types/mpv_video_item_qml_type.h`：使用 `QML_FOREIGN` + `QML_NAMED_ELEMENT(MpvVideoItem)` 描述既有 render type，不把 Presentation URI 语义写入 playback infrastructure。
- `Player.Presentation` 恢复 Qt CMake 的自动 `.qmltypes` 与 C++ 类型注册生成；删除重复的 `presentation_type_registration.*` 手写 `qmlRegisterType()` 路径，运行时与 tooling 只保留一份类型注册真值。

### 影响与兼容性

- C++ 公共接口、PlaybackSession、libmpv、依赖版本、配置和 `Player.Presentation/App` 启动入口不变。
- QML 类型名和 URI 保持 `Player.Presentation 1.0 / MpvVideoItem`；变化仅是注册来源从应用启动时手写注册切换为 Qt QML module 自动注册。
- `MpvVideoItem` 本体、Renderer、Render context、Playback 状态和所有权均未修改；未引入新的生产依赖。
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
- 同一轮 build 仍有 6 条 `VideoSurface.qml` qmllint warning，根 warning 为 `MpvVideoItem was not found`。用户提供的 `all_files.txt` 已确认 QtQuick 模块、`MpvVideoItem` 源文件及既有注册文件均实际存在，因此该问题定性为静态 QML type metadata 缺口，而非缺文件。

### qmllint metadata 治理

- 未采用 `QT_QML_SKIP_QMLLINT`、warning suppression、手写 `.qmltypes` 或降低门禁。
- 原手写 `qmlRegisterType<MpvVideoItem>("Player.Presentation", 1, 0, "MpvVideoItem")` 只在运行时执行，qmllint 不执行应用 bootstrap，因此无法获得类型的静态元数据，并连带产生 anchors/objectName 等 5 条派生 warning。
- 治理改用 Qt 6.8 官方 QML registration metadata 模式：Presentation 层通过 `QML_FOREIGN` descriptor 暴露既有 `MpvVideoItem`，`qt_add_qml_module()` 生成 `.qmltypes` 和注册代码；Playback render 类保持纯渲染职责。
- 因该治理修改了 CMake/type-registration 生成链，当前候选仍必须重新执行 Windows configure、build、qmllint、42-test CTest，并实际启动 `Player.exe` 做 QML root 创建 smoke；只有 warning 清零且运行时启动行为不回归后，R5-01 才能标 **Complete**。

### 早期 configure 阻断与修复记录

- 2026-08-13 本机 Qt 6.8.3 / MSVC configure 两次在 Generate 阶段失败，错误均为 `$<TARGET_FILE:::qmltyperegistrar>` / `No target "::qmltyperegistrar"`；因此当时 build 的 `rules.ninja` 缺失与 test 的 development runtime marker 缺失均属于 configure 未完成后的连锁结果，不作为独立故障处理。
- 第一轮曾把问题误判为三个空 QML 模块的 typeinfo 生成，并对 `Primitives/Controls/Surfaces` 添加 `NO_GENERATE_QMLTYPES`；第二次本机复测证明该假设无效，三个参数已全部撤销，未保留无效绕过。
- 对照 Qt 6.8.3 `Qt6QmlMacros.cmake` 后确认真正触发点是 executable QML module `player_app` 使用 `DEPENDENCIES TARGET player_presentation_theme`：Qt 会为 TARGET-based dependency 在 `PROJECT_SOURCE_DIR` deferred finalizer 中合并 build-tree `qt.conf`，该路径依赖 `QT_CMAKE_EXPORT_NAMESPACE`；本项目 Qt package 在 `cmake/` 子目录作用域加载，defer 回项目根后该内部变量不可用，最终把工具目标展开成 `::qmltyperegistrar`。
- 修复改用 Qt 支持的 URI 依赖 `DEPENDENCIES Player.Presentation.Theme`，保留现有 `target_link_libraries(player_app PRIVATE player_presentation_theme)` 作为真实链接关系；不移动 `find_package(Qt6)`、不改变项目 CMake 分层、不引入 Qt 内部变量补丁。
