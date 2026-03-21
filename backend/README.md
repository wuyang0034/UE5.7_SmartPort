# PortDigitalTwin.Api

这是一个最小可运行的 **港口数字孪生 AI 中台** 示例，负责：

- 提供港口数字孪生状态快照：`GET /api/twin/snapshot`
- 提供 AI 智能体问答接口：`POST /api/chat`
- 提供命令下发接口：`POST /api/commands/execute`
- 通过 WebSocket 广播台风/天气/设备播报与状态：`GET /ws`
- 对接免费本地大模型：**Ollama**（推荐模型：`qwen2.5:3b-instruct`）

## 1. 运行环境

- .NET 8 SDK
- Ollama（可选，但推荐）

## 2. 启动免费 AI 模型

```bash
ollama serve
ollama pull qwen2.5:3b-instruct
```

> 若未安装 Ollama，后端会自动退化为内置规则引擎回复，不会阻塞 UI 演示。

## 3. 启动后端

```bash
cd backend/PortDigitalTwin.Api
dotnet restore
dotnet run
```

默认地址：`http://localhost:5000`（如果你的本地 `launchSettings.json` 配置不同，请同步修改 `ui/app.js` 中的地址）。

## 4. 接入 UE5/UMG 的建议

### 方案 A：当前仓库内的 Web 大屏叠加 UE5 场景
- UE5 使用 Pixel Streaming 输出画面。
- `ui/index.html` 放在独立浏览器或 WebView 中作为上层大屏。
- `#ueStage` 替换为 Pixel Streaming iframe。

### 方案 B：UE5 内嵌 WebView / UMG + HTTP/WebSocket
- UMG 中放置 Web Browser Widget 直接加载 `ui/index.html`。
- 蓝图或 C++ 侧接收 `/api/commands/execute` 返回的指令确认。
- UE5 自己也可作为 `/ws` 客户端订阅播报，驱动场景动画和语音字幕。

## 5. 推荐的 UE5 消息协议

### 场景命令
```json
{
  "commandId": "2e4c69a6-6a09-4e37-9dd7-571f4e6e40c5",
  "kind": "scene",
  "command": "StormWarningOn",
  "source": "port-ui",
  "timestamp": "2026-03-20T09:00:00Z"
}
```

### WebSocket 推送
```json
{
  "type": "snapshot",
  "payload": {
    "weather": { "windSpeed": 21.5, "rainfall": 12.6 },
    "kpi": { "vesselsInPort": 12, "dailyTeu": 18610 }
  },
  "broadcast": {
    "title": "AI 智能播报",
    "content": "模拟台风外圈影响增强，请限制岸桥高空作业。",
    "severity": "warning"
  }
}
```
