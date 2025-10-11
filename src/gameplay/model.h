#pragma once

#include "resource/material.h"
#include "resource/mesh.h"
#include <memory>
#include <vector>

using namespace RealmEngine;

namespace Temp
{
    /**
     * @brief class <Model> should be removed.
     * <entity> with <mesh_renderer_component> should be used instead.
     *
     */
    class Model
    {
    public:
        Model()  = default;
        ~Model() = default;

        Model(const Model&)                     = delete;
        Model& operator=(const Model&)          = delete;
        Model(Model&& that) noexcept            = default;
        Model& operator=(Model&& that) noexcept = default;

    private:
        std::unique_ptr<Mesh>                  m_mesh;
        std::vector<std::unique_ptr<Material>> m_materials;
    };
} // namespace Temp