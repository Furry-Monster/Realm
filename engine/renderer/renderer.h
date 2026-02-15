#pragma once

#include <filesystem>
#include <memory>

#include "renderer/render_camera.h"
#include "renderer/render_pipeline.h"
#include "renderer/render_scene.h"
#include "renderer/viewport_display_mode.h"

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

    class ShadowPass;
    class GeometryPass;
    class SSAOPass;
    class SSAOBlurPass;
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

        ViewportDisplayMode getViewportDisplayMode() const { return m_display_mode; }
        void                setViewportDisplayMode(ViewportDisplayMode mode) { m_display_mode = mode; }

        void        setRenderToViewportTexture(bool enable);
        RHITexture* getViewportTexture() const;

    private:
        void buildPipeline(ConfigManager& config);
        void precomputeIBL(ConfigManager& config);

        // RHI
        std::unique_ptr<RHIDevice> m_device;

        // Pipeline
        RenderPipeline m_pipeline;

        // Non-owning pass pointers for cross-pass wiring
        ShadowPass*      m_shadow_pass {nullptr};
        GeometryPass*    m_geometry_pass {nullptr};
        SSAOPass*        m_ssao_pass {nullptr};
        SSAOBlurPass*    m_ssao_blur_pass {nullptr};
        SkyboxPass*      m_skybox_pass {nullptr};
        BloomPass*       m_bloom_pass {nullptr};
        PostProcessPass* m_postprocess_pass {nullptr};

        std::unique_ptr<EquirectangularCubemap> m_ibl_equirect;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse;
        std::unique_ptr<SpecularMap>            m_ibl_specular;
        RHITexture*                             m_ibl_diffuse_tex {nullptr};
        RHITexture*                             m_ibl_prefiltered_tex {nullptr};
        RHITexture*                             m_ibl_brdf_tex {nullptr};

        // Skybox + fullscreen quad (still use GL internally for this phase)
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
