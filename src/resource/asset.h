#pragma once

#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/node.h"
#include <cstddef>
#include <memory>
#include <vector>
namespace RealmEngine
{
    class Asset
    {
    public:
        Asset()           = default;
        ~Asset() noexcept = default;

        Asset(const Asset&)                = delete;
        Asset& operator=(const Asset&)     = delete;
        Asset(Asset&&) noexcept            = default;
        Asset& operator=(Asset&&) noexcept = default;

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