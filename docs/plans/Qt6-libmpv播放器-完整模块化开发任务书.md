# Qt 6 + libmpv 跨平台桌面播放器
# 完整模块化开发任务书

> 文档性质：从 0 到正式落地的项目级任务执行书  
> 首发平台：Windows 10/11 x64  
> 架构目标：Windows 首发可交付，同时保持 macOS、Linux 的明确适配边界  
> UI 目标：沉浸、克制、现代、接近 IINA 的视觉与交互质感，但不机械复制 IINA  
> 播放核心：libmpv  
> UI 技术：Qt 6、Qt Quick、QML、Qt Quick Controls 2  
> 核心语言：C++20  
> 构建系统：CMake + Ninja  
> 文档版本：1.0  
> 制定日期：2026-08-06

---

## 0. 文档用途与执行约束

本任务书用于指导一个全新的 Qt 6 + libmpv 播放器从项目初始化、播放核心、渲染接入、UI 设计系统、核心功能、平台能力、测试、打包到正式发布的全过程。

本任务书不是概念性目录示例，而是后续开发时的执行基线。任何阶段开始前都必须先确认：

1. 当前 Atomic Task 的目标、输入、输出、影响范围和验收条件。
2. 当前工作区是否存在用户未提交修改。
3. 相关模块是否已经存在，现有职责是否仍然清晰。
4. 新功能应归属哪个模块，是否引入独立状态、依赖、生命周期、错误处理或测试边界。
5. 是否需要把现有单文件模块升级为目录模块。
6. 修改后必须执行哪些最小验证和阶段回归验证。
7. 任何源码、配置、依赖、接口或用户可观察行为变化，都必须同步更新根目录 `README.md` 的变更记录。

### 0.1 任务书中的“规划文件”不等于立即创建空文件

目录树表示项目的最终职责地图，而不是要求在 R1 阶段一次性创建全部空目录和空文件。

执行原则：

- 当前任务真正需要的模块才创建。
- 未来模块只保留清晰的归属位置，不创建无实现、无调用、无测试的空壳。
- 不为假想复用创建接口、插件系统、转发层或兼容层。
- 已确认会发生的扩展应保留边界，但不提前实现功能。

### 0.2 单文件升级为目录模块的强制规则

当一个已有文件需要增加第二项可独立描述、独立测试、独立演进的职责时，禁止继续把新逻辑塞入原文件。

必须把单文件升级为目录模块：

```text
升级前：
playback_controller.cpp
playback_controller.h

升级后：
playback_controller/
├─ playback_controller.cpp      # 原文件原有核心职责
├─ playback_controller.h
├─ seek_interaction.cpp         # 新增的独立职责
├─ seek_interaction.h
├─ playback_request_tracker.cpp # 后续再次出现的新职责
├─ playback_request_tracker.h
└─ CMakeLists.txt
```

具体约束：

- 原文件和新文件必须放入同一职责目录中。
- 原文件内容应迁移到职责命名明确的文件，不保留路径转发副本。
- 禁止使用 `old`、`new`、`v2`、`final`、`copy` 等版本后缀保存历史。
- 历史由 Git 管理，不在源码目录复制旧版本。
- 目录中的每个文件必须有不同且明确的变更原因。
- 如果新文件只是转发原文件调用，没有独立职责，不允许拆分。
- 如果拆分后产生循环依赖、重复状态或双重写入，则拆分方案无效，必须重新设计。

---

# 第一部分：项目目标、边界与成功定义

## 1. 产品目标

构建一个以 libmpv 为播放内核、使用 Qt Quick/QML 实现现代原生桌面体验的播放器，达到以下目标：

- 本地视频、音频稳定播放。
- 支持常见网络媒体 URL。
- 支持硬件解码、字幕、音轨、章节、播放列表和记忆进度。
- 视频画面与 QML 控件统一合成，允许悬浮控制栏、抽屉、HUD、动画和覆盖层。
- 播放核心、业务状态、渲染、平台能力、数据持久化、UI 完全分层。
- 不允许 QML 直接调用 libmpv。
- 不允许任何页面把全部交互、状态、数据和视觉堆在一个 QML 文件中。
- 能够持续增加功能，而不迫使核心播放器页面或播放引擎不断膨胀。
- 具备崩溃诊断、日志、错误反馈、自动化测试和可重复发布能力。

## 2. 首版 MVP 范围

### 2.1 必须交付

- 打开本地文件。
- 拖放文件播放。
- 打开网络 URL。
- 播放、暂停、停止。
- 绝对 Seek、相对 Seek。
- 进度显示和拖动预览。
- 音量、静音。
- 倍速。
- 全屏。
- 播放列表。
- 音轨选择。
- 字幕轨选择。
- 加载外挂字幕。
- 字幕延迟、音频延迟。
- 章节选择。
- 当前媒体信息。
- 播放错误提示。
- 加载、缓冲、暂停、播放结束状态。
- 最近播放。
- 记忆播放位置。
- 快捷键。
- 媒体键。
- 防止播放时系统休眠。
- 单实例与文件关联。
- Windows 安装包。

### 2.2 第二阶段增强

- 截图。
- A-B 循环。
- 画面比例、裁剪、旋转。
- 字幕样式。
- 播放质量预设。
- Shader 管理。
- 迷你播放器。
- 画中画。
- 高级播放统计。
- macOS 平台适配。
- Linux 平台适配。

### 2.3 不属于首版

以下能力不得在 MVP 阶段提前实现，仅保留未来归属位置：

- 在线站点解析与资源抓取。
- 媒体服务器。
- DLNA、AirPlay、Chromecast。
- 云同步。
- 账号系统。
- 在线字幕搜索。
- 插件市场。
- 脚本商店。
- 视频剪辑、转码。
- 媒体资产管理系统。
- AI 字幕或 AI 画质增强。

## 3. 可观察的完成标准

项目不能以“代码已写完”作为完成依据。正式落地至少满足：

- 4K 本地视频可连续播放、暂停、Seek、切换全屏，不出现 UI 线程阻塞。
- 连续打开和关闭至少 100 个媒体文件，不崩溃、不残留播放线程。
- 损坏文件、不支持编码、无权限文件和不可达 URL 均产生明确错误。
- 进度条拖动期间不被后台位置更新抢回。
- 快速切换媒体时，旧媒体事件不能污染新媒体状态。
- 关闭窗口时，渲染上下文必须先于 mpv handle 安全释放。
- 播放结束、停止、加载失败、切换文件后，轨道、章节、时长等状态必须正确清空或替换。
- 配置和播放历史写入失败不能导致播放器核心崩溃。
- QML 页面不存在直接 `mpv_command`、`mpv_set_property` 或 C 指针操作。
- 任一核心模块可以通过单元测试或集成测试独立验证。
- 安装包在干净 Windows 环境完成安装、启动、播放、卸载。

---

# 第二部分：技术基线与关键决策

## 4. 技术栈

| 类别 | 选择 | 说明 |
|---|---|---|
| UI 框架 | Qt 6 + Qt Quick/QML | 用于沉浸式播放器界面、动画和统一场景合成 |
| 语言 | C++20 | Qt 原生集成成熟，便于 libmpv C API、OpenGL 和平台 API 接入 |
| 播放内核 | libmpv 动态链接 | 不修改 mpv 源码，不启动外部 mpv.exe 作为正式架构 |
| 视频渲染 | libmpv Render API + OpenGL | 首版通过 QQuickFramebufferObject 接入 Qt Quick Scene Graph |
| UI 控件 | Qt Quick Controls 2，自定义样式 | 禁止依赖默认桌面控件外观形成最终 UI |
| 构建 | CMake + Ninja | 统一本地、CI 和发布构建 |
| 测试 | Qt Test + Qt Quick Test + CTest | 覆盖 C++ 单元、集成、QML 交互和阶段回归 |
| 持久化 | SQLite + Qt SQL | 播放历史、恢复位置、设置元数据；不新增生产依赖 |
| 日志 | QLoggingCategory + 自定义日志落盘 | 分模块、可过滤、可轮转、可脱敏 |
| 打包 | CPack/平台脚本 + windeployqt | Windows 首发，后续添加 macdeployqt 和 Linux 打包 |
| 依赖策略 | 动态链接、显式版本清单 | libmpv、Qt、运行库和许可证必须可追踪 |

## 5. Qt 版本策略

- API 兼容基线采用 Qt 6.8 系列。
- Windows 首发工具链采用 MSVC 2022 x64。
- 实际锁定的 Qt patch 版本必须写入 `cmake/DependencyVersions.cmake`。
- 开源版和商业版的 LTS 获取范围不同，R0 阶段必须先确认许可证和可获得补丁版本。
- 不允许本地开发、CI、发布分别使用不明版本。

## 6. Render API 选择

首版采用：

```text
libmpv OpenGL Render API
        ↓
MpvVideoItem : QQuickFramebufferObject
        ↓
MpvVideoRenderer : QQuickFramebufferObject::Renderer
        ↓
Qt Quick Scene Graph
        ↓
QML 覆盖控件统一合成
```

原因：

- libmpv 的 OpenGL Render API 是成熟路径。
- `QQuickFramebufferObject` 明确用于将 OpenGL FBO 渲染集成进 Qt Quick。
- Qt 6 中该类只在 Qt Quick 使用 OpenGL 后端时工作，因此应用启动阶段必须显式固定 OpenGL 图形 API。
- `QQuickRhiItem` 可作为未来多图形 API 研究方向，但其底层 QRhi 使用涉及私有 API 兼容性，不作为 MVP 首发依赖。

## 7. 关键架构决策

### 7.1 libmpv 不是应用领域模型

libmpv 的字符串属性、命令、事件和 `mpv_node` 只允许存在于 `infrastructure/mpv` 内部。

UI 和应用层只能使用：

- `PlaybackCommand`
- `PlaybackEvent`
- `PlaybackSnapshot`
- `MediaDescriptor`
- `TrackDescriptor`
- `PlaybackError`

### 7.2 单一播放状态权威

`PlaybackSession` 是播放生命周期和播放状态的唯一权威拥有者。

禁止：

- QML 自己维护一份“真实播放状态”。
- 播放列表模块自己修改播放状态。
- mpv 适配器和 ViewModel 分别维护可写状态。
- 数据库恢复位置直接写入 QML。

### 7.3 命令与事件分离

```text
QML 用户操作
  ↓
PlayerViewModel
  ↓
PlaybackCommandBus
  ↓
PlaybackSession
  ↓
MpvClient
  ↓
libmpv

libmpv event/property
  ↓
MpvEventDecoder
  ↓
PlaybackEvent
  ↓
PlaybackReducer
  ↓
PlaybackSnapshot
  ↓
PlayerViewModel
  ↓
QML
```

### 7.4 UI 不拥有业务副作用

QML 可以拥有：

- Hover、Pressed、Focus。
- 控件显隐动画。
- 临时弹层开关。
- 拖动过程中的临时像素位置。

QML 不可以拥有：

- 播放历史写入。
- 最终 Seek 提交规则。
- 自动播放下一项。
- 音轨和字幕选择业务状态。
- 文件关联。
- 设置落盘。
- mpv 生命周期。

---

# 第三部分：总体分层与依赖方向

## 8. 分层定义

```text
Presentation
    ↓
Application
    ↓
Domain
    ↑
Infrastructure / Persistence / Platform
```

### 8.1 Presentation

负责：

- QML 视觉与交互。
- ViewModel 和只读列表模型。
- 将用户意图转换为应用层调用。
- 将领域快照转换为 UI 友好展示值。

不得负责：

- libmpv 调用。
- SQL。
- 文件系统副作用。
- 平台原生 API。
- 播放状态真值。

### 8.2 Application

负责：

- 用例编排。
- 跨领域协作。
- 命令路由。
- 生命周期协调。
- 自动下一项、恢复播放位置等工作流。

不得负责：

- QML 视觉。
- mpv C API 细节。
- SQL 字符串。
- Win32/AppKit/X11/Wayland 细节。

### 8.3 Domain

负责：

- 播放命令、事件、状态和状态迁移规则。
- 媒体、轨道、章节、播放列表等领域概念。
- 与具体框架无关的策略和验证。

不得依赖：

- Qt Quick。
- OpenGL。
- SQL。
- libmpv 头文件。
- 平台 SDK。

可使用 Qt Core 的前提：

- 项目初期允许使用 `QString`、`QUrl`、`QDateTime` 等稳定值类型，降低无收益的转换层。
- 禁止 Domain 引入 `QObject`、QML、窗口、SQL、网络和图形模块。

### 8.4 Infrastructure

负责：

- libmpv 适配。
- Render API。
- 外部文件、网络和进程边界。
- 将外部错误翻译为领域错误。

### 8.5 Persistence

负责：

- SQLite 连接、迁移、事务。
- 仓储实现。
- 数据行和领域对象转换。

### 8.6 Platform

负责：

- Windows、macOS、Linux 的原生能力。
- 窗口材质、媒体键、休眠抑制、文件关联、单实例。

## 9. 禁止依赖

- `domain` → `presentation`
- `domain` → `infrastructure/mpv`
- `presentation/qml` → SQL
- `presentation/qml` → libmpv
- `persistence` → QML
- `platform/windows` → 播放状态内部结构
- 任一 Feature QML 页面直接依赖另一个 Feature 的内部 QML 文件
- `settings` 直接改写 mpv，必须经过 Playback Application
- `playlist` 直接持有 `mpv_handle`

