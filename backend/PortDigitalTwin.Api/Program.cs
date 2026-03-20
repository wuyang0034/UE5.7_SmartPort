using System.Net.WebSockets;
using System.Text;
using Microsoft.Extensions.Options;
using PortDigitalTwin.Api.Contracts;
using PortDigitalTwin.Api.Infrastructure;
using PortDigitalTwin.Api.Models;
using PortDigitalTwin.Api.Services;

var builder = WebApplication.CreateBuilder(args);

builder.Services.Configure<AgentOptions>(builder.Configuration.GetSection("Agent"));
builder.Services.Configure<DigitalTwinOptions>(builder.Configuration.GetSection("DigitalTwin"));
builder.Services.AddSingleton<PortStateStore>();
builder.Services.AddSingleton<WebSocketConnectionManager>();
builder.Services.AddHttpClient<AiAgentService>((sp, client) =>
{
    var options = sp.GetRequiredService<IOptions<AgentOptions>>().Value;
    client.BaseAddress = new Uri(options.BaseUrl);
    client.Timeout = TimeSpan.FromSeconds(12);
});
builder.Services.AddHostedService<BroadcastWorker>();
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
        policy.AllowAnyOrigin()
              .AllowAnyHeader()
              .AllowAnyMethod());
});

var app = builder.Build();
app.UseCors();
app.UseWebSockets();

app.MapGet("/api/twin/snapshot", (PortStateStore store, IOptions<DigitalTwinOptions> options) =>
    Results.Ok(store.GetSnapshot(options.Value.PortName)));

app.MapPost("/api/chat", async (ChatRequest request, AiAgentService agentService, PortStateStore store, IOptions<DigitalTwinOptions> options, CancellationToken cancellationToken) =>
{
    var snapshot = store.GetSnapshot(options.Value.PortName);
    var response = await agentService.AskAsync(request, snapshot, cancellationToken);
    return Results.Ok(response);
});

app.MapPost("/api/commands/execute", async (CommandRequest request, PortStateStore store, WebSocketConnectionManager ws, CancellationToken cancellationToken) =>
{
    var alarm = new AlarmItem
    {
        Severity = request.Command.Contains("Emergency", StringComparison.OrdinalIgnoreCase) ? "critical" : "info",
        Source = request.Kind,
        Message = $"收到命令 {request.Command}，来源 {request.Source}，已进入模拟执行队列。",
        Timestamp = request.Timestamp
    };

    store.PushAlarm(alarm);
    await ws.BroadcastAsync(new
    {
        type = "commandAck",
        payload = new
        {
            request.CommandId,
            request.Kind,
            request.Command,
            request.Source,
            status = "accepted",
            timestamp = DateTimeOffset.UtcNow
        }
    }, cancellationToken);

    return Results.Accepted($"/api/commands/{request.CommandId}", new
    {
        request.CommandId,
        status = "accepted"
    });
});

app.Map("/ws", async context =>
{
    if (!context.WebSockets.IsWebSocketRequest)
    {
        context.Response.StatusCode = StatusCodes.Status400BadRequest;
        return;
    }

    var manager = context.RequestServices.GetRequiredService<WebSocketConnectionManager>();
    var options = context.RequestServices.GetRequiredService<IOptions<DigitalTwinOptions>>().Value;
    var store = context.RequestServices.GetRequiredService<PortStateStore>();

    using var socket = await context.WebSockets.AcceptWebSocketAsync();
    var socketId = manager.AddSocket(socket);

    await manager.BroadcastAsync(new
    {
        type = "presence",
        payload = new { socketId, status = "joined" }
    }, context.RequestAborted);

    await socket.SendAsync(
        Encoding.UTF8.GetBytes(System.Text.Json.JsonSerializer.Serialize(new
        {
            type = "snapshot",
            payload = store.GetSnapshot(options.PortName)
        })),
        WebSocketMessageType.Text,
        true,
        context.RequestAborted);

    var buffer = new byte[4096];

    try
    {
        while (socket.State == WebSocketState.Open && !context.RequestAborted.IsCancellationRequested)
        {
            var result = await socket.ReceiveAsync(buffer, context.RequestAborted);
            if (result.MessageType == WebSocketMessageType.Close)
            {
                break;
            }
        }
    }
    finally
    {
        await manager.RemoveSocketAsync(socketId);
    }
});

app.Run();
