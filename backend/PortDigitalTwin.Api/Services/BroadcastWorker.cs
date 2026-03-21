using Microsoft.Extensions.Options;
using PortDigitalTwin.Api.Infrastructure;
using PortDigitalTwin.Api.Models;

namespace PortDigitalTwin.Api.Services;

public sealed class BroadcastWorker : BackgroundService
{
    private readonly PortStateStore _stateStore;
    private readonly AiAgentService _agentService;
    private readonly WebSocketConnectionManager _connectionManager;
    private readonly DigitalTwinOptions _options;
    private readonly ILogger<BroadcastWorker> _logger;
    private readonly Random _random = new();

    public BroadcastWorker(
        PortStateStore stateStore,
        AiAgentService agentService,
        WebSocketConnectionManager connectionManager,
        IOptions<DigitalTwinOptions> options,
        ILogger<BroadcastWorker> logger)
    {
        _stateStore = stateStore;
        _agentService = agentService;
        _connectionManager = connectionManager;
        _options = options.Value;
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var timer = new PeriodicTimer(TimeSpan.FromSeconds(Math.Max(3, _options.BroadcastIntervalSeconds)));

        while (!stoppingToken.IsCancellationRequested && await timer.WaitForNextTickAsync(stoppingToken))
        {
            _stateStore.ApplyWeatherTick(_random, _options);
            var snapshot = _stateStore.GetSnapshot(_options.PortName);
            var narration = _agentService.BuildBroadcastNarration(snapshot);
            var severity = snapshot.Weather.WindSpeed >= _options.HighRiskWindSpeed ? "critical" : snapshot.Weather.WindSpeed >= _options.TyphoonAlertWindSpeed ? "warning" : "info";

            var broadcast = new BroadcastMessage
            {
                Type = "broadcast",
                Title = "AI 智能播报",
                Content = narration,
                Severity = severity,
                Timestamp = DateTimeOffset.UtcNow
            };

            _stateStore.PushBroadcast(broadcast);
            var latestSnapshot = _stateStore.GetSnapshot(_options.PortName);

            await _connectionManager.BroadcastAsync(new
            {
                type = "snapshot",
                payload = latestSnapshot,
                broadcast
            }, stoppingToken);

            _logger.LogInformation("Broadcast tick completed with wind speed {WindSpeed}", latestSnapshot.Weather.WindSpeed);
        }
    }
}
