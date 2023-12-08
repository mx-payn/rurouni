// pch
#include "rurouni/graphics/shader.hpp"
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "default/shader.hpp"
#include "rurouni/dev/logger.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

BatchRenderer::BatchRenderer() {
    auto quadShader = std::make_shared<Shader>(
        DEFAULT_QUAD_SHADER_VERTEX_SRC, DEFAULT_QUAD_SHADER_FRAGMENT_SRC);
    auto textShader = std::make_shared<Shader>(
        DEFAULT_TEXT_SHADER_VERTEX_SRC, DEFAULT_TEXT_SHADER_FRAGMENT_SRC);

    BatchRenderer(quadShader, textShader);
}

BatchRenderer::BatchRenderer(std::shared_ptr<Shader> quadShader,
                             std::shared_ptr<Shader> textShader)
    : m_QuadShader(quadShader), m_TextShader(textShader) {
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
    generate_text_va();

    // init white texture
    m_WhiteTexture = std::make_shared<Texture>(math::ivec2{1, 1});
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
    begin_text();
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
    begin_text();
}

void BatchRenderer::end() {
    flush_quads();
    flush_text();
}

void BatchRenderer::begin_quads() {
    m_QuadVertexArrayData.clear();
    m_TextureSlotIndex = 1;
}

void BatchRenderer::begin_text() {
    m_TextVertexArrayData.clear();
    m_FontSlotIndex = 0;
}

void BatchRenderer::flush_quads() {
    if (m_QuadVertexArrayData.empty())
        return;

    uint32_t indexCount = m_QuadVertexArrayData.size() / 4 * 6;
    uint32_t dataSize = m_QuadVertexArrayData.size() * sizeof(QuadVertex);

    m_QuadVertexBuffer->set_data(m_QuadVertexArrayData.data(), dataSize);

    for (uint32_t i = 0; i < m_TextureSlotIndex; i++) {
        auto slot = m_TextureSlots[i].lock();
        dev::LOG->require(
            slot,
            "encountered invalid weak_ptr to texture! asset manager "
            "seems to unload textures before they get drawn");
        slot->bind(i);
    }

    m_QuadShader->bind();
    m_QuadShader->set_mat_4("u_ViewProjection", m_ViewProjectionMatrix);
    api::draw_indexed(m_QuadVertexArray, indexCount);
}

void BatchRenderer::flush_text() {
    if (m_TextVertexArrayData.empty())
        return;

    uint32_t indexCount = m_TextVertexArrayData.size() / 4 * 6;
    uint32_t dataSize = m_TextVertexArrayData.size() * sizeof(TextVertex);

    m_TextVertexBuffer->set_data(m_TextVertexArrayData.data(), dataSize);

    // m_FontAtlas.lock()->bind(0);
    for (uint32_t i = 0; i < m_FontSlotIndex; i++) {
        auto slot = m_FontSlots[i].lock();
        dev::LOG->require(slot,
                          "encountered invalid weak_ptr to font! asset manager "
                          "seems to unload fonts before they get drawn");
        slot->get_atlas_texture().lock()->bind(i + FONT_SLOTS_BEGIN_IDX);
    }

    m_TextShader->bind();
    m_TextShader->set_mat_4("u_ViewProjection", m_ViewProjectionMatrix);
    api::draw_indexed(m_TextVertexArray, indexCount);
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
                                 uint32_t entityId,
                                 const std::array<math::vec2, 4> uvTexCoords) {
    // if the texture is expired
    if (texture.expired()) {
        dev::LOG->error(
            "texture is not valid any more. drawing ugly replacement");
        draw_quad(transform, math::vec4{1.0f, 0.0f, 1.0f, 1.0f}, entityId);
        return;
    }

    if (m_QuadVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_quads();
        begin_quads();
    }

    constexpr math::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    float textureIndex = get_texture_index(texture);

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         uvTexCoords[i], textureIndex, entityId};

        m_QuadVertexArrayData.push_back(qv);
    }
}

void BatchRenderer::draw_text(const math::mat4& transform,
                              const Text& text,
                              const math::vec4& color) {
    // if the texture is expired
    if (text.get_font().expired()) {
        dev::LOG->error(
            "font atlas is not valid any more. drawing ugly replacement");
        draw_quad(transform, math::vec4{1.0f, 0.0f, 1.0f, 1.0f}, 0);
        return;
    }

    if (m_TextVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_text();
        begin_text();
    }

    // m_FontAtlas = text.get_font_atlas();
    float textureIndex = get_font_index(text.get_font());

    for (const auto& character : text.get_characters()) {
        for (uint32_t i = 0; i < 4; i++) {
            TextVertex tv = {
                transform *
                    math::vec4(character.quadPosition_uv[i], 1.0f, 1.0f),
                color, character.textureCoords_uv[i], textureIndex, 0};
            m_TextVertexArrayData.push_back(tv);
        }
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

void BatchRenderer::generate_text_va() {
    m_TextVertexArray = std::make_shared<VertexArray>();

    m_TextVertexBuffer =
        std::make_shared<VertexBuffer>(MAX_VERTICES * sizeof(TextVertex));
    m_TextVertexBuffer->set_layout(TEXT_SHADER_BUFFER_LAYOUT);
    m_TextVertexArray->add_vertex_buffer(m_TextVertexBuffer);
    m_TextVertexArray->set_index_buffer(m_QuadIndexBuffer);

    int32_t samplers[MAX_FONT_SLOTS];
    for (uint32_t i = 0; i < MAX_FONT_SLOTS; i++)
        samplers[i] = i + FONT_SLOTS_BEGIN_IDX;

    m_TextShader->bind();
    m_TextShader->set_int_array("u_Textures", samplers, MAX_FONT_SLOTS);
}

float BatchRenderer::get_texture_index(std::weak_ptr<Texture> texture) {
    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < m_TextureSlotIndex; i++) {
        auto slot = m_TextureSlots[i].lock();
        dev::LOG->require(
            slot,
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

float BatchRenderer::get_font_index(std::weak_ptr<Font> font) {
    float fontIndex = -1.0f;

    for (uint32_t i = 0; i < m_FontSlotIndex; i++) {
        auto slot = m_FontSlots[i].lock();
        dev::LOG->require(slot,
                          "encountered invalid weak_ptr to font! asset manager "
                          "seems to unload fonts before they get drawn");

        if (slot->get_uuid() == font.lock()->get_uuid()) {
            fontIndex = (float)i;
            break;
        }
    }

    // texture already has slot
    if (fontIndex != -1.0f) {
        return fontIndex;
    }

    // new texture, but no more slots available
    if (m_FontSlots.size() == MAX_FONT_SLOTS) {
        flush_quads();
        begin_quads();
    }

    fontIndex = (float)m_FontSlotIndex;
    m_FontSlots[m_FontSlotIndex] = font;
    m_FontSlotIndex++;

    return fontIndex;
}

}  // namespace rr::graphics
