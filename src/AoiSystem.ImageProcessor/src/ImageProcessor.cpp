#include "camera/CameraModule.hpp"
#include "ai/InferenceModule.hpp"
#include <cuda_runtime.h>
#include <algorithm>

struct AoiContext {
    CameraModule* cam;
    InferenceModule* ai;
    float* gpuInputBuffer; 
};

extern "C" void PreProcessForAI(unsigned char* d_src, float* d_dest, int sw, int sh, int dw, int dh, int xOffset, int fullWidth);

extern "C" {
    AoiContext* InitializeAOI(const char* imagePath, const char* modelPath) {
        AOIContext* ctx = new AoiContext();
        ctx->cam = new CameraModule(imagePath);
        ctx->ai = new InferenceModule();

        if (ctx->cam->Open() && ctx->cam->ConfigureEmulator() && ctx->ai->LoadEngine(modelPath)) {
            cudaMalloc(&ctx->gpuInputBuffer, 640 * 640 * 3 * sizeof(float));
            return ctx;
        }
        return nullptr;
    }

    int ProcessSlice(AoiContext* ctx) {
        auto grab = ctx->cam->GrabFrame();
        
        if (!grab->GrabSucceeded()) return -1;

        int fullWidth = 4096;
        int fullHeight = 640;
        int windowSize = 640;
        int stepSize = 480;
        
        int highestSeverityDefect = -1;

        for (int x = 0; x <= (fullWidth - windowSize); ) {
            PreProcessForAI((unsigned char*)grab->GetBuffer(), ctx->gpuInputBuffer, 
                            windowSize, fullHeight, 640, 640, x, fullWidth);
            
            int result = ctx->ai->RunInference(ctx->gpuInputBuffer);
            
            if (result > highestSeverityDefect) highestSeverityDefect = result;

            if (x == fullWidth - windowSize) break;
            x = std::min(x + stepSize, fullWidth - windowSize);
        }

        return highestSeverityDefect; 
    }

    void DeinitAOI(AoiContext* ctx) {
        if (!ctx) return;
        cudaFree(ctx->gpuInputBuffer);
        delete ctx->cam;
        delete ctx->ai;
        delete ctx;
    }
}