#pragma once

#include <functional>
#include <vector>

#include "render/render_mesh.h"

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

        void forEachMesh(const std::function<void(RenderMesh&)>& fn);

        void drawShadow(RHIShader& shader);

        [[nodiscard]] RenderMesh*       getMesh(size_t index);
        [[nodiscard]] const RenderMesh* getMesh(size_t index) const;

    private:
        std::vector<RenderMesh> m_meshes;
    };

} // namespace RealmEngine
