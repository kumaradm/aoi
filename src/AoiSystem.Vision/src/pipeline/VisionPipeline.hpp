#pragma once
#include <opencv2/opencv.hpp>
#include <expected>
#include <atomic>
#include <thread>
#include <memory>
#include "CameraModule.hpp"
#include "InferenceModule.hpp"
#include "TSQueue.hpp"
#include "Snowflake.hpp"

struct VisionMetadata {
    cv::Mat frame;
    std::vector<Detection> detections;
    int64_t frameId;
    int64_t timestampMs;
    int64_t processingTimeMs;
};

typedef void (*ResultCallback)(VisionMetadata* data);

class VisionPipeline {
private:
    std::unique_ptr<CameraModule> cam;
    std::unique_ptr<InferenceModule> ai;

    TSQueue<VisionMetadata> processQueue;

    std::atomic<bool> isProcessing{false};
    std::thread processThread;

    Snowflake idGenerator;

    ResultCallback resultCallback;

    void ProcessWorker();
    
public:
    VisionPipeline(const char* cameraId, const char* enginePath);
    ~VisionPipeline();

    SystemError Start();
    SystemError Stop();
    SystemError Capture();

    void RegisterCallback(ResultCallback callback) {
        resultCallback = callback;
    };
};