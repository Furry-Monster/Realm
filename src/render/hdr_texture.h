#pragma once

#include <string>

namespace RealmEngine
{
    /**
     * A texture loaded from a .hdr file.
     */
    class HDRTexture
    {
    public:
        explicit HDRTexture(const std::string& path);
        unsigned int getId() const;

    private:
        unsigned int mId;
    };
} // namespace RealmEngine

