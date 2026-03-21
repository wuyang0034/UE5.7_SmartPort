using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Options;
using PortDigitalTwin.Api.Contracts;
using PortDigitalTwin.Api.Models;

namespace PortDigitalTwin.Api.Services;

public sealed class AiAgentService
{
    private readonly HttpClient _httpClient;
    private readonly AgentOptions _options;
    private readonly DigitalTwinOptions _twinOptions;
    private readonly ILogger<AiAgentService> _logger;

    public AiAgentService(HttpClient httpClient, IOptions<AgentOptions> options, IOptions<DigitalTwinOptions> twinOptions, ILogger<AiAgentService> logger)
    {
        _httpClient = httpClient;
        _options = options.Value;
        _twinOptions = twinOptions.Value;
        _logger = logger;
    }

    public async Task<ChatResponse> AskAsync(ChatRequest request, TwinSnapshot snapshot, CancellationToken cancellationToken)
    {
        var sessionId = string.IsNullOrWhiteSpace(request.SessionId) ? Guid.NewGuid().ToString("N") : request.SessionId!;
        var prompt = BuildPrompt(request.Message, snapshot);

        var (reply, provider) = await TryCallOllamaAsync(prompt, cancellationToken)
            ?? (BuildFallbackReply(request.Message, snapshot), "FallbackRuleEngine");

        return new ChatResponse(
            sessionId,
            reply,
            provider,
            _options.Model,
            DateTimeOffset.UtcNow,
            BuildSuggestedActions(snapshot),
            snapshot);
    }

    public string BuildBroadcastNarration(TwinSnapshot snapshot)
    {
        var latestAlarm = snapshot.Alarms.FirstOrDefault();
        var weather = snapshot.Weather;

        if (latestAlarm is not null && latestAlarm.Severity is "critical" or "warning")
        {
            return $"智能播报：{_twinOptions.PortName}当前出现{latestAlarm.Severity}级事件，{latestAlarm.Message} 当前风速{weather.WindSpeed}米每秒、降雨{weather.Rainfall}毫米，请值班人员立即检查相关设备。";
        }

        return $"智能播报：{_twinOptions.PortName}运行平稳，在港船舶{snapshot.Kpi.VesselsInPort}艘，今日吞吐{snapshot.Kpi.DailyTeu}TEU，当前天气{weather.Condition}，能见度{weather.VisibilityKm}公里。";
    }

    private async Task<(string Reply, string Provider)?> TryCallOllamaAsync(string prompt, CancellationToken cancellationToken)
    {
        if (!string.Equals(_options.Provider, "Ollama", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        try
        {
            using var response = await _httpClient.PostAsJsonAsync(
                "/api/chat",
                new
                {
                    model = _options.Model,
                    stream = false,
                    messages = new[]
                    {
                        new { role = "system", content = _options.SystemPrompt },
                        new { role = "user", content = prompt }
                    }
                },
                cancellationToken);

            if (!response.IsSuccessStatusCode)
            {
                _logger.LogWarning("Ollama returned HTTP {StatusCode}", response.StatusCode);
                return null;
            }

            using var doc = JsonDocument.Parse(await response.Content.ReadAsStringAsync(cancellationToken));
            var content = doc.RootElement.GetProperty("message").GetProperty("content").GetString();

            return string.IsNullOrWhiteSpace(content) ? null : (content, "Ollama");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to call Ollama; fallback reply will be used.");
            return null;
        }
    }

    private string BuildPrompt(string userMessage, TwinSnapshot snapshot)
    {
        var sb = new StringBuilder();
        sb.AppendLine("用户问题：" + userMessage);
        sb.AppendLine("以下是当前港口数字孪生状态快照，请基于这些数据回答：");
        sb.AppendLine(JsonSerializer.Serialize(snapshot, new JsonSerializerOptions { WriteIndented = false }));
        sb.AppendLine("回答要求：1）先给结论；2）若存在风险，给出处理建议；3）不确定时明确说明是模拟数据。 ");
        return sb.ToString();
    }

    private static string BuildFallbackReply(string userMessage, TwinSnapshot snapshot)
    {
        var weather = snapshot.Weather;
        var topAlarm = snapshot.Alarms.FirstOrDefault();

        if (userMessage.Contains("台风") || userMessage.Contains("天气"))
        {
            return $"当前为模拟天气联动数据：风速 {weather.WindSpeed}m/s，降雨 {weather.Rainfall}mm，能见度 {weather.VisibilityKm}km。" +
                   (topAlarm is null ? "当前未检测到高优先告警。" : $"最新告警为：{topAlarm.Message}") +
                   "建议在 UI 大屏上联动显示台风路径、风速变化和岸桥防风状态。";
        }

        if (userMessage.Contains("吞吐") || userMessage.Contains("堆场") || userMessage.Contains("箱"))
        {
            var busiest = snapshot.Yard.OrderByDescending(x => x.Occupancy).First();
            return $"今日吞吐为 {snapshot.Kpi.DailyTeu} TEU，当前最繁忙堆场是 {busiest.Area}，占用率 {busiest.Occupancy}% 。建议优先关注高占用区域的调度与通道拥塞。";
        }

        return $"我是港口数字孪生智能体。目前在港船舶 {snapshot.Kpi.VesselsInPort} 艘，在线设备 {snapshot.Kpi.OnlineDevices} 台，高优先告警 {snapshot.Kpi.HighPriorityAlarms} 条。你可以继续问我天气、台风、堆场、设备或联动处置建议。";
    }

    private static IEnumerable<string> BuildSuggestedActions(TwinSnapshot snapshot)
    {
        var actions = new List<string> { "查看台风处置建议", "汇总当前告警", "查询堆场占用率" };

        if (snapshot.Weather.WindSpeed >= 18)
        {
            actions.Insert(0, "启动台风预警演练");
        }

        return actions.Distinct().Take(4);
    }
}
