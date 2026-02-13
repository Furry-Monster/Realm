#pragma once

#include <string>

namespace RealmEngine
{
    // Legacy texture handle -- wraps a native GL texture ID.
    // TODO: Migrate to RHITexture once the texture loading pipeline is refactored.
    struct Texture
    {
        unsigned int m_id {0};
        std::string  m_path; // used to de-dupe textures loaded

        // Bind this texture to the given texture unit (GL_TEXTURE0 + unit).
        void bind(unsigned int unit) const;
    };
} // namespace RealmEngine
