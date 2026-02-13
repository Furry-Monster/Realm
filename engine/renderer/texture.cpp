#include "renderer/texture.h"

#include <glad/gl.h>

namespace RealmEngine
{
    void Texture::bind(unsigned int unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

} // namespace RealmEngine
