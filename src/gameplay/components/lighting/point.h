#pragma once

#include "gameplay/components/lighting/light_component.h"

namespace RealmEngine
{
    class Point : public LightComponent
    {
    public:
        Point();
        ~Point() noexcept override = default;

        size_t getTypeId() const override;

        void setConstantAttenuation(float constant);
        void setLinearAttenuation(float linear);
        void setQuadraticAttenuation(float quadratic);
        void setRange(float range);

        float getConstantAttenuation() const { return m_constant; }
        float getLinearAttenuation() const { return m_linear; }
        float getQuadraticAttenuation() const { return m_quadratic; }
        float getRange() const { return m_range; }

    private:
        float m_constant;
        float m_linear;
        float m_quadratic;
        float m_range;
    };

} // namespace RealmEngine
