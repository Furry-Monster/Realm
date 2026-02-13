#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <vector>

#include "renderer/render_mesh.h"

namespace RealmEngine
{
    class RHIShader;

    class RenderObject
    {
    public:
        explicit RenderObject(std::vector<RenderMesh> meshes);

        bool isEmpty() const { return m_meshes.empty(); }

        void      setPosition(glm::vec3 position);
        glm::vec3 getPosition() const;
        void      setScale(glm::vec3 scale);
        glm::vec3 getScale() const;
        void      setOrientation(glm::quat orientation);
        glm::quat getOrientation() const;

        void draw(RHIShader& shader);

    private:
        glm::vec3               m_position {glm::vec3(0.0)};
        glm::vec3               m_scale {glm::vec3(1.0, 1.0, 1.0)};
        glm::quat               m_orientation {glm::quat(1.0, 0.0, 0.0, 0.0)};
        std::vector<RenderMesh>  m_meshes;
    };

} // namespace RealmEngine
