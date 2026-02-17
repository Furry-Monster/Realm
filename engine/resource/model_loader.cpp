#include "resource/model_loader.h"

#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <glm/glm.hpp>

#include "core/log/log_macros.h"
#include "renderer/material.h"
#include "renderer/render_mesh.h"
#include "resource/asset_manager.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace fs = std::filesystem;

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
        stbi_set_flip_vertically_on_load_thread(flip_textures);
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

        stbi_set_flip_vertically_on_load_thread(true);
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
            Material                  material;
            auto&                     props = material.properties;

            material.shading_model = ShadingModel::StandardPBR;

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

            // Defaults for PBR properties
            glm::vec3 albedo(0.7f, 0.7f, 0.7f);
            float     opacity       = 1.0f;
            float     metallic_val  = 0.0f;
            float     roughness_val = 0.5f;
            float     ao_val        = 1.0f;
            glm::vec3 emissive_val(0.0f);
            float     emissive_str     = 1.0f;
            float     alpha_cutout_val = 0.5f;
            bool      is_transparent   = false;
            bool      is_double_sided  = false;

            if (mesh->mMaterialIndex < scene->mNumMaterials)
            {
                const aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];
                aiString          mat_name;
                if (ai_material->Get(AI_MATKEY_NAME, mat_name) == aiReturn_SUCCESS)
                    material.name = std::string(mat_name.C_Str());

                aiColor4D diffuse_color(0.8f, 0.8f, 0.8f, 1.0f);
                if (ai_material->Get(AI_MATKEY_BASE_COLOR, diffuse_color) != aiReturn_SUCCESS)
                    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
                albedo = glm::vec3(diffuse_color.r, diffuse_color.g, diffuse_color.b);

                if (ai_material->Get(AI_MATKEY_OPACITY, opacity) != aiReturn_SUCCESS)
                    opacity = diffuse_color.a;

                ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic_val);

                float roughness_tmp = 0.5f;
                if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_tmp) == aiReturn_SUCCESS)
                    roughness_val = roughness_tmp;
                else
                {
                    float glossiness = 0.0f;
                    if (ai_material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness) == aiReturn_SUCCESS)
                        roughness_val = 1.0f - glossiness;
                    else
                    {
                        float shininess = 0.0f;
                        if (ai_material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS && shininess > 0.0f)
                        {
                            float gloss   = std::min(shininess / 128.0f, 1.0f);
                            roughness_val = 1.0f - std::sqrt(gloss);
                        }
                    }
                }

                aiColor3D emissive_c(0.0f, 0.0f, 0.0f);
                if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_c) == aiReturn_SUCCESS)
                    emissive_val = glm::vec3(emissive_c.r, emissive_c.g, emissive_c.b);

                ai_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissive_str);

                // Texture loading helper
                auto load_tex = [&](aiTextureType type, const char* use_name, const char* tex_name, int unit) {
                    auto tex =
                        loadMaterialTexture(ai_material, type, scene, device, directory, asset_mgr, textures_loaded);
                    if (tex)
                    {
                        props.setBool(use_name, true);
                        props.setTexture(tex_name, std::move(tex), unit);
                    }
                    else
                    {
                        props.setBool(use_name, false);
                    }
                };

                bool has_albedo_tex = false;
                if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR))
                {
                    load_tex(aiTextureType_BASE_COLOR,
                             "material.useTextureAlbedo",
                             "material.textureAlbedo",
                             TEXTURE_UNIT_ALBEDO);
                    has_albedo_tex = true;
                }
                else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE))
                {
                    load_tex(aiTextureType_DIFFUSE,
                             "material.useTextureAlbedo",
                             "material.textureAlbedo",
                             TEXTURE_UNIT_ALBEDO);
                    has_albedo_tex = true;
                }
                if (!has_albedo_tex)
                    props.setBool("material.useTextureAlbedo", false);

                bool has_mr = false;
                if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
                {
                    load_tex(aiTextureType_GLTF_METALLIC_ROUGHNESS,
                             "material.useTextureMetallicRoughness",
                             "material.textureMetallicRoughness",
                             TEXTURE_UNIT_METALLIC_ROUGHNESS);
                    has_mr = true;
                }
                else if (ai_material->GetTextureCount(aiTextureType_UNKNOWN))
                {
                    load_tex(aiTextureType_UNKNOWN,
                             "material.useTextureMetallicRoughness",
                             "material.textureMetallicRoughness",
                             TEXTURE_UNIT_METALLIC_ROUGHNESS);
                    has_mr = true;
                }
                if (!has_mr)
                    props.setBool("material.useTextureMetallicRoughness", false);

                if (ai_material->GetTextureCount(aiTextureType_NORMALS))
                    load_tex(aiTextureType_NORMALS,
                             "material.useTextureNormal",
                             "material.textureNormal",
                             TEXTURE_UNIT_NORMAL);
                else
                    props.setBool("material.useTextureNormal", false);

                bool has_ao = false;
                if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION))
                {
                    load_tex(aiTextureType_AMBIENT_OCCLUSION,
                             "material.useTextureAmbientOcclusion",
                             "material.textureAmbientOcclusion",
                             TEXTURE_UNIT_AMBIENT_OCCLUSION);
                    has_ao = true;
                }
                else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP))
                {
                    load_tex(aiTextureType_LIGHTMAP,
                             "material.useTextureAmbientOcclusion",
                             "material.textureAmbientOcclusion",
                             TEXTURE_UNIT_AMBIENT_OCCLUSION);
                    has_ao = true;
                }
                if (!has_ao)
                    props.setBool("material.useTextureAmbientOcclusion", false);

                if (ai_material->GetTextureCount(aiTextureType_EMISSIVE))
                    load_tex(aiTextureType_EMISSIVE,
                             "material.useTextureEmissive",
                             "material.textureEmissive",
                             TEXTURE_UNIT_EMISSIVE);
                else
                    props.setBool("material.useTextureEmissive", false);

                if (ai_material->GetTextureCount(aiTextureType_OPACITY))
                    load_tex(aiTextureType_OPACITY,
                             "material.useTextureOpacity",
                             "material.textureOpacity",
                             TEXTURE_UNIT_OPACITY);
                else
                    props.setBool("material.useTextureOpacity", false);

                ai_material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alpha_cutout_val);

                aiString alpha_mode;
                if (ai_material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode) == aiReturn_SUCCESS)
                {
                    const char* mode = alpha_mode.C_Str();
                    if (std::strcmp(mode, "BLEND") == 0)
                        is_transparent = true;
                    else if (std::strcmp(mode, "MASK") == 0)
                        is_transparent = false;
                    else
                        is_transparent = false;
                }
                else
                    is_transparent = (opacity < 1.0f);

                int two_sided = 0;
                if (ai_material->Get(AI_MATKEY_TWOSIDED, two_sided) == aiReturn_SUCCESS && two_sided != 0)
                    is_double_sided = true;
            }

            // Store PBR properties using standard uniform names
            props.setVec3("material.albedo", albedo);
            props.setFloat("material.opacity", opacity);
            props.setFloat("material.alphaCutout", alpha_cutout_val);
            props.setFloat("material.metallic", metallic_val);
            props.setFloat("material.roughness", roughness_val);
            props.setFloat("material.ambientOcclusion", ao_val);
            props.setVec3("material.emissive", emissive_val);
            props.setFloat("material.emissiveStrength", emissive_str);

            // SSS defaults (off)
            props.setBool("material.subsurfaceEnabled", false);
            props.setFloat("material.subsurfaceRadius", 1.0f);
            props.setVec3("material.subsurfaceColor", glm::vec3(1.0f, 0.2f, 0.1f));

            // Material rendering config
            material.alpha_cutoff = alpha_cutout_val;
            material.blend_mode   = is_transparent ? BlendMode::Transparent : BlendMode::Opaque;
            material.render_face  = is_double_sided ? RenderFace::Both : RenderFace::Front;

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
            int                        width, height, num_channels;
            unsigned char*             stbi_data = nullptr;
            std::vector<unsigned char> raw_data;
            unsigned char*             texture_data = nullptr;

            if (embedded->mHeight == 0)
            {
                stbi_data    = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(embedded->pcData),
                                                  static_cast<int>(embedded->mWidth),
                                                  &width,
                                                  &height,
                                                  &num_channels,
                                                  0);
                texture_data = stbi_data;
            }
            else
            {
                width             = static_cast<int>(embedded->mWidth);
                height            = static_cast<int>(embedded->mHeight);
                num_channels      = 4;
                size_t resolution = static_cast<size_t>(width) * static_cast<size_t>(height);
                raw_data.resize(resolution * 4);
                const aiTexel* src = embedded->pcData;
                for (size_t i = 0; i < resolution; ++i)
                {
                    raw_data[i * 4 + 0] = src[i].r;
                    raw_data[i * 4 + 1] = src[i].g;
                    raw_data[i * 4 + 2] = src[i].b;
                    raw_data[i * 4 + 3] = src[i].a;
                }
                texture_data = raw_data.data();
            }

            if (!texture_data)
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
                    if (stbi_data)
                        stbi_image_free(stbi_data);
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
            desc.data       = texture_data;

            auto tex = device.createTexture(desc);
            if (stbi_data)
                stbi_image_free(stbi_data);

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
