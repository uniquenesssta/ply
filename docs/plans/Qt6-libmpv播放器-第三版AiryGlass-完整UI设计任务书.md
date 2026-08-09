# Qt 6 + libmpv 桌面播放器
# 第三版 Airy Glass UI / UX 完整设计任务书

> 文档性质：独立 UI/UX 设计执行书  
> 技术背景：Qt 6 + Qt Quick/QML + libmpv 桌面播放器  
> 设计对象：播放器产品层，不重新定义播放内核、PlaybackSession、数据库、发布等开发任务  
> 视觉基准：已确认的第三版 Airy Glass 框架  
> 设计方向：浅雾、柔焦、柔紫蓝、半透明玻璃、强留白、轻悬浮、现代 Grotesk、低攻击性配色  
> 设计阶段：D0–D9  
> 重要说明：本文学习原 `docs/plans` 与 `docs/plans/stages` 的任务书写法，但**不整合旧 R2–R14 内容，不重排既有开发阶段，不把播放器误写成网页页面**

---

# 0. 文档用途与执行约束

本任务书用于指导第三版播放器 UI 从视觉基准、设计基础、主窗口、OSC、Inspector、状态、Preferences、窗口模式、组件系统到 Prototype/Handoff 的完整设计过程。

它解决的是“播放器应该长什么样、怎么组织、怎么交互、如何保持一致、如何交给开发实现”，而不是“libmpv 如何初始化、PlaybackSession 如何建线程”等源码开发问题。

每个 Design Atomic Task 开始前必须确认：

1. 当前要解决的产品视觉/交互问题是什么。
2. 当前已确认的第三版框架是什么样。
3. 该任务属于 Window、OSC、Inspector、Overlay、Preferences、Component 还是 Foundation。
4. 是否已经存在可以复用的源组件。
5. 是否会改变全局 Token、组件几何或响应式规则。
6. 若改变全局规则，需要同步回刷哪些已完成画面。
7. 本任务完成后用什么截图、状态矩阵或 Prototype 验证。

## 0.1 Figma Page 不等于播放器产品页面

播放器是桌面应用，产品结构主要由窗口、浮层、控制系统、Inspector 和状态组成。

推荐 Figma Page 只承担设计文件分类：

```text
00 Direction
01 Foundations
02 Components
03 Player Window
04 Inspector
05 States & Feedback
06 Preferences
07 Window Modes
08 Prototype
09 Handoff
```

这些 Page 不代表用户在播放器中会“跳转到 03/04/05 页面”。

真正产品结构：

```text
Player Window
├─ Video Viewport
├─ Floating Header
├─ OSC
├─ Inspector Host
│  ├─ Playlist
│  ├─ Tracks
│  ├─ Subtitles
│  └─ Chapters
└─ Overlay Host
   ├─ Loading / Buffering
   ├─ Seek Preview
   ├─ HUD / Toast
   ├─ Error
   └─ Dialog

Window Modes
├─ Windowed
├─ Fullscreen
└─ Mini

Preferences Window
├─ Playback
├─ Video
├─ Audio
├─ Subtitles
├─ Interface
├─ Advanced
└─ Shortcuts
```

## 0.2 “完全重新设计”的强制定义

第三版只继承产品功能，不继承第一/第二版视觉骨架。

禁止从旧稿复制或变形继承：

- 旧 Player Shell。
- 旧黑蓝配色。
- 大蓝圆播放键。
- 厚重底部控制栏。
- 纯黑 Drawer。
- 后台式卡片分区。
- 旧圆角、间距、阴影、组件比例。
- 旧页面展示排版。

如果一个新稿仅仅把旧结构变浅、变透明、换成紫色，则仍视为未完成“重新设计”。

---

# 第一部分：第三版产品视觉目标

## 1. 总体目标

第三版要建立一个“视频优先、轻盈、柔和、高级、现代”的桌面播放器。

核心感受：

- 视频内容是第一视觉主体。
- UI 像悬浮在视频上的轻薄物体，而不是覆盖画面的厚重面板。
- 浅雾、柔紫蓝、暖光和半透明玻璃共同形成氛围。
- 留白和间距本身承担设计，不靠大量装饰。
- 控件小而精，避免“为了可见就做大”。
- 既有未来感，又不能像炫技概念 UI。
- Preferences、Inspector、Fullscreen、Mini 都必须属于同一产品。

## 2. 视觉关键词

固定使用：

