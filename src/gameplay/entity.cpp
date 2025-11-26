#include "gameplay/entity.h"

namespace RealmEngine
{
    Entity::Entity(size_t id) : m_id(id) {}

    void Entity::addComponent(std::unique_ptr<Component> component)
    {
        if (!component)
            return;

        size_t type_id = component->getTypeId();
        m_components[type_id] = std::move(component);
    }

    Component* Entity::getComponent(size_t type_id)
    {
        auto it = m_components.find(type_id);
        return (it != m_components.end()) ? it->second.get() : nullptr;
    }

    const Component* Entity::getComponent(size_t type_id) const
    {
        auto it = m_components.find(type_id);
        return (it != m_components.end()) ? it->second.get() : nullptr;
    }

    bool Entity::hasComponent(size_t type_id) const
    {
        return m_components.find(type_id) != m_components.end();
    }

    void Entity::removeComponent(size_t type_id)
    {
        m_components.erase(type_id);
    }

} // namespace RealmEngine
