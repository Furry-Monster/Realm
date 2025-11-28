#pragma once

#include "gameplay/components/lighting/light_component.h"

namespace RealmEngine
{
    class Directional : public LightComponent
    {
    public:
        Directional();
        ~Directional() noexcept override = default;

        size_t getTypeId() const override;
    };

} // namespace RealmEngine
