#include "gameplay/components/lighting/area.h"
#include <typeindex>

namespace RealmEngine
{
    Area::Area() : LightComponent(LightType::Area), m_width(1.0f), m_height(1.0f) {}

    size_t Area::getTypeId() const { return std::type_index(typeid(Area)).hash_code(); }

    void Area::setWidth(float width) { m_width = width; }

    void Area::setHeight(float height) { m_height = height; }

} // namespace RealmEngine
