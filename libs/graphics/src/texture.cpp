// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// external
#include <glad/gl.h>
// NOTE: emmintrin.h drops a strange error in debug builds
// so no simd for now. Seems to not be an issue on release builds,
// when problematic function propably gets inlined.
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace rr::graphics {

Texture::Texture(const DataTextureSpecification& spec) {
    m_Size = spec.size;
    m_UUID = spec.id;
    m_PixelFormat = spec.pixelFormat;
    m_DataFormat = spec.dataFormat;

    if (m_UUID.is_null())
        m_UUID.generate();

    uint32_t rendererId;
    glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);
    glTextureStorage2D(rendererId, 1, static_cast<uint32_t>(m_PixelFormat),
                       m_Size.x, m_Size.y);

    glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_RendererID = rendererId;
}

Texture::Texture(const system::Path& path)
    : Texture(ImageTextureSpecification{path, path.filename().string()}) {}

Texture::Texture(const ImageTextureSpecification& spec) {
    m_UUID = spec.id;

    if (m_UUID.is_null())
        m_UUID.generate();

    // -- loading image data using stbi --
    int width, height, channels;
    // image is upside down without flip
    // stbi_set_flip_vertically_on_load(1);
    stbi_uc* data =
        stbi_load(spec.Filepath.c_str(), &width, &height, &channels, 0);
    trace("loaded image_data: {} . channels: {}", spec.Filepath, channels);
    require(data != nullptr, "Failed to load image!");
    if (!data) {
        error("failed loading image! path: '{}', uuid: '{}'", spec.Filepath,
              m_UUID);
        throw TextureException();
    }

    m_Size = {width, height};

    // -- image data format parsing --
    if (channels == 4) {
        m_PixelFormat = TexturePixelFormat::RGBA8;
        m_DataFormat = TextureDataFormat::RGBA;
    } else if (channels == 3) {
        m_PixelFormat = TexturePixelFormat::RGB8;
        m_DataFormat = TextureDataFormat::RGB;
    } else if (channels == 1) {
        m_PixelFormat = TexturePixelFormat::R8;
        m_DataFormat = TextureDataFormat::RED;
    }

    if (!((uint32_t)m_DataFormat & (uint32_t)m_PixelFormat)) {
        error(
            "the texture format of the loaded image is not supported! path: "
            "'{}', uuid: '{}'",
            spec.Filepath, m_UUID);
        throw TextureException();
    }

    m_RendererID = 0;

    // -- uploading image data to GPU memory --
    // reserving memory space on GPU and getting unique identifier for space
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID.value());
    glTextureStorage2D(m_RendererID.value(), 1,
                       static_cast<uint32_t>(m_PixelFormat), m_Size.x,
                       m_Size.y);

    // setting texture parameters for rendering
    glTextureParameteri(m_RendererID.value(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID.value(), GL_TEXTURE_MAG_FILTER,
                        GL_NEAREST);
    glTextureParameteri(m_RendererID.value(), GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID.value(), GL_TEXTURE_WRAP_T, GL_REPEAT);

    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererID.value(), 0, 0, 0, m_Size.x, m_Size.y,
                        static_cast<uint32_t>(m_DataFormat), GL_UNSIGNED_BYTE,
                        data);

    // image data not needed anymore
    stbi_image_free(data);
}

Texture::~Texture() {
    if (m_RendererID.has_value()) {
        glDeleteTextures(1, &m_RendererID.value());
    }
}

// TODO checking the validity of renderer_id may be caller
//      responsibility to handle invalid textures with special
//      texture drawing
void Texture::bind(uint32_t slot) const {
    if (auto id = get_renderer_id()) {
        glBindTextureUnit(slot, id.value());
    } else {
        error("tried to bind an invalid texture");
    }
}

void Texture::set_data(void* data, uint32_t size) {
    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererID.value(), 0, 0, 0, m_Size.x, m_Size.y,
                        static_cast<uint32_t>(m_DataFormat), GL_UNSIGNED_BYTE,
                        data);
}

}  // namespace rr::graphics
