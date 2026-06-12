#include "CameraModule.hpp"

CameraModule::CameraModule(const char* camId) {
    cap.open(camId);
    if (!cap.isOpened()) {
        throw std::runtime_error("Failed to open camera with ID: " + std::string(camId));
    }
}

CameraModule::~CameraModule() {
    StopStreaming();
    if (cap.isOpened()) cap.release();
}

CameraError CameraModule::StartStreaming() {
    if (!cap.isOpened()) return CameraError::ERR_CANNOT_OPEN_CAMERA;
    if (isStreaming) return CameraError::ERR_CAMERA_ALREADY_STREAMING;

    cv::Mat initialFrame;
    if (!cap.read(initialFrame) || initialFrame.empty())
        return CameraError::ERR_CANNOT_READ_INITIAL_FRAME;

    {
        std::lock_guard<std::mutex> lock(swapMutex);
        buffers[0] = initialFrame.clone();
        buffers[1] = initialFrame.clone();
        firstFrameReceived = false;
    }

    isStreaming = true;
    captureThread = std::thread(&CameraModule::StreamWorker, this);

    {
        std::unique_lock<std::mutex> lock(swapMutex);
        bool ok = frameReady.wait_for(lock, std::chrono::seconds(5),
                    [this] { return firstFrameReceived.load(); });
        if (!ok) {
            StopStreaming();
            return CameraError::ERR_CAMERA_STREAMING_TIMEOUT;
        }
    }
    return CameraError::SUCCESS;
}

void CameraModule::StreamWorker() {
    while (isStreaming) {
        if (!cap.read(buffers[writeIdx]) || buffers[writeIdx].empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(swapMutex);
            std::swap(writeIdx, readIdx);
            firstFrameReceived = true;
        }

        frameReady.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

CameraError CameraModule::StopStreaming() {
    if (!isStreaming) return CameraError::SUCCESS;
    
    isStreaming  = false;
    frameReady.notify_all();

    if (captureThread.joinable()) captureThread.join();

    std::lock_guard<std::mutex> lock(swapMutex);
    firstFrameReceived = false;
    return CameraError::SUCCESS;
}

std::expected<cv::Mat, CameraError> CameraModule::GetFrame() {
    std::lock_guard<std::mutex> lock(swapMutex);
    if (!firstFrameReceived) return std::unexpected(CameraError::ERR_CANNOT_GET_FRAME);
    
    return buffers[readIdx].clone();
}

CameraError CameraModule::SaveFrame(const cv::Mat frame, const char* path) {
    if (frame.empty()) return CameraError::ERR_EMPTY_FRAME;
    if (!cv::imwrite(path, frame)) return CameraError::ERR_CANNOT_WRITE_FRAME;
    return CameraError::SUCCESS;
}