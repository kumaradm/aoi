// using Microsoft.Extensions.Logging;

// namespace AoiSystem.Orchestrator.Services;

// public sealed class PipelineService : IPipelineService
// {
//     private readonly IAoiMcuManagerService _motionController;
//     private readonly IImageProcessorInteropService _imageProcessor;
//     private readonly AoiConfig _config;
//     private readonly ILogger<AoiPipelineService> _logger;

//     public AoiPipelineService(
//         IMotionControlService motionController,
//         IImageProcessorInteropService imageProcessor,
//         AoiConfig config,
//         ILogger<AoiPipelineService> logger
//     )
//     {
//         _motionController = motionController;
//         _imageProcessor = imageProcessor;
//         _config = config;
//         _logger = logger;
//     }

//     public void Initialize()
//     {

//         var ok = _motionController.Initialize(_config.MotionControllerComPort, _config.MotionControllerBaudRate);
//         if (!ok)
//         {
//             _logger.LogError("Failed to initialize motion controller.");
//             throw new InvalidOperationException("Failed to initialize motion controller.");
//         }
//         ok = _imageProcessor.Initialize(_config.ImageProcessorEndpoint);
//         if (!ok)
//         {
//             _logger.LogError("Failed to initialize image processor.");
//             throw new InvalidOperationException("Failed to initialize image processor.");
//         }
//         _logger.LogInformation("AOI pipeline service initialized.");
//     }

//     public async Task ScanBoardAsync(CancellationToken cts, int normalizedBoardWidth, int normalizedBoardHeight)
//     {
//         _logger.LogInformation("Starting board scan.");

//     }
// }