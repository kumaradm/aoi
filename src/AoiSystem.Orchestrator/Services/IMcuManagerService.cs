using System.IO.Ports;

namespace AoiSystem.Orchestrator.Services;

public interface IMcuManagerService
{   
    Task WaitingDataAsync(TimeSpan timeout, CancellationToken ct);
    Task SendCommand(string command);
}