using System.Collections.Concurrent;
using PortDigitalTwin.Api.Models;

namespace PortDigitalTwin.Api.Services;

public sealed class PortStateStore
{
    private readonly object _gate = new();
    private readonly ConcurrentQueue<BroadcastMessage> _broadcasts = new();
    private readonly ConcurrentQueue<AlarmItem> _alarms = new();

    private readonly List<RouteStatus> _routes =
    [
        new() { Name = "洋山 -> 新加坡", Vessel = "MV COSCO NEBULA", Eta = "13:20", Status = "在途" },
        new() { Name = "宁波 -> 釜山", Vessel = "MV HANSEATIC STAR", Eta = "14:05", Status = "待靠泊" },
        new() { Name = "青岛 -> 洛杉矶", Vessel = "MV PACIFIC LANE", Eta = "16:40", Status = "装卸中" }
    ];

    private readonly List<TyphoonPoint> _typhoonTrack =
    [
        new() { X = 42, Y = 136, TimeLabel = "08:00", WindSpeed = 14 },
        new() { X = 80, Y = 120, TimeLabel = "11:00", WindSpeed = 16 },
        new() { X = 128, Y = 97, TimeLabel = "14:00", WindSpeed = 19 },
        new() { X = 182, Y = 84, TimeLabel = "17:00", WindSpeed = 22 },
        new() { X = 246, Y = 66, TimeLabel = "20:00", WindSpeed = 24 }
    ];

    private readonly List<YardStatus> _yard =
    [
        new() { Area = "A1", ContainerCount = 1480, Capacity = 2200 },
        new() { Area = "A2", ContainerCount = 2055, Capacity = 2350 },
        new() { Area = "B1", ContainerCount = 1268, Capacity = 2000 },
        new() { Area = "B2", ContainerCount = 1640, Capacity = 2080 }
    ];

    private readonly PortMetrics _kpi = new()
    {
        VesselsInPort = 12,
        DailyTeu = 18420,
        OnlineDevices = 67,
        HighPriorityAlarms = 1
    };

    private readonly WeatherStatus _weather = new()
    {
        WindSpeed = 17.2,
        Rainfall = 5.6,
        VisibilityKm = 11.2,
        Condition = "阵雨",
        TemperatureC = 28
    };

    public PortStateStore()
    {
        PushAlarm(new AlarmItem
        {
            Severity = "warning",
            Source = "WeatherEngine",
            Message = "东码头阵风增强，请关注桥吊防风状态。"
        });
    }

    public TwinSnapshot GetSnapshot(string portName)
    {
        lock (_gate)
        {
            return new TwinSnapshot
            {
                PortName = portName,
                Kpi = new PortMetrics
                {
                    VesselsInPort = _kpi.VesselsInPort,
                    DailyTeu = _kpi.DailyTeu,
                    OnlineDevices = _kpi.OnlineDevices,
                    HighPriorityAlarms = _kpi.HighPriorityAlarms
                },
                Weather = new WeatherStatus
                {
                    WindSpeed = _weather.WindSpeed,
                    Rainfall = _weather.Rainfall,
                    VisibilityKm = _weather.VisibilityKm,
                    Condition = _weather.Condition,
                    TemperatureC = _weather.TemperatureC
                },
                Routes = _routes.Select(x => new RouteStatus
                {
                    Name = x.Name,
                    Vessel = x.Vessel,
                    Eta = x.Eta,
                    Status = x.Status
                }).ToList(),
                TyphoonTrack = _typhoonTrack.Select(x => new TyphoonPoint
                {
                    X = x.X,
                    Y = x.Y,
                    TimeLabel = x.TimeLabel,
                    WindSpeed = x.WindSpeed
                }).ToList(),
                Yard = _yard.Select(x => new YardStatus
                {
                    Area = x.Area,
                    Capacity = x.Capacity,
                    ContainerCount = x.ContainerCount
                }).ToList(),
                Alarms = _alarms.ToArray(),
                Broadcasts = _broadcasts.ToArray(),
                Timestamp = DateTimeOffset.UtcNow
            };
        }
    }

    public void ApplyWeatherTick(Random random, DigitalTwinOptions options)
    {
        lock (_gate)
        {
            _weather.WindSpeed = Math.Round(Math.Clamp(_weather.WindSpeed + random.NextDouble() * 4 - 1.8, 9, 32), 1);
            _weather.Rainfall = Math.Round(Math.Clamp(_weather.Rainfall + random.NextDouble() * 2 - 0.5, 0, 35), 1);
            _weather.VisibilityKm = Math.Round(Math.Clamp(13 - (_weather.Rainfall * 0.12) - (_weather.WindSpeed * 0.08), 1.8, 15), 1);
            _weather.Condition = _weather.Rainfall > 18 ? "暴雨" : _weather.Rainfall > 8 ? "强降雨" : _weather.WindSpeed > 20 ? "大风" : "多云转阵雨";

            _kpi.DailyTeu += random.Next(12, 60);
            _kpi.OnlineDevices = Math.Clamp(_kpi.OnlineDevices + random.Next(-1, 2), 62, 69);

            foreach (var item in _yard)
            {
                item.ContainerCount = Math.Clamp(item.ContainerCount + random.Next(-18, 22), 900, item.Capacity);
            }

            var latestPoint = _typhoonTrack[^1];
            _typhoonTrack.RemoveAt(0);
            _typhoonTrack.Add(new TyphoonPoint
            {
                X = Math.Clamp(latestPoint.X + random.Next(6, 18), 48, 282),
                Y = Math.Clamp(latestPoint.Y + random.Next(-12, 4), 20, 150),
                TimeLabel = DateTimeOffset.UtcNow.ToLocalTime().AddHours(3).ToString("HH:mm"),
                WindSpeed = Math.Round(Math.Clamp(latestPoint.WindSpeed + random.NextDouble() * 2.4 - 0.3, 12, 30), 1)
            });

            if (_weather.WindSpeed >= options.TyphoonAlertWindSpeed)
            {
                PushAlarm(new AlarmItem
                {
                    Severity = _weather.WindSpeed >= options.HighRiskWindSpeed ? "critical" : "warning",
                    Source = "TyphoonSimulator",
                    Message = $"模拟台风外圈影响增强，当前风速 {_weather.WindSpeed}m/s，建议限制岸桥高空作业。"
                });
            }

            _kpi.HighPriorityAlarms = _alarms.Count(x => x.Severity is "critical" or "warning");
        }
    }

    public void PushAlarm(AlarmItem alarm)
    {
        _alarms.Enqueue(alarm);
        while (_alarms.Count > 8 && _alarms.TryDequeue(out _))
        {
        }
    }

    public void PushBroadcast(BroadcastMessage broadcast)
    {
        _broadcasts.Enqueue(broadcast);
        while (_broadcasts.Count > 8 && _broadcasts.TryDequeue(out _))
        {
        }
    }
}
