#pragma once

#include <filesystem>
#include <memory>

#include "functional/render/render_camera.h"
#include "functional/render/render_pipeline.h"
#include "functional/render/render_scene.h"
#include "functional/render/viewport_display_mode.h"
#include "functional/resource/config_manager.h"

namespace RealmEngine
{
    class Window;
    class ConfigManager;
    class RHIDevice;
    class Scene;
    class RHITexture;

    class EquirectangularCubemap;
    class DiffuseIrradianceMap;
    class SpecularMap;
    class Skybox;
    class FullscreenQuad;
    class RHIFramebuffer;
    class SceneColorSource;

    class LightProbeBaker;
    class CSMShadowPass;
    class OpaquePass;
    class TransparentPass;
    class GBufferPass;
    class DeferredLightingPass;
    class PointShadowPass;
    class SpotShadowPass;
    class ClusteredLightCullPass;
    class GTAOPass;
    class GTAOBlurPass;
    class HiZPass;
    class SSRPass;
    class SkyboxPass;
    class BloomPass;
    class PostProcessPass;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() noexcept;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;

        void initialize(const ConfigManager& config, Window& window);
        void disposal();
        void render();
        void onResize(int width, int height);

        std::shared_ptr<RenderCamera> getCamera() const { return m_camera; }
        std::shared_ptr<RenderScene>  getRenderScene() const { return m_render_scene; }
        RHIDevice&                    getDevice() { return *m_device; }

        PipelineMode getPipelineMode() const { return m_pipeline_mode; }

        void reloadCustomShaders();

        ViewportDisplayMode getViewportDisplayMode() const { return m_display_mode; }
        void                setViewportDisplayMode(const ViewportDisplayMode mode) { m_display_mode = mode; }

        void        setRenderToViewportTexture(bool enable);
        RHITexture* getViewportTexture() const;

        void setCurrentEcsScene(Scene* scene) { m_current_ecs_scene = scene; }

        LightProbeBaker* getLightProbeBaker() const { return m_probe_baker.get(); }

        enum class GBufferSlot
        {
            AlbedoModelID,
            NormalMetallic,
            EmissiveRoughness,
            Depth
        };
        RHITexture* getGBufferTexture(GBufferSlot slot) const;
        RHITexture* getGBufferAlbedoModelID() const { return getGBufferTexture(GBufferSlot::AlbedoModelID); }
        RHITexture* getGBufferNormalMetallic() const { return getGBufferTexture(GBufferSlot::NormalMetallic); }
        RHITexture* getGBufferEmissiveRoughness() const { return getGBufferTexture(GBufferSlot::EmissiveRoughness); }
        RHITexture* getGBufferDepth() const { return getGBufferTexture(GBufferSlot::Depth); }

    private:
        void buildForwardPipeline(const ConfigManager& config);
        void buildDeferredPipeline(const ConfigManager& config);
        void precomputeIBL(const ConfigManager& config);
        void createSharedFramebuffers(int width, int height, const RendererConfig& rc);
        void recreateSharedFramebuffers(int width, int height);

        std::unique_ptr<RHIDevice> m_device;

        PipelineMode   m_pipeline_mode {PipelineMode::Forward};
        RenderPipeline m_pipeline;

        SceneColorSource* m_scene_color_source {nullptr};

        CSMShadowPass*          m_shadow_pass {nullptr};
        PointShadowPass*        m_point_shadow_pass {nullptr};
        SpotShadowPass*         m_spot_shadow_pass {nullptr};
        OpaquePass*             m_opaque_pass {nullptr};
        TransparentPass*        m_transparent_pass {nullptr};
        ClusteredLightCullPass* m_cluster_cull_pass {nullptr};
        GTAOPass*               m_gtao_pass {nullptr};
        GTAOBlurPass*           m_gtao_blur_pass {nullptr};
        HiZPass*                m_hiz_pass {nullptr};
        SSRPass*                m_ssr_pass {nullptr};
        SkyboxPass*             m_skybox_pass {nullptr};
        BloomPass*              m_bloom_pass {nullptr};
        PostProcessPass*        m_postprocess_pass {nullptr};

        GBufferPass*          m_gbuffer_pass {nullptr};
        DeferredLightingPass* m_deferred_lighting_pass {nullptr};

        std::unique_ptr<EquirectangularCubemap> m_ibl_equirect;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse;
        std::unique_ptr<SpecularMap>            m_ibl_specular;
        RHITexture*                             m_ibl_diffuse_tex {nullptr};
        RHITexture*                             m_ibl_prefiltered_tex {nullptr};
        RHITexture*                             m_ibl_brdf_tex {nullptr};

        std::unique_ptr<LightProbeBaker> m_probe_baker;
        std::unique_ptr<Skybox>          m_skybox;
        std::unique_ptr<FullscreenQuad>  m_fullscreen_quad;
        std::unique_ptr<RHITexture>      m_default_white;

        Window*                       m_window {nullptr};
        Scene*                        m_current_ecs_scene {nullptr};
        std::shared_ptr<RenderScene>  m_render_scene;
        std::shared_ptr<RenderCamera> m_camera;

        std::filesystem::path m_shader_path;

        ViewportDisplayMode             m_display_mode {ViewportDisplayMode::Lit};
        bool                            m_render_to_viewport_texture {false};
        std::unique_ptr<RHIFramebuffer> m_viewport_framebuffer;
    };

} // namespace RealmEngine
