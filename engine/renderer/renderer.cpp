#include "renderer/renderer.h"
#include <functional>
#include <memory>
#include <optional>

#include "core/log/log_macros.h"
#include "platform/window/window.h"
#include "renderer/render_scene.h"
#include "resource/config_manager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace RealmEngine
{
    void Renderer::initialize(ConfigManager& config, Window& window)
    {
        m_window = &window;

        m_root_path   = config.getRootFolder();
        m_shader_path = config.getShaderFolder();
        m_asset_path  = config.getAssetFolder();

        m_camera       = std::make_shared<RenderCamera>();
        m_render_scene = std::make_shared<RenderScene>();
        m_light_ubo    = std::make_unique<LightUBO>();

        compileShaders();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        const RendererConfig& render_config = config.getRendererConfig();

        m_camera->initialize();
        m_camera->setPerspective(render_config.camera_fov,
                                 static_cast<float>(m_window->getWidth()) / static_cast<float>(m_window->getHeight()),
                                 render_config.camera_near_plane,
                                 render_config.camera_far_plane);
        m_camera->setPosition(glm::vec3(render_config.camera_initial_pos_x,
                                        render_config.camera_initial_pos_y,
                                        render_config.camera_initial_pos_z));
        m_camera->lookAt(
            glm::vec3(render_config.camera_look_at_x, render_config.camera_look_at_y, render_config.camera_look_at_z));

        m_bloom_enabled           = render_config.bloom_enabled;
        m_bloom_intensity         = render_config.bloom_intensity;
        m_bloom_iterations        = render_config.bloom_iterations;
        m_bloom_direction         = static_cast<BloomDirection>(render_config.bloom_direction);
        m_bloom_brightness_cutoff = render_config.bloom_brightness_cutoff;
        m_tonemapping_enabled     = render_config.tonemapping_enabled;
        m_gamma_correction_factor = render_config.gamma_correction_factor;

        m_clear_color_r = render_config.clear_color_r;
        m_clear_color_g = render_config.clear_color_g;
        m_clear_color_b = render_config.clear_color_b;
        m_clear_color_a = render_config.clear_color_a;

        m_shadow_framebuffer = std::make_unique<ShadowFramebuffer>(SHADOW_WIDTH, SHADOW_HEIGHT);
        m_shadow_framebuffer->init();

        m_pbr_framebuffer = std::make_unique<PBRFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_pbr_framebuffer->init();

        // IBL precomputation uses hdri_path from config
        std::string root_path = m_root_path.generic_string();
        std::string hdri_path = (m_asset_path / render_config.hdri_path).generic_string();

        m_ibl_equirectangular_cubemap = std::make_unique<EquirectangularCubemap>(root_path, hdri_path);
        m_ibl_equirectangular_cubemap->compute();

        m_ibl_diffuse_irradiance_map =
            std::make_unique<DiffuseIrradianceMap>(root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_diffuse_irradiance_map->compute();

        m_ibl_specular_map = std::make_unique<SpecularMap>(root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_specular_map->computePrefilteredEnvMap();
        m_ibl_specular_map->computeBrdfConvolutionMap();

        m_ibl_skybox = std::make_unique<Skybox>(m_ibl_equirectangular_cubemap->getCubemapId());

        m_fullscreen_quad       = std::make_unique<FullscreenQuad>();
        m_bloom_framebuffers[0] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[0]->init();
        m_bloom_framebuffers[1] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[1]->init();

        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

        RE_LOG_INFO("Renderer initialized.");
    }

    void Renderer::disposal()
    {
        m_pbr_shader.reset();
        m_bloom_shader.reset();
        m_post_shader.reset();
        m_skybox_shader.reset();
        m_shadow_shader.reset();

        m_pbr_framebuffer.reset();
        m_shadow_framebuffer.reset();
        m_ibl_equirectangular_cubemap.reset();
        m_ibl_diffuse_irradiance_map.reset();
        m_ibl_specular_map.reset();
        m_ibl_skybox.reset();
        m_fullscreen_quad.reset();
        m_bloom_framebuffers[0].reset();
        m_bloom_framebuffers[1].reset();

        m_camera->disposal();
        m_camera.reset();
        m_render_scene.reset();
        m_window = nullptr;

        RE_LOG_INFO("Renderer shutdown.");
    }

    void Renderer::compileShaders()
    {
        std::filesystem::path vertex_path;
        std::filesystem::path fragment_path;

        vertex_path   = m_shader_path / "pbr.vert";
        fragment_path = m_shader_path / "pbr.frag";
        m_pbr_shader  = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        m_pbr_shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);

        vertex_path    = m_shader_path / "bloom.vert";
        fragment_path  = m_shader_path / "bloom.frag";
        m_bloom_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path   = m_shader_path / "post.vert";
        fragment_path = m_shader_path / "post.frag";
        m_post_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path     = m_shader_path / "skybox.vert";
        fragment_path   = m_shader_path / "skybox.frag";
        m_skybox_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path     = m_shader_path / "shadow.vert";
        fragment_path   = m_shader_path / "shadow.frag";
        m_shadow_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());
    }

    void Renderer::render()
    {
        if (!m_render_scene)
        {
            RE_LOG_ERROR("Render scene not set.");
            return;
        }

        renderShadow();

        // Main pass
        m_pbr_framebuffer->bind();
        m_pbr_shader->use();

        // set basic opengl states
        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

        glClearColor(m_clear_color_r, m_clear_color_g, m_clear_color_b, m_clear_color_a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // update camera
        m_camera->update();
        glm::vec3 camera_position = m_camera->getPosition();
        glm::mat4 projection      = m_camera->getProjMatrix();
        glm::mat4 view            = m_camera->getViewMatrix();
        m_pbr_shader->setVec3("cameraPosition", camera_position);

        // update light ubo
        m_light_ubo->updateLights(m_render_scene->m_lights);
        m_light_ubo->bind(LIGHT_UBO_BINDING_POINT);

        // set IBL stuff
        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        m_pbr_shader->setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ibl_diffuse_irradiance_map->getCubemapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        m_pbr_shader->setInt("prefilteredEnvMap", TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ibl_specular_map->getPrefilteredEnvMapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        m_pbr_shader->setInt("brdfConvolutionMap", TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        glBindTexture(GL_TEXTURE_2D, m_ibl_specular_map->getBrdfConvolutionMapId());

        // set post stuff
        m_pbr_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);

        // set shadow stuff
        if (m_shadow_enabled)
        {
            glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_SHADOW_MAP);
            m_pbr_shader->setInt("shadowMap", TEXTURE_UNIT_SHADOW_MAP);
            glBindTexture(GL_TEXTURE_2D, m_shadow_framebuffer->getDepthTextureId());
            m_pbr_shader->setMat4("lightSpaceMatrix", m_light_space_matrix);
            m_pbr_shader->setBool("shadowEnabled", true);
        }
        else
        {
            m_pbr_shader->setMat4("lightSpaceMatrix", glm::mat4(1.0f)); // identity matrix when no shadow
            m_pbr_shader->setBool("shadowEnabled", false);
        }

        // Render entities
        for (auto& ro : m_render_scene->m_render_objects)
        {
            glm::mat4 model = glm::mat4(1.0f);

            auto rotation_matrix = glm::toMat4(ro->getOrientation());
            model                = rotation_matrix * model;
            model                = glm::translate(model, ro->getPosition());
            model                = glm::scale(model, ro->getScale());

            m_pbr_shader->setModelViewProjectionMatrices(model, view, projection);

            glDepthMask(GL_TRUE);
            ro->draw(*m_pbr_shader);
        }

        renderSkybox();

        renderBloom();

        applyPostprocess();
    }

    void Renderer::renderShadow()
    {
        // Shadow pass
        std::optional<std::reference_wrapper<Light>> directional_light;
        for (auto& light : m_render_scene->m_lights)
        {
            if (light.type == LightType::Directional)
            {
                directional_light = std::ref(light);
                break;
            }
        }

        if (!directional_light.has_value())
        {
            m_shadow_enabled = false;
            return;
        }

        m_shadow_framebuffer->bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        m_shadow_shader->use();

        // Orthographic projection for directional light shadow
        static constexpr float SHADOW_NEAR_PLANE   = 0.1f;
        static constexpr float SHADOW_FAR_PLANE    = 50.0f;
        static constexpr float SHADOW_FRUSTUM_SIZE = 20.0f;

        Light&    light            = directional_light->get();
        glm::mat4 light_projection = glm::ortho(-SHADOW_FRUSTUM_SIZE,
                                                SHADOW_FRUSTUM_SIZE,
                                                -SHADOW_FRUSTUM_SIZE,
                                                SHADOW_FRUSTUM_SIZE,
                                                SHADOW_NEAR_PLANE,
                                                SHADOW_FAR_PLANE);
        glm::vec3 light_dir        = glm::normalize(light.direction);
        glm::vec3 scene_center     = m_camera->getPosition();
        glm::vec3 light_target     = scene_center;
        glm::vec3 light_up         = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(light_dir, light_up)) > 0.9f)
            light_up = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::mat4 light_view =
            glm::lookAt(light_target - light_dir * SHADOW_FRUSTUM_SIZE * 0.5f, light_target, light_up);

        m_light_space_matrix = light_projection * light_view;
        m_shadow_enabled     = true;

        for (auto& ro : m_render_scene->m_render_objects)
        {
            glm::mat4 model           = glm::mat4(1.0f);
            auto      rotation_matrix = glm::toMat4(ro->getOrientation());
            model                     = rotation_matrix * model;
            model                     = glm::translate(model, ro->getPosition());
            model                     = glm::scale(model, ro->getScale());

            m_shadow_shader->setMat4("lightSpaceMatrix", m_light_space_matrix);
            m_shadow_shader->setMat4("model", model);

            ro->draw(*m_shadow_shader);
        }

        glCullFace(GL_BACK); // restore back face culling
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Renderer::renderSkybox()
    {
        // Skybox pass
        m_skybox_shader->use();
        glm::mat4 skybox_model = glm::mat4(1.0f);
        glm::mat4 skybox_view  = glm::mat4(glm::mat3(m_camera->getViewMatrix()));
        glm::mat4 skybox_proj  = m_camera->getProjMatrix();

        m_skybox_shader->setModelViewProjectionMatrices(skybox_model, skybox_view, skybox_proj);
        m_skybox_shader->setInt("skybox", 0);
        m_skybox_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);

        m_ibl_skybox->draw();
    }

    void Renderer::renderBloom()
    {
        // Bloom pass
        glm::vec2 blur_direction_x = glm::vec2(1.0f, 0.0f);
        glm::vec2 blur_direction_y = glm::vec2(0.0f, 1.0f);

        switch (m_bloom_direction)
        {
            case BloomDirection::HORIZONTAL:
                blur_direction_y = blur_direction_x;
                break;
            case BloomDirection::VERTICAL:
                blur_direction_x = blur_direction_y;
                break;
            default:
                break;
        }

        glBindTexture(GL_TEXTURE_2D, m_pbr_framebuffer->getBloomColorTextureId());
        glGenerateMipmap(GL_TEXTURE_2D);

        m_bloom_shader->use();

        static constexpr int BLOOM_MAX_MIP_LEVEL = 5;
        for (auto mip_level = 0; mip_level <= BLOOM_MAX_MIP_LEVEL; mip_level++)
        {
            m_bloom_framebuffers[0]->setMipLevel(mip_level);
            m_bloom_framebuffers[1]->setMipLevel(mip_level);

            // First iteration samples from the main pass bloom buffer
            m_bloom_framebuffers[0]->bind();
            glBindTexture(GL_TEXTURE_2D, m_pbr_framebuffer->getBloomColorTextureId());
            m_bloom_shader->setInt("sampleMipLevel", mip_level);
            m_bloom_shader->setVec2("blurDirection", blur_direction_x);

            m_fullscreen_quad->draw();

            // ping-pong it
            unsigned int bloom_framebuffer = 1;
            unsigned int source_buffer     = 0;
            for (auto i = 1; i < m_bloom_iterations; i++)
            {
                source_buffer = bloom_framebuffer == 1 ? 0 : 1;

                m_bloom_framebuffers[bloom_framebuffer]->bind();
                auto blur_direction = bloom_framebuffer == 1 ? blur_direction_y : blur_direction_x;
                m_bloom_shader->setVec2("blurDirection", blur_direction);
                glBindTexture(GL_TEXTURE_2D, m_bloom_framebuffers[source_buffer]->getColorTextureId());

                m_fullscreen_quad->draw();
                bloom_framebuffer = source_buffer;
            }

            m_bloom_result_id = bloom_framebuffer;
        }
    }

    void Renderer::applyPostprocess()
    {
        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // switch back to default framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_post_shader->use();

        m_post_shader->setBool("bloomEnabled", m_bloom_enabled);
        m_post_shader->setFloat("bloomIntensity", m_bloom_intensity);
        m_post_shader->setBool("tonemappingEnabled", m_tonemapping_enabled);
        m_post_shader->setFloat("gammaCorrectionFactor", m_gamma_correction_factor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_pbr_framebuffer->getColorTextureId());
        m_post_shader->setInt("colorTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_bloom_framebuffers[m_bloom_result_id]->getColorTextureId());
        m_post_shader->setInt("bloomTexture", 1);

        m_fullscreen_quad->draw();
    }
} // namespace RealmEngine
