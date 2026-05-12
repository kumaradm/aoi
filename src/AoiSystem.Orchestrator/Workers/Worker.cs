// using AoiSystem.Orchestrator.Services;
// using Microsoft.Extensions.Hosting;
// using Microsoft.Extensions.Logging;

// namespace AoiSystem.Orchestrator.Workers;

// public sealed class AoiWorker : BackgroundService
// {
//     private readonly IAoiPipelineService _pipeline;
//     private readonly ILogger<AoiWorker> _logger;
//     public AoiWorker(
//         IAoiPipelineService pipeline,
//         ILogger<AoiWorker> logger
//     )
//     {
//         _pipeline = pipeline;
//         _logger = logger;
//     }

//     protected override async Task ExecuteAsync(CancellationToken cts)
//     {
//         _logger.LogInformation("AOI worker is starting.");

//         while (!cts.IsCancellationRequested)
//         {
//             try
//             {
//                 await _pipeline.ProcessAsync(cts);
//             }
//             catch (Exception ex)
//             {
//                 _logger.LogError(ex, "An error occurred while processing the AOI pipeline.");
//             }
//         }

//         _logger.LogInformation("AOI worker is stopping.");
//     }
// }