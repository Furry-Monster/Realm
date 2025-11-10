#pragma once

#include <string>

namespace RealmEngine
{
    struct Texture
    {
        unsigned int m_id;
        std::string  m_path; // used to de-dupe textures loaded
    };
} // namespace RealmEngine
