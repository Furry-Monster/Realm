#include "renderer/ibl/ibl_geometry.h"

#include "core/geometry/primitive_vertices.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    IblCubeMesh createIblCubeMesh(RHIDevice& device)
    {
        IblCubeMesh mesh;
        mesh.vbo = device.createBuffer(BufferType::Vertex,
                                       BufferUsage::Static,
                                       PrimitiveVertices::k_cube,
                                       PrimitiveVertices::k_cube_vertex_count *
                                           PrimitiveVertices::k_cube_floats_per_vertex * sizeof(float));

        VertexLayout layout;
        layout.stride = PrimitiveVertices::k_cube_floats_per_vertex * sizeof(float);
        layout.attributes.push_back({0, 3, AttributeType::Float, 0, false});

        mesh.vertex_input = device.createVertexInput(layout, *mesh.vbo, nullptr);
        return mesh;
    }

    IblFullscreenQuadMesh createIblFullscreenQuadMesh(RHIDevice& device)
    {
        IblFullscreenQuadMesh mesh;
        mesh.vbo = device.createBuffer(BufferType::Vertex,
                                       BufferUsage::Static,
                                       PrimitiveVertices::k_fullscreen_quad,
                                       PrimitiveVertices::k_fullscreen_quad_vertex_count *
                                           PrimitiveVertices::k_fullscreen_quad_floats_per_vertex * sizeof(float));

        VertexLayout layout;
        layout.stride = PrimitiveVertices::k_fullscreen_quad_floats_per_vertex * sizeof(float);
        layout.attributes.push_back({0, 2, AttributeType::Float, 0, false});
        layout.attributes.push_back({1, 2, AttributeType::Float, 2 * sizeof(float), false});

        mesh.vertex_input = device.createVertexInput(layout, *mesh.vbo, nullptr);
        return mesh;
    }
} // namespace RealmEngine
