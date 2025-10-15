#include "render_pipeline.h"

#include "config_manager.h"
#include "utils.h"
#include <glad/gl.h>

namespace RealmEngine
{
    /// -----------------------
    ///     ForwardPipeline
    /// -----------------------
    void ForwardPipeline::initialize()
    {
        info("<Forward Rendering Pipeline> initializing...");

        // Load shaders
        loadShaders();

        // Setup shadow mapping
        setupShadowMapping();

        info("<Forward Rendering Pipeline> initialized successfully");
    }

    void ForwardPipeline::disposal()
    {
        info("<Forward Rendering Pipeline> disposing...");

        // Clean up shadow mapping resources
        if (m_rhi)
        {
            if (m_shadow_fbo != 0)
                m_rhi->deleteFramebuffer(m_shadow_fbo);
            if (m_shadow_map != 0)
                m_rhi->deleteTexture(m_shadow_map);
        }

        info("<Forward Rendering Pipeline> disposed.");
    }

    void ForwardPipeline::render()
    {
        if (!m_rhi || !m_scene || !m_camera)
        {
            warn("ForwardPipeline: Missing render context");
            return;
        }

        renderShadowMaps();
        renderForward();
        renderPostprocess();
    }

    void ForwardPipeline::setRenderContext(std::shared_ptr<RHI>          rhi,
                                           std::shared_ptr<RenderScene>  scene,
                                           std::shared_ptr<RenderCamera> camera,
                                           std::shared_ptr<RenderResource> resource)
    {
        m_rhi    = rhi;
        m_scene  = scene;
        m_camera = camera;
        m_resource = resource;
    }

    void ForwardPipeline::loadShaders()
    {
        if (!m_rhi)
        {
            err("ForwardPipeline: RHI not available for shader loading");
            return;
        }

        // Get shader directory path
        std::string shader_dir = g_context.m_cfg->getShaderFolder().generic_string();
        info("Shader directory: " + shader_dir);

        // Load PBR shader program
        std::string vert_path = shader_dir + "/pbr.vert";
        std::string frag_path = shader_dir + "/pbr.frag";
        info("Loading vertex shader: " + vert_path);
        info("Loading fragment shader: " + frag_path);
        m_pbr_program = m_rhi->loadShaderProgram("pbr", vert_path, frag_path, "");
        if (m_pbr_program == 0)
        {
            err("Failed to load PBR shader program");
            return;
        }

        // Load shadow mapping shader program
        m_shadow_program = m_rhi->loadShaderProgram("shadow", shader_dir + "/shadow.vert", shader_dir + "/shadow.frag", "");
        if (m_shadow_program == 0)
        {
            err("Failed to load shadow shader program");
            return;
        }

        // Load skybox shader program
        m_skybox_program = m_rhi->loadShaderProgram("skybox", shader_dir + "/skybox.vert", shader_dir + "/skybox.frag", "");
        if (m_skybox_program == 0)
        {
            err("Failed to load skybox shader program");
            return;
        }

        info("All shader programs loaded successfully");
    }

