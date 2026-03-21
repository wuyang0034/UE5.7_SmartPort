#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PortWebSocketService.generated.h"

class IWebSocket;

DECLARE_MULTICAST_DELEGATE(FPortWebSocketConnectedNative);
DECLARE_MULTICAST_DELEGATE_OneParam(FPortWebSocketDisconnectedNative, int32 /*StatusCode*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FPortWebSocketMessageNative, const FString& /*Payload*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FPortWebSocketErrorNative, const FString& /*Error*/);

UCLASS(BlueprintType)
class UPortWebSocketService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(const FString& InWsUrl);

    void Connect();
    void Disconnect();
    void SendText(const FString& Payload);
    bool IsConnected() const;

    FPortWebSocketConnectedNative OnConnected;
    FPortWebSocketDisconnectedNative OnDisconnected;
    FPortWebSocketMessageNative OnMessageReceived;
    FPortWebSocketErrorNative OnError;

private:
    FString WsUrl;
    TSharedPtr<IWebSocket> Socket;

    void BindSocketEvents();
};
