#pragma once

#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "renderer/render_mesh.h"

namespace RealmEngine
{
    class RHIDevice;
    class RHITexture;

    using TextureCache = std::unordered_map<std::string, std::shared_ptr<RHITexture>>;

    class RenderObject
    {
    public:
        RenderObject(std::string path, RHIDevice& device);
        RenderObject(std::string path, bool flip_textures_vertically, RHIDevice& device);

        void      setPosition(glm::vec3 position);
        glm::vec3 getPosition() const;
        void      setScale(glm::vec3 scale);
        glm::vec3 getScale() const;
        void      setOrientation(glm::quat orientation);
        glm::quat getOrientation() const;

        void draw(Shader& shader);

    private:
        void loadModel(std::string path, bool flip_textures_vertically, RHIDevice& device);

        void                        processNode(aiNode* node, const aiScene* scene, RHIDevice& device);
        RenderMesh                  processMesh(aiMesh* mesh, const aiScene* scene, RHIDevice& device);
        std::shared_ptr<RHITexture> loadMaterialTexture(aiMaterial* material, aiTextureType type, RHIDevice& device);
        std::shared_ptr<RHITexture>
        textureFromFile(const char* file_name, std::string directory, aiTextureType type, RHIDevice& device);

        glm::vec3               m_position {glm::vec3(0.0)};
        glm::vec3               m_scale {glm::vec3(1.0, 1.0, 1.0)};
        glm::quat               m_orientation {glm::quat(1.0, 0.0, 0.0, 0.0)};
        std::vector<RenderMesh> m_meshes;
        std::string             m_directory;
        TextureCache            m_textures_loaded;
    };
} // namespace RealmEngine
