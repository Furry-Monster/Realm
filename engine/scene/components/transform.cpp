#include "scene/components/transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    glm::mat4 Transform::getModelMatrix() const
    {
        // M = T * R * S
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::mat4_cast(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        return T * R * S;
    }

    glm::vec3 Transform::getEulerAngles() const { return glm::eulerAngles(rotation); }

    glm::vec3 Transform::getForward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }

    glm::vec3 Transform::getRight() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }

    glm::vec3 Transform::getUp() const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

} // namespace RealmEngine
