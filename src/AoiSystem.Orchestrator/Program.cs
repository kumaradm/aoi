using System;
using System.IO;
using Microsoft.Extensions.Configuration;
using AoiSystem.Orchestrator.Interop;

class Program
{
    static void Main(string[] args)
    {
        string projectRoot = GetProjectRoot();

        var config = new ConfigurationBuilder()
            // .SetBasePath(AppContext.BaseDirectory)
            .SetBasePath(projectRoot)
            .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
            .Build();

        using var aoi = new ImageProcessorContext(
            camId:  config["AoiSettings:CameraId"],
            enginePath: config["AoiSettings:EnginePath"]
        );

        var detections = aoi.Inspect(resize: true);

        Console.WriteLine($"Found {detections.Length} object(s):");
        foreach (var det in detections)
        {
            Console.WriteLine(
                $"  {det.ClassName,-20} " +
                $"score={det.Score:F2}  " +
                $"box=({det.X:F0},{det.Y:F0}) {det.W:F0}x{det.H:F0}");
        }

        aoi.SaveInspectedFrame("result.png");
    }

    private static string GetProjectRoot()
    {
        var currentDir = new DirectoryInfo(AppContext.BaseDirectory);

        // Walk up the directory tree looking for a solution or project file
        while (currentDir != null)
        {
            if (currentDir.GetFiles("*.sln").Length > 0 || currentDir.GetFiles("*.csproj").Length > 0)
            {
                return currentDir.FullName;
            }
            currentDir = currentDir.Parent;
        }

        // Fallback to BaseDirectory if we can't find it (e.g., in a production/published environment)
        return AppContext.BaseDirectory;
    }
}