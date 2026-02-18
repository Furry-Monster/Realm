#pragma once

#include <memory>

#include "module/render/rhi/rhi_buffer.h"
#include "module/render/rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    class RHIDevice;

    struct IblCubeMesh
    {
        std::unique_ptr<RHIBuffer>      vbo;
        std::unique_ptr<RHIVertexInput> vertex_input;
    };

    struct IblFullscreenQuadMesh
    {
        std::unique_ptr<RHIBuffer>      vbo;
        std::unique_ptr<RHIVertexInput> vertex_input;
    };

    IblCubeMesh           createIblCubeMesh(RHIDevice& device);
    IblFullscreenQuadMesh createIblFullscreenQuadMesh(RHIDevice& device);
} // namespace RealmEngine
