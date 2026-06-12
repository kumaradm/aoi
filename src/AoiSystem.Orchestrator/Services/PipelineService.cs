// using Microsoft.Extensions.Logging;

// namespace AoiSystem.Orchestrator.Services;

// public sealed class PipelineService : IPipelineService
// {
//     private readonly IMcuManagerService _mcuManager;
//     private readonly IImageProcessorInteropService _imageProcessor;
//     private readonly ILogger<PipelineService> _logger;

//     public AoiPipelineService(
//         IMcuManagerService mcuManager,
//         IImageProcessorInteropService imageProcessor,
//         AoiConfig config,
//         ILogger<AoiPipelineService> logger
//     )
//     {
//         _mcuController = mcuController;
//         _imageProcessor = imageProcessor;
//         _logger = logger;
//     }

//     public void Init()
//     {

//         var ok = _mcuManager.Init(_config.McuPortName, _config.McuBaudRate);
//         if (!ok)
//         {
//             _logger.LogError("Failed to initialize motion controller.");
//             throw new InvalidOperationException("Failed to initialize motion controller.");
//         }
//         ok = _imageProcessor.Init(_config.ImageProcessorEndpoint);
//         if (!ok)
//         {
//             _logger.LogError("Failed to initialize image processor.");
//             throw new InvalidOperationException("Failed to initialize image processor.");
//         }
//         _logger.LogInformation("AOI pipeline service initialized.");
//     }

//     public async Task InpectAsync(CancellationToken cts)
//     {
//         _logger.LogInformation("Starting board scan.");
//         await _imageProcessor.

//     }
// }