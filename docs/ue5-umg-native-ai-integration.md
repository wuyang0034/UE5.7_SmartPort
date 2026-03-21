# UE5.7 原生 UMG 控件接入数字孪生数据与 AI 的完整实施手册

> 适用场景：
>
> - 你已经有一个 **UE5.7 港口码头场景**；
> - 你已经有一个 **UMG 原生大屏界面**；
> - 你不想再嵌 Web 页面；
> - 你希望 **原来的 UMG 大屏继续保留**；
> - 你只想把 **数据能力** 和 **AI 智能体能力** 接进来；
>
> 那么这份文档就是专门写给你的。

---

# 一、先说结论：原生 UMG 接法，建议采用什么框架结构？

如果你要长期做，不建议把所有逻辑都堆进 Widget 蓝图里。

最合适的 UE5 原生结构，我建议用：

```text
表现层（Presentation）
├─ UMG Widgets
├─ Widget Blueprints
└─ MVVM ViewModel

应用层（Application）
├─ UGameInstanceSubsystem
├─ UI Controller / Facade
└─ Event Dispatcher / Delegates

服务层（Services）
├─ HTTP Service
├─ WebSocket Service
├─ AI Chat Service
└─ Command Service

数据层（Data / Domain）
├─ USTRUCT DTOs
├─ Twin Snapshot Models
├─ Alarm / Weather / Typhoon Models
└─ Command / Chat Models
```

也就是 4 层：

1. **UMG 只负责显示和交互**
2. **Subsystem 负责统一调度**
3. **Service 负责网络通信**
4. **Data Model 负责承载结构化数据**

---

# 二、为什么推荐“MVVM + Subsystem + Service Layer”而不是全蓝图堆一起？

因为你这个项目不是一个简单菜单 UI，而是一个 **数字孪生运行大屏**。

它会有这些特点：
- 实时数据刷新
- AI 对话
- WebSocket 持续推送
- 告警播报
- 多个 UI 面板同时更新
- 后续还可能接设备状态、船舶状态、天气状态、台风状态

如果你把这些都直接写在 Widget 蓝图里，会出现这些问题：

## 2.1 Widget 蓝图会越来越乱
后面你会在同一个 Widget 里看到：
- HTTP 请求
- WebSocket 连接
- JSON 解析
- UI 更新
- 按钮事件
- AI 聊天逻辑
- 告警队列

最后会非常难维护。

## 2.2 多个 Widget 无法共享统一状态
例如：
- 左侧天气面板要显示风速
- 顶部状态条也要显示风速
- 右侧告警区也要根据风速改变颜色

如果没有统一 ViewModel / Subsystem，数据同步会非常麻烦。

## 2.3 后期从模拟数据换真实数据时，改动会很大
如果网络逻辑写死在 Widget 里，后续替换后端接口会非常痛苦。

---

# 三、推荐的 UE5 工程内目录结构

如果你准备在自己的 UE5 项目里正式接入，建议你在项目中整理出如下结构：

```text
Content/
├─ Blueprints/
│  ├─ UI/
│  │  ├─ Widgets/
│  │  │  ├─ WBP_Dashboard
│  │  │  ├─ WBP_WeatherPanel
│  │  │  ├─ WBP_TyphoonPanel
│  │  │  ├─ WBP_AlarmPanel
│  │  │  ├─ WBP_AIChatPanel
│  │  │  └─ WBP_CommandPanel
│  │  └─ ViewModels/
│  │     ├─ VM_Dashboard
│  │     ├─ VM_Weather
│  │     ├─ VM_AIChat
│  │     └─ VM_Alarm
│  ├─ Systems/
│  │  ├─ BP_PortTwinManager
│  │  └─ BP_PortBroadcastRouter
│  └─ Data/
│     └─ DT_PortConfig
│
Source/<YourProject>/
├─ Twin/
│  ├─ Models/
│  │  ├─ PortTwinTypes.h
│  │  ├─ PortTwinSnapshot.h
│  │  └─ PortTwinCommandTypes.h
│  ├─ Services/
│  │  ├─ PortHttpService.h/.cpp
│  │  ├─ PortWebSocketService.h/.cpp
│  │  ├─ PortAiService.h/.cpp
│  │  └─ PortCommandService.h/.cpp
│  ├─ Subsystems/
│  │  └─ PortTwinRuntimeSubsystem.h/.cpp
│  ├─ ViewModels/
│  │  ├─ PortDashboardViewModel.h/.cpp
│  │  ├─ PortWeatherViewModel.h/.cpp
│  │  └─ PortAiChatViewModel.h/.cpp
│  └─ Utils/
│     └─ PortJsonMapper.h/.cpp
```