- Airy
- Ethereal
- Soft Glass
- Pastel Glow
- Misty Light
- Luxury Minimal
- Sharp Grotesk
- Editorial Spacing
- Floating Controls
- Cinematic Softness

这些不是宣传词，而是执行条件：

| 关键词 | 产品化含义 |
|---|---|
| Airy | 主画面留白、模块不拥挤、控件不铺满 |
| Ethereal | 轻透明、弱边界、柔焦背景支持 |
| Soft Glass | 半透明表面 + Blur + 弱边界 + 柔阴影 |
| Pastel Glow | 紫/蓝/暖光只做氛围和焦点，不做大色块 |
| Luxury Minimal | 信息少而准，视觉精度高 |
| Sharp Grotesk | 字体利落，标题有现代编辑感 |
| Editorial Spacing | 用间距、字距、对齐构建层级 |
| Floating Controls | Header/OSC/Inspector 是附着层，不是硬工具栏 |
| Cinematic Softness | UI 为视频让路，状态反馈不过度 |

## 3. 禁用视觉

以下均为第三版失败信号：

- 大面积纯黑或深灰底。
- 高饱和蓝色填满主要按钮/列表选中。
- 大圆形主播放键抢占视频中心。
- 底部控制栏占据明显高度。
- Inspector 看起来像后台抽屉。
- Settings 看起来像网页表单或企业管理台。
- 卡片套卡片。
- 强描边、强投影、红色整框错误。
- 每个功能分别发明一套玻璃和圆角。
- 为了“完整”把所有信息同时显示。

---

# 第二部分：设计架构与唯一职责

## 4. Player Window System

唯一职责：决定播放器主窗口的空间结构和 Host。

它不拥有：

- Timeline 内部交互。
- Playlist delegate。
- 字幕选择逻辑。
- Settings 内容。
- Error 文案。

## 5. OSC System

唯一职责：播放器主控制系统。

包含：

- Timeline。
- Transport。
- Volume。
- Utility Actions。
- 显隐状态。

不包含：

- Playlist 内容。
- Tracks 内容。
- Settings。
- Dialog 业务文案。

## 6. Inspector System

唯一职责：统一侧板容器和内容切换。

Shell 拥有：

- 位置。
- 宽度。
- 材质。
- Header。
- Mode switch。
- Content slot。
- Footer。
- 打开/关闭。

内容模块分别拥有：

- Playlist。
- Tracks。
- Subtitles。
- Chapters。

四种内容不能各自再建立新的 Drawer/Shell。

## 7. Feedback System

分层：

```text
HUD       高频、短时、可合并
Toast     非阻断结果
Overlay   当前媒体状态
Dialog    需要用户决策
```

任何一个事件只能有一个主反馈层级。

## 8. Preferences System

独立窗口，和播放器共享设计语言，但不是 Player Window 的 Drawer。

## 9. Window Modes

Windowed / Fullscreen / Mini 共享：

- 播放状态语义。
- Icon。
- Timeline 源组件。
- Transport 源组件。
- Token。

只改变：

- 布局。
- 信息密度。
- 可见功能。
- Host 几何。
- 显隐策略。

---

# 第三部分：Figma 模块化规划

## 10. Design System 分层

```text
Foundations
  ↓
Primitives
  ↓
Controls
  ↓
Surfaces
  ↓
Composite Components
  ↓
Product Features
  ↓
Window / Overlay Composition
```

### Foundations

Color、Typography、Spacing、Radius、Blur、Shadow、Opacity、Motion、Z-order。

### Primitives

Icon、Text、Divider、Focus Ring、Glass Layer 等最小原语。

### Controls

Icon Button、Playback Button、Timeline、Slider、Toggle、Select、Segmented、Search、Keycap。

### Surfaces

OSC、Inspector、Popover、HUD、Toast、Dialog。

### Composite

Floating Header、Playlist Row、Track Row、Settings Row、Source List Item。

### Product Features

Playlist Content、Tracks Content、Subtitles Content、Chapters Content、Settings Sections 等。

## 11. 强制拆分触发器

Figma 组件出现以下任一情况必须拆分或升级 Component Set：

- 同一组件同时承担容器材质和业务内容。
- 同一个 Variant 同时编码 size、state、window mode、feature type，导致组合爆炸。
- 一个组件为了适配第二个 Feature 出现大量“仅此处”隐藏层。
- 同样的列表行在 Playlist/Tracks/Chapters 各自复制。
- OSC 为 Fullscreen 复制第二套 Timeline。
- Inspector 每个 mode 复制 Shell。

