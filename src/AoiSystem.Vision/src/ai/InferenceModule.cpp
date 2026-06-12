#include "InferenceModule.hpp"

InferenceModule::InferenceModule(const char* enginePath) 
    : runtime(nullptr), engine(nullptr), context(nullptr), stream(nullptr),
    modelInputIdx(0), modelOutputIdx(1), modelInputC(0), modelInputH(0), modelInputW(0),
    maxDetection(1000), d_frame(nullptr), d_count(nullptr), d_detections(nullptr) {
    for (int i = 0; i < 2; i++) modelBuffers[i] = nullptr;
    maxFrameBytes = 3 * 8192 * 8192;
    
    cudaStreamCreate(&stream);
    cudaMalloc(&d_frame, maxFrameBytes * sizeof(unsigned char));
    cudaMalloc(&d_count, sizeof(int));
    cudaMalloc(&d_detections, maxDetection * sizeof(Detection));

    namespace fs = std::filesystem;
    if (!fs::exists(enginePath)) {
        this->~InferenceModule();
        throw std::runtime_error("Engine path does not exist");
    }
    
    auto configResult = ModelConfig::Load(enginePath);
    if (!configResult) {
        this->~InferenceModule();
        throw std::runtime_error("Failed to load model config");
    }
    config = std::move(*configResult);

    size_t fileSize = fs::file_size(enginePath);
    std::ifstream file(enginePath, std::ios::binary);
    std::vector<char> modelData(fileSize);
    file.read(modelData.data(), fileSize);

    runtime.reset(nvinfer1::createInferRuntime(logger));
    if (!runtime) {
        this->~InferenceModule();
        throw std::runtime_error("Failed to create inference runtime");
    }

    engine.reset(runtime->deserializeCudaEngine(modelData.data(), fileSize));
    if (!engine) {
        this->~InferenceModule();
        throw std::runtime_error("Failed to deserialize CUDA engine");
    }

    context.reset(engine->createExecutionContext());
    if (!context) {
        this->~InferenceModule();
        throw std::runtime_error("Failed to create execution context");
    }

    for (int i = 0; i < engine->getNbIOTensors(); ++i) {
        auto name = engine->getIOTensorName(i);
        if (engine->getTensorIOMode(name) == nvinfer1::TensorIOMode::kINPUT) {
            modelInputIdx = i;
            auto dims = engine->getTensorShape(name);
            modelInputC = dims.d[1];
            modelInputH = dims.d[2];
            modelInputW = dims.d[3];
            cudaMalloc(&modelBuffers[modelInputIdx], modelInputC * modelInputH * modelInputW * sizeof(float));
        } else {
            modelOutputIdx = i;
            auto dims = engine->getTensorShape(name);
            size_t numElements = 1;
            for (int j = 0; j < dims.nbDims; ++j) numElements *= dims.d[j];
            cudaMalloc(&modelBuffers[modelOutputIdx], numElements * sizeof(float));
        }
    }
}

InferenceModule::~InferenceModule() {
    if (stream) cudaStreamDestroy(stream);
    
    if (modelBuffers[modelInputIdx])  cudaFree(modelBuffers[modelInputIdx]);
    if (modelBuffers[modelOutputIdx]) cudaFree(modelBuffers[modelOutputIdx]);

    if (d_frame) cudaFree(d_frame);
    if (d_count) cudaFree(d_count);
    if (d_detections) cudaFree(d_detections);
}

