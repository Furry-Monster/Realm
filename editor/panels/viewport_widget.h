#pragma once

#include <memory>
#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;
    class EditorContext;

    class ViewportWidget : public Widget
    {
    public:
        ViewportWidget(EditorEngineBridge& bridge, const std::shared_ptr<EditorContext>& context);
        ~ViewportWidget() override = default;

        ViewportWidget(const ViewportWidget&)            = delete;
        ViewportWidget& operator=(const ViewportWidget&) = delete;

        void render() override;

    private:
        EditorEngineBridge*            m_bridge;
        std::shared_ptr<EditorContext> m_context;
        int                            m_gbuffer_preview {0};
    };

} // namespace RealmEngine
