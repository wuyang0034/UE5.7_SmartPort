# UE5.7.3 蓝图节点连接顺序说明书（逐节点超详细版）

> 目标：
>
> 这份文档不是讲思路，而是讲 **蓝图节点具体怎么连**。
>
> 适合你现在这种情况：
>
> - 已经有 UE5.7.3 港口场景；
> - 已经有原生 UMG 大屏；
> - 想保留原生 UMG；
> - 想把后端快照、AI 聊天、WebSocket 播报、按钮命令，一步一步接进来；
>
> 这份文档默认你已经有：
> - 一个后端（本仓库 `backend/PortDigitalTwin.Api`）
> - 一个 UE5.7.3 工程
> - 一个主大屏 Widget（例如 `WBP_Dashboard`）

---

# 一、你最终要做出的蓝图对象有哪些

为了避免一张蓝图接天下，建议你至少有以下对象：

## 1.1 `WBP_Dashboard`
主大屏 Widget。
负责：
- 承载天气、告警、聊天、播报、按钮
- 初始化 ViewModel / Runtime Subsystem

## 1.2 `BP_PortTwinManager`
放在关卡里的蓝图 Actor。
负责：
- BeginPlay 时初始化系统
- 获取 Runtime Subsystem
- 创建主大屏
- 把大屏加到视口

## 1.3 `BP_PortBroadcastRouter`
负责：
- 监听播报 / 告警
- 驱动灯光、天气特效、桥吊动画等场景联动

## 1.4 `PortTwinRuntimeSubsystem`
建议是 C++ Subsystem，但也可以被蓝图调用。
负责：
- 请求快照
- 连接 WebSocket
- 发聊天请求
- 发命令请求
- 广播数据更新事件

---

# 二、蓝图接线总顺序（先做什么，后做什么）

不要乱做。
建议严格按这个顺序：

```text
第 1 部分：先让主界面出现
第 2 部分：先拿到 Runtime Subsystem
第 3 部分：先请求一次 Snapshot
第 4 部分：把天气/KPI/告警绑上去
第 5 部分：再连接 WebSocket
第 6 部分：再接 AI 聊天按钮
第 7 部分：再接命令按钮
第 8 部分：最后接场景联动
```

---

# 三、第一部分：`BP_PortTwinManager` 的 BeginPlay 逐节点连接顺序

这一部分是整个系统的入口。

## 3.1 目标
运行游戏后自动做这几件事：
1. 获取 Runtime Subsystem
2. 创建主界面
3. 把界面加入屏幕
4. 初始化后端连接
5. 请求第一次快照
6. 连接 WebSocket

## 3.2 节点连接顺序（逐节点）

### 起点节点
```text
Event BeginPlay
```

### 第 1 个节点
```text
Get Game Instance
```

作用：
拿到当前 GameInstance。

### 第 2 个节点
```text
Get Subsystem (PortTwinRuntimeSubsystem)
```

作用：
获取你的运行时子系统。

### 第 3 个节点
```text
Promote to Variable -> RuntimeSubsystem
```

变量名建议：
```text
RuntimeSubsystem
```

类型：
```text
PortTwinRuntimeSubsystem Object Reference
```

### 第 4 个节点
```text
Is Valid
```

检查 `RuntimeSubsystem` 是否有效。

### 第 5 个节点（Valid 分支）
```text
Create Widget
```

Class 选择：
```text
WBP_Dashboard
```

Owning Player：
```text
Get Player Controller
```

### 第 6 个节点
```text
Promote to Variable -> DashboardWidget
```

变量名建议：
```text
DashboardWidget
```

### 第 7 个节点
```text
Add to Viewport
```

作用：
把主大屏显示出来。

### 第 8 个节点
```text
Initialize Runtime
```

这是你在 `PortTwinRuntimeSubsystem` 里暴露的初始化函数。

### 第 9 个节点
```text
Request Snapshot
```

作用：
启动时先拉一次全量状态。

### 第 10 个节点
```text
Connect Realtime Channel
```

作用：
连接 WebSocket。

---

