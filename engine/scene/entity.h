#pragma once

#include <cstddef>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "scene/component.h"

namespace RealmEngine
{
    class Entity
    {
        using ComponentSet = std::unordered_map<size_t, std::shared_ptr<Component>>;

    public:
        explicit Entity(size_t id);
        ~Entity() noexcept = default;

        Entity(const Entity&)                = delete;
        Entity& operator=(const Entity&)     = delete;
        Entity(Entity&&) noexcept            = default;
        Entity& operator=(Entity&&) noexcept = default;

        size_t getId() const { return m_id; }

        void                             addComponent(std::shared_ptr<Component> component);
        std::shared_ptr<Component>       getComponent(size_t type_id);
        std::shared_ptr<const Component> getComponent(size_t type_id) const;
        bool                             hasComponent(size_t type_id) const;
        void                             removeComponent(size_t type_id);

        // Template methods
        template<typename T>
        std::shared_ptr<T> getComponent();

        template<typename T>
        std::shared_ptr<const T> getComponent() const;

        template<typename T>
        bool hasComponent() const;

        template<typename T>
        void removeComponent();

    private:
        size_t       m_id;
        ComponentSet m_components;
    };

    template<typename T>
    std::shared_ptr<T> Entity::getComponent()
    {
        size_t type_id = std::type_index(typeid(T)).hash_code();
        auto   comp    = getComponent(type_id);
        if (comp)
            return std::dynamic_pointer_cast<T>(comp);
        return nullptr;
    }

    template<typename T>
    std::shared_ptr<const T> Entity::getComponent() const
    {
        size_t type_id = std::type_index(typeid(T)).hash_code();
        auto   comp    = getComponent(type_id);
        if (comp)
            return std::dynamic_pointer_cast<const T>(comp);
        return nullptr;
    }

    template<typename T>
    bool Entity::hasComponent() const
    {
        size_t type_id = std::type_index(typeid(T)).hash_code();
        return hasComponent(type_id);
    }

    template<typename T>
    void Entity::removeComponent()
    {
        size_t type_id = std::type_index(typeid(T)).hash_code();
        removeComponent(type_id);
    }

} // namespace RealmEngine
