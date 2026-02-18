#include "editor_engine_bridge.h"

#include "core/base/macros.h"
#include "engine.h"
#include "platform/window/window.h"
#include "render/renderer.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_texture.h"
#include "render/rhi/rhi_types.h"
#include "resource/asset_manager.h"
#include "resource/config_manager.h"
#include "resource/config_serializer.h"
#include "scene/components/camera_controller.h"
#include "scene/components/renderable.h"
#include "scene/components/transform.h"
#include "scene/components/world_transform.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "scene/scene_node.h"
#include "scene/serialization/scene_serializer.h"

#include <cstring>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

namespace RealmEngine
{
    EditorEngineBridge::EditorEngineBridge(Engine& engine) : m_engine(&engine) {}

    std::shared_ptr<Scene> EditorEngineBridge::createDefaultScene() const
    {
        return m_engine->getSceneManager().createDefaultScene(m_engine->getRenderer().getDevice());
    }

    std::shared_ptr<Scene> EditorEngineBridge::loadScene(const std::string& path) const
    {
        return m_engine->getSceneManager().loadScene(path, m_engine->getRenderer().getDevice());
    }

    bool EditorEngineBridge::saveCurrentScene(const std::string& path) const
    {
        return m_engine->getSceneManager().saveCurrentScene(path);
    }

    void EditorEngineBridge::setCurrentScene(std::shared_ptr<Scene> scene) const
    {
        m_engine->getSceneManager().setCurrentScene(std::move(scene));
    }

    std::shared_ptr<Scene> EditorEngineBridge::getCurrentScene() const
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

    void EditorEngineBridge::requestWindowClose() const { m_engine->getWindow().requestClose(); }

    void EditorEngineBridge::initializeCameraForScene(const std::shared_ptr<Scene>& scene) const
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

    std::shared_ptr<RHITexture> EditorEngineBridge::getTextureForPreview(const std::filesystem::path& path) const
    {
        auto& device = m_engine->getRenderer().getDevice();
        auto& assets = m_engine->getAssets();
        return assets.getOrLoadTextureForPreview(path.generic_string(), device);
    }

    bool EditorEngineBridge::addModelToScene(const std::filesystem::path& model_path) const
    {
        const auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene)
        {
            RE_LOG_WARN("No scene loaded, cannot add model");
            return false;
        }

        const std::string model_path_str = model_path.generic_string();
        const std::string entity_name    = model_path.stem().string();

        try
        {
            const auto node   = scene->createNodeWithEntity(entity_name);
            auto       entity = scene->findEntity(entity_name);

            auto& transform    = entity.emplace<Transform>();
            transform.position = glm::vec3(0.0f, 0.0f, 0.0f);

            auto& renderable      = entity.emplace<Renderable>();
            renderable.model_path = model_path_str;
            loadRenderableModel(renderable, m_engine->getRenderer().getDevice(), m_engine->getAssets());

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
                                                                            std::shared_ptr<SceneNode> parent) const
    {
        const auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene || !parent)
            return nullptr;
        auto* device = &m_engine->getRenderer().getDevice();
        auto* assets = &m_engine->getAssets();
        auto  node   = SceneSerializer::pasteNodeFromJson(json, *scene, std::move(parent), *device, assets);
        if (node)
            scene->markDirty();
        return node;
    }

    ConfigManager& EditorEngineBridge::getConfig() const { return m_engine->getConfig(); }

    void EditorEngineBridge::saveConfig() const
    {
        const std::filesystem::path path = m_engine->getConfig().getRootFolder() / "config.json";
        ConfigSerializer::saveToFile(m_engine->getConfig(), path.string());
    }

    void EditorEngineBridge::reloadCustomShaders() const { m_engine->getRenderer().reloadCustomShaders(); }

    EventBus& EditorEngineBridge::getEventBus() const { return m_engine->getEventBus(); }

    PipelineMode EditorEngineBridge::getPipelineMode() const { return m_engine->getRenderer().getPipelineMode(); }

    ViewportDisplayMode EditorEngineBridge::getViewportDisplayMode() const
    {
        return m_engine->getRenderer().getViewportDisplayMode();
    }

    void EditorEngineBridge::setViewportDisplayMode(const ViewportDisplayMode mode) const
    {
        m_engine->getRenderer().setViewportDisplayMode(mode);
    }

    void EditorEngineBridge::setRenderToViewportTexture(const bool enable) const
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

    RHITexture* EditorEngineBridge::getGBufferAlbedoModelID() const
    {
        return m_engine->getRenderer().getGBufferAlbedoModelID();
    }

    RHITexture* EditorEngineBridge::getGBufferNormalMetallic() const
    {
        return m_engine->getRenderer().getGBufferNormalMetallic();
    }

    RHITexture* EditorEngineBridge::getGBufferEmissiveRoughness() const
    {
        return m_engine->getRenderer().getGBufferEmissiveRoughness();
    }

    RHITexture* EditorEngineBridge::getGBufferDepth() const { return m_engine->getRenderer().getGBufferDepth(); }

    void EditorEngineBridge::getCameraViewProj(float* view, float* proj) const
    {
        const auto& cam = m_engine->getRenderer().getCamera();
        if (!cam || !view || !proj)
            return;
        memcpy(view, glm::value_ptr(cam->getViewMatrix()), 16 * sizeof(float));
        memcpy(proj, glm::value_ptr(cam->getProjMatrix()), 16 * sizeof(float));
    }

    entt::entity EditorEngineBridge::pickEntityAtViewport(const float vp_x,
                                                          const float vp_y,
                                                          const float vp_w,
                                                          const float vp_h,
                                                          const float mouse_x,
                                                          const float mouse_y) const
    {
        const auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene)
            return entt::null;

        const glm::mat4 view   = m_engine->getRenderer().getCamera()->getViewMatrix();
        const glm::mat4 proj   = m_engine->getRenderer().getCamera()->getProjMatrix();
        const glm::mat4 inv_vp = glm::inverse(proj * view);

        const float ndc_x = (mouse_x - vp_x) / vp_w * 2.0f - 1.0f;
        const float ndc_y = 1.0f - (mouse_y - vp_y) / vp_h * 2.0f;

        const glm::vec4 near_ndc(ndc_x, ndc_y, 0.0f, 1.0f);
        const glm::vec4 far_ndc(ndc_x, ndc_y, 1.0f, 1.0f);
        const glm::vec4 near_ws = inv_vp * near_ndc;
        const glm::vec4 far_ws  = inv_vp * far_ndc;

        const glm::vec3 ray_origin(near_ws / near_ws.w);
        const glm::vec3 ray_dir = glm::normalize(glm::vec3(far_ws / far_ws.w) - ray_origin);

        entt::entity hit       = entt::null;
        float        hit_t_min = std::numeric_limits<float>::max();

        const auto view_registry = scene->getRegistry().view<Renderable, WorldTransform>();
        for (const auto entity : view_registry)
        {
            const glm::vec3 center = glm::vec3(scene->get<WorldTransform>(entity).matrix[3]);
            const float     t      = glm::dot(center - ray_origin, ray_dir);
            if (t < 0.1f)
                continue;

            const glm::vec3 proj_pt        = ray_origin + ray_dir * t;
            const float     dist_sq        = glm::dot(center - proj_pt, center - proj_pt);
            const float     pick_radius_sq = 4.0f * 4.0f;
            if (dist_sq < pick_radius_sq && t < hit_t_min)
            {
                hit_t_min = t;
                hit       = entity;
            }
        }

        return hit;
    }

} // namespace RealmEngine
