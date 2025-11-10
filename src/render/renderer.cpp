#include "render/renderer.h"

#include "config_manager.h"
#include "gameplay/scene.h"
#include "global_context.h"
#include "render/render_camera.h"
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
        mWindow = window;

        // GLAD manages the function pointers for OpenGL
        // Note: GLAD should be initialized after window creation
        // For now, we assume GLAD is already initialized by the window system

        // OpenGL options
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // interpolate between cubemap faces

        // Setup camera
        mCamera = std::make_shared<RenderCamera>();
        mCamera->initialize();
        mCamera->setPerspective(
            45.0f, static_cast<float>(window->getWidth()) / static_cast<float>(window->getHeight()), 0.1f, 100.0f);
        mCamera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        mCamera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

        // Setup shaders
        // Use ConfigManager to get the correct shader path
        mShaderRootPath = g_context.m_cfg->getShaderFolder().generic_string();
        setupShaders();

        // Setup framebuffers
        setupFramebuffers();

        glViewport(0, 0, window->getWidth(), window->getHeight());

        info("Renderer initialized.");
    }

    void Renderer::shutdown()
    {
        mPbrShader.reset();
        mFramebuffer.reset();
        mCamera.reset();
        mWindow.reset();

        info("Renderer shutdown.");
    }

    void Renderer::render(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;

        mScene = scene;

        // Main pass
        mFramebuffer->bind();
        glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Slightly lighter background to see if rendering works
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Ensure depth testing is enabled (should already be enabled in initialize, but ensure it)
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glm::vec3 camera_position = mCamera->getPosition();
        glm::mat4 projection      = mCamera->getProjMatrix();
        glm::mat4 view            = mCamera->getViewMatrix();

        mPbrShader->use();

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

        mPbrShader->setVec3Array("lightPositions", light_positions);
        mPbrShader->setVec3Array("lightColors", light_colors);
        mPbrShader->setVec3("cameraPosition", camera_position);

        // Set IBL uniforms (temporarily disabled - set to default values)
        // TODO: Implement IBL when needed
        mPbrShader->setFloat("bloomBrightnessCutoff", 1.0f);
        
        // Bind dummy textures for IBL uniforms (shader declares them but we don't use them yet)
        // This prevents OpenGL errors from unbound sampler uniforms
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // diffuseIrradianceMap
        mPbrShader->setInt("diffuseIrradianceMap", 5);
        
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // prefilteredEnvMap
        mPbrShader->setInt("prefilteredEnvMap", 6);
        
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, 0); // brdfConvolutionMap
        mPbrShader->setInt("brdfConvolutionMap", 7);
        
        glActiveTexture(GL_TEXTURE0); // Reset to texture unit 0

        // Render entities
        debug("Rendering " + std::to_string(scene->m_entities.size()) + " entities");
        for (auto& entity : scene->m_entities)
        {
            // Build model matrix: T * R * S (scale first, then rotate, then translate)
            glm::mat4 model = glm::mat4(1.0f);
            
            // Scale first
            model = glm::scale(model, entity.getScale());
            
            // Then rotate
            auto rotation_matrix = glm::toMat4(entity.getOrientation());
            model = model * rotation_matrix;
            
            // Finally translate
            model = glm::translate(model, entity.getPosition());

            mPbrShader->setModelViewProjectionMatrices(model, view, projection);

            if (auto model_ptr = entity.getModel())
            {
                model_ptr->draw(*mPbrShader);
            }
            else
            {
                warn("Entity has no model!");
            }
        }

        // Post-processing pass - blit framebuffer to screen
        glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer->getFramebufferId());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // default framebuffer
        glBlitFramebuffer(0,
                          0,
                          mWindow->getWidth(),
                          mWindow->getHeight(),
                          0,
                          0,
                          mWindow->getWidth(),
                          mWindow->getHeight(),
                          GL_COLOR_BUFFER_BIT,
                          GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Renderer::setupShaders()
    {
        std::string vertex_path   = mShaderRootPath + "/pbr.vert";
        std::string fragment_path = mShaderRootPath + "/pbr.frag";

        mPbrShader = std::make_unique<Shader>(vertex_path, fragment_path);
    }

    void Renderer::setupFramebuffers()
    {
        mFramebuffer = std::make_unique<Framebuffer>(mWindow->getWidth(), mWindow->getHeight());
        mFramebuffer->init();
    }
} // namespace RealmEngine
