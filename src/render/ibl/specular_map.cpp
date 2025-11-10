#include "render/ibl/specular_map.h"

#include "render/cube.h"
#include "render/fullscreen_quad.h"
#include "render/shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    SpecularMap::SpecularMap(const std::string& engineRoot, const unsigned int environmentCubemapId)
        : environmentCubemapId(environmentCubemapId)
    {
        // pre-filtered env map
        std::string prefilteredEnvMapVertexShaderPath   = engineRoot + "/shaders/ibl/specularenv.vert";
        std::string prefilteredEnvMapFragmentShaderPath = engineRoot + "/shaders/ibl/specularenv.frag";

        prefilteredEnvMapShader     = std::make_unique<Shader>(prefilteredEnvMapVertexShaderPath, prefilteredEnvMapFragmentShaderPath);
        prefilteredEnvMapFramebuffer = std::make_unique<MipmapCubemapFramebuffer>(prefilteredEnvMapWidth, prefilteredEnvMapHeight);

        // BRDF convolution
        std::string brdfConvolutionVertexShaderPath   = engineRoot + "/shaders/ibl/brdfconvolution.vert";
        std::string brdfConvolutionFragmentShaderPath = engineRoot + "/shaders/ibl/brdfconvolution.frag";

        brdfConvolutionShader     = std::make_unique<Shader>(brdfConvolutionVertexShaderPath, brdfConvolutionFragmentShaderPath);
        brdfConvolutionFramebuffer = std::make_unique<BrdfConvolutionFramebuffer>(brdfConvolutionMapWidth, brdfConvolutionMapHeight);
    }

    void SpecularMap::computePrefilteredEnvMap()
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);
        glm::vec3 unitX(1.0f, 0.0f, 0.0f);
        glm::vec3 unitY(0.0f, 1.0f, 0.0f);
        glm::vec3 unitZ(0.0f, 0.0f, 1.0f);

        glm::mat4 cameraAngles[] = {
            glm::lookAt(origin, unitX, -unitY),  glm::lookAt(origin, -unitX, -unitY),
            glm::lookAt(origin, unitY, unitZ),   glm::lookAt(origin, -unitY, -unitZ),
            glm::lookAt(origin, unitZ, -unitY),  glm::lookAt(origin, -unitZ, -unitY)};
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), // 90 degrees to cover one face
                                                 1.0f,                // its a square
                                                 0.1f,
                                                 2.0f);

        auto cube = Cube();
        prefilteredEnvMapFramebuffer->bind();
        prefilteredEnvMapShader->use();
        prefilteredEnvMapShader->setInt("environmentCubemap", 0);

        for (unsigned int mipLevel = 0; mipLevel < prefilteredEnvMapMipLevels; mipLevel++)
        {
            prefilteredEnvMapFramebuffer->setMipLevel(mipLevel);

            glViewport(0, 0, prefilteredEnvMapFramebuffer->getWidth(), prefilteredEnvMapFramebuffer->getHeight());

            // each mip level has increasing roughness
            float roughness = static_cast<float>(mipLevel) / static_cast<float>(prefilteredEnvMapMipLevels - 1);
            prefilteredEnvMapShader->setFloat("roughness", roughness);

            // render to each side of the cubemap
            for (auto i = 0; i < 6; i++)
            {
                prefilteredEnvMapShader->setModelViewProjectionMatrices(model, cameraAngles[i], projection);
                prefilteredEnvMapFramebuffer->setCubeFace(i);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemapId);
                cube.draw();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int SpecularMap::getPrefilteredEnvMapId() const { return prefilteredEnvMapFramebuffer->getCubemapTextureId(); }

    void SpecularMap::computeBrdfConvolutionMap()
    {
        auto fullscreenQuad = FullscreenQuad();
        brdfConvolutionFramebuffer->bind();
        brdfConvolutionShader->use();

        glViewport(0, 0, brdfConvolutionMapWidth, brdfConvolutionMapHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fullscreenQuad.draw();

        brdfConvolutionMapId = brdfConvolutionFramebuffer->getColorTextureId();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int SpecularMap::getBrdfConvolutionMapId() const { return brdfConvolutionMapId; }
} // namespace RealmEngine