---

# 第四部分：项目根目录规划

## 10A. 工作区与共享依赖结构

所有体积较大、可被多个仓库复用、会由安装器或构建工具下载的依赖必须直接位于 Git 仓库的上一层，不得再增加名为 `dependencies`、`third_party` 或其他名称的总目录，也不得复制进每个项目根目录。默认布局：

```text
workspace/
├─ Qt/<version>/msvc2022_64/
├─ libmpv/windows-x64/
├─ cmake/                       # 可选：独立可移植 CMake
├─ ninja/                       # 可选：独立可移植 Ninja
├─ downloads/
├─ cache/cmake/fetchcontent/
└─ player/
```

所有源代码、CMake 预设、脚本和文档中的依赖路径必须从仓库根目录使用 `../` 开始计算，不保存开发机盘符或其他绝对路径。仓库名称和仓库所在的具体绝对位置不得参与路径约定。Qt 使用 `../Qt/<version>/msvc2022_64`，libmpv 使用 `../libmpv/<platform>`；CMake 和 Ninja 优先复用 `../cmake`、`../ninja` 或 Qt Installer 已安装在 `../Qt/Tools` 下的工具，最后才回退到系统 `PATH`。所有未来 `FetchContent` 下载复用 `../cache/cmake/fetchcontent`。

上一层目录只保存第三方 SDK、运行库、可移植工具、下载缓存和构建缓存；项目源码、配置、测试和产品资源仍归 Git 仓库所有。Visual Studio/MSVC 与 Windows SDK 属于系统级工具链，不复制到上一层目录。

## 10. 完整仓库结构

```text
player/
├─ ALL_AI_CODE.md
├─ AI_PROJECT_RULES.md
├─ README.md
├─ LICENSES/
│  ├─ README.md
│  ├─ Qt.txt
│  ├─ mpv.txt
│  ├─ FFmpeg.txt
│  └─ third-party-notices.md
├─ CMakeLists.txt
├─ CMakePresets.json
├─ cmake/
│  ├─ AppTargets.cmake
│  ├─ CompilerOptions.cmake
│  ├─ CompilerWarnings.cmake
│  ├─ DependencyPaths.cmake
│  ├─ DependencyVersions.cmake
│  ├─ FindLibMpv.cmake
│  ├─ InstallLayout.cmake
│  ├─ Packaging.cmake
│  ├─ Sanitizers.cmake
│  ├─ StaticAnalysis.cmake
│  └─ WindowsRuntime.cmake
├─ config/
│  ├─ defaults/
│  │  ├─ playback_defaults.json
│  │  ├─ shortcut_defaults.json
│  │  └─ ui_defaults.json
│  └─ schemas/
│     ├─ playback_settings.schema.json
│     ├─ shortcut_settings.schema.json
│     └─ ui_settings.schema.json
├─ docs/
│  ├─ architecture/
│  │  ├─ dependency-rules.md
│  │  ├─ state-ownership.md
│  │  ├─ threading-model.md
│  │  ├─ rendering-lifecycle.md
│  │  └─ persistence-contracts.md
│  ├─ decisions/
│  │  ├─ ADR-0001-libmpv-render-api.md
│  │  ├─ ADR-0002-opengl-first.md
│  │  ├─ ADR-0003-single-playback-owner.md
│  │  └─ ADR-0004-qml-boundaries.md
│  ├─ plans/
│  │  └─ Qt6-libmpv播放器-完整模块化开发任务书.md
│  └─ testing/
│     ├─ media-fixture-policy.md
│     ├─ manual-smoke-matrix.md
│     └─ release-acceptance.md
├─ packaging/
│  ├─ windows/
│  │  ├─ installer.wxs
│  │  ├─ file-associations.wxs
│  │  ├─ package.ps1
│  │  └─ verify-package.ps1
│  ├─ macos/
│  │  ├─ entitlements.plist
│  │  └─ package.sh
│  └─ linux/
│     ├─ app.desktop.in
│     ├─ app.metainfo.xml.in
│     └─ package.sh
├─ resources/
│  ├─ icons/
│  │  ├─ app/
│  │  ├─ controls/
│  │  ├─ media/
│  │  └─ status/
│  ├─ images/
│  │  ├─ placeholders/
│  │  └─ onboarding/
│  ├─ shaders/
│  │  ├─ built-in/
│  │  └─ README.md
│  └─ translations/
│     ├─ player_zh_CN.ts
│     └─ player_en_US.ts
├─ scripts/
│  ├─ modules/
│  │  └─ DependencyPaths.psm1
│  ├─ bootstrap-workspace.ps1
│  ├─ configure.ps1
│  ├─ build.ps1
│  ├─ test.ps1
│  ├─ package.ps1
│  ├─ verify-dependencies.ps1
│  └─ verify-clean-tree.ps1
├─ src/
│  ├─ CMakeLists.txt
│  ├─ app/
│  ├─ foundation/
│  ├─ playback/
│  ├─ media/
│  ├─ playlist/
│  ├─ tracks/
│  ├─ history/
│  ├─ settings/
│  ├─ shortcuts/
│  ├─ screenshots/
│  ├─ persistence/
│  ├─ platform/
│  ├─ diagnostics/
│  └─ presentation/
├─ tests/
│  ├─ CMakeLists.txt
│  ├─ fixtures/
│  ├─ unit/
│  ├─ integration/
│  ├─ qml/
│  ├─ platform/
│  └─ release/
└─ tools/
   ├─ fixture_inspector/
   ├─ playback_probe/
   └─ state_trace_viewer/
```

---

# 第五部分：根目录文件职责

## 11. 根文件

| 文件 | 唯一职责 | 禁止内容 |
|---|---|---|
| `CMakeLists.txt` | 声明项目、加载顶层 CMake 模块、添加 `src` 与 `tests` | 不直接堆积每个源码文件、不写平台打包细节 |
| `CMakePresets.json` | 定义 dev、debug、release、asan 等可重复构建预设 | 不保存用户本机绝对路径和秘密配置 |
| `README.md` | 项目入口、构建运行、架构概览、实际变更记录、已知限制 | 不保存冗长过程日志，不创建平行 CHANGELOG |
| `ALL_AI_CODE.md` | 通用开发准则 | 不由普通功能任务随意修改 |
| `AI_PROJECT_RULES.md` | 项目补充准则 | 不复制成多份规则文件 |

## 12. `cmake/` 文件职责

| 文件 | 职责 |
|---|---|
| `AppTargets.cmake` | 创建主程序、测试工具和资源目标的公共函数 |
| `CompilerOptions.cmake` | C++ 标准、可见性、统一编译选项 |
| `CompilerWarnings.cmake` | MSVC/Clang/GCC 警告等级和项目自有代码警告策略 |
| `DependencyPaths.cmake` | 计算仓库上一层共享依赖根目录，发现 Qt kit，固定 libmpv SDK 与 FetchContent 缓存位置；不得包含下载逻辑 |
| `DependencyVersions.cmake` | Qt、libmpv、最低 CMake、工具链版本的唯一锁定位置 |
| `FindLibMpv.cmake` | 查找 libmpv 头文件、动态库、运行时文件并创建导入目标 `LibMpv::LibMpv` |
| `InstallLayout.cmake` | 安装目录、资源目录、许可证目录布局 |
| `Packaging.cmake` | 统一调用平台打包模块 |
| `Sanitizers.cmake` | ASan/UBSan 等开发验证选项，不进入默认 Release |
| `StaticAnalysis.cmake` | clang-tidy、cppcheck 等静态检查开关 |
| `WindowsRuntime.cmake` | Windows 运行库、DLL 复制、PDB 策略 |

## 13. `config/` 文件职责

配置文件只保存用户可配置的产品设置默认值，不直接保存 mpv 原始属性字符串。

- `playback_defaults.json`：音量、倍速、硬解模式、恢复播放等产品设置。
- `shortcut_defaults.json`：动作 ID 到快捷键序列的默认映射。
- `ui_defaults.json`：主题、控制栏自动隐藏时间、窗口恢复策略。
- `schemas/*.schema.json`：用于开发期验证配置结构和迁移工具，不在运行时成为第二套业务模型。

## 13A. `scripts/` 依赖路径职责

| 文件 | 职责 |
|---|---|
| `modules/DependencyPaths.psm1` | PowerShell 侧唯一的工作区、Qt、CMake、Ninja 和 libmpv 路径解析模块 |
| `bootstrap-workspace.ps1` | 只创建上一层共享依赖目录结构，不下载、不安装、不修改系统环境 |
| `verify-dependencies.ps1` | 检查当前阶段必需的 Qt、MSVC、CMake、Ninja；libmpv 在 R2 前仅提示不阻塞 |
| `configure.ps1` | 解析共享依赖路径并显式传入 CMake preset |
| `build.ps1` | 使用已解析的 CMake 执行构建，不重新下载依赖 |
| `test.ps1` | 构建测试目标并执行 CTest，不拥有依赖发现规则 |

禁止在多个脚本中复制 Qt 搜索、工具搜索和路径拼接逻辑；这些规则只能存在于 `DependencyPaths.psm1`。

---

# 第六部分：`src/app` 应用启动与生命周期

## 14. 目录结构

```text
src/app/
├─ CMakeLists.txt
├─ main.cpp
├─ bootstrap/
│  ├─ application_bootstrap.cpp
│  ├─ application_bootstrap.h
│  ├─ graphics_backend_bootstrap.cpp
│  ├─ graphics_backend_bootstrap.h
│  ├─ qml_bootstrap.cpp
│  ├─ qml_bootstrap.h
│  ├─ runtime_paths.cpp
│  └─ runtime_paths.h
├─ composition/
│  ├─ application_container.cpp
│  ├─ application_container.h
│  ├─ playback_composition.cpp
│  ├─ playback_composition.h
│  ├─ persistence_composition.cpp
│  ├─ persistence_composition.h
│  ├─ platform_composition.cpp
│  └─ platform_composition.h
└─ lifecycle/
   ├─ application_lifecycle.cpp
   ├─ application_lifecycle.h
   ├─ shutdown_coordinator.cpp
   └─ shutdown_coordinator.h
```

## 15. 逐文件职责

### `main.cpp`

唯一职责：

- 创建 `QGuiApplication`。
- 在任何 `QQuickWindow` 创建前调用图形后端 bootstrap。
- 调用 `ApplicationBootstrap::run()`。
- 返回应用退出码。

禁止：

- 创建 mpv handle。
- 注册几十个 QML 类型。
- 初始化数据库细节。
- 处理播放命令。
- 塞入平台判断分支。

### `bootstrap/application_bootstrap.*`

负责启动顺序：

1. 解析运行参数。
2. 初始化运行路径。
3. 初始化日志。
4. 构建依赖容器。
5. 执行数据库迁移。
6. 注册 QML 类型和单例。
7. 加载根 QML。
8. 连接关闭流程。

不持有业务状态，不实现具体服务。

### `bootstrap/graphics_backend_bootstrap.*`

负责：

- 强制 Qt Quick 使用 OpenGL。
- 设置兼容的 OpenGL format。
- 配置必要的渲染环境变量。
- 启动时验证图形后端是否满足 libmpv Render API。

不得创建播放器对象。

### `bootstrap/qml_bootstrap.*`

负责：

- 注册 QML module。
- 注册 ViewModel、只读 model 和 value type。
- 设置 QML import path。
- 加载 `App.qml`。
- 捕获 QML 对象创建失败。

### `bootstrap/runtime_paths.*`

负责解析并提供：

- 可执行文件目录。
- 用户配置目录。
- 数据库目录。
- 日志目录。
- 截图目录。
- Shader 目录。
- 临时目录。

路径对象创建后只读，禁止业务模块自行拼接相同路径。

### `composition/application_container.*`

唯一 composition root：

- 持有顶层对象所有权。
- 明确构造和销毁顺序。
- 连接接口与实现。
- 不包含业务算法。

### `composition/playback_composition.*`

只负责创建和装配：

- `PlaybackSession`
- `MpvClient`
- `PlaybackCommandBus`
- `PlaybackStatePublisher`
- `PlayerViewModel`
- 播放历史和播放列表协作者

### `composition/persistence_composition.*`

只负责：

- 数据库连接配置。
- 迁移器。
- 仓储实现。
- 持久化工作线程。

### `composition/platform_composition.*`

根据编译平台创建：

- 窗口集成。
- 媒体键源。
- 休眠抑制器。
- 单实例协调器。
- 文件关联入口。

### `lifecycle/application_lifecycle.*`

负责应用级状态：

```text
Constructed → Starting → Running → Stopping → Stopped
```

禁止把播放器详细状态混入应用生命周期。

### `lifecycle/shutdown_coordinator.*`

必须执行并验证严格关闭顺序：

1. 停止接受新的 UI 命令。
2. 关闭弹层和计时器。
3. 停止媒体键与外部入口。
4. 停止播放列表自动推进。
5. 停止渲染更新回调。
6. 等待正在进行的 render 返回。
7. 释放 `mpv_render_context`。
8. 请求 PlaybackSession 停止。
9. 消费并终止 mpv 事件循环。
10. 销毁 `mpv_handle`。
11. 刷新历史和设置写入。
12. 关闭数据库。
13. 关闭日志。

