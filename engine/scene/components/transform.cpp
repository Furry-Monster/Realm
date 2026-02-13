#include "scene/components/transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace RealmEngine
{
    glm::mat4 Transform::getModelMatrix() const
    {
        // M = T * R * S
        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::scale(model, scale);
        model           = glm::mat4_cast(rotation) * model;
        model           = glm::translate(model, position);
        return model;
    }

    glm::vec3 Transform::getEulerAngles() const { return glm::eulerAngles(rotation); }

    glm::vec3 Transform::getForward() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }

    glm::vec3 Transform::getRight() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }

    glm::vec3 Transform::getUp() const { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

} // namespace RealmEngine
