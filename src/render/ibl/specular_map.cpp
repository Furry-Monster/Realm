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
        : m_environment_cubemap_id(environmentCubemapId)
    {
        // pre-filtered env map
        std::string prefilteredEnvMapVertexShaderPath   = engineRoot + "/shaders/ibl/specularenv.vert";
        std::string prefilteredEnvMapFragmentShaderPath = engineRoot + "/shaders/ibl/specularenv.frag";

        m_prefiltered_env_map_shader     = std::make_unique<Shader>(prefilteredEnvMapVertexShaderPath, prefilteredEnvMapFragmentShaderPath);
        m_prefiltered_env_map_framebuffer = std::make_unique<MipmapCubemapFramebuffer>(m_prefiltered_env_map_width, m_prefiltered_env_map_height);

        // BRDF convolution
        std::string brdfConvolutionVertexShaderPath   = engineRoot + "/shaders/ibl/brdfconvolution.vert";
        std::string brdfConvolutionFragmentShaderPath = engineRoot + "/shaders/ibl/brdfconvolution.frag";

        m_brdf_convolution_shader     = std::make_unique<Shader>(brdfConvolutionVertexShaderPath, brdfConvolutionFragmentShaderPath);
        m_brdf_convolution_framebuffer = std::make_unique<BrdfConvolutionFramebuffer>(m_brdf_convolution_map_width, m_brdf_convolution_map_height);
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
        m_prefiltered_env_map_framebuffer->bind();
        m_prefiltered_env_map_shader->use();
        m_prefiltered_env_map_shader->setInt("environmentCubemap", 0);

        for (unsigned int mipLevel = 0; mipLevel < m_prefiltered_env_map_mip_levels; mipLevel++)
        {
            m_prefiltered_env_map_framebuffer->setMipLevel(mipLevel);

            glViewport(0, 0, m_prefiltered_env_map_framebuffer->getWidth(), m_prefiltered_env_map_framebuffer->getHeight());

            // each mip level has increasing roughness
            float roughness = static_cast<float>(mipLevel) / static_cast<float>(m_prefiltered_env_map_mip_levels - 1);
            m_prefiltered_env_map_shader->setFloat("roughness", roughness);

            // render to each side of the cubemap
            for (auto i = 0; i < 6; i++)
            {
                m_prefiltered_env_map_shader->setModelViewProjectionMatrices(model, cameraAngles[i], projection);
                m_prefiltered_env_map_framebuffer->setCubeFace(i);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, m_environment_cubemap_id);
                cube.draw();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int SpecularMap::getPrefilteredEnvMapId() const { return m_prefiltered_env_map_framebuffer->getCubemapTextureId(); }

    void SpecularMap::computeBrdfConvolutionMap()
    {
        auto fullscreenQuad = FullscreenQuad();
        m_brdf_convolution_framebuffer->bind();
        m_brdf_convolution_shader->use();

        glViewport(0, 0, m_brdf_convolution_map_width, m_brdf_convolution_map_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fullscreenQuad.draw();

        m_brdf_convolution_map_id = m_brdf_convolution_framebuffer->getColorTextureId();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int SpecularMap::getBrdfConvolutionMapId() const { return m_brdf_convolution_map_id; }
} // namespace RealmEngine

