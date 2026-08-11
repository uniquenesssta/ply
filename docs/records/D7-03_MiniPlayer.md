# D7-03 Mini Player

> Stage：D7 窗口模式与响应式行为
> Task：D7-03 Mini Player
> Scope：Figma 设计契约与验证；不代表 Qt/QML runtime 已实现。

## Figma

```text
Page                     382:5777  07 Window Modes
Source / Contract        401:47    D7-03 / Mini Player
Verification             401:48    D7-03 / Verification
Geometry Contract        402:61    Mini / Geometry Contract
Rest Source              402:70    Mini / Rest · 420×236
Hover Source             402:76    Mini / Hover · 420×236
Minimum Hover Source     402:100   Mini / Minimum Hover · 320×180
Hover Density Contract   402:124   Mini / Hover Density Contract
Shared Authority         402:134   Mini / Shared Authority
Acceptance Gate          402:143   D7-03 / Acceptance Gate
Verification Rest        403:49
Verification Hover       403:58
Verification Minimum     403:85
Always-on-top Context    403:112
Verification Assertions  403:151
```

## Contract

Mini 是同一播放器的独立 Window Mode，不是第二播放器，也不是主窗口等比缩放。

```text
mode=mini
  → compact host
  → video first
  → hover reveals essential controls
  → shared playback / seek truth
```

Mini 只保留：

```text
Video
Title
Play / Pause
Timeline
Expand
Close
```

Mini 明确不提供：

```text
Inspector
Playlist
Tracks
Subtitles
Chapters
Volume
More
Minimize
Pin / Lock UI
```

完整 Playlist / Tracks / Subtitles / Chapters 能力必须先 Expand 回 Windowed，再进入既有 D4 Inspector；Mini 不携带隐藏的 off-canvas Inspector。

`always-on-top` 是 Window flag，不增加 Pin/Lock 按钮或 badge，也不把窗口 flag 混入 playback state。

## Geometry

```text
Default Mini   420×236
Minimum Mini   320×180
Window radius  32

Hover top pods     40px
Hover bottom pod   56px
PlayPause          40×40
Timeline hit       16px
Timeline track      3px
Timeline thumb     10px
```

不通过缩小交互目标解决密度。Minimum 仍保留 40×40 PlayPause、16px Timeline hit target 与 10px thumb；低优先级能力直接不进入 Mini。

Video Viewport 继续消费 D2 的 `Fit + preserve aspect + centered + no stretch + no crop` 语义。

## Density / ownership

- Rest：只显示视频；不显示 Mini title/action/control chrome。
- Hover：原位显示 Title、Expand、Close、PlayPause、Timeline。
- Hover 只改变信息密度，不改变 playback truth。
- PlayPause state / geometry 继续归 D3-04。
- Timeline geometry / seek truth 继续归 D3-02 / D3-03。
- Window surface / elevation / visual language 继续归 D2 / D1。
- Mode transition、Focus 与 Popup cleanup 留给 D7-05，不在 D7-03 创建提前的跨模式迁移状态机。

## Verification

```text
Rest              420×236  Chrome 0
Hover             420×236  Title 1 / Actions 1 / Controls 1
Minimum Hover     320×180  long title truncation / no overlap
Always-on-top     Mini overlaps Windowed context and is above it in z-order
```

Default / Minimum Hover 均验证：

```text
Expand    1
Close     1
Minimize  0
Pin       0
Inspector 0
Playlist  0
Tracks    0
Subtitles 0
Chapters  0
Volume    0
More      0
```

PlayPause：`40×40`。
Timeline：`hit=16 / track=3 / thumb=10×10`。

Final structural audit：

```text
Source Section fit                  PASS
Verification Section fit            PASS
Visible unbound product paints      0
Generic unnamed residues            0
D7-03 Reactions                     0
D7-03 AFTER_TIMEOUT owners          0
D7-03 formal Components             0
New Mini global Variables           0
Section outline residue             0
Always-on-top overlap               PASS
Mini above Windowed context         PASS
D7-01 protected geometry            PASS
D7-02 protected geometry            PASS
Acceptance Gate                     9 / 9 PASS
Verification Assertions             10 / 10 PASS
```

Synthetic `Media Test / Dark Scene` 使用 D7-01 既有测试背景，仅作为 Verification 媒体内容，不计入产品 Semantic Paint 审计；产品 UI visible unbound paint=`0`。

实施中最终只清理了 D7-03 本地复制的 Pause SVG 默认 `Vector` 子路径名，改为 `Pause Bar / Left` / `Pause Bar / Right`；D3 source 未修改。

一次最终只读审计因 JavaScript 局部变量误用保留字导致 SyntaxError，被 Figma 原子拒绝，未提交任何画布变化；修正变量名后同一审计通过。

## Runtime boundary

当前 `agent/r4-stage` 仍是早期：

```text
PlayerScreen
  → VideoSurface
  → PlayerChrome
```

尚无 Mini Window runtime owner、always-on-top window flag 实现或 mode transition runtime。因此 D7-03 完成的是 Figma 设计契约，不将其描述为源码功能已实现。

本任务没有修改播放器源码、配置、依赖、数据格式或运行时接口；无构建、单元测试或运行时测试项。

**D7-03：Complete。**
