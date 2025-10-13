#pragma once

namespace RealmEngine
{
    class Entity
    {
    public:
        Entity()           = default;
        ~Entity() noexcept = default;

        Entity(Entity&&) noexcept            = default;
        Entity& operator=(Entity&&) noexcept = default;
        Entity(const Entity&)                = delete;
        Entity& operator=(const Entity&)     = delete;

    private:
    };

} // namespace RealmEngine
