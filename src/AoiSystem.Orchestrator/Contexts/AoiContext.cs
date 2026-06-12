using System;
using System.Runtime.InteropServices;
using AoiSystem.Orchestrator.Interop;

namespace AoiSystem.Orchestrator.Context;

public sealed class AoiContext : IDisposable
{
    private IntPtr _ctx;
    private bool   _disposed = false;

    public AoiContext(string cameraId, string modelPath)
    {
        _ctx = ImageProcessorInterop.Init(cameraId, modelPath);
        if (_ctx == IntPtr.Zero)
            throw new InvalidOperationException(
                "Failed to initialize AOI context. Check camera and model path.");
    }

    public bool TriggerInspect()
        => ImageProcessorInterop.TriggerInspect(_ctx);

    public bool IsResultReady()
        => ImageProcessorInterop.IsResultReady(_ctx);

    public void ConsumeResult()
        => ImageProcessorInterop.ConsumeResult(_ctx);

    public string GetLastError()
        => Marshal.PtrToStringAnsi(ImageProcessorInterop.GetLastError(_ctx)) ?? "Unknown error.";

    public Detection[] GetLastResult(out long timestampMs)
    {
        var buffer = new Detection[1000];
        int count  = ImageProcessorInterop.GetLastResult(_ctx, buffer, 1000, out timestampMs);

        if (count <= 0)
        {
            timestampMs = 0;
            return Array.Empty<Detection>();
        }

        return buffer.Take(count).ToArray();
    }

    public bool SaveLastInspectedFrame(string filePath)
        => ImageProcessorInterop.SaveLastInspectedFrame(_ctx, filePath);

    public bool SaveCurrentFrame(string filePath)
        => ImageProcessorInterop.SaveCurrentFrame(_ctx, filePath);

    public Detection[] Inspect(bool resize = true, int maxDetections = 1000)
    {
        var buffer = new Detection[maxDetections];
        int count  = ImageProcessorInterop.Inspect(_ctx, buffer, resize);

        if (count < 0)
            throw new InvalidOperationException(
                $"Inspect failed (code {count}): {GetLastError()}");

        return buffer.Take(count).ToArray();
    }

    public void Dispose()
    {
        if (!_disposed && _ctx != IntPtr.Zero)
        {
            ImageProcessorInterop.Deinit(_ctx);
            _ctx      = IntPtr.Zero;
            _disposed = true;
        }
    }
}