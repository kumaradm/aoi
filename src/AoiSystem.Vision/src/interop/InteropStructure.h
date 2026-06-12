#pragma once
#include <cstdint>

// Flat, memory-aligned representation of your internal Detection struct
struct InteropDetection {
    int32_t classId;
    float score;
    float x;
    float y;
    float width;
    float height;
};

// Flat representation of VisionMetadata that safely crosses the language border
struct InteropVisionResult {
    int64_t frameId;
    int64_t timestampMs;
    int64_t processingTimeMs;

    // Raw image pointer data (shares memory with cv::Mat)
    uint8_t* pImageData;
    int32_t imageWidth;
    int32_t imageHeight;
    int32_t imageStride; // step[0] in OpenCV, critical for memory alignment

    // Pointer to the array of detections
    InteropDetection* pDetections;
    int32_t detectionCount;
};

// Define the Interop Callback signature
typedef void (*InteropResultCallback)(InteropVisionResult result);