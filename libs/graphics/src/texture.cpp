// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/dev/logger.hpp"
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

namespace utils {

static GLenum image_format_to_gl_data_format(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGB8:
            return GL_RGB;
        case ImageFormat::RGBA8:
            return GL_RGBA;
        default:
            dev::LOG->require(false, "unhandled data format");
    }

    return 0;
}

static GLenum image_format_to_gl_internal_format(ImageFormat format) {
    switch (format) {
        case ImageFormat::RGB8:
            return GL_RGB8;
        case ImageFormat::RGBA8:
            return GL_RGBA8;
        default:
            dev::LOG->require(false, "unhandled data format");
    }

    return 0;
}

}  // namespace utils

Texture::Texture(const TextureSpecification& spec, const UUID& uuid)
    : m_Size(spec.size), m_UUID(uuid) {
    if (m_UUID.is_null())
        m_UUID.generate();

    m_InternalFormat = utils::image_format_to_gl_internal_format(spec.format);
    m_DataFormat = utils::image_format_to_gl_data_format(spec.format);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Size.x, m_Size.y);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(const math::ivec2& size, const UUID& uuid)
    : m_Size(size), m_UUID(uuid) {
    if (m_UUID.is_null())
        m_UUID.generate();

    m_InternalFormat = GL_RGBA8;
    m_DataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Size.x, m_Size.y);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(const system::Path& path, const UUID& uuid)
    : m_Path(path), m_UUID(uuid) {
    if (m_UUID.is_null())
        m_UUID.generate();

    // -- loading image data using stbi --
    int width, height, channels;
    // image is upside down without flip
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    dev::LOG->trace("loaded image_data: {} . channels: {}", path.c_str(),
                    channels);
    dev::LOG->require(data != nullptr, "Failed to load image!");
    if (!data) {
        dev::LOG->error("failed loading image! path: '{}', uuid: '{}'", path,
                        m_UUID);
        throw TextureException();
    }

    m_Size = {width, height};

    // -- image data format parsing --
    GLenum internalFormat = 0, dataFormat = 0;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    }

    m_InternalFormat = internalFormat;
    m_DataFormat = dataFormat;

    if (!(internalFormat & dataFormat)) {
        dev::LOG->error(
            "the texture format of the loaded image is not supported! path: "
            "'{}', uuid: '{}'",
            path, m_UUID);
        throw TextureException();
    }

    // -- uploading image data to GPU memory --
    // reserving memory space on GPU and getting unique identifier for space
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Size.x, m_Size.y);

    // setting texture parameters for rendering
    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Size.x, m_Size.y, m_DataFormat,
                        GL_UNSIGNED_BYTE, data);

    // image data not needed anymore
    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_RendererID);
}

void Texture::bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_RendererID);
    // glActiveTexture(GL_TEXTURE0 + slot);
    // glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::set_data(void* data, uint32_t size) {
    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Size.x, m_Size.y, m_DataFormat,
                        GL_UNSIGNED_BYTE, data);
}

}  // namespace rr::graphics
