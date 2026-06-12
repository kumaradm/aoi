#pragma once
#include <variant>
#include <string_view>
#include <iostream>

enum class CameraError {
    SUCCESS = 0,
    ERR_CANNOT_OPEN_CAMERA = 101,
    ERR_CAMERA_ALREADY_STREAMING = 102,
    ERR_CANNOT_READ_INITIAL_FRAME = 103,
    ERR_CAMERA_STREAMING_TIMEOUT = 104,
    ERR_CANNOT_GET_FRAME = 105,
    ERR_EMPTY_FRAME = 106,
    ERR_CANNOT_WRITE_FRAME = 107
};

enum class InferenceError {
    SUCCESS = 0,
    ERR_CANNOT_LOAD_MODEL_CONFIG = 201,
    ERR_ENGINE_PATH_NOT_EXISTS = 202,
    ERR_CANNOT_CREATE_INFERENCE_RUNTIME = 203,
    ERR_CANNOT_DESERIALIZED_CUDA_ENGINE = 204,
    ERR_CANNOT_CREATE_EXECUTION_CONTEXT = 205,
    ERR_CANNOT_ENQUEUE = 206,
    ERR_FRAME_EXCEEDS_MAX_BOUNDS = 207,
    ERR_INFERENCE_BREAKDOWN = 208,
    ERR_INVALID_INPUT_FRAME = 209
};

class SystemError {
public:
    // Implicit constructors allow passing CameraError or InferenceError directly
    SystemError(CameraError err) : errorValue(err) {}
    SystemError(InferenceError err) : errorValue(err) {}

    // Check if it's a specific type of error
    bool isCameraError() const { return std::holds_alternative<CameraError>(errorValue); }
    bool isInferenceError() const { return std::holds_alternative<InferenceError>(errorValue); }

    // Check if everything is OK
    bool isSuccess() const {
        if (isCameraError()) return std::get<CameraError>(errorValue) == CameraError::SUCCESS;
        return std::get<InferenceError>(errorValue) == InferenceError::SUCCESS;
    }

    // Retrieve the underlying code as an integer
    int code() const {
        return std::visit([](auto&& arg) -> int {
            return static_cast<int>(arg);
        }, errorValue);
    }

    // Human-readable categorization
    std::string_view category() const {
        if (isCameraError()) return "Camera Module";
        return "Inference Module";
    }

private:
    std::variant<CameraError, InferenceError> errorValue;
};