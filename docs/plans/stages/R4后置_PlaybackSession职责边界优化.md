# R4 后置：PlaybackSession 职责边界优化任务书

> 文档性质：R4 完成后的独立架构优化任务书  
> 执行触发：仅在 Stage R4 已正式关闭后，由用户明确调用时执行  
> 当前状态：Deferred / Not Started  
> 目标对象：`src/playback/application/session/playback_session.*` 及其直接职责边界  
> 重要说明：本任务不属于 R4-04～R4-09，不改变 Stage R4 的完成条件，也不得提前夹带进 Render 主链实施。

---

## 1. 为什么需要这次优化

当前 `PlaybackSession` 的总体架构仍然正确：它是 Playback Application 层的会话协调器，也是 `PlaybackSnapshot` 与 `MediaGeneration` 的唯一权威 owner。

现有代码已经正确拆出了：

- `PlaybackSessionThread`：线程 host 与有界 shutdown；
- `PlaybackSessionBackend`：R2/libmpv infrastructure 适配；
- `RequestTracker` / `RequestTimeoutMonitor` / `RequestSupersessionPolicy`：异步请求生命周期；
- `MediaGenerationGate`：旧媒体代际事件过滤；
- `playback_shutdown.*`：Session 关闭顺序；
- `StatePublisher`：向 GUI 发布快照；
- `PlaybackReducer` / `PlaybackCleanupPolicy`：领域状态转换与清理规则。

因此本任务**不是重写 PlaybackSession，也不是按文件大小机械拆分**。

审计确认的主要积累点是：

1. `shouldApplyBackendEvent()` / `isMediaPropertyLifecycle()` 已形成独立的“事件 × 生命周期 → 是否准入 Reducer”策略；
2. 该策略在新增 Cache / Tracks / Chapters / Stream 信息时持续扩张，已经具有独立变化原因和独立测试价值；
3. Session 内还存在一组错误/失败翻译逻辑，可在完成第一步后判断是否已经达到独立职责边界。

本任务的核心目标是让 `PlaybackSession` 保持为**纯粹的会话 orchestrator**，而不是继续演变成事件策略与错误策略的汇总文件。

---

## 2. 必须保持不变的架构真值

本优化不得改变以下所有权：

```text
PlaybackSession
├─ 唯一拥有 PlaybackSnapshot 当前真值
├─ 唯一分配/推进 MediaGeneration
├─ 决定 Command/Event orchestration 顺序
└─ 唯一执行 authoritative snapshot commit

RequestTracker
└─ 唯一拥有异步请求记录/完成/取消状态

MediaGenerationGate
└─ 唯一负责 generation-scoped stale event 准入

PlaybackReducer
└─ 唯一负责 PlaybackEvent → PlaybackSnapshot 状态转换

PlaybackSessionBackend
└─ 唯一负责 Application → R2/libmpv infrastructure 边界
```

禁止为了“拆文件”引入第二个可写 Snapshot owner、第二个 Generation owner、重复 Request 状态或透明转发 manager。

---

## 3. 目标模块结构

建议目标结构：

```text
src/playback/application/session/
├─ playback_session.*
├─ playback_session_thread.*
├─ playback_shutdown.*
├─ media_generation_gate.*
├─ policies/
│  ├─ playback_event_admission_policy.*
│  └─ playback_failure_policy.*        # 仅在 Atomic 2 审计确认需要时创建
└─ backend/
   ├─ playback_session_backend.*
   └─ mpv_media_generation_attributor.*
```

`policies/` 只允许容纳真正具有独立规则、独立变化原因和独立测试矩阵的 Session 应用层策略。不得把简单 helper 机械搬入该目录。

---

## 4. Atomic 1：提取 Playback Event Admission Policy

### 目的

把 Session 内部已经独立演进的事件生命周期准入矩阵提取为单独策略模块。

### 当前来源

- `shouldApplyBackendEvent()`
- `isMediaPropertyLifecycle()`

### 建议文件

```text
src/playback/application/session/policies/
├─ playback_event_admission_policy.cpp
├─ playback_event_admission_policy.h
└─ CMakeLists.txt
```

### 目标接口语义

策略输入只允许包含判断所需的只读信息，例如：

```text
PlaybackLifecycleState + PlaybackEvent
        ↓
accept / reject
```

策略不得：

- 修改 `PlaybackSnapshot`；
- 修改 `MediaGeneration`；
- 调用 backend；
- 调用 RequestTracker；
- 发 signal；
- 访问 QML/GUI；
- 持有 libmpv 类型。

### Session 收口后的主链

```text
Backend PlaybackEvent
      ↓
CommandReply special handling
      ↓
MediaGenerationGate
      ↓
PlaybackEventAdmissionPolicy
      ↓
PlaybackReducer
      ↓
commitSnapshot()
```

### 必须覆盖的准入矩阵

至少覆盖：

- Pause / Seeking；
- Buffering / BufferingProgress / CacheStatus；
- Position / Duration / Seekable；
- Media title/path；
- Track list 与 selected video/audio/subtitle track；
- Chapter list；
- VideoStreamInfo / AudioStreamInfo；
- 生命周期 `Empty / Opening / Ready / Ended / Failed / Closing` 的适用组合；
- 不受生命周期限制的事件仍保持既有行为。

### 验证

- 新增纯单元测试验证 admission matrix；
- 原 `playback_session_test` 主链保持通过；
- 快速 A→B generation 测试保持通过；
- 不改变任何已有 Event/Snapshot 数据结构。

### 完成判断

`playback_session.cpp` 不再维护具体事件类型与生命周期组合表；新增媒体状态事件时，可以独立修改并测试 admission policy。

---

## 5. Atomic 2：审计并决定 Failure Translation 是否独立

