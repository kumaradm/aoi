cmake_minimum_required(VERSION 3.18)
project(BaslerAoiBridge LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CUDA_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES "75;87") 
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)

find_package(OpenCV REQUIRED)
find_package(CUDAToolkit REQUIRED)

find_path(TRT_INCLUDE_DIR NAMES NvInfer.h 
HINTS /usr/include/x86_64-linux-gnu /usr/include/aarch64-linux-gnu /usr/local/cuda/include)

find_library(TRT_LIB NAMES nvinfer 
HINTS /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)

if(NOT TRT_LIB)
    message(FATAL_ERROR "TensorRT library not found. Please check your .deb installation.")
endif()

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/ImageProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/camera/CameraModule.cpp
    ${CMAKE_SOURCE_DIR}/src/camera/CameraModule.hpp
    ${CMAKE_SOURCE_DIR}/src/ai/InferenceModule.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/InferenceModule.hpp
)

set(CUDA_SOURCES
    ${CMAKE_SOURCE_DIR}/src/process/preprocess.cu
    ${CMAKE_SOURCE_DIR}/src/process/postprocess.cu
)

add_library(image_processor SHARED ${SOURCES} ${CUDA_SOURCES})

target_include_directories(image_processor PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/camera
    ${CMAKE_SOURCE_DIR}/src/process
    ${CMAKE_SOURCE_DIR}/src/ai
    ${CMAKE_SOURCE_DIR}/include
    ${PYLON_INCLUDE_DIR}
    ${TRT_INCLUDE_DIR}
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(image_processor PRIVATE 
    ${TRT_LIB}
    ${GENAPI_LIB}
    CUDA::cudart
    ${OpenCV_LIBS}
)

set_target_properties(image_processor PROPERTIES
    OUTPUT_NAME "image_processor"
    PREFIX "lib"
)

message(STATUS "Build Configuration:")
message(STATUS "  > CUDA Arch: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "  > TRT Lib:   ${TRT_LIB}")