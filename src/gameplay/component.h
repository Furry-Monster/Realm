#pragma once

namespace RealmEngine
{
    class Component
    {
    public:
        Component()           = default;
        ~Component() noexcept = default;

        Component(Component&&) noexcept            = default;
        Component& operator=(Component&&) noexcept = default;
        Component(const Component&)                = delete;
        Component& operator=(const Component&)     = delete;

    private:
    };

} // namespace RealmEngine