#include "render/hdri_cube.h"

#include "render/cube.h"
#include "render/hdr_texture.h"
#include "render/shader.h"
#include <glad/gl.h>

namespace RealmEngine
{
    HDRICube::HDRICube(const std::string& hdriPath) : m_hdr_texture(HDRTexture(hdriPath))
    {
        m_cube = std::make_unique<Cube>();
    }

    void HDRICube::draw(Shader& shader)
    {
        shader.setInt("hdri", 0);

        // draw mesh
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_hdr_texture.getId());
        m_cube->draw();
    }
} // namespace RealmEngine

