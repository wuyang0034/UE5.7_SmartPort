# UE5.7 港口数字孪生 + 免费 AI 智能体完整方案

## 1. 目标

把当前“只有封面 UI 的 UE5 港口场景”升级为一个可演示、可扩展、可接真实数据的数字孪生系统，并引入一个免费 AI 大模型充当：

1. **值班智能体**：回答用户关于天气、台风、设备、堆场和吞吐的提问。
2. **智能播报员**：将台风、天气和后端模拟突发事件自动播报到 UI 大屏。
3. **联动决策助手**：根据规则生成处置建议，并可触发 UE5 场景联动命令。

---

## 2. 推荐技术路线

### 免费模型方案

优先建议使用 **Ollama + Qwen/Llama 本地模型**：

- 免费
- 可离线部署
- 不依赖商业 API Key
- 支持 OpenAI 风格的“系统提示词 + 用户问题 + 状态快照”模式

推荐模型：
- `qwen2.5:3b-instruct`：部署轻、中文效果好
- `qwen2.5:7b-instruct`：更强，但需要更高显存
- `llama3.2:3b`：英文环境备选

### 整体架构

```text
UE5.7 港口场景 / UMG 大屏
      ↓
Web UI 覆盖层（本仓库 ui）
      ↓
.NET 8 AI 中台（本仓库 backend）
      ├─ Mock 台风数据引擎
      ├─ 天气/堆场/告警状态引擎
      ├─ AI Agent Service
      ├─ Command API
      └─ WebSocket 广播服务
      ↓
Ollama 免费模型
```

---

## 3. 交互链路

### 3.1 用户问答

```text
用户在大屏中输入问题
  -> ui/app.js 调用 POST /api/chat
  -> 后端拼接实时数字孪生快照
  -> 调用 Ollama 模型生成回答
  -> 返回智能体答复 + 建议操作 + 当前快照
  -> UI 渲染聊天记录与建议按钮
```

### 3.2 自动播报

```text
BroadcastWorker 周期更新天气/台风/堆场数据
  -> 触发规则引擎判断风险等级
  -> AI Agent Service 生成播报文案
  -> WebSocket 推送 snapshot + broadcast
  -> UI 大屏 banner / 播报列表 / 聊天区同步显示
  -> UE5 也可订阅同一通道触发风雨特效与预警动画
```

### 3.3 控制联动

```text
用户点击大屏按钮
  -> POST /api/commands/execute
  -> 后端记录命令并回推 commandAck
  -> UE5 侧接收到 scene 命令，执行动画
  -> 硬件侧接收到 device 命令，执行设备动作
```

---

## 4. 你现在这个项目里已经补上的内容

### 前端（`ui/`）
- 新增 **AI 智能体值班席**
- 新增 **天气卡片**
- 新增 **AI 播报列表**
- 新增 **播报横幅 banner**
- 新增 **聊天输入、建议问题、模型提供方显示**
- 支持从后端拉取快照、接收 WebSocket 广播、发送命令、调用 AI 问答

### 后端（`backend/PortDigitalTwin.Api/`）
- `GET /api/twin/snapshot`
- `POST /api/chat`
- `POST /api/commands/execute`
- `GET /ws`
- `BroadcastWorker` 周期模拟台风、天气、堆场和告警变化
- `AiAgentService` 支持 **Ollama** 和 **Fallback 规则引擎** 两种模式

---

## 5. 如何在 UE5 中真正落地

### 方式一：继续使用你现在的 UMG 封面，嵌入 Web 内容

如果你现在的大屏已经是 UMG 做出来的，有两种最稳妥的补法：

#### 方案 1：UMG 中嵌 Web Browser Widget
- 在 UMG 内放一个 Web Browser Widget
- 加载 `http://127.0.0.1:8080`
- 将这个浏览器区域放在你的大屏右侧或底部作为 AI 智能体面板

#### 方案 2：UMG 原生 UI，仅通过 HTTP/WebSocket 接口取数
- 保持你当前 UMG 不变
- 把本仓库后端接口当成数据源
- 在蓝图/C++ 中：
  - 定时请求 `/api/twin/snapshot`
  - 用户点击发送 `/api/chat`
  - 实时订阅 `/ws`
- 把 AI 返回的文字显示到 UMG TextBlock / RichTextBlock

### 方式二：让 UE5 本身也订阅播报

建议 UE5 场景侧也订阅 `/ws`：

- 收到 `broadcast.severity = warning/critical`
  - 切换天空材质、雨效、风效
  - 打开警报灯
  - 高亮桥吊、闸机、泊位区域
- 收到 `commandAck`
  - 回显动画是否开始执行

---

## 6. 推荐的 UE5 蓝图/C++ 对接点

### 蓝图层
- `ConnectBackend`：连接 WebSocket
- `RequestSnapshot`：获取快照
- `AskAgent`：发送聊天问题
- `HandleBroadcast`：处理播报
- `HandleSceneCommand`：将 `StormWarningOn`、`PlayCraneAnim` 等映射为 Sequencer/动画蓝图事件

### C++ 插件层（后续增强）
- 自定义 `UPortTwinWebSocketComponent`
- 自定义 `UPortTwinHttpSubsystem`
- 将 JSON 数据转为结构体：
  - `FPortWeatherState`
  - `FTyphoonTrack`
  - `FAlarmItem`
  - `FAgentChatResponse`

---

## 7. 生产化建议

### 7.1 AI 安全
- 对智能体回答做长度限制
- 避免直接执行 AI 自主生成的控制指令
- 所有“执行类动作”必须通过规则引擎和人工确认

### 7.2 数据隔离
- 天气、台风、设备、堆场数据独立采集
- AI 只读取统一快照，不直接触碰原始设备控制通道

### 7.3 演示优先级
如果你现在先做演示，建议按下面顺序：

1. 启动前端大屏，先看离线 mock 演示
2. 启动 .NET 后端，打通实时快照和播报
3. 启动 Ollama，接入真正 AI 问答
4. 最后再把 UE5 场景命令和真实设备协议接上

---

## 8. 本仓库推荐启动顺序

### 启动前端
```bash
cd ui
python3 -m http.server 8080
```

### 启动免费模型
```bash
ollama serve
ollama pull qwen2.5:3b-instruct
```

### 启动后端
```bash
cd backend/PortDigitalTwin.Api
dotnet restore
dotnet run
```

---

## 9. 你下一步可以怎么继续

如果你愿意继续，我建议下一阶段直接做这 3 件事：

1. **把 UE5 的台风特效、雨效、风效和本仓库 `/ws` 联动起来**。
2. **把 UMG 的按钮真正接到 `/api/chat` 和 `/api/commands/execute`**。
3. **把模拟天气数据换成真实天气 API 或你自己的港口业务数据接口**。

这样你当前这个“纯场景项目”就能正式演化为一个“可交互、可播报、可联动”的港口数字孪生系统。
