#include "render/render_entity.h"

namespace RealmEngine
{
    RenderEntity::RenderEntity(std::shared_ptr<RenderObject> object) : m_render_object(object) {}

    void RenderEntity::setPosition(glm::vec3 position) { m_position = position; }

    glm::vec3 RenderEntity::getPosition() const { return m_position; }

    void RenderEntity::setScale(glm::vec3 scale) { m_scale = scale; }

    glm::vec3 RenderEntity::getScale() const { return m_scale; }

    void RenderEntity::setOrientation(glm::quat orientation) { m_orientation = orientation; }

    glm::quat RenderEntity::getOrientation() const { return m_orientation; }

    std::shared_ptr<RenderObject> RenderEntity::getObject() const { return m_render_object; }
} // namespace RealmEngine
