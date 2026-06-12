#include "VisionPipeline.hpp"

VisionPipeline::VisionPipeline(const char* cameraId, const char* enginePath) : idGenerator(1) {
    cam = std::make_unique<CameraModule>(cameraId);
    ai = std::make_unique<InferenceModule>(enginePath);
}

VisionPipeline::~VisionPipeline() {
    Stop();
}

SystemError VisionPipeline::Start() {
    SystemError err = cam->StartStreaming();

    if (!err.isSuccess()) {
        std::cerr << "[ERROR] Failed to start camera streaming: " << err.category() << " (code " << err.code() << ")\n";
        return CameraError::ERR_CAMERA_ALREADY_STREAMING;
    }

    isProcessing = true;
    processThread = std::thread(&VisionPipeline::ProcessWorker, this);
    return CameraError::SUCCESS;
}

SystemError VisionPipeline::Stop() {
    cam->StopStreaming();
    isProcessing = false;

    if (processThread.joinable()) processThread.join();
    return CameraError::SUCCESS;
}

SystemError VisionPipeline::Capture() {
    auto frameResult = cam->GetFrame();
    if (!frameResult.has_value()) {
        std::cerr << "[ERROR] Failed to capture frame\n";
        return frameResult.error();
    }
    VisionMetadata data;
    data.frame = frameResult.value();
    data.frameId = idGenerator.generate();
    data.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    processQueue.push(data);
    return CameraError::SUCCESS;
}

void VisionPipeline::ProcessWorker() {
    while (isProcessing) {
        auto frameOpt = processQueue.wait_and_pop(500);
        
        if (!frameOpt.has_value()) {
            continue;
        }
        
        uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        VisionMetadata data = std::move(frameOpt.value());

        auto detectionResult = ai->DetectObjects(data.frame);
        if (!detectionResult.has_value()) {
            std::cerr << "[ERROR] Inference failed\n";
            continue;
        }

        std::vector<Detection> detections = std::move(detectionResult.value());

        uint64_t endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

        if (resultCallback) {
            data.detections = detections;
            data.processingTimeMs = endTime - startTime;
            resultCallback(&data);
        }
    }
}