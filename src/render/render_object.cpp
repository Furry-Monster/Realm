#include "render_object.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace RealmEngine
{
    void RenderObject::setModelMatrix(const glm::mat4& matrix)
    {
        m_model_matrix  = matrix;
        m_normal_matrix = glm::transpose(glm::inverse(m_model_matrix));
    }
    const glm::mat4& RenderObject::getModelMatrix() const { return m_model_matrix; }
    const glm::mat4& RenderObject::getNormalMatrix() const { return m_normal_matrix; }

    void RenderObject::setMesh(std::shared_ptr<RenderMesh> mesh) { m_mesh = std::move(mesh); }
    void RenderObject::setMaterial(std::shared_ptr<RenderMaterial> material) { m_material = std::move(material); }
    std::shared_ptr<RenderMesh>     RenderObject::getMesh() const { return m_mesh; }
    std::shared_ptr<RenderMaterial> RenderObject::getMaterial() const { return m_material; }

    void        RenderObject::setWorldBounds(const AABB& bounds) { m_world_bounds = bounds; }
    const AABB& RenderObject::getWorldBounds() const { return m_world_bounds; }

    void RenderObject::setCastShadows(bool cast) { m_cast_shadows = cast; }
    void RenderObject::setReceiveShadows(bool receive) { m_receive_shadows = receive; }
    void RenderObject::setVisible(bool visible) { m_visible = visible; }
    void RenderObject::setLayer(uint32_t layer) { m_layer = layer; }

    bool     RenderObject::castsShadows() const { return m_cast_shadows; }
    bool     RenderObject::receivesShadows() const { return m_receive_shadows; }
    bool     RenderObject::isVisible() const { return m_visible; }
    uint32_t RenderObject::getLayer() const { return m_layer; }
} // namespace RealmEngine