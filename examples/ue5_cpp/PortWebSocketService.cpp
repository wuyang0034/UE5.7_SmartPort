#include "PortWebSocketService.h"

#include "IWebSocket.h"
#include "WebSocketsModule.h"

void UPortWebSocketService::Initialize(const FString& InWsUrl)
{
    WsUrl = InWsUrl;
}

void UPortWebSocketService::Connect()
{
    if (WsUrl.IsEmpty())
    {
        OnError.Broadcast(TEXT("WebSocket URL is empty."));
        return;
    }

    if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
    {
        FModuleManager::Get().LoadModule(TEXT("WebSockets"));
    }

    Socket = FWebSocketsModule::Get().CreateWebSocket(WsUrl);
    BindSocketEvents();
    Socket->Connect();
}

void UPortWebSocketService::Disconnect()
{
    if (Socket.IsValid())
    {
        Socket->Close();
        Socket.Reset();
    }
}

void UPortWebSocketService::SendText(const FString& Payload)
{
    if (Socket.IsValid() && Socket->IsConnected())
    {
        Socket->Send(Payload);
    }
}

bool UPortWebSocketService::IsConnected() const
{
    return Socket.IsValid() && Socket->IsConnected();
}

void UPortWebSocketService::BindSocketEvents()
{
    if (!Socket.IsValid())
    {
        return;
    }

    Socket->OnConnected().AddLambda([this]()
    {
        OnConnected.Broadcast();
    });

    Socket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        OnDisconnected.Broadcast(StatusCode);
    });

    Socket->OnMessage().AddLambda([this](const FString& Payload)
    {
        OnMessageReceived.Broadcast(Payload);
    });

    Socket->OnConnectionError().AddLambda([this](const FString& Error)
    {
        OnError.Broadcast(Error);
    });
}