    void ForwardPipeline::setupShadowMapping()
    {
        if (!m_rhi)
            return;

        // Create shadow map texture
        m_shadow_map = m_rhi->createTexture2D(
            SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // Create shadow framebuffer
        m_shadow_fbo = m_rhi->createFramebuffer();
        m_rhi->attachTextureToFramebuffer(m_shadow_fbo, m_shadow_map, GL_DEPTH_ATTACHMENT);

        // Check if framebuffer is complete
        if (!m_rhi->checkFramebufferComplete(m_shadow_fbo))
        {
            err("Shadow framebuffer is not complete");
        }

        info("Shadow mapping setup completed");
    }

    void ForwardPipeline::renderShadowMaps()
    {
        if (!m_rhi || !m_scene)
            return;

        // TODO: Implement shadow map rendering
        // This will be implemented when we have lighting system
    }

    void ForwardPipeline::renderForward()
    {
        if (!m_rhi || !m_scene || !m_camera)
            return;

        // Use PBR shader program
        glUseProgram(m_pbr_program);

        // Setup camera uniforms
        setupCameraUniforms();

        // Setup lighting uniforms
        setupLighting();

        // Render all opaque objects
        const auto& opaque_objects = m_scene->getOpaqueObjects();
        for (const auto& obj : opaque_objects)
        {
            renderObject(obj);
        }

        // Render transparent objects (back to front)
        const auto& transparent_objects = m_scene->getTransparentObjects();
        for (const auto& obj : transparent_objects)
        {
            renderObject(obj);
        }
    }

    void ForwardPipeline::renderPostprocess()
    {
        // TODO: Implement post-processing effects
    }

    void ForwardPipeline::renderObject(const RenderObject& obj)
    {
        if (!m_resource || !obj.isVisible())
            return;
            
        // Get mesh and material from resource manager
        RenderMesh* mesh = m_resource->getRenderMesh(obj.getMesh());
        RenderMaterial* material = m_resource->getRenderMaterial(obj.getMaterial());
        
        if (!mesh || !material)
            return;
            
        // Set model matrix uniform
        glUniformMatrix4fv(glGetUniformLocation(m_pbr_program, "uModel"), 1, GL_FALSE, &obj.getModelMatrix()[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(m_pbr_program, "uNormalMatrix"), 1, GL_FALSE, &obj.getNormalMatrix()[0][0]);
        
        // Bind material (this will bind textures and set material uniforms)
        material->bind();
        
        // Bind mesh and draw
        mesh->bind();
        glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
        mesh->unbind();
        
        // Unbind material
        material->unbind();
    }

    void ForwardPipeline::setupMaterial(const Material& material)
    {
        // TODO: Implement material setup
        // This will bind textures and set material uniforms
    }

    void ForwardPipeline::setupLighting()
    {
        if (!m_scene)
            return;
            
        // Set up directional light (assuming first directional light for now)
        const auto& dir_lights = m_scene->getDirectionalLights();
        if (!dir_lights.empty())
        {
            const auto& light = dir_lights[0];
            glUniform3fv(glGetUniformLocation(m_pbr_program, "uLightDirection"), 1, &light.direction[0]);
            glUniform3fv(glGetUniformLocation(m_pbr_program, "uLightColor"), 1, &light.color[0]);
            glUniform1f(glGetUniformLocation(m_pbr_program, "uLightIntensity"), light.intensity);
        }
        else
        {
            // Default directional light
            glm::vec3 default_dir = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f));
            glm::vec3 default_color = glm::vec3(1.0f, 1.0f, 1.0f);
            float default_intensity = 1.0f;
            
            glUniform3fv(glGetUniformLocation(m_pbr_program, "uLightDirection"), 1, &default_dir[0]);
            glUniform3fv(glGetUniformLocation(m_pbr_program, "uLightColor"), 1, &default_color[0]);
            glUniform1f(glGetUniformLocation(m_pbr_program, "uLightIntensity"), default_intensity);
        }
        
        // Set up ambient light
        const auto& ambient = m_scene->getAmbientLight();
        glUniform3fv(glGetUniformLocation(m_pbr_program, "uAmbientColor"), 1, &ambient.color[0]);
        glUniform1f(glGetUniformLocation(m_pbr_program, "uAmbientIntensity"), ambient.intensity);
        
        // Set up shadow mapping
        glUniform1i(glGetUniformLocation(m_pbr_program, "uUseShadows"), 1);
        glActiveTexture(GL_TEXTURE5); // Shadow map texture unit
        glBindTexture(GL_TEXTURE_2D, m_shadow_map);
        glUniform1i(glGetUniformLocation(m_pbr_program, "uShadowMap"), 5);
        
        // Set up IBL (placeholder for now)
        glUniform1i(glGetUniformLocation(m_pbr_program, "uUseIBL"), 0);
    }

    void ForwardPipeline::setupCameraUniforms()
    {
        if (!m_camera)
            return;

        // Set camera matrices
        glUniformMatrix4fv(glGetUniformLocation(m_pbr_program, "uView"), 1, GL_FALSE, &m_camera->getViewMatrix()[0][0]);
        glUniformMatrix4fv(
            glGetUniformLocation(m_pbr_program, "uProjection"), 1, GL_FALSE, &m_camera->getProjMatrix()[0][0]);

        // Set camera position
        glUniform3fv(glGetUniformLocation(m_pbr_program, "uCameraPos"), 1, &m_camera->getPosition()[0]);
    }

    /// -----------------------
    ///     DefferedPipeline
    /// -----------------------

    void DefferedPipeline::initialize() { info("<Deffered Rendering Pipeline> initialized"); }

    void DefferedPipeline::disposal() { info("<Deffered Rendering Pipeline> disposed."); }

    void DefferedPipeline::render() {}

} // namespace RealmEngine