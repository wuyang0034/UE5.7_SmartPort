#include "PortHttpService.h"

#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UPortHttpService::Initialize(const FString& InApiBaseUrl)
{
    ApiBaseUrl = InApiBaseUrl;
}

void UPortHttpService::RequestSnapshot()
{
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ApiBaseUrl / TEXT("twin/snapshot"));
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindUObject(this, &UPortHttpService::OnSnapshotHttpResponse);
    Request->ProcessRequest();
}

void UPortHttpService::SendChatMessage(const FString& UserText, const FString& SessionId)
{
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ApiBaseUrl / TEXT("chat"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    const FString Payload = FString::Printf(
        TEXT("{\"message\":\"%s\",\"sessionId\":\"%s\",\"includeSnapshot\":true}"),
        *UserText.ReplaceCharWithEscapedChar(),
        *SessionId.ReplaceCharWithEscapedChar());

    Request->SetContentAsString(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &UPortHttpService::OnChatHttpResponse);
    Request->ProcessRequest();
}

void UPortHttpService::ExecuteCommand(EPortCommandKind Kind, const FString& Command)
{
    const FString KindString = Kind == EPortCommandKind::Scene ? TEXT("scene") : TEXT("device");
    const FString CommandId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ApiBaseUrl / TEXT("commands/execute"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    const FString Payload = FString::Printf(
        TEXT("{\"commandId\":\"%s\",\"kind\":\"%s\",\"command\":\"%s\",\"source\":\"ue5-umg\",\"timestamp\":\"%s\"}"),
        *CommandId,
        *KindString,
        *Command.ReplaceCharWithEscapedChar(),
        *FDateTime::UtcNow().ToIso8601());

    Request->SetContentAsString(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &UPortHttpService::OnCommandHttpResponse);
    Request->ProcessRequest();
}

void UPortHttpService::OnSnapshotHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    if (!bConnectedSuccessfully || !Response.IsValid())
    {
        OnHttpError.Broadcast(TEXT("Snapshot"), TEXT("HTTP request failed."));
        return;
    }

    FPortTwinSnapshot Snapshot;
    if (TryParseSnapshotJson(Response->GetContentAsString(), Snapshot))
    {
        OnSnapshotResponse.Broadcast(Snapshot);
    }
    else
    {
        OnHttpError.Broadcast(TEXT("Snapshot"), TEXT("Snapshot JSON parse failed."));
    }
}

void UPortHttpService::OnChatHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    if (!bConnectedSuccessfully || !Response.IsValid())
    {
        OnHttpError.Broadcast(TEXT("Chat"), TEXT("HTTP request failed."));
        return;
    }

    FPortAiChatResponse ChatResponse;
    if (TryParseChatJson(Response->GetContentAsString(), ChatResponse))
    {
        OnChatResponse.Broadcast(ChatResponse);
    }
    else
    {
        OnHttpError.Broadcast(TEXT("Chat"), TEXT("Chat JSON parse failed."));
    }
}

void UPortHttpService::OnCommandHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
    if (!bConnectedSuccessfully || !Response.IsValid())
    {
        OnHttpError.Broadcast(TEXT("Command"), TEXT("HTTP request failed."));
        return;
    }

    FPortCommandAck Ack;
    if (TryParseCommandAckJson(Response->GetContentAsString(), Ack))
    {
        OnCommandAck.Broadcast(Ack);
    }
    else
    {
        Ack.Status = TEXT("accepted");
        Ack.Timestamp = FDateTime::UtcNow().ToIso8601();
        OnCommandAck.Broadcast(Ack);
    }
}

bool UPortHttpService::TryParseSnapshotJson(const FString& JsonText, FPortTwinSnapshot& OutSnapshot) const
{
    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        return false;
    }

    OutSnapshot.PortName = RootObject->GetStringField(TEXT("portName"));
    OutSnapshot.Timestamp = RootObject->GetStringField(TEXT("timestamp"));

    if (const TSharedPtr<FJsonObject>* WeatherObject = nullptr; RootObject->TryGetObjectField(TEXT("weather"), WeatherObject))
    {
        OutSnapshot.Weather.WindSpeed = static_cast<float>((*WeatherObject)->GetNumberField(TEXT("windSpeed")));
        OutSnapshot.Weather.Rainfall = static_cast<float>((*WeatherObject)->GetNumberField(TEXT("rainfall")));
        OutSnapshot.Weather.VisibilityKm = static_cast<float>((*WeatherObject)->GetNumberField(TEXT("visibilityKm")));
        OutSnapshot.Weather.Condition = (*WeatherObject)->GetStringField(TEXT("condition"));
        OutSnapshot.Weather.TemperatureC = (*WeatherObject)->GetIntegerField(TEXT("temperatureC"));
    }

    if (const TSharedPtr<FJsonObject>* KpiObject = nullptr; RootObject->TryGetObjectField(TEXT("kpi"), KpiObject))
    {
        OutSnapshot.Kpi.VesselsInPort = (*KpiObject)->GetIntegerField(TEXT("vesselsInPort"));
        OutSnapshot.Kpi.DailyTeu = (*KpiObject)->GetIntegerField(TEXT("dailyTeu"));
        OutSnapshot.Kpi.OnlineDevices = (*KpiObject)->GetIntegerField(TEXT("onlineDevices"));
        OutSnapshot.Kpi.HighPriorityAlarms = (*KpiObject)->GetIntegerField(TEXT("highPriorityAlarms"));
    }

    return true;
}

bool UPortHttpService::TryParseChatJson(const FString& JsonText, FPortAiChatResponse& OutResponse) const
{
    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        return false;
    }

    OutResponse.SessionId = RootObject->GetStringField(TEXT("sessionId"));
    OutResponse.Reply = RootObject->GetStringField(TEXT("reply"));

    const TArray<TSharedPtr<FJsonValue>>* SuggestedActions = nullptr;
    if (RootObject->TryGetArrayField(TEXT("suggestedActions"), SuggestedActions))
    {
        for (const TSharedPtr<FJsonValue>& Item : *SuggestedActions)
        {
            OutResponse.SuggestedActions.Add(Item->AsString());
        }
    }

    const TSharedPtr<FJsonObject>* SnapshotObject = nullptr;
    if (RootObject->TryGetObjectField(TEXT("snapshot"), SnapshotObject))
    {
        FString SnapshotJson;
        const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SnapshotJson);
        FJsonSerializer::Serialize((*SnapshotObject).ToSharedRef(), Writer);
        TryParseSnapshotJson(SnapshotJson, OutResponse.Snapshot);
    }

    return true;
}

bool UPortHttpService::TryParseCommandAckJson(const FString& JsonText, FPortCommandAck& OutAck) const
{
    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        return false;
    }

    OutAck.CommandId = RootObject->GetStringField(TEXT("commandId"));
    OutAck.Status = RootObject->GetStringField(TEXT("status"));
    return true;
}
