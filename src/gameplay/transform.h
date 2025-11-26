#pragma once

#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"

namespace RealmEngine
{
    class Transform
    {
    public:
        Transform();
        ~Transform() noexcept = default;

        Transform(const Transform&)                = default;
        Transform& operator=(const Transform&)     = default;
        Transform(Transform&&) noexcept            = default;
        Transform& operator=(Transform&&) noexcept = default;

        // Position
        void      setPosition(const glm::vec3& position);
        void      setPosition(float x, float y, float z);
        glm::vec3 getPosition() const;
        void      translate(const glm::vec3& translation);
        void      translate(float x, float y, float z);

        // Rotation
        void      setRotation(const glm::quat& rotation);
        void      setRotation(const glm::vec3& euler_angles);
        void      setRotation(float x, float y, float z);
        glm::quat getRotation() const;
        glm::vec3 getEulerAngles() const;
        void      rotate(const glm::quat& rotation);
        void      rotate(const glm::vec3& axis, float angle);
        void      rotate(float x, float y, float z);

        // Scale
        void      setScale(const glm::vec3& scale);
        void      setScale(float uniform_scale);
        void      setScale(float x, float y, float z);
        glm::vec3 getScale() const;
        void      scale(const glm::vec3& scale);
        void      scale(float uniform_scale);

        // Misc
        glm::mat4 getModelMatrix() const;

        glm::vec3 getForward() const;
        glm::vec3 getRight() const;
        glm::vec3 getUp() const;

        void reset();

    private:
        glm::vec3 m_position;
        glm::quat m_rotation;
        glm::vec3 m_scale;
    };

} // namespace RealmEngine