# 四、第二部分：`WBP_Dashboard` 的 Construct 逐节点连接顺序

这个阶段要解决的问题是：

> Widget 自己怎么拿到 Runtime Subsystem，并开始监听更新事件？

## 4.1 起点节点
```text
Event Construct
```

## 4.2 节点顺序

### 第 1 个节点
```text
Get Game Instance
```

### 第 2 个节点
```text
Get Subsystem (PortTwinRuntimeSubsystem)
```

### 第 3 个节点
```text
Promote to Variable -> RuntimeSubsystem
```

### 第 4 个节点
```text
Bind Event to OnSnapshotUpdated
```

这里你需要新建一个自定义事件，例如：
```text
Handle Snapshot Updated
```

### 第 5 个节点
```text
Bind Event to OnBroadcastReceived
```

对应自定义事件：
```text
Handle Broadcast Received
```

### 第 6 个节点
```text
Bind Event to OnChatReplyReceived
```

对应自定义事件：
```text
Handle Chat Reply Received
```

### 第 7 个节点
```text
Bind Event to OnCommandAckReceived
```

对应自定义事件：
```text
Handle Command Ack Received
```

### 第 8 个节点
```text
Bind Event to OnConnectionStateChanged
```

对应自定义事件：
```text
Handle Connection State Changed
```

---

# 五、第三部分：天气/KPI/告警 的刷新逻辑（`Handle Snapshot Updated`）

这是最关键的 UI 更新节点链。

## 5.1 自定义事件
```text
Handle Snapshot Updated
```

输入参数建议：
```text
Snapshot (PortTwinSnapshot)
```

## 5.2 节点连接顺序

### 第 1 个节点
```text
Break PortTwinSnapshot
```

拆出：
- Weather
- Kpi
- Alarms
- Broadcasts
- TyphoonTrack

### 第 2 个节点（天气）
```text
Break PortWeatherState
```

取出：
- WindSpeed
- Rainfall
- VisibilityKm
- Condition

### 第 3 个节点（更新天气文本）
逐个连接到：
```text
SetText (TextBlock_WeatherWind)
SetText (TextBlock_WeatherRain)
SetText (TextBlock_WeatherVisibility)
SetText (TextBlock_WeatherCondition)
```

如果需要格式化，插入：
```text
Format Text
```

例如：
- `风速：{0} m/s`
- `降雨：{0} mm`

### 第 4 个节点（KPI）
```text
Break PortKpiState
```

取出：
- VesselsInPort
- DailyTeu
- OnlineDevices
- HighPriorityAlarms

然后分别连到：
```text
SetText (TextBlock_VesselCount)
SetText (TextBlock_DailyTeu)
SetText (TextBlock_OnlineDevice)
SetText (TextBlock_HighAlarmCount)
```

### 第 5 个节点（告警列表）
这里推荐两种方式：

#### 方式 A：简单演示版
如果你只显示“最新一条告警”：
- 从 `Alarms` 数组取 `Last` 或 `Get(0)`
- `Break PortAlarmItem`
- `SetText (TextBlock_LatestAlarm)`

#### 方式 B：正式版
如果你显示列表：
- `Clear Children`（Alarm List 容器）
- `ForEachLoop` 遍历 `Alarms`
- 每一项 `Create Widget (WBP_AlarmItem)`
- 设置条目文本
- `Add Child`

---

# 六、第四部分：播报处理逻辑（`Handle Broadcast Received`）

## 6.1 自定义事件
```text
Handle Broadcast Received
```

输入参数建议：
```text
Broadcast (PortBroadcastItem)
```

## 6.2 节点顺序

### 第 1 个节点
```text
Break PortBroadcastItem
```

取出：
- Title
- Content
- Severity
- Timestamp

### 第 2 个节点
更新字幕区：
```text
SetText (TextBlock_BroadcastTitle)
SetText (TextBlock_BroadcastContent)
```

### 第 3 个节点
根据 `Severity` 做分支：

```text
Switch on EPortAlarmSeverity
```

分支建议：
- `Info`
- `Warning`
- `Critical`