这个结构的好处是：
- UI 和逻辑分离
- 数据结构统一
- 后续扩展真实业务接口时不会推倒重来

---

# 四、UMG 原生接入时，各个模块分别负责什么？

## 4.1 UMG Widget 负责什么
UMG Widget 只负责三件事：

1. 显示数据
2. 收集用户输入
3. 响应 ViewModel 变化

例如：
- `WBP_WeatherPanel` 只负责显示风速、降雨、能见度
- `WBP_AIChatPanel` 只负责聊天记录和输入框
- `WBP_AlarmPanel` 只负责告警列表

## 4.2 ViewModel 负责什么
ViewModel 是 UI 的直接数据来源。

例如：
- 当前风速是多少
- 当前告警列表是什么
- AI 最近一条回复是什么
- 当前连接状态是什么

ViewModel 不直接发 HTTP 请求。
它只负责：
- 保存可绑定属性
- 通知 UMG 刷新

## 4.3 Runtime Subsystem 负责什么
建议你把核心调度放到：

```text
UGameInstanceSubsystem
```

原因是：
- 生命周期稳定
- 场景切换时不容易丢失
- 适合持有 WebSocket 和全局状态

它负责：
- 启动时初始化服务
- 请求快照
- 连接 WebSocket
- 转发 AI 回复
- 分发告警与播报
- 向 ViewModel 广播数据更新事件

## 4.4 HTTP Service 负责什么
负责：
- `GET /api/twin/snapshot`
- `POST /api/chat`
- `POST /api/commands/execute`

## 4.5 WebSocket Service 负责什么
负责：
- 连接 `/ws`
- 持续接收后端推送
- 解析实时消息
- 把消息交给 Runtime Subsystem

## 4.6 AI Service 负责什么
负责：
- 组织聊天请求
- 调用后端 AI 接口
- 返回 AI 回复和建议动作

## 4.7 Command Service 负责什么
负责：
- 把 UMG 按钮点击转换成标准命令结构
- 调用 `/api/commands/execute`
- 处理命令确认回执

---

# 五、全流程总览：原生 UMG 接入应该怎么做？

你可以把整个实施过程理解成 10 步。

```text
第 1 步：准备 UE5 插件和工程结构
第 2 步：定义数据结构（USTRUCT）
第 3 步：实现 Runtime Subsystem
第 4 步：实现 HTTP Service
第 5 步：实现 WebSocket Service
第 6 步：实现 ViewModel
第 7 步：把 UMG 控件绑定到 ViewModel
第 8 步：接入 AI 聊天流程
第 9 步：接入播报与告警联动流程
第 10 步：接入命令按钮与场景联动
```

下面我按这个顺序详细展开。

---

# 六、第 1 步：先准备 UE5 项目环境

## 6.1 打开你的 UE5.7 工程
先打开你现在已经有的港口项目。

## 6.2 启用建议插件
建议启用以下插件：

### 必选
- **Model View ViewModel**（MVVM）
- **HTTP**（通常默认可用）
- **Json** / **Json Utilities**

### 推荐
- **WebSockets**
- **UMG**
- **CommonUI**（可选）

## 6.3 为什么一定建议开 MVVM
因为你现在已经决定：
- 保留 UMG 原生大屏
- 不用网页嵌入

这种情况下，MVVM 是最适合把：
- 数据状态
- 界面显示
- 用户交互

做解耦的方式。

## 6.4 建一个专门的 Twin 模块或代码目录
如果你有 C++ 工程，建议创建一组专用类，放在：

```text
Source/<YourProject>/Twin/
```

这样后面不会和你原来的场景逻辑混在一起。

---

# 七、第 2 步：先定义所有需要的 USTRUCT 数据结构

这一点非常关键。

你不要一上来就写 HTTP 请求。
正确顺序是：

> 先把“系统里有哪些数据”定义清楚。

## 7.1 你至少应该定义这些结构

### 天气结构
字段建议：
- WindSpeed
- Rainfall
- VisibilityKm
- Condition
- TemperatureC

