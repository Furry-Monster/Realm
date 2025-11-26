#pragma once

#include <memory>
#include "gameplay/component.h"
#include "render/render_object.h"

namespace RealmEngine
{
    class Renderable : public Component
    {
    public:
        Renderable();
        explicit Renderable(std::shared_ptr<RenderObject> render_object);
        explicit Renderable(const std::string& model_path);
        Renderable(const std::string& model_path, bool flip_textures_vertically);
        ~Renderable() noexcept override = default;

        Renderable(const Renderable&)                = delete;
        Renderable& operator=(const Renderable&)     = delete;
        Renderable(Renderable&&) noexcept            = default;
        Renderable& operator=(Renderable&&) noexcept = default;

        size_t getTypeId() const override;

        void                          setRenderObject(std::shared_ptr<RenderObject> render_object);
        std::shared_ptr<RenderObject> getRenderObject() const;
        bool                          hasRenderObject() const;

        void                setModelPath(const std::string& model_path);
        const std::string&  getModelPath() const;
        bool                hasModelPath() const;

    private:
        std::shared_ptr<RenderObject> m_render_object;
        std::string                   m_model_path;
    };

} // namespace RealmEngine
