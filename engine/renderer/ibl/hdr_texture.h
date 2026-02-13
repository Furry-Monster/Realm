#pragma once

#include <string>

namespace RealmEngine
{
    class HDRTexture
    {
    public:
        explicit HDRTexture(const std::string& path);
        unsigned int getId() const;

    private:
        unsigned int m_id;
    };
} // namespace RealmEngine
