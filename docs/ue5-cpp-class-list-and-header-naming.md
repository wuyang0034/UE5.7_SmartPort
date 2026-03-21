# UE5 C++ 类清单 + 头文件命名建议（港口数字孪生 UMG 原生接入版）

> 目标：
>
> 给你一份可以直接拿去在 UE5.7.3 工程里落地的 **C++ 类设计清单**。
>
> 这份文档解决两个问题：
>
> 1. 你到底应该建哪些类；
> 2. 这些类的 `.h / .cpp` 文件应该怎么命名，才不会越做越乱。

---

# 一、推荐的总原则

如果你要做的是：
- UE5 原生 UMG 大屏
- AI 聊天
- WebSocket 实时播报
- HTTP 快照/命令调用
- 场景联动

那么建议你把类分成 6 组：

```text
1. Data / Models
2. Services
3. Subsystems
4. ViewModels
5. Widgets / UI Helpers
6. Scene / Runtime Controllers
```

---

# 二、命名总规范建议

## 2.1 类名前缀建议

### `F` 前缀
用于：
- `USTRUCT`
- 轻量数据结构

例如：
- `FPortWeatherState`
- `FPortAlarmItem`
- `FPortTwinSnapshot`

### `U` 前缀
用于：
- `UObject`
- `Subsystem`
- `Widget`
- `ViewModel`
- `BlueprintFunctionLibrary`

例如：
- `UPortTwinRuntimeSubsystem`
- `UPortHttpService`
- `UPortDashboardViewModel`

### `A` 前缀
用于：
- Actor
- 运行时场景控制器

例如：
- `APortTwinManagerActor`
- `APortBroadcastRouterActor`

### `E` 前缀
用于：
- 枚举

例如：
- `EPortConnectionState`
- `EPortAlarmSeverity`
- `EPortCommandKind`

---

## 2.2 文件命名建议

推荐统一使用：

```text
Port + 领域 + 类型
```

例如：
- `PortTwinTypes.h`
- `PortTwinSnapshot.h`
- `PortHttpService.h`
- `PortWebSocketService.h`
- `PortAiChatService.h`
- `PortTwinRuntimeSubsystem.h`
- `PortDashboardViewModel.h`

为什么这样命名？
因为后期你项目里很可能还会有：
- 船舶系统
- 摄像头系统
- 设备系统
- 其他业务 UI

统一以 `Port` 开头，检索最方便，也不容易跟别的模块撞名。

---

# 三、推荐目录结构

```text
Source/<YourProject>/Twin/
├─ Models/
│  ├─ PortTwinTypes.h
│  ├─ PortTwinSnapshot.h
│  ├─ PortCommandTypes.h
│  ├─ PortAiTypes.h
│  └─ PortConnectionTypes.h
│
├─ Services/
│  ├─ PortHttpService.h
│  ├─ PortHttpService.cpp
│  ├─ PortWebSocketService.h
│  ├─ PortWebSocketService.cpp
│  ├─ PortAiChatService.h
│  ├─ PortAiChatService.cpp
│  ├─ PortCommandService.h
│  └─ PortCommandService.cpp
│
├─ Subsystems/
│  ├─ PortTwinRuntimeSubsystem.h
│  └─ PortTwinRuntimeSubsystem.cpp
│
├─ ViewModels/
│  ├─ PortDashboardViewModel.h
│  ├─ PortDashboardViewModel.cpp
│  ├─ PortWeatherViewModel.h
│  ├─ PortWeatherViewModel.cpp
│  ├─ PortAlarmViewModel.h
│  ├─ PortAlarmViewModel.cpp
│  ├─ PortAiChatViewModel.h
│  └─ PortAiChatViewModel.cpp
│
├─ UI/
│  ├─ PortUiFunctionLibrary.h
│  ├─ PortUiFunctionLibrary.cpp
│  ├─ PortWidgetBinder.h
│  └─ PortWidgetBinder.cpp
│
├─ Runtime/
│  ├─ PortTwinManagerActor.h
│  ├─ PortTwinManagerActor.cpp
│  ├─ PortBroadcastRouterActor.h
│  └─ PortBroadcastRouterActor.cpp
│
└─ Utils/
   ├─ PortJsonMapper.h
   ├─ PortJsonMapper.cpp
   ├─ PortLogChannels.h
   └─ PortLogChannels.cpp
```

