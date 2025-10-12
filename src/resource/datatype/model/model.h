#pragma once

#include "resource/datatype/model/material.h"
#include "resource/datatype/model/mesh.h"
#include "resource/datatype/model/node.h"
#include <cstddef>
#include <memory>
#include <vector>
namespace RealmEngine
{
    class Model
    {
    public:
        Model()           = default;
        ~Model() noexcept = default;

        Model(const Model&)                = delete;
        Model& operator=(const Model&)     = delete;
        Model(Model&&) noexcept            = default;
        Model& operator=(Model&&) noexcept = default;

        // node tree
        Node*       getRoot();
        const Node* getRoot() const;
        void        setRoot(std::unique_ptr<Node> root);

        // mesh management
        size_t      getMeshCount() const;
        Mesh&       getMesh(size_t idx);
        const Mesh& getMesh(size_t idx) const;
        const Mesh* tryGetMesh(size_t idx) const;
        size_t      addMesh(Mesh&& mesh);
        void        clearMeshes();

        // material management
        size_t          getMaterialCount() const;
        Material&       getMaterial(size_t idx);
        const Material& getMaterial(size_t idx) const;
        const Material* tryGetMaterial(size_t idx) const;
        size_t          addMaterial(Material&& material);
        void            clearMaterials();

        // misc
        void clear();
        bool isEmpty() const;
        AABB calculateAABB() const;

    private:
        std::unique_ptr<Node> m_root;
        std::vector<Mesh>     m_meshes;
        std::vector<Material> m_materials;
    };
} // namespace RealmEngine