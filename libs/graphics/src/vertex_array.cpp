// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/vertex_array.hpp"

// external
#include <glad/gl.h>

namespace rr::graphics {

static GLenum shader_data_type_to_open_gl_base_type(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            return GL_FLOAT;
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4:
            return GL_INT;
        case ShaderDataType::UInt:
            // TODO should this return GL_INT?
            return GL_UNSIGNED_INT;
        case ShaderDataType::Bool:
            return GL_BOOL;
        default:
            require(false, "unknown ShaderDataType!");
            return 0;
    }
}

VertexArray::VertexArray() {
    glCreateVertexArrays(1, &m_RendererID);
}

VertexArray::VertexArray(float* quadVertices,
                         uint32_t quadVertexCount,
                         const BufferLayout& quadLayout) {
    const uint32_t QUAD_INDEX_COUNT = 6;
    require(quadVertexCount / QUAD_INDEX_COUNT ==
                quadLayout.get_elements().size() * 2,
            "vertexCount and layout do not match to a quad");
    glCreateVertexArrays(1, &m_RendererID);

    std::shared_ptr<VertexBuffer> quadVB = std::make_shared<VertexBuffer>(
        quadVertices, quadVertexCount * sizeof(float));
    quadVB->set_layout(quadLayout);
    add_vertex_buffer(quadVB);

    uint32_t quadIndices[QUAD_INDEX_COUNT] = {0, 1, 2, 2, 3, 0};

    auto indexBuffer = std::make_shared<IndexBuffer>(quadIndices, 6);
    set_index_buffer(indexBuffer);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::bind() const {
    glBindVertexArray(m_RendererID);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::add_vertex_buffer(
    std::shared_ptr<VertexBuffer>& vertexBuffer) {
    require(vertexBuffer->get_layout().get_elements().size() > 0,
            "Vertex Buffer has no layout!");

    glBindVertexArray(m_RendererID);
    vertexBuffer->bind();

    const auto& layout = vertexBuffer->get_layout();
    for (const auto& element : layout) {
        switch (element.Type) {
            case ShaderDataType::Float:
            case ShaderDataType::Float2:
            case ShaderDataType::Float3:
            case ShaderDataType::Float4: {
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribPointer(
                    m_VertexBufferIndex, element.GetComponentCount(),
                    shader_data_type_to_open_gl_base_type(element.Type),
                    element.Normalized ? GL_TRUE : GL_FALSE,
                    layout.get_stride(),
                    reinterpret_cast<const void*>(element.Offset));
                m_VertexBufferIndex++;
                break;
            }
            case ShaderDataType::UInt:
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4:
            case ShaderDataType::Bool: {
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribIPointer(
                    m_VertexBufferIndex, element.GetComponentCount(),
                    shader_data_type_to_open_gl_base_type(element.Type),
                    layout.get_stride(),
                    reinterpret_cast<const void*>(element.Offset));
                m_VertexBufferIndex++;
                break;
            }
            case ShaderDataType::Mat3:
            case ShaderDataType::Mat4: {
                int32_t count = element.GetComponentCount();
                for (uint8_t i = 0; i < count; i++) {
                    glEnableVertexAttribArray(m_VertexBufferIndex);
                    glVertexAttribPointer(
                        m_VertexBufferIndex, (uint8_t)count,
                        shader_data_type_to_open_gl_base_type(element.Type),
                        element.Normalized ? GL_TRUE : GL_FALSE,
                        layout.get_stride(),
                        reinterpret_cast<const void*>(element.Offset +
                                                      (int32_t)sizeof(float) *
                                                          count * i));
                    glVertexAttribDivisor(m_VertexBufferIndex, 1);
                    m_VertexBufferIndex++;
                }
                break;
            }
            default:
                require(false, "unknown ShaderDataType!");
                break;
        }
    }

    m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::set_index_buffer(std::shared_ptr<IndexBuffer>& indexBuffer) {
    glBindVertexArray(m_RendererID);
    indexBuffer->bind();

    m_IndexBuffer = indexBuffer;
}

}  // namespace rr::graphics
