#include "render/rhi/opengl/gl_vertex_input.h"

#include <glad/glad.h>

#include "render/rhi/rhi_buffer.h"

namespace RealmEngine
{
    namespace
    {
        GLenum toGLPrimitive(PrimitiveType p)
        {
            switch (p)
            {
                case PrimitiveType::Triangles:
                    return GL_TRIANGLES;
                case PrimitiveType::TriangleStrip:
                    return GL_TRIANGLE_STRIP;
                case PrimitiveType::Lines:
                    return GL_LINES;
                case PrimitiveType::Points:
                    return GL_POINTS;
            }
            return GL_TRIANGLES;
        }

        GLenum toGLAttributeType(AttributeType t)
        {
            switch (t)
            {
                case AttributeType::Float:
                    return GL_FLOAT;
                case AttributeType::Int:
                    return GL_INT;
                case AttributeType::UnsignedInt:
                    return GL_UNSIGNED_INT;
                case AttributeType::Short:
                    return GL_SHORT;
                case AttributeType::UnsignedShort:
                    return GL_UNSIGNED_SHORT;
                case AttributeType::Byte:
                    return GL_BYTE;
                case AttributeType::UnsignedByte:
                    return GL_UNSIGNED_BYTE;
            }
            return GL_FLOAT;
        }

        GLenum toGLIndexType(IndexType t) { return (t == IndexType::UInt16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT; }

        bool isIntegerType(AttributeType t) { return t != AttributeType::Float; }
    } // namespace

    GLVertexInput::GLVertexInput(const VertexLayout& layout, RHIBuffer& vertex_buffer, RHIBuffer* index_buffer)
    {
        m_has_index_buffer = (index_buffer != nullptr);
        m_vertex_buffer    = &vertex_buffer;
        m_index_buffer     = index_buffer;

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        vertex_buffer.bind(); // GL_ARRAY_BUFFER

        for (const auto& attr : layout.attributes)
        {
            glEnableVertexAttribArray(attr.location);

            GLenum gl_type = toGLAttributeType(attr.type);

            if (isIntegerType(attr.type) && !attr.normalized)
            {
                // Integer attributes (used as-is in shader, e.g. bone IDs)
                glVertexAttribIPointer(attr.location,
                                       static_cast<GLint>(attr.component_count),
                                       gl_type,
                                       static_cast<GLsizei>(layout.stride),
                                       reinterpret_cast<const void*>(attr.offset));
            }
            else
            {
                // Float attributes (also handles normalized integer -> float conversion)
                glVertexAttribPointer(attr.location,
                                      static_cast<GLint>(attr.component_count),
                                      gl_type,
                                      attr.normalized ? GL_TRUE : GL_FALSE,
                                      static_cast<GLsizei>(layout.stride),
                                      reinterpret_cast<const void*>(attr.offset));
            }
        }

        if (index_buffer)
            index_buffer->bind(); // GL_ELEMENT_ARRAY_BUFFER bound inside VAO

        glBindVertexArray(0);
    }

    GLVertexInput::~GLVertexInput()
    {
        if (m_vao != 0)
            glDeleteVertexArrays(1, &m_vao);
    }

    void GLVertexInput::bind() { glBindVertexArray(m_vao); }

    void GLVertexInput::unbind() { glBindVertexArray(0); }

    void GLVertexInput::draw(PrimitiveType primitive, uint32_t count)
    {
        if (m_vertex_buffer)
            m_vertex_buffer->bind();
        glBindVertexArray(m_vao);
        glDrawArrays(toGLPrimitive(primitive), 0, static_cast<GLsizei>(count));
        glBindVertexArray(0);
    }

    void GLVertexInput::drawIndexed(PrimitiveType primitive, uint32_t index_count, IndexType idx_type)
    {
        if (m_vertex_buffer)
            m_vertex_buffer->bind();
        glBindVertexArray(m_vao);
        glDrawElements(toGLPrimitive(primitive), static_cast<GLsizei>(index_count), toGLIndexType(idx_type), nullptr);
        glBindVertexArray(0);
    }

    void GLVertexInput::drawInstanced(PrimitiveType primitive, uint32_t count, uint32_t instance_count)
    {
        if (m_vertex_buffer)
            m_vertex_buffer->bind();
        glBindVertexArray(m_vao);
        glDrawArraysInstanced(
            toGLPrimitive(primitive), 0, static_cast<GLsizei>(count), static_cast<GLsizei>(instance_count));
        glBindVertexArray(0);
    }

    void GLVertexInput::drawIndexedInstanced(PrimitiveType primitive,
                                             uint32_t      index_count,
                                             uint32_t      instance_count,
                                             IndexType     idx_type)
    {
        if (m_vertex_buffer)
            m_vertex_buffer->bind();
        glBindVertexArray(m_vao);
        glDrawElementsInstanced(toGLPrimitive(primitive),
                                static_cast<GLsizei>(index_count),
                                toGLIndexType(idx_type),
                                nullptr,
                                static_cast<GLsizei>(instance_count));
        glBindVertexArray(0);
    }

} // namespace RealmEngine
