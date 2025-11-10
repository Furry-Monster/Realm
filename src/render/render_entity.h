#pragma once

#include "render/render_model.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace RealmEngine
{
    class RenderEntity
    {
    public:
        explicit RenderEntity(std::shared_ptr<RenderModel> model);
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

        std::shared_ptr<RenderModel> getModel() const;

    private:
        glm::vec3                    m_position {glm::vec3(0.0)};
        glm::vec3                    m_scale {glm::vec3(1.0, 1.0, 1.0)};
        glm::quat                    m_orientation {glm::quat(1.0, 0.0, 0.0, 0.0)};
        std::shared_ptr<RenderModel> m_model;
    };
} // namespace RealmEngine
