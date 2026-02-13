#pragma once

#include <glm/glm.hpp>

namespace RealmEngine
{
    /**
     * @brief
     * Computed each frame by TransformSystem.
     * Represents the final world-space model matrix after hierarchy propagation.
     */
    struct WorldTransform
    {
        glm::mat4 matrix {1.0f};
    };

} // namespace RealmEngine
