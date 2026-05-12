namespace AoiSystem.Orchestrator.Services;

public interface IMcuManagerService
{   
    void DataReceivedHandler();
    void SendCommand();
}