### KPI 结构
字段建议：
- VesselsInPort
- DailyTeu
- OnlineDevices
- HighPriorityAlarms

### 告警结构
字段建议：
- Id
- Severity
- Source
- Message
- Timestamp

### 播报结构
字段建议：
- Title
- Content
- Severity
- Timestamp

### AI 聊天消息结构
字段建议：
- Role
- Content
- Timestamp

### 台风点位结构
字段建议：
- X
- Y
- TimeLabel
- WindSpeed

### 总快照结构 `TwinSnapshot`
字段建议：
- PortName
- Kpi
- Weather
- Routes
- TyphoonTrack
- Yard
- Alarms
- Broadcasts
- Timestamp

## 7.2 为什么必须先定义结构
因为后面所有模块都依赖这套结构：
- HTTP 解析要用它
- WebSocket 解析要用它
- ViewModel 要用它
- Widget 绑定要用它
- AI 播报也要用它

如果这里不统一，后面一定混乱。

---

# 八、第 3 步：实现 Runtime Subsystem（核心调度中心）

这一层是整个原生 UMG 接入的核心。

建议创建：

```text
UPortTwinRuntimeSubsystem : UGameInstanceSubsystem
```

## 8.1 它的职责
它要负责：

1. 保存当前最新的 `TwinSnapshot`
2. 保存当前连接状态
3. 启动时初始化服务
4. 提供蓝图可调用函数
5. 分发数据更新事件
6. 分发 AI 回复事件
7. 分发播报和告警事件

## 8.2 它应该对外暴露哪些能力
建议至少暴露这些 BlueprintCallable / C++ 接口：

- `InitializeRuntime()`
- `ConnectRealtimeChannel()`
- `DisconnectRealtimeChannel()`
- `RequestSnapshot()`
- `SendChatMessage(FString UserText)`
- `ExecuteSceneCommand(FName CommandName)`
- `ExecuteDeviceCommand(FName CommandName)`
- `GetLatestSnapshot()`
- `GetConnectionState()`

## 8.3 它还应该暴露哪些事件
建议用 Delegate / Multicast Delegate：

- `OnSnapshotUpdated`
- `OnWeatherUpdated`
- `OnAlarmReceived`
- `OnBroadcastReceived`
- `OnChatReplyReceived`
- `OnConnectionStateChanged`
- `OnCommandAckReceived`

这样 UI 层就可以只监听事件，而不用知道 HTTP / WebSocket 细节。

---

# 九、第 4 步：实现 HTTP Service

建议创建一个专门的服务类，例如：

```text
UPortHttpService
```

或者纯 C++ 非 UObject service 也可以。

## 9.1 它要负责哪些接口

### 请求快照
```text
GET /api/twin/snapshot
```

### AI 聊天
```text
POST /api/chat
```

### 命令执行
```text
POST /api/commands/execute
```

## 9.2 快照请求的标准流程

### 第一步
Runtime Subsystem 调用 `RequestSnapshot()`。

### 第二步
`RequestSnapshot()` 内部调用 HTTP Service。

### 第三步
HTTP Service 发出 `GET /api/twin/snapshot`。

### 第四步
拿到返回 JSON 后，解析成 `FTwinSnapshot`。

### 第五步
Subsystem 更新内部状态，并广播 `OnSnapshotUpdated`。

### 第六步
ViewModel 收到更新后刷新绑定字段。

### 第七步
UMG 自动刷新界面。

## 9.3 AI 聊天请求的标准流程

### 第一步
用户在 UMG 输入框输入问题。

### 第二步
点击发送按钮。

### 第三步
按钮事件调用 `SendChatMessage()`。

### 第四步
Subsystem 调用 HTTP Service 发送：

```json
{
  "message": "当前台风风险如何？",
  "sessionId": "ue-session-001",
  "includeSnapshot": true
}
```

### 第五步
后端返回：
- AI 回复文本
- 建议动作
- 最新快照

### 第六步
Subsystem 更新聊天记录，并刷新对应 ViewModel。

### 第七步
UMG 聊天面板显示：
- 用户消息
- AI 回复
- 建议按钮

## 9.4 命令执行的标准流程

### 第一步
用户点击 UMG 按钮，例如：
- 台风预警演示
- 播放吊机动画
- 紧急停止

