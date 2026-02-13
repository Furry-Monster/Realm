#include "resource/model_loader.h"

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include "core/log/log_macros.h"
#include "renderer/render_material.h"
#include "renderer/render_mesh.h"
#include "resource/asset_manager.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    namespace
    {
        void processNode(const aiNode*                                                 node,
                         const aiScene*                                                scene,
                         RHIDevice&                                                    device,
                         const std::string&                                            directory,
                         AssetManager*                                                 asset_mgr,
                         std::vector<RenderMesh>&                                      out_meshes,
                         std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded);

        RenderMesh processMesh(const aiMesh*                                                 mesh,
                               const aiScene*                                                scene,
                               RHIDevice&                                                    device,
                               const std::string&                                            directory,
                               AssetManager*                                                 asset_mgr,
                               std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded);

        std::shared_ptr<RHITexture>
        loadMaterialTexture(const aiMaterial*                                             material,
                            aiTextureType                                                 type,
                            RHIDevice&                                                    device,
                            const std::string&                                            directory,
                            AssetManager*                                                 asset_mgr,
                            std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded);

        std::shared_ptr<RHITexture>
        textureFromFile(const char* file_name, const std::string& directory, aiTextureType type, RHIDevice& device);
    } // namespace

    std::vector<RenderMesh>
    ModelLoader::load(const std::string& path, bool flip_textures, RHIDevice& device, AssetManager* asset_mgr)
    {
        Assimp::Importer importer;
        stbi_set_flip_vertically_on_load(flip_textures);
        const aiScene* scene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            RE_LOG_ERROR("Error loading model: " + std::string(importer.GetErrorString()));
            return {};
        }

        std::string directory;
        size_t      last_slash = path.find_last_of("/\\");
        if (last_slash != std::string::npos)
            directory = path.substr(0, last_slash);
        else
            directory = ".";

        RE_LOG_INFO("Loading model from: " + path);
        RE_LOG_INFO("Model directory: " + directory);
        RE_LOG_INFO("Scene has " + std::to_string(scene->mNumMeshes) + " meshes, " +
                    std::to_string(scene->mNumMaterials) + " materials");

        std::vector<RenderMesh>                                      meshes;
        std::unordered_map<std::string, std::shared_ptr<RHITexture>> textures_loaded;
        processNode(scene->mRootNode, scene, device, directory, asset_mgr, meshes, textures_loaded);

        RE_LOG_INFO("Loaded " + std::to_string(meshes.size()) + " meshes from model");

        stbi_set_flip_vertically_on_load(true);
        return meshes;
    }

    namespace
    {
        void processNode(const aiNode*                                                 node,
                         const aiScene*                                                scene,
                         RHIDevice&                                                    device,
                         const std::string&                                            directory,
                         AssetManager*                                                 asset_mgr,
                         std::vector<RenderMesh>&                                      out_meshes,
                         std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded)
        {
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                out_meshes.push_back(processMesh(mesh, scene, device, directory, asset_mgr, textures_loaded));
            }

            for (unsigned int i = 0; i < node->mNumChildren; i++)
                processNode(node->mChildren[i], scene, device, directory, asset_mgr, out_meshes, textures_loaded);
        }

        RenderMesh processMesh(const aiMesh*                                                 mesh,
                               const aiScene*                                                scene,
                               RHIDevice&                                                    device,
                               const std::string&                                            directory,
                               AssetManager*                                                 asset_mgr,
                               std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded)
        {
            std::vector<RenderVertex> vertices;
            std::vector<unsigned int> indices;
            RenderMaterial            material;

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                RenderVertex vertex;

                vertex.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                vertex.m_normal   = mesh->mNormals ?
                                        glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) :
                                        glm::vec3(0.0f, 1.0f, 0.0f);

                if (mesh->mTextureCoords[0])
                    vertex.m_texture_coordinates =
                        glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
                else
                    vertex.m_texture_coordinates = glm::vec2(0.0f, 0.0f);

                if (mesh->mTangents && mesh->mNumVertices > 0)
                    vertex.m_tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                else
                    vertex.m_tangent = glm::vec3(0.0f);

                if (mesh->mBitangents && mesh->mNumVertices > 0)
                    vertex.m_bitangent =
                        glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
                else
                    vertex.m_bitangent = glm::vec3(0.0f);

                vertices.push_back(vertex);
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                const aiFace& face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            if (mesh->mMaterialIndex < scene->mNumMaterials)
            {
                const aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];

                if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR))
                {
                    material.use_texture_albedo = true;
                    material.texture_albedo     = loadMaterialTexture(
                        ai_material, aiTextureType_BASE_COLOR, device, directory, asset_mgr, textures_loaded);
                }
                else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE))
                {
                    material.use_texture_albedo = true;
                    material.texture_albedo     = loadMaterialTexture(
                        ai_material, aiTextureType_DIFFUSE, device, directory, asset_mgr, textures_loaded);
                }

                if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
                {
                    material.use_texture_metallic_roughness = true;
                    material.texture_metallic_roughness     = loadMaterialTexture(ai_material,
                                                                              aiTextureType_GLTF_METALLIC_ROUGHNESS,
                                                                              device,
                                                                              directory,
                                                                              asset_mgr,
                                                                              textures_loaded);
                }
                else if (ai_material->GetTextureCount(aiTextureType_UNKNOWN))
                {
                    material.use_texture_metallic_roughness = true;
                    material.texture_metallic_roughness     = loadMaterialTexture(
                        ai_material, aiTextureType_UNKNOWN, device, directory, asset_mgr, textures_loaded);
                }

                if (ai_material->GetTextureCount(aiTextureType_NORMALS))
                {
                    material.use_texture_normal = true;
                    material.texture_normal     = loadMaterialTexture(
                        ai_material, aiTextureType_NORMALS, device, directory, asset_mgr, textures_loaded);
                }

                if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION))
                {
                    material.use_texture_ambient_occlusion = true;
                    material.texture_ambient_occlusion     = loadMaterialTexture(
                        ai_material, aiTextureType_AMBIENT_OCCLUSION, device, directory, asset_mgr, textures_loaded);
                }
                else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP))
                {
                    material.use_texture_ambient_occlusion = true;
                    material.texture_ambient_occlusion     = loadMaterialTexture(
                        ai_material, aiTextureType_LIGHTMAP, device, directory, asset_mgr, textures_loaded);
                }

                if (ai_material->GetTextureCount(aiTextureType_EMISSIVE))
                {
                    material.use_texture_emissive = true;
                    material.texture_emissive     = loadMaterialTexture(
                        ai_material, aiTextureType_EMISSIVE, device, directory, asset_mgr, textures_loaded);
                }
            }

            return RenderMesh(std::move(vertices), std::move(indices), std::move(material), device);
        }

        std::shared_ptr<RHITexture>
        loadMaterialTexture(const aiMaterial*                                             material,
                            aiTextureType                                                 type,
                            RHIDevice&                                                    device,
                            const std::string&                                            directory,
                            AssetManager*                                                 asset_mgr,
                            std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded)
        {
            aiString path;
            material->GetTexture(type, 0, &path);

            auto it = textures_loaded.find(std::string(path.C_Str()));
            if (it != textures_loaded.end())
                return it->second;

            std::shared_ptr<RHITexture> texture;
            if (asset_mgr)
            {
                bool is_srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR);
                texture      = asset_mgr->getOrLoadTexture(path.C_Str(), directory, is_srgb, device);
            }
            else
            {
                texture = textureFromFile(path.C_Str(), directory, type, device);
            }

            if (texture)
                textures_loaded.emplace(std::string(path.C_Str()), texture);

            return texture;
        }

        std::shared_ptr<RHITexture>
        textureFromFile(const char* file_name, const std::string& directory, aiTextureType type, RHIDevice& device)
        {
            int width, height, num_channels;

            std::string relative_path = file_name;
            std::string path;
            if (!relative_path.empty() &&
                (relative_path[0] == '/' || (relative_path.length() > 1 && relative_path[1] == ':')))
                path = relative_path;
            else
                path = directory + '/' + relative_path;

            RE_LOG_DEBUG("Loading texture: " + path);

            unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);
            if (!data)
            {
                RE_LOG_ERROR("Failed to load texture data: " + path);
                return nullptr;
            }

            bool is_srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR);

            TextureFormat format;
            switch (num_channels)
            {
                case 1:
                    format = TextureFormat::R8;
                    break;
                case 3:
                    format = is_srgb ? TextureFormat::SRGB8 : TextureFormat::RGB8;
                    break;
                case 4:
                    format = is_srgb ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
                    break;
                default:
                    RE_LOG_ERROR("Unsupported texture format with " + std::to_string(num_channels) + " channels");
                    stbi_image_free(data);
                    return nullptr;
            }

            TextureDesc desc;
            desc.type       = TextureType::Texture2D;
            desc.format     = format;
            desc.width      = width;
            desc.height     = height;
            desc.min_filter = TextureFilter::Linear;
            desc.mag_filter = TextureFilter::Linear;
            desc.wrap_s     = TextureWrap::Repeat;
            desc.wrap_t     = TextureWrap::Repeat;
            desc.gen_mips   = true;
            desc.data       = data;

            auto tex = device.createTexture(desc);
            stbi_image_free(data);

            if (!tex)
                return nullptr;

            return std::shared_ptr<RHITexture>(std::move(tex));
        }
    } // namespace
} // namespace RealmEngine