---

# 四、核心数据结构类清单（必建）

## 4.1 `PortTwinTypes.h`
建议放：
- `EPortAlarmSeverity`
- `EPortConnectionState`
- `EPortCommandKind`
- `FPortRouteInfo`
- `FPortTyphoonPoint`
- `FPortYardStatus`
- `FPortWeatherState`
- `FPortKpiState`
- `FPortAlarmItem`
- `FPortBroadcastItem`

### 为什么这样拆
这是“基础公共类型文件”。
很多别的类都会 include 它。
所以不要在这里塞太多业务逻辑。

---

## 4.2 `PortTwinSnapshot.h`
建议放：
- `FPortTwinSnapshot`

它负责聚合：
- 天气
- KPI
- 航线
- 台风点位
- 堆场
- 告警
- 播报
- 时间戳

### 为什么单独拆出来
因为 `Snapshot` 是整个系统最核心的数据对象。
把它单独放文件，后续维护最清晰。

---

## 4.3 `PortCommandTypes.h`
建议放：
- `FPortCommandRequest`
- `FPortCommandAck`

作用：
- UMG 按钮点击后发命令
- 后端确认后回写命令状态

---

## 4.4 `PortAiTypes.h`
建议放：
- `FPortAiChatRequest`
- `FPortAiChatResponse`
- `FPortAiSuggestionItem`
- `FPortAiMessage`

作用：
- 聊天输入输出
- 推荐问题
- 聊天记录

---

## 4.5 `PortConnectionTypes.h`
建议放：
- `FPortBackendEndpointConfig`
- `FPortRealtimeConnectionInfo`

作用：
- 统一保存 API 地址
- 统一保存 WebSocket 地址
- 统一保存连接状态

---

# 五、服务类清单（必建）

## 5.1 `UPortHttpService`

### 文件名
- `PortHttpService.h`
- `PortHttpService.cpp`

### 职责
负责 REST 请求：
- `GET /api/twin/snapshot`
- `POST /api/chat`
- `POST /api/commands/execute`

### 建议公开方法
- `RequestSnapshot()`
- `SendAiChat()`
- `ExecuteCommand()`

### 不要放什么
不要在这里直接更新 Widget。
它只负责网络请求和结果回调。

---

## 5.2 `UPortWebSocketService`

### 文件名
- `PortWebSocketService.h`
- `PortWebSocketService.cpp`

### 职责
负责：
- 建立 `/ws` 连接
- 接收消息
- 重连
- 断开连接
- 把原始消息上抛给 Runtime Subsystem

### 建议公开方法
- `Connect()`
- `Disconnect()`
- `SendRawMessage()`（可选）
- `IsConnected()`

### 建议事件
- `OnConnected`
- `OnDisconnected`
- `OnMessageReceived`
- `OnConnectionError`

---

## 5.3 `UPortAiChatService`

### 文件名
- `PortAiChatService.h`
- `PortAiChatService.cpp`

### 职责
它其实可以内部调用 `UPortHttpService`，但建议单独封装出来。

负责：
- 组织 AI 请求对象
- 发送聊天请求
- 解析 AI 回复
- 提供推荐问题数据

### 为什么建议单独建
因为 AI 这块未来最容易扩展：
- 大模型切换
- Prompt 拼接策略变化
- 多轮会话管理
- 工具调用扩展

单独拆服务最稳。

---

## 5.4 `UPortCommandService`

### 文件名
- `PortCommandService.h`
- `PortCommandService.cpp`

### 职责
负责：
- 统一生成命令结构
- 调用执行命令接口
- 处理命令回执

### 为什么不要直接在 Widget 里发命令
因为后面你会接：
- 权限
- 审计
- 联锁
- 真设备执行回执

命令一定要独立成服务。

---

# 六、子系统类清单（核心）

## 6.1 `UPortTwinRuntimeSubsystem`

### 文件名
- `PortTwinRuntimeSubsystem.h`
- `PortTwinRuntimeSubsystem.cpp`

### 这是整个系统最核心的类
建议使用：

```cpp
UGameInstanceSubsystem
```

### 它负责：
- 保存全局 `FPortTwinSnapshot`
- 初始化所有 Service
- 管理连接状态
- 接收 HTTP / WebSocket 返回
- 对外广播数据更新事件
- 给 ViewModel 提供统一访问入口

