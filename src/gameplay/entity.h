#pragma once

#include <cstddef>

namespace RealmEngine
{
    class Entity
    {
    public:
        explicit Entity(size_t id);
        ~Entity() noexcept = default;

        Entity(const Entity&)                = default;
        Entity& operator=(const Entity&)     = default;
        Entity(Entity&&) noexcept            = default;
        Entity& operator=(Entity&&) noexcept = default;

        size_t getId() const { return m_id; }

    private:
        size_t m_id;
    };
} // namespace RealmEngine