### 第 4 个节点（Info）
执行：
- `Set Color and Opacity` -> 普通蓝色/白色
- 可选播放轻提示动画

### 第 5 个节点（Warning）
执行：
- `Set Color and Opacity` -> 黄色
- `Play Animation`（警示条闪烁）
- 调用广播路由器 `Trigger Warning`

### 第 6 个节点（Critical）
执行：
- `Set Color and Opacity` -> 红色
- `Play Animation`（强警告闪烁）
- 调用广播路由器 `Trigger Critical`

---

# 七、第五部分：连接状态显示逻辑（`Handle Connection State Changed`）

## 7.1 自定义事件
```text
Handle Connection State Changed
```

参数：
```text
NewState (EPortConnectionState)
```

## 7.2 节点顺序

### 第 1 个节点
```text
Switch on EPortConnectionState
```

### 第 2 个节点（Disconnected）
- `SetText (TextBlock_ConnectionStatus)` -> “未连接”
- `Set Color and Opacity` -> 橙色

### 第 3 个节点（Connecting）
- `SetText` -> “连接中”
- `Set Color and Opacity` -> 蓝色

### 第 4 个节点（Connected）
- `SetText` -> “已连接”
- `Set Color and Opacity` -> 绿色

### 第 5 个节点（Error）
- `SetText` -> “连接失败”
- `Set Color and Opacity` -> 红色

---

# 八、第六部分：AI 聊天发送按钮接线（逐节点）

这一部分是用户主动交互最重要的部分。

## 8.1 目标
用户点击发送按钮后：
1. 读取输入框内容
2. 过滤空文本
3. 先把用户消息显示出来
4. 调用 Runtime Subsystem 发请求
5. 等后端返回 AI 回复

## 8.2 发送按钮节点顺序

### 起点节点
```text
OnClicked (Button_SendChat)
```

### 第 1 个节点
```text
GetText (EditableTextBox_ChatInput)
```

### 第 2 个节点
```text
Conv_TextToString
```

### 第 3 个节点
```text
Trim
```

### 第 4 个节点
```text
Len
```

### 第 5 个节点
```text
> 0
```

### 第 6 个节点
```text
Branch
```

如果为 False：
- 直接 Return

如果为 True：
继续往下。

### 第 7 个节点
```text
Add User Message To Chat Panel
```

如果你没有单独函数，就手工做：
- `Create Widget (WBP_ChatMessageItem)`
- 设置 `Role = User`
- 设置 `Content = 输入文本`
- `Add Child to ScrollBox`

### 第 8 个节点
```text
Send Chat Message
```

调用对象：
```text
RuntimeSubsystem
```

参数：
```text
UserText = 输入文本
```

### 第 9 个节点
```text
SetText (EditableTextBox_ChatInput) = Empty
```

### 第 10 个节点
```text
SetText (TextBlock_ChatStatus) = “AI 正在思考...”
```

---

# 九、第七部分：AI 回复接线（`Handle Chat Reply Received`）

## 9.1 自定义事件
```text
Handle Chat Reply Received
```

参数建议：
```text
Reply (PortAiChatResponse)
```

## 9.2 节点顺序

### 第 1 个节点
```text
Break PortAiChatResponse
```

取出：
- ReplyText
- SuggestedActions
- Snapshot

### 第 2 个节点
把 AI 回复显示到聊天区：
- `Create Widget (WBP_ChatMessageItem)`
- `Set Role = Assistant`
- `Set Content = ReplyText`
- `Add Child to ScrollBox`

### 第 3 个节点
更新状态：
```text
SetText (TextBlock_ChatStatus) = “AI 已回复”
```

### 第 4 个节点
清空推荐问题区：
```text
Clear Children (WrapBox_Suggestions)
```

### 第 5 个节点
```text
ForEachLoop (SuggestedActions)
```

循环体里：
- `Create Widget (WBP_SuggestionButton)`
- 设置按钮文字
- 绑定点击事件
- `Add Child`

### 第 6 个节点（如果返回了 Snapshot）
可以直接：
```text
Handle Snapshot Updated
```

这样 AI 回答后，界面也一起更新。

---

