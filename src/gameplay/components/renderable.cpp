#include "gameplay/components/renderable.h"
#include <typeindex>
#include "render/render_object.h"

namespace RealmEngine
{
    Renderable::Renderable() : m_render_object(nullptr) {}

    Renderable::Renderable(std::shared_ptr<RenderObject> render_object) : m_render_object(render_object) {}

    Renderable::Renderable(const std::string& model_path) : m_model_path(model_path)
    {
        m_render_object = std::make_shared<RenderObject>(model_path, false);
    }

    Renderable::Renderable(const std::string& model_path, bool flip_textures_vertically) : m_model_path(model_path)
    {
        m_render_object = std::make_shared<RenderObject>(model_path, flip_textures_vertically);
    }

    size_t Renderable::getTypeId() const { return std::type_index(typeid(Renderable)).hash_code(); }

    void Renderable::setRenderObject(std::shared_ptr<RenderObject> render_object) { m_render_object = render_object; }

    std::shared_ptr<RenderObject> Renderable::getRenderObject() const { return m_render_object; }

    bool Renderable::hasRenderObject() const { return m_render_object != nullptr; }

    void               Renderable::setModelPath(const std::string& model_path) { m_model_path = model_path; }
    const std::string& Renderable::getModelPath() const { return m_model_path; }
    bool               Renderable::hasModelPath() const { return !m_model_path.empty(); }

} // namespace RealmEngine
