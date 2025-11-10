#include "render/ibl/equirectangular_cubemap.h"

#include "render/shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    EquirectangularCubemap::EquirectangularCubemap(const std::string& engineRoot, const std::string& hdriPath)
    {
        std::string hdriVertexShaderPath   = engineRoot + "/shaders/ibl/hdricube.vert";
        std::string hdriFragmentShaderPath = engineRoot + "/shaders/ibl/hdricube.frag";

        hdriShader   = std::make_unique<Shader>(hdriVertexShaderPath, hdriFragmentShaderPath);
        hdriCube     = std::make_unique<HDRICube>(hdriPath);
        framebuffer  = std::make_unique<CubemapFramebuffer>(cubemapWidth, cubemapHeight);
    }

    void EquirectangularCubemap::compute()
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

        glViewport(0, 0, cubemapWidth, cubemapHeight);

        // render the equirectangular HDR texture to a cubemap
        framebuffer->bind();
        hdriShader->use();

        // render to each side of the cubemap
        for (auto i = 0; i < 6; i++)
        {
            hdriShader->setModelViewProjectionMatrices(model, cameraAngles[i], projection);
            framebuffer->setCubeFace(i);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            hdriCube->draw(*hdriShader);
        }

        framebuffer->generateMipmap();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int EquirectangularCubemap::getCubemapId() const { return framebuffer->getCubemapTextureId(); }
} // namespace RealmEngine

