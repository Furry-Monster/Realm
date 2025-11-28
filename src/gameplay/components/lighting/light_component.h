#pragma once

#include <glm/glm.hpp>
#include "gameplay/component.h"
#include "render/light.h"

namespace RealmEngine
{
    class LightComponent : public Component
    {
    public:
        explicit LightComponent(LightType type);
        virtual ~LightComponent() noexcept = default;

        LightComponent(const LightComponent&)                = delete;
        LightComponent& operator=(const LightComponent&)     = delete;
        LightComponent(LightComponent&&) noexcept            = default;
        LightComponent& operator=(LightComponent&&) noexcept = default;

        size_t getTypeId() const override = 0;

        LightType getLightType() const { return m_type; }

        void      setColor(const glm::vec3& color);
        glm::vec3 getColor() const { return m_color; }

        void  setIntensity(float intensity);
        float getIntensity() const { return m_intensity; }

        void setEnabled(bool enabled);
        bool isEnabled() const { return m_enabled; }

    protected:
        LightType m_type;
        glm::vec3 m_color;
        float     m_intensity;
        bool      m_enabled;
    };

} // namespace RealmEngine
