#include "rhi/opengl/gl_vertex_input.h"

#include <glad/gl.h>

#include "rhi/rhi_buffer.h"

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
    } // namespace

    GLVertexInput::GLVertexInput(const VertexLayout& layout, RHIBuffer& vertex_buffer, RHIBuffer* index_buffer)
    {
        m_has_index_buffer = (index_buffer != nullptr);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        vertex_buffer.bind(); // GL_ARRAY_BUFFER

        for (auto& attr : layout.attributes)
        {
            glEnableVertexAttribArray(attr.location);
            glVertexAttribPointer(attr.location,
                                  static_cast<GLint>(attr.component_count),
                                  GL_FLOAT,
                                  attr.normalized ? GL_TRUE : GL_FALSE,
                                  static_cast<GLsizei>(layout.stride),
                                  reinterpret_cast<const void*>(attr.offset));
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
        glBindVertexArray(m_vao);
        glDrawArrays(toGLPrimitive(primitive), 0, static_cast<GLsizei>(count));
        glBindVertexArray(0);
    }

    void GLVertexInput::drawIndexed(PrimitiveType primitive, uint32_t index_count)
    {
        glBindVertexArray(m_vao);
        glDrawElements(toGLPrimitive(primitive), static_cast<GLsizei>(index_count), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

} // namespace RealmEngine
