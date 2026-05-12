cmake_minimum_required(VERSION 3.18)
project(BaslerAoiBridge LANGUAGES CXX CUDA)

# --- 1. COMPILER & CUDA SETTINGS ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# AUTO-DETECT ARCHITECTURE: 
# Sets sm_75 for your GTX 1650 Ti (WSL2) or sm_87 for Jetson Orin.
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES "75;87") 
endif()

# Output the .so to the libs folder for C# to find
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)

# --- 2. FIND PYLON (Basler Camera) ---
find_path(PYLON_INCLUDE_DIR NAMES pylon/PylonIncludes.h PATHS /opt/pylon/include)
find_library(PYLON_BASE_LIB NAMES pylonbase PATHS /opt/pylon/lib /opt/pylon/lib64)
find_library(PYLON_UTILITY_LIB NAMES pylonutility PATHS /opt/pylon/lib /opt/pylon/lib64)

# --- 3. FIND CUDA ---
find_package(CUDAToolkit REQUIRED)

# --- 4. FIND TENSORRT (The AI Brain) ---
# We look in x86_64 paths (WSL2) and aarch64 paths (Jetson)
find_path(TRT_INCLUDE_DIR NAMES NvInfer.h 
    HINTS /usr/include/x86_64-linux-gnu /usr/include/aarch64-linux-gnu /usr/local/cuda/include)

find_library(TRT_LIB NAMES nvinfer 
    HINTS /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)

# Verify TensorRT was found (Fail early if missing)
if(NOT TRT_LIB)
    message(FATAL_ERROR "TensorRT library not found. Please check your .deb installation.")
endif()

# --- 5. SOURCE FILES ---
# Collects your modules: Camera, AI, and CUDA Preprocessing
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*.cu"
)

# --- 6. CREATE THE SHARED LIBRARY ---
add_library(image_processor SHARED ${SOURCES})

# --- 7. INCLUDE DIRECTORIES ---
target_include_directories(image_processor PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${PYLON_INCLUDE_DIR}
    ${TRT_INCLUDE_DIR}
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

# --- 8. LINK LIBRARIES ---
target_link_libraries(image_processor PRIVATE 
    ${PYLON_BASE_LIB} 
    ${PYLON_UTILITY_LIB}
    ${TRT_LIB}
    CUDA::cudart
)

# --- 9. STATUS MESSAGE ---
message(STATUS "Build Configuration:")
message(STATUS "  > CUDA Arch: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "  > TRT Lib:   ${TRT_LIB}")
message(STATUS "  > Pylon:     ${PYLON_BASE_LIB}")