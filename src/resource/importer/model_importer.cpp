#include "model_importer.h"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "resource/datatype/model/material.h"
#include "resource/datatype/model/node.h"
#include "utils.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace RealmEngine
{
    std::unique_ptr<Model> ModelImporter::loadModel(const std::string& filepath, const LoadOptions& options)
    {
        info("Loading model from: " + filepath);

        Assimp::Importer importer;

        // process flags
        unsigned int ai_flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords |
                                aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure;
        if (options.calculate_tangents)
            ai_flags |= aiProcess_CalcTangentSpace;
        if (options.flip_uvs)
            ai_flags |= aiProcess_FlipUVs;
        if (options.optimize_graph)
            ai_flags |= aiProcess_OptimizeGraph;
        if (options.optimize_meshes)
            ai_flags |= aiProcess_OptimizeMeshes;
        debug("< Import options > - Calculate tangents: " + std::string(options.calculate_tangents ? "ON" : "OFF") +
              ", Flip UVs: " + std::string(options.flip_uvs ? "ON" : "OFF") +
              ", Optimize meshes: " + std::string(options.optimize_meshes ? "ON" : "OFF") +
              ", Optimize graph: " + std::string(options.optimize_graph ? "ON" : "OFF"));

        // load from file
        const aiScene* scene = importer.ReadFile(filepath, ai_flags);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            err("An error occured when Assimp try to load model from :" + filepath +
                " - Error: " + importer.GetErrorString());
            return nullptr;
        }

        debug("< Assimp scene loaded successfully > - Meshes: " + std::to_string(scene->mNumMeshes) + ", Materials: " +
              std::to_string(scene->mNumMaterials) + ", Textures: " + std::to_string(scene->mNumTextures) +
              ", Animations: " + std::to_string(scene->mNumAnimations));

        std::unique_ptr<Model> model = std::make_unique<Model>();

        // get base dir path
        std::string base_dir;
        size_t      last_slash_pos = filepath.find_last_of("/\\");
        if (last_slash_pos != std::string::npos)
            base_dir = filepath.substr(0, last_slash_pos + 1);
        else
            base_dir = "./";

        // load all material
        debug("< Processing " + std::to_string(scene->mNumMaterials) + " material(s)... >");
        for (size_t i = 0; i < scene->mNumMaterials; ++i)
        {
            Material&& material = processMaterial(scene->mMaterials[i], base_dir);
            model->addMaterial(std::move(material));
        }

        // load all mesh
        debug("< Processing " + std::to_string(scene->mNumMeshes) + " mesh(es)... >");
        size_t total_vertices  = 0;
        size_t total_triangles = 0;
        for (size_t i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* ai_mesh = scene->mMeshes[i];
            total_vertices += ai_mesh->mNumVertices;
            total_triangles += ai_mesh->mNumFaces;

            Mesh&& mesh = processMesh(ai_mesh);
            model->addMesh(std::move(mesh));
        }
        debug("Total vertices: " + std::to_string(total_vertices) +
              ", Total triangles: " + std::to_string(total_triangles));

        // recursively process node tree
        debug("< Processing scene graph hierarchy... >");
        auto root = processNode(scene->mRootNode, scene);
        model->setRoot(std::move(root));

        return model;
    }

    std::unique_ptr<Node> ModelImporter::processNode(const aiNode* ai_node, const aiScene* ai_scene)
    {
        if (!ai_node || !ai_scene)
        {
            warn("Invalid parameters for node processing.");
            return nullptr;
        }

        std::unique_ptr<Node> node = std::make_unique<Node>();
        node->setLocalTransform(convertMatrix(ai_node->mTransformation));

        // Add all mesh references to this node
        for (size_t i = 0; i < ai_node->mNumMeshes; ++i)
            node->addMeshIndex(ai_node->mMeshes[i]);

        // Recursively process all child nodes
        for (size_t i = 0; i < ai_node->mNumChildren; ++i)
        {
            if (auto child = processNode(ai_node->mChildren[i], ai_scene))
                node->addChild(std::move(child));
        }

        return node;
    }

    Mesh ModelImporter::processMesh(const aiMesh* ai_mesh)
    {
        Mesh mesh;

        if (!ai_mesh || ai_mesh->mNumVertices == 0)
        {
            warn("Invalid or empty aiMesh for processing.");
            return mesh;
        }

        // Log mesh attributes
        std::string mesh_name = ai_mesh->mName.length > 0 ? std::string(ai_mesh->mName.C_Str()) : "<unnamed>";
        debug("  Mesh '" + mesh_name + "' - Vertices: " + std::to_string(ai_mesh->mNumVertices) +
              ", Faces: " + std::to_string(ai_mesh->mNumFaces) +
              ", UVs: " + std::string(ai_mesh->HasTextureCoords(0) ? "YES" : "NO") +
              ", Normals: " + std::string(ai_mesh->HasNormals() ? "YES" : "NO") +
              ", Tangents: " + std::string(ai_mesh->HasTangentsAndBitangents() ? "YES" : "NO") +
              ", Colors: " + std::string(ai_mesh->HasVertexColors(0) ? "YES" : "NO"));

        // vert process
        std::vector<Vertex> vertices;
        vertices.reserve(ai_mesh->mNumVertices);

        for (size_t i = 0; i < ai_mesh->mNumVertices; ++i)
        {
            Vertex vert;

            // pos
            vert.position = glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);

            // norm
            if (ai_mesh->HasNormals())
                vert.normal = glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
            else
                vert.normal = glm::vec3(0.0f, 1.0f, 0.0f);

            // uv
            if (ai_mesh->HasTextureCoords(0))
                vert.tex_coord = glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
            else
                vert.tex_coord = glm::vec2(0.0f);

            // tang / bitang
            if (ai_mesh->HasTangentsAndBitangents())
            {
                vert.tangent = glm::vec3(ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z);
                vert.bitangent =
                    glm::vec3(ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z);
            }
            else
            {
                vert.tangent   = glm::vec3(1.0f, 0.0f, 0.0f);
                vert.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
            }

            // vert color
            if (ai_mesh->HasVertexColors(0))
                vert.color = glm::vec4(ai_mesh->mColors[0][i].r,
                                       ai_mesh->mColors[0][i].g,
                                       ai_mesh->mColors[0][i].b,
                                       ai_mesh->mColors[0][i].a);
            else
                vert.color = glm::vec4(1.0f);

            vertices.push_back(vert);
        }

        mesh.setVertices(std::move(vertices));

        // index process
        std::vector<uint32_t> indices;
        indices.reserve(ai_mesh->mNumFaces * 3);

        for (size_t i = 0; i < ai_mesh->mNumFaces; ++i)
        {
            const aiFace& face = ai_mesh->mFaces[i];

            for (size_t j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[i]);
        }

        mesh.setIndices(std::move(indices));

        // auto gen the default submesh
        SubMesh submesh(0, static_cast<uint32_t>(indices.size()), ai_mesh->mMaterialIndex);
        mesh.addSubMesh(submesh);

        // calculate bounding box.
        mesh.calculateAABB();
        return mesh;
    }

    Material ModelImporter::processMaterial(const aiMaterial* ai_material, const std::string& base_dir)
    {
        Material material;

        if (!ai_material || base_dir.empty())
        {
            warn("Invalid ai_material or empty base dir path.");
            return material;
        }

        // Get material name
        aiString    mat_name;
        std::string material_name = "<unnamed>";
        if (ai_material->Get(AI_MATKEY_NAME, mat_name) == AI_SUCCESS && mat_name.length > 0)
            material_name = std::string(mat_name.C_Str());

        debug("  Material '" + material_name + "' - Textures: " +
              std::to_string(ai_material->GetTextureCount(aiTextureType_BASE_COLOR) +
                             ai_material->GetTextureCount(aiTextureType_DIFFUSE) +
                             ai_material->GetTextureCount(aiTextureType_NORMALS) +
                             ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) +
                             ai_material->GetTextureCount(aiTextureType_EMISSIVE)));

        aiString texture_path;

        // ===== Material Textures (PBR Textures) =====
        // Base Color / Albedo
        if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
        {
            // glTF 2.0 PBR
            if (ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path) == AI_SUCCESS)
                material.setBaseColorTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            // FBX,OBJ,3DS usage
            if (ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == AI_SUCCESS)
                material.setBaseColorTexture(base_dir + std::string(texture_path.C_Str()));
        }

        // Metallic-Roughness
        if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) > 0)
        {
            // glTF combined metallic-roughness (r = unused, g = roughness, b = metallic)
            if (ai_material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texture_path) == AI_SUCCESS)
                material.setMetallicRoughnessTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_METALNESS) > 0)
        {
            // FBX metallic
            if (ai_material->GetTexture(aiTextureType_METALNESS, 0, &texture_path) == AI_SUCCESS)
                material.setMetallicRoughnessTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
        {
            // single Roughness
            if (ai_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texture_path) == AI_SUCCESS)
                material.setMetallicRoughnessTexture(base_dir + std::string(texture_path.C_Str()));
        }

        // Normal Map
        if (ai_material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            // standard Normal map for glTF,FBX,etc.
            if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == AI_SUCCESS)
                material.setNormalTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
        {
            // alternate for Normal map
            if (ai_material->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &texture_path) == AI_SUCCESS)
                material.setNormalTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            // use bump map as normal map for OBJ and etc.
            if (ai_material->GetTexture(aiTextureType_HEIGHT, 0, &texture_path) == AI_SUCCESS)
                material.setNormalTexture(base_dir + std::string(texture_path.C_Str()));
        }

        // AO
        if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
        {
            // glTF AO map
            if (ai_material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texture_path) == AI_SUCCESS)
                material.setOcclusionTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP) > 0)
        {
            // lightmap with AO for FBX,etc.
            if (ai_material->GetTexture(aiTextureType_LIGHTMAP, 0, &texture_path) == AI_SUCCESS)
                material.setOcclusionTexture(base_dir + std::string(texture_path.C_Str()));
        }

        // Emissive
        if (ai_material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
        {
            // standard emissive
            if (ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path) == AI_SUCCESS)
                material.setEmissiveTexture(base_dir + std::string(texture_path.C_Str()));
        }
        else if (ai_material->GetTextureCount(aiTextureType_EMISSION_COLOR) > 0)
        {
            // glTF emission color
            if (ai_material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texture_path) == AI_SUCCESS)
                material.setEmissiveTexture(base_dir + std::string(texture_path.C_Str()));
        }

        // ===== Material Factors (PBR Parameters) =====
        // Base Color Factor
        aiColor4D base_color(1.0f, 1.0f, 1.0f, 1.0f);
        if (ai_material->Get(AI_MATKEY_BASE_COLOR, base_color) == AI_SUCCESS)
        {
            // glTF base color
            material.setBaseColorFactor(glm::vec4(base_color.r, base_color.g, base_color.b, base_color.a));
        }
        else if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color) == AI_SUCCESS)
        {
            // FBX/OBJ diffuse color fallback
            material.setBaseColorFactor(glm::vec4(base_color.r, base_color.g, base_color.b, base_color.a));
        }

        // Opacity (Alpha)
        float opacity = 1.0f;
        if (ai_material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            glm::vec4 color = material.getBaseColorFactor();
            color.a         = opacity;
            material.setBaseColorFactor(color);
        }

        // Metallic Factor
        float metallic = 1.0f;
        if (ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
        {
            material.setMetallicFactor(metallic);
        }

        // Roughness Factor
        float roughness = 1.0f;
        if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
        {
            material.setRoughnessFactor(roughness);
        }
        else
        {
            // Convert from glossiness (Specular/Glossiness workflow)
            float glossiness = 0.0f;
            if (ai_material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness) == AI_SUCCESS)
            {
                material.setRoughnessFactor(1.0f - glossiness);
            }
            else
            {
                // Convert from shininess (Phong/Blinn-Phong)
                float shininess = 0.0f;
                if (ai_material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
                {
                    // Map shininess (0-128) to roughness (0-1)
                    // roughness ≈ sqrt(2 / (shininess + 2))
                    roughness = std::sqrt(2.0f / (shininess + 2.0f));
                    material.setRoughnessFactor(roughness);
                }
            }
        }

        // Emissive Factor
        aiColor3D emissive_color(0.0f, 0.0f, 0.0f);
        if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_color) == AI_SUCCESS)
        {
            material.setEmissiveFactor(glm::vec3(emissive_color.r, emissive_color.g, emissive_color.b));
        }

        // Emissive Intensity (glTF extension)
        float emissive_intensity = 1.0f;
        if (ai_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive_intensity) == AI_SUCCESS)
        {
            glm::vec3 emissive = material.getEmissiveFactor();
            material.setEmissiveFactor(emissive * emissive_intensity);
        }

        // Normal Scale (bump strength)
        float normal_scale = 1.0f;
        if (ai_material->Get(AI_MATKEY_BUMPSCALING, normal_scale) == AI_SUCCESS)
            material.setNormalScale(normal_scale);

        // Occlusion Strength
        material.setOcclusionStrength(1.0f);

        // ===== Render State =====
        RenderState render_state;

        // Blend Mode
        int blend_mode_int = aiBlendMode_Default;
        if (ai_material->Get(AI_MATKEY_BLEND_FUNC, blend_mode_int) == AI_SUCCESS)
        {
            if (blend_mode_int == aiBlendMode_Additive)
                render_state.blend_mode = RenderState::BlendMode::Additive;
            else if (blend_mode_int == aiBlendMode_Default)
            {
                // Auto-detect from opacity
                if (opacity < 1.0f || base_color.a < 1.0f)
                    render_state.blend_mode = RenderState::BlendMode::AlphaBlend;
                else
                    render_state.blend_mode = RenderState::BlendMode::Opaque;
            }
        }
        else
        {
            // Auto-detect blend mode
            if (opacity < 1.0f || base_color.a < 1.0f)
                render_state.blend_mode = RenderState::BlendMode::AlphaBlend;
            else
                render_state.blend_mode = RenderState::BlendMode::Opaque;
        }

        // Cull Mode (two-sided rendering)
        int two_sided = 0;
        if (ai_material->Get(AI_MATKEY_TWOSIDED, two_sided) == AI_SUCCESS)
            render_state.cull_mode = two_sided ? RenderState::CullMode::None : RenderState::CullMode::Back;
        else
            render_state.cull_mode = RenderState::CullMode::Back;

        // Depth Test and Write
        render_state.depth_test  = RenderState::DepthTest::Less;
        render_state.depth_write = (render_state.blend_mode == RenderState::BlendMode::Opaque);

        material.setRenderState(render_state);

        return material;
    }

    constexpr glm::mat4 ModelImporter::convertMatrix(const aiMatrix4x4& ai_mat)
    {
        // assimp use row-major
        // glm use column-major
        // here we convert it with Matrix Transpose
        return glm::mat4(ai_mat.a1,
                         ai_mat.b1,
                         ai_mat.c1,
                         ai_mat.d1, // col 1 to row 1
                         ai_mat.a2,
                         ai_mat.b2,
                         ai_mat.c2,
                         ai_mat.d2, // col 2 to rol 2
                         ai_mat.a3,
                         ai_mat.b3,
                         ai_mat.c3,
                         ai_mat.d3, // col 3 to row 3
                         ai_mat.a4,
                         ai_mat.b4,
                         ai_mat.c4,
                         ai_mat.d4 // col 4 to row 4
        );
    }

} // namespace RealmEngine