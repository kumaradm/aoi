#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <functional>
#include <expected>
#include <string>
#include <iostream>
#include "SystemErrorCodes.hpp"

class CameraModule {
private:
    cv::VideoCapture cap;

    cv::Mat buffers[2];
    int writeIdx = 0;
    int readIdx = 1;
    std::mutex swapMutex;

    std::condition_variable frameReady;
    std::atomic<bool> isStreaming{false};
    std::atomic<bool> firstFrameReceived{false};
    std::thread captureThread;

    void StreamWorker();

public:
    CameraModule(const char* camId);
    ~CameraModule();

    CameraError StartStreaming();
    CameraError StopStreaming();
    std::expected<cv::Mat, CameraError> GetFrame();
    CameraError SaveFrame(const cv::Mat frame, const char* path);
};

#endif