#pragma once

#include <memory>
#include "widget.h"

namespace RealmEngine
{
    class EditorContext;
    class SceneManager;

    class PropertiesWidget : public Widget
    {
    public:
        PropertiesWidget(std::shared_ptr<EditorContext> context, SceneManager& scene_mgr);
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
        SceneManager&                  m_scene_mgr;
    };

} // namespace RealmEngine
