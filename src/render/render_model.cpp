#include "render/render_model.h"

#include <glad/gl.h>
// STB_IMAGE_IMPLEMENTATION already defined in hdr_texture.cpp
#include "utils.h"
#include <stb/stb_image.h>
#include <assimp/GltfMaterial.h>

namespace RealmEngine
{
    RenderModel::RenderModel(std::string path) { loadModel(path, true); }

    RenderModel::RenderModel(std::string path, bool flipTexturesVertically) { loadModel(path, flipTexturesVertically); }

    RenderModel::RenderModel(std::string path, std::shared_ptr<RenderMaterial> material, bool flipTexturesVertically) :
        mMaterialOverride(material)
    {
        loadModel(path, flipTexturesVertically);
    }

    void RenderModel::draw(Shader& shader)
    {
        for (auto& mesh : mMeshes)
        {
            mesh.draw(shader);
        }
    }

    void RenderModel::loadModel(std::string path, bool flipTexturesVertically)
    {
        Assimp::Importer importer;
        stbi_set_flip_vertically_on_load(flipTexturesVertically);
        const aiScene* scene =
            importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            err("Error loading model: " + std::string(importer.GetErrorString()));
            return;
        }

        // Extract directory path - handle both '/' and '\' separators
        size_t last_slash = path.find_last_of("/\\");
        if (last_slash != std::string::npos)
        {
            mDirectory = path.substr(0, last_slash);
        }
        else
        {
            mDirectory = ".";
        }

        info("Loading model from: " + path);
        info("Model directory: " + mDirectory);
        info("Scene has " + std::to_string(scene->mNumMeshes) + " meshes, " + 
             std::to_string(scene->mNumMaterials) + " materials");

        processNode(scene->mRootNode, scene);
        
        info("Loaded " + std::to_string(mMeshes.size()) + " meshes from model");
        
