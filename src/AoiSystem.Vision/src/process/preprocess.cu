#include <cuda_runtime.h>
#include <device_launch_parameters.h>

__global__ void PreprocessKernelBilinear(unsigned char* src, float* dst, int tileSrcW, int tileSrcH, int dstW, int dstH, int xOffset, int yOffset, int srcW) {
    
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= dstW || y >= dstH) return;

    float scale = min((float)dstW / tileSrcW, (float)dstH / tileSrcH);
    int scaledW = (int)(tileSrcW * scale);
    int scaledH = (int)(tileSrcH * scale);
    int padX = (dstW - scaledW) / 2;
    int padY = (dstH - scaledH) / 2;

    int dstIdx = y * dstW + x;
    int planeSize = dstW * dstH;

    // Letterbox padding
    if (x < padX || x >= padX + scaledW || y < padY || y >= padY + scaledH) {
        float gray = 114.0f / 255.0f;
        dst[dstIdx] = gray;
        dst[dstIdx + planeSize] = gray;
        dst[dstIdx + 2 * planeSize] = gray;
        return;
    }

    // Map destination pixel back to floating point source coordinates
    float fSrcX = (float)(x - padX) / scale + xOffset;
    float fSrcY = (float)(y - padY) / scale + yOffset;

    // Four neighbor coordinates
    int x1 = (int)floorf(fSrcX);
    int y1 = (int)floorf(fSrcY);
    int x2 = x1 + 1;
    int y2 = y1 + 1;

    // Interpolation weights
    float u = fSrcX - (float)x1;
    float v = fSrcY - (float)y1;

    // Clamp all four neighbors to frame boundaries
    x1 = max(0, min(x1, srcW - 1));
    x2 = max(0, min(x2, srcW - 1));
    y1 = max(0, min(y1, tileSrcH + yOffset - 1));
    y2 = max(0, min(y2, tileSrcH + yOffset - 1));

    // Read all four neighbors using srcW as row stride
    // OpenCV HWC BGR → swap R and B on read for RGB planar output
    int idx11 = (y1 * srcW + x1) * 3;  // top-left
    int idx21 = (y1 * srcW + x2) * 3;  // top-right
    int idx12 = (y2 * srcW + x1) * 3;  // bottom-left
    int idx22 = (y2 * srcW + x2) * 3;  // bottom-right

    // Bilinear interpolation per channel, write CHW RGB planar
    for (int i = 0; i < 3; i++) {
        // Read channel: OpenCV is BGR so idx+0=B, idx+1=G, idx+2=R
        // Remap: i=0→R(idx+2), i=1→G(idx+1), i=2→B(idx+0)
        int srcCh = 2 - i;

        float p11 = src[idx11 + srcCh];
        float p21 = src[idx21 + srcCh];
        float p12 = src[idx12 + srcCh];
        float p22 = src[idx22 + srcCh];

        float val = (1.0f - u) * (1.0f - v) * p11 + u * (1.0f - v) * p21 +
                    (1.0f - u) * v * p12 + u * v * p22;

        dst[dstIdx + i * planeSize] = val / 255.0f;
    }
}

extern "C" {
    void Preprocess(unsigned char* src, float* dst, int tileSrcW, int tileSrcH, int dstW, int dstH, int xOffset, int yOffset, int srcW, cudaStream_t stream) {
        dim3 block(16, 16);
        dim3 grid((dstW + block.x - 1) / block.x, (dstH + block.y - 1) / block.y);

        PreprocessKernelBilinear<<<grid, block, 0, stream>>>(src, dst, tileSrcW, tileSrcH, dstW, dstH, xOffset, yOffset, srcW);
    }
}