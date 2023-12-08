#ifndef RR_LIB_GRAPHICS_RENDER_API_H
#define RR_LIB_GRAPHICS_RENDER_API_H

#include "rurouni/graphics/vertex_array.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics::api {

void init();
void bind_default_framebuffer();
void set_clear_color(const math::vec4& color);
void clear();
void set_viewport(const math::ivec2& size);

void draw_lines(std::shared_ptr<VertexArray> vertexArray, int32_t vertexCount);
void draw_indexed(std::shared_ptr<VertexArray> vertexArray,
                  int32_t indexCount = 0);

}  // namespace rr::graphics::api

#endif  // !RR_LIB_GRAPHICS_RENDER_API_H
