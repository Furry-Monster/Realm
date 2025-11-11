#include "render/skybox.h"

#include <glad/gl.h>
#include <stb/stb_image.h>
#include "render/cube.h"
#include "utils.h"

namespace RealmEngine
{
    Skybox::Skybox(std::string texture_directory_path)
    {
        loadCubemapTextures(texture_directory_path);
        m_cube = std::make_unique<Cube>();
    }

    Skybox::Skybox(unsigned int texture_id) : m_texture_id(texture_id) { m_cube = std::make_unique<Cube>(); }

    void Skybox::draw()
    {
        // NOTE:
        // default depth buffer value is 1.0
        // skybox depth is 1.0 everywhere
        // need equality to make sure skybox passes depth test in default value places
        glDepthFunc(GL_LEQUAL);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_id);
        m_cube->draw();
        glDepthFunc(GL_LESS);
    }

    void Skybox::loadCubemapTextures(std::string texture_directory_path)
    {
        stbi_set_flip_vertically_on_load(false);

        glGenTextures(1, &m_texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_id);

        int width, height, num_channels;

        for (unsigned int i = 0; i < m_face_texture_file_names.size(); i++)
        {
            std::string    path = texture_directory_path + '/' + m_face_texture_file_names[i];
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);

            if (!data)
            {
                warn("Cubemap texture data failed to load for path: " + path);
                continue;
            }

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_set_flip_vertically_on_load(true);
    }
} // namespace RealmEngine
