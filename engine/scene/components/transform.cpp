#include "scene/components/transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    glm::mat4 Transform::getModelMatrix() const
    {
        // M = T * R * S
        const glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
        const glm::mat4 r = glm::mat4_cast(rotation);
        const glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
        return t * r * s;
    }

    glm::vec3 Transform::getEulerAngles() const { return glm::eulerAngles(rotation); }

    glm::vec3 Transform::getForward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }

    glm::vec3 Transform::getRight() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }

    glm::vec3 Transform::getUp() const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

} // namespace RealmEngine
