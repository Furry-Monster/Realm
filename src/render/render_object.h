#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "math.h"
#include "render/render_material.h"
#include "render/render_mesh.h"
#include <cstdint>
#include <memory>

namespace RealmEngine
{
    class RenderObject
    {
    public:
        RenderObject()           = default;
        ~RenderObject() noexcept = default;

        RenderObject(const RenderObject&)                = delete;
        RenderObject& operator=(const RenderObject&)     = delete;
        RenderObject(RenderObject&&) noexcept            = default;
        RenderObject& operator=(RenderObject&&) noexcept = default;

        void             setModelMatrix(const glm::mat4& matrix);
        const glm::mat4& getModelMatrix() const;
        const glm::mat4& getNormalMatrix() const;

        void                            setMesh(std::shared_ptr<RenderMesh> mesh);
        void                            setMaterial(std::shared_ptr<RenderMaterial> material);
        std::shared_ptr<RenderMesh>     getMesh() const;
        std::shared_ptr<RenderMaterial> getMaterial() const;

        void        setWorldBounds(const AABB& bounds);
        const AABB& getWorldBounds() const;

        void setCastShadows(bool cast);
        void setReceiveShadows(bool receive);
        void setVisible(bool visible);
        void setLayer(uint32_t layer);

        bool     castsShadows() const;
        bool     receivesShadows() const;
        bool     isVisible() const;
        uint32_t getLayer() const;

    private:
        glm::mat4 m_model_matrix {1.0f};
        glm::mat4 m_normal_matrix {1.0f};

        std::shared_ptr<RenderMesh>     m_mesh;
        std::shared_ptr<RenderMaterial> m_material;

        AABB m_world_bounds;

        bool     m_cast_shadows {true};
        bool     m_receive_shadows {true};
        bool     m_visible {true};
        uint32_t m_layer {0};
    };
} // namespace RealmEngine