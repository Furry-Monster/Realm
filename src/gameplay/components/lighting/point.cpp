#include "gameplay/components/lighting/point.h"
#include <typeindex>

namespace RealmEngine
{
    Point::Point() :
        LightComponent(LightType::Point), m_constant(1.0f), m_linear(0.09f), m_quadratic(0.032f), m_range(50.0f)
    {}

    size_t Point::getTypeId() const { return std::type_index(typeid(Point)).hash_code(); }

    void Point::setConstantAttenuation(float constant) { m_constant = constant; }

    void Point::setLinearAttenuation(float linear) { m_linear = linear; }

    void Point::setQuadraticAttenuation(float quadratic) { m_quadratic = quadratic; }

    void Point::setRange(float range) { m_range = range; }

} // namespace RealmEngine
