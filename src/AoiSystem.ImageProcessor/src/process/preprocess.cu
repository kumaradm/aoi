#include <cuda_runtime.h>
#include <device_launch_parameters.h>

/**
 * Full CUDA Kernel for RGB Tiled Preprocessing
 * - src: Pointer to RGB interleaved buffer (3 bytes per pixel)
 * - dest: Pointer to float buffer (Planar CHW format)
 * - sw, sh: The size of the TILE to crop (e.g., 640x512)
 * - dw, dh: The size of the AI INPUT (e.g., 640x640)
 * - xOffset: Starting X position of the tile in the big frame
 * - fullWidth: Total width of the raw camera frame (e.g., 4096)
 */
__global__ void PreProcessKernelRGB(unsigned char* src, float* dest, 
                                   int sw, int sh, 
                                   int dw, int dh,
                                   int xOffset, int fullWidth) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < dw && y < dh) {
        // 1. Map destination (AI input size) back to source tile size
        float scaleX = (float)sw / dw;
        float scaleY = (float)sh / dh;

        int iSrcX = (int)(x * scaleX);
        int iSrcY = (int)(y * scaleY);

        // 2. Bound checking
        if (iSrcX >= sw) iSrcX = sw - 1;
        if (iSrcY >= sh) iSrcY = sh - 1;

        // 3. RGB Memory Address Calculation
        // Since input is RGB, each pixel takes 3 bytes.
        // We calculate the pixel start index, then multiply by 3.
        int srcIdx = ((iSrcY * fullWidth) + (iSrcX + xOffset)) * 3;
        
        // 4. Destination Calculation (Planar CHW)
        int destIdx = y * dw + x;
        int planeSize = dw * dh;

        // 5. Read, Normalize, and Store in Planar Format
        // We grab R, G, and B from the interleaved source and 
        // place them into separate 'planes' in the destination.
        dest[destIdx] = (float)src[srcIdx] / 255.0f;                 // R Plane
        dest[destIdx + planeSize] = (float)src[srcIdx + 1] / 255.0f;     // G Plane
        dest[destIdx + 2 * planeSize] = (float)src[srcIdx + 2] / 255.0f; // B Plane
    }
}

extern "C" {
    void PreProcessForAI(unsigned char* d_src, float* d_dest, 
                         int sw, int sh, int dw, int dh, 
                         int xOffset, int fullWidth) {
        
        // Use 16x16 threads per block (256 threads total per block)
        dim3 block(16, 16);
        
        // Calculate grid size to cover the destination image
        dim3 grid((dw + block.x - 1) / block.x, (dh + block.y - 1) / block.y);

        // Launch the RGB specific kernel
        PreProcessKernelRGB<<<grid, block>>>(d_src, d_dest, sw, sh, dw, dh, xOffset, fullWidth);

        // Ensure the GPU finished before the CPU continues
        cudaError_t err = cudaDeviceSynchronize();
        if (err != cudaSuccess) {
            // In production, you'd want to handle this error
        }
    }
}