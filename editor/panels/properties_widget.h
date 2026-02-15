#pragma once

#include <memory>

#include "renderer/render_material.h"
#include "rhi/rhi_texture.h"
#include "widget.h"

namespace RealmEngine
{
    class EditorContext;
    class EditorEngineBridge;

    class PropertiesWidget : public Widget
    {
    public:
        PropertiesWidget(std::shared_ptr<EditorContext> context, EditorEngineBridge& bridge);
        ~PropertiesWidget() override = default;

        PropertiesWidget(const PropertiesWidget&)            = delete;
        PropertiesWidget& operator=(const PropertiesWidget&) = delete;
        PropertiesWidget(PropertiesWidget&&)                 = delete;
        PropertiesWidget& operator=(PropertiesWidget&&)      = delete;

        void render() override;

    private:
        void renderTransform();
        void renderRenderable();
        void renderMaterialPreview(const RenderMaterial& mat);
        void renderTextureSlot(const char* label, bool use_tex, const std::shared_ptr<RHITexture>& tex);
        void renderPointLight();
        void renderSpotLight();
        void renderDirectionalLight();
        void renderAreaLight();

        std::shared_ptr<EditorContext> m_context;
        EditorEngineBridge*            m_bridge;
    };

} // namespace RealmEngine