# 十、第八部分：推荐问题按钮接线（逐节点）

## 10.1 目标
用户点推荐问题按钮时：
- 可以回填到输入框
- 或者直接发送

我建议直接发送，交互更顺。

## 10.2 按钮点击节点顺序

### 起点节点
```text
OnClicked (SuggestionButton)
```

### 第 1 个节点
```text
Get Suggestion Text
```

### 第 2 个节点
```text
SetText (EditableTextBox_ChatInput)
```

### 第 3 个节点
直接调用：
```text
OnClicked (Button_SendChat) 的同一套发送逻辑
```

建议你把“发送逻辑”封装成函数：
```text
SendChatFromInput
```

这样推荐按钮和发送按钮都调用同一个函数。

---

# 十一、第九部分：命令按钮接线（场景命令/设备命令）

## 11.1 目标
用户点击按钮后：
- 发命令给后端
- 界面记录日志
- 回执后更新状态
- 同时触发本地场景逻辑

## 11.2 示例：`Button_StormWarningOn`

### 起点节点
```text
OnClicked (Button_StormWarningOn)
```

### 第 1 个节点
```text
Execute Scene Command
```

对象：
```text
RuntimeSubsystem
```

参数：
```text
CommandName = StormWarningOn
```

### 第 2 个节点
```text
Append Command Log
```

显示：
```text
[时间] 已发送场景命令：StormWarningOn
```

## 11.3 示例：`Button_EmergencyStop`

### 起点节点
```text
OnClicked (Button_EmergencyStop)
```

### 第 1 个节点
```text
Execute Device Command
```

对象：
```text
RuntimeSubsystem
```

参数：
```text
CommandName = EmergencyStop
```

### 第 2 个节点
```text
Append Command Log
```

文本：
```text
[时间] 已发送设备命令：EmergencyStop
```

---

# 十二、第十部分：命令回执接线（`Handle Command Ack Received`）

## 12.1 自定义事件
```text
Handle Command Ack Received
```

参数建议：
```text
Ack (PortCommandAck)
```

## 12.2 节点顺序

### 第 1 个节点
```text
Break PortCommandAck
```

### 第 2 个节点
追加日志：
```text
Append Command Log
```

格式建议：
```text
[{Time}] 命令确认：{Command} / {Status}
```

### 第 3 个节点
根据命令内容做 `Switch on Name` 或 `Switch on String`：
- `PlayCraneAnim`
- `StartVesselIn`
- `StormWarningOn`
- `StopAllDemo`
- `EmergencyStop`

### 第 4 个节点
触发本地场景动作：
- 播放 Sequencer
- 调用桥吊 Actor 自定义事件
- 调用天气系统自定义事件
- 调用报警灯自定义事件

---

# 十三、第十一部分：`BP_PortBroadcastRouter` 场景联动蓝图接线

这个蓝图专门负责把播报和告警转成场景动作。

## 13.1 BeginPlay 接线

### 起点节点
```text
Event BeginPlay
```

### 第 1 个节点
```text
Get Game Instance
```

### 第 2 个节点
```text
Get Subsystem (PortTwinRuntimeSubsystem)
```

### 第 3 个节点
```text
Bind Event to OnBroadcastReceived
```

绑定到：
```text
Handle Router Broadcast
```

### 第 4 个节点
```text
Bind Event to OnAlarmReceived
```

绑定到：
```text
Handle Router Alarm
```

## 13.2 `Handle Router Broadcast` 节点顺序

### 第 1 个节点
```text
Break PortBroadcastItem
```

### 第 2 个节点
```text
Switch on EPortAlarmSeverity
```

#### Info 分支
- 可选：轻微字幕动画

#### Warning 分支
顺序建议：
1. `Play Sound 2D`
2. `Set Warning Light On`
3. `Set Material Parameter`（黄色强调）
4. `Start Wind FX`

#### Critical 分支
顺序建议：
1. `Play Sound 2D`（警报音）
2. `Set Critical Light On`
3. `Start Heavy Rain FX`
4. `Call Crane Stop`
5. `Focus Camera To Critical Area`

