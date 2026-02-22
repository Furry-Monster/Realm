#include "editor_engine_bridge.h"

#include "core/base/macros.h"
#include "core/math/aabb.h"
#include "engine.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_texture.h"
#include "functional/render/rhi/rhi_types.h"
#include "functional/resource/asset_manager.h"
#include "functional/resource/config_manager.h"
#include "functional/resource/config_serializer.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_manager.h"
#include "functional/scene/scene_node.h"
#include "functional/scene/scene_serializer.h"
#include "module/render/components/renderable.h"
#include "module/render/renderer.h"
#include "module/render/viewport_controller.h"
#include "platform/window/window.h"

#include <cstring>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
        if (!scene->getViewportController())
            scene->setViewportController(std::make_shared<ViewportController>());
        const GamePlayConfig& gp = m_engine->getConfig().getGamePlayConfig();
        scene->getViewportController()->initialize(m_engine->getRenderer().getCamera(),
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

    ViewportMode EditorEngineBridge::getViewportMode() const { return m_engine->getViewportMode(); }

    void EditorEngineBridge::setViewportMode(const ViewportMode mode) const { m_engine->setViewportMode(mode); }

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
                                                          const float mouse_y,
                                                          const int   render_w,
                                                          const int   render_h) const
    {
        const auto scene = m_engine->getSceneManager().getCurrentScene();
        if (!scene || render_w <= 0 || render_h <= 0)
            return entt::null;

        const float u = (mouse_x - vp_x) / vp_w;
        const float v = (mouse_y - vp_y) / vp_h;
        if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
            return entt::null;

        const glm::mat4  view = m_engine->getRenderer().getCamera()->getViewMatrix();
        const glm::mat4  proj = m_engine->getRenderer().getCamera()->getProjMatrix();
        const glm::ivec4 viewport(0, 0, render_w, render_h);

        const float win_x = u * static_cast<float>(render_w);
        const float win_y = (1.0f - v) * static_cast<float>(render_h);

        const glm::vec3 ray_origin = glm::unProject(glm::vec3(win_x, win_y, 0.0f), view, proj, viewport);
        const glm::vec3 ray_dir =
            glm::normalize(glm::unProject(glm::vec3(win_x, win_y, 1.0f), view, proj, viewport) - ray_origin);

        entt::entity hit       = entt::null;
        float        hit_t_min = std::numeric_limits<float>::max();

        for (const auto entity : scene->getRegistry().view<Renderable, WorldTransform>())
        {
            const auto* ro = scene->get<Renderable>(entity).render_object.get();
            if (!ro || ro->isEmpty())
                continue;

            const AABB  world_aabb = ro->getLocalAABB().transform(scene->get<WorldTransform>(entity).matrix);
            const float t          = world_aabb.rayIntersectT(ray_origin, ray_dir);
            if (t >= 0.1f && t < hit_t_min)
            {
                hit_t_min = t;
                hit       = entity;
            }
        }

        return hit;
    }

} // namespace RealmEngine
