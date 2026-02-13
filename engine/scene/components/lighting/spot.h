#pragma once

#include "scene/components/lighting/light_component.h"

namespace RealmEngine
{
    class Spot : public LightComponent
    {
    public:
        Spot();
        ~Spot() noexcept override = default;

        size_t getTypeId() const override;

        void setConstantAttenuation(float constant);
        void setLinearAttenuation(float linear);
        void setQuadraticAttenuation(float quadratic);
        void setRange(float range);

        float getConstantAttenuation() const { return m_constant; }
        float getLinearAttenuation() const { return m_linear; }
        float getQuadraticAttenuation() const { return m_quadratic; }
        float getRange() const { return m_range; }

        void  setInnerConeAngle(float angle);
        void  setOuterConeAngle(float angle);
        float getInnerConeAngle() const { return m_inner_cone_angle; }
        float getOuterConeAngle() const { return m_outer_cone_angle; }

    private:
        float m_constant;
        float m_linear;
        float m_quadratic;
        float m_range;
        float m_inner_cone_angle;
        float m_outer_cone_angle;
    };

} // namespace RealmEngine
