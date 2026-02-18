#pragma once

#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;

    class ProjectSettingsWidget : public Widget
    {
    public:
        explicit ProjectSettingsWidget(EditorEngineBridge& bridge);
        ~ProjectSettingsWidget() override = default;

        ProjectSettingsWidget(const ProjectSettingsWidget&)            = delete;
        ProjectSettingsWidget& operator=(const ProjectSettingsWidget&) = delete;
        ProjectSettingsWidget(ProjectSettingsWidget&&)                 = delete;
        ProjectSettingsWidget& operator=(ProjectSettingsWidget&&)      = delete;

        void render() override;

    private:
        void renderWindowSection();
        void renderRendererSection();
        void renderInputSection();
        void renderPhysicsSection();

        EditorEngineBridge* m_bridge;
    };

} // namespace RealmEngine
