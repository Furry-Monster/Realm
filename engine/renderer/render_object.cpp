#include "renderer/render_object.h"

#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::vector<RenderMesh> meshes) : m_meshes(std::move(meshes)) {}

    void RenderObject::setPosition(glm::vec3 position) { m_position = position; }

    glm::vec3 RenderObject::getPosition() const { return m_position; }

    void RenderObject::setScale(glm::vec3 scale) { m_scale = scale; }

    glm::vec3 RenderObject::getScale() const { return m_scale; }

    void RenderObject::setOrientation(glm::quat orientation) { m_orientation = orientation; }

    glm::quat RenderObject::getOrientation() const { return m_orientation; }

    void RenderObject::draw(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.draw(shader);
    }

} // namespace RealmEngine
