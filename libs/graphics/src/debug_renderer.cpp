// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/debug_renderer.hpp"
#include "rurouni/graphics/render_api.hpp"

namespace rr::graphics {

void DebugRenderer::recalculate_view_projection() {
    m_ViewProjectionMatrix =
        m_CameraProjection * math::inverse(m_CameraTransform);
}

DebugRenderer::DebugRenderer(std::shared_ptr<Shader> circleShader,
                             std::shared_ptr<Shader> lineShader)
    : m_CircleShader(circleShader), m_LineShader(lineShader) {
    // init vertex positions
    m_QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

    // init indices
    uint32_t offset = 0;
    for (uint32_t i = 0; i < MAX_INDICES; i += 6) {
        // first triangle
        m_QuadIndices[i + 0] = offset + 0;
        m_QuadIndices[i + 1] = offset + 1;
        m_QuadIndices[i + 2] = offset + 2;
        // second triangle
        m_QuadIndices[i + 3] = offset + 2;
        m_QuadIndices[i + 4] = offset + 3;
        m_QuadIndices[i + 5] = offset + 0;

        offset += 4;
    }

    m_QuadIndexBuffer =
        std::make_shared<IndexBuffer>(m_QuadIndices, MAX_INDICES);

    // init vertex arrays
    generate_circle_va();
    generate_line_va();
}

void DebugRenderer::begin(const math::mat4& viewPropjectionMatrix) {
    m_ViewProjectionMatrix = viewPropjectionMatrix;
    begin_circles();
    begin_lines();
}

void DebugRenderer::begin(const math::mat4& cameraProjection,
                          const math::mat4& cameraTransform) {
    if (m_CameraProjection != cameraProjection ||
        m_CameraTransform != cameraTransform) {
        m_CameraProjection = cameraProjection;
        m_CameraTransform = cameraTransform;
        recalculate_view_projection();
    }

    begin_circles();
    begin_lines();
}

void DebugRenderer::end() {
    flush_circles();
    flush_lines();
}

void DebugRenderer::begin_circles() {
    m_CircleVertexArrayData.clear();
}

void DebugRenderer::begin_lines() {
    m_LineVertexArrayData.clear();
}

void DebugRenderer::flush_circles() {
    if (m_CircleVertexArrayData.empty())
        return;

    uint32_t indexCount = m_CircleVertexArrayData.size() / 4 * 6;
    uint32_t dataSize = m_CircleVertexArrayData.size() * sizeof(CircleVertex);

    m_CircleVertexBuffer->set_data(m_CircleVertexArrayData.data(), dataSize);

    m_CircleShader->bind();
    m_CircleShader->set_mat_4("u_ViewProjection", m_ViewProjectionMatrix);
    api::draw_indexed(m_CircleVertexArray, indexCount);
}

void DebugRenderer::flush_lines() {
    if (m_LineVertexArrayData.empty())
        return;

    uint32_t indexCount = m_LineVertexArrayData.size();
    uint32_t dataSize = m_LineVertexArrayData.size() * sizeof(LineVertex);

    m_LineVertexBuffer->set_data(m_LineVertexArrayData.data(), dataSize);

    m_LineShader->bind();
    m_LineShader->set_mat_4("u_ViewProjection", m_ViewProjectionMatrix);
    api::draw_lines(m_LineVertexArray, indexCount);
}

void DebugRenderer::draw_circle(const math::mat4& transform,
                                const math::vec4& color,
                                float thickness,
                                float fade,
                                uint32_t entityId) {
    if (m_CircleVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_circles();
        begin_circles();
    }

    for (size_t i = 0; i < 4; i++) {
        CircleVertex cv{transform * m_QuadVertexPositions[i],
                        m_QuadVertexPositions[i] * 2.0f,
                        color,
                        thickness,
                        fade,
                        entityId};

        m_CircleVertexArrayData.push_back(cv);
    }
}

void DebugRenderer::draw_line(const math::vec3& p1,
                              const math::vec3& p2,
                              const math::vec4& color,
                              uint32_t entityId) {
    if (m_LineVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_lines();
        begin_lines();
    }

    LineVertex lv{p1, color, entityId};
    m_LineVertexArrayData.push_back(lv);

    lv = LineVertex{p2, color, entityId};
    m_LineVertexArrayData.push_back(lv);
}

void DebugRenderer::draw_rect(const math::vec3& position,
                              const math::vec2& size,
                              const math::vec4& color,
                              uint32_t entityId) {
    math::vec3 p0 = math::vec3(position.x - size.x * 0.5f,
                               position.y - size.y * 0.5f, position.z);
    math::vec3 p1 = math::vec3(position.x + size.x * 0.5f,
                               position.y - size.y * 0.5f, position.z);
    math::vec3 p2 = math::vec3(position.x + size.x * 0.5f,
                               position.y + size.y * 0.5f, position.z);
    math::vec3 p3 = math::vec3(position.x - size.x * 0.5f,
                               position.y + size.y * 0.5f, position.z);

    draw_line(p0, p1, color, entityId);
    draw_line(p1, p2, color, entityId);
    draw_line(p2, p3, color, entityId);
    draw_line(p3, p0, color, entityId);
}

void DebugRenderer::draw_rect(const math::mat4& transform,
                              const math::vec4& color,
                              uint32_t entityId) {
    math::vec3 lineVertices[4];
    for (size_t i = 0; i < 4; i++)
        lineVertices[i] = transform * m_QuadVertexPositions[i];

    draw_line(lineVertices[0], lineVertices[1], color, entityId);
    draw_line(lineVertices[1], lineVertices[2], color, entityId);
    draw_line(lineVertices[2], lineVertices[3], color, entityId);
    draw_line(lineVertices[3], lineVertices[0], color, entityId);
}

void DebugRenderer::generate_circle_va() {
    m_CircleVertexArray = std::make_shared<VertexArray>();

    m_CircleVertexBuffer =
        std::make_shared<VertexBuffer>(MAX_VERTICES * sizeof(CircleVertex));
    BufferLayout layout = {{ShaderDataType::Float3, "a_Position"},
                           {ShaderDataType::Float3, "a_UVPosition"},
                           {ShaderDataType::Float4, "a_Color"},
                           {ShaderDataType::Float, "a_Thickness"},
                           {ShaderDataType::Float, "a_Fade"},
                           {ShaderDataType::Int, "a_EntityId"}};
    m_CircleVertexBuffer->set_layout(layout);
    m_CircleVertexArray->add_vertex_buffer(m_CircleVertexBuffer);
    m_CircleVertexArray->set_index_buffer(m_QuadIndexBuffer);
}

void DebugRenderer::generate_line_va() {
    m_LineVertexArray = std::make_shared<VertexArray>();

    m_LineVertexBuffer =
        std::make_shared<VertexBuffer>(MAX_VERTICES * sizeof(LineVertex));
    BufferLayout layout = {{ShaderDataType::Float3, "a_Position"},
                           {ShaderDataType::Float4, "a_Color"},
                           {ShaderDataType::Int, "a_EntityID"}};
    m_LineVertexBuffer->set_layout(layout);
    m_LineVertexArray->add_vertex_buffer(m_LineVertexBuffer);
}

}  // namespace rr::graphics
