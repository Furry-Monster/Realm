#include "render/renderer.h"

#include "config_manager.h"
#include "global_context.h"
#include "utils.h"
#include "window.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace RealmEngine
{
    Renderer::Renderer()
    {
        m_root_path   = g_context.m_config->getRootFolder();
        m_shader_path = g_context.m_config->getShaderFolder();
        m_asset_path  = g_context.m_config->getAssetFolder();

        m_camera = std::make_shared<RenderCamera>();

        setupShaders();
    }

    void Renderer::initialize()
    {
        // NOTE: renderer must be initialized after window
        m_window = g_context.m_window;

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        m_camera->initialize();
        m_camera->setPerspective(
            45.0f, static_cast<float>(m_window->getWidth()) / static_cast<float>(m_window->getHeight()), 0.1f, 100.0f);
        m_camera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        m_camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        m_pbr_framebuffer = std::make_unique<PBRFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_pbr_framebuffer->init();

        setupIBL();

        m_fullscreen_quad       = std::make_unique<FullscreenQuad>();
        m_bloom_framebuffers[0] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[0]->init();
        m_bloom_framebuffers[1] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[1]->init();

        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());

        info("Renderer initialized.");
    }

    void Renderer::disposal()
    {
        m_pbr_shader.reset();
        m_bloom_shader.reset();
        m_post_shader.reset();
        m_skybox_shader.reset();

        m_pbr_framebuffer.reset();
        m_ibl_equirectangular_cubemap.reset();
        m_ibl_diffuse_irradiance_map.reset();
        m_ibl_specular_map.reset();
        m_ibl_skybox.reset();
        m_fullscreen_quad.reset();
        m_bloom_framebuffers[0].reset();
        m_bloom_framebuffers[1].reset();

        m_camera->disposal();
        m_camera.reset();
        m_window.reset();

        info("Renderer shutdown.");
    }

    void Renderer::render(std::shared_ptr<RenderScene> render_scene)
    {
        if (!render_scene)
            return;

        m_scene = render_scene;

        // Main pass
        m_pbr_framebuffer->bind();
        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // update camera first.
        m_camera->update();
        glm::vec3 camera_position = m_camera->getPosition();
        glm::mat4 projection      = m_camera->getProjMatrix();
        glm::mat4 view            = m_camera->getViewMatrix();

        m_pbr_shader->use();

        // Set light data
        std::vector<glm::vec3> light_positions(4, glm::vec3(0.0f));
        std::vector<glm::vec3> light_colors(4, glm::vec3(0.0f));
        for (size_t i = 0; i < std::min(render_scene->m_light_positions.size(), static_cast<size_t>(4)); ++i)
            light_positions[i] = render_scene->m_light_positions[i];
        for (size_t i = 0; i < std::min(render_scene->m_light_colors.size(), static_cast<size_t>(4)); ++i)
            light_colors[i] = render_scene->m_light_colors[i];

        m_pbr_shader->setVec3Array("lightPositions", light_positions);
        m_pbr_shader->setVec3Array("lightColors", light_colors);
        m_pbr_shader->setVec3("cameraPosition", camera_position);

        // IBL stuff
        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        m_pbr_shader->setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ibl_diffuse_irradiance_map->getCubemapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        m_pbr_shader->setInt("prefilteredEnvMap", TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ibl_specular_map->getPrefilteredEnvMapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        m_pbr_shader->setInt("brdfConvolutionMap", TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        glBindTexture(GL_TEXTURE_2D, m_ibl_specular_map->getBrdfConvolutionMapId());

        // post stuff
        m_pbr_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);

        // Render entities
        for (auto& ro : render_scene->m_render_objects)
        {
            glm::mat4 model = glm::mat4(1.0f);

            auto rotation_matrix = glm::toMat4(ro.getOrientation());
            model                = rotation_matrix * model;
            model                = glm::translate(model, ro.getPosition());
            model                = glm::scale(model, ro.getScale());

            m_pbr_shader->setModelViewProjectionMatrices(model, view, projection);

            glDepthMask(GL_TRUE);
            ro.draw(*m_pbr_shader);
        }

        renderSkybox();

        renderBloom();

        applyPostprocess();
    }

    void Renderer::setupShaders()
    {
        std::filesystem::path vertex_path;
        std::filesystem::path fragment_path;

        vertex_path   = m_shader_path / "pbr.vert";
        fragment_path = m_shader_path / "pbr.frag";
        m_pbr_shader  = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path    = m_shader_path / "bloom.vert";
        fragment_path  = m_shader_path / "bloom.frag";
        m_bloom_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path   = m_shader_path / "post.vert";
        fragment_path = m_shader_path / "post.frag";
        m_post_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());

        vertex_path     = m_shader_path / "skybox.vert";
        fragment_path   = m_shader_path / "skybox.frag";
        m_skybox_shader = std::make_unique<Shader>(vertex_path.generic_string(), fragment_path.generic_string());
    }

    void Renderer::setupIBL()
    {
        std::string root_path = m_root_path.generic_string();
        std::string hdri_path = (m_asset_path / "hdr/barcelona_rooftop.hdr").generic_string();

        // Pre-compute IBL stuff
        m_ibl_equirectangular_cubemap = std::make_unique<EquirectangularCubemap>(root_path, hdri_path);
        m_ibl_equirectangular_cubemap->compute();

        m_ibl_diffuse_irradiance_map =
            std::make_unique<DiffuseIrradianceMap>(root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_diffuse_irradiance_map->compute();

        m_ibl_specular_map = std::make_unique<SpecularMap>(root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_specular_map->computePrefilteredEnvMap();
        m_ibl_specular_map->computeBrdfConvolutionMap();

        // Create skybox from the equirectangular cubemap
        m_ibl_skybox = std::make_unique<Skybox>(m_ibl_equirectangular_cubemap->getCubemapId());
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

        for (auto mip_level = 0; mip_level <= 5; mip_level++)
        {
            m_bloom_framebuffers[0]->setMipLevel(mip_level);
            m_bloom_framebuffers[1]->setMipLevel(mip_level);

            // NOTE: first iteration we'll use the bloom buffer from the main render pass
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
