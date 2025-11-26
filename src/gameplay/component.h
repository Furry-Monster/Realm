#pragma once

#include <cstddef>

namespace RealmEngine
{
    class Component
    {
    public:
        Component()                   = default;
        virtual ~Component() noexcept = default;

        Component(const Component&)                = delete;
        Component& operator=(const Component&)     = delete;
        Component(Component&&) noexcept            = default;
        Component& operator=(Component&&) noexcept = default;

        virtual size_t getTypeId() const = 0;
    };

} // namespace RealmEngine