## 12. 不应拆分的情况

- 只是为了层级树看起来更丰富。
- 只出现一次且没有独立演进原因。
- 拆分后只是透明转发。
- 会导致 Slot 和父组件耦合更复杂。

---

# 第四部分：设计系统核心规格方向

## 13. Color

颜色使用原则：

- 背景主基调：雾白、冷白、微暖灰。
- 氛围：柔紫、蓝紫、少量天青、暖奶油光。
- 文本：深灰蓝而非纯黑。
- Accent：小面积。
- Selection：柔和，优先透明/淡色。
- Error/Warning：低攻击性，但仍满足辨识和对比。

## 14. Typography

至少建立：

- Display。
- Window Title。
- Section Title。
- Body。
- Label。
- Caption。
- Timecode。
- Keycap。
- Metadata。

播放器控制区域的文字不能沿用 Hero 大标题比例。

## 15. Spacing

至少区分：

- Window outer breathing space。
- Floating region edge distance。
- Surface padding。
- List row horizontal padding。
- Control gap。
- Icon-text gap。
- Section gap。

“完美留白”必须通过规则达成，而不是每张图人工拖位置。

## 16. Glass / Blur / Shadow

材质分层：

- Floating Control Glass。
- Inspector Glass。
- Popover Glass。
- Strong Dialog Glass。
- HUD Glass。

Blur 与 alpha 要配合复杂背景验证。

## 17. Motion

建议语义：

- Instant feedback。
- Hover transition。
- Press feedback。
- OSC enter/exit。
- Inspector open/close。
- Popover。
- Dialog。

必须支持 Reduce Motion。

---

# 第五部分：核心产品系统

## 18. Main Player

构成：

```text
Player Window
├─ Video Viewport
├─ Floating Header
├─ OSC
├─ Optional Inspector
└─ Feedback Layers
```

验收重点：

- 视频第一。
- Header 不像厚工具栏。
- OSC 不铺满。
- Inspector 不重排视频主体。
- 控件光学对齐。
- 复杂背景下可读。

## 19. Timeline

Timeline 必须把视觉轨道和交互命中区分离。

状态至少：

- default。
- hover。
- scrubbing。
- pending seek。
- non-seekable。
- unknown duration。
- chapters。
- buffered。

## 20. Inspector

同一 Shell，四种 Content。

### Playlist

必须区分：

- current playing。
- selected。
- hover。
- keyboard focus。
- invalid/unavailable。

### Tracks

- selected。
- pending。
- off/default。
- audio/video/subtitle 类型。

### Subtitles

- track。
- off。
- external。
- delay。
- limited style quick controls。

### Chapters

- current。
- hover。
- timecode。
- seek action。

## 21. Playback States

至少：

- Empty。
- Loading。
- Playing。
- Paused。
- Buffering。
- Seeking。
- Ended。
- Error。

状态设计保持同一播放器几何。

## 22. Preferences

独立 Window Shell。

要求：

- Source List。
- Content Header。
- Section。
- Row。
- Controls。
- Shortcuts。
- Restart/validation feedback。

## 23. Fullscreen

要求：

- 视频绝对优先。
- OSC 比 Windowed 更轻。
- Header 最小。
- Hidden/Rest/Active 显隐完整。
- Bright/dark video 都可读。

## 24. Mini Player

要求：

- 保留最必要信息。
- 不复制完整主窗口。
- 不提供完整 Inspector。
- 与主播放器共享同一控件源。

---

# 第六部分：状态与交互矩阵

## 25. OSC Visibility

至少处理：

| 场景 | 期望 |
|---|---|
| playing + inactive | Rest → Hidden |
| pointer move | Active |
| paused | 可保持 Rest/Active |
| scrubbing | Locked Visible |
| popup open | Locked Visible |
| dialog open | 控制层按规则退居后层 |
| error overlay | 不抖动 |
| keyboard action | 显示 OSC 或 HUD |
| fullscreen inactive | 更快回 Hidden |

## 26. Inspector

至少处理：

- open。
- close。
- mode switch。
- narrow overlay。
- escape。
- outside click（如采用）。
- popup/dialog coexistence。
- window mode change。

## 27. Feedback

必须避免：

