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
// TODO: emmintrin.h drops a strange error in debug builds
// so no simd for now. Seems to not be an issue on release builds,
// when problematic function propably gets inlined.
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

namespace rr::graphics {

Texture::Texture(const DataTextureSpecification& spec)
    : m_Name(spec.Name),
      m_Id(spec.Id),
      m_Size(spec.Size),
      m_PixelFormat(spec.PixelFormat),
      m_DataFormat(spec.DataFormat),
      m_DistanceFieldType(spec.DistanceFieldType) {
    if (m_Id.is_null())
        m_Id.generate();

    uint32_t rendererId;
    glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);
    glTextureStorage2D(rendererId, 1, rrPixelFormatToGl(m_PixelFormat),
                       m_Size.x, m_Size.y);

    glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_RendererId = rendererId;

    set_data(spec.get_data(), spec.get_data_size());
}

Texture::Texture(const std::string& name, const UUID& id)
    : m_Name(name), m_Id(id) {}

Texture::Texture(const ImageTextureSpecification& spec)
    : m_Name(spec.Name),
      m_Id(spec.Id),
      m_DistanceFieldType(spec.DistanceFieldType) {
    if (m_Id.is_null())
        m_Id.generate();

    // -- loading image data using stbi --
    int width, height, channels;
    // image is upside down without flip
    // we don't flip to have automatic y index 0 at top
    // stbi_set_flip_vertically_on_load(1);
    stbi_uc* data =
        stbi_load(spec.Filepath.c_str(), &width, &height, &channels, 0);
    trace("loaded image_data: {} . channels: {}", spec.Filepath, channels);
    if (!data) {
        error("failed loading image! path: '{}', uuid: '{}'", spec.Filepath,
              m_Id);
        require(data != nullptr, "Failed to load image!");

        std::string name = spec.Name;
        m_Name = name.append("##FAILED");
        m_Id = spec.Id;
        m_Size = math::ivec2{1, 1};
        m_DataFormat = TextureDataFormat::RGBA;
        m_PixelFormat = TexturePixelFormat::RGBA8;

        uint32_t rendererId;
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);
        glTextureStorage2D(rendererId, 1, rrPixelFormatToGl(m_PixelFormat),
                           m_Size.x, m_Size.y);

        glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

        m_RendererId = rendererId;

        uint32_t purpleTextureData = 0xFFFFFFFF;
        set_data(&purpleTextureData, sizeof(purpleTextureData));
        return;
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
            spec.Filepath, m_Id);

        DataTextureSpecification purpleSpec;
        std::string name = spec.Name;
        m_Name = name.append("##FAILED");
        m_Id = spec.Id;
        m_Size = math::ivec2{1, 1};
        m_DataFormat = TextureDataFormat::RGBA;
        m_PixelFormat = TexturePixelFormat::RGBA8;

        uint32_t rendererId;
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererId);
        glTextureStorage2D(rendererId, 1, rrPixelFormatToGl(m_PixelFormat),
                           m_Size.x, m_Size.y);

        glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

        m_RendererId = rendererId;

        uint32_t purpleTextureData = 0xFFFFFFFF;
        set_data(&purpleTextureData, sizeof(purpleTextureData));
        return;
    }

    m_RendererId = 0;

    // -- uploading image data to GPU memory --
    // reserving memory space on GPU and getting unique identifier for space
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
    glTextureStorage2D(m_RendererId, 1, rrPixelFormatToGl(m_PixelFormat),
                       m_Size.x, m_Size.y);

    // setting texture parameters for rendering
    glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Size.x, m_Size.y,
                        rrDataFormatToGl(m_DataFormat), GL_UNSIGNED_BYTE, data);

    // image data not needed anymore
    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_RendererId);
}

void Texture::bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_RendererId);
}

void Texture::set_data(const void* data, uint32_t size) {
    // uploading image data to reserved GPU memory space
    glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Size.x, m_Size.y,
                        rrDataFormatToGl(m_DataFormat), GL_UNSIGNED_BYTE, data);
}

}  // namespace rr::graphics
