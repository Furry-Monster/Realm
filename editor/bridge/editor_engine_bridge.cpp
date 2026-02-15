#include "editor_engine_bridge.h"

#include "core/log/log_macros.h"
#include "engine.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/asset_manager.h"
#include "resource/config_manager.h"
#include "resource/config_serializer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"
#include "scene/components/camera_controller.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "scene/scene_node.h"
#include "scene/serialization/scene_serializer.h"

#include <filesystem>
#include <glm/glm.hpp>

namespace RealmEngine
{
    EditorEngineBridge::EditorEngineBridge(Engine& engine) : m_engine(&engine) {}

    std::shared_ptr<Scene> EditorEngineBridge::createDefaultScene()
    {
        return m_engine->getSceneManager().createDefaultScene(m_engine->getRenderer().getDevice());
    }

    std::shared_ptr<Scene> EditorEngineBridge::loadScene(const std::string& path)
    {
        return m_engine->getSceneManager().loadScene(path, m_engine->getRenderer().getDevice());
    }

    bool EditorEngineBridge::saveCurrentScene(const std::string& path)
    {
        return m_engine->getSceneManager().saveCurrentScene(path);
    }

    void EditorEngineBridge::setCurrentScene(std::shared_ptr<Scene> scene)
    {
        m_engine->getSceneManager().setCurrentScene(std::move(scene));
    }

    std::shared_ptr<Scene> EditorEngineBridge::getCurrentScene()
    {
        return m_engine->getSceneManager().getCurrentScene();
    }

    std::filesystem::path EditorEngineBridge::getConfigRootFolder() const
    {
        return m_engine->getConfig().getRootFolder();
    }

    std::filesystem::path EditorEngineBridge::getSceneFileFromConfig() const
    {
        return m_engine->getConfig().getRootFolder() / m_engine->getConfig().getGamePlayConfig().scene_file;
    }

    void EditorEngineBridge::requestWindowClose() { m_engine->getWindow().requestClose(); }

    void EditorEngineBridge::initializeCameraForScene(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;
        const GamePlayConfig& gp = m_engine->getConfig().getGamePlayConfig();
        scene->getCameraController()->initialize(m_engine->getRenderer().getCamera(),
                                                 m_engine->getInput(),
                                                 gp.camera_mouse_sensitivity,
                                                 gp.camera_move_speed,
                                                 gp.camera_sprint_multiplier);
    }

    std::filesystem::path EditorEngineBridge::getAssetFolder() const { return m_engine->getConfig().getAssetFolder(); }

    std::shared_ptr<RHITexture> EditorEngineBridge::getTextureForPreview(const std::filesystem::path& path)
    {
        auto& device = m_engine->getRenderer().getDevice();
        auto& assets = m_engine->getAssets();
        return assets.getOrLoadTextureForPreview(path.generic_string(), device);
    }

    bool EditorEngineBridge::addModelToScene(const std::filesystem::path& model_path)
    {
        auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene)
        {
            RE_LOG_WARN("No scene loaded, cannot add model");
            return false;
        }

        std::string model_path_str = model_path.generic_string();
        std::string entity_name    = model_path.stem().string();

        try
        {
            auto node   = scene->createNodeWithEntity(entity_name);
            auto entity = scene->findEntity(entity_name);

            auto& transform    = entity.emplace<Transform>();
            transform.position = glm::vec3(0.0f, 0.0f, 0.0f);

            auto& renderable      = entity.emplace<Renderable>();
            renderable.model_path = model_path_str;
            renderable.loadModel(m_engine->getRenderer().getDevice(), m_engine->getAssets());

            scene->getRoot()->addChild(node);
            scene->markDirty();

            RE_LOG_INFO("Added model to scene: " + entity_name);
            return true;
        }
        catch (const std::exception& e)
        {
            RE_LOG_ERROR("Failed to add model to scene: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<SceneNode> EditorEngineBridge::pasteEntityFromClipboard(const std::string&         json,
                                                                            std::shared_ptr<SceneNode> parent)
    {
        auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene || !parent)
            return nullptr;
        auto* device = &m_engine->getRenderer().getDevice();
        auto* assets = &m_engine->getAssets();
        auto  node   = SceneSerializer::pasteNodeFromJson(json, *scene, std::move(parent), *device, assets);
        if (node)
            scene->markDirty();
        return node;
    }

    ConfigManager& EditorEngineBridge::getConfig() { return m_engine->getConfig(); }

    void EditorEngineBridge::saveConfig()
    {
        std::filesystem::path path = m_engine->getConfig().getRootFolder() / "config.json";
        ConfigSerializer::saveToFile(m_engine->getConfig(), path.string());
    }

    EventBus& EditorEngineBridge::getEventBus() { return m_engine->getEventBus(); }

    ViewportDisplayMode EditorEngineBridge::getViewportDisplayMode() const
    {
        return m_engine->getRenderer().getViewportDisplayMode();
    }

    void EditorEngineBridge::setViewportDisplayMode(ViewportDisplayMode mode)
    {
        m_engine->getRenderer().setViewportDisplayMode(mode);
    }

    void EditorEngineBridge::setRenderToViewportTexture(bool enable)
    {
        m_engine->getRenderer().setRenderToViewportTexture(enable);
    }

    RHITexture* EditorEngineBridge::getViewportTexture() const { return m_engine->getRenderer().getViewportTexture(); }

    void EditorEngineBridge::bindDefaultFramebufferForImGui() const
    {
        auto& device = m_engine->getRenderer().getDevice();
        device.bindDefaultFramebuffer();
        device.setClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        device.clear(ClearFlags::Color | ClearFlags::Depth);
    }

} // namespace RealmEngine
