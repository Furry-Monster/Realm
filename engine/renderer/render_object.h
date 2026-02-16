#pragma once

#include <functional>
#include <glm/glm.hpp>

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
        [[nodiscard]] int    getTriangleCount(size_t mesh_index) const;

        // Generic iteration
        void forEachMesh(const std::function<void(RenderMesh&)>& fn);

        // Filtered iteration by blend mode / shader
        [[nodiscard]] bool hasTransparentMeshes() const;
        [[nodiscard]] bool hasCustomShaderMeshes() const;

        void drawOpaque(RHIShader& shader);
        void drawTransparent(RHIShader& shader, RHIDevice& device);
        void drawHair(RHIShader& shader);
        void drawShadow(RHIShader& shader);

        void forEachCustomOpaqueMesh(const std::function<void(RenderMesh&)>& fn);
        void forEachCustomTransparentMesh(const std::function<void(RenderMesh&)>& fn);

        [[nodiscard]] RenderMesh*       getMesh(size_t index);
        [[nodiscard]] const RenderMesh* getMesh(size_t index) const;

    private:
        // Phase 1 helper: hair is identified by ShadingModel::Custom + property
        static bool isHairMesh(const RenderMesh& mesh);
        static bool isStandardOpaque(const RenderMesh& mesh);
        static bool isStandardTransparent(const RenderMesh& mesh);

        std::vector<RenderMesh> m_meshes;
    };

} // namespace RealmEngine
