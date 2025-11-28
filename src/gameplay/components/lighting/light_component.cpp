#include "gameplay/components/lighting/light_component.h"

namespace RealmEngine
{
    LightComponent::LightComponent(LightType type) :
        m_type(type), m_color(glm::vec3(1.0f)), m_intensity(1.0f), m_enabled(true)
    {}

    void LightComponent::setColor(const glm::vec3& color) { m_color = color; }

    void LightComponent::setIntensity(float intensity) { m_intensity = intensity; }

    void LightComponent::setEnabled(bool enabled) { m_enabled = enabled; }

} // namespace RealmEngine