std::expected<std::vector<Detection>, InferenceError> InferenceModule::RunInference(int srcW, int srcH, int xOffset, int yOffset) {
    const char* inputName = engine->getIOTensorName(modelInputIdx);
    const char* outputName = engine->getIOTensorName(modelOutputIdx);

    context->setTensorAddress(inputName, modelBuffers[modelInputIdx]);
    context->setTensorAddress(outputName, modelBuffers[modelOutputIdx]);
    
    cudaMemsetAsync(d_count, 0, sizeof(int), stream);

    if (!context->enqueueV3(stream)) return std::unexpected(InferenceError::ERR_CANNOT_ENQUEUE);
    
    nvinfer1::Dims outDims = engine->getTensorShape(outputName);
    int nChannels = outDims.d[1]; 
    int nAnchors = outDims.d[2];
    int nClasses = nChannels - 4;
    
    Postprocess((float*)modelBuffers[modelOutputIdx], d_detections, d_count, config.confidenceThreshold,
        nAnchors, nClasses, xOffset, yOffset, srcW, srcH, modelInputW, modelInputH, maxDetection, stream);
    
    size_t h_count = 0;
    cudaMemcpyAsync(&h_count, d_count, sizeof(int), cudaMemcpyDeviceToHost, stream);
    cudaStreamSynchronize(stream);

    std::vector<Detection> detections;
    if (h_count > 0) {
        int copyCount = std::min(h_count, maxDetection);
        size_t currentSize = detections.size();
        detections.resize(currentSize + copyCount);
        cudaMemcpyAsync(detections.data() + currentSize, d_detections, copyCount * sizeof(Detection), cudaMemcpyDeviceToHost, stream);
        cudaStreamSynchronize(stream);   
    }
    return std::move(detections);
}

std::expected<std::vector<Detection>, InferenceError> InferenceModule::DetectObjects(cv::Mat& frame) {   
    if (frame.empty()) return std::unexpected(InferenceError::ERR_INVALID_INPUT_FRAME);
    
    int srcW = frame.cols;
    int srcH = frame.rows;    
    
    const size_t frameBytes = frame.step[0] * srcH;
    if (frameBytes > maxFrameBytes) return std::unexpected(InferenceError::ERR_FRAME_EXCEEDS_MAX_BOUNDS);
    
    if (frame.isContinuous()) {
        cudaMemcpyAsync(d_frame, frame.data, frameBytes, cudaMemcpyHostToDevice, stream);
    } else {
        cudaMemcpy2DAsync(d_frame, srcW * 3, frame.data, frame.step[0], srcW * 3, srcH, cudaMemcpyHostToDevice, stream);
    }
    
    Preprocess(d_frame, (float*)modelBuffers[modelInputIdx], srcW, srcH, modelInputW, modelInputH, 0, 0, srcW, stream);
    auto inferenceResult = RunInference(srcW, srcH, 0, 0);
    if (!inferenceResult.has_value()) return std::unexpected(inferenceResult.error());
    
    std::vector<Detection> detections = std::move(inferenceResult.value());
    detections.erase(
        std::remove_if(detections.begin(), detections.end(),
            [](const Detection& d) { return d.score < 0.01f; }),
        detections.end());

    detections = InferenceModule::NMS(detections, config.iouThreshold, config.topK);
    
    return std::move(detections);
}

float InferenceModule::IoU(const Detection& a, const Detection& b) {
    float interX1 = std::max(a.x, b.x);
    float interY1 = std::max(a.y, b.y);
    float interX2 = std::min(a.x + a.w, b.x + b.w);
    float interY2 = std::min(a.y + a.h, b.y + b.h);

    float interW  = interX2 - interX1;
    float interH  = interY2 - interY1;
    if (interW <= 0 || interH <= 0) return 0.f;

    float interArea = interW * interH;
    float unionArea = a.w * a.h + b.w * b.h - interArea;
    return interArea / unionArea;
}

std::vector<Detection> InferenceModule::NMS(std::vector<Detection>& dets, float iouThreshold, int topK) {
    if (dets.empty()) return {};

    std::sort(dets.begin(), dets.end(),
        [](const Detection& a, const Detection& b) {
            return a.score > b.score;
        });

    std::vector<bool> suppressed(dets.size(), false);
    std::vector<Detection> result;
    result.reserve(std::min((int)dets.size(), topK));

    for (size_t i = 0; i < dets.size(); i++) {
        if (suppressed[i]) continue;

        result.push_back(dets[i]);
        if ((int)result.size() >= topK) break;

        for (size_t j = i + 1; j < dets.size(); j++) {
            if (suppressed[j]) continue;

            if (std::roundf(dets[i].classId) != std::roundf(dets[j].classId)) continue;

            if (IoU(dets[i], dets[j]) > iouThreshold) suppressed[j] = true;
        }
    }

    return result;
}