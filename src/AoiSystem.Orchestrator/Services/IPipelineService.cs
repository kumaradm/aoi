namespace AoiSystem.Orchestrator.Services;

public interface IPipelineService
{
    void Start();
    Task ScanBoardAsync(CancellationToken cancellationToken);
    Task ProcessAsync(CancellationToken cancellationToken);
}