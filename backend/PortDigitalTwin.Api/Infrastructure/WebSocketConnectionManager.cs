using System.Collections.Concurrent;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;

namespace PortDigitalTwin.Api.Infrastructure;

public sealed class WebSocketConnectionManager
{
    private readonly ConcurrentDictionary<Guid, WebSocket> _sockets = new();

    public Guid AddSocket(WebSocket socket)
    {
        var id = Guid.NewGuid();
        _sockets[id] = socket;
        return id;
    }

    public async Task RemoveSocketAsync(Guid id)
    {
        if (_sockets.TryRemove(id, out var socket))
        {
            if (socket.State is WebSocketState.Open or WebSocketState.CloseReceived)
            {
                await socket.CloseAsync(WebSocketCloseStatus.NormalClosure, "closed", CancellationToken.None);
            }

            socket.Dispose();
        }
    }

    public async Task BroadcastAsync(object payload, CancellationToken cancellationToken)
    {
        var json = JsonSerializer.Serialize(payload);
        var buffer = Encoding.UTF8.GetBytes(json);
        var segment = new ArraySegment<byte>(buffer);
        var deadSockets = new List<Guid>();

        foreach (var pair in _sockets)
        {
            if (pair.Value.State != WebSocketState.Open)
            {
                deadSockets.Add(pair.Key);
                continue;
            }

            try
            {
                await pair.Value.SendAsync(segment, WebSocketMessageType.Text, true, cancellationToken);
            }
            catch
            {
                deadSockets.Add(pair.Key);
            }
        }

        foreach (var deadSocket in deadSockets)
        {
            await RemoveSocketAsync(deadSocket);
        }
    }
}
