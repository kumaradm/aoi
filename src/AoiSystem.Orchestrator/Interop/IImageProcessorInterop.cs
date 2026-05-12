namespace AoiSystem.Orchestrator.Interop;

public interface IImageProcessorInterop : IDisposable
{
    int InspectSlice(int y);
    void Dispose();
}