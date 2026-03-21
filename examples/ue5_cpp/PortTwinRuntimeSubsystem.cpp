#include "PortTwinRuntimeSubsystem.h"

#include "PortHttpService.h"
#include "PortWebSocketService.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UPortTwinRuntimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    SessionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
    HttpService = NewObject<UPortHttpService>(this);
    WebSocketService = NewObject<UPortWebSocketService>(this);
    BindServiceCallbacks();
}

void UPortTwinRuntimeSubsystem::Deinitialize()
{
    DisconnectRealtimeChannel();
    Super::Deinitialize();
}

void UPortTwinRuntimeSubsystem::InitializeRuntime(const FString& InApiBaseUrl, const FString& InWsUrl)
{
    ApiBaseUrl = InApiBaseUrl;
    WsUrl = InWsUrl;

    if (HttpService)
    {
        HttpService->Initialize(ApiBaseUrl);
    }

    if (WebSocketService)
    {
        WebSocketService->Initialize(WsUrl);
    }
}

void UPortTwinRuntimeSubsystem::RequestSnapshot()
{
    if (HttpService)
    {
        HttpService->RequestSnapshot();
    }
}

void UPortTwinRuntimeSubsystem::ConnectRealtimeChannel()
{
    if (!WebSocketService)
    {
        return;
    }

    SetConnectionState(EPortConnectionState::Connecting);
    WebSocketService->Connect();
}

void UPortTwinRuntimeSubsystem::DisconnectRealtimeChannel()
{
    if (WebSocketService)
    {
        WebSocketService->Disconnect();
    }

    SetConnectionState(EPortConnectionState::Disconnected);
}

void UPortTwinRuntimeSubsystem::SendChatMessage(const FString& UserText)
{
    if (HttpService)
    {
        HttpService->SendChatMessage(UserText, SessionId);
    }
}

void UPortTwinRuntimeSubsystem::ExecuteSceneCommand(const FString& Command)
{
    if (HttpService)
    {
        HttpService->ExecuteCommand(EPortCommandKind::Scene, Command);
    }
}

void UPortTwinRuntimeSubsystem::ExecuteDeviceCommand(const FString& Command)
{
    if (HttpService)
    {
        HttpService->ExecuteCommand(EPortCommandKind::Device, Command);
    }
}

void UPortTwinRuntimeSubsystem::SetConnectionState(EPortConnectionState NewState)
{
    ConnectionState = NewState;
    OnConnectionStateChanged.Broadcast(NewState);
}

void UPortTwinRuntimeSubsystem::BindServiceCallbacks()
{
    if (HttpService)
    {
        HttpService->OnSnapshotResponse.AddLambda([this](const FPortTwinSnapshot& Snapshot)
        {
            CurrentSnapshot = Snapshot;
            OnSnapshotUpdated.Broadcast(CurrentSnapshot);
        });

        HttpService->OnChatResponse.AddLambda([this](const FPortAiChatResponse& Reply)
        {
            CurrentSnapshot = Reply.Snapshot;
            OnChatReplyReceived.Broadcast(Reply);
            OnSnapshotUpdated.Broadcast(CurrentSnapshot);
        });

        HttpService->OnCommandAck.AddLambda([this](const FPortCommandAck& Ack)
        {
            OnCommandAckReceived.Broadcast(Ack);
        });

        HttpService->OnHttpError.AddLambda([this](const FString& Stage, const FString& Message)
        {
            SetConnectionState(EPortConnectionState::Error);
        });
    }

    if (WebSocketService)
    {
        WebSocketService->OnConnected.AddLambda([this]()
        {
            SetConnectionState(EPortConnectionState::Connected);
        });

        WebSocketService->OnDisconnected.AddLambda([this](int32 StatusCode)
        {
            SetConnectionState(EPortConnectionState::Disconnected);
        });

        WebSocketService->OnMessageReceived.AddLambda([this](const FString& Payload)
        {
            HandleRealtimeMessage(Payload);
        });

        WebSocketService->OnError.AddLambda([this](const FString& Error)
        {
            SetConnectionState(EPortConnectionState::Error);
        });
    }
}

void UPortTwinRuntimeSubsystem::HandleRealtimeMessage(const FString& Payload)
{
    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        return;
    }

    const FString MessageType = RootObject->GetStringField(TEXT("type"));

    if (MessageType == TEXT("snapshot"))
    {
        if (const TSharedPtr<FJsonObject>* BroadcastObject = nullptr; RootObject->TryGetObjectField(TEXT("broadcast"), BroadcastObject))
        {
            FPortBroadcastItem Broadcast;
            Broadcast.Title = (*BroadcastObject)->GetStringField(TEXT("title"));
            Broadcast.Content = (*BroadcastObject)->GetStringField(TEXT("content"));
            Broadcast.Timestamp = (*BroadcastObject)->GetStringField(TEXT("timestamp"));
            OnBroadcastReceived.Broadcast(Broadcast);
        }

        // 这里保留给你：后续可以把 payload.snapshot 进一步完整解析回 CurrentSnapshot。
    }
    else if (MessageType == TEXT("commandAck"))
    {
        if (const TSharedPtr<FJsonObject>* PayloadObject = nullptr; RootObject->TryGetObjectField(TEXT("payload"), PayloadObject))
        {
            FPortCommandAck Ack;
            Ack.Command = (*PayloadObject)->GetStringField(TEXT("command"));
            Ack.Status = (*PayloadObject)->GetStringField(TEXT("status"));
            Ack.Timestamp = (*PayloadObject)->GetStringField(TEXT("timestamp"));
            OnCommandAckReceived.Broadcast(Ack);
        }
    }
}
