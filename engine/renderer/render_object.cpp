#include "renderer/render_object.h"

#include <assimp/GltfMaterial.h>
#include <stb/stb_image.h>

#include "core/log/log_macros.h"
#include "resource/asset_manager.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    RenderObject::RenderObject(std::string path, RHIDevice& device) { loadModel(path, true, device); }

    RenderObject::RenderObject(std::string path, bool flip_textures_vertically, RHIDevice& device)
    {
        loadModel(path, flip_textures_vertically, device);
    }

    RenderObject::RenderObject(std::string   path,
                               bool          flip_textures_vertically,
                               RHIDevice&    device,
                               AssetManager* asset_mgr) : m_asset_mgr(asset_mgr)
    {
        loadModel(path, flip_textures_vertically, device);
    }

    void RenderObject::setPosition(glm::vec3 position) { m_position = position; }

    glm::vec3 RenderObject::getPosition() const { return m_position; }

    void RenderObject::setScale(glm::vec3 scale) { m_scale = scale; }

    glm::vec3 RenderObject::getScale() const { return m_scale; }

    void RenderObject::setOrientation(glm::quat orientation) { m_orientation = orientation; }

    glm::quat RenderObject::getOrientation() const { return m_orientation; }

    void RenderObject::draw(RHIShader& shader)
    {
        for (auto& mesh : m_meshes)
            mesh.draw(shader);
    }

    void RenderObject::loadModel(std::string path, bool flip_textures_vertically, RHIDevice& device)
    {
        Assimp::Importer importer;
        stbi_set_flip_vertically_on_load(flip_textures_vertically);
        const aiScene* scene =
            importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            RE_LOG_ERROR("Error loading model: " + std::string(importer.GetErrorString()));
            return;
        }

        size_t last_slash = path.find_last_of("/\\");
        if (last_slash != std::string::npos)
            m_directory = path.substr(0, last_slash);
        else
            m_directory = ".";

        RE_LOG_INFO("Loading model from: " + path);
        RE_LOG_INFO("Model directory: " + m_directory);
        RE_LOG_INFO("Scene has " + std::to_string(scene->mNumMeshes) + " meshes, " +
                    std::to_string(scene->mNumMaterials) + " materials");

        processNode(scene->mRootNode, scene, device);

        RE_LOG_INFO("Loaded " + std::to_string(m_meshes.size()) + " meshes from model");

        stbi_set_flip_vertically_on_load(true);
    }

    void RenderObject::processNode(aiNode* node, const aiScene* scene, RHIDevice& device)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene, device));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene, device);
    }

    RenderMesh RenderObject::processMesh(aiMesh* mesh, const aiScene* scene, RHIDevice& device)
    {
        std::vector<RenderVertex> vertices;
        std::vector<unsigned int> indices;
        RenderMaterial            material;

        // Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            RenderVertex vertex;

            vertex.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.m_normal   = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->mTextureCoords[0])
                vertex.m_texture_coordinates = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.m_texture_coordinates = glm::vec2(0.0f, 0.0f);

            if (mesh->mTangents && mesh->mNumVertices > 0)
                vertex.m_tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            else
                vertex.m_tangent = glm::vec3(0.0f);

            if (mesh->mBitangents && mesh->mNumVertices > 0)
                vertex.m_bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            else
                vertex.m_bitangent = glm::vec3(0.0f);

            vertices.push_back(vertex);
        }

        // Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Material
        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];

            // Albedo: try glTF base color first, then fallback to diffuse
            if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR))
            {
                material.use_texture_albedo = true;
                material.texture_albedo     = loadMaterialTexture(ai_material, aiTextureType_BASE_COLOR, device);
            }
            else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE))
            {
                material.use_texture_albedo = true;
                material.texture_albedo     = loadMaterialTexture(ai_material, aiTextureType_DIFFUSE, device);
            }

            // Metallic-roughness (combined in glTF 2.0)
            if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
            {
                material.use_texture_metallic_roughness = true;
                material.texture_metallic_roughness =
                    loadMaterialTexture(ai_material, aiTextureType_GLTF_METALLIC_ROUGHNESS, device);
            }
            else if (ai_material->GetTextureCount(aiTextureType_UNKNOWN))
            {
                material.use_texture_metallic_roughness = true;
                material.texture_metallic_roughness = loadMaterialTexture(ai_material, aiTextureType_UNKNOWN, device);
            }

            // Normal
            if (ai_material->GetTextureCount(aiTextureType_NORMALS))
            {
                material.use_texture_normal = true;
                material.texture_normal     = loadMaterialTexture(ai_material, aiTextureType_NORMALS, device);
            }

            // Ambient occlusion: try glTF AO first, then lightmap fallback
            if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION))
            {
                material.use_texture_ambient_occlusion = true;
                material.texture_ambient_occlusion =
                    loadMaterialTexture(ai_material, aiTextureType_AMBIENT_OCCLUSION, device);
            }
            else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP))
            {
                material.use_texture_ambient_occlusion = true;
                material.texture_ambient_occlusion = loadMaterialTexture(ai_material, aiTextureType_LIGHTMAP, device);
            }

            // Emissive
            if (ai_material->GetTextureCount(aiTextureType_EMISSIVE))
            {
                material.use_texture_emissive = true;
                material.texture_emissive     = loadMaterialTexture(ai_material, aiTextureType_EMISSIVE, device);
            }
        }

        return RenderMesh(vertices, indices, material, device);
    }

    std::shared_ptr<RHITexture>
    RenderObject::loadMaterialTexture(aiMaterial* material, aiTextureType type, RHIDevice& device)
    {
        aiString path;
        material->GetTexture(type, 0, &path);

        auto iterator = m_textures_loaded.find(std::string(path.C_Str()));
        if (iterator != m_textures_loaded.end())
            return iterator->second;

        std::shared_ptr<RHITexture> texture;
        if (m_asset_mgr)
        {
            bool is_srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR);
            texture      = m_asset_mgr->getOrLoadTexture(path.C_Str(), m_directory, is_srgb, device);
        }
        else
        {
            texture = textureFromFile(path.C_Str(), m_directory, type, device);
        }

        if (texture)
            m_textures_loaded.insert({std::string(path.C_Str()), texture});

        return texture;
    }

    std::shared_ptr<RHITexture>
    RenderObject::textureFromFile(const char* file_name, std::string directory, aiTextureType type, RHIDevice& device)
    {
        int width, height, num_channels;

        std::string relative_path = file_name;
        std::string path;
        if (relative_path[0] == '/' || (relative_path.length() > 1 && relative_path[1] == ':'))
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

        // Determine if this texture should be in sRGB color space.
        // Diffuse / base-color textures are authored in sRGB; all others are linear.
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

        auto texture = device.createTexture(desc);
        stbi_image_free(data);

        return texture;
    }
} // namespace RealmEngine
