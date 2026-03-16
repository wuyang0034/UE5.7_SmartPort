# UE5 港口数字孪生前端 UI（方案 A）

这是一个**零构建依赖**的前端大屏模板，适合你当前只有 UE5.7 纯场景时快速上手。

## 快速运行

```bash
cd ui
python3 -m http.server 8080
```

浏览器打开 `http://localhost:8080`。

## 对接点（后期接 UE/C#）

在 `app.js` 中修改：

- `apiBaseUrl`：C# 后端命令接口（默认 `POST /commands/execute`）
- `wsUrl`：实时通道地址（默认 WebSocket）

```js
const appConfig = {
  apiBaseUrl: 'http://localhost:5000/api',
  wsUrl: 'ws://localhost:5000/ws'
};
```

## 已实现页面能力

- 左侧：航线列表、台风路径简图、告警列表
- 中间：UE5 背景层占位区（可替换 Pixel Streaming iframe）+ KPI 卡片
- 右侧：
  - 场景演示控制按钮（触发 UE 指令）
  - 硬件控制按钮（触发设备命令）
  - 堆场集装箱状态表格
  - 命令执行日志

## 推荐接入顺序

1. 先保持离线 mock 数据，确认 UI 交互流程。
2. 接入 C# `/commands/execute`。
3. 接入 WebSocket 实时推送。
4. 将 `#ueStage` 区域替换为 UE Pixel Streaming 画面。
