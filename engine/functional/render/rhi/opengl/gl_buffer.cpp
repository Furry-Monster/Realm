#include "functional/render/rhi/opengl/gl_buffer.h"

#include <glad/glad.h>

namespace RealmEngine
{
    namespace
    {
        GLenum toGLTarget(BufferType type)
        {
            switch (type)
            {
                case BufferType::Vertex:
                    return GL_ARRAY_BUFFER;
                case BufferType::Index:
                    return GL_ELEMENT_ARRAY_BUFFER;
                case BufferType::Uniform:
                    return GL_UNIFORM_BUFFER;
                case BufferType::ShaderStorage:
                    return GL_SHADER_STORAGE_BUFFER;
            }
            return GL_ARRAY_BUFFER;
        }

        GLenum toGLUsage(BufferUsage usage)
        {
            switch (usage)
            {
                case BufferUsage::Static:
                    return GL_STATIC_DRAW;
                case BufferUsage::Dynamic:
                    return GL_DYNAMIC_DRAW;
                case BufferUsage::Stream:
                    return GL_STREAM_DRAW;
            }
            return GL_STATIC_DRAW;
        }
    } // namespace

    GLBuffer::GLBuffer(BufferType type, BufferUsage usage, const void* data, size_t size)
    {
        m_type   = type;
        m_usage  = usage;
        m_target = toGLTarget(type);

        glGenBuffers(1, &m_id);
        glBindBuffer(m_target, m_id);
        glBufferData(m_target, static_cast<GLsizeiptr>(size), data, toGLUsage(usage));
        glBindBuffer(m_target, 0);
    }

    GLBuffer::~GLBuffer() noexcept
    {
        if (m_id != 0)
            glDeleteBuffers(1, &m_id);
    }

    void GLBuffer::bind() { glBindBuffer(m_target, m_id); }

    void GLBuffer::unbind() { glBindBuffer(m_target, 0); }

    void GLBuffer::setData(const void* data, size_t size)
    {
        glBindBuffer(m_target, m_id);
        glBufferData(m_target, static_cast<GLsizeiptr>(size), data, toGLUsage(m_usage));
        glBindBuffer(m_target, 0);
    }

    void GLBuffer::setSubData(const void* data, size_t offset, size_t size)
    {
        glBindBuffer(m_target, m_id);
        glBufferSubData(m_target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
        glBindBuffer(m_target, 0);
    }

    void GLBuffer::bindBase(uint32_t binding_point) { glBindBufferBase(m_target, binding_point, m_id); }

} // namespace RealmEngine
