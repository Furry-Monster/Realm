#include "gameplay/components/lighting/spot.h"
#include <typeindex>

namespace RealmEngine
{
    Spot::Spot() :
        LightComponent(LightType::Spot), m_constant(1.0f), m_linear(0.09f), m_quadratic(0.032f), m_range(50.0f),
        m_inner_cone_angle(12.5f), m_outer_cone_angle(17.5f)
    {}

    size_t Spot::getTypeId() const { return std::type_index(typeid(Spot)).hash_code(); }

    void Spot::setConstantAttenuation(float constant) { m_constant = constant; }

    void Spot::setLinearAttenuation(float linear) { m_linear = linear; }

    void Spot::setQuadraticAttenuation(float quadratic) { m_quadratic = quadratic; }

    void Spot::setRange(float range) { m_range = range; }

    void Spot::setInnerConeAngle(float angle) { m_inner_cone_angle = angle; }

    void Spot::setOuterConeAngle(float angle) { m_outer_cone_angle = angle; }

} // namespace RealmEngine
