namespace PortDigitalTwin.Api.Contracts;

public sealed record ChatRequest(string Message, string? SessionId, bool IncludeSnapshot = true);

public sealed record ChatResponse(
    string SessionId,
    string Reply,
    string Provider,
    string Model,
    DateTimeOffset Timestamp,
    IEnumerable<string> SuggestedActions,
    TwinSnapshot Snapshot);

public sealed record CommandRequest(
    Guid CommandId,
    string Kind,
    string Command,
    string Source,
    DateTimeOffset Timestamp,
    Dictionary<string, string>? Metadata = null);
