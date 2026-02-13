#include "renderer/ibl/ibl_geometry.h"

#include "rhi/rhi_device.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    namespace
    {
        const float k_cube_vertices[] = {
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,
            -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,
            1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,
            -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
        };

        const float k_quad_vertices[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
                                         -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
    } // namespace

    IblCubeMesh createIblCubeMesh(RHIDevice& device)
    {
        IblCubeMesh mesh;
        mesh.vbo =
            device.createBuffer(BufferType::Vertex, BufferUsage::Static, k_cube_vertices, sizeof(k_cube_vertices));

        VertexLayout layout;
        layout.stride = 3 * sizeof(float);
        layout.attributes.push_back({0, 3, AttributeType::Float, 0, false});

        mesh.vertex_input = device.createVertexInput(layout, *mesh.vbo, nullptr);
        return mesh;
    }

    IblFullscreenQuadMesh createIblFullscreenQuadMesh(RHIDevice& device)
    {
        IblFullscreenQuadMesh mesh;
        mesh.vbo =
            device.createBuffer(BufferType::Vertex, BufferUsage::Static, k_quad_vertices, sizeof(k_quad_vertices));

        VertexLayout layout;
        layout.stride = 4 * sizeof(float);
        layout.attributes.push_back({0, 2, AttributeType::Float, 0, false});
        layout.attributes.push_back({1, 2, AttributeType::Float, 2 * sizeof(float), false});

        mesh.vertex_input = device.createVertexInput(layout, *mesh.vbo, nullptr);
        return mesh;
    }
} // namespace RealmEngine
