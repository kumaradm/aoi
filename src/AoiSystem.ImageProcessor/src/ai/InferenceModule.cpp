#include "InferenceModule.hpp"
#include <fstream>
#include <cuda_runtime_api.h>

InferenceModule::InferenceModule() 
    : runtime(nullptr), engine(nullptr), context(nullptr) {}

InferenceModule::~InferenceModule() {
    // In TRT 10, use 'delete' instead of '.destroy()'
    if (context) delete context;
    if (engine) delete engine;
    if (runtime) delete runtime;
    cudaStreamDestroy(stream);
    cudaFree(buffers[1]); 
}

bool InferenceModule::LoadEngine(const char* enginePath) {
    std::ifstream file(enginePath, std::ios::binary);
    if (!file.good()) return false;

    cudaStreamCreate(&stream);

    file.seekg(0, file.end);
    size_t size = file.tellg();
    file.seekg(0, file.beg);
    std::vector<char> modelData(size);
    file.read(modelData.data(), size);

    runtime = nvinfer1::createInferRuntime(logger);
    engine = runtime->deserializeCudaEngine(modelData.data(), size);
    if (!engine) return false;
    
    context = engine->createExecutionContext();

    outputIndex = 1;

    cudaMalloc(&buffers[outputIndex], 84 * 8400 * sizeof(float)); 
    
    return true;
}

int InferenceModule::RunInference(float* d_inputBuffer) {
    if (!d_inputBuffer || !buffers[outputIndex]) return -3;

    context->setTensorAddress("images", d_inputBuffer);
    context->setTensorAddress("output0", buffers[outputIndex]);
    
    context->setInputShape("images", nvinfer1::Dims4{1, 3, 640, 640});

    if (!context->enqueueV3(stream)) {
        return -2;
    }

    cudaStreamSynchronize(stream);
    
    int num_channels = 84;
    int num_anchors = 8400;
    std::vector<float> h_output(num_channels * num_anchors);
    
    cudaMemcpy(h_output.data(), buffers[outputIndex], 
               h_output.size() * sizeof(float), cudaMemcpyDeviceToHost);

    float maxScore = 0.0f;
    int detectedClass = -1;

    for (int col = 0; col < num_anchors; col++) {
        for (int row = 4; row < num_channels; row++) {
            float score = h_output[row * num_anchors + col];
            if (score > maxScore) {
                maxScore = score;
                detectedClass = row - 4;
            }
        }
    }

    return (maxScore > 0.5f) ? detectedClass : -1;
}