---

# 十四、第十二部分：推荐封装成函数的蓝图逻辑

为了避免图越连越长，建议把下面这些逻辑封装成函数：

## 14.1 `InitializeDashboard`
负责：
- Widget 初始化
- 事件绑定

## 14.2 `RefreshWeatherPanel`
负责：
- 更新风速/降雨/能见度/天气文本

## 14.3 `RefreshKpiPanel`
负责：
- 更新 KPI 数值

## 14.4 `RefreshAlarmList`
负责：
- 重建告警列表

## 14.5 `AppendChatMessage`
负责：
- 向聊天 ScrollBox 添加一条消息

## 14.6 `AppendCommandLog`
负责：
- 向命令日志添加一条文本

## 14.7 `ApplyBroadcastVisualStyle`
负责：
- 根据 Info / Warning / Critical 改 UI 风格

## 14.8 `SendChatFromInput`
负责：
- 统一发送逻辑
- 让发送按钮和推荐按钮共用

---

# 十五、第十三部分：你实际施工时，最稳的蓝图搭建顺序

## 第 1 天：只做入口蓝图
你先只做：
- `BP_PortTwinManager`
- BeginPlay 创建 Widget
- 请求第一次 Snapshot

## 第 2 天：把 Snapshot UI 刷新做完
你再做：
- `Handle Snapshot Updated`
- 天气
- KPI
- 告警

## 第 3 天：做 WebSocket 和播报
你再做：
- `Handle Broadcast Received`
- 连接状态显示
- 告警样式变化

## 第 4 天：做聊天
你再做：
- 输入框
- 发送按钮
- AI 回复
- 推荐问题按钮

## 第 5 天：做命令和场景联动
你最后做：
- 命令按钮
- 命令回执
- 播报路由器
- 风雨/灯光/桥吊联动

---

# 十六、最容易出错的节点连接点

## 16.1 BeginPlay 里先 Connect 再 Create Widget
不推荐。
建议先把 Widget 建出来，再初始化系统，这样你能第一时间看到连接状态变化。

## 16.2 Event Construct 里重复 Bind
很容易重复绑定事件，导致一条消息触发多次。

建议：
- 只在初始化时绑定一次
- 或绑定前先 Unbind

## 16.3 聊天消息直接用 TextBlock 覆盖，而不是列表追加
这样会导致只能看到最后一条消息。

建议一定做消息列表。

## 16.4 WebSocket 消息直接在 Widget 各处分散处理
不建议。
应该统一走 Runtime Subsystem。

## 16.5 命令按钮直接播本地动画，不经过后端
短期看省事，长期一定乱。

建议统一流程：
- 先发命令
- 后端确认
- 再本地执行

---

# 十七、最后给你一个最短可行蓝图版本

如果你明天就要先做出 Demo，最短可行版本只需要完成这 4 条链：

## 链路 1：启动链
`BP_PortTwinManager.BeginPlay`
-> 获取 RuntimeSubsystem
-> Create Widget
-> Add to Viewport
-> Initialize Runtime
-> Request Snapshot
-> Connect Realtime

## 链路 2：数据刷新链
`OnSnapshotUpdated`
-> Handle Snapshot Updated
-> 更新天气/KPI/告警

## 链路 3：聊天链
`Button_SendChat.OnClicked`
-> SendChatFromInput
-> RuntimeSubsystem.SendChatMessage
-> OnChatReplyReceived
-> AppendChatMessage

## 链路 4：播报联动链
`OnBroadcastReceived`
-> Handle Broadcast Received
-> 更新 UI 样式
-> BP_PortBroadcastRouter.Trigger Scene FX

只要这 4 条链打通，你的 UE5.7.3 原生 UMG 大屏就已经能形成一个完整演示闭环。

---

如果你愿意，我下一步还可以继续给你补：

1. **`PortTwinRuntimeSubsystem` 的 UE5 C++ 骨架代码**
2. **`UPortHttpService` / `UPortWebSocketService` 的 C++ 示例代码**
3. **`WBP_Dashboard` 的控件命名清单 + 蓝图变量命名表**
