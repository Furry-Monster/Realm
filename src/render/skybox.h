#pragma once

#include <memory>
#include <string>
#include <vector>
#include "render/cube.h"

namespace RealmEngine
{
    class Skybox
    {
    public:
        /**
         * Create a skybox by loading 6 cube face textures.
         * @param texture_directory_path
         */
        explicit Skybox(std::string texture_directory_path);

        /**
         * Create a skybox from an existing cubemap texture.
         * @param texture_id
         */
        explicit Skybox(unsigned int texture_id);

        void draw();

    private:
        void loadCubemapTextures(std::string texture_directory_path);

        std::unique_ptr<Cube> m_cube;
        unsigned int          m_texture_id;

        std::vector<std::string> m_face_texture_file_names =
            {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
    };
} // namespace RealmEngine
