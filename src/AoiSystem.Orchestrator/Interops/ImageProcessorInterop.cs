using System;
using System.Runtime.InteropServices;

namespace AoiSystem.Orchestrator.Interop;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct Detection
{
    public float X;
    public float Y;
    public float W;
    public float H;
    public float Score;
    public float ClassId;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string ClassName;
}

public sealed class ImageProcessorInterop
{
    private const string LibPath = "libimage_processor.so"; 

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr Init(
        [MarshalAs(UnmanagedType.LPStr)] string cameraId,
        [MarshalAs(UnmanagedType.LPStr)] string enginePath);
    
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern int Inspect(
        IntPtr ctx,
        [Out] Detection[] outDetection,
        [MarshalAs(UnmanagedType.I1)] bool resize);
    
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool SaveCurrentFrame(
        IntPtr ctx,
        [MarshalAs(UnmanagedType.LPStr)] string filePath);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool SaveInspectedFrame(
        IntPtr ctx,
        [MarshalAs(UnmanagedType.LPStr)] string filePath,
        [MarshalAs(UnmanagedType.I1)] bool resize);
    
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Deinit(IntPtr ctx);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr GetLastError(IntPtr ctx);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool TriggerInspect(IntPtr ctx);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetLastResult(
        IntPtr      ctx,
        [Out] Detection[] outDetections,
        int         maxDetections,
        out long    outTimestamp);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool SaveLastInspectedFrame(
        IntPtr ctx,
        [MarshalAs(UnmanagedType.LPStr)] string filePath);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool IsResultReady(IntPtr ctx);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ConsumeResult(IntPtr ctx);
}