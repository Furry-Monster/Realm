#include "gameplay/lighting.h"
#include <typeindex>

namespace RealmEngine
{
    Lighting::Lighting() : m_position(glm::vec3(0.0f)), m_color(glm::vec3(1.0f)) {}

    Lighting::Lighting(const glm::vec3& position, const glm::vec3& color) : m_position(position), m_color(color) {}

    size_t Lighting::getTypeId() const { return std::type_index(typeid(Lighting)).hash_code(); }

    void Lighting::setPosition(const glm::vec3& position) { m_position = position; }

    glm::vec3 Lighting::getPosition() const { return m_position; }

    void Lighting::setColor(const glm::vec3& color) { m_color = color; }

    glm::vec3 Lighting::getColor() const { return m_color; }

} // namespace RealmEngine
