#pragma once

#include <filesystem>
#include <memory>

#include "renderer/bloom_framebuffer.h"
#include "renderer/fullscreen_quad.h"
#include "renderer/ibl/diffuse_irradiance_map.h"
#include "renderer/ibl/equirectangular_cubemap.h"
#include "renderer/ibl/specular_map.h"
#include "renderer/pbr_framebuffer.h"
#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "renderer/shader.h"
#include "renderer/shadow_framebuffer.h"
#include "renderer/skybox.h"

namespace RealmEngine
{
    class Window;
    class ConfigManager;

    class Renderer
    {
    public:
        Renderer()           = default;
        ~Renderer() noexcept = default;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;

        void initialize(ConfigManager& config, Window& window);
        void disposal();
        void render();

        std::shared_ptr<RenderCamera> getCamera() const { return m_camera; }
        std::shared_ptr<RenderScene>  getRenderScene() const { return m_render_scene; }

    private:
        void compileShaders();
        void precomputeIBL();

        void renderShadow();
        void renderSkybox();
        void renderBloom();
        void applyPostprocess();

        // shadow pass
        std::unique_ptr<ShadowFramebuffer> m_shadow_framebuffer;
        glm::mat4                          m_light_space_matrix;
        bool                               m_shadow_enabled = false;

        // main pass
        std::unique_ptr<PBRFramebuffer>         m_pbr_framebuffer;
        std::unique_ptr<Skybox>                 m_ibl_skybox;
        std::unique_ptr<EquirectangularCubemap> m_ibl_equirectangular_cubemap;
        std::unique_ptr<DiffuseIrradianceMap>   m_ibl_diffuse_irradiance_map;
        std::unique_ptr<SpecularMap>            m_ibl_specular_map;

        // post pass
        std::unique_ptr<FullscreenQuad>   m_fullscreen_quad;
        bool                              m_bloom_enabled           = true;
        float                             m_bloom_intensity         = 1.0f;
        int                               m_bloom_iterations        = 10;
        BloomDirection                    m_bloom_direction         = BloomDirection::BOTH;
        bool                              m_tonemapping_enabled     = true;
        float                             m_gamma_correction_factor = 2.2f;
        float                             m_bloom_brightness_cutoff = 1.0f;
        std::unique_ptr<BloomFramebuffer> m_bloom_framebuffers[2];
        unsigned int                      m_bloom_result_id = 0;

        // clear color (from config)
        float m_clear_color_r = 0.0f;
        float m_clear_color_g = 0.0f;
        float m_clear_color_b = 0.0f;
        float m_clear_color_a = 1.0f;

        // misc
        Window*                       m_window {nullptr};
        std::shared_ptr<RenderScene>  m_render_scene;
        std::shared_ptr<RenderCamera> m_camera;

        std::filesystem::path m_root_path;
        std::filesystem::path m_shader_path;
        std::filesystem::path m_asset_path;

        std::unique_ptr<Shader> m_pbr_shader;
        std::unique_ptr<Shader> m_bloom_shader;
        std::unique_ptr<Shader> m_post_shader;
        std::unique_ptr<Shader> m_skybox_shader;
        std::unique_ptr<Shader> m_shadow_shader;

        std::unique_ptr<LightUBO> m_light_ubo;
    };
} // namespace RealmEngine
