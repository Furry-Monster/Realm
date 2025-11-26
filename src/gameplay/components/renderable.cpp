#include "gameplay/components/renderable.h"
#include <typeindex>

namespace RealmEngine
{
    Renderable::Renderable() : m_render_object(nullptr) {}

    Renderable::Renderable(std::shared_ptr<RenderObject> render_object) : m_render_object(render_object) {}

    size_t Renderable::getTypeId() const { return std::type_index(typeid(Renderable)).hash_code(); }

    void Renderable::setRenderObject(std::shared_ptr<RenderObject> render_object) { m_render_object = render_object; }

    std::shared_ptr<RenderObject> Renderable::getRenderObject() const { return m_render_object; }

    bool Renderable::hasRenderObject() const { return m_render_object != nullptr; }

} // namespace RealmEngine
