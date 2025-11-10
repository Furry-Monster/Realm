#pragma once

#include <assimp/Importer.hpp>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "render/render_mesh.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    /**
     * A collection of meshes.
     */
    class RenderModel
    {
    public:
        /**
         * Load a glTF 2.0 model.
         * @param path
         */
        explicit RenderModel(std::string path);
        RenderModel(std::string path, bool flipTexturesVertically);
        /**
         * Load a glTF 2.0 model using a provided material. This will ignore any material
         * present in the model file.
         * @param path
         */
        RenderModel(std::string path, std::shared_ptr<RenderMaterial> material, bool flipTexturesVertically);
        void draw(Shader& shader);

    private:
        void loadModel(std::string path, bool flipTexturesVertically);

        // recursively load all meshes in the node tree
        void processNode(aiNode* node, const aiScene* scene);

        // convert assimp mesh to our own mesh class
        RenderMesh processMesh(aiMesh* mesh, const aiScene* scene);

        // loads the first texture of given type
        std::shared_ptr<Texture> loadMaterialTexture(aiMaterial* material, aiTextureType type);
        unsigned int             textureFromFile(const char* fileName, std::string directory, aiTextureType type);

        // data
        std::vector<RenderMesh>                        mMeshes;
        std::string                                     mDirectory;
        std::map<std::string, std::shared_ptr<Texture>> mTexturesLoaded;
        std::shared_ptr<RenderMaterial>                 mMaterialOverride;
    };
} // namespace RealmEngine