### 建议成员
- `FPortTwinSnapshot CurrentSnapshot`
- `EPortConnectionState ConnectionState`
- `UPortHttpService* HttpService`
- `UPortWebSocketService* WebSocketService`
- `UPortAiChatService* AiChatService`
- `UPortCommandService* CommandService`

### 建议公开方法
- `InitializeRuntime()`
- `RequestSnapshot()`
- `ConnectRealtime()`
- `DisconnectRealtime()`
- `SendChatMessage()`
- `ExecuteSceneCommand()`
- `ExecuteDeviceCommand()`
- `GetCurrentSnapshot()`

### 建议事件
- `OnSnapshotUpdated`
- `OnBroadcastReceived`
- `OnAlarmReceived`
- `OnChatReplyReceived`
- `OnConnectionStateChanged`
- `OnCommandAckReceived`

---

# 七、ViewModel 类清单（强烈建议）

## 7.1 `UPortDashboardViewModel`

### 文件名
- `PortDashboardViewModel.h`
- `PortDashboardViewModel.cpp`

### 负责
- 顶部连接状态
- KPI 总览
- 当前播报摘要
- 系统运行状态

---

## 7.2 `UPortWeatherViewModel`

### 文件名
- `PortWeatherViewModel.h`
- `PortWeatherViewModel.cpp`

### 负责
- 风速文本
- 雨量文本
- 能见度文本
- 天气描述
- 风险颜色

---

## 7.3 `UPortAlarmViewModel`

### 文件名
- `PortAlarmViewModel.h`
- `PortAlarmViewModel.cpp`

### 负责
- 告警数组
- 告警条目颜色
- 最新告警摘要

---

## 7.4 `UPortAiChatViewModel`

### 文件名
- `PortAiChatViewModel.h`
- `PortAiChatViewModel.cpp`

### 负责
- 聊天消息列表
- 当前输入内容
- 推荐问题列表
- 发送状态
- AI 思考中状态

---

# 八、UI 辅助类清单（推荐）

## 8.1 `UPortUiFunctionLibrary`

### 文件名
- `PortUiFunctionLibrary.h`
- `PortUiFunctionLibrary.cpp`

### 负责
一些非常适合抽成通用静态方法的 UI 工具：
- 告警等级转颜色
- 风速转风险级别
- 时间戳格式化
- 数值转 `FText`

### 为什么值得建
因为这些转换逻辑如果散落在蓝图里，会非常难维护。

---

## 8.2 `UPortWidgetBinder`

### 文件名
- `PortWidgetBinder.h`
- `PortWidgetBinder.cpp`

### 可选职责
如果你后面 UI 变复杂，可以用这个类专门处理：
- Widget 与 ViewModel 的初始化绑定
- Widget 初始化时的统一接线

如果你的项目不大，这个类也可以不建。

---

# 九、运行时场景类清单（推荐）

## 9.1 `APortTwinManagerActor`

### 文件名
- `PortTwinManagerActor.h`
- `PortTwinManagerActor.cpp`

### 负责
- 启动时获取 Runtime Subsystem
- 驱动场景级初始化
- 在关卡里充当场景联动入口

### 适合做什么
- 启动自动连接
- 初始化 UMG 大屏
- 将关键事件转给天气/灯光/动画系统

---

## 9.2 `APortBroadcastRouterActor`

### 文件名
- `PortBroadcastRouterActor.h`
- `PortBroadcastRouterActor.cpp`

### 负责
- 监听播报和告警事件
- 根据等级触发场景联动

### 示例职责
- `warning` -> 黄灯、提示音
- `critical` -> 红灯、风雨增强、桥吊停止

---

# 十、工具类清单（推荐）

## 10.1 `FPortJsonMapper` / `UPortJsonMapper`

### 文件名
- `PortJsonMapper.h`
- `PortJsonMapper.cpp`

### 负责
- 把后端 JSON 转成 UE 结构体
- 把命令对象转成 JSON

### 为什么建议单独建
JSON 映射逻辑通常会越来越多。
单独放一个工具类，最方便维护。

---

## 10.2 `PortLogChannels.h`

### 文件名
- `PortLogChannels.h`
- `PortLogChannels.cpp`