- 同时 Error Overlay + Toast 重复错误。
- Buffering 遮住 Error。
- Paused 状态误认为 Buffering 结束后自动 Playing。
- HUD 队列刷屏。

---

# 第七部分：可访问性与精度

## 28. Accessibility

- 主要文本具备足够对比。
- Focus visible。
- 不能只靠颜色表达 selected/error。
- 控件点击目标不因视觉缩小而缩小。
- Reduce Motion。
- 键盘导航可解释。

## 29. 几何精度

每次视觉 QA 必查：

- 图标光学中心。
- 文本 baseline。
- 控件内部 padding。
- Timeline thumb 与 progress。
- List row 左右起线。
- Header/OSC/Inspector 对齐关系。
- 不同窗口模式的控件尺寸。

---

# 第八部分：Design Atomic Task 总体计划

具体任务见 `stages/`。

```text
D0 视觉基准与设计治理
→ D1 设计基础与变量系统
→ D2 播放器窗口与视频视口
→ D3 OSC 与时间轴控制系统
→ D4 Inspector 系统与媒体内部结构
→ D5 播放状态与反馈系统
→ D6 Preferences 与快捷键窗口
→ D7 窗口模式与响应式行为
→ D8 组件系统与全局收口
→ D9 原型、交付与最终验收
```

---

# 第九部分：新增想法的归属决策流程

用户未来提出任何 UI 新想法，必须按以下顺序判断：

1. 它改变的是窗口结构、控制系统、Inspector、Feedback、Preferences 还是 Design System？
2. 是否已有组件可表达？
3. 是否引入新的独立交互状态？
4. 是否改变全局 Token 或响应式规则？
5. 是否只属于某 Feature？
6. 是否需要回刷 Main/Fullscreen/Mini/Preferences？
7. 是否需要新增 Prototype 验收？
8. 是否需要在 Handoff 更新契约？

禁止直接在当前画面“找个地方塞进去”。

---

# 第十部分：正式交付前最终检查表

## 30. 视觉方向

- [ ] 与第一/第二版一眼不同。
- [ ] 无黑蓝科技风回归。
- [ ] 无大蓝圆播放键。
- [ ] 无厚底栏。
- [ ] 无纯黑 Drawer。
- [ ] 强留白成立。
- [ ] 玻璃轻而可读。

## 31. 产品结构

- [ ] Fullscreen 是 Window Mode，不是第二个独立产品。
- [ ] Mini 共享播放状态和组件。
- [ ] Playlist/Tracks/Subtitles/Chapters 共用 Inspector Shell。
- [ ] HUD/Toast/Overlay/Dialog 层级明确。
- [ ] Preferences 是独立窗口。

## 32. Design System

- [ ] 公共颜色绑定 Semantic Token。
- [ ] 圆角/Blur/Shadow/Motion 有统一来源。
- [ ] 公共控件使用 Component Instance。
- [ ] 无 Detached 关键实例。
- [ ] 无默认 `Frame/Vector` 无语义命名残留。
- [ ] 无 Slot 默认实色背景。
- [ ] 无重复 Timeline/OSC/Inspector Shell。

## 33. 视觉 QA

- [ ] 图标光学居中。
- [ ] 文本 baseline 对齐。
- [ ] 行高一致。
- [ ] 列表起线一致。
- [ ] 明亮视频背景可读。
- [ ] 暗色视频背景可读。
- [ ] 暖色高光背景可读。
- [ ] 窄窗口不重叠。
- [ ] Fullscreen 控制器不过大。
- [ ] Mini 不拥挤。

## 34. Prototype / Handoff

- [ ] Play/Pause 可点击演示。
- [ ] Seek 完整。
- [ ] Inspector 切换完整。
- [ ] Fullscreen/Mini 切换完整。
- [ ] Error recovery 完整。
- [ ] Handoff 数字与源组件一致。
- [ ] 未实现开发能力不在 Handoff 假装已完成。

---

# 结论

第三版设计不再围绕“做多少张页面”，而围绕以下主线：

```text
视觉母语
→ 设计基础
→ Player Window
→ OSC / Timeline
→ Inspector
→ Feedback
→ Preferences
→ Window Modes
→ Component System
→ Prototype / Handoff
```

最重要的长期约束：

> 第三版只继承播放器功能，不继承旧视觉；任何跨场景重复结构必须回到唯一源组件处理；任何全局视觉变化必须全局回刷；播放器始终按窗口、控制系统、Inspector、状态和组件组织，而不是按网页页面组织。
