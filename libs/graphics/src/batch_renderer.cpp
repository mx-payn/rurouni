// pch
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/default/shader.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

BatchRenderer::BatchRenderer() {
    m_QuadShader = std::make_shared<Shader>(DEFAULT_QUAD_SHADER_VERTEX_SRC,
                                            DEFAULT_QUAD_SHADER_FRAGMENT_SRC);

    // init vertex positions
    m_QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    m_QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

    // TODO this is duplicate code
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
    generate_quad_va();

    // init white texture
    DataTextureSpecification spec;
    spec.size = math::ivec2{1, 1};
    spec.dataFormat = TextureDataFormat::RGBA;
    spec.pixelFormat = TexturePixelFormat::RGBA8;
    m_WhiteTexture = std::make_shared<Texture>(spec);
    uint32_t whiteTextureData = 0xFFFFFFFF;
    m_WhiteTexture->set_data(&whiteTextureData, sizeof(uint32_t));
    m_TextureSlots[0] = m_WhiteTexture;
}

BatchRenderer::BatchRenderer(std::shared_ptr<Shader> quadShader)
    : m_QuadShader(quadShader) {
    memset(m_TextureSlots.data(), 0, m_TextureSlots.size() * sizeof(uint32_t));

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
    generate_quad_va();

    // init white texture
    DataTextureSpecification spec;
    spec.size = math::ivec2{1, 1};
    spec.dataFormat = TextureDataFormat::RGBA;
    spec.pixelFormat = TexturePixelFormat::RGBA8;
    m_WhiteTexture = std::make_shared<Texture>(spec);
    uint32_t whiteTextureData = 0xFFFFFFFF;
    m_WhiteTexture->set_data(&whiteTextureData, sizeof(uint32_t));
    m_TextureSlots[0] = m_WhiteTexture;
}

BatchRenderer::~BatchRenderer() {}

void BatchRenderer::recalculate_view_projection() {
    m_ViewProjectionMatrix =
        m_CameraProjection * math::inverse(m_CameraTransform);
}

void BatchRenderer::begin(const math::mat4& viewProjectionMatrix) {
    m_ViewProjectionMatrix = viewProjectionMatrix;
    begin_quads();
}

void BatchRenderer::begin(const math::mat4& cameraProjection,
                          const math::mat4& cameraTransform) {
    if (m_CameraProjection != cameraProjection ||
        m_CameraTransform != cameraTransform) {
        m_CameraProjection = cameraProjection;
        m_CameraTransform = cameraTransform;
        recalculate_view_projection();
    }

    begin_quads();
}

void BatchRenderer::end() {
    flush_quads();
}

void BatchRenderer::begin_quads() {
    m_QuadVertexArrayData.clear();
    m_TextureSlotIndex = 1;
}

void BatchRenderer::flush_quads() {
    if (m_QuadVertexArrayData.empty())
        return;

    uint32_t indexCount = m_QuadVertexArrayData.size() / 4 * 6;
    uint32_t dataSize = m_QuadVertexArrayData.size() * sizeof(QuadVertex);

    m_QuadVertexBuffer->set_data(m_QuadVertexArrayData.data(), dataSize);

    for (uint32_t i = 0; i < m_TextureSlotIndex; i++) {
        auto slot = m_TextureSlots[i].lock();
        require(slot,
                "encountered invalid weak_ptr to texture! asset manager "
                "seems to unload textures before they get drawn");
        require(slot->get_renderer_id().has_value(),
                "texture slot has can not have no value.");
        slot->bind(i);
    }

    m_QuadShader->bind();
    m_QuadShader->set_mat_4("u_ViewProjection", m_ViewProjectionMatrix);
    api::draw_indexed(m_QuadVertexArray, indexCount);
}

void BatchRenderer::draw_quad(const math::mat4& transform,
                              const math::vec4& color,
                              uint32_t entityId) {
    if (m_QuadVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_quads();
        begin_quads();
    }

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         DEFAULT_UV_TEXTURE_COORDS[i], 0.0f, entityId};

        m_QuadVertexArrayData.push_back(qv);
    }
}

void BatchRenderer::draw_texture(const math::mat4& transform,
                                 std::weak_ptr<Texture> texture,
                                 math::vec4 color,
                                 uint32_t entityId) {
    // if the texture is expired
    if (texture.expired()) {
        error("texture is not valid any more. drawing ugly replacement");
        draw_quad(transform, math::vec4{1.0f, 0.0f, 1.0f, 1.0f}, entityId);
        return;
    }

    if (m_QuadVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_quads();
        begin_quads();
    }

    float textureIndex = get_texture_index(texture);

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         texture.lock()->get_uv_coords()[i], textureIndex,
                         entityId};

        m_QuadVertexArrayData.push_back(qv);
    }
}

void BatchRenderer::generate_quad_va() {
    m_QuadVertexArray = std::make_shared<VertexArray>();

    m_QuadVertexBuffer =
        std::make_shared<VertexBuffer>(MAX_VERTICES * sizeof(QuadVertex));
    m_QuadVertexBuffer->set_layout(QUAD_SHADER_BUFFER_LAYOUT);
    m_QuadVertexArray->add_vertex_buffer(m_QuadVertexBuffer);
    m_QuadVertexArray->set_index_buffer(m_QuadIndexBuffer);

    int32_t samplers[MAX_TEXTURE_SLOTS];
    for (uint32_t i = 0; i < MAX_TEXTURE_SLOTS; i++)
        samplers[i] = i;

    m_QuadShader->bind();
    m_QuadShader->set_int_array("u_Textures", samplers, MAX_TEXTURE_SLOTS);
}

float BatchRenderer::get_texture_index(std::weak_ptr<Texture> texture) {
    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < m_TextureSlotIndex; i++) {
        auto slot = m_TextureSlots[i].lock();
        require(slot,
                "encountered invalid weak_ptr to texture! asset manager "
                "seems to unload textures before they get drawn");

        if (slot->get_uuid() == texture.lock()->get_uuid()) {
            textureIndex = (float)i;
            break;
        }
    }

    // texture already has slot
    if (textureIndex != 0.0f) {
        return textureIndex;
    }

    // new texture, but no more slots available
    if (m_TextureSlots.size() == MAX_TEXTURE_SLOTS) {
        flush_quads();
        begin_quads();
    }

    textureIndex = (float)m_TextureSlotIndex;
    m_TextureSlots[m_TextureSlotIndex] = texture;
    m_TextureSlotIndex++;

    return textureIndex;
}

}  // namespace rr::graphics