---

# 第七部分：`src/foundation` 基础设施原语

## 16. 目录结构

```text
src/foundation/
├─ CMakeLists.txt
├─ errors/
│  ├─ application_error.cpp
│  ├─ application_error.h
│  ├─ error_code.h
│  └─ error_context.h
├─ ids/
│  ├─ media_id.cpp
│  ├─ media_id.h
│  ├─ request_id.cpp
│  └─ request_id.h
├─ logging/
│  ├─ log_categories.cpp
│  ├─ log_categories.h
│  ├─ log_file_sink.cpp
│  ├─ log_file_sink.h
│  ├─ log_redactor.cpp
│  └─ log_redactor.h
├─ threading/
│  ├─ thread_affinity.cpp
│  ├─ thread_affinity.h
│  ├─ cancellation_token.cpp
│  └─ cancellation_token.h
└─ time/
   ├─ monotonic_clock.cpp
   ├─ monotonic_clock.h
   ├─ wall_clock.cpp
   └─ wall_clock.h
```

## 17. 边界说明

`foundation` 不是垃圾桶。

允许进入的内容必须同时满足：

- 与任何单一业务 Feature 无关。
- 被至少两个顶层模块真实使用，或属于架构基础边界。
- 职责可以单独命名和测试。

禁止创建：

- `utils.*`
- `helpers.*`
- `common.*`
- `misc.*`
- 无边界的 `manager.*`

### 17.1 `errors/`

- `error_code.h`：稳定的产品错误码枚举。
- `error_context.h`：路径、媒体 ID、请求 ID、外部错误码等结构化上下文。
- `application_error.*`：错误值对象；支持用户消息、诊断消息、严重级别、可恢复性。

### 17.2 `ids/`

- `media_id.*`：媒体条目的稳定标识，不直接使用 UI 列表索引。
- `request_id.*`：异步命令关联标识，防止旧回复污染新请求。

### 17.3 `logging/`

- `log_categories.*`：声明 `app.lifecycle`、`playback.mpv`、`playback.render`、`ui.interaction`、`persistence` 等分类。
- `log_file_sink.*`：日志文件写入、轮转、刷新。
- `log_redactor.*`：移除 URL token、用户路径中的敏感片段和命令参数。

### 17.4 `threading/`

- `thread_affinity.*`：开发期断言对象运行线程。
- `cancellation_token.*`：应用层异步任务取消，不包装 mpv 内部取消机制。

### 17.5 `time/`

- `monotonic_clock.*`：计算持续时间、节流和超时。
- `wall_clock.*`：生成持久化时间戳。

---

# 第八部分：`src/playback` 播放核心

## 18. 总体目录

```text
src/playback/
├─ CMakeLists.txt
├─ domain/
│  ├─ commands/
│  ├─ events/
│  ├─ state/
│  ├─ models/
│  ├─ policies/
│  └─ ports/
├─ application/
│  ├─ session/
│  ├─ command_bus/
│  ├─ state_publisher/
│  ├─ workflows/
│  └─ requests/
└─ infrastructure/
   └─ mpv/
      ├─ client/
      ├─ initialization/
      ├─ commands/
      ├─ properties/
      ├─ events/
      ├─ render/
      └─ errors/
```

## 19. Domain：命令

```text
src/playback/domain/commands/
├─ playback_command.h
├─ load_media_command.h
├─ transport_command.h
├─ seek_command.h
├─ volume_command.h
├─ speed_command.h
├─ track_command.h
├─ subtitle_command.h
├─ video_adjustment_command.h
└─ lifecycle_command.h
```

### 文件职责

- `playback_command.h`：所有命令的封闭 variant 类型和公共元数据，不实现每种命令逻辑。
- `load_media_command.h`：加载、替换、追加媒体所需参数。
- `transport_command.h`：播放、暂停、切换暂停、停止、上一项、下一项。
- `seek_command.h`：绝对、相对、百分比 Seek，精确/关键帧策略。
- `volume_command.h`：音量、静音。
- `speed_command.h`：速度设置和重置。
- `track_command.h`：选择音频、字幕、视频轨道。
- `subtitle_command.h`：加载、移除、重新加载、延迟和样式意图。
- `video_adjustment_command.h`：比例、旋转、裁剪等第二阶段命令。
- `lifecycle_command.h`：初始化、终止、取消加载等生命周期命令。

规则：

- 命令是不可变值对象。
- 命令不携带 QObject 指针。
- 命令不包含 mpv 属性名称。
- 命令必须带 `RequestId`，需要关联回复时不得省略。

## 20. Domain：事件

```text
src/playback/domain/events/
├─ playback_event.h
├─ lifecycle_event.h
├─ media_event.h
├─ position_event.h
├─ buffering_event.h
├─ track_event.h
├─ chapter_event.h
├─ property_event.h
└─ failure_event.h
```

职责：

- `playback_event.h`：领域事件 variant。
- `lifecycle_event.h`：初始化、就绪、停止、终止。
- `media_event.h`：开始加载、文件加载完成、播放结束、媒体替换。
- `position_event.h`：位置、时长、Seek 开始/完成。
- `buffering_event.h`：缓冲开始、缓冲进度、缓冲结束。
- `track_event.h`：轨道列表、选择变化。
- `chapter_event.h`：章节列表、当前章节。
- `property_event.h`：经过类型化后的非核心属性更新；不得成为通用字符串容器。
- `failure_event.h`：命令失败、加载失败、渲染失败和不可恢复错误。

## 21. Domain：状态

```text
src/playback/domain/state/
├─ playback_lifecycle.h
├─ playback_snapshot.h
├─ playback_reducer.cpp
├─ playback_reducer.h
├─ playback_invariants.cpp
├─ playback_invariants.h
├─ playback_selectors.cpp
└─ playback_selectors.h
```

### `playback_lifecycle.h`

定义状态机：

```text
Uninitialized
  → Initializing
  → Idle
  → Loading
  → Ready
  → Playing ↔ Paused
  → Ended
  → Idle

任意可运行状态 → RecoverableError
任意状态 → ShuttingDown → Stopped
```

### `playback_snapshot.h`

唯一只读播放快照，至少包括：

- lifecycle
- activeMediaId
- generation
- paused
- buffering
- seeking
- eofReached
- position
- duration
- volume
- muted
- speed
- cacheInfo
- videoInfo
- audioInfo
- tracks
- chapters
- selectedTrackIds
- currentError

`generation` 在每次媒体替换时递增，所有异步事件必须携带 generation，旧 generation 事件一律丢弃。

### `playback_reducer.*`

唯一职责：

- 接收旧快照和领域事件。
- 产生新快照。
- 不执行外部副作用。
- 不访问数据库。
- 不发 mpv 命令。

### `playback_invariants.*`

集中验证：

- 无 active media 时不能处于 Playing。
- position 不得小于 0。
- 已知 duration 时 position 不得无界超过 duration。
- 轨道选择必须存在于当前 generation 的轨道列表。
- Stopped 后不得仍保持 buffering、seeking。

### `playback_selectors.*`

提供 UI 和工作流常用派生值：

- `canPlay`
- `canPause`
- `canSeek`
- `isBusy`
- `displayPosition`
- `hasSubtitles`
- `shouldPreventSleep`

不得修改状态。

## 22. Domain：模型

```text
src/playback/domain/models/
├─ media_descriptor.cpp
├─ media_descriptor.h
├─ playback_position.h
├─ playback_duration.h
├─ cache_status.h
├─ video_stream_info.h
├─ audio_stream_info.h
├─ track_descriptor.h
├─ chapter_descriptor.h
└─ playback_statistics.h
```

每个文件只保存对应概念及其验证，不包含 UI 文案和 mpv 字符串解析。

## 23. Domain：策略

```text
src/playback/domain/policies/
├─ seek_policy.cpp
├─ seek_policy.h
├─ resume_policy.cpp
├─ resume_policy.h
├─ auto_advance_policy.cpp
├─ auto_advance_policy.h
├─ progress_persistence_policy.cpp
└─ progress_persistence_policy.h
```

- `seek_policy`：快进快退步长、精确 Seek 策略。
- `resume_policy`：何时提示恢复、何时自动恢复、接近结尾时是否忽略。
- `auto_advance_policy`：播放结束如何推进列表。
- `progress_persistence_policy`：写入频率、最小位置、接近结尾清除规则。

## 24. Domain：端口

```text
src/playback/domain/ports/
├─ playback_backend.h
├─ playback_state_observer.h
├─ resume_position_store.h
└─ sleep_inhibition_port.h
```

- `playback_backend.h`：应用层操作播放后端的抽象，不暴露 mpv。
- `playback_state_observer.h`：订阅快照的边界。
- `resume_position_store.h`：恢复位置存储接口。
- `sleep_inhibition_port.h`：播放状态触发平台休眠抑制。

端口必须服务真实边界，不创建仅转发一次调用的抽象。

## 25. Application：播放会话

```text
src/playback/application/session/
├─ playback_session.cpp
├─ playback_session.h
├─ playback_session_state.cpp
├─ playback_session_state.h
├─ playback_event_dispatcher.cpp
├─ playback_event_dispatcher.h
├─ playback_shutdown.cpp
└─ playback_shutdown.h
```

### `playback_session.*`

唯一权威协调者：

- 运行在专用 playback thread。
- 接收命令。
- 调用 PlaybackBackend。
- 接收后端事件。
- 调用 reducer。
- 发布新快照。
- 管理 generation。
- 不包含 QML。

当文件继续增长时，不把请求追踪、关闭流程、事件分发塞回该文件，使用上述同目录文件承接。

### `playback_session_state.*`

保存 session 内部运行状态：

- 当前 generation。
- 待处理请求表。
- 是否接受新命令。
- 是否正在关闭。

不得复制 `PlaybackSnapshot` 中的业务状态。

### `playback_event_dispatcher.*`

根据事件类型将事件路由给 reducer、请求跟踪器和工作流通知，不实现状态规则。

### `playback_shutdown.*`

封装 session 级关闭协议，不负责应用全局关闭。

## 26. Application：命令总线

```text
src/playback/application/command_bus/
├─ playback_command_bus.cpp
├─ playback_command_bus.h
├─ playback_command_queue.cpp
├─ playback_command_queue.h
├─ command_coalescer.cpp
└─ command_coalescer.h
```

- `playback_command_bus`：线程安全提交入口。
- `playback_command_queue`：有序队列和关闭语义。
- `command_coalescer`：合并高频音量、预览 Seek 等可安全合并命令；不得合并加载、停止等语义命令。

## 27. Application：状态发布

```text
src/playback/application/state_publisher/
├─ playback_state_publisher.cpp
├─ playback_state_publisher.h
├─ snapshot_delivery_policy.cpp
└─ snapshot_delivery_policy.h
```

- 发布不可变快照。
- 将高频位置更新节流至 UI 可接受频率。
- 关键状态变化立即发布。
- 不让 UI 轮询 mpv。

## 28. Application：工作流

```text
src/playback/application/workflows/
├─ load_media_workflow.cpp
├─ load_media_workflow.h
├─ resume_playback_workflow.cpp
├─ resume_playback_workflow.h
├─ auto_advance_workflow.cpp
├─ auto_advance_workflow.h
├─ progress_persistence_workflow.cpp
├─ progress_persistence_workflow.h
├─ sleep_inhibition_workflow.cpp
└─ sleep_inhibition_workflow.h
```

每个工作流只编排一个跨模块业务流程。

示例 `resume_playback_workflow`：

1. 媒体加载完成。
2. 查询历史位置。
3. 应用 resume policy。
4. 发出 Seek 命令或通知 UI 提示。
5. 记录结果。

## 29. Application：异步请求

```text
src/playback/application/requests/
├─ playback_request.cpp
├─ playback_request.h
├─ playback_request_tracker.cpp
├─ playback_request_tracker.h
├─ request_timeout_policy.cpp
└─ request_timeout_policy.h
```

- 跟踪异步命令回复。
- 使用 request ID 和 generation 双重匹配。
- 超时只代表应用不再等待，不可假设 mpv 已取消。
- 关闭时清理全部 pending request 并返回明确取消结果。

---

# 第九部分：libmpv 基础设施适配

## 30. 目录结构

