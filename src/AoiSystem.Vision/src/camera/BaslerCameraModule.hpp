#pragma once
#include <pylon/PylonIncludes.h>
#include <string>

class BaslerCameraModule {
private:
    Pylon::CInstantCamera camera;
    std::string inputFilename;

public:
    BaslerCameraModule(const std::string& pcbImagePath);
    ~BaslerCameraModule();

    bool Open();
    bool ConfigureEmulator();
    Pylon::CGrabResultPtr GrabFrame();
    
    // For Manual Conveyor Simulation
    void SetOffsetY(int y);
    void SaveFrame(Pylon::CGrabResultPtr grabResult, const std::string& filename);
};