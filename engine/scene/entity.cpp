#include "scene/entity.h"

namespace RealmEngine
{
    Entity::Entity(size_t id) : m_id(id) {}

    void Entity::addComponent(std::shared_ptr<Component> component)
    {
        if (!component)
            return;

        size_t type_id        = component->getTypeId();
        m_components[type_id] = component;
    }

    std::shared_ptr<Component> Entity::getComponent(size_t type_id)
    {
        auto it = m_components.find(type_id);
        return (it != m_components.end()) ? it->second : nullptr;
    }

    std::shared_ptr<const Component> Entity::getComponent(size_t type_id) const
    {
        auto it = m_components.find(type_id);
        return (it != m_components.end()) ? it->second : nullptr;
    }

    bool Entity::hasComponent(size_t type_id) const { return m_components.find(type_id) != m_components.end(); }

    void Entity::removeComponent(size_t type_id) { m_components.erase(type_id); }

} // namespace RealmEngine