```text
src/playback/infrastructure/mpv/
├─ CMakeLists.txt
├─ client/
│  ├─ mpv_handle.cpp
│  ├─ mpv_handle.h
│  ├─ mpv_client.cpp
│  ├─ mpv_client.h
│  ├─ mpv_wakeup_bridge.cpp
│  ├─ mpv_wakeup_bridge.h
│  ├─ mpv_event_loop.cpp
│  └─ mpv_event_loop.h
├─ initialization/
│  ├─ mpv_initializer.cpp
│  ├─ mpv_initializer.h
│  ├─ mpv_option_profile.cpp
│  ├─ mpv_option_profile.h
│  ├─ mpv_option_validator.cpp
│  └─ mpv_option_validator.h
├─ commands/
│  ├─ mpv_command_encoder.cpp
│  ├─ mpv_command_encoder.h
│  ├─ mpv_command_executor.cpp
│  ├─ mpv_command_executor.h
│  ├─ mpv_command_reply.cpp
│  └─ mpv_command_reply.h
├─ properties/
│  ├─ mpv_property_registry.cpp
│  ├─ mpv_property_registry.h
│  ├─ mpv_property_observer.cpp
│  ├─ mpv_property_observer.h
│  ├─ mpv_property_decoder.cpp
│  └─ mpv_property_decoder.h
├─ events/
│  ├─ mpv_event_decoder.cpp
│  ├─ mpv_event_decoder.h
│  ├─ mpv_node_reader.cpp
│  ├─ mpv_node_reader.h
│  ├─ mpv_track_list_decoder.cpp
│  ├─ mpv_track_list_decoder.h
│  ├─ mpv_chapter_list_decoder.cpp
│  └─ mpv_chapter_list_decoder.h
├─ render/
│  ├─ mpv_render_context.cpp
│  ├─ mpv_render_context.h
│  ├─ mpv_gl_proc_resolver.cpp
│  ├─ mpv_gl_proc_resolver.h
│  ├─ mpv_render_update_bridge.cpp
│  ├─ mpv_render_update_bridge.h
│  ├─ mpv_render_parameters.cpp
│  └─ mpv_render_parameters.h
└─ errors/
   ├─ mpv_error_mapper.cpp
   ├─ mpv_error_mapper.h
   ├─ mpv_error_text.cpp
   └─ mpv_error_text.h
```

## 31. 客户端文件职责

### `mpv_handle.*`

RAII 封装：

- 创建 `mpv_handle`。
- 禁止复制，允许受控移动或固定所有权。
- 析构时仅在正确生命周期阶段调用销毁。
- 不执行初始化选项。
- 不处理事件。

### `mpv_client.*`

实现 `PlaybackBackend`：

- 接收类型化领域命令。
- 调用命令 encoder 和 executor。
- 管理属性观察注册。
- 将 mpv 事件交给 decoder。
- 不保存 UI 状态。

### `mpv_wakeup_bridge.*`

- 注册 `mpv_set_wakeup_callback`。
- 将 C callback 安全唤醒 Qt playback thread。
- callback 中不得执行阻塞工作、不得直接修改 QML。
- 关闭前注销或使 callback 无效化。

### `mpv_event_loop.*`

- 在被唤醒后循环调用 `mpv_wait_event(handle, 0)`，直到 `MPV_EVENT_NONE`。
- 保证事件队列持续消费。
- 控制单次批量处理上限，避免饿死 Qt 事件循环。
- 将原始事件传给 decoder。

## 32. 初始化文件职责

### `mpv_initializer.*`

严格顺序：

1. 创建 handle。
2. 设置 pre-initialize options。
3. 禁用未经允许的用户级 mpv 配置，或使用应用独立 config dir。
4. 设置日志等级。
5. 调用 `mpv_initialize`。
6. 注册属性观察。
7. 创建 Render Context 所需前置配置。

### `mpv_option_profile.*`

将产品设置映射为经过批准的 mpv option 集合：

- 默认安全配置。
- 性能优先。
- 质量优先。
- 兼容模式。

不允许 UI 自由写入任意 option。

### `mpv_option_validator.*`

验证：

- option 可设置阶段。
- 值范围。
- 平台支持。
- 是否需要重启播放核心。
- 是否属于高级用户明确启用的原始参数。

## 33. 命令文件职责

### `mpv_command_encoder.*`

将领域命令转换为 `mpv_node` 或 argv，不执行命令。

### `mpv_command_executor.*`

- 调用 `mpv_command_async` 或相应异步 API。
- 绑定 request ID。
- 处理立即失败。
- 除初始化与安全关闭外，禁止在 UI 可达路径使用可能长期阻塞的同步调用。

### `mpv_command_reply.*`

将 `MPV_EVENT_COMMAND_REPLY` 转成领域回复或失败事件。

## 34. 属性文件职责

### `mpv_property_registry.*`

唯一属性清单：

- 属性名。
- mpv format。
- 领域目标类型。
- 是否高频。
- 初始化时是否必须。
- 失败是否致命。

禁止在多个文件散落重复字符串属性名。

### `mpv_property_observer.*`

- 分配稳定 observer ID。
- 注册和注销属性观察。
- 关闭时保证不再分发。

### `mpv_property_decoder.*`

将 `MPV_EVENT_PROPERTY_CHANGE` 安全转换为类型化事件。

## 35. 事件解码职责

### `mpv_event_decoder.*`

只做事件类型分发和公共校验。

当文件需要同时解析复杂 track list、chapter list 时，必须使用独立 decoder，不得塞在主 decoder 中。

### `mpv_node_reader.*`

- 只读访问 `mpv_node` map/array。
- 类型检查。
- 缺失字段处理。
- 数值转换范围检查。
- 不承载 track 或 chapter 业务含义。

### `mpv_track_list_decoder.*`

只负责 track-list → `TrackDescriptor`。

### `mpv_chapter_list_decoder.*`

只负责 chapter-list → `ChapterDescriptor`。

## 36. Render Context 文件职责

### `mpv_render_context.*`

RAII 封装 `mpv_render_context`：

- 在有效 OpenGL context 下创建。
- 注册 update callback。
- 接收 FBO 渲染参数。
- 调用 `mpv_render_context_render`。
- 释放时确保没有并发 render。

### `mpv_gl_proc_resolver.*`

仅负责把 libmpv 请求的 OpenGL 函数名解析到 Qt 当前 context。

### `mpv_render_update_bridge.*`

- 接收 mpv render update callback。
- 只发出线程安全的“需要重绘”信号。
- 不在 callback 内直接调用 QML 或进行 render。

### `mpv_render_parameters.*`

构建：

- FBO id。
- 像素尺寸。
- 翻转 Y。
- 背景透明策略。
- ICC/HDR 未来参数。

---

# 第十部分：媒体、播放列表、轨道与历史

## 37. `src/media`

```text
src/media/
├─ CMakeLists.txt
├─ domain/
│  ├─ media_source.cpp
│  ├─ media_source.h
│  ├─ media_identity.cpp
│  ├─ media_identity.h
│  ├─ media_metadata.cpp
│  ├─ media_metadata.h
│  ├─ media_capabilities.h
│  └─ media_validation_result.h
├─ application/
│  ├─ media_open_coordinator.cpp
│  ├─ media_open_coordinator.h
│  ├─ media_drop_handler.cpp
│  ├─ media_drop_handler.h
│  ├─ url_open_workflow.cpp
│  └─ url_open_workflow.h
└─ infrastructure/
   ├─ local_media_validator.cpp
   ├─ local_media_validator.h
   ├─ url_media_validator.cpp
   └─ url_media_validator.h
```

### 职责

- `media_source`：本地文件、URL 等来源类型，不执行 IO。
- `media_identity`：规范化路径/URL 后生成稳定 identity，用于历史匹配。
- `media_metadata`：标题、容器、尺寸等媒体元数据。
- `media_capabilities`：是否可 Seek、是否 live、是否有视频/音频。
- `media_open_coordinator`：统一文件选择、拖放、命令行、文件关联入口。
- `media_drop_handler`：解析拖放 payload 并排序，不直接加载 mpv。
- `local_media_validator`：存在性、权限、类型基础检查。
- `url_media_validator`：URL scheme 和基本结构检查，不实现网站解析。

## 38. `src/playlist`

```text
src/playlist/
├─ CMakeLists.txt
├─ domain/
│  ├─ playlist.cpp
│  ├─ playlist.h
│  ├─ playlist_entry.cpp
│  ├─ playlist_entry.h
│  ├─ playlist_position.h
│  ├─ repeat_mode.h
│  ├─ shuffle_state.cpp
│  └─ shuffle_state.h
├─ application/
│  ├─ playlist_controller.cpp
│  ├─ playlist_controller.h
│  ├─ playlist_navigation.cpp
│  ├─ playlist_navigation.h
│  ├─ playlist_mutation.cpp
│  ├─ playlist_mutation.h
│  ├─ playlist_auto_advance.cpp
│  └─ playlist_auto_advance.h
└─ presentation/
   ├─ playlist_list_model.cpp
   └─ playlist_list_model.h
```

### 状态所有权

- `Playlist` 拥有队列顺序和当前 entry ID。
- `PlaybackSession` 拥有当前实际加载媒体。
- `playlist_auto_advance` 协调两者，但不复制任何一方状态。
- UI 只能通过 `playlist_controller` 请求增删、重排、选择。

## 39. `src/tracks`

```text
src/tracks/
├─ CMakeLists.txt
├─ domain/
│  ├─ audio_track.cpp
│  ├─ audio_track.h
│  ├─ subtitle_track.cpp
│  ├─ subtitle_track.h
│  ├─ video_track.cpp
│  ├─ video_track.h
│  ├─ track_language.cpp
│  └─ track_language.h
├─ application/
│  ├─ track_selection.cpp
│  ├─ track_selection.h
│  ├─ external_subtitle_loader.cpp
│  ├─ external_subtitle_loader.h
│  ├─ subtitle_delay_controller.cpp
│  ├─ subtitle_delay_controller.h
│  ├─ audio_delay_controller.cpp
│  └─ audio_delay_controller.h
└─ presentation/
   ├─ audio_track_list_model.cpp
   ├─ audio_track_list_model.h
   ├─ subtitle_track_list_model.cpp
   └─ subtitle_track_list_model.h
```

## 40. `src/history`

```text
src/history/
├─ CMakeLists.txt
├─ domain/
│  ├─ playback_history_entry.cpp
│  ├─ playback_history_entry.h
│  ├─ resume_position.cpp
│  └─ resume_position.h
├─ application/
│  ├─ history_recorder.cpp
│  ├─ history_recorder.h
│  ├─ recent_media_query.cpp
│  ├─ recent_media_query.h
│  ├─ resume_position_query.cpp
│  └─ resume_position_query.h
└─ presentation/
   ├─ recent_media_list_model.cpp
   └─ recent_media_list_model.h
```

`history_recorder` 只接收经过 progress policy 过滤后的写入请求，不直接订阅所有高频 position 事件后每次写数据库。

---

# 第十一部分：设置、快捷键、截图

## 41. `src/settings`

```text
src/settings/
├─ CMakeLists.txt
├─ domain/
│  ├─ playback_settings.cpp
│  ├─ playback_settings.h
│  ├─ ui_settings.cpp
│  ├─ ui_settings.h
│  ├─ subtitle_settings.cpp
│  ├─ subtitle_settings.h
│  ├─ settings_snapshot.cpp
│  └─ settings_snapshot.h
├─ application/
│  ├─ settings_controller.cpp
│  ├─ settings_controller.h
│  ├─ settings_change_router.cpp
│  ├─ settings_change_router.h
│  ├─ settings_restart_policy.cpp
│  └─ settings_restart_policy.h
└─ presentation/
   ├─ settings_view_model.cpp
   └─ settings_view_model.h
```

### 规则

- 设置领域模型使用产品语义，例如 `HardwareDecodingMode::Auto`，不保存 `hwdec=auto-safe` 等 mpv 字符串。
- `settings_change_router` 决定变更是即时应用、下次媒体应用、重建 render context 还是重启应用。
- QML 不直接写 JSON 或数据库。

## 42. `src/shortcuts`

```text
src/shortcuts/
├─ CMakeLists.txt
├─ domain/
│  ├─ action_id.h
│  ├─ shortcut_binding.cpp
│  ├─ shortcut_binding.h
│  ├─ shortcut_context.h
│  └─ shortcut_conflict.cpp
├─ application/
│  ├─ shortcut_dispatcher.cpp
│  ├─ shortcut_dispatcher.h
│  ├─ shortcut_registry.cpp
│  ├─ shortcut_registry.h
│  ├─ shortcut_editor.cpp
│  └─ shortcut_editor.h
└─ presentation/
   ├─ shortcut_list_model.cpp
   └─ shortcut_list_model.h
```

- `action_id` 是稳定动作标识，UI 文案和按键都不能充当业务 ID。
- `shortcut_context` 区分全局播放器、文本输入、列表焦点、全屏等上下文。
- `shortcut_conflict` 检测冲突并提供明确结果，不静默覆盖。

## 43. `src/screenshots`

```text
src/screenshots/
├─ CMakeLists.txt
├─ domain/
│  ├─ screenshot_request.h
│  ├─ screenshot_result.h
│  └─ screenshot_naming_policy.cpp
├─ application/
│  ├─ screenshot_workflow.cpp
│  └─ screenshot_workflow.h
└─ infrastructure/
   ├─ screenshot_file_writer.cpp
   └─ screenshot_file_writer.h
```

首版截图工作流可以调用 mpv screenshot 命令，但文件命名、目录、冲突策略、结果通知必须由产品模块负责。

---

# 第十二部分：持久化

## 44. 目录结构

