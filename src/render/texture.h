#pragma once

#include <string>

namespace RealmEngine
{
    /**
     * A texture that has been loaded in video memory.
     */
    struct Texture
    {
        unsigned int m_id;
        std::string  m_path; // used to de-dupe textures loaded
    };
} // namespace RealmEngine
