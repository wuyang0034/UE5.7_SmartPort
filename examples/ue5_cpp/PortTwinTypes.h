#pragma once

#include "CoreMinimal.h"
#include "PortTwinTypes.generated.h"

UENUM(BlueprintType)
enum class EPortConnectionState : uint8
{
    Disconnected UMETA(DisplayName = "Disconnected"),
    Connecting UMETA(DisplayName = "Connecting"),
    Connected UMETA(DisplayName = "Connected"),
    Error UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EPortAlarmSeverity : uint8
{
    Info UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EPortCommandKind : uint8
{
    Scene UMETA(DisplayName = "Scene"),
    Device UMETA(DisplayName = "Device")
};

USTRUCT(BlueprintType)
struct FPortWeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Rainfall = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VisibilityKm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TemperatureC = 0;
};

USTRUCT(BlueprintType)
struct FPortKpiState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 VesselsInPort = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DailyTeu = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OnlineDevices = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HighPriorityAlarms = 0;
};

USTRUCT(BlueprintType)
struct FPortAlarmItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPortAlarmSeverity Severity = EPortAlarmSeverity::Info;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Source;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Timestamp;
};

USTRUCT(BlueprintType)
struct FPortBroadcastItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPortAlarmSeverity Severity = EPortAlarmSeverity::Info;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Timestamp;
};

USTRUCT(BlueprintType)
struct FPortAiMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Timestamp;
};

USTRUCT(BlueprintType)
struct FPortCommandAck
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CommandId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Command;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Timestamp;
};

USTRUCT(BlueprintType)
struct FPortTwinSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PortName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPortWeatherState Weather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPortKpiState Kpi;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPortAlarmItem> Alarms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPortBroadcastItem> Broadcasts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Timestamp;
};

USTRUCT(BlueprintType)
struct FPortAiChatResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SessionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Reply;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SuggestedActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPortTwinSnapshot Snapshot;
};
