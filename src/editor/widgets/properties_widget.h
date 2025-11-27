#pragma once

#include "editor/widget.h"

namespace RealmEngine
{
    class EditorContext;

    class PropertiesWidget : public Widget
    {
    public:
        explicit PropertiesWidget(EditorContext* context);
        ~PropertiesWidget() override = default;

        PropertiesWidget(const PropertiesWidget&)            = delete;
        PropertiesWidget& operator=(const PropertiesWidget&) = delete;
        PropertiesWidget(PropertiesWidget&&)                 = default;
        PropertiesWidget& operator=(PropertiesWidget&&)      = default;

        void render() override;

    private:
        void renderTransform();
        void renderRenderable();
        void renderLighting();

        EditorContext* m_context {nullptr};
    };

} // namespace RealmEngine