### 第二步
UI 把按钮意图转换成标准命令对象。

### 第三步
Subsystem 调用 HTTP Service 发请求到：

```text
POST /api/commands/execute
```

### 第四步
后端返回命令已接收。

### 第五步
Subsystem 分发 `OnCommandAckReceived`。

### 第六步
UI 显示“命令已接收”，同时场景逻辑开始执行。

---

# 十、第 5 步：实现 WebSocket Service

这一层负责实时推送。

建议创建：

```text
UPortWebSocketService
```

## 10.1 它负责什么
- 连接 `/ws`
- 接收后端广播
- 判断消息类型
- 转发给 Runtime Subsystem

## 10.2 为什么 WebSocket 很重要
因为数字孪生不是“点一下刷新一次”的系统。

而是：
- 风速在变
- 台风路径在变
- 告警在变
- AI 播报在变

所以不能只靠轮询。

## 10.3 WebSocket 接入后的典型消息类型
你至少要处理：

### `snapshot`
表示后台推送了一份新快照。

### `broadcast`
表示有新的 AI 播报。

### `commandAck`
表示某条命令已经被后端接收。

### `presence`
表示连接状态变化。

## 10.4 推荐做法
WebSocket Service 不要直接操作 UMG。

正确做法是：
- WebSocket Service 收到消息
- 解析 JSON
- 调用 Runtime Subsystem 的对应处理函数
- Runtime Subsystem 再广播给 ViewModel / Widget

这样层级才清晰。

---

# 十一、第 6 步：实现 ViewModel（这是 UMG 原生接法的关键）

如果你使用 MVVM，建议至少拆成 4 类 ViewModel。

## 11.1 `DashboardViewModel`
负责：
- 总体连接状态
- KPI 汇总
- 当前播报
- 当前系统状态

## 11.2 `WeatherViewModel`
负责：
- 风速
- 雨量
- 能见度
- 天气描述

## 11.3 `AlarmViewModel`
负责：
- 告警列表
- 告警等级颜色
- 最新告警摘要

## 11.4 `AIChatViewModel`
负责：
- 聊天消息列表
- 用户输入文本
- AI 回复
- 推荐问题
- 发送状态

## 11.5 ViewModel 的工作方式
ViewModel 不直接发请求。
它做的事是：
- 监听 Runtime Subsystem 的事件
- 把数据转换成可供 UMG 绑定的字段
- 通知界面刷新

---

# 十二、第 7 步：把现有 UMG 原生控件绑定到 ViewModel

这是你真正开始“保留原来的 UMG 大屏”的步骤。

## 12.1 先不要推翻你原来的 UMG 布局
你原来的 UMG 可以继续保留。
你要做的是：
- 保留已有视觉布局
- 只给现有控件补数据绑定

例如：
- 原来顶部有一个风速文本 -> 现在绑定到 `WeatherViewModel.WindSpeedText`
- 原来有一个告警列表 -> 现在绑定到 `AlarmViewModel.AlarmItems`
- 原来有一个 AI 展示框 -> 现在绑定到 `AIChatViewModel.ChatMessages`

## 12.2 典型绑定方式

### 文本类控件
绑定到：
- `FText`
- `FString`
- 数值转文本属性

### 颜色类控件
绑定到：
- `FLinearColor`
- 告警等级颜色

### 列表类控件
绑定到：
- `ListView`
- `TileView`
- 动态子 Widget

### 按钮类控件
点击后调用：
- `SendChatMessage`
- `ExecuteSceneCommand`
- `ExecuteDeviceCommand`

## 12.3 你原来的 UMG 页面可以怎么拆
建议拆成这些面板：

- 顶部运行状态条
- 左侧天气与台风面板
- 中部三维场景覆盖层文字区
- 右侧 AI 聊天面板
- 下方告警与播报条
- 操作按钮区

每个面板绑定各自的 ViewModel 数据，不要所有东西都绑到一个巨大的 Widget 上。

---

# 十三、第 8 步：接入 AI 聊天全流程（原生 UMG 版本）

这一部分是很多人最关心的。

你要实现的用户体验应该是：

1. 用户在原生 UMG 输入框输入问题
2. 点击发送
3. UMG 出现“用户消息”
4. 后端返回 AI 回复
5. UMG 追加一条“AI 回复”
6. 如果后端返回了建议动作，界面上显示快捷按钮