```text
src/persistence/
├─ CMakeLists.txt
├─ database/
│  ├─ database_connection.cpp
│  ├─ database_connection.h
│  ├─ database_worker.cpp
│  ├─ database_worker.h
│  ├─ database_transaction.cpp
│  └─ database_transaction.h
├─ migrations/
│  ├─ migration.cpp
│  ├─ migration.h
│  ├─ migration_registry.cpp
│  ├─ migration_registry.h
│  ├─ migration_runner.cpp
│  └─ migration_runner.h
├─ repositories/
│  ├─ sqlite_history_repository.cpp
│  ├─ sqlite_history_repository.h
│  ├─ sqlite_settings_repository.cpp
│  ├─ sqlite_settings_repository.h
│  ├─ sqlite_playlist_repository.cpp
│  └─ sqlite_playlist_repository.h
└─ mapping/
   ├─ history_row_mapper.cpp
   ├─ history_row_mapper.h
   ├─ settings_row_mapper.cpp
   └─ settings_row_mapper.h
```

## 45. 数据库职责

### `database_connection.*`

- 创建命名连接。
- 设置 busy timeout、foreign keys 和日志。
- 只在所属线程使用连接。
- 不执行具体业务 SQL。

### `database_worker.*`

- 独立线程顺序执行数据库任务。
- 提供关闭和 flush。
- 禁止 UI 线程直接执行可能阻塞的数据库写入。

### `database_transaction.*`

- RAII transaction。
- 明确 commit/rollback。
- 失败不得静默忽略。

### 迁移

数据库 schema 变更必须通过 migration registry，禁止运行时“发现缺列就 ALTER”散落在仓储中。

建议首版表：

```sql
schema_migrations(
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL
)

playback_history(
    media_identity TEXT PRIMARY KEY,
    display_title TEXT,
    source_type INTEGER NOT NULL,
    source_value TEXT NOT NULL,
    last_position_ms INTEGER NOT NULL,
    duration_ms INTEGER,
    completed INTEGER NOT NULL DEFAULT 0,
    last_played_at TEXT NOT NULL
)

settings(
    namespace TEXT NOT NULL,
    key TEXT NOT NULL,
    value_json TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    PRIMARY KEY(namespace, key)
)

saved_playlists(
    playlist_id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
)

saved_playlist_entries(
    playlist_id TEXT NOT NULL,
    entry_id TEXT NOT NULL,
    ordinal INTEGER NOT NULL,
    media_identity TEXT NOT NULL,
    source_type INTEGER NOT NULL,
    source_value TEXT NOT NULL,
    display_title TEXT,
    PRIMARY KEY(playlist_id, entry_id),
    FOREIGN KEY(playlist_id) REFERENCES saved_playlists(playlist_id) ON DELETE CASCADE
)
```

数据迁移原则：

- 每个 migration 只做一个可描述 schema 变化。
- migration 一旦发布不得修改内容，只能新增下一版本。
- 迁移前备份策略必须在正式发布前验证。
- 失败时应用进入可诊断状态，不允许带着半迁移数据库继续运行。

---

# 第十三部分：平台模块

## 46. 总体结构

```text
src/platform/
├─ CMakeLists.txt
├─ ports/
│  ├─ native_window_port.h
│  ├─ media_key_source.h
│  ├─ sleep_inhibitor.h
│  ├─ single_instance_port.h
│  ├─ file_association_port.h
│  └─ system_theme_port.h
├─ windows/
│  ├─ CMakeLists.txt
│  ├─ window/
│  │  ├─ windows_window_integration.cpp
│  │  ├─ windows_window_integration.h
│  │  ├─ windows_native_event_filter.cpp
│  │  ├─ windows_native_event_filter.h
│  │  ├─ windows_hit_test.cpp
│  │  ├─ windows_hit_test.h
│  │  ├─ windows_dwm_material.cpp
│  │  └─ windows_dwm_material.h
│  ├─ media_keys/
│  │  ├─ windows_media_key_source.cpp
│  │  └─ windows_media_key_source.h
│  ├─ power/
│  │  ├─ windows_sleep_inhibitor.cpp
│  │  └─ windows_sleep_inhibitor.h
│  ├─ instance/
│  │  ├─ windows_single_instance.cpp
│  │  └─ windows_single_instance.h
│  └─ associations/
│     ├─ windows_file_association.cpp
│     └─ windows_file_association.h
├─ macos/
│  ├─ window/
│  ├─ media_keys/
│  ├─ power/
│  ├─ instance/
│  └─ associations/
└─ linux/
   ├─ window/
   ├─ media_keys/
   ├─ power/
   ├─ instance/
   └─ associations/
```

## 47. Windows 窗口职责

### `windows_window_integration.*`

- 获取原生 HWND。
- 应用无边框窗口策略。
- 协调最大化、最小化、全屏和恢复。
- 调用 hit test 和 DWM material 模块。

### `windows_native_event_filter.*`

- 只接收和分发需要处理的 Windows message。
- 不承载所有窗口逻辑。
- 遇到新增 WM_* 场景时，若形成独立职责，创建同目录文件而不是继续堆 switch。

### `windows_hit_test.*`

- 计算拖动标题区域、边缘 resize、窗口按钮区域。
- 以 UI 暴露的几何信息为输入，不读取 QML 内部对象树。

### `windows_dwm_material.*`

- 深色模式。
- 圆角策略。
- Mica/Acrylic 可用性判断。
- 失败时回退为普通背景，不影响播放。

## 48. 媒体键、休眠、单实例和文件关联

- `windows_media_key_source`：将系统媒体键转成稳定 `ActionId`。
- `windows_sleep_inhibitor`：仅根据 `shouldPreventSleep` 开关系统执行状态；必须幂等释放。
- `windows_single_instance`：第二实例把文件/URL 参数发送给主实例后退出。
- `windows_file_association`：实现文件类型注册的产品接口；安装器实际写入由 packaging 负责。

macOS/Linux 目录只有进入对应平台阶段才创建具体源文件，不允许提前复制 Windows 实现形成假适配。

---

# 第十四部分：诊断模块

## 49. 目录结构

```text
src/diagnostics/
├─ CMakeLists.txt
├─ playback_trace/
│  ├─ playback_trace_event.cpp
│  ├─ playback_trace_event.h
│  ├─ playback_trace_buffer.cpp
│  ├─ playback_trace_buffer.h
│  ├─ playback_trace_exporter.cpp
│  └─ playback_trace_exporter.h
├─ runtime/
│  ├─ runtime_diagnostics.cpp
│  ├─ runtime_diagnostics.h
│  ├─ dependency_report.cpp
│  └─ dependency_report.h
└─ crash/
   ├─ crash_marker.cpp
   └─ crash_marker.h
```

### 职责

- `playback_trace_event`：结构化记录 command、event、generation、request ID、状态摘要。
- `playback_trace_buffer`：有界环形缓冲，不能无限增长。
- `playback_trace_exporter`：用户主动导出诊断包，执行脱敏。
- `runtime_diagnostics`：图形后端、Qt、libmpv、平台能力状态。
- `dependency_report`：运行时实际加载的库版本和路径。
- `crash_marker`：检测上次是否异常退出；不实现第三方崩溃上传。

---

# 第十五部分：Presentation C++ 边界

## 50. 目录结构

```text
src/presentation/
├─ CMakeLists.txt
├─ qml_registration/
│  ├─ qml_type_registration.cpp
│  ├─ qml_type_registration.h
│  ├─ qml_singleton_registration.cpp
│  └─ qml_singleton_registration.h
├─ viewmodels/
│  ├─ player/
│  │  ├─ player_view_model.cpp
│  │  ├─ player_view_model.h
│  │  ├─ player_transport_view_model.cpp
│  │  ├─ player_transport_view_model.h
│  │  ├─ player_timeline_view_model.cpp
│  │  ├─ player_timeline_view_model.h
│  │  ├─ player_track_view_model.cpp
│  │  ├─ player_track_view_model.h
│  │  ├─ player_window_view_model.cpp
│  │  └─ player_window_view_model.h
│  ├─ playlist/
│  ├─ settings/
│  ├─ history/
│  └─ diagnostics/
├─ models/
│  ├─ chapter_list_model.cpp
│  ├─ chapter_list_model.h
│  ├─ menu_action_list_model.cpp
│  └─ menu_action_list_model.h
├─ formatting/
│  ├─ duration_formatter.cpp
│  ├─ duration_formatter.h
│  ├─ media_title_formatter.cpp
│  └─ media_title_formatter.h
├─ render/
│  ├─ mpv_video_item.cpp
│  ├─ mpv_video_item.h
│  ├─ mpv_video_renderer.cpp
│  └─ mpv_video_renderer.h
└─ qml/
   └─ ...
```

## 51. ViewModel 拆分

禁止创建一个几千行 `PlayerViewModel` 包含全部播放器行为。

### `player_view_model.*`

- 仅聚合子 ViewModel。
- 暴露当前媒体概要和顶层状态。
- 不重复实现 transport、timeline、track、window 行为。

### `player_transport_view_model.*`

- play、pause、stop、next、previous。
- `canPlay`、`canPause`、`isPlaying`。

### `player_timeline_view_model.*`

- 展示位置、总时长、缓冲范围。
- 管理 UI scrub session：begin/update/commit/cancel。
- 拖动期间冻结后台 position 对 thumb 的覆盖。
- 最终 commit 只发送一次 Seek，除非产品明确启用实时预览。

### `player_track_view_model.*`

- 音轨、字幕、章节模型。
- 选择和延迟操作。

### `player_window_view_model.*`

- 全屏、置顶、迷你模式、窗口标题。
- 不调用 Win32，依赖 `NativeWindowPort`。

## 52. Render Item

### `mpv_video_item.*`

GUI 线程对象：

- QML 可见属性。
- 创建 Renderer。
- 将尺寸、DPR、可见性和 render context 句柄安全同步到渲染线程。
- 不直接调用 `mpv_render_context_render`。

### `mpv_video_renderer.*`

Qt Quick 渲染线程对象：

- 创建 FBO。
- 同步 item 状态。
- 调用 mpv render context。
- 保存最小渲染资源。
- 不访问数据库、播放列表或 QML 对象。

---

# 第十六部分：QML UI 完整目录规划

## 53. QML 总体结构

```text
src/presentation/qml/
├─ qmldir
├─ App.qml
├─ shell/
│  ├─ MainWindow.qml
│  ├─ WindowChrome.qml
│  ├─ WindowTitleRegion.qml
│  ├─ WindowControls.qml
│  ├─ WindowResizeHandles.qml
│  ├─ GlobalOverlayLayer.qml
│  └─ GlobalDialogLayer.qml
├─ theme/
│  ├─ Theme.qml
│  ├─ tokens/
│  │  ├─ ColorTokens.qml
│  │  ├─ TypographyTokens.qml
│  │  ├─ SpacingTokens.qml
│  │  ├─ RadiusTokens.qml
│  │  ├─ ElevationTokens.qml
│  │  ├─ MotionTokens.qml
│  │  ├─ OpacityTokens.qml
│  │  └─ ZOrderTokens.qml
│  ├─ palettes/
│  │  ├─ DarkPalette.qml
│  │  ├─ LightPalette.qml
│  │  └─ HighContrastPalette.qml
│  └─ metrics/
│     ├─ ControlMetrics.qml
│     ├─ PlayerMetrics.qml
│     └─ WindowMetrics.qml
├─ primitives/
│  ├─ typography/
│  │  ├─ AppText.qml
│  │  ├─ HeadingText.qml
│  │  ├─ BodyText.qml
│  │  ├─ CaptionText.qml
│  │  └─ TimecodeText.qml
│  ├─ input/
│  │  ├─ HoverArea.qml
│  │  ├─ FocusRing.qml
│  │  ├─ PressFeedback.qml
│  │  └─ WheelHandler.qml
│  ├─ shape/
│  │  ├─ RoundedSurface.qml
│  │  ├─ Divider.qml
│  │  └─ Scrim.qml
│  └─ icon/
│     ├─ AppIcon.qml
│     └─ IconGlyph.qml
├─ controls/
│  ├─ buttons/
│  │  ├─ IconButton.qml
│  │  ├─ TextButton.qml
│  │  ├─ ToggleIconButton.qml
│  │  ├─ TransportButton.qml
│  │  └─ WindowControlButton.qml
│  ├─ sliders/
│  │  ├─ TimelineSlider.qml
│  │  ├─ VolumeSlider.qml
│  │  ├─ SettingsSlider.qml
│  │  └─ SliderHandle.qml
│  ├─ fields/
│  │  ├─ AppTextField.qml
│  │  ├─ UrlTextField.qml
│  │  └─ SearchField.qml
│  ├─ selection/
│  │  ├─ AppCheckBox.qml
│  │  ├─ AppRadioButton.qml
│  │  ├─ SegmentedControl.qml
│  │  └─ SelectionRow.qml
│  ├─ menu/
│  │  ├─ ContextMenu.qml
│  │  ├─ MenuItem.qml
│  │  ├─ MenuSeparator.qml
│  │  └─ SubmenuItem.qml
│  └─ feedback/
│     ├─ Tooltip.qml
│     ├─ Toast.qml
│     ├─ InlineError.qml
│     ├─ BusyIndicator.qml
│     └─ EmptyState.qml
├─ surfaces/
│  ├─ FloatingPanel.qml
│  ├─ GlassPanel.qml
│  ├─ DrawerSurface.qml
│  ├─ DialogSurface.qml
│  ├─ PopoverSurface.qml
│  └─ HudSurface.qml
├─ features/
│  ├─ player/
│  ├─ playlist/
│  ├─ tracks/
│  ├─ chapters/
│  ├─ history/
│  ├─ settings/
│  ├─ shortcuts/
│  └─ diagnostics/
├─ screens/
│  ├─ player/
│  ├─ settings/
│  ├─ history/
│  └─ diagnostics/
└─ dialogs/
   ├─ OpenUrlDialog.qml
   ├─ OpenFileDialog.qml
   ├─ ResumePlaybackDialog.qml
   ├─ ErrorDetailsDialog.qml
   └─ AboutDialog.qml
```

