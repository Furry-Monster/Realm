#pragma once

#include <glm/glm.hpp>
#include "gameplay/component.h"

namespace RealmEngine
{
    class Lighting : public Component
    {
    public:
        Lighting();
        Lighting(const glm::vec3& position, const glm::vec3& color);
        ~Lighting() noexcept override = default;

        Lighting(const Lighting&)                = delete;
        Lighting& operator=(const Lighting&)     = delete;
        Lighting(Lighting&&) noexcept            = default;
        Lighting& operator=(Lighting&&) noexcept = default;

        size_t getTypeId() const override;

        void      setPosition(const glm::vec3& position);
        glm::vec3 getPosition() const;

        void      setColor(const glm::vec3& color);
        glm::vec3 getColor() const;

    private:
        glm::vec3 m_position;
        glm::vec3 m_color;
    };

} // namespace RealmEngine
