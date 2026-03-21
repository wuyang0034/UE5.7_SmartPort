# UE5 C++ 示例骨架文件

本目录提供一套 **可直接参考到 UE5.7.3 工程中的示例骨架**，用于原生 UMG 接入港口数字孪生后端。

## 包含文件

- `PortTwinTypes.h`：基础枚举与数据结构
- `PortHttpService.h/.cpp`：HTTP 快照/聊天/命令请求示例
- `PortWebSocketService.h/.cpp`：WebSocket 实时连接示例
- `PortTwinRuntimeSubsystem.h/.cpp`：统一调度入口示例

## 建议放入 UE 工程的位置

```text
Source/<YourProject>/Twin/
├─ Models/
│  └─ PortTwinTypes.h
├─ Services/
│  ├─ PortHttpService.h/.cpp
│  └─ PortWebSocketService.h/.cpp
└─ Subsystems/
   └─ PortTwinRuntimeSubsystem.h/.cpp
```

## `Build.cs` 建议模块

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "HTTP",
    "Json",
    "JsonUtilities",
    "WebSockets",
    "UMG"
});
```

> 说明：这些文件是“工程骨架模板”，用于帮助你快速起步；你需要根据自己项目中的模块名、命名空间、日志通道和数据结构做二次调整。
