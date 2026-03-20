namespace PortDigitalTwin.Api.Models;

public sealed class RouteStatus
{
    public string Name { get; init; } = string.Empty;
    public string Vessel { get; init; } = string.Empty;
    public string Eta { get; init; } = string.Empty;
    public string Status { get; init; } = string.Empty;
}

public sealed class TyphoonPoint
{
    public int X { get; init; }
    public int Y { get; init; }
    public string TimeLabel { get; init; } = string.Empty;
    public double WindSpeed { get; init; }
}

public sealed class YardStatus
{
    public string Area { get; init; } = string.Empty;
    public int ContainerCount { get; set; }
    public int Capacity { get; init; }
    public double Occupancy => Capacity == 0 ? 0 : Math.Round(ContainerCount * 100d / Capacity, 1);
}

public sealed class WeatherStatus
{
    public double WindSpeed { get; set; }
    public double Rainfall { get; set; }
    public double VisibilityKm { get; set; }
    public string Condition { get; set; } = "多云";
    public int TemperatureC { get; set; }
}

public sealed class PortMetrics
{
    public int VesselsInPort { get; set; }
    public int DailyTeu { get; set; }
    public int OnlineDevices { get; set; }
    public int HighPriorityAlarms { get; set; }
}

public sealed class AlarmItem
{
    public Guid Id { get; init; } = Guid.NewGuid();
    public string Severity { get; init; } = "info";
    public string Source { get; init; } = string.Empty;
    public string Message { get; init; } = string.Empty;
    public DateTimeOffset Timestamp { get; init; } = DateTimeOffset.UtcNow;
}

public sealed class BroadcastMessage
{
    public string Type { get; init; } = string.Empty;
    public string Title { get; init; } = string.Empty;
    public string Content { get; init; } = string.Empty;
    public string Severity { get; init; } = "info";
    public DateTimeOffset Timestamp { get; init; } = DateTimeOffset.UtcNow;
}

public sealed class TwinSnapshot
{
    public string PortName { get; init; } = string.Empty;
    public PortMetrics Kpi { get; init; } = new();
    public WeatherStatus Weather { get; init; } = new();
    public IReadOnlyList<RouteStatus> Routes { get; init; } = Array.Empty<RouteStatus>();
    public IReadOnlyList<TyphoonPoint> TyphoonTrack { get; init; } = Array.Empty<TyphoonPoint>();
    public IReadOnlyList<YardStatus> Yard { get; init; } = Array.Empty<YardStatus>();
    public IReadOnlyList<AlarmItem> Alarms { get; init; } = Array.Empty<AlarmItem>();
    public IReadOnlyList<BroadcastMessage> Broadcasts { get; init; } = Array.Empty<BroadcastMessage>();
    public DateTimeOffset Timestamp { get; init; } = DateTimeOffset.UtcNow;
}

public sealed class AgentOptions
{
    public string Provider { get; init; } = "Ollama";
    public string BaseUrl { get; init; } = "http://127.0.0.1:11434";
    public string Model { get; init; } = "qwen2.5:3b-instruct";
    public string SystemPrompt { get; init; } = string.Empty;
}

public sealed class DigitalTwinOptions
{
    public string PortName { get; init; } = "Smart Port Alpha";
    public int BroadcastIntervalSeconds { get; init; } = 8;
    public double TyphoonAlertWindSpeed { get; init; } = 18;
    public double HighRiskWindSpeed { get; init; } = 25;
}
