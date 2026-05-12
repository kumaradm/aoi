using System;
using System.Runtime.InteropServices;

namespace AoiSystem.Orchestrator.Interop;

public sealed class ImageProcessorInterop : IImageProcessorInterop
{
    private const string LibPath = "/home/kumaradm/portfolio/aoi-on-jetson/src/AoiSystem.ImageProcessor/libs/libimage_processor.so"; 

    [DllImport(LibPath)]
    private static extern IntPtr InitializeAOI(string imagePath, string modelPath);

    [DllImport(LibPath)]
    private static extern int ProcessSlice(IntPtr ctx, int conveyorY);

    [DllImport(LibPath)]
    private static extern void DeinitAOI(IntPtr ctx);

    private IntPtr _context;

    public ImageProcessorInterop(string imagePath, string modelPath)
    {
        _context = InitializeAOI(imagePath, modelPath);
        if (_context == IntPtr.Zero) 
            throw new Exception("Failed to initialize AOI GPU Context.");
    }

    public int InspectSlice(int y) => ProcessSlice(_context, y);

    public void Dispose()
    {
        if (_context != IntPtr.Zero) DeinitAOI(_context);
    }
}