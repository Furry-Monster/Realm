#pragma once

#include <entt/entity/registry.hpp>

namespace RealmEngine
{
    class Entity
    {
        // NOTE:
        // Lightweight handle wrapping entt::entity + registry pointer.
        // Provides ergonomic component access: entity.get<Transform>(), entity.emplace<T>(), etc.
        // Not intended for long-term storage (registry pointer may become stale).
    public:
        Entity() = default;
        Entity(const entt::entity handle, entt::registry* registry) : m_handle(handle), m_registry(registry) {}

        template<typename T, typename... Args>
        T& emplace(Args&&... args)
        {
            return m_registry->emplace<T>(m_handle, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        T& emplaceOrReplace(Args&&... args)
        {
            return m_registry->emplace_or_replace<T>(m_handle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& get()
        {
            return m_registry->get<T>(m_handle);
        }

        template<typename T>
        const T& get() const
        {
            return m_registry->get<T>(m_handle);
        }

        template<typename T>
        T* tryGet()
        {
            return m_registry->try_get<T>(m_handle);
        }

        template<typename T>
        const T* tryGet() const
        {
            return m_registry->try_get<T>(m_handle);
        }

        template<typename T>
        bool has() const
        {
            return m_registry->all_of<T>(m_handle);
        }

        template<typename T>
        void remove()
        {
            m_registry->remove<T>(m_handle);
        }

        entt::entity handle() const { return m_handle; }
        bool         valid() const { return m_registry && m_registry->valid(m_handle); }

        explicit operator entt::entity() const { return m_handle; }
        explicit operator bool() const { return valid(); }

        bool operator==(const Entity& other) const { return m_handle == other.m_handle; }
        bool operator!=(const Entity& other) const { return m_handle != other.m_handle; }

    private:
        entt::entity    m_handle {entt::null};
        entt::registry* m_registry {nullptr};
    };

} // namespace RealmEngine
