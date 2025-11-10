#include "render/ibl/diffuse_irradiance_map.h"

#include "render/cube.h"
#include "render/shader.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    DiffuseIrradianceMap::DiffuseIrradianceMap(const std::string& engineRoot, const unsigned int environmentCubemapId)
        : m_environment_cubemap_id(environmentCubemapId)
    {
        std::string diffuseIrradianceVertexShaderPath   = engineRoot + "/shaders/ibl/diffuseirradiance.vert";
        std::string diffuseIrradianceFragmentShaderPath = engineRoot + "/shaders/ibl/diffuseirradiance.frag";

        m_diffuse_irradiance_shader     = std::make_unique<Shader>(diffuseIrradianceVertexShaderPath, diffuseIrradianceFragmentShaderPath);
        m_diffuse_irradiance_framebuffer = std::make_unique<CubemapFramebuffer>(m_diffuse_irradiance_map_width, m_diffuse_irradiance_map_height);
    }

    void DiffuseIrradianceMap::compute()
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
        glViewport(0, 0, m_diffuse_irradiance_map_width, m_diffuse_irradiance_map_height);
        m_diffuse_irradiance_framebuffer->bind();
        m_diffuse_irradiance_shader->use();

        // render to each side of the cubemap
        for (auto i = 0; i < 6; i++)
        {
            m_diffuse_irradiance_shader->setModelViewProjectionMatrices(model, cameraAngles[i], projection);
            m_diffuse_irradiance_framebuffer->setCubeFace(i);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_diffuse_irradiance_shader->setInt("environmentCubemap", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_environment_cubemap_id);
            cube.draw();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int DiffuseIrradianceMap::getCubemapId() const { return m_diffuse_irradiance_framebuffer->getCubemapTextureId(); }
} // namespace RealmEngine

