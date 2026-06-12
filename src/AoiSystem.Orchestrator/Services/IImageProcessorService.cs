using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using AoiSystem.Orchestrator.Context;
using AoiSystem.Orchestrator.Interop;

namespace AoiSystem.Orchestrator.Services;
/// <summary>
/// High-level service — one instance per AOI station.
/// Manages lifetime, MCU trigger, and result retrieval.
/// </summary>
public sealed class AoiService : IDisposable
{
    private readonly ImageProcessorContext _ctx;
    private readonly SemaphoreSlim _inspectionLock = new(1, 1);
    private bool _disposed = false;

    public string CameraId  { get; }
    public string ModelPath { get; }

    public AoiService(string cameraId, string modelPath)
    {
        CameraId  = cameraId;
        ModelPath = modelPath;
        _ctx      = new ImageProcessorContext(cameraId, modelPath);
    }

    /// <summary>
    /// Fires the MCU trigger, waits for inference to complete,
    /// and returns the result — all in one awaitable call.
    /// </summary>
    /// <param name="resize">
    ///     true  = whole frame scaled to 640x640 (fast)
    ///     false = sliding window tiling (catches small objects)
    /// </param>
    /// <param name="timeoutMs">
    ///     How long to wait for inference before giving up.
    ///     Default 10s — adjust based on your inference time.
    /// </param>
    /// <param name="ct">Cancellation token for graceful shutdown.</param>
    public async Task<InspectionResult> TriggerAndWaitAsync(
        bool              resize    = true,
        int               timeoutMs = 10_000,
        CancellationToken ct        = default)
    {
        // Only one inspection at a time — queue them if called concurrently
        await _inspectionLock.WaitAsync(ct);
        try
        {
            return await RunInspectionAsync(resize, timeoutMs, ct);
        }
        finally
        {
            _inspectionLock.Release();
        }
    }

    private async Task<InspectionResult> RunInspectionAsync(
        bool resize, int timeoutMs, CancellationToken ct)
    {
        // Clear any stale result from a previous trigger
        _ctx.ConsumeResult();

        // Fire the trigger — enqueues current frame for inference
        bool triggered = _ctx.TriggerInspect();
        if (!triggered)
            throw new InvalidOperationException(
                $"Failed to trigger inspection: {_ctx.GetLastError()}");

        // Wait for inference to complete
        using var timeoutCts  = new CancellationTokenSource(timeoutMs);
        using var linkedCts   = CancellationTokenSource.CreateLinkedTokenSource(
                                    ct, timeoutCts.Token);

        while (!linkedCts.Token.IsCancellationRequested)
        {
            if (_ctx.IsResultReady())
            {
                var detections = _ctx.GetLastResult(out long timestampMs);
                _ctx.ConsumeResult();

                return new InspectionResult
                {
                    Detections   = detections,
                    TimestampMs  = timestampMs,
                    CapturedAt   = DateTimeOffset
                                    .FromUnixTimeMilliseconds(timestampMs)
                                    .LocalDateTime,
                    HasDetections = detections.Length > 0
                };
            }

            await Task.Delay(10, linkedCts.Token);
        }

        // Distinguish timeout from cancellation
        if (ct.IsCancellationRequested)
            throw new OperationCanceledException("Inspection was cancelled.", ct);

        throw new TimeoutException(
            $"Inspection did not complete within {timeoutMs}ms.");
    }

    /// <summary>
    /// Saves the annotated frame from the last completed inspection.
    /// Call after TriggerAndWaitAsync returns.
    /// </summary>
    public bool SaveLastFrame(string filePath)
        => _ctx.SaveLastInspectedFrame(filePath);

    /// <summary>
    /// Saves the current raw camera frame without inference.
    /// </summary>
    public bool SaveCurrentFrame(string filePath)
        => _ctx.SaveCurrentFrame(filePath);

    public void Dispose()
    {
        if (!_disposed)
        {
            _inspectionLock.Dispose();
            _ctx.Dispose();
            _disposed = true;
        }
    }
}

/// <summary>
/// Result returned from a single inspection cycle.
/// </summary>
public sealed class InspectionResult
{
    public Detection[]  Detections    { get; init; } = Array.Empty<Detection>();
    public long         TimestampMs   { get; init; }
    public DateTime     CapturedAt    { get; init; }
    public bool         HasDetections { get; init; }

    public override string ToString()
    {
        if (!HasDetections) return $"[{CapturedAt:HH:mm:ss.fff}] No detections.";

        var lines = Detections.Select((d, i) =>
            $"  [{i}] {d.ClassName,-20} " +
            $"score={d.Score:F2}  " +
            $"box=({d.X:F0},{d.Y:F0}) {d.W:F0}x{d.H:F0}");

        return $"[{CapturedAt:HH:mm:ss.fff}] {Detections.Length} detection(s):\n" +
                string.Join("\n", lines);
    }
}
