#pragma once

#include "gameplay/component.h"

namespace RealmEngine
{
    class Transform : public Component
    {
    public:
        Transform()  = default;
        ~Transform() = default;

        Transform(Transform&&) noexcept            = default;
        Transform& operator=(Transform&&) noexcept = default;
        Transform(const Transform&)                = delete;
        Transform& operator=(const Transform&)     = delete;

    private:
    };

} // namespace RealmEngine