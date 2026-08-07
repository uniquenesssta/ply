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

# Stage R13：Windows 发布

## 1. 本阶段最终要得到什么

把经过 R12 硬化的候选构建成真正可以发给别人安装使用的 Windows 桌面软件。R13 完成的定义不是“编译成功”，而是干净机器安装、启动、播放、升级、卸载都成立。

## 2. 本阶段怎么快速推进

先锁 Release 构建和 staging，再解决运行时依赖；确认目录版能在 clean VM 跑后再做安装器。安装器正常后做升级/卸载，最后才 tag。

## 3. 目标模块结构

```text
packaging/windows/
├─ deploy_qt/
├─ deploy_runtime/
├─ installer/
└─ manifests/
tests/release/
LICENSES/
```

## 4. 阶段主链

```text
Verified Source Commit
  ↓
Exact Release Build
  ↓
Runtime Staging
  ↓
Clean VM Directory Smoke
  ↓
Installer
  ↓
Install/Upgrade/Uninstall Smoke
  ↓
Release Matrix → Tag
```

## 5. 关键状态/资源归属

- 版本身份：Release metadata/Git tag。
- 依赖身份：manifest。
- 用户数据：安装器不得随意拥有/删除。
- 发布是否通过：Release matrix。

## 6. Atomic Tasks

### R13-01 Release 构建锁定

**目的**  
固定最终 release preset、版本和依赖身份。

**主要模块 / 文件**  
release preset/version metadata

**主要链路**  
`verified commit → exact toolchain → clean release build`

**实施重点**
不在 release 阶段偷偷升级依赖。

**基本验证**
clean build。

**完成判断**  
Release 身份唯一。

**高风险约束**
发布必须从干净、已验证提交开始。

### R13-02 部署 Qt/QML

**目的**  
把运行所需 Qt/QML 真正带进 staging。

**主要模块 / 文件**  
packaging/windows/deploy_qt

**主要链路**  
`release exe → deploy Qt/QML → staging`

**实施重点**
先启动，再按依赖扫描删冗余。

**基本验证**
干净环境启动。

**完成判断**  
不依赖开发 Qt。

### R13-03 部署 libmpv 和运行库

**目的**  
带上已审计 libmpv/FFmpeg/MSVC runtime。

**主要模块 / 文件**  
deploy_runtime + manifest

**主要链路**  
`manifest → copy exact runtime → runtime probe`

**实施重点**
拒绝随机 DLL。

**基本验证**
dependency scan + clean VM。

**完成判断**  
运行依赖完整可追溯。

### R13-04 许可证与 notices

**目的**  
按最终二进制依赖生成许可证材料。

**主要模块 / 文件**  
LICENSES/ + notices

**主要链路**  
`binary inventory → license mapping → notices/source obligations`

**实施重点**
以实际产物为准。

**基本验证**
人工+脚本核对。

**完成判断**  
分发义务完整。

### R13-05 安装器

**目的**  
生成 Windows 安装器。

**主要模块 / 文件**  
packaging/windows/installer

**主要链路**  
`staging → installer → install actions`

**实施重点**
保留用户设置/历史；不删除用户媒体。

**基本验证**
安装/修复/卸载。

**完成判断**  
用户能正常安装。

### R13-06 干净环境 smoke

**目的**  
在没有开发环境的 Win10/11 测。

**主要模块 / 文件**  
release smoke tests

**主要链路**  
`clean VM → install → launch → open → play → close`

**实施重点**
这是硬门槛。

**基本验证**
Win10/Win11。

**完成判断**  
基础播放器真正可交付。

**高风险约束**
干净机器无法启动或播放必须停止发布。

### R13-07 升级安装测试

**目的**  
验证上一版升级。

**主要模块 / 文件**  
upgrade matrix

**主要链路**  
`old install/data → new installer → migrate → launch`

**实施重点**
重点 history/settings/schema。

**基本验证**
升级一轮。

**完成判断**  
用户数据不丢。

**高风险约束**
迁移失败不可发布。

### R13-08 卸载测试

**目的**  
验证卸载只清产品拥有内容。

**主要模块 / 文件**  
uninstall checklist

**主要链路**  
`install/use → uninstall → inspect`

**实施重点**
用户数据政策要明确。

**基本验证**
干净 VM。

**完成判断**  
不误删用户媒体/系统项。

### R13-09 发布验收矩阵

**目的**  
把实际证据汇总成发布判断。

**主要模块 / 文件**  
release matrix + README

**主要链路**  
`test evidence → matrix → go/no-go`

**实施重点**
未跑项目不能标绿。

**基本验证**
逐项核对。

**完成判断**  
发布状态透明。

### R13-10 版本标记与 README

**目的**  
最终 tag、hash、README 记录。

**主要模块 / 文件**  
README + Git tag/release metadata

**主要链路**  
`verified commit → tag → artifacts/hashes`

**实施重点**
tag 不得指向未验证 commit。

**基本验证**
tag/hash 复核。

**完成判断**  
正式版本可追溯。

## 7. 框架打通后优先修复

- 漏 Qt plugin/QML module。
- DLL 搜索路径错误。
- 安装器误删用户数据。
- 升级 schema 失败。
- notices 与实际二进制不匹配。

## 8. 本阶段最小可运行里程碑

在无 Qt、无 mpv、无开发 PATH 的干净 Windows 上安装后，可以双击媒体或打开应用并正常播放。

## 9. 阶段关闭条件

clean Win10/11 smoke、升级、卸载、许可证/依赖清单全部完成；发布矩阵无阻断项；tag 指向唯一验证提交。

## 10. 本阶段暂不要求

R13 不新增播放器功能，也不做架构重写。发现功能缺陷回原模块修复并重跑相关发布验证。

## 11. 后续扩展位置

自动更新属于独立 `updates` 模块，发布后再单独立项。
