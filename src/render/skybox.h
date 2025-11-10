#pragma once

#include "render/cube.h"
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    /**
     * A skybox created from either face textures or an existing cubemap texture.
     */
    class Skybox
    {
    public:
        /**
         * Create a skybox by loading 6 cube face textures.
         * @param textureDirectoryPath
         */
        explicit Skybox(std::string textureDirectoryPath);

        /**
         * Create a skybox from an existing cubemap texture.
         * @param textureId
         */
        explicit Skybox(unsigned int textureId);
        void draw();

    private:
        void loadCubemapTextures(std::string textureDirectoryPath);

        std::unique_ptr<Cube> mCube;
        unsigned int          mTextureId;

        std::vector<std::string> mFaceTextureFileNames = {
            "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
    };
} // namespace RealmEngine

