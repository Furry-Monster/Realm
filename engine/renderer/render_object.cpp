#include "renderer/render_object.h"

#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::vector<RenderMesh> meshes) : m_meshes(std::move(meshes)) {}

    void RenderObject::setPosition(glm::vec3 position) { m_position = position; }

    glm::vec3 RenderObject::getPosition() const { return m_position; }

    void RenderObject::setScale(glm::vec3 scale) { m_scale = scale; }

    glm::vec3 RenderObject::getScale() const { return m_scale; }

    void RenderObject::setOrientation(glm::quat orientation) { m_orientation = orientation; }

    glm::quat RenderObject::getOrientation() const { return m_orientation; }

    bool RenderObject::hasTransparentMeshes() const
    {
        for (const auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && mesh.isTransparent())
                return true;
        }
        return false;
    }

    int RenderObject::getTriangleCount(size_t mesh_index) const
    {
        if (mesh_index >= m_meshes.size())
            return 0;
        return m_meshes[mesh_index].getTriangleCount();
    }

    void RenderObject::draw(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.draw(shader);
    }

    void RenderObject::drawOpaque(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && !mesh.isTransparent())
                mesh.draw(shader);
        }
    }

    void RenderObject::drawTransparent(RHIShader& shader, RHIDevice& device)
    {
        for (auto& mesh : m_meshes)
        {
            if (!mesh.isHair() && mesh.isTransparent())
            {
                device.setCullFace(mesh.m_material.double_sided ? CullFace::None : CullFace::Back);
                mesh.draw(shader);
            }
        }
    }

    void RenderObject::drawHair(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.drawHair(shader);
    }

    RenderMesh* RenderObject::getMesh(size_t index) { return index < m_meshes.size() ? &m_meshes[index] : nullptr; }

    const RenderMesh* RenderObject::getMesh(size_t index) const
    {
        return index < m_meshes.size() ? &m_meshes[index] : nullptr;
    }

    void RenderObject::drawShadow(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.drawShadow(shader);
    }

} // namespace RealmEngine
