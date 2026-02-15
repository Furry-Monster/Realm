#include "resource/model_loader.h"

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <glm/glm.hpp>

#include "core/log/log_macros.h"

namespace fs = std::filesystem;
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
                            const aiScene*                                                scene,
                            RHIDevice&                                                    device,
                            const std::string&                                            directory,
                            AssetManager*                                                 asset_mgr,
                            std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded);

        std::shared_ptr<RHITexture>
        textureFromEmbedded(const aiTexture* embedded, aiTextureType type, RHIDevice& device);
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
                aiString          mat_name;
                if (ai_material->Get(AI_MATKEY_NAME, mat_name) == aiReturn_SUCCESS)
                    material.name = std::string(mat_name.C_Str());

                aiColor4D diffuse_color(0.8f, 0.8f, 0.8f, 1.0f);
                if (ai_material->Get(AI_MATKEY_BASE_COLOR, diffuse_color) != aiReturn_SUCCESS)
                    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
                material.albedo = glm::vec3(diffuse_color.r, diffuse_color.g, diffuse_color.b);

                float opacity = 1.0f;
                if (ai_material->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
                    material.opacity = opacity;
                else
                    material.opacity = diffuse_color.a;

                float metallic = 0.0f;
                if (ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS)
                    material.metallic = metallic;

                float roughness = 0.5f;
                if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS)
                    material.roughness = roughness;
                else
                {
                    float glossiness = 0.0f;
                    if (ai_material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness) == aiReturn_SUCCESS)
                        material.roughness = 1.0f - glossiness;
                    else
                    {
                        float shininess = 0.0f;
                        if (ai_material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS && shininess > 0.0f)
                        {
                            float gloss        = std::min(shininess / 128.0f, 1.0f);
                            material.roughness = 1.0f - std::sqrt(gloss);
                        }
                    }
                }

                aiColor3D emissive(0.0f, 0.0f, 0.0f);
                if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == aiReturn_SUCCESS)
                    material.emissive = glm::vec3(emissive.r, emissive.g, emissive.b);

                float emissive_strength = 1.0f;
                if (ai_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive_strength) == aiReturn_SUCCESS)
                    material.emissive_strength = emissive_strength;

                auto load_tex = [&](aiTextureType type, bool& use_flag, std::shared_ptr<RHITexture>& out_tex) {
                    auto tex =
                        loadMaterialTexture(ai_material, type, scene, device, directory, asset_mgr, textures_loaded);
                    if (tex)
                    {
                        use_flag = true;
                        out_tex  = std::move(tex);
                    }
                };

                if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR))
                    load_tex(aiTextureType_BASE_COLOR, material.use_texture_albedo, material.texture_albedo);
                else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE))
                    load_tex(aiTextureType_DIFFUSE, material.use_texture_albedo, material.texture_albedo);

                if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
                    load_tex(aiTextureType_GLTF_METALLIC_ROUGHNESS,
                             material.use_texture_metallic_roughness,
                             material.texture_metallic_roughness);
                else if (ai_material->GetTextureCount(aiTextureType_UNKNOWN))
                    load_tex(aiTextureType_UNKNOWN,
                             material.use_texture_metallic_roughness,
                             material.texture_metallic_roughness);

                if (ai_material->GetTextureCount(aiTextureType_NORMALS))
                    load_tex(aiTextureType_NORMALS, material.use_texture_normal, material.texture_normal);

                if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION))
                    load_tex(aiTextureType_AMBIENT_OCCLUSION,
                             material.use_texture_ambient_occlusion,
                             material.texture_ambient_occlusion);
                else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP))
                    load_tex(aiTextureType_LIGHTMAP,
                             material.use_texture_ambient_occlusion,
                             material.texture_ambient_occlusion);

                if (ai_material->GetTextureCount(aiTextureType_EMISSIVE))
                    load_tex(aiTextureType_EMISSIVE, material.use_texture_emissive, material.texture_emissive);

                if (ai_material->GetTextureCount(aiTextureType_OPACITY))
                    load_tex(aiTextureType_OPACITY, material.use_texture_opacity, material.texture_opacity);

                if (ai_material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, material.alpha_cutout) == aiReturn_SUCCESS)
                    ; // use loaded value

                aiString alpha_mode;
                if (ai_material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode) == aiReturn_SUCCESS)
                {
                    const char* mode = alpha_mode.C_Str();
                    if (std::strcmp(mode, "BLEND") == 0)
                        material.is_transparent = true;
                    else if (std::strcmp(mode, "MASK") == 0)
                        material.is_transparent = false; // cutout via alpha_cutout
                    else
                        material.is_transparent = false; // OPAQUE or unknown
                }
                else
                    material.is_transparent = (material.opacity < 1.0f);

                int two_sided = 0;
                if (ai_material->Get(AI_MATKEY_TWOSIDED, two_sided) == aiReturn_SUCCESS && two_sided != 0)
                    material.double_sided = true;
            }

            std::string mesh_name = mesh->mName.length > 0 ? std::string(mesh->mName.C_Str()) : "";
            return RenderMesh(std::move(vertices), std::move(indices), std::move(material), device, mesh_name);
        }

        std::string getTextureFilename(const std::string& path)
        {
            size_t slash = path.find_last_of("/\\");
            return slash != std::string::npos ? path.substr(slash + 1) : path;
        }

        std::string findTextureInDirectory(const std::string& search_root, const std::string& filename)
        {
            if (search_root.empty() || filename.empty())
                return "";
            try
            {
                fs::path root(search_root);
                if (!fs::exists(root) || !fs::is_directory(root))
                    return "";
                for (const auto& entry :
                     fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied))
                {
                    if (entry.is_regular_file() && entry.path().filename() == filename)
                        return entry.path().string();
                }
            }
            catch (const fs::filesystem_error&)
            {}
            return "";
        }

        std::string resolveTexturePath(const std::string& raw_path)
        {
            if (raw_path.empty())
                return "";
            if (raw_path[0] == '*')
                return raw_path;
            std::string path = raw_path;
            for (char& c : path)
                if (c == '\\')
                    c = '/';
            if (path[0] == '/' || (path.length() > 1 && path[1] == ':'))
                return getTextureFilename(path);
            return path;
        }

        std::shared_ptr<RHITexture>
        loadMaterialTexture(const aiMaterial*                                             material,
                            aiTextureType                                                 type,
                            const aiScene*                                                scene,
                            RHIDevice&                                                    device,
                            const std::string&                                            directory,
                            AssetManager*                                                 asset_mgr,
                            std::unordered_map<std::string, std::shared_ptr<RHITexture>>& textures_loaded)
        {
            aiString path;
            material->GetTexture(type, 0, &path);
            std::string path_str(path.C_Str());

            auto it = textures_loaded.find(path_str);
            if (it != textures_loaded.end())
                return it->second;

            std::shared_ptr<RHITexture> texture;
            if (path_str[0] == '*')
            {
                const aiTexture* embedded = scene->GetEmbeddedTexture(path_str.c_str());
                if (embedded)
                    texture = textureFromEmbedded(embedded, type, device);
            }
            if (!texture)
            {
                std::string resolved = resolveTexturePath(path_str);
                if (!resolved.empty() && resolved[0] != '*')
                {
                    bool is_srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR);
                    if (asset_mgr)
                        texture = asset_mgr->getOrLoadTexture(resolved, directory, is_srgb, device);
                    if (!texture)
                        texture = textureFromFile(resolved.c_str(), directory, type, device);
                    if (!texture && resolved != getTextureFilename(resolved))
                    {
                        std::string fallback = getTextureFilename(resolved);
                        if (asset_mgr)
                            texture = asset_mgr->getOrLoadTexture(fallback, directory, is_srgb, device);
                        if (!texture)
                            texture = textureFromFile(fallback.c_str(), directory, type, device);
                    }
                    if (!texture)
                    {
                        std::string filename = getTextureFilename(resolved);
                        std::string found    = findTextureInDirectory(directory, filename);
                        if (found.empty())
                        {
                            try
                            {
                                fs::path parent = fs::path(directory).parent_path();
                                if (parent != fs::path(directory))
                                    found = findTextureInDirectory(parent.string(), filename);
                            }
                            catch (const fs::filesystem_error&)
                            {}
                        }
                        if (!found.empty())
                        {
                            if (asset_mgr)
                                texture = asset_mgr->getOrLoadTexture(found, "", is_srgb, device);
                            if (!texture)
                                texture = textureFromFile(found.c_str(), "", type, device);
                        }
                    }
                }
            }

            if (texture)
                textures_loaded.emplace(path_str, texture);

            return texture;
        }

        std::shared_ptr<RHITexture>
        textureFromEmbedded(const aiTexture* embedded, aiTextureType type, RHIDevice& device)
        {
            int            width, height, num_channels;
            unsigned char* data = nullptr;

            if (embedded->mHeight == 0)
            {
                data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(embedded->pcData),
                                             static_cast<int>(embedded->mWidth),
                                             &width,
                                             &height,
                                             &num_channels,
                                             0);
            }
            else
            {
                width        = static_cast<int>(embedded->mWidth);
                height       = static_cast<int>(embedded->mHeight);
                num_channels = 4;
                size_t size  = static_cast<size_t>(width) * height * 4;
                data         = static_cast<unsigned char*>(malloc(size));
                if (data)
                {
                    const aiTexel* src = embedded->pcData;
                    for (int i = 0; i < width * height; ++i)
                    {
                        data[i * 4 + 0] = src[i].r;
                        data[i * 4 + 1] = src[i].g;
                        data[i * 4 + 2] = src[i].b;
                        data[i * 4 + 3] = src[i].a;
                    }
                }
            }

            if (!data)
            {
                RE_LOG_ERROR("Failed to load embedded texture");
                return nullptr;
            }

            bool          is_srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR);
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
                    free(data);
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
            if (embedded->mHeight != 0)
                free(data);
            else
                stbi_image_free(data);

            return tex ? std::shared_ptr<RHITexture>(std::move(tex)) : nullptr;
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
