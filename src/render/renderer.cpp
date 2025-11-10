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
        mWindow = window;

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

        // Setup paths
        mEngineRootPath = g_context.m_cfg->getRootFolder().generic_string();
        mShaderRootPath = g_context.m_cfg->getShaderFolder().generic_string();
        mHdriPath       = g_context.m_cfg->getAssetFolder().generic_string() + "/hdr/newport_loft.hdr";

        // Setup shaders
        setupShaders();

        // Setup framebuffers
        setupFramebuffers();

        // Setup IBL
        setupIBL();

        // Setup fullscreen quad
        mFullscreenQuad = std::make_unique<FullscreenQuad>();

        glViewport(0, 0, window->getWidth(), window->getHeight());

        info("Renderer initialized.");
    }

    void Renderer::shutdown()
    {
        mPbrShader.reset();
        mBloomShader.reset();
        mPostShader.reset();
        mSkyboxShader.reset();
        mFramebuffer.reset();
        mBloomFramebuffers[0].reset();
        mBloomFramebuffers[1].reset();
        mIblEquirectangularCubemap.reset();
        mIblDiffuseIrradianceMap.reset();
        mIblSpecularMap.reset();
        mSkybox.reset();
        mFullscreenQuad.reset();
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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE); // Enable depth writing

        // Ensure camera matrices are up to date
        mCamera->update();

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

        // IBL stuff
        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        mPbrShader->setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mIblDiffuseIrradianceMap->getCubemapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        mPbrShader->setInt("prefilteredEnvMap", TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mIblSpecularMap->getPrefilteredEnvMapId());

        glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        mPbrShader->setInt("brdfConvolutionMap", TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        glBindTexture(GL_TEXTURE_2D, mIblSpecularMap->getBrdfConvolutionMapId());

        // post stuff for main shader
        mPbrShader->setFloat("bloomBrightnessCutoff", mBloomBrightnessCutoff);

        // Render entities
        for (auto& entity : scene->m_entities)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // Match reference implementation transformation order
            auto rotation_matrix = glm::toMat4(entity.getOrientation());
            model = rotation_matrix * model;

            model = glm::translate(model, entity.getPosition());
            model = glm::scale(model, entity.getScale());

            mPbrShader->setModelViewProjectionMatrices(model, view, projection);

            if (auto model_ptr = entity.getModel())
            {
                // Ensure depth writing is enabled for models
                glDepthMask(GL_TRUE);
                model_ptr->draw(*mPbrShader);
            }
        }

        // skybox (draw this last to avoid running fragment shader in places where objects are present
        mSkyboxShader->use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 skybox_view = glm::mat4(glm::mat3(view)); // remove translation so skybox is always surrounding camera

        mSkyboxShader->setModelViewProjectionMatrices(model, skybox_view, projection);
        mSkyboxShader->setInt("skybox", 0); // set skybox sampler to slot 0
        mSkyboxShader->setFloat("bloomBrightnessCutoff", mBloomBrightnessCutoff);
        mSkybox->draw();

        renderBloom();

        // Post-processing pass
        glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // switch back to default fb
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mPostShader->use();

        mPostShader->setBool("bloomEnabled", mBloomEnabled);
        mPostShader->setFloat("bloomIntensity", mBloomIntensity);
        mPostShader->setBool("tonemappingEnabled", mTonemappingEnabled);
        mPostShader->setFloat("gammaCorrectionFactor", mGammaCorrectionFactor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mFramebuffer->getColorTextureId());
        mPostShader->setInt("colorTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mBloomFramebuffers[mBloomFramebufferResult]->getColorTextureId());
        mPostShader->setInt("bloomTexture", 1);

        mFullscreenQuad->draw();
    }

    void Renderer::setupShaders()
    {
        std::string vertex_path   = mShaderRootPath + "/pbr.vert";
        std::string fragment_path = mShaderRootPath + "/pbr.frag";
        mPbrShader                = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path   = mShaderRootPath + "/bloom.vert";
        fragment_path = mShaderRootPath + "/bloom.frag";
        mBloomShader  = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path   = mShaderRootPath + "/post.vert";
        fragment_path = mShaderRootPath + "/post.frag";
        mPostShader   = std::make_unique<Shader>(vertex_path, fragment_path);

        vertex_path    = mShaderRootPath + "/skybox.vert";
        fragment_path  = mShaderRootPath + "/skybox.frag";
        mSkyboxShader  = std::make_unique<Shader>(vertex_path, fragment_path);
    }

    void Renderer::setupFramebuffers()
    {
        mFramebuffer = std::make_unique<Framebuffer>(mWindow->getWidth(), mWindow->getHeight());
        mFramebuffer->init();

        mBloomFramebuffers[0] = std::make_unique<BloomFramebuffer>(mWindow->getWidth(), mWindow->getHeight());
        mBloomFramebuffers[0]->init();
        mBloomFramebuffers[1] = std::make_unique<BloomFramebuffer>(mWindow->getWidth(), mWindow->getHeight());
        mBloomFramebuffers[1]->init();
    }

    void Renderer::setupIBL()
    {
        // Pre-compute IBL stuff
        mIblEquirectangularCubemap = std::make_unique<EquirectangularCubemap>(mEngineRootPath, mHdriPath);
        mIblEquirectangularCubemap->compute();

        mIblDiffuseIrradianceMap =
            std::make_unique<DiffuseIrradianceMap>(mEngineRootPath, mIblEquirectangularCubemap->getCubemapId());
        mIblDiffuseIrradianceMap->compute();

        mIblSpecularMap = std::make_unique<SpecularMap>(mEngineRootPath, mIblEquirectangularCubemap->getCubemapId());
        mIblSpecularMap->computePrefilteredEnvMap();
        mIblSpecularMap->computeBrdfConvolutionMap();

        // Create skybox from the equirectangular cubemap
        mSkybox = std::make_unique<Skybox>(mIblEquirectangularCubemap->getCubemapId());
    }

    void Renderer::renderBloom()
    {
        // Bloom pass
        glm::vec2 blur_direction_x = glm::vec2(1.0f, 0.0f);
        glm::vec2 blur_direction_y = glm::vec2(0.0f, 1.0f);

        switch (mBloomDirection)
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

        glBindTexture(GL_TEXTURE_2D, mFramebuffer->getBloomColorTextureId());
        glGenerateMipmap(GL_TEXTURE_2D);

        mBloomShader->use();

        for (auto mip_level = 0; mip_level <= 5; mip_level++)
        {
            mBloomFramebuffers[0]->setMipLevel(mip_level);
            mBloomFramebuffers[1]->setMipLevel(mip_level);

            // first iteration we use the bloom buffer from the main render pass
            mBloomFramebuffers[0]->bind();
            glBindTexture(GL_TEXTURE_2D, mFramebuffer->getBloomColorTextureId());
            mBloomShader->setInt("sampleMipLevel", mip_level);
            mBloomShader->setVec2("blurDirection", blur_direction_x);

            mFullscreenQuad->draw();

            unsigned int bloom_framebuffer = 1; // which buffer to use

            for (auto i = 1; i < mBloomIterations; i++)
            {
                unsigned int source_buffer = bloom_framebuffer == 1 ? 0 : 1;
                mBloomFramebuffers[bloom_framebuffer]->bind();
                auto blur_direction = bloom_framebuffer == 1 ? blur_direction_y : blur_direction_x;
                mBloomShader->setVec2("blurDirection", blur_direction);
                glBindTexture(GL_TEXTURE_2D, mBloomFramebuffers[source_buffer]->getColorTextureId());
                mFullscreenQuad->draw();
                bloom_framebuffer = source_buffer;
            }

            mBloomFramebufferResult = bloom_framebuffer;
        }
    }
} // namespace RealmEngine
