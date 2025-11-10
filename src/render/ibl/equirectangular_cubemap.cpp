#include "render/ibl/equirectangular_cubemap.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render/shader.h"

namespace RealmEngine
{
    EquirectangularCubemap::EquirectangularCubemap(const std::string& engineRoot, const std::string& hdriPath)
    {
        std::string hdri_vertex_shader_path   = engineRoot + "/shaders/ibl/hdricube.vert";
        std::string hdri_fragment_shader_path = engineRoot + "/shaders/ibl/hdricube.frag";

        m_hdri_shader = std::make_unique<Shader>(hdri_vertex_shader_path, hdri_fragment_shader_path);
        m_hdri_cube   = std::make_unique<HDRICube>(hdriPath);
        m_framebuffer = std::make_unique<CubemapFramebuffer>(m_cubemap_width, m_cubemap_height);
    }

    void EquirectangularCubemap::compute()
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 origin(0.0f, 0.0f, 0.0f);
        glm::vec3 unit_x(1.0f, 0.0f, 0.0f);
        glm::vec3 unit_y(0.0f, 1.0f, 0.0f);
        glm::vec3 unit_z(0.0f, 0.0f, 1.0f);

        glm::mat4 camera_angles[] = {glm::lookAt(origin, unit_x, -unit_y),
                                     glm::lookAt(origin, -unit_x, -unit_y),
                                     glm::lookAt(origin, unit_y, unit_z),
                                     glm::lookAt(origin, -unit_y, -unit_z),
                                     glm::lookAt(origin, unit_z, -unit_y),
                                     glm::lookAt(origin, -unit_z, -unit_y)};
        glm::mat4 projection      = glm::perspective(glm::radians(90.0f), // 90 degrees to cover one face
                                                1.0f,                // its a square
                                                0.1f,
                                                2.0f);

        glViewport(0, 0, m_cubemap_width, m_cubemap_height);

        // render the equirectangular HDR texture to a cubemap
        m_framebuffer->bind();
        m_hdri_shader->use();

        // render to each side of the cubemap
        for (auto i = 0; i < 6; i++)
        {
            m_hdri_shader->setModelViewProjectionMatrices(model, camera_angles[i], projection);
            m_framebuffer->setCubeFace(i);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_hdri_cube->draw(*m_hdri_shader);
        }

        m_framebuffer->generateMipmap();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int EquirectangularCubemap::getCubemapId() const { return m_framebuffer->getCubemapTextureId(); }
} // namespace RealmEngine