        stbi_set_flip_vertically_on_load(true);
    }

    // recursively load all meshes in the node tree
    void RenderModel::processNode(aiNode* node, const aiScene* scene)
    {
        // process all of this node's meshes if it has any
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            mMeshes.push_back(processMesh(mesh, scene));
        }

        // continue with children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    // convert assimp mesh to our own mesh class
    RenderMesh RenderModel::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<RenderVertex> vertices;
        std::vector<unsigned int> indices;
        RenderMaterial             material;

        if (mMaterialOverride)
        {
            material = *mMaterialOverride;
        }

        // vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            RenderVertex vertex;

            // position
            glm::vec3 position;
            position.x = mesh->mVertices[i].x;
            position.y = mesh->mVertices[i].y;
            position.z = mesh->mVertices[i].z;

            vertex.m_position = position;

            // normal
            glm::vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;

            vertex.m_normal = normal;

            // texture coordinates
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 texture_coordinates;
                texture_coordinates.x        = mesh->mTextureCoords[0][i].x;
                texture_coordinates.y        = mesh->mTextureCoords[0][i].y;
                vertex.m_texture_coordinates = texture_coordinates;
            }
            else
            {
                vertex.m_texture_coordinates = glm::vec2(0.0f, 0.0f);
            }

            // tangents
            if (mesh->mTangents && mesh->mNumVertices > 0)
            {
                glm::vec3 tangent;
                tangent.x        = mesh->mTangents[i].x;
                tangent.y        = mesh->mTangents[i].y;
                tangent.z        = mesh->mTangents[i].z;
                vertex.m_tangent = tangent;
            }
            else
            {
                vertex.m_tangent = glm::vec3(0.0f);
            }

            // bitangents
            if (mesh->mBitangents && mesh->mNumVertices > 0)
            {
                glm::vec3 bitangent;
                bitangent.x        = mesh->mBitangents[i].x;
                bitangent.y        = mesh->mBitangents[i].y;
                bitangent.z        = mesh->mBitangents[i].z;
                vertex.m_bitangent = bitangent;
            }
            else
            {
                vertex.m_bitangent = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        // indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // material
        if (!mMaterialOverride)
        {
            if (mesh->mMaterialIndex >= 0)
            {
                aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];

                // albedo - try glTF base color first, then fallback to diffuse
                if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR))
                {
                    // glTF 2.0 base color
                    material.use_texture_albedo = true;
                    material.texture_albedo     = loadMaterialTexture(ai_material, aiTextureType_BASE_COLOR);
                }
                else if (ai_material->GetTextureCount(aiTextureType_DIFFUSE))
                {
                    // FBX/OBJ diffuse fallback
                    material.use_texture_albedo = true;
                    material.texture_albedo     = loadMaterialTexture(ai_material, aiTextureType_DIFFUSE);
                }

                // metallicRoughness (in gltf 2.0 they are combined in one texture)
                // Try glTF-specific texture type first
                if (ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS))
                {
                    // glTF combined metallic-roughness texture
                    material.use_texture_metallic_roughness = true;
                    material.texture_metallic_roughness     = loadMaterialTexture(ai_material, aiTextureType_GLTF_METALLIC_ROUGHNESS);
                }
                else if (ai_material->GetTextureCount(aiTextureType_UNKNOWN))
                {
                    // Fallback to UNKNOWN type (defined in assimp pbrmaterial.h)
                    // https://github.com/assimp/assimp/blob/master/include/assimp/pbrmaterial.h#L57
                    material.use_texture_metallic_roughness = true;
                    material.texture_metallic_roughness     = loadMaterialTexture(ai_material, aiTextureType_UNKNOWN);
                }

                // normal
                if (ai_material->GetTextureCount(aiTextureType_NORMALS))
                {
                    material.use_texture_normal = true;
                    material.texture_normal     = loadMaterialTexture(ai_material, aiTextureType_NORMALS);
                }

                // ambient occlusion - try glTF AO first, then lightmap
                if (ai_material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION))
                {
                    // glTF AO map
                    material.use_texture_ambient_occlusion = true;
                    material.texture_ambient_occlusion     = loadMaterialTexture(ai_material, aiTextureType_AMBIENT_OCCLUSION);
                }
                else if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP))
                {
                    // FBX lightmap fallback
                    material.use_texture_ambient_occlusion = true;
                    material.texture_ambient_occlusion     = loadMaterialTexture(ai_material, aiTextureType_LIGHTMAP);
                }

                // emissive
                if (ai_material->GetTextureCount(aiTextureType_EMISSIVE))
                {
                    material.use_texture_emissive = true;
                    material.texture_emissive     = loadMaterialTexture(ai_material, aiTextureType_EMISSIVE);
                }
            }
        }

        return RenderMesh(vertices, indices, material);
    }

    // loads the first texture of given type
    std::shared_ptr<Texture> RenderModel::loadMaterialTexture(aiMaterial* material, aiTextureType type)
    {
        aiString path;
        material->GetTexture(type, 0, &path);

        // check if we already have it loaded and use that if so
        auto iterator = mTexturesLoaded.find(std::string(path.C_Str()));
        if (iterator != mTexturesLoaded.end())
        {
            return iterator->second;
        }

        auto texture = std::make_shared<Texture>();

        texture->m_id   = textureFromFile(path.C_Str(), mDirectory, type);
        texture->m_path = path.C_Str();

        // cache it for future lookups
        mTexturesLoaded.insert(std::pair<std::string, std::shared_ptr<Texture>>(path.C_Str(), texture));

        return texture;
    }

    unsigned int RenderModel::textureFromFile(const char* fileName, std::string directory, aiTextureType type)
    {
        int width, height, num_channels;

        std::string relative_path = fileName;
        // Handle both absolute and relative paths
        std::string path;
        if (relative_path[0] == '/' || (relative_path.length() > 1 && relative_path[1] == ':'))
        {
            // Absolute path
            path = relative_path;
        }
        else
        {
            // Relative path - combine with directory
            path = directory + '/' + relative_path;
        }
        
        debug("Loading texture: " + path);

        unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);

        if (!data)
        {
            err("Failed to load texture data: " + path);
            return 0;
        }

        GLenum format;

        switch (num_channels)
        {
            case 1:
                format = GL_RED;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                err("Unsupported texture format with " + std::to_string(num_channels) + " channels");
                stbi_image_free(data);
                return 0;
        }

        GLenum internal_format = format;

        // account for sRGB textures here
        //
        // diffuse textures are in sRGB space (non-linear)
        // metallic/roughness/normals are usually in linear
        // AO depends
        if (type == aiTextureType_DIFFUSE)
        {
            if (internal_format == GL_RGB)
            {
                internal_format = GL_SRGB;
            }
            else if (internal_format == GL_RGBA)
            {
                internal_format = GL_SRGB_ALPHA;
            }
        }

        unsigned int texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // generate the texture
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // image is resized using bilinear filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // image is enlarged using bilinear filtering

        // free the image data
        stbi_image_free(data);

        return texture_id;
    }
} // namespace RealmEngine
