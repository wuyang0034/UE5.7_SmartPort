#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "PortTwinTypes.h"
#include "PortHttpService.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FPortSnapshotResponseNative, const FPortTwinSnapshot&);
DECLARE_MULTICAST_DELEGATE_OneParam(FPortChatResponseNative, const FPortAiChatResponse&);
DECLARE_MULTICAST_DELEGATE_OneParam(FPortCommandAckNative, const FPortCommandAck&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPortHttpErrorNative, const FString& /*Stage*/, const FString& /*Message*/);

UCLASS(BlueprintType)
class UPortHttpService : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(const FString& InApiBaseUrl);

    void RequestSnapshot();
    void SendChatMessage(const FString& UserText, const FString& SessionId);
    void ExecuteCommand(EPortCommandKind Kind, const FString& Command);

    FPortSnapshotResponseNative OnSnapshotResponse;
    FPortChatResponseNative OnChatResponse;
    FPortCommandAckNative OnCommandAck;
    FPortHttpErrorNative OnHttpError;

private:
    FString ApiBaseUrl;

    void OnSnapshotHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
    void OnChatHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
    void OnCommandHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

    bool TryParseSnapshotJson(const FString& JsonText, FPortTwinSnapshot& OutSnapshot) const;
    bool TryParseChatJson(const FString& JsonText, FPortAiChatResponse& OutResponse) const;
    bool TryParseCommandAckJson(const FString& JsonText, FPortCommandAck& OutAck) const;
};
