#pragma once

#include <filesystem>
#include <memory>

#include "renderer/render_camera.h"
#include "renderer/render_pipeline.h"
#include "renderer/render_scene.h"
#include "renderer/viewport_display_mode.h"
#include "resource/config_manager.h"

namespace RealmEngine
{
    class Window;
    class ConfigManager;
    class RHIDevice;
    class RHITexture;

    class EquirectangularCubemap;
    class DiffuseIrradianceMap;
    class SpecularMap;
    class Skybox;
    class FullscreenQuad;
    class RHIFramebuffer;
    class SceneColorSource;

    class ShadowPass;
    class GeometryPass;
    class GBufferPass;
    class DeferredLightingPass;
    class HairPass;
    class SSAOPass;
    class SSAOBlurPass;
    class SSSPass;
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

        void initialize(ConfigManager& config, Window& window);
        void disposal();
        void render();
        void onResize(int width, int height);

        std::shared_ptr<RenderCamera> getCamera() const { return m_camera; }
        std::shared_ptr<RenderScene>  getRenderScene() const { return m_render_scene; }
        RHIDevice&                    getDevice() { return *m_device; }

        PipelineMode getPipelineMode() const { return m_pipeline_mode; }

        ViewportDisplayMode getViewportDisplayMode() const { return m_display_mode; }
        void                setViewportDisplayMode(ViewportDisplayMode mode) { m_display_mode = mode; }

        void        setRenderToViewportTexture(bool enable);
        RHITexture* getViewportTexture() const;

    private:
        void buildForwardPipeline(ConfigManager& config);
        void buildDeferredPipeline(ConfigManager& config);
        void precomputeIBL(ConfigManager& config);
        void createSharedFramebuffers(int width, int height, const RendererConfig& rc);
        void recreateSharedFramebuffers(int width, int height);

        // RHI
        std::unique_ptr<RHIDevice> m_device;

        // Pipeline
        PipelineMode   m_pipeline_mode {PipelineMode::Forward};
        RenderPipeline m_pipeline;

        // Scene color source -- points to either GeometryPass (forward) or
        // DeferredLightingPass (deferred). Non-owning; lifetime is the pipeline's.
        SceneColorSource* m_scene_color_source {nullptr};

        // Non-owning pass pointers for cross-pass wiring (forward)
        ShadowPass*      m_shadow_pass {nullptr};
        GeometryPass*    m_geometry_pass {nullptr};
        HairPass*        m_hair_pass {nullptr};
        SSAOPass*        m_ssao_pass {nullptr};
        SSAOBlurPass*    m_ssao_blur_pass {nullptr};
        SSSPass*         m_sss_pass {nullptr};
        SkyboxPass*      m_skybox_pass {nullptr};
        BloomPass*       m_bloom_pass {nullptr};
        PostProcessPass* m_postprocess_pass {nullptr};

        // Non-owning pass pointers (deferred-specific)
        GBufferPass*          m_gbuffer_pass {nullptr};
        DeferredLightingPass* m_deferred_lighting_pass {nullptr};

        std::unique_ptr<EquirectangularCubemap> m_ibl_equirect;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse;
        std::unique_ptr<SpecularMap>            m_ibl_specular;
        RHITexture*                             m_ibl_diffuse_tex {nullptr};
        RHITexture*                             m_ibl_prefiltered_tex {nullptr};
        RHITexture*                             m_ibl_brdf_tex {nullptr};

        std::unique_ptr<Skybox>         m_skybox;
        std::unique_ptr<FullscreenQuad> m_fullscreen_quad;

        // Scene & camera
        Window*                       m_window {nullptr};
        std::shared_ptr<RenderScene>  m_render_scene;
        std::shared_ptr<RenderCamera> m_camera;

        std::filesystem::path m_shader_path;

        ViewportDisplayMode             m_display_mode {ViewportDisplayMode::Lit};
        bool                            m_render_to_viewport_texture {false};
        std::unique_ptr<RHIFramebuffer> m_viewport_framebuffer;
    };

} // namespace RealmEngine
