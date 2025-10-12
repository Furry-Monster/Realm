#pragma once

#include "resource/datatype/model/model.h"
#include "resource/importer/model_importer.h"
#include "utils.h"
#include <future>
#include <memory>
#include <string>
#include <unordered_map>

namespace RealmEngine
{
    class AssetManager
    {
    public:
        AssetManager()           = default;
        ~AssetManager() noexcept = default;

        AssetManager(const AssetManager& that)            = delete;
        AssetManager(AssetManager&& that)                 = delete;
        AssetManager& operator=(const AssetManager& that) = delete;
        AssetManager& operator=(AssetManager&& that)      = delete;

        void initialize();
        void disposal();

        Model*              loadModel(const std::string& path, const ModelImporter::LoadOptions& options = {});
        std::future<Model*> loadModelAsync(const std::string& path);

        Model* getModel(const std::string& path);
        bool   isModelLoaded(const std::string& path) const;

        void unloadModel(const std::string& path);
        void unloadAllModels();

    private:
        ModelImporter m_model_importer;

        std::unordered_map<std::string, std::unique_ptr<Model>> m_models;
    };
} // namespace RealmEngine