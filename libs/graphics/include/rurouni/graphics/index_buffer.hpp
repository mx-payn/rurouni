#ifndef RR_LIB_GRAPHICS_INDEX_BUFFER_H
#define RR_LIB_GRAPHICS_INDEX_BUFFER_H

#include <cstdint>

namespace rr::graphics {

class IndexBuffer {
   public:
    IndexBuffer(uint32_t* indices, int32_t count);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;

    int32_t get_count() const { return m_Count; }

   private:
    uint32_t m_RendererID;
    int32_t m_Count;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_INDEX_BUFFER_H
