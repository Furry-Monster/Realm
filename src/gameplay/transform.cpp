#include "gameplay/transform.h"
#include <typeindex>
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtc/quaternion.hpp"

namespace RealmEngine
{
    Transform::Transform() { reset(); }

    size_t Transform::getTypeId() const { return std::type_index(typeid(Transform)).hash_code(); }

    // Position
    void Transform::setPosition(const glm::vec3& position) { m_position = position; }

    void Transform::setPosition(float x, float y, float z) { m_position = glm::vec3(x, y, z); }

    glm::vec3 Transform::getPosition() const { return m_position; }

    void Transform::translate(const glm::vec3& translation) { m_position += translation; }

    void Transform::translate(float x, float y, float z) { m_position += glm::vec3(x, y, z); }

    // Rotation
    void Transform::setRotation(const glm::quat& rotation) { m_rotation = glm::normalize(rotation); }

    void Transform::setRotation(const glm::vec3& euler_angles) { m_rotation = glm::quat(euler_angles); }

    void Transform::setRotation(float x, float y, float z) { m_rotation = glm::quat(glm::vec3(x, y, z)); }

    glm::quat Transform::getRotation() const { return m_rotation; }

    glm::vec3 Transform::getEulerAngles() const { return glm::eulerAngles(m_rotation); }

    void Transform::rotate(const glm::quat& rotation) { m_rotation = glm::normalize(rotation * m_rotation); }

    void Transform::rotate(const glm::vec3& axis, float angle)
    {
        glm::quat rotation = glm::angleAxis(angle, glm::normalize(axis));
        m_rotation         = glm::normalize(rotation * m_rotation);
    }

    void Transform::rotate(float x, float y, float z)
    {
        glm::quat rotation = glm::quat(glm::vec3(x, y, z));
        m_rotation         = glm::normalize(rotation * m_rotation);
    }

    // Scale
    void Transform::setScale(const glm::vec3& scale) { m_scale = scale; }

    void Transform::setScale(float uniform_scale) { m_scale = glm::vec3(uniform_scale); }

    void Transform::setScale(float x, float y, float z) { m_scale = glm::vec3(x, y, z); }

    glm::vec3 Transform::getScale() const { return m_scale; }

    void Transform::scale(const glm::vec3& scale) { m_scale *= scale; }

    void Transform::scale(float uniform_scale) { m_scale *= uniform_scale; }

    // Misc
    glm::mat4 Transform::getModelMatrix() const
    {
        // Standard model matrix order: M = T * R * S
        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::scale(model, m_scale);
        model           = glm::mat4_cast(m_rotation) * model;
        model           = glm::translate(model, m_position);

        return model;
    }

    glm::vec3 Transform::getForward() const { return m_rotation * glm::vec3(0.0f, 0.0f, -1.0f); }

    glm::vec3 Transform::getRight() const { return m_rotation * glm::vec3(1.0f, 0.0f, 0.0f); }

    glm::vec3 Transform::getUp() const { return m_rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

    void Transform::reset()
    {
        m_position = glm::vec3(0.0f);
        m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        m_scale    = glm::vec3(1.0f);
    }

} // namespace RealmEngine
