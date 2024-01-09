// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/batch_renderer.hpp"
#include "rurouni/graphics/default/shader.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"

namespace rr::graphics {

BatchRenderer::BatchRenderer()
    : BatchRenderer(
          std::make_shared<Shader>(DEFAULT_QUAD_SHADER_VERTEX_SRC,
                                   DEFAULT_QUAD_SHADER_FRAGMENT_SRC)) {}

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
    DataTextureSpecification whiteSpec;
    whiteSpec.Id.generate();
    whiteSpec.Size = math::ivec2{1, 1};
    whiteSpec.DataFormat = TextureDataFormat::RGBA;
    whiteSpec.PixelFormat = TexturePixelFormat::RGBA8;
    uint32_t whiteTextureData = 0xFFFFFFFF;
    whiteSpec.set_data(&whiteTextureData, sizeof(whiteTextureData));
    m_WhiteTexture = std::make_shared<Texture>(whiteSpec);

    // init rect texture
    graphics::DataTextureSpecification rectSpec;
    rectSpec.Id.generate();
    rectSpec.DataFormat = graphics::TextureDataFormat::RGBA;
    rectSpec.PixelFormat = graphics::TexturePixelFormat::RGBA8;
    rectSpec.Size = math::ivec2{32};
    std::vector<uint32_t> rectData;
    for (int y = 0; y < rectSpec.Size.y; y++) {
        for (int x = 0; x < rectSpec.Size.x; x++) {
            if (y == 0 || y == rectSpec.Size.y - 1 || x == 0 ||
                x == rectSpec.Size.x - 1) {
                rectData.push_back(0xFFFFFFFF);
            } else {
                rectData.push_back(0x00000000);
            }
        }
    }
    rectSpec.set_data(rectData.data(), rectData.size() * sizeof(uint32_t));
    m_RectTexture = std::make_unique<graphics::Texture>(rectSpec);
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
    m_TextureSlotIndex = 0;
}

void BatchRenderer::flush_quads() {
    if (m_QuadVertexArrayData.empty())
        return;

    uint32_t indexCount = m_QuadVertexArrayData.size() / 4 * 6;
    uint32_t dataSize = m_QuadVertexArrayData.size() * sizeof(QuadVertex);

    m_QuadVertexBuffer->set_data(m_QuadVertexArrayData.data(), dataSize);

    for (uint32_t i = 0; i < m_TextureSlotIndex; i++) {
        auto texturePtr = m_TextureSlots[i].lock();

        if (texturePtr == nullptr) {
            error(
                "encountered invalid weak_ptr to texture! asset manager "
                "seems to unload textures before they get drawn.");
            m_WhiteTexture->bind(i);
            continue;
        }

        // if (!texturePtr->get_renderer_id().has_value()) {
        //     error(
        //         "encountered a texture without renderer id. maybe its atlas "
        //         "handle is invalid.");
        //     m_WhiteTexture->bind(i);
        //     continue;
        // }

        texturePtr->bind(i);
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

    float textureIndex = get_texture_index(m_WhiteTexture);

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         DEFAULT_UV_TEXTURE_COORDS[i], textureIndex, entityId};

        m_QuadVertexArrayData.push_back(qv);
    }
}

void BatchRenderer::draw_rect(const math::mat4& transform,
                              const math::vec4& color,
                              uint32_t entityId) {
    if (m_QuadVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_quads();
        begin_quads();
    }

    float textureIndex = get_texture_index(m_RectTexture);

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         DEFAULT_UV_TEXTURE_COORDS[i], textureIndex, entityId};

        m_QuadVertexArrayData.push_back(qv);
    }
}

void BatchRenderer::draw_texture(const math::mat4& transform,
                                 std::weak_ptr<Texture> texture,
                                 uint32_t entityId,
                                 math::vec4 color) {
    if (m_QuadVertexArrayData.size() >= MAX_VERTICES) {
        // start new batch
        flush_quads();
        begin_quads();
    }

    float textureIndex = get_texture_index(texture);

    for (size_t i = 0; i < 4; i++) {
        QuadVertex qv = {transform * m_QuadVertexPositions[i], color,
                         texture.lock()->get_texture_coords()[i], textureIndex,
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
    for (uint32_t i = 0; i < m_TextureSlotIndex; i++) {
        auto slot = m_TextureSlots[i].lock();

        require(slot,
                "encountered invalid weak_ptr to texture! asset manager "
                "seems to unload textures before they get drawn");

        if (slot->get_id() == texture.lock()->get_id()) {
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
