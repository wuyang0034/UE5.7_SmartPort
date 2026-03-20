# UE5.7 Smart Port Digital Twin

本仓库现在包含一个 **港口数字孪生前端大屏**、一个 **免费 AI 智能体后端示例**，以及一套 **从 0 到可运行的超详细使用说明书**，用于把 UE5.7 港口码头场景升级为可交互、可播报、可联动的数字孪生系统。

## 你应该先看哪份文档

如果你现在最需要的是：

- 从头到尾一步一步照着做；
- 不想只看架构图，而是想看“具体每一步怎么操作”；
- 想知道如何把这套系统接进你现有的 UE5.7 + UMG 项目；

那么请优先阅读：

- `docs/ue5-port-ai-agent-manual.md`：**超详细操作手册（推荐先看）**

其次再看：

- `docs/ue5-port-ai-agent-solution.md`：总体方案说明
- `docs/ue5-port-digital-twin-csharp.md`：原始 C# 驱动 + UI 大屏方案说明

## 目录

- `ui/`：可直接运行的前端大屏，含 AI 智能体问答面板、天气卡片、台风路径、告警列表、AI 播报和控制中心。
- `backend/PortDigitalTwin.Api/`：.NET 8 最小后端，提供快照、聊天、命令和 WebSocket 广播，并支持对接免费 Ollama 模型。
- `docs/ue5-port-ai-agent-manual.md`：从 0 到运行的超详细操作说明书。
- `docs/ue5-port-ai-agent-solution.md`：免费 AI 智能体接入 + 数字孪生联动总体方案。

## 快速开始（极简版）

### 1. 启动前端

```bash
cd ui
python3 -m http.server 8080
```

打开：`http://localhost:8080`

### 2. 启动免费 AI 模型（推荐）

```bash
ollama serve
ollama pull qwen2.5:3b-instruct
```

### 3. 启动后端

```bash
cd backend/PortDigitalTwin.Api
dotnet restore
dotnet run
```

如果本机暂时没有安装 Ollama，后端会退化为规则引擎回复；如果本机暂时没有安装 .NET，前端仍然可以进行离线 mock 演示。
