#include "render/renderer.h"

#include "config_manager.h"
#include "gameplay/scene.h"
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
    void Renderer::initialize(std::shared_ptr<Window> window)
    {
        m_window = window;

        // OpenGL options
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // interpolate between cubemap faces

        // Setup camera
        m_camera = std::make_shared<RenderCamera>();
        m_camera->initialize();
        m_camera->setPerspective(
            45.0f, static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight()), 0.1f, 100.0f);
        m_camera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        m_camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        // Setup paths
        m_engine_root_path = g_context.m_cfg->getRootFolder().generic_string();
        m_shader_root_path = g_context.m_cfg->getShaderFolder().generic_string();
        m_hdri_path        = g_context.m_cfg->getAssetFolder().generic_string() + "/hdr/newport_loft.hdr";

        // Setup shaders
        setupShaders();

        // Setup framebuffers
        setupFramebuffers();

        // Setup IBL
        setupIBL();

        // Setup fullscreen quad
        m_fullscreen_quad = std::make_unique<FullscreenQuad>();

        glViewport(0, 0, window->getWidth(), window->getHeight());

        info("Renderer initialized.");
    }

    void Renderer::disposal()
    {
        m_pbr_shader.reset();
        m_bloom_shader.reset();
        m_post_shader.reset();
        m_skybox_shader.reset();
        m_framebuffer.reset();
        m_bloom_framebuffers[0].reset();
        m_bloom_framebuffers[1].reset();
        m_ibl_equirectangular_cubemap.reset();
        m_ibl_diffuse_irradiance_map.reset();
        m_ibl_specular_map.reset();
        m_skybox.reset();
        m_fullscreen_quad.reset();
        m_camera.reset();
        m_window.reset();

        info("Renderer shutdown.");
    }

    void Renderer::render(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;

        m_scene = scene;

        // Main pass
        m_framebuffer->bind();
        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE); // Enable depth writing

        // Ensure camera matrices are up to date
        m_camera->update();

        glm::vec3 camera_position = m_camera->getPosition();
        glm::mat4 projection      = m_camera->getProjMatrix();
        glm::mat4 view            = m_camera->getViewMatrix();

        m_pbr_shader->use();

        // Set light data (pad to 4 lights if needed)
        std::vector<glm::vec3> light_positions(4, glm::vec3(0.0f));
        std::vector<glm::vec3> light_colors(4, glm::vec3(0.0f));
        for (size_t i = 0; i < std::min(scene->m_light_positions.size(), static_cast<size_t>(4)); ++i)
        {
            light_positions[i] = scene->m_light_positions[i];
        }
        for (size_t i = 0; i < std::min(scene->m_light_colors.size(), static_cast<size_t>(4)); ++i)
        {
            light_colors[i] = scene->m_light_colors[i];
        }

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

        // post stuff for main shader
        m_pbr_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);

        // Render entities
        for (auto& entity : scene->m_entities)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // Match reference implementation transformation order
            auto rotation_matrix = glm::toMat4(entity.getOrientation());
            model                = rotation_matrix * model;

            model = glm::translate(model, entity.getPosition());
            model = glm::scale(model, entity.getScale());

            m_pbr_shader->setModelViewProjectionMatrices(model, view, projection);

            if (auto model_ptr = entity.getModel())
            {
                // Ensure depth writing is enabled for models
                glDepthMask(GL_TRUE);
                model_ptr->draw(*m_pbr_shader);
            }
        }

        // skybox (draw this last to avoid running fragment shader in places where objects are present
        m_skybox_shader->use();
        glm::mat4 model       = glm::mat4(1.0f);
        glm::mat4 skybox_view = glm::mat4(glm::mat3(view)); // remove translation so skybox is always surrounding camera

        m_skybox_shader->setModelViewProjectionMatrices(model, skybox_view, projection);
        m_skybox_shader->setInt("skybox", 0); // set skybox sampler to slot 0
        m_skybox_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);
        m_skybox->draw();

        renderBloom();

        // Post-processing pass
        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // switch back to default fb
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_post_shader->use();

        m_post_shader->setBool("bloomEnabled", m_bloom_enabled);
        m_post_shader->setFloat("bloomIntensity", m_bloom_intensity);
        m_post_shader->setBool("tonemappingEnabled", m_tonemapping_enabled);
        m_post_shader->setFloat("gammaCorrectionFactor", m_gamma_correction_factor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer->getColorTextureId());
        m_post_shader->setInt("colorTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_bloom_framebuffers[m_bloom_framebuffer_result]->getColorTextureId());
        m_post_shader->setInt("bloomTexture", 1);

        m_fullscreen_quad->draw();
    }

    void Renderer::setupShaders()
    {
        std::string vertex_path   = m_shader_root_path + "/pbr.vert";
        std::string fragment_path = m_shader_root_path + "/pbr.frag";
        m_pbr_shader              = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path    = m_shader_root_path + "/bloom.vert";
        fragment_path  = m_shader_root_path + "/bloom.frag";
        m_bloom_shader = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path   = m_shader_root_path + "/post.vert";
        fragment_path = m_shader_root_path + "/post.frag";
        m_post_shader = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path     = m_shader_root_path + "/skybox.vert";
        fragment_path   = m_shader_root_path + "/skybox.frag";
        m_skybox_shader = std::make_unique<Shader>(vertex_path, fragment_path);
    }

    void Renderer::setupFramebuffers()
    {
        m_framebuffer = std::make_unique<Framebuffer>(m_window->getWidth(), m_window->getHeight());
        m_framebuffer->init();

        m_bloom_framebuffers[0] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[0]->init();
        m_bloom_framebuffers[1] = std::make_unique<BloomFramebuffer>(m_window->getWidth(), m_window->getHeight());
        m_bloom_framebuffers[1]->init();
    }

    void Renderer::setupIBL()
    {
        // Pre-compute IBL stuff
        m_ibl_equirectangular_cubemap = std::make_unique<EquirectangularCubemap>(m_engine_root_path, m_hdri_path);
        m_ibl_equirectangular_cubemap->compute();

        m_ibl_diffuse_irradiance_map =
            std::make_unique<DiffuseIrradianceMap>(m_engine_root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_diffuse_irradiance_map->compute();

        m_ibl_specular_map =
            std::make_unique<SpecularMap>(m_engine_root_path, m_ibl_equirectangular_cubemap->getCubemapId());
        m_ibl_specular_map->computePrefilteredEnvMap();
        m_ibl_specular_map->computeBrdfConvolutionMap();

        // Create skybox from the equirectangular cubemap
        m_skybox = std::make_unique<Skybox>(m_ibl_equirectangular_cubemap->getCubemapId());
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

        glBindTexture(GL_TEXTURE_2D, m_framebuffer->getBloomColorTextureId());
        glGenerateMipmap(GL_TEXTURE_2D);

        m_bloom_shader->use();

        for (auto mip_level = 0; mip_level <= 5; mip_level++)
        {
            m_bloom_framebuffers[0]->setMipLevel(mip_level);
            m_bloom_framebuffers[1]->setMipLevel(mip_level);

            // first iteration we use the bloom buffer from the main render pass
            m_bloom_framebuffers[0]->bind();
            glBindTexture(GL_TEXTURE_2D, m_framebuffer->getBloomColorTextureId());
            m_bloom_shader->setInt("sampleMipLevel", mip_level);
            m_bloom_shader->setVec2("blurDirection", blur_direction_x);

            m_fullscreen_quad->draw();

            unsigned int bloom_framebuffer = 1; // which buffer to use

            for (auto i = 1; i < m_bloom_iterations; i++)
            {
                unsigned int source_buffer = bloom_framebuffer == 1 ? 0 : 1;
                m_bloom_framebuffers[bloom_framebuffer]->bind();
                auto blur_direction = bloom_framebuffer == 1 ? blur_direction_y : blur_direction_x;
                m_bloom_shader->setVec2("blurDirection", blur_direction);
                glBindTexture(GL_TEXTURE_2D, m_bloom_framebuffers[source_buffer]->getColorTextureId());
                m_fullscreen_quad->draw();
                bloom_framebuffer = source_buffer;
            }

            m_bloom_framebuffer_result = bloom_framebuffer;
        }
    }
} // namespace RealmEngine
