#include "render/hdri_cube.h"

#include <glad/gl.h>
#include "render/cube.h"
#include "render/hdr_texture.h"
#include "render/shader.h"

namespace RealmEngine
{
    HDRICube::HDRICube(const std::string& hdri_path) : m_hdr_texture(HDRTexture(hdri_path))
    {
        m_cube = std::make_unique<Cube>();
    }

    void HDRICube::draw(Shader& shader)
    {
        shader.setInt("hdri", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_hdr_texture.getId());
        m_cube->draw();
    }
} // namespace RealmEngine
