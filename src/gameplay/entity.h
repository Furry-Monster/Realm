#pragma once

namespace RealmEngine
{
    class Entity
    {
    public:
        Entity()           = default;
        ~Entity() noexcept = default;

        Entity(const Entity&)                = default;
        Entity& operator=(const Entity&)     = default;
        Entity(Entity&&) noexcept            = default;
        Entity& operator=(Entity&&) noexcept = default;
    };
} // namespace RealmEngine
