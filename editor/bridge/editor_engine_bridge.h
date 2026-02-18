#pragma once

#include <entt/entity/entity.hpp>
#include <filesystem>
#include <memory>
#include <string>

#include "module/render/viewport_display_mode.h"
#include "module/resource/config_manager.h"

namespace RealmEngine
{
    class ConfigManager;
    class Engine;
    class RHITexture;
    class SceneNode;
    class Scene;
    class EventBus;

    class EditorEngineBridge
    {
    public:
        explicit EditorEngineBridge(Engine& engine);
        ~EditorEngineBridge() noexcept = default;

        EditorEngineBridge(const EditorEngineBridge&)            = delete;
        EditorEngineBridge& operator=(const EditorEngineBridge&) = delete;
        EditorEngineBridge(EditorEngineBridge&&)                 = delete;
        EditorEngineBridge& operator=(EditorEngineBridge&&)      = delete;

        std::shared_ptr<Scene>      createDefaultScene() const;
        std::shared_ptr<Scene>      loadScene(const std::string& path) const;
        bool                        saveCurrentScene(const std::string& path) const;
        void                        setCurrentScene(std::shared_ptr<Scene> scene) const;
        std::shared_ptr<Scene>      getCurrentScene() const;
        std::filesystem::path       getConfigRootFolder() const;
        std::filesystem::path       getSceneFileFromConfig() const;
        void                        requestWindowClose() const;
        void                        initializeCameraForScene(const std::shared_ptr<Scene>& scene) const;
        std::filesystem::path       getAssetFolder() const;
        bool                        addModelToScene(const std::filesystem::path& model_path) const;
        std::shared_ptr<RHITexture> getTextureForPreview(const std::filesystem::path& path) const;
        ConfigManager&              getConfig() const;

        PipelineMode getPipelineMode() const;

        ViewportDisplayMode getViewportDisplayMode() const;
        void                setViewportDisplayMode(ViewportDisplayMode mode) const;
        void                setRenderToViewportTexture(bool enable) const;
        RHITexture*         getViewportTexture() const;
        void                bindDefaultFramebufferForImGui() const;

        // G-Buffer texture access (deferred only)
        RHITexture*                getGBufferAlbedoModelID() const;
        RHITexture*                getGBufferNormalMetallic() const;
        RHITexture*                getGBufferEmissiveRoughness() const;
        RHITexture*                getGBufferDepth() const;
        void                       saveConfig() const;
        std::shared_ptr<SceneNode> pasteEntityFromClipboard(const std::string&         json,
                                                            std::shared_ptr<SceneNode> parent) const;

        void      reloadCustomShaders() const;
        EventBus& getEventBus() const;

        void         getCameraViewProj(float* view, float* proj) const;
        entt::entity pickEntityAtViewport(float vp_x,
                                          float vp_y,
                                          float vp_w,
                                          float vp_h,
                                          float mouse_x,
                                          float mouse_y,
                                          int   render_w,
                                          int   render_h) const;

    private:
        Engine* m_engine;
    };

} // namespace RealmEngine
