#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <expected>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ModelConfig {
    std::string name;
    float confidenceThreshold;
    float nmsThreshold;
    float iouThreshold;
    int topK;
    std::vector<std::string> classNames;

    const std::string& GetClassName(int classId) const {
        static const std::string unknown = "unknown";
        if (classId < 0 || classId >= (int)classNames.size())
            return unknown;
        return classNames[classId];
    }

    static std::expected<ModelConfig, std::string> Load(const std::string& enginePath) {
        namespace fs = std::filesystem;

        fs::path configPath = fs::path(enginePath).parent_path() / "model.json";
        if (!fs::exists(configPath))
            return std::unexpected("No model.json found at: " + configPath.string());

        std::ifstream file(configPath);
        if (!file.is_open())
            return std::unexpected("Cannot open config: " + configPath.string());

        json j;
        try {
            file >> j;
        } catch (const json::exception& e) {
            return std::unexpected("JSON parse error: " + std::string(e.what()));
        }

        ModelConfig cfg;
        cfg.name = j.value("name", "unknown");
        cfg.confidenceThreshold = j.value("confidence_threshold", 0.2f);
        cfg.nmsThreshold = j.value("nms_threshold", 0.5f);
        cfg.iouThreshold = j.value("iou_threshold", 0.45f);
        cfg.topK = j.value("top_k", 50);
        cfg.classNames = j.value("class_names", std::vector<std::string>{});

        if (cfg.classNames.empty())
            return std::unexpected("model.json has no class_names.");

        return cfg;
    }
};