### 目的

判断以下逻辑是否已经形成独立的 Application failure translation policy：

- `makeFailure()`；
- `requestTrackDiagnostic()`；
- LoadMedia submission failure → `MediaFailedEvent`；
- 普通 command submission failure → `PlaybackFailureEvent`；
- failed `CommandReplyEvent` 根据 request type 映射为媒体失败或普通命令失败；
- tracking failure → Protocol category。

### 拆分触发条件

只有满足以下任一条件才创建 `playback_failure_policy.*`：

- failure category / event mapping 已有两个以上独立调用入口；
- 后续 command/request 类型会继续扩展同一映射；
- 可以形成不依赖 Session mutable state 的纯输入→输出规则；
- 单独测试能够显著降低 Session 测试复杂度。

若不满足，则保持在 Session，不为了减少行数创建透明 wrapper。

### 若实施拆分

建议策略只负责：

```text
application failure context
        ↓
PlaybackFailure / PlaybackEvent
```

Session 仍负责：

- 何时调用；
- authoritative `commitSnapshot()`；
- request/generation/backend orchestration。

### 验证

- Load failure 语义不变；
- command failure 语义不变；
- Protocol/Media/Command category 不变；
- 不改变日志等级、错误文本契约或公开 signal。

---

## 6. Atomic 3：PlaybackSession Orchestrator 收口

### 目标

完成前两步后，重新审计 `playback_session.cpp`，只允许保留与会话 orchestrator 同责的功能：

- construction / destruction；
- initialize / shutdown；
- command dispatch；
- media-load orchestration；
- tracked command orchestration；
- backend event orchestration；
- command reply orchestration；
- authoritative Snapshot commit；
- authoritative MediaGeneration allocation；
- owning-thread guard。

### 明确不拆的高价值逻辑

除非后续证据改变，不得机械拆出：

- `beginMediaLoad()`；
- `submitTrackedCommand()`；
- `commitSnapshot()`；
- `allocateMediaGeneration()`。

这些函数需要同时协调 Session 所拥有的真值与多个已独立模块；它们属于 orchestrator 本身，而不是第二职责。

### 禁止结果

不得出现：

- `PlaybackSessionManager` / `Coordinator` / `Helper` 之类仅透明转发的新层；
- 多个模块都能直接写 Snapshot；
- Generation allocation 下沉给 backend；
- RequestTracker 状态复制到 Session policy；
- 为减少单文件大小而一函数一文件；
- 任何 UI/Render/Playlist/Database 逻辑进入 Session。

---

## 7. Atomic 4：完整回归与架构验收

### 最小验证

必须执行任务开始时现有的全部相关测试，而不是固定依赖旧的测试数量。

至少：

1. event admission policy 独立单元测试；
2. request tracker / supersession 测试；
3. media generation gate 测试；
4. playback session 主链测试；
5. playback shutdown 测试；
6. state publisher 测试；
7. reducer / invariant 回归；
8. 当前 Stage R4 已有全部 render tests；
9. configure / build / CTest 全量门禁；
10. Windows 本机真实媒体 load/play/pause/seek/stop smoke。

### 架构验收

必须确认：

- `PlaybackSession` 仍是唯一 Snapshot owner；
- `PlaybackSession` 仍是唯一 Generation allocator；
- QML/GUI 不直接访问 Session mutable state；
- admission policy 为纯判断；
- failure policy（若创建）为纯翻译；
- 没有循环依赖；
- 没有重复状态；
- 没有新增生产依赖；
- public API、数据结构、持久化、配置和用户可观察播放行为保持不变。

---

## 8. 任务边界与非目标

本优化只处理已经确认的 PlaybackSession 职责积累，不顺手重构整个 R2/R3。

明确不包含：

- R4 Render 架构；
- QML Video Item / Renderer；
- Playlist；
- Track/Subtitle/Chapter 产品功能；
- Timeline UI；
- ViewModel；
- 数据库；
- 设置；
- 新功能；
- 新生产依赖；
- public API 改版。

发现与本任务无关的问题时，只记录，不夹带修改；只有确认属于同根职责问题且不处理会破坏本次拆分正确性时才允许纳入当前 Atomic Task。

---

## 9. 执行顺序与停止条件

执行顺序固定：

```text
Stage R4 正式 Complete
      ↓
用户明确调用本任务
      ↓
重新读取规则/任务书 + 检查工作区
      ↓
Atomic 1 Event Admission Policy
      ↓ 最小测试 + 阶段回归
Atomic 2 Failure Translation Audit
      ↓ 若需要才拆 + 验证
Atomic 3 Session Orchestrator 收口
      ↓ 架构审计
Atomic 4 全量 Windows 回归
      ↓
本优化 Complete
```

任一 Atomic Task 出现以下情况必须停止，不得带失败进入下一步：

- build/configure 失败；
- Session/Request/Generation owner 发生歧义；
- 出现循环依赖；
- 新旧实现同时保留导致双路径；
- 快速媒体切换、shutdown、late event/reply 回归；
- public API 或用户可观察行为发生非预期变化。

---

## 10. 最终完成标准

本优化完成后应达到：

```text
PlaybackSession
= Lifecycle
+ Command orchestration
+ Event orchestration
+ authoritative state commit
+ authoritative generation allocation

Event Admission Policy
= PlaybackEvent × Lifecycle → Accept/Reject

Failure Policy（仅在确有必要时）
= Application failure context → Domain failure/event
```

最终目标不是让 `playback_session.cpp` 变得“尽可能短”，而是保证每个文件只有一个清晰变化原因，Session 保持唯一播放真值，并为后续 R5/R6/R7/R8 功能扩展避免持续向 Session 堆积策略分支。
