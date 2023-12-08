#ifndef RR_LIB_GRAPHICS_VERTEX_ARRAY_H
#define RR_LIB_GRAPHICS_VERTEX_ARRAY_H

// rurouni
#include "rurouni/graphics/index_buffer.hpp"
#include "rurouni/graphics/vertex_buffer.hpp"

// std
#include <memory>
#include <vector>

namespace rr::graphics {

class VertexArray {
   public:
    VertexArray();
    VertexArray(float* vertices,
                uint32_t vertexCount,
                const BufferLayout& layout);
    ~VertexArray();

    void bind() const;
    void unbind() const;

    void add_vertex_buffer(std::shared_ptr<VertexBuffer>& vertexBuffer);
    void set_index_buffer(std::shared_ptr<IndexBuffer>& indexBuffer);

    uint32_t get_renderer_id() const { return m_RendererID; }

    const std::vector<std::shared_ptr<VertexBuffer>>& get_vertex_buffers()
        const {
        return m_VertexBuffers;
    }
    const std::shared_ptr<IndexBuffer>& get_index_buffer() const {
        return m_IndexBuffer;
    }

   private:
    uint32_t m_RendererID;
    uint32_t m_VertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_VERTEX_ARRAY_H
