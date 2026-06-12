#include <cuda_runtime.h>
#include <math.h>

struct Detection {
    float x, y, w, h;
    float score;
    float classId;
};

__global__ void PostprocessKernel(float* input, Detection* output, int* count,
    float threshold, int numAnchors, int numClasses,
    int xOffset, int yOffset,
    int srcW, int srcH,
    int modelW, int modelH,
    int maxDetection) {

    int col = blockIdx.x * blockDim.x + threadIdx.x;
    if (col >= numAnchors) return;

    // Find best class
    float bestScore = -1.0f;
    int bestClass   = -1;
    for (int i = 0; i < numClasses; i++) {
        float score = input[(4 + i) * numAnchors + col];
        if (score > bestScore) { bestScore = score; bestClass = i; }
    }

    if (bestScore < threshold) return;

    int idx = atomicAdd(count, 1);
    if (idx >= maxDetection) return;

    // Raw model-space box (cx, cy, w, h) — already decoded, no sigmoid needed
    float cx = input[0 * numAnchors + col];
    float cy = input[1 * numAnchors + col];
    float bw = input[2 * numAnchors + col];
    float bh = input[3 * numAnchors + col];

    // Inverse letterbox — mirrors Preprocess exactly
    float scale = fminf((float)modelW / srcW, (float)modelH / srcH);
    float padX  = (modelW - srcW * scale) * 0.5f;
    float padY  = (modelH - srcH * scale) * 0.5f;

    // Convert cx/cy/w/h from model space → original frame space
    float x1 = (cx - bw * 0.5f - padX) / scale + xOffset;
    float y1 = (cy - bh * 0.5f - padY) / scale + yOffset;
    float x2 = (cx + bw * 0.5f - padX) / scale + xOffset;
    float y2 = (cy + bh * 0.5f - padY) / scale + yOffset;

    // Clamp to frame boundaries
    x1 = fmaxf(0.0f, fminf(x1, (float)(srcW + xOffset)));
    y1 = fmaxf(0.0f, fminf(y1, (float)(srcH + yOffset)));
    x2 = fmaxf(0.0f, fminf(x2, (float)(srcW + xOffset)));
    y2 = fmaxf(0.0f, fminf(y2, (float)(srcH + yOffset)));

    output[idx].x = x1;
    output[idx].y = y1;
    output[idx].w = x2 - x1;
    output[idx].h = y2 - y1;
    output[idx].score = bestScore;
    output[idx].classId = (float)bestClass;
}

extern "C" {
    void Postprocess(float* input, Detection* output, 
        int* count, float threshold, int numAnchors, int numClasses,
        int xOffset, int yOffset, int srcW, int srcH,    
        int modelW, int modelH,
        int maxDetection, cudaStream_t stream) {

        int blockSize = 256;
        int gridSize  = (numAnchors + blockSize - 1) / blockSize;

        PostprocessKernel<<<gridSize, blockSize, 0, stream>>>(
            input, output, count, threshold, numAnchors, numClasses,
            xOffset, yOffset,
            srcW, srcH, modelW, modelH,
            maxDetection);
    }
}