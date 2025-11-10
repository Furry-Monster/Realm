#include "render/skybox.h"

#include "render/cube.h"
#include "utils.h"
#include <glad/gl.h>
// STB_IMAGE_IMPLEMENTATION already defined in hdr_texture.cpp
#include <stb/stb_image.h>

namespace RealmEngine
{
    Skybox::Skybox(std::string textureDirectoryPath)
    {
        loadCubemapTextures(textureDirectoryPath);
        mCube = std::make_unique<Cube>();
    }

    Skybox::Skybox(unsigned int textureId) : mTextureId(textureId) { mCube = std::make_unique<Cube>(); }

    void Skybox::draw()
    {
        // default depth buffer value is 1.0
        // skybox depth is 1.0 everywhere
        // need equality to make sure skybox passes depth test in default value places
        glDepthFunc(GL_LEQUAL);

        // draw mesh
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureId);
        mCube->draw();
        glDepthFunc(GL_LESS); // go back to default depth comparison
    }

    void Skybox::loadCubemapTextures(std::string textureDirectoryPath)
    {
        stbi_set_flip_vertically_on_load(false); // cubemaps in OpenGL are weird and don't require flip

        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureId);

        int width, height, num_channels;

        for (unsigned int i = 0; i < mFaceTextureFileNames.size(); i++)
        {
            std::string path = textureDirectoryPath + '/' + mFaceTextureFileNames[i];
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);

            if (!data)
            {
                warn("Cubemap texture data failed to load for path: " + path);
                continue;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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

