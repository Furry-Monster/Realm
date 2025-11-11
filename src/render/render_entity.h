#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include "render/render_object.h"

namespace RealmEngine
{
    class RenderEntity
    {
    public:
        explicit RenderEntity(std::shared_ptr<RenderObject> object);
        ~RenderEntity() noexcept = default;

        RenderEntity(const RenderEntity&)                = default;
        RenderEntity& operator=(const RenderEntity&)     = default;
        RenderEntity(RenderEntity&&) noexcept            = default;
        RenderEntity& operator=(RenderEntity&&) noexcept = default;

        void      setPosition(glm::vec3 position);
        glm::vec3 getPosition() const;

        void      setScale(glm::vec3 scale);
        glm::vec3 getScale() const;

        void      setOrientation(glm::quat orientation);
        glm::quat getOrientation() const;

        std::shared_ptr<RenderObject> getObject() const;

    private:
        glm::vec3                     m_position {glm::vec3(0.0)};
        glm::vec3                     m_scale {glm::vec3(1.0, 1.0, 1.0)};
        glm::quat                     m_orientation {glm::quat(1.0, 0.0, 0.0, 0.0)};
        std::shared_ptr<RenderObject> m_render_object;
    };
} // namespace RealmEngine
