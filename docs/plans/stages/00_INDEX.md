# 第三版 Airy Glass UI 设计任务书索引

## 这套任务书是什么

这是**独立的第三版 UI/UX 设计任务书**。

它只学习原仓库 `docs/plans` 与 `docs/plans/stages` 的任务书写法，包括：

- 阶段目标；
- 目标结构；
- 主链；
- 状态/资源归属；
- Atomic Task；
- 实施重点；
- 基本验证；
- 完成判断；
- 阶段优先修复；
- 最小里程碑；
- 关闭条件；
- 暂不要求；
- 后续扩展位置。

**它不整合、不替代、不重排现有 R2–R14 源码开发任务。**

## 设计基准

当前第三版已确认的视觉方向：

- 浅雾；
- 柔焦；
- 柔紫蓝；
- 暖光；
- 半透明玻璃；
- 强留白；
- 悬浮式控件；
- 现代 Grotesk；
- 高级极简；
- 视频优先。

产品结构按 Window / OSC / Inspector / Overlay / Preferences / Window Modes / Design System 组织。

## 主任务书

- [Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md](../Qt6-libmpv播放器-第三版AiryGlass-完整UI设计任务书.md)

## 阶段文件

- [D0_视觉基准与设计治理.md](D0_视觉基准与设计治理.md)
- [D1_设计基础与变量系统.md](D1_设计基础与变量系统.md)
- [D2_播放器窗口与视频视口.md](D2_播放器窗口与视频视口.md)
- [D3_OSC与时间轴控制系统.md](D3_OSC与时间轴控制系统.md)
- [D4_Inspector系统与媒体内部结构.md](D4_Inspector系统与媒体内部结构.md)
- [D5_播放状态与反馈系统.md](D5_播放状态与反馈系统.md)
- [D6_Preferences与快捷键窗口.md](D6_Preferences与快捷键窗口.md)
- [D7_窗口模式与响应式行为.md](D7_窗口模式与响应式行为.md)
- [D8_组件系统与全局收口.md](D8_组件系统与全局收口.md)
- [D9_原型交付与最终验收.md](D9_原型交付与最终验收.md)

## 推荐执行方式

当前已完成并确认的三张核心框架视为 D0 的视觉基准输入。

后续建议从 D1 开始：

```text
D1 Token / Foundation
→ D2 Player Window
→ D3 OSC / Timeline
→ D4 Inspector
→ D5 States / Feedback
→ D6 Preferences
→ D7 Window Modes
→ D8 Design System 收口
→ D9 Prototype / Handoff
```

若在执行过程中发现某个视觉基准需要根本改变，应回 D0 明确修改，不允许只在某一张成品稿里局部偏离。
