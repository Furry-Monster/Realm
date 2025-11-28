#pragma once

#include <memory>
#include "editor/widget.h"

namespace RealmEngine
{
    class EditorContext;

    class PropertiesWidget : public Widget
    {
    public:
        explicit PropertiesWidget(std::shared_ptr<EditorContext> context);
        ~PropertiesWidget() override = default;

        PropertiesWidget(const PropertiesWidget&)            = delete;
        PropertiesWidget& operator=(const PropertiesWidget&) = delete;
        PropertiesWidget(PropertiesWidget&&)                 = default;
        PropertiesWidget& operator=(PropertiesWidget&&)      = default;

        void render() override;

    private:
        void renderTransform();
        void renderRenderable();
        void renderPointLight();
        void renderSpotLight();
        void renderDirectionalLight();
        void renderAreaLight();

        std::shared_ptr<EditorContext> m_context;
    };

} // namespace RealmEngine
