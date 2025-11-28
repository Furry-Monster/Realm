#include "gameplay/components/lighting/directional.h"
#include <typeindex>

namespace RealmEngine
{
    Directional::Directional() : LightComponent(LightType::Directional) {}

    size_t Directional::getTypeId() const { return std::type_index(typeid(Directional)).hash_code(); }

} // namespace RealmEngine