## 54. QML 分层规则

### Primitives

最小视觉原语：字体、形状、图标、交互反馈。不得依赖任何业务 ViewModel。

### Controls

可复用交互控件。可以暴露通用 value、checked、triggered，但不知道“播放”“字幕”等业务含义。

### Surfaces

负责面板、弹窗、抽屉、HUD 的视觉容器，不负责面板中的业务内容。

### Features

组合 controls 并绑定某一业务 ViewModel，例如音量控制、播放列表、轨道选择。

### Screens

负责页面级布局与 Feature 组合，不实现 Feature 内部细节。

### Shell

负责主窗口、全局层级和窗口交互，不实现播放器内容。

## 55. Player Feature 目录

```text
src/presentation/qml/features/player/
├─ transport/
│  ├─ TransportControls.qml
│  ├─ PrimaryPlayButton.qml
│  ├─ PreviousButton.qml
│  ├─ NextButton.qml
│  └─ StopButton.qml
├─ timeline/
│  ├─ PlayerTimeline.qml
│  ├─ TimelineTrack.qml
│  ├─ TimelineBufferedRange.qml
│  ├─ TimelineProgress.qml
│  ├─ TimelineThumb.qml
│  ├─ TimelineHoverPreview.qml
│  └─ TimelineChapterMarker.qml
├─ volume/
│  ├─ VolumeControl.qml
│  ├─ VolumeButton.qml
│  ├─ VolumePopover.qml
│  └─ VolumeHud.qml
├─ status/
│  ├─ LoadingOverlay.qml
│  ├─ BufferingOverlay.qml
│  ├─ PausedIndicator.qml
│  ├─ EndedOverlay.qml
│  ├─ PlaybackErrorOverlay.qml
│  └─ UnsupportedMediaOverlay.qml
├─ metadata/
│  ├─ MediaTitle.qml
│  ├─ MediaTechnicalSummary.qml
│  └─ LiveIndicator.qml
├─ actions/
│  ├─ PlayerActionBar.qml
│  ├─ SubtitleAction.qml
│  ├─ AudioTrackAction.qml
│  ├─ PlaylistAction.qml
│  ├─ MoreActions.qml
│  └─ FullscreenAction.qml
├─ hud/
│  ├─ PlayerHudLayer.qml
│  ├─ SeekHud.qml
│  ├─ SpeedHud.qml
│  ├─ TrackChangedHud.qml
│  └─ ScreenshotHud.qml
└─ visibility/
   ├─ ControlVisibilityRegion.qml
   ├─ CursorVisibilityController.qml
   └─ ControlAutoHideTimer.qml
```

### 特别约束

- `TransportControls.qml` 不包含 timeline、volume、playlist drawer。
- `PlayerTimeline.qml` 不包含播放按钮和全屏逻辑。
- `VolumeControl.qml` 不写播放设置持久化。
- `PlayerHudLayer.qml` 只负责 HUD 消息队列显示，不判断业务事件。
- `ControlAutoHideTimer.qml` 只处理交互超时，不直接暂停或播放。
- `CursorVisibilityController.qml` 只控制鼠标隐藏，必须尊重弹窗、菜单、拖动和无障碍状态。

## 56. Player Screen 目录

```text
src/presentation/qml/screens/player/
├─ PlayerScreen.qml
├─ layout/
│  ├─ PlayerContentLayout.qml
│  ├─ VideoViewport.qml
│  ├─ PlayerTopRegion.qml
│  ├─ PlayerBottomRegion.qml
│  └─ PlayerSideRegion.qml
├─ overlays/
│  ├─ PlayerOverlayStack.qml
│  ├─ PlayerControlOverlay.qml
│  ├─ PlayerStatusOverlay.qml
│  ├─ PlayerHudOverlay.qml
│  └─ PlayerDropOverlay.qml
├─ drawers/
│  ├─ PlayerDrawerHost.qml
│  ├─ PlaylistDrawer.qml
│  ├─ TrackDrawer.qml
│  └─ ChapterDrawer.qml
└─ states/
   ├─ PlayerVisualState.qml
   ├─ PlayerFullscreenState.qml
   └─ PlayerInteractionState.qml
```

### `PlayerScreen.qml`

只允许：

- 组合 `VideoViewport`、布局区、overlay stack 和 drawer host。
- 绑定顶层 PlayerViewModel。
- 设置页面级焦点范围。

禁止：

- 定义所有按钮。
- 写 Seek 计算。
- 写播放列表 delegate。
- 写字幕选择菜单。
- 写复杂动画。
- 直接调用平台 API。

如果 `PlayerScreen.qml` 因新增功能超过其组合职责，必须把功能放进对应 feature 或 screen 子目录，不允许继续添加内联组件。

## 57. Playlist QML

```text
src/presentation/qml/features/playlist/
├─ PlaylistPanel.qml
├─ PlaylistHeader.qml
├─ PlaylistList.qml
├─ PlaylistEntryDelegate.qml
├─ PlaylistEntryArtwork.qml
├─ PlaylistEntryMetadata.qml
├─ PlaylistEntryActions.qml
├─ PlaylistDropIndicator.qml
├─ PlaylistEmptyState.qml
└─ PlaylistFooter.qml
```

- Delegate 不直接修改模型内部数组。
- 重排由 ViewModel 命令完成。
- 当前播放项、选中项、Hover 项必须是不同状态。
- 长标题、缺失时长、失效文件、网络 URL 必须有明确展示规则。

## 58. Tracks QML

```text
src/presentation/qml/features/tracks/
├─ TrackPanel.qml
├─ AudioTrackSection.qml
├─ SubtitleTrackSection.qml
├─ TrackSelectionList.qml
├─ TrackSelectionDelegate.qml
├─ ExternalSubtitleAction.qml
├─ SubtitleDelayControl.qml
└─ AudioDelayControl.qml
```

## 59. Settings QML

```text
src/presentation/qml/features/settings/
├─ SettingsNavigation.qml
├─ SettingsSection.qml
├─ SettingsRow.qml
├─ SettingsDescription.qml
├─ playback/
│  ├─ PlaybackSettingsPanel.qml
│  ├─ HardwareDecodeSetting.qml
│  ├─ ResumePlaybackSetting.qml
│  └─ CacheSetting.qml
├─ video/
│  ├─ VideoSettingsPanel.qml
│  ├─ ScalingSetting.qml
│  └─ RenderQualitySetting.qml
├─ audio/
│  ├─ AudioSettingsPanel.qml
│  └─ PreferredAudioLanguageSetting.qml
├─ subtitle/
│  ├─ SubtitleSettingsPanel.qml
│  ├─ PreferredSubtitleLanguageSetting.qml
│  └─ SubtitleStyleSetting.qml
├─ interface/
│  ├─ InterfaceSettingsPanel.qml
│  ├─ ThemeSetting.qml
│  └─ ControlVisibilitySetting.qml
└─ advanced/
   ├─ AdvancedSettingsPanel.qml
   ├─ RawMpvOptionWarning.qml
   └─ DiagnosticsExportAction.qml
```

每个设置行只绑定一个设置概念。复杂设置必须升级为独立组件，不把多个不相关选项堆在 `SettingsPanel.qml`。

## 60. UI 视觉规则

### 60.1 层级

- 视频始终是第一视觉主体。
- 控制栏默认隐藏，交互时出现。
- 一级操作显著，二级操作收敛到抽屉或菜单。
- 错误和缓冲必须清晰，但不长期遮挡画面。

### 60.2 Token

所有颜色、字号、字重、间距、圆角、阴影、动画时间必须来自 token。

禁止：

- Feature 文件散落 `#FFFFFF`。
- 页面随意写 `radius: 13`。
- 每个按钮自定义一套 Hover 动画。
- 同类文本使用不同字号但无语义名称。

### 60.3 动效

- 按钮反馈：80–120 ms。
- 控件出现：120–180 ms。
- 控件消失：160–220 ms。
- 抽屉：180–260 ms。
- 动效必须可被“减少动态效果”设置关闭或简化。

### 60.4 可访问性

- 所有按钮必须有 accessible name。
- 键盘焦点可见。
- 不只通过颜色表达状态。
- 文本和控件满足基本对比度。
- 关键操作支持键盘。
- 禁用状态仍可提供 tooltip 说明原因。

---

# 第十七部分：线程模型与资源生命周期

## 61. 线程划分

```text
GUI Thread
├─ QGuiApplication
├─ QML object tree
├─ ViewModels
└─ 用户输入

Playback Thread
├─ PlaybackSession
├─ MpvClient
├─ mpv_handle
├─ mpv_wait_event 消费
└─ 命令执行与状态 reducer

Qt Quick Render Thread
├─ MpvVideoRenderer
├─ OpenGL context
├─ FBO
└─ mpv_render_context_render

Database Thread
├─ QSqlDatabase named connection
├─ migrations after startup gate
└─ repository read/write queue
```

## 62. 所有权表

| 状态/资源 | 唯一拥有者 | 其他模块访问方式 |
|---|---|---|
| `mpv_handle` | `MpvHandle`，Playback Thread | 只能通过 `MpvClient` |
| `mpv_render_context` | `MpvRenderContext` | Render Item 获得受控句柄/协调接口 |
| 播放快照 | `PlaybackSession` | 不可变快照发布 |
| 当前队列 | `Playlist` | Controller 命令和只读 model |
| QML 对象树 | Qt GUI Thread | 不跨线程直接访问 |
| FBO | `MpvVideoRenderer` | 仅 Render Thread |
| SQLite connection | `DatabaseWorker` | Repository 异步任务 |
| 设置真值 | `SettingsController` + Repository | 快照和变更命令 |
| 播放历史 | History Repository | 查询和记录用例 |

## 63. 关键竞态防护

### 快速切换媒体

- 每次 load 生成新的 `generation`。
- 所有后端事件附带 generation。
- 旧 generation 的 property 和 command reply 不更新当前状态。

### Seek 拖动

- `beginScrub` 后 UI thumb 使用预览值。
- 后台 position 仍更新快照，但不覆盖预览值。
- `commitScrub` 发送一次最终 Seek。
- 收到 seek 完成或目标附近 position 后退出 pending 状态。

### 关闭与回调

- shutdown flag 先阻止新 command。
- render callback bridge 先失效。
- 等待 render critical section 退出。
- 释放 render context。
- 再销毁 mpv handle。

### 数据库写入

- position 写入节流。
- 同一 media identity 的写入顺序化。
- 应用关闭执行 bounded flush；超时必须记录未落盘风险，不无限阻塞退出。

---

# 第十八部分：错误体系

## 64. 错误分类

```text
ApplicationError
├─ ConfigurationError
├─ DependencyError
├─ MediaOpenError
├─ PlaybackBackendError
├─ RenderError
├─ PersistenceError
├─ PlatformIntegrationError
└─ ValidationError
```

每个错误至少包含：

- 稳定错误码。
- 用户可读消息 key。
- 诊断文本。
- 上下文。
- 是否可恢复。
- 建议动作。
- 原始外部错误码，可选。

## 65. 错误展示规则

- 短暂、可恢复操作失败：Toast。
- 当前媒体无法播放：播放器中部错误 overlay。
- 需要用户决策：Dialog。
- 仅诊断信息：日志，不打扰用户。
- 启动阻塞错误：启动失败页面。

禁止捕获后静默忽略。

---

# 第十九部分：测试目录与验证边界

## 66. 测试结构

```text
tests/
├─ CMakeLists.txt
├─ fixtures/
│  ├─ README.md
│  ├─ generated/
│  ├─ manifests/
│  │  ├─ local-media.json
│  │  ├─ malformed-media.json
│  │  └─ subtitle-fixtures.json
│  └─ subtitles/
├─ unit/
│  ├─ playback/
│  ├─ playlist/
│  ├─ media/
│  ├─ tracks/
│  ├─ settings/
│  ├─ history/
│  └─ persistence/
├─ integration/
│  ├─ mpv/
│  ├─ render/
│  ├─ playback_session/
│  ├─ persistence/
│  └─ application_lifecycle/
├─ qml/
│  ├─ controls/
│  ├─ features/
│  ├─ screens/
│  └─ accessibility/
├─ platform/
│  └─ windows/
└─ release/
   ├─ startup_smoke.cpp
   ├─ playback_smoke.cpp
   └─ shutdown_smoke.cpp
```

## 67. 单元测试重点

### Playback reducer

- 各生命周期合法迁移。
- 非法事件不破坏状态。
- generation 过滤。
- load 后旧轨道清空。
- EOF、stop、error 后状态一致。

### Policies

- 恢复位置阈值。
- 接近结尾不恢复。
- 播放结束清理历史位置。
- 自动下一项各种 repeat/shuffle 模式。

### Playlist

