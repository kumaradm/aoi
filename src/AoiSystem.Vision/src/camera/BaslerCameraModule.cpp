#include "BaslerCameraModule.hpp"
#include <iostream>

using namespace Pylon;
using namespace GenApi;

BaslerCameraModule::BaslerCameraModule(const std::string& pcbImagePath) 
    : inputFilename(pcbImagePath) {
    PylonInitialize();
}

BaslerCameraModule::~BaslerCameraModule() {
    if (camera.IsOpen()) camera.Close();
    // PylonTerminate();
}

bool BaslerCameraModule::Open() {
    try {
        camera.Attach(CTlFactory::GetInstance().CreateFirstDevice());
        camera.Open();
        std::cout << "BaslerCameraModule: Connected to " << camera.GetDeviceInfo().GetModelName() << std::endl;
        return true;
    } catch (const GenericException &e) {
        std::cerr << "BaslerCameraModule Error: " << e.GetDescription() << std::endl;
        return false;
    }
}

bool BaslerCameraModule::ConfigureEmulator() {
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

Pylon::CGrabResultPtr BaslerCameraModule::GrabFrame() {
    CGrabResultPtr ptrGrabResult;
    camera.GrabOne(5000, ptrGrabResult);
    return ptrGrabResult;
}

void BaslerCameraModule::SaveFrame(Pylon::CGrabResultPtr grabResult, const std::string& filename) {
    if (grabResult->GrabSucceeded()) {
        CPylonImage pylonImage;
        pylonImage.AttachGrabResultBuffer(grabResult);
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Png, filename.c_str(), pylonImage);
    }
}