## 13.1 聊天面板最少需要哪些控件
建议至少有：
- 输入框 `EditableTextBox`
- 发送按钮 `Button`
- 消息滚动区 `ScrollBox`
- 消息列表项 Widget
- 推荐问题按钮区
- 状态文本（例如：AI 正在思考 / 已连接 / 未连接）

## 13.2 发送消息时的流程

### 第一步
用户输入文本。

### 第二步
点击“发送”。

### 第三步
Widget 调用 ViewModel 的 `SendMessage`。

### 第四步
ViewModel 通知 Runtime Subsystem 发起聊天请求。

### 第五步
Runtime Subsystem 调用 HTTP Service 请求 `/api/chat`。

### 第六步
返回后，Subsystem 触发 `OnChatReplyReceived`。

### 第七步
AIChatViewModel 更新消息数组。

### 第八步
聊天 Widget 刷新列表。

## 13.3 推荐问题按钮怎么做
后端会返回建议动作或建议问题。
你可以把它们显示成：
- 3~4 个快捷按钮
- 点击后直接回填到输入框
- 或点击后直接发送

这会让你的大屏 AI 更像一个真正的值班智能体，而不是普通聊天框。

---

# 十四、第 9 步：接入自动播报、告警与场景联动

这一部分是“数字孪生味道”最强的部分。

## 14.1 播报应该怎么显示在原生 UMG 里
建议至少做 3 个显示位置：

### 位置 1：顶部滚动播报条
显示：
- 当前最新播报
- 风险级别
- 时间

### 位置 2：中央警示弹层
当 `warning` 或 `critical` 事件出现时显示。

### 位置 3：右侧播报历史列表
显示最近 N 条播报记录。

## 14.2 告警和播报不要混成一个东西
虽然它们相关，但建议你 UI 上分开：

### 告警
更偏系统异常事实：
- 东码头风速过高
- 某区域设备离线
- 某泊位作业异常

### 播报
更偏 AI 翻译后的自然语言：
- “当前台风外圈影响增强，建议限制岸桥高空作业。”

## 14.3 原生 UMG 联动场景时，推荐的规则映射

### 轻度信息 `info`
- 改变字幕内容
- 顶部状态条闪动一次

### 预警 `warning`
- 告警条变黄
- 高亮关键设备
- 播放提示音
- 打开警示灯

### 严重 `critical`
- 全屏警示 UI
- 场景天气加重
- 红色告警灯
- 禁止高风险控制按钮
- 相机切到关键区域

## 14.4 典型联动例子
例如后端推送：
- 风速 26m/s
- 告警等级 critical
- 播报内容要求停止高空作业

那么你可以在 UE5 中这样做：

1. `BroadcastReceived` 事件触发
2. Runtime Subsystem 解析到 `critical`
3. 通知 `DashboardViewModel` 更新红色警示 UI
4. 同时通知场景控制器：
   - 关闭桥吊动画
   - 开启风雨特效
   - 打开红色警报灯
5. UMG 弹出强提醒文本

---

# 十五、第 10 步：接入命令按钮与场景控制

这是从“看板”走向“可交互控制面板”的关键。

## 15.1 你现有 UMG 上建议保留哪些按钮

### 场景演示类
- 播放吊机动画
- 船舶进港演示
- 台风预警演示
- 停止所有演示

### 设备控制类
- 闸机开启
- 皮带机启动
- 堆场照明开启
- 紧急停止

## 15.2 原生 UMG 按钮点击后的标准流程

### 第一步
按钮 OnClicked。

### 第二步
调用 ViewModel / Controller。

### 第三步
构造统一命令对象：
- CommandId
- Kind
- Command
- Source
- Timestamp

### 第四步
Subsystem 调用 `POST /api/commands/execute`。

### 第五步
后端返回 `accepted`。

### 第六步
Subsystem 广播 `OnCommandAckReceived`。

### 第七步
UMG 显示成功日志。

### 第八步
场景控制器执行本地动画或设备模拟动作。

## 15.3 为什么要“先后端确认，再本地执行”
因为后面你会接真实系统。

如果你现在直接点击按钮就本地执行，未来接真设备时会变得混乱。

正确逻辑应该从现在就统一成：
- UI 发命令
- 后端确认
- UE5 执行
- 状态回写

这样未来扩展为真正工业控制流程时最顺。

---

# 十六、真正的实施顺序：你应该怎么从头做到尾

下面给你一个真正适合施工的顺序。

## 阶段 1：先打通基础数据
目标：让原生 UMG 面板先“活起来”。

你先做：
1. 定义 USTRUCT
2. 写 Runtime Subsystem
3. 写 `GET /api/twin/snapshot`
4. 让天气、KPI、告警先显示出来

不要一开始就做聊天和 WebSocket。

## 阶段 2：再接实时播报
目标：让界面实时更新。

你再做：
1. 接 WebSocket
2. 接播报消息
3. 接告警消息
4. 让 UMG 的播报条和告警区刷新

## 阶段 3：再接 AI 聊天
目标：让 AI 真正出现在 UMG 原生界面里。

你再做：
1. 聊天面板控件
2. `POST /api/chat`
3. 聊天消息列表
4. 推荐问题按钮

## 阶段 4：最后接控制按钮和场景联动
目标：让它从“信息大屏”升级成“可交互数字孪生系统”。

你最后做：
1. 命令按钮
2. `/api/commands/execute`
3. 场景动画映射
4. 告警等级与天气特效映射

---

# 十七、你现有项目里最建议的落地方式

如果我是按你的现状来做，我会建议：

## 第一层：保留你现在的 UMG 视觉界面
不要推翻重做。
只做数据绑定和功能接入。

## 第二层：增加一个 `PortTwinRuntimeSubsystem`
把所有网络和状态统一收口。

## 第三层：用 MVVM 把 UMG 控件绑上去
这样后面扩展不会崩。

## 第四层：先接快照和播报
先让界面活起来。

## 第五层：再接 AI 聊天
让 AI 进入原生大屏。

## 第六层：最后再接按钮和 UE5 场景联动
这样每一步都可测试、可演示、可回退。

---

# 十八、你实施时最容易踩的坑

## 18.1 把 HTTP 和 JSON 解析写进 Widget
不建议。
这会导致 Widget 蓝图变得巨大且不可维护。

## 18.2 所有数据都只存在 Widget 里
不建议。
正确做法是把状态保存在 Runtime Subsystem 或统一状态仓库里。

## 18.3 一个 Widget 负责全屏所有逻辑
不建议。
应该拆分：
- 天气面板
- AI 面板
- 告警面板
- 命令面板

## 18.4 WebSocket 直接改 UI
不建议。
应该先交给 Runtime Subsystem，再分发到 ViewModel。

## 18.5 还没统一数据结构就开始写界面绑定
不建议。
先定义 USTRUCT，再做网络，再做 ViewModel，再做绑定。

---

# 十九、最终你会得到怎样的原生 UMG 系统

当你按这套方案做完后，你的 UE5 原生大屏会变成这样：

## 19.1 左侧
- 实时天气
- 台风路径
- 告警概览

## 19.2 中间
- UE5 港口三维场景
- 播报字幕
- 关键告警弹层

## 19.3 右侧
- AI 智能体聊天面板
- 推荐问题
- 历史播报
- 命令按钮

## 19.4 背后逻辑
- Runtime Subsystem 统一管理状态
- HTTP Service 管快照/聊天/命令
- WebSocket Service 管实时推送
- ViewModel 驱动 UMG 控件刷新
- 场景控制器负责特效和动画联动

这就不是“一个静态大屏”，而是一个真正的：

> **UE5 原生 UMG 数字孪生智能控制界面**

---

# 二十、最后给你的建议：真正开工时，先做哪 3 件事

如果你明天就准备开工，我建议你按下面顺序做：

## 第 1 件事：先把 USTRUCT 和 Runtime Subsystem 建好
这是整个系统的骨架。

## 第 2 件事：先接 `/api/twin/snapshot`，把天气/KPI/告警显示出来
这是最快看到成果的一步。

## 第 3 件事：再接 `/api/chat` 和 `/ws`
这样 AI 和实时播报就能逐步进来。

等这三件事做完后，再去做：
- 原生聊天 UI 完善
- 场景联动
- 按钮命令闭环
- 真实数据替换

---

如果你愿意，我下一步还可以继续给你补两份更“工程化”的内容：

1. **UE5 C++ 类清单 + 头文件命名建议**
2. **UE5 蓝图节点连接顺序说明书（逐节点版）**
