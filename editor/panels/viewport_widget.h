#pragma once

#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;

    class ViewportWidget : public Widget
    {
    public:
        explicit ViewportWidget(EditorEngineBridge& bridge);
        ~ViewportWidget() override = default;

        ViewportWidget(const ViewportWidget&)            = delete;
        ViewportWidget& operator=(const ViewportWidget&) = delete;

        void render() override;

    private:
        EditorEngineBridge* m_bridge;
    };

} // namespace RealmEngine
