# AI-Powered Automated Optical Inspection (AOI) System

A high-speed AOI system designed for PCB defect detection. The system uses line scan camera to capture high-quality image

## Project Overview

Slide 1: Project Overview & The Challenge
Title: AI-Powered Automated Optical Inspection (AOI) SystemThe Problem: Manual PCB inspection is slow and prone to human error. High-speed assembly lines require sub-second defect detection.The Goal: Build a system that synchronizes hardware triggers (MCU) with high-speed imaging (Line Scan) and ML inference.Tech Stack: Python/C++, TensorFlow/PyTorch, Serial (UART), OpenCV.

Slide 2: System Architecture (The "Hardware/Physical" Map)
Visual: A block diagram showing the physical flow.Input: Photoelectric Sensor → MCU.Process: Industrial PC (running your software).Output: Line Scan Camera (Input) and Pneumatic Reject Arm (Output).Key Point: Explain the Trigger Latency. "The system must respond to the MCU signal within $X$ milliseconds to ensure the camera captures the PCB at the correct position."

Slide 3: Software Architecture (The "Logic" Map)
Visual: Use the Three-Component Diagram we discussed (MCU Manager, Orchestrator, Image Processor).Architecture Style: Event-Driven & Decoupled.The "Why": "I chose a decoupled, event-driven architecture so that the Image Processing (heavy compute) doesn't block the MCU Listener (time-critical). This allows the system to remain responsive even during intensive ML inference."

Slide 4: The Process Flow (The Sequence Diagram)Visual: A Sequence Diagram (Swimlanes).Lane 1 (MCU): Emits Trigger_Signal.Lane 2 (Orchestrator): Receives signal $\rightarrow$ Commands Camera $\rightarrow$ Receives Image.Lane 3 (ML Processor): Receives Image_Buffer $\rightarrow$ Returns Defect_Score.Highlight: Point out the Asynchronous handoff. "The Orchestrator manages the state, ensuring that Image A is linked to PCB Serial Number A."

Slide 5: Machine Learning & Image ProcessingVisual: A "Before and After" image. Raw line-scan image vs. Inference Overlay (bounding boxes around defects).Details: Mention your model (e.g., "YOLOv8 for real-time detection") and how you handled the high-aspect-ratio images typical of line-scan cameras.

Slide 6: Engineering Trade-offs & ChallengesThe "Real Talk": This is where you gain 100% credibility.Example: "The biggest challenge was Race Conditions. If the belt moves too fast, the next trigger arrives before the first image is saved. I solved this by implementing a Producer-Consumer Queue to buffer incoming signals."Slide 7: Results & ImpactMetrics:Accuracy: 98% detection rate.Throughput: Processed 10 PCBs per minute.Reliability: System handles disconnects/reconnects of the MCU gracefully.

Directory Structure:

AoiSystem/
├── .devcontainer/
│   ├── devcontainer.json
│   └── Dockerfile
│
├── src/
│   ├── AoiSystem.Orchestrator/
│   │   ├── Program.cs
│   │   ├── Services/
│   │   │   ├── IPipelineService.cs
│   │   │   ├── PipelineService.cs
│   │   │   ├── IJobSchedulerService.cs
│   │   │   └── JobSchedulerService.cs
│   │   ├── Interop/
│   │   │   ├── IImageProcessorInterop.cs
│   │   │   └── ImageProcessorInterop.cs
│   │   ├── Workers/
│   │   │   └── InspectionWorker.cs
│   │   └── AoiSystem.Orchestrator.csproj
│   │
│   ├── AoiSystem.Shared/
│   │   ├── Models/
│   │   │   ├── InspectionResult.cs
│   │   │   ├── DefectRecord.cs
│   │   │   └── Native/
│   │   │       └── InspectionResultNative.cs
│   │   └── AoiSystem.Shared.csproj
│   │
│   ├── AoiSystem.ImageProcessor/
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── FrameGrabber.h
│   │   │   ├── Preprocessor.h
│   │   │   ├── DefectDetector.h
│   │   │   └── Classifier.h
│   │   └── src/
│   │       ├── FrameGrabber.cpp
│   │       ├── Preprocessor.cpp
│   │       ├── DefectDetector.cpp
│   │       ├── Classifier.cpp
│   │       └── aoi_api.cpp          # ← exported C API (replaces main.cpp)
│   │
│   └── AoiSystem.MLTraining/
│       ├── train.py
│       ├── export_onnx.py
│       ├── requirements.txt
│       ├── dataset/
│       └── models/
│
├── tests/
│   ├── AoiSystem.Orchestrator.Tests/
│   └── AoiSystem.ImageProcessor.Tests/
│
├── docs/
│   └── architecture.md
│
├── AoiSystem.sln
└── README.md