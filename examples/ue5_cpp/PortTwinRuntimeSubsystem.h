#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PortTwinTypes.h"
#include "PortTwinRuntimeSubsystem.generated.h"

class UPortHttpService;
class UPortWebSocketService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortSnapshotUpdatedDynamic, const FPortTwinSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortBroadcastReceivedDynamic, const FPortBroadcastItem&, Broadcast);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortChatReplyReceivedDynamic, const FPortAiChatResponse&, Reply);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortCommandAckReceivedDynamic, const FPortCommandAck&, Ack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPortConnectionStateChangedDynamic, EPortConnectionState, NewState);

UCLASS()
class UPortTwinRuntimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void InitializeRuntime(const FString& InApiBaseUrl, const FString& InWsUrl);

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void RequestSnapshot();

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void ConnectRealtimeChannel();

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void DisconnectRealtimeChannel();

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void SendChatMessage(const FString& UserText);

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void ExecuteSceneCommand(const FString& Command);

    UFUNCTION(BlueprintCallable, Category = "PortTwin")
    void ExecuteDeviceCommand(const FString& Command);

    UFUNCTION(BlueprintPure, Category = "PortTwin")
    FPortTwinSnapshot GetCurrentSnapshot() const { return CurrentSnapshot; }

    UFUNCTION(BlueprintPure, Category = "PortTwin")
    EPortConnectionState GetConnectionState() const { return ConnectionState; }

    UPROPERTY(BlueprintAssignable, Category = "PortTwin|Events")
    FPortSnapshotUpdatedDynamic OnSnapshotUpdated;

    UPROPERTY(BlueprintAssignable, Category = "PortTwin|Events")
    FPortBroadcastReceivedDynamic OnBroadcastReceived;

    UPROPERTY(BlueprintAssignable, Category = "PortTwin|Events")
    FPortChatReplyReceivedDynamic OnChatReplyReceived;

    UPROPERTY(BlueprintAssignable, Category = "PortTwin|Events")
    FPortCommandAckReceivedDynamic OnCommandAckReceived;

    UPROPERTY(BlueprintAssignable, Category = "PortTwin|Events")
    FPortConnectionStateChangedDynamic OnConnectionStateChanged;

private:
    UPROPERTY()
    UPortHttpService* HttpService = nullptr;

    UPROPERTY()
    UPortWebSocketService* WebSocketService = nullptr;

    UPROPERTY()
    FString ApiBaseUrl;

    UPROPERTY()
    FString WsUrl;

    UPROPERTY()
    FString SessionId;

    UPROPERTY()
    FPortTwinSnapshot CurrentSnapshot;

    UPROPERTY()
    EPortConnectionState ConnectionState = EPortConnectionState::Disconnected;

    void SetConnectionState(EPortConnectionState NewState);
    void BindServiceCallbacks();
    void HandleRealtimeMessage(const FString& Payload);
};
