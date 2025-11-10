#include "asset_manager.h"
#include <future>
#include <memory>
#include <utility>
#include "resource/datatype/model/model.h"
#include "utils.h"

namespace RealmEngine
{
    void AssetManager::initialize() { info("Asset manager initialized."); }

    void AssetManager::disposal()
    {
        unloadAllModels();
        info("Asset manager disposed all resource.");
    }

    Model* AssetManager::loadModel(const std::string& path, const ModelImporter::LoadOptions& options)
    {
        auto it = m_models.find(path);
        if (it != m_models.end())
            return it->second.get();

        std::unique_ptr<Model> loaded = m_model_importer.loadModel(path, options);
        if (!loaded)
        {
            err("Failed to load model from :" + path);
            return nullptr;
        }

        Model* model_ptr = loaded.get();
        m_models[path]   = std::move(loaded);

        return model_ptr;
    }
    std::future<Model*> AssetManager::loadModelAsync(const std::string& /*path*/)
    {
        warn("NOT IMPLEMENTED YET");
        return std::future<Model*>();
    }

    Model* AssetManager::getModel(const std::string& path)
    {
        auto it = m_models.find(path);
        if (it != m_models.end())
            return it->second.get();

        err("Trying to get an unload model by :" + path);
        return nullptr;
    }
    bool AssetManager::isModelLoaded(const std::string& path) const { return m_models.count(path); }

    void AssetManager::unloadModel(const std::string& path)
    {
        auto it = m_models.find(path);
        if (it == m_models.end())
            return;

        it->second.reset();
        m_models.erase(it);
    }
    void AssetManager::unloadAllModels()
    {
        for (auto it = m_models.begin(); it != m_models.end();)
            it = m_models.erase(it);
    }

} // namespace RealmEngine