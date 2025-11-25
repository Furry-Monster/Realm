#pragma once

#include <filesystem>
#include <memory>

#include "render/bloom_framebuffer.h"
#include "render/fullscreen_quad.h"
#include "render/ibl/diffuse_irradiance_map.h"
#include "render/ibl/equirectangular_cubemap.h"
#include "render/ibl/specular_map.h"
#include "render/pbr_framebuffer.h"
#include "render/render_camera.h"
#include "render/render_scene.h"
#include "render/shader.h"
#include "render/skybox.h"

namespace RealmEngine
{
    class Window;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() noexcept = default;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;

        void initialize();
        void disposal();
        void render(std::shared_ptr<RenderScene> render_scene);

        std::shared_ptr<RenderCamera> getCamera() const { return m_camera; }

    private:
        void setupShaders();
        void setupIBL();

        void renderSkybox();
        void renderBloom();
        void renderPostprocess();

        // main pass
        std::unique_ptr<PBRFramebuffer>         m_pbr_framebuffer;
        std::unique_ptr<Skybox>                 m_ibl_skybox;
        std::unique_ptr<EquirectangularCubemap> m_ibl_equirectangular_cubemap;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse_irradiance_map;
        std::unique_ptr<SpecularMap>            m_ibl_specular_map;

        // post-processing stuff
        std::unique_ptr<FullscreenQuad>   m_fullscreen_quad;
        bool                              m_bloom_enabled           = true;
        float                             m_bloom_intensity         = 1.0f;
        int                               m_bloom_iterations        = 10;
        BloomDirection                    m_bloom_direction         = BloomDirection::BOTH;
        bool                              m_tonemapping_enabled     = true;
        float                             m_gamma_correction_factor = 2.2f;
        float                             m_bloom_brightness_cutoff = 1.0f;
        std::unique_ptr<BloomFramebuffer> m_bloom_framebuffers[2];
        unsigned int                      m_bloom_framebuffer_result;

        // misc rendering stuff
        std::shared_ptr<Window>       m_window;
        std::shared_ptr<RenderScene>  m_scene;
        std::shared_ptr<RenderCamera> m_camera;

        std::filesystem::path m_root_path;
        std::filesystem::path m_shader_path;
        std::filesystem::path m_asset_path;

        std::unique_ptr<Shader> m_pbr_shader;
        std::unique_ptr<Shader> m_bloom_shader;
        std::unique_ptr<Shader> m_post_shader;
        std::unique_ptr<Shader> m_skybox_shader;
    };
} // namespace RealmEngine
