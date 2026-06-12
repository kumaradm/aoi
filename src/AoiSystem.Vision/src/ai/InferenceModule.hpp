#pragma once
#include <NvInfer.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <string>
#include <memory>
#include <expected>
#include <opencv2/opencv.hpp>
#include "ModelConfig.hpp"
#include "SystemErrorCodes.hpp"

extern "C" void Preprocess(unsigned char* src, float* dst,
    int srcW, int srcH,
    int dstW, int dstH,
    int xOffset, int yOffset,
    int frameWidth,
    cudaStream_t stream);

extern "C" void Postprocess(float* input, struct Detection* output, int* count,
    float threshold, int numAnchors, int numClasses,
    int xOffset, int yOffset,
    int srcW, int srcH,
    int modelW, int modelH,
    int maxDetection, cudaStream_t stream);
  
struct Detection {
    float x, y, w, h;
    float score;
    float classId;
};

struct TRTDeleter {
    template <typename T>
    void operator()(T* obj) const {
        if (obj) delete obj;
    }
};

struct CudaDeleter {
    void operator()(void* ptr) const {
        if (ptr) {
            cudaFree(ptr);
        }
    }
};

class TRTLogger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) std::cout << "[TensorRT] " << msg << std::endl;
    }
};

class InferenceModule {
private:
    std::unique_ptr<nvinfer1::IRuntime, TRTDeleter> runtime;
    std::unique_ptr<nvinfer1::ICudaEngine, TRTDeleter> engine;
    std::unique_ptr<nvinfer1::IExecutionContext, TRTDeleter> context;
    
    TRTLogger logger;
    cudaStream_t stream;

    Detection* d_detections;
    int* d_count;
    unsigned char* d_frame;
    
    void* modelBuffers[2];
    
    size_t maxFrameBytes;
    size_t maxDetection;
    
    int modelInputIdx;
    int modelOutputIdx;
    int modelInputW;
    int modelInputH;
    int modelInputC;
    int tileSrcW, tileSrcH;
    
    std::expected<std::vector<Detection>, InferenceError> RunInference(int srcW, int srcH, int xOffset, int yOffset); 
    float IoU(const Detection& a, const Detection& b);
    std::vector<Detection> NMS(std::vector<Detection>& dets, float iouThreshold = 0.45f, int topK = 50);
    
public:
    ModelConfig config;
    InferenceModule(const char* enginePath);
    ~InferenceModule();

    std::expected<std::vector<Detection>, InferenceError> DetectObjects(cv::Mat& frame);
};