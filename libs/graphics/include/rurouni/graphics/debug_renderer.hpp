#ifndef RR_LIBS_GRAPHICS_DEBUG_RENDERER_H
#define RR_LIBS_GRAPHICS_DEBUG_RENDERER_H

#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/vertex_array.hpp"
#include "rurouni/math/mat.hpp"
#include "rurouni/math/vec.hpp"

namespace rr::graphics {

/** renderer that draws primitive shapes. */
class DebugRenderer {
   public:
    DebugRenderer(std::shared_ptr<Shader> circleShader,
                  std::shared_ptr<Shader> lineShader);
    ~DebugRenderer() = default;

    void begin(const math::mat4& viewProjectionMatrix);
    void begin(const math::mat4& cameraProjection,
               const math::mat4& cameraTransform);
    void end();

    void draw_circle(const math::mat4& transform,
                     const math::vec4& color,
                     float thickness,
                     float fade,
                     uint32_t entityId);
    void draw_line(const math::vec3& p1,
                   const math::vec3& p2,
                   const math::vec4& color,
                   uint32_t entityId);
    void draw_rect(const math::vec3& position,
                   const math::vec2& size,
                   const math::vec4& color,
                   uint32_t entityId);
    void draw_rect(const math::mat4& transform,
                   const math::vec4& color,
                   uint32_t entityId);

    const math::mat4& get_view_projection_matrix() const {
        return m_ViewProjectionMatrix;
    }

   private:
    void generate_circle_va();
    void generate_line_va();

    void begin_circles();
    void begin_lines();
    void flush_circles();
    void flush_lines();

    void recalculate_view_projection();

   private:
    struct CircleVertex {
        math::vec3 position;
        math::vec3 uvPosition;
        math::vec4 color;
        float thickness;
        float fade;
        uint32_t entityID;
    };

    struct LineVertex {
        math::vec3 position;
        math::vec4 color;

        uint32_t entityID;
    };

   private:
    static constexpr uint32_t MAX_QUADS = 1000;
    static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
    static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;

   private:
    // view projection
    math::mat4 m_ViewProjectionMatrix = math::mat4{1.0f};
    math::mat4 m_CameraTransform = math::mat4{1.0f};
    math::mat4 m_CameraProjection = math::mat4{1.0f};

    // general purpose
    math::mat4 m_QuadVertexPositions;
    uint32_t m_QuadIndices[MAX_INDICES];
    std::shared_ptr<IndexBuffer> m_QuadIndexBuffer;

    // circle
    std::shared_ptr<VertexArray> m_CircleVertexArray;
    std::vector<CircleVertex> m_CircleVertexArrayData;
    std::shared_ptr<VertexBuffer> m_CircleVertexBuffer;
    std::shared_ptr<Shader> m_CircleShader;

    // line
    std::shared_ptr<VertexArray> m_LineVertexArray;
    std::vector<LineVertex> m_LineVertexArrayData;
    std::shared_ptr<VertexBuffer> m_LineVertexBuffer;
    std::shared_ptr<Shader> m_LineShader;
};

}  // namespace rr::graphics

#endif  // !RR_LIBS_GRAPHICS_DEBUG_RENDERER_H
