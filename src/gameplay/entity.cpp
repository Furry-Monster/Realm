#include "gameplay/entity.h"

namespace RealmEngine
{
    void Entity::setPosition(glm::vec3 position) { m_position = position; }

    glm::vec3 Entity::getPosition() const { return m_position; }

    void Entity::setScale(glm::vec3 scale) { m_scale = scale; }

    glm::vec3 Entity::getScale() const { return m_scale; }

    void Entity::setOrientation(glm::quat orientation) { m_orientation = orientation; }

    glm::quat Entity::getOrientation() const { return m_orientation; }

} // namespace RealmEngine
