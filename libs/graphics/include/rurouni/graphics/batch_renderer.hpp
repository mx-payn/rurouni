#ifndef RR_LIB_GRAPHICS_BATCH_RENDERER_H
#define RR_LIB_GRAPHICS_BATCH_RENDERER_H

// rurouni
#include "rurouni/graphics/index_buffer.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/graphics/vertex_array.hpp"
#include "rurouni/graphics/vertex_buffer.hpp"
#include "rurouni/math.hpp"

// std
#include <cstdint>
#include <memory>

namespace rr::graphics {

static const BufferLayout QUAD_SHADER_BUFFER_LAYOUT = {
    {ShaderDataType::Float3, "a_Position"},
    {ShaderDataType::Float4, "a_Color"},
    {ShaderDataType::Float2, "a_TexCoord"},
    {ShaderDataType::Float, "a_TexIndex"},
    {ShaderDataType::Int, "a_EntityID"}};

class BatchRenderer {
   public:
    BatchRenderer();
    BatchRenderer(std::shared_ptr<Shader> quadShader);
    ~BatchRenderer();

    void begin(const math::mat4& viewProjectionMatrix);
    void begin(const math::mat4& cameraProjection,
               const math::mat4& cameraTransform);
    void end();

    // draw calls
    void draw_texture(const math::mat4& transform,
                      std::weak_ptr<Texture> texture,
                      math::vec4 color,
                      uint32_t entityId);
    void draw_quad(const math::mat4& transform,
                   const math::vec4& color,
                   uint32_t entityId);

    const math::mat4& get_view_projection_matrix() const {
        return m_ViewProjectionMatrix;
    }

   private:
    void recalculate_view_projection();

    float get_texture_index(std::weak_ptr<Texture> texture);

    void generate_quad_va();

    void begin_quads();
    void flush_quads();

   private:
    struct QuadVertex {
        math::vec3 position;
        math::vec4 color;
        math::vec2 texCoord;
        float texIndex = 0;
        uint32_t entityID;
    };

   private:
    // consts
    static constexpr uint32_t MAX_QUADS = 1000;
    static constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
    static constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;

    static constexpr uint32_t MAX_TEXTURE_SLOTS = 32;
    static constexpr std::array<math::vec2, 4> DEFAULT_UV_TEXTURE_COORDS = {
        math::vec2{0.0f, 0.0f}, math::vec2{1.0f, 0.0f}, math::vec2{1.0f, 1.0f},
        math::vec2{0.0f, 1.0f}};

    // view projection
    math::mat4 m_ViewProjectionMatrix = math::mat4{1.0f};
    math::mat4 m_CameraTransform = math::mat4{1.0f};
    math::mat4 m_CameraProjection = math::mat4{1.0f};

    // general purpose
    math::mat4 m_QuadVertexPositions;
    uint32_t m_QuadIndices[MAX_INDICES];
    std::shared_ptr<IndexBuffer> m_QuadIndexBuffer;

    // quad
    std::shared_ptr<VertexArray> m_QuadVertexArray;
    std::vector<QuadVertex> m_QuadVertexArrayData;
    std::shared_ptr<VertexBuffer> m_QuadVertexBuffer;
    std::shared_ptr<Shader> m_QuadShader;

    // texture slots
    std::shared_ptr<Texture> m_WhiteTexture;
    std::array<std::weak_ptr<Texture>, MAX_TEXTURE_SLOTS> m_TextureSlots;
    uint32_t m_TextureSlotIndex = 1;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_BATCH_RENDERER_H
