#include "CameraModule.hpp"
#include <iostream>

using namespace Pylon;
using namespace GenApi;

CameraModule::CameraModule(const std::string& pcbImagePath) 
    : inputFilename(pcbImagePath) {
    PylonInitialize();
}

CameraModule::~CameraModule() {
    if (camera.IsOpen()) camera.Close();
    // PylonTerminate();
}

bool CameraModule::Open() {
    try {
        camera.Attach(CTlFactory::GetInstance().CreateFirstDevice());
        camera.Open();
        std::cout << "CameraModule: Connected to " << camera.GetDeviceInfo().GetModelName() << std::endl;
        return true;
    } catch (const GenericException &e) {
        std::cerr << "CameraModule Error: " << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraModule::ConfigureEmulator() {
    try {
        INodeMap& nodemap = camera.GetNodeMap();

        CIntegerPtr(nodemap.GetNode("Width"))->SetValue(4096);
        CIntegerPtr(nodemap.GetNode("Height"))->SetValue(4096);

        CEnumerationPtr triggerMode(nodemap.GetNode("TriggerMode"));
        if (triggerMode.IsValid()) triggerMode->FromString("Off");

        CEnumerationPtr testImageSelector(nodemap.GetNode("TestImageSelector"));
        if (testImageSelector.IsValid()) testImageSelector->FromString("Off");

        CEnumerationPtr pixelFormat(nodemap.GetNode("PixelFormat"));
        if (pixelFormat.IsValid()) pixelFormat->FromString("RGB8Packed");

        CEnumerationPtr fileMode(nodemap.GetNode("ImageFileMode"));
        if (fileMode.IsValid()) {
            fileMode->FromString("On");
            CStringPtr(nodemap.GetNode("ImageFilename"))->SetValue(inputFilename.c_str());
        }
        return true;
    } catch (const GenericException &e) {
        return false;
    }
}

Pylon::CGrabResultPtr CameraModule::GrabFrame() {
    CGrabResultPtr ptrGrabResult;
    camera.GrabOne(5000, ptrGrabResult);
    return ptrGrabResult;
}

void CameraModule::SaveFrame(Pylon::CGrabResultPtr grabResult, const std::string& filename) {
    if (grabResult->GrabSucceeded()) {
        CPylonImage pylonImage;
        pylonImage.AttachGrabResultBuffer(grabResult);
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Png, filename.c_str(), pylonImage);
    }
}