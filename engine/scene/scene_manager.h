#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "scene/scene.h"

namespace RealmEngine
{
    class AssetManager;
    class RHIDevice;

    class SceneManager
    {
        using SceneTable = std::unordered_map<std::string, std::shared_ptr<Scene>>;
        using SceneChangeCallback =
            std::function<void(std::shared_ptr<Scene> old_scene, std::shared_ptr<Scene> new_scene)>;

    public:
        SceneManager()           = default;
        ~SceneManager() noexcept = default;

        SceneManager(const SceneManager&)                = delete;
        SceneManager& operator=(const SceneManager&)     = delete;
        SceneManager(SceneManager&&) noexcept            = default;
        SceneManager& operator=(SceneManager&&) noexcept = default;

        void initialize(const std::filesystem::path& asset_folder);
        void setAssetManager(AssetManager* asset_mgr) { m_asset_mgr = asset_mgr; }

        std::shared_ptr<Scene> createScene(const std::string& name);
        std::shared_ptr<Scene> createDefaultScene(RHIDevice& device) const;
        std::shared_ptr<Scene> loadScene(const std::string& filepath, RHIDevice& device);
        bool                   saveScene(const std::string& name, const std::string& filepath);
        bool                   saveCurrentScene(const std::string& filepath);

        void                   setCurrentScene(const std::string& name);
        void                   setCurrentScene(const std::shared_ptr<Scene>& scene);
        std::shared_ptr<Scene> getCurrentScene() const;
        std::shared_ptr<Scene> getCurrentOrNewScene();
        std::shared_ptr<Scene> getScene(const std::string& name) const;
        bool                   hasScene(const std::string& name) const;
        void                   removeScene(const std::string& name);

        void setOnSceneChanged(const SceneChangeCallback& callback);

    private:
        std::filesystem::path  m_asset_folder;
        AssetManager*          m_asset_mgr {nullptr};
        SceneTable             m_scenes;
        std::shared_ptr<Scene> m_current_scene {nullptr};
        SceneChangeCallback    m_on_scene_changed;
    };

} // namespace RealmEngine
