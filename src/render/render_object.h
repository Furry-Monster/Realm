#pragma once

#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "render/render_mesh.h"

namespace RealmEngine
{
    class RenderObject
    {
    public:
        explicit RenderObject(std::string path);
        RenderObject(std::string path, bool flipTexturesVertically);
        RenderObject(std::string path, std::shared_ptr<RenderMaterial> material, bool flipTexturesVertically);

        void draw(Shader& shader);

    private:
        void loadModel(std::string path, bool flipTexturesVertically);

        void                     processNode(aiNode* node, const aiScene* scene);
        RenderMesh               processMesh(aiMesh* mesh, const aiScene* scene);
        std::shared_ptr<Texture> loadMaterialTexture(aiMaterial* material, aiTextureType type);
        unsigned int             textureFromFile(const char* file_name, std::string directory, aiTextureType type);

        std::vector<RenderMesh>                         m_meshes;
        std::string                                     m_directory;
        std::map<std::string, std::shared_ptr<Texture>> m_textures_loaded;
        std::shared_ptr<RenderMaterial>                 m_material_override;
    };
} // namespace RealmEngine
