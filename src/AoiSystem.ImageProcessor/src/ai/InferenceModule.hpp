#pragma once
#include <NvInfer.h>
#include <vector>
#include <iostream>

// Standard TensorRT Logger
class TRTLogger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) std::cout << "[TensorRT] " << msg << std::endl;
    }
};

class InferenceModule {
private:
    nvinfer1::IRuntime* runtime;
    nvinfer1::ICudaEngine* engine;
    nvinfer1::IExecutionContext* context;
    cudaStream_t stream;
    TRTLogger logger;

    // Buffer pointers for GPU memory
    void* buffers[2]; 
    int inputIndex, outputIndex;

public:
    InferenceModule();
    ~InferenceModule();

    bool LoadEngine(const char* enginePath);
    // Takes a pointer to GPU memory (already preprocessed) and returns class ID
    int RunInference(float* d_inputBuffer); 
};