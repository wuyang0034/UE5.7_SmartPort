# WBP_Dashboard 控件命名清单 + 蓝图变量命名表

> 目标：
>
> 给你一份可以直接照着建的 `WBP_Dashboard` 命名参考表，避免后面控件名、变量名、函数名越来越乱。

---

# 一、总命名原则

建议统一遵循：

```text
前缀 + 功能 + 含义
```

例如：
- `TextBlock_WeatherWind`
- `Button_SendChat`
- `ScrollBox_ChatMessages`
- `WrapBox_Suggestions`
- `Border_BroadcastBanner`

---

# 二、控件前缀建议

## 常见控件前缀
- `Canvas_`
- `Overlay_`
- `Border_`
- `Image_`
- `TextBlock_`
- `RichText_`
- `Button_`
- `EditableTextBox_`
- `ScrollBox_`
- `WrapBox_`
- `VerticalBox_`
- `HorizontalBox_`
- `ListView_`
- `SizeBox_`
- `Spacer_`

---

# 三、推荐的主界面控件命名清单

## 3.1 根节点与主布局
- `Canvas_Root`
- `Overlay_Main`
- `Border_MainGlass`
- `HorizontalBox_MainLayout`

## 3.2 顶部状态栏
- `Border_TopBar`
- `TextBlock_ScreenTitle`
- `TextBlock_ProjectSubtitle`
- `TextBlock_ConnectionStatus`
- `Button_ConnectBackend`

## 3.3 左侧天气与态势区
- `VerticalBox_LeftPanel`
- `Border_WeatherCard`
- `TextBlock_WeatherTitle`
- `TextBlock_WeatherCondition`
- `TextBlock_WeatherWind`
- `TextBlock_WeatherRain`
- `TextBlock_WeatherVisibility`
- `TextBlock_WeatherTemperature`
- `Border_TyphoonPanel`
- `TextBlock_TyphoonTitle`
- `Image_TyphoonMap`
- `Border_AlarmPanel`
- `TextBlock_AlarmTitle`
- `ScrollBox_AlarmList`

## 3.4 中间场景与播报区
- `Overlay_CenterStage`
- `Border_StageOverlay`
- `TextBlock_StageTitle`
- `TextBlock_StageSubtitle`
- `Border_BroadcastBanner`
- `TextBlock_BroadcastTitle`
- `RichText_BroadcastContent`
- `Border_KpiRow`
- `TextBlock_KpiVessels`
- `TextBlock_KpiDailyTeu`
- `TextBlock_KpiOnlineDevices`
- `TextBlock_KpiHighAlarms`

## 3.5 右侧 AI 面板
- `Border_AIChatPanel`
- `TextBlock_AIChatTitle`
- `TextBlock_AIProvider`
- `ScrollBox_ChatMessages`
- `WrapBox_Suggestions`
- `EditableTextBox_ChatInput`
- `Button_SendChat`
- `TextBlock_ChatStatus`

## 3.6 右侧命令控制区
- `Border_CommandPanel`
- `TextBlock_CommandPanelTitle`
- `Button_PlayCraneAnim`
- `Button_StartVesselIn`
- `Button_StormWarningOn`
- `Button_StopAllDemo`
- `Button_GateOpen`
- `Button_ConveyorStart`
- `Button_LightOn`
- `Button_EmergencyStop`
- `ScrollBox_CommandLog`

---

# 四、蓝图变量命名表

## 4.1 系统引用类变量
- `RuntimeSubsystem`
- `BroadcastRouterActor`
- `DashboardViewModel`
- `WeatherViewModel`
- `AlarmViewModel`
- `AiChatViewModel`

## 4.2 状态变量
- `CurrentSnapshot`
- `CurrentConnectionState`
- `CurrentSessionId`
- `LastBroadcast`
- `LatestAlarm`
- `bHasInitialized`
- `bIsSendingChat`

## 4.3 聊天相关变量
- `PendingChatInput`
- `ChatMessageWidgets`
- `SuggestionButtons`

## 4.4 命令相关变量
- `CommandLogEntries`
- `LastCommandName`
- `LastCommandStatus`

---

# 五、推荐的蓝图函数命名

## 初始化
- `InitializeDashboard`
- `BindRuntimeEvents`
- `SyncInitialSnapshot`

## UI 刷新
- `RefreshWeatherPanel`
- `RefreshKpiPanel`
- `RefreshAlarmList`
- `RefreshBroadcastBanner`
- `RefreshConnectionStatus`

## 聊天
- `SendChatFromInput`
- `AppendChatMessage`
- `RebuildSuggestionButtons`

## 命令
- `AppendCommandLog`
- `SendSceneCommand`
- `SendDeviceCommand`

## 联动
- `ApplyBroadcastVisualStyle`
- `HandleCriticalAlarmStyle`
- `HandleWarningAlarmStyle`

---

# 六、推荐的自定义事件命名

- `HandleSnapshotUpdated`
- `HandleBroadcastReceived`
- `HandleChatReplyReceived`
- `HandleCommandAckReceived`
- `HandleConnectionStateChanged`
- `HandleSuggestionClicked`

---

# 七、最容易踩坑的命名问题

## 不建议
- `Button1`
- `Text_1`
- `InputBox`
- `Status`
- `PanelA`

## 建议
- `Button_SendChat`
- `TextBlock_ConnectionStatus`
- `EditableTextBox_ChatInput`
- `Border_BroadcastBanner`
- `ScrollBox_CommandLog`

因为后期蓝图一大，只有规范命名才能快速定位。

---

# 八、推荐的最终整理方式

你可以把 `WBP_Dashboard` 的变量区整理成 4 组：

## System
- `RuntimeSubsystem`
- `CurrentConnectionState`
- `bHasInitialized`

## Snapshot
- `CurrentSnapshot`
- `LatestAlarm`
- `LastBroadcast`

## Chat
- `CurrentSessionId`
- `PendingChatInput`
- `bIsSendingChat`

## Command
- `LastCommandName`
- `LastCommandStatus`
- `CommandLogEntries`

这样后期维护会非常轻松。
