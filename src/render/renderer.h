#pragma once

#include "gameplay/scene.h"
#include "render/bloom_framebuffer.h"
#include "render/framebuffer.h"
#include "render/fullscreen_quad.h"
#include "render/ibl/diffuse_irradiance_map.h"
#include "render/ibl/equirectangular_cubemap.h"
#include "render/ibl/specular_map.h"
#include "render/render_camera.h"
#include "render/shader.h"
#include "render/skybox.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Window;

    enum class BloomDirection
    {
        BOTH       = 0,
        HORIZONTAL = 1,
        VERTICAL   = 2
    };

    class Renderer
    {
    public:
        Renderer()           = default;
        ~Renderer() noexcept = default;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;

        void initialize(std::shared_ptr<Window> window);
        void disposal();
        void render(std::shared_ptr<Scene> scene);

        std::shared_ptr<RenderCamera> getCamera() const { return m_camera; }

    private:
        void setupShaders();
        void setupFramebuffers();
        void setupIBL();
        void renderBloom();

        std::shared_ptr<Window> m_window;

        // framebuffers
        std::unique_ptr<Framebuffer>      m_framebuffer;
        std::unique_ptr<BloomFramebuffer> m_bloom_framebuffers[2];
        unsigned int                      m_bloom_framebuffer_result;

        // pre-computed IBL stuff
        std::unique_ptr<EquirectangularCubemap> m_ibl_equirectangular_cubemap;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse_irradiance_map;
        std::unique_ptr<SpecularMap>            m_ibl_specular_map;

        std::unique_ptr<Skybox>       m_skybox;
        std::shared_ptr<Scene>        m_scene;
        std::shared_ptr<RenderCamera> m_camera;

        std::unique_ptr<Shader> m_pbr_shader;
        std::unique_ptr<Shader> m_bloom_shader;
        std::unique_ptr<Shader> m_post_shader;
        std::unique_ptr<Shader> m_skybox_shader;

        // post-processing stuff
        std::unique_ptr<FullscreenQuad> m_fullscreen_quad;
        bool                            m_bloom_enabled           = true;
        float                           m_bloom_intensity         = 1.0f;
        int                             m_bloom_iterations        = 10;
        BloomDirection                  m_bloom_direction         = BloomDirection::BOTH;
        bool                            m_tonemapping_enabled     = false;
        float                           m_gamma_correction_factor = 2.2f;
        float                           m_bloom_brightness_cutoff = 1.0f;

        std::string m_shader_root_path;
        std::string m_engine_root_path;
        std::string m_hdri_path;

        // IBL texture units
        static const int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10;
        static const int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
        static const int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    };
} // namespace RealmEngine
