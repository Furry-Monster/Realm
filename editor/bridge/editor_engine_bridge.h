#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace RealmEngine
{
    class ConfigManager;
    class Engine;
    class SceneNode;
    class Scene;
    class EventBus;

    class EditorEngineBridge
    {
    public:
        explicit EditorEngineBridge(Engine& engine);
        ~EditorEngineBridge() = default;

        EditorEngineBridge(const EditorEngineBridge&)            = delete;
        EditorEngineBridge& operator=(const EditorEngineBridge&) = delete;
        EditorEngineBridge(EditorEngineBridge&&)                 = delete;
        EditorEngineBridge& operator=(EditorEngineBridge&&)      = delete;

        std::shared_ptr<Scene>     createDefaultScene();
        std::shared_ptr<Scene>     loadScene(const std::string& path);
        bool                       saveCurrentScene(const std::string& path);
        void                       setCurrentScene(std::shared_ptr<Scene> scene);
        std::shared_ptr<Scene>     getCurrentScene();
        std::filesystem::path      getConfigRootFolder() const;
        std::filesystem::path      getSceneFileFromConfig() const;
        void                       requestWindowClose();
        void                       initializeCameraForScene(std::shared_ptr<Scene> scene);
        std::filesystem::path      getAssetFolder() const;
        bool                       addModelToScene(const std::filesystem::path& model_path);
        ConfigManager&             getConfig();
        void                       saveConfig();
        std::shared_ptr<SceneNode> pasteEntityFromClipboard(const std::string& json, std::shared_ptr<SceneNode> parent);

        EventBus& getEventBus();

    private:
        Engine* m_engine;
    };

} // namespace RealmEngine