- 添加、删除、重排。
- 删除当前项。
- 重复媒体 identity。
- shuffle 稳定性。
- 失效媒体处理。

### Settings

- 合法范围。
- 迁移。
- 即时应用/下次应用/重启分类。

## 68. libmpv 集成测试

必须使用真实 libmpv，不使用 mock 代替完整链路：

- 初始化和正常关闭。
- 加载短媒体。
- pause/play。
- seek。
- track list。
- 外挂字幕。
- command reply request ID。
- 加载失败。
- 快速连续 load。
- EOF。

Mock 仅用于 reducer、policy、application workflow 的隔离测试。

## 69. Render 集成测试

- 创建 OpenGL context。
- 创建 render context。
- 渲染到 FBO。
- 非 1.0 DPR。
- resize。
- 最小化/恢复。
- 隐藏/显示。
- 多次重建 FBO。
- 关闭时无并发 render。

## 70. QML 测试

- 控件 Hover/Pressed/Disabled。
- Timeline scrub 流程。
- 控制栏自动显隐。
- 弹层打开时不自动隐藏鼠标。
- Playlist delegate 选中/当前/hover 分离。
- 键盘焦点顺序。
- 高对比度主题。
- 长文本、空列表、错误态。

## 71. 手工媒体矩阵

至少覆盖：

- H.264 1080p。
- H.265 4K。
- AV1。
- 10-bit 视频。
- 高帧率视频。
- 纯音频。
- 多音轨 MKV。
- 多字幕轨。
- 外挂 SRT/ASS。
- 可 Seek HTTP。
- 不可 Seek live stream。
- 损坏文件。
- 无权限文件。
- 极短文件。
- 超长 duration 元数据。

测试媒体必须使用可合法分发的小型 fixture，或在 manifest 中记录本地不可提交样本。

---

# 第二十部分：构建、CI、打包与发布

## 72. 构建预设

建议预设：

- `windows-msvc-debug`
- `windows-msvc-release`
- `windows-msvc-asan`
- `windows-msvc-ci`

每个预设必须固定：

- generator。
- architecture。
- build type。
- 所有第三方路径均直接从仓库根目录使用 `../` 定位。
- Qt 路径来源：`../Qt/<version>/msvc2022_64` 自动发现。
- libmpv 路径来源：`../libmpv/<platform>`。
- FetchContent 共享缓存来源：`../cache/cmake/fetchcontent`。
- 测试开关。
- 静态检查开关。

## 73. CI 阶段

1. 配置。
2. 编译。
3. 单元测试。
4. 无 UI 集成测试。
5. QML 测试。
6. 静态检查。
7. 打包 smoke。
8. 产物清单和依赖清单。

任何硬性阶段失败不得继续标记发布成功。

## 74. Windows 打包内容

- 主程序。
- Qt runtime。
- QML modules。
- libmpv DLL。
- FFmpeg/相关运行时 DLL。
- MSVC runtime，按分发策略。
- 图标、翻译、内置 shader。
- 许可证。
- 默认配置。

打包验证：

- 不依赖开发机 PATH。
- 不依赖 Qt 安装目录。
- 不依赖系统已有 mpv。
- DLL 路径可诊断。
- 卸载不删除用户媒体和非应用拥有文件。

## 75. 版本与发布

- 应用版本来自唯一 CMake/version 配置。
- README 记录实际完成内容和验证。
- 不创建平行 CHANGELOG。
- 依赖版本和许可证随产物保存。
- 发布前生成 SHA-256。
- 正式更新机制不是 MVP 必需；不得在没有签名和回滚设计时半成品接入自动更新。

---

# 第二十一部分：Atomic Task 详细执行计划

以下每个 Atomic Task 必须独立实施、独立验证、独立记录。不得跨阶段混改。

## Stage R0：项目定义与不可逆决策

### R0-01 建立治理与仓库基线

**目标**：建立可执行项目根结构。  
**创建/修改**：根规则文件、README、顶层 CMake、docs/plans。  
**实施**：

- 初始化 Git。
- 放置规则文件。
- 建立 README 基础段落和 Change Log。
- 导入本任务书到 `docs/plans/`。
- 设置 `.gitignore`，排除构建目录、用户配置、日志、数据库、IDE 文件。

**验收**：干净仓库、规则可读、README 存在、无生成物入库。

### R0-02 冻结 MVP 功能边界

**输出**：README 的 Scope 段落和验收矩阵。  
**禁止**：开始编码未确认的在线解析、插件市场等能力。  
**验收**：每个 MVP 功能有可观察验收条件。

### R0-03 许可证与依赖决策

**内容**：

- Qt 开源/商业使用方式。
- libmpv 构建来源和许可证。
- FFmpeg 组件。
- 动态链接策略。
- 第三方通知。

**验收**：`LICENSES/README.md` 能说明每个运行时依赖来源和分发义务。

### R0-04 工具链矩阵与共享依赖目录

**输出**：

- Windows 版本。
- MSVC 版本。
- Qt patch。
- CMake/Ninja。
- libmpv 版本。
- 仓库上一层 `..` 直接承载第三方包的工作区布局。
- Qt、libmpv、portable tools、downloads、FetchContent cache 的唯一相对目录。
- 所有依赖路径必须以 `../` 开始，不允许命名总依赖目录或提交绝对路径。

**验收**：

- 开发机和 CI 使用同一版本清单。
- 删除仓库内 `build/` 后重新配置不会重新下载共享依赖。
- 同级第二个仓库可以复用同一个 Qt、libmpv 和依赖缓存。
- Git 暂存区不包含第三方 SDK、下载包和共享缓存。

### R0-05 渲染 ADR

记录 OpenGL 首发、QQuickFramebufferObject、线程和销毁顺序。  
**验收**：没有未决的“使用 wid 还是 Render API”歧义。

### R0-06 测试媒体策略

创建 fixture policy，确定可提交和不可提交媒体。  
**验收**：所有后续播放测试有合法样本来源。

---

## Stage R1：构建骨架与应用启动

### R1-01 CMake 模块化骨架

- 顶层只 add_subdirectory。
- 建立 `cmake/*.cmake`。
- 建立 `src/CMakeLists.txt` 和 `tests/CMakeLists.txt`。

**验证**：空应用 configure/build 成功。

### R1-02 RuntimePaths

- 配置目录。
- 数据目录。
- 日志目录。
- 截图目录。

**测试**：便携模式和安装模式路径行为。

### R1-03 日志系统

- categories。
- 文件 sink。
- 轮转。
- 脱敏。

**验证**：多线程日志、关闭 flush、目录不可写错误。

### R1-04 GraphicsBackendBootstrap

- 强制 OpenGL。
- 验证 OpenGL context。
- 记录实际 renderer。

**验证**：不支持环境给出启动错误，不进入黑屏主窗口。

### R1-05 Composition Root

- 创建 application container。
- 明确所有权和销毁顺序。

**验证**：启动和退出无泄漏、无双重析构。

### R1-06 QML 最小壳

- `App.qml`。
- `MainWindow.qml`。
- 空 `PlayerScreen.qml`。

**验证**：QML 加载错误可诊断，窗口正常显示。

---

## Stage R2：无 UI libmpv 播放核心

### R2-01 FindLibMpv 与运行时探测

- CMake 导入目标。
- 运行时版本记录。
- 缺 DLL 错误。

**验证**：开发和打包路径都能加载。

### R2-02 MpvHandle RAII

- create/destroy。
- 非复制。
- 生命周期断言。

**测试**：创建失败、重复关闭、异常路径。

### R2-03 初始化 profile

- no-config 或独立 config-dir。
- 日志回调。
- 默认 option。

**验证**：用户系统 mpv.conf 不改变产品默认行为。

### R2-04 Wakeup bridge 和事件循环

- wakeup callback。
- Qt thread 唤醒。
- drain events。

**验证**：持续播放事件队列不堵塞。

### R2-05 命令 encoder/executor

- loadfile。
- play/pause/stop。
- seek。
- volume/mute/speed。

**验证**：全部使用异步请求并收到 reply。

### R2-06 Property registry/observer

注册核心属性。  
**验证**：属性格式错误不会导致未定义读取。

### R2-07 Event decoder

- start-file。
- file-loaded。
- end-file。
- command-reply。
- property-change。
- log-message。
- shutdown。

**验证**：未知事件被记录但不崩溃。

### R2-08 控制台 playback probe

建立 `tools/playback_probe`，验证无 UI 播放链路。  
**验收**：文件加载、播放、暂停、seek、结束、关闭全部可观察。

---

## Stage R3：领域状态与 PlaybackSession

### R3-01 Command/Event 类型

建立类型化命令与事件。  
**测试**：构造验证和不可变性。

### R3-02 PlaybackSnapshot

定义全部核心状态和 generation。  
**验收**：UI 所需状态不必查询 mpv 原始属性。

### R3-03 Reducer

实现纯状态转换。  
**测试**：完整状态机矩阵。

### R3-04 Invariants

开发期断言与生产诊断。  
**验证**：非法组合被拒绝或纠正并记录。

### R3-05 PlaybackSession thread

- 独立线程。
- command queue。
- event dispatch。
- state update。

**验证**：GUI 线程不执行 mpv wait/command。

### R3-06 Request tracker

request ID + generation。  
**测试**：旧 reply、超时、关闭取消。

### R3-07 State publisher

- 关键状态即时。
- position 节流。

**验证**：UI 不超过目标刷新频率，暂停/错误无延迟。

### R3-08 Session shutdown

严格关闭流程。  
**验证**：循环创建销毁 100 次。

---

## Stage R4：libmpv OpenGL Render API

### R4-01 OpenGL proc resolver

**验证**：必需函数可解析，缺失时明确失败。

### R4-02 MpvRenderContext RAII

**验证**：创建、render、free 顺序。

### R4-03 Render update bridge

**验证**：callback 只触发 update，不跨线程访问 QML。

### R4-04 MpvVideoItem

- QML item。
- 尺寸/DPR/可见性同步。

**验证**：QML 中可显示占位和视频区域。

### R4-05 MpvVideoRenderer

- FBO。
- render params。
- Y flip。

**验收**：实际视频渲染正确，无上下颠倒。

### R4-06 Resize 和 DPI

**测试**：窗口 resize、多 DPI、全屏。

### R4-07 最小化/恢复与不可见

**验证**：不出现 render storm、黑屏或上下文错误。

### R4-08 Render shutdown race

**验证**：关闭时 callback、render 和 context free 无竞态。

### R4-09 Render 性能基线

记录 1080p/4K 帧时间、CPU/GPU、丢帧。  
**禁止**：未测量就进行无依据优化。

---

## Stage R5：UI 设计系统

### R5-01 QML module 和 import 边界

**验收**：Feature 只通过公开模块引用 controls/theme。

### R5-02 Color/Typography/Spacing tokens

**验收**：核心页面无散落硬编码颜色和尺寸。

### R5-03 Motion/Radius/Elevation tokens

**验收**：同类动画统一，可关闭减少动态效果。

### R5-04 图标管线

- SVG 规范。
- 命名。
- 状态颜色。
- 打包验证。

### R5-05 Typography primitives

**验收**：标题、正文、说明、时间码层级清晰。

### R5-06 Button controls

**测试**：hover、press、disabled、focus、tooltip。

### R5-07 Slider controls

**测试**：键盘、鼠标、触摸板滚轮、精度。

### R5-08 Surface controls

**验证**：panel、drawer、popover、HUD 的层级和阴影一致。

### R5-09 Feedback controls

**验证**：Toast、Error、Loading、Empty 不混用。

### R5-10 可访问性基线

**验证**：键盘焦点链和 accessible name。

---

## Stage R6：播放器主界面与基础交互

### R6-01 PlayerScreen 组合骨架

只组合 viewport、overlay、drawer。  
**验收**：文件职责符合本任务书。

### R6-02 视频 viewport

- MpvVideoItem。
- aspect fit。
- 空画面背景。

### R6-03 顶部区域

- 标题。
- 窗口按钮。
- 精简元数据。

### R6-04 底部控制区域

组合 transport、timeline、volume、actions。  
**禁止**：把各组件实现内联在一个文件。

### R6-05 Transport ViewModel 和 Controls

**验证**：按钮 enable 状态与 selectors 一致。

### R6-06 Timeline ViewModel 和 scrub session

**验证**：拖动不抖动、取消恢复、commit 一次 Seek。

### R6-07 Volume

**验证**：slider、wheel、mute 恢复和 HUD 一致。

### R6-08 Fullscreen

**验证**：进入/退出、Esc、双击、窗口恢复几何。

### R6-09 控制栏自动隐藏

**验证**：菜单、拖动、错误、暂停策略下不误隐藏。

### R6-10 鼠标隐藏

**验证**：与控制栏、弹窗和触摸交互协调。

### R6-11 状态 overlay

Loading、Buffering、Ended、Error。  
**验收**：状态互斥和优先级明确。

### R6-12 HUD 消息队列

音量、Seek、倍速、轨道切换。  
**验收**：消息合并、超时、优先级。

---

## Stage R7：媒体打开与播放列表

### R7-01 本地文件选择

**验证**：取消、无权限、失效路径。

### R7-02 拖放

**验证**：单文件、多文件、目录、非媒体、URL。

### R7-03 URL 打开

**验证**：scheme、空值、不可达、live stream。

### R7-04 命令行和文件关联入口

所有入口汇入 `MediaOpenCoordinator`。  
**禁止**：不同入口各自直接 load mpv。

### R7-05 Playlist Domain

**测试**：增删、重排、当前项、重复模式。

### R7-06 Playlist Controller

**验收**：所有修改命令化，model 只读。

### R7-07 Playlist QML

**验证**：大列表基础性能、状态区分、空态。

### R7-08 自动下一项

**测试**：EOF、错误跳过、repeat one/all、shuffle。

### R7-09 删除当前项

明确停止、切换或保持策略。  
**验收**：不存在悬空 current ID。

---

## Stage R8：音轨、字幕、章节

### R8-01 Track decoder

**验证**：缺字段、外部轨、默认轨、语言。

### R8-02 Track list models

**验证**：重载媒体后旧模型完全替换。

### R8-03 轨道选择

**验证**：关闭字幕、无音轨、选择失败。

### R8-04 外挂字幕

**验证**：SRT、ASS、编码错误、重复加载。

### R8-05 字幕延迟

**验证**：正负值、重置、HUD。

### R8-06 音频延迟

同上。

### R8-07 Chapter decoder/model

**验证**：无章节、重复时间、长标题。

### R8-08 Chapter UI

**验收**：点击章节产生明确 Seek，不直接改 timeline thumb。

---

## Stage R9：历史、恢复播放与设置

### R9-01 Database worker

**验证**：线程连接、关闭 flush、写失败。

### R9-02 Migration 001

创建 schema。  
**验证**：新库、重复启动、失败回滚。

### R9-03 History repository

**验证**：upsert、查询、清理、损坏行。

### R9-04 Progress persistence workflow

**验证**：节流、暂停、停止、关闭、接近结尾。

### R9-05 Resume policy/workflow

**验证**：短片、完成片、远离结尾、用户取消。

### R9-06 Recent media

**验收**：失效路径可识别，不自动删除历史。

### R9-07 Settings repository

**验证**：默认值、损坏 JSON、版本迁移。

### R9-08 Settings Controller

**验证**：即时/下次媒体/重启分类。

### R9-09 Settings UI

**验收**：每个设置组件单一职责，错误和恢复默认明确。

---

## Stage R10：快捷键与 Windows 平台能力

### R10-01 Action Registry

**验收**：UI 菜单、快捷键、媒体键使用同一 ActionId。

### R10-02 Shortcut Dispatcher

**验证**：上下文和文本输入不冲突。

### R10-03 Shortcut Editor

**验证**：冲突提示、恢复默认、非法组合。

### R10-04 无边框窗口与 hit test

**验证**：拖动、resize、最大化、DPI、Snap。

### R10-05 DWM 材质

**验证**：支持时启用，不支持时正确降级。

### R10-06 媒体键

**验证**：后台窗口、重复按键、关闭释放。

### R10-07 休眠抑制

**验证**：播放视频时启用，暂停/停止/退出后释放。

### R10-08 单实例

**验证**：第二实例发送多个文件和 URL。

### R10-09 文件关联

**验证**：安装、打开、升级、卸载后的注册状态。

---

## Stage R11：高级播放能力

每项独立推进，不允许一次性混入 PlayerScreen。

### R11-01 截图

### R11-02 A-B 循环

### R11-03 画面比例

### R11-04 旋转与裁剪

### R11-05 倍速预设和精细调节

### R11-06 字幕样式

### R11-07 播放质量预设

### R11-08 Shader 管理

### R11-09 迷你播放器

### R11-10 画中画可行性与平台实现

每项必须：

- 先新增领域命令/模型。
- 再新增应用工作流。
- 再适配 mpv。
- 再创建 Feature QML。
- 最后挂入 screen。
- 不得直接在页面中调用 mpv property。

---

## Stage R12：可靠性与性能硬化

### R12-01 快速媒体切换压力测试

### R12-02 100 次创建销毁测试

### R12-03 损坏媒体矩阵

### R12-04 网络超时和断流

### R12-05 4K/高帧率性能

### R12-06 长时间播放

### R12-07 内存与资源泄漏

### R12-08 DPI、多屏、全屏切换

### R12-09 数据库故障

### R12-10 配置损坏恢复

### R12-11 诊断包导出

### R12-12 最终 diff 和依赖方向审查

---

## Stage R13：Windows 发布

### R13-01 Release 构建锁定

### R13-02 部署 Qt/QML

### R13-03 部署 libmpv 和运行库

### R13-04 许可证与 notices

### R13-05 安装器

### R13-06 干净环境 smoke

### R13-07 升级安装测试

### R13-08 卸载测试

### R13-09 发布验收矩阵

### R13-10 版本标记与 README 记录

---

## Stage R14：macOS 与 Linux 适配

必须分别开阶段，不允许用平台条件分支污染 Windows 文件。

### macOS

- AppKit 窗口适配。
- 原生 vibrancy。
- 交通灯和标题栏。
- 系统媒体键。
- IOPMAssertion 休眠抑制。
- 文件关联。
- 签名、公证、打包。

### Linux

- X11/Wayland 窗口差异。
- MPRIS 媒体控制。
- systemd-inhibit/桌面接口。
- desktop file 和 MIME。
- AppImage/Flatpak 或发行包策略。

---

# 第二十二部分：新增想法的归属决策流程

用户未来提出任何新想法时，必须执行以下顺序：

1. 描述用户可观察行为。
2. 判断是否属于已有领域能力。
3. 确定状态唯一拥有者。
4. 确定副作用边界。
5. 确定 UI 归属：primitive、control、surface、feature、screen 或 shell。
6. 检查现有文件是否仍然只有一个变更原因。
7. 如果出现第二职责，把单文件升级为目录模块。
8. 定义接口、错误和生命周期。
9. 定义最小测试。
10. 定义阶段回归。
11. 更新 README 实际变更。

## 76. 未来能力归属地图

| 未来想法 | 预计顶层模块 | 不能放入 |
|---|---|---|
| 在线字幕搜索 | `subtitles/search` 新模块 | `PlayerScreen.qml`、`MpvClient` |
| 媒体库 | `library` 新顶层模块 | `playlist`、`history` |
| DLNA 投屏 | `casting` 新顶层模块 | `platform` 通用窗口模块 |
| 在线站点解析 | `sources` 或 `resolvers` 新模块 | `media/url_media_validator` |
| 云同步 | `sync` 新顶层模块 | `persistence/sqlite_*` |
| 插件系统 | 单独立项后 `extensions` | 任何现有 feature 临时脚本入口 |
| 视频滤镜 UI | `video_filters` | `settings` 通用面板 |
| 音频均衡器 | `audio_processing` | `tracks` |
| 弹幕 | `danmaku` | `subtitles`，两者生命周期与数据源不同 |
| 缩略图预览 | `timeline_preview` | `PlayerTimeline.qml` 内联生成线程 |
| 媒体信息面板 | `media_info` feature | `MediaTitle.qml` |
| 自动更新 | `updates` | `ApplicationBootstrap` 直接联网 |

---

# 第二十三部分：模块拆分触发器

## 77. 必须拆分的信号

- 文件开始同时拥有业务状态和 IO。
- QML 文件开始同时包含页面布局、复杂控件实现和业务请求。
- 一个 C++ 文件出现多个不同外部依赖。
- 一个类同时管理 mpv、数据库和窗口。
- 新功能要求独立错误码或配置。
- 新功能需要独立生命周期或线程。
- 单元测试必须构造大量不相关依赖才能测试一个行为。
- 同一文件反复增加按模式、来源、平台、功能类型分支。
- 一个 change 经常误伤文件中的另一功能。

## 78. 不应拆分的情况

- 仅因为行数超过某个数字。
- 一个私有算法由多个紧密配合的小函数组成。
- 新文件只包含一个无语义的转发方法。
- 拆分会复制状态或造成双向依赖。
- 仅为了目录看起来更“丰富”。

## 79. 文件规模预警而非硬限制

建议审查阈值：

- C++ `.cpp` 超过约 400–600 行时必须审查职责，但不自动拆分。
- C++ header 超过约 200–300 行时审查公开面是否过宽。
- Feature QML 超过约 250–350 行时审查是否混入多个子控件。
- Screen QML 超过约 150–250 行时审查是否不再只是布局组合。

最终决定依据永远是职责和变更原因，不是行数。

---

# 第二十四部分：每个 Atomic Task 的提交模板

## 80. 开始前

```text
Task ID:
目标:
当前现象/基线:
相关入口:
直接相关文件:
上下游影响:
保持不变的行为:
计划创建/修改文件:
验证计划:
风险:
```

## 81. 完成后 README 记录

```markdown
### YYYY-MM-DD — <Task ID> <结果标题>

- 实现：<实际完成内容>
- 影响：<模块、接口、行为>
- 兼容性：<无变化/具体变化>
- 验证：<实际执行命令与结果>
- 未验证：<原因和剩余风险；没有则写无>
```

## 82. 完成回复

对话中仅说明：

- 完成结果。
- 验证结果。
- 实际限制或剩余风险。
- 用户必须执行的下一步。

详细文件和验证记录统一进入 README。

---

# 第二十五部分：正式发布前最终检查表

## 83. 架构

- [ ] QML 不直接调用 libmpv。
- [ ] domain 不包含 mpv、SQL、QML、Win32。
- [ ] 播放状态只有一个权威拥有者。
- [ ] playlist 和 playback 没有重复当前状态。
- [ ] render 线程不访问 QML 对象和数据库。
- [ ] 所有平台代码位于对应平台目录。
- [ ] 没有 `utils/helpers/common/misc` 垃圾文件。
- [ ] 没有 Old/New/V2/Final 副本。
- [ ] 新职责已升级为目录模块。
- [ ] 没有空壳接口和无意义转发层。

## 84. 播放

- [ ] 本地文件。
- [ ] URL。
- [ ] 播放/暂停/停止。
- [ ] Seek。
- [ ] 音量/静音。
- [ ] 倍速。
- [ ] 全屏。
- [ ] 轨道。
- [ ] 字幕。
- [ ] 章节。
- [ ] 播放列表。
- [ ] 自动下一项。
- [ ] EOF。
- [ ] 错误处理。
- [ ] 恢复播放。

## 85. UI

- [ ] 视频视觉优先。
- [ ] 控制栏自动隐藏正确。
- [ ] 所有颜色和尺寸来自 token。
- [ ] 当前项/选中项/Hover 项区分。
- [ ] 加载、空、错误、禁用状态齐全。
- [ ] 键盘焦点清晰。
- [ ] 长文本和小窗口布局正确。
- [ ] 高 DPI 正确。
- [ ] 动效可减少。

## 86. 生命周期

- [ ] 快速切换媒体无旧事件污染。
- [ ] Render Context 先于 mpv handle 释放。
- [ ] 关闭后无 callback 访问已销毁对象。
- [ ] 数据库正常 flush。
- [ ] 休眠抑制释放。
- [ ] 媒体键注销。
- [ ] 单实例资源释放。

## 87. 发布

- [ ] 干净环境安装。
- [ ] 不依赖开发机 PATH。
- [ ] 许可证齐全。
- [ ] 文件关联正确。
- [ ] 升级和卸载正确。
- [ ] 版本信息一致。
- [ ] README 记录真实验证。
- [ ] 无临时日志、调试开关和测试资源泄漏进生产包。

---

# 第二十六部分：官方技术依据

以下官方资料是本任务书关键技术决策的依据，实施时应以锁定版本对应文档为准：

1. Qt Quick Scene Graph：  
   https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html
2. QQuickFramebufferObject：  
   https://doc.qt.io/qt-6/qquickframebufferobject.html
3. Qt 6 中 QQuickFramebufferObject 的 OpenGL 限制：  
   https://doc.qt.io/qt-6/quick-changes-qt6.html
4. QQuickRhiItem：  
   https://doc.qt.io/qt-6/qquickrhiitem.html
5. mpv 稳定版手册：  
   https://mpv.io/manual/stable/
6. mpv client API：  
   https://github.com/mpv-player/mpv/blob/master/include/mpv/client.h
7. mpv render API：  
   https://github.com/mpv-player/mpv/blob/master/include/mpv/render.h
8. mpv OpenGL render API：  
   https://github.com/mpv-player/mpv/blob/master/include/mpv/render_gl.h
9. libmpv 官方示例：  
   https://github.com/mpv-player/mpv-examples/tree/master/libmpv

---

# 结论

该项目应按以下主线实施：

```text
产品边界与许可证
→ 构建和生命周期骨架
→ 无 UI libmpv 核心
→ 类型化命令/事件/状态
→ OpenGL Render API
→ QML 设计系统
→ 主播放器闭环
→ 播放列表/轨道/字幕/章节
→ 历史/设置/快捷键
→ Windows 原生能力
→ 可靠性与性能硬化
→ 安装包与正式发布
→ macOS/Linux 独立适配阶段
```

最重要的长期约束：

> 新功能必须先确定责任归属和状态所有者。现有单文件一旦需要承载第二项独立职责，就升级为目录模块，把原职责文件和新增职责文件放入同一模块目录，禁止继续向原文件堆积，也禁止用复制旧文件的方式伪造模块化。
