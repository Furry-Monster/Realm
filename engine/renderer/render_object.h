#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

#include "renderer/render_mesh.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;

    class RenderObject
    {
    public:
        explicit RenderObject(std::vector<RenderMesh> meshes);

        [[nodiscard]] bool   isEmpty() const { return m_meshes.empty(); }
        [[nodiscard]] size_t getMeshCount() const { return m_meshes.size(); }
        [[nodiscard]] bool   hasTransparentMeshes() const;
        [[nodiscard]] bool   hasCustomShaderMeshes() const;
        [[nodiscard]] int    getTriangleCount(size_t mesh_index) const;

        void draw(RHIShader& shader);
        void drawOpaque(RHIShader& shader);
        void drawTransparent(RHIShader& shader, RHIDevice& device);
        void drawHair(RHIShader& shader);
        void drawShadow(RHIShader& shader);

        // Iterate custom shader meshes, callback receives (RenderMesh&)
        void forEachCustomOpaqueMesh(const std::function<void(RenderMesh&)>& fn);
        void forEachCustomTransparentMesh(const std::function<void(RenderMesh&)>& fn);

        [[nodiscard]] RenderMesh*       getMesh(size_t index);
        [[nodiscard]] const RenderMesh* getMesh(size_t index) const;

    private:
        std::vector<RenderMesh> m_meshes;
    };

} // namespace RealmEngine