### 负责
- 定义日志分类，例如：
  - `LogPortTwin`
  - `LogPortAI`
  - `LogPortWebSocket`

### 为什么建议加
后期排查网络、AI、场景联动问题时非常有用。

---

# 十一、类之间的推荐依赖关系

建议依赖关系如下：

```text
Widget / Widget Blueprint
    ↓
ViewModel
    ↓
PortTwinRuntimeSubsystem
    ↓
HttpService / WebSocketService / AiChatService / CommandService
    ↓
后端 API / WebSocket / AI 服务
```

场景联动则是：

```text
PortTwinRuntimeSubsystem
    ↓
PortBroadcastRouterActor / 场景控制逻辑
    ↓
天气系统 / 灯光系统 / 动画系统 / 告警系统
```

### 非常重要的原则
不要反过来依赖。

例如：
- `HttpService` 不要依赖 Widget
- `WebSocketService` 不要直接改 TextBlock
- `ViewModel` 不要自己 new HTTP 请求

---

# 十二、头文件 include 建议

## 12.1 尽量前向声明
在 `.h` 里尽量使用 forward declaration，减少编译依赖。

例如：

```cpp
class UPortHttpService;
class UPortWebSocketService;
class UPortAiChatService;
class UPortCommandService;
```

## 12.2 把重 include 放到 `.cpp`
例如：
- Json
- HTTP
- WebSockets
- Widget 类

尽量不要在所有 `.h` 里都 include。

---

# 十三、实际最小可落地类清单（如果你想先做最小版本）

如果你不想一口气建很多类，可以先从最小版本开始。

## 第 1 批必须建
- `PortTwinTypes.h`
- `PortTwinSnapshot.h`
- `PortHttpService.h/.cpp`
- `PortWebSocketService.h/.cpp`
- `PortTwinRuntimeSubsystem.h/.cpp`
- `PortDashboardViewModel.h/.cpp`
- `PortAiChatViewModel.h/.cpp`

## 第 2 批建议补
- `PortWeatherViewModel.h/.cpp`
- `PortAlarmViewModel.h/.cpp`
- `PortCommandService.h/.cpp`
- `PortAiChatService.h/.cpp`

## 第 3 批用于工程化完善
- `PortJsonMapper.h/.cpp`
- `PortUiFunctionLibrary.h/.cpp`
- `PortBroadcastRouterActor.h/.cpp`
- `PortLogChannels.h/.cpp`

---

# 十四、最终命名模板建议

你可以直接按下面这个模板来命名：

## 数据结构
- `FPortWeatherState`
- `FPortKpiState`
- `FPortAlarmItem`
- `FPortBroadcastItem`
- `FPortTwinSnapshot`
- `FPortAiMessage`
- `FPortCommandRequest`

## 枚举
- `EPortAlarmSeverity`
- `EPortConnectionState`
- `EPortCommandKind`

## Service
- `UPortHttpService`
- `UPortWebSocketService`
- `UPortAiChatService`
- `UPortCommandService`

## Subsystem
- `UPortTwinRuntimeSubsystem`

## ViewModel
- `UPortDashboardViewModel`
- `UPortWeatherViewModel`
- `UPortAlarmViewModel`
- `UPortAiChatViewModel`

## Actor
- `APortTwinManagerActor`
- `APortBroadcastRouterActor`

## UI 工具
- `UPortUiFunctionLibrary`
- `UPortWidgetBinder`

---

# 十五、最后给你的建议

如果你现在就准备在 UE5.7.3 里真正开始写代码，我建议你就按下面顺序建类：

1. `PortTwinTypes.h`
2. `PortTwinSnapshot.h`
3. `PortHttpService`
4. `PortWebSocketService`
5. `PortTwinRuntimeSubsystem`
6. `PortDashboardViewModel`
7. `PortAiChatViewModel`
8. `PortBroadcastRouterActor`

这样你最短时间就能先打通：
- 快照显示
- AI 聊天
- 实时播报
- UMG 原生刷新
- 场景联动

如果你愿意，我下一步还可以继续给你补：

1. **这些类的 `.h/.cpp` 空骨架模板**
2. **`PortTwinRuntimeSubsystem` 的代码起手模板**
3. **`PortHttpService` / `PortWebSocketService` 的 UE5 C++ 示例骨架**
