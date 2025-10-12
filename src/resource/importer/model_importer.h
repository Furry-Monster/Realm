#pragma once

#include "assimp/material.h"
#include "assimp/matrix4x4.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "resource/datatype/model/material.h"
#include "resource/datatype/model/mesh.h"
#include "resource/datatype/model/model.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class ModelImporter
    {
    public:
        struct LoadOptions
        {
            bool calculate_tangents {true};
            bool flip_uvs {true};
            bool optimize_meshes {true};
            bool optimize_graph {false};
        };

        static std::unique_ptr<Model> loadModel(const std::string& filepath, const LoadOptions& options);

    private:
        static std::unique_ptr<Node> processNode(const aiNode* ai_node, const aiScene* ai_scene);
        static Mesh                  processMesh(const aiMesh* ai_mesh);
        static Material              processMaterial(const aiMaterial* ai_material, const std::string& base_dir);

        constexpr static glm::mat4 convertMatrix(const aiMatrix4x4& ai_mat);
    };
} // namespace RealmEngine