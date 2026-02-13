#pragma once

#include "scene/components/lighting/light_component.h"

namespace RealmEngine
{
    class Area : public LightComponent
    {
    public:
        Area();
        ~Area() noexcept override = default;

        size_t getTypeId() const override;

        void  setWidth(float width);
        void  setHeight(float height);
        float getWidth() const { return m_width; }
        float getHeight() const { return m_height; }

    private:
        float m_width;
        float m_height;
    };

} // namespace RealmEngine
