#ifndef RR_LIB_GRAPHICS_VERTEX_BUFFER_H
#define RR_LIB_GRAPHICS_VERTEX_BUFFER_H

// rurouni
#include "rurouni/graphics/logger.hpp"

// std
#include <cstdint>
#include <string>
#include <vector>

namespace rr::graphics {

enum class ShaderDataType {
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    UInt,
    Int,
    Int2,
    Int3,
    Int4,
    Bool
};

static int32_t shader_data_type_size(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::UInt:
        case ShaderDataType::Int:
        case ShaderDataType::Float:
            return 4;
        case ShaderDataType::Int2:
        case ShaderDataType::Float2:
            return 4 * 2;
        case ShaderDataType::Int3:
        case ShaderDataType::Float3:
            return 4 * 3;
        case ShaderDataType::Int4:
        case ShaderDataType::Float4:
            return 4 * 4;
        case ShaderDataType::Mat3:
            return 4 * 3 * 3;
        case ShaderDataType::Mat4:
            return 4 * 4 * 4;
        case ShaderDataType::Bool:
            return 1;
        default:
            require(false, "unknown ShaderDataType!");
            return 0;
    }
}

struct BufferElement {
    ShaderDataType Type;
    std::string Name;
    int32_t Size;
    int32_t Offset;
    bool Normalized;

    BufferElement() {}

    BufferElement(ShaderDataType type,
                  const std::string& name,
                  bool normalized = false)
        : Type(type),
          Name(name),
          Size(shader_data_type_size(type)),
          Offset(0),
          Normalized(normalized) {}

    int32_t GetComponentCount() const {
        switch (Type) {
            case ShaderDataType::UInt:
            case ShaderDataType::Int:
            case ShaderDataType::Float:
                return 1;
            case ShaderDataType::Int2:
            case ShaderDataType::Float2:
                return 2;
            case ShaderDataType::Int3:
            case ShaderDataType::Float3:
                return 3;
            case ShaderDataType::Int4:
            case ShaderDataType::Float4:
                return 4;
            case ShaderDataType::Mat3:
                return 3 * 3;
            case ShaderDataType::Mat4:
                return 4 * 4;
            case ShaderDataType::Bool:
                return 1;
            default:
                require(false, "unknown ShaderDataType!");
                return 0;
        }
    }
};

class BufferLayout {
   public:
    BufferLayout() {}

    BufferLayout(const std::initializer_list<BufferElement>& elements)
        : m_Elements(elements) {
        calculate_offsets_and_stride();
    }

    inline const std::vector<BufferElement>& get_elements() const {
        return m_Elements;
    }
    inline int32_t get_stride() const { return m_Stride; }

    std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
    std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const {
        return m_Elements.begin();
    }
    std::vector<BufferElement>::const_iterator end() const {
        return m_Elements.end();
    }

   private:
    void calculate_offsets_and_stride() {
        int32_t offset = 0;
        m_Stride = 0;
        for (auto& element : m_Elements) {
            element.Offset = offset;
            offset += element.Size;
            m_Stride += element.Size;
        }
    }

   private:
    std::vector<BufferElement> m_Elements;
    int32_t m_Stride = 0;
};

class VertexBuffer {
   public:
    VertexBuffer(size_t size);
    VertexBuffer(float* vertices, size_t size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;

    void set_data(const void* data, uint32_t size);

    void set_layout(const BufferLayout& layout) { m_Layout = layout; }
    const BufferLayout& get_layout() const { return m_Layout; }

   private:
    uint32_t m_RendererID;
    BufferLayout m_Layout;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_VERTEX_BUFFER_H
