#pragma once
#include <pylon/PylonIncludes.h>
#include <string>

class CameraModule {
private:
    Pylon::CInstantCamera camera;
    std::string inputFilename;

public:
    CameraModule(const std::string& pcbImagePath);
    ~CameraModule();

    bool Open();
    bool ConfigureEmulator();
    Pylon::CGrabResultPtr GrabFrame();
    
    // For Manual Conveyor Simulation
    void SetOffsetY(int y);
    void SaveFrame(Pylon::CGrabResultPtr grabResult, const std::string& filename);
};