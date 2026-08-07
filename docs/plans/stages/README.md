# Qt6 + libmpv R2–R14 任务书：快速框架实施版

本目录是已确认采用的后续阶段执行基线。

目标不是把每个 Atomic Task 写成复杂固定合同，而是让后续开发更快：

> 先搭正确框架 → 跑通真实主链 → 继续铺开功能 → 根据真实问题修复 → R12 集中硬化 → R13 发布。

本任务书仍保证：

- 模块职责清晰；
- 不把页面/播放/数据库/平台/Render 堆到一个文件；
- 播放状态、Playlist、Render、DB 等关键资源都有单一 owner；
- 高风险生命周期任务保留必要硬约束；
- R13 最终目标是可以在干净 Windows 上安装使用的真正桌面软件。

## 2026-08-07 成熟播放器行为补强

从 R2-02 起，所有阶段任务还必须同时应用 [成熟播放器行为补强与验收矩阵.md](成熟播放器行为补强与验收矩阵.md)。该补充基于对 Screenbox 当前 `main`（commit `20ff2f5ea0ee1651bdf96ab4224eb0aa428e9962`）的行为与模块边界研究，重点补齐：

- MediaGeneration 与旧媒体事件/异步 reply 隔离；
- PlaybackSnapshot 生命周期、transport、buffering 等状态轴分责；
- Timeline actual/preview 双状态、scrub、pause/buffering/end seek；
- 控制栏和 cursor 自动隐藏策略；
- Queue stable identity、mutation、repeat/shuffle、EOF/Next 幂等；
- Tracks/Subtitles/Chapters 的 generation 生命周期；
- Resume/Progress 的 generation guard；
- UI/快捷键/媒体键统一 Action 语义；
- Mini Player/PiP 复用同一 PlaybackSession；
- R12 成熟播放器行为回归矩阵和 R13 发布门禁。

这份补充只增加行为完整性和验收要求，不改变既定 Qt6 + libmpv 架构，不扩大 MVP，不复制 Screenbox GPL-3.0 源码，也不引入 Screenbox/LibVLCSharp/UWP 依赖。

R2-01 当前实现和验收流程保持原样：继续完成 MSYS2/CLANG64 源码构建与固定 libmpv sibling package 本机验收，不因这次规划更新返工。
