#include "render/hdr_texture.h"

#include "utils.h"
#include <glad/gl.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>

namespace RealmEngine
{
    HDRTexture::HDRTexture(const std::string& path)
    {
        int    width, height, num_channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &num_channels, 0);

        if (!data)
        {
            err("Failed to load HDR texture data: " + path);
            stbi_image_free(data);
            return;
        }

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    unsigned int HDRTexture::getId() const { return m_id; }
} // namespace RealmEngine
