#ifndef RR_LIB_GRAPHICS_TEXTURE_H
#define RR_LIB_GRAPHICS_TEXTURE_H

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// std
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <typeinfo>
#include <vector>

namespace rr::graphics {

/** values from gl.h */
// TODO Maybe TextureDataType is better name?
enum class TexturePixelFormat : uint32_t { None = 0, R8, RGB8, RGBA8, RGBA32F };

/** values from gl.h */
enum class TextureDataFormat : uint32_t {
    None = 0,
    RED,
    RGB,
    RGBA,
};

constexpr uint32_t rrPixelFormatToGl(TexturePixelFormat format) {
    switch (format) {
        case TexturePixelFormat::R8:
            return 0x8229;
        case TexturePixelFormat::RGB8:
            return 0x8051;
        case TexturePixelFormat::RGBA8:
            return 0x8058;
        case TexturePixelFormat::RGBA32F:
            return 0x8814;
        default:
            require(false, "unhandled pixel format");
            return 0;
    }
}

constexpr uint32_t rrDataFormatToGl(TextureDataFormat format) {
    switch (format) {
        case TextureDataFormat::RED:
            return 0x1903;
        case TextureDataFormat::RGB:
            return 0x1907;
        case TextureDataFormat::RGBA:
            return 0x1908;
        default:
            require(false, "unhandled pixel format");
            return 0;
    }
}

enum class TextureDistanceFieldType {
    None,
    HardMask,
    SoftMask,
    SDF,
    PSDF,
    MSDF,
    MTSDF
};

struct DataTextureSpecification {
    std::string Name = std::string();
    UUID Id = UUID();
    math::ivec2 Size = math::ivec2(0.0f);
    TextureDataFormat DataFormat = TextureDataFormat::None;
    TexturePixelFormat PixelFormat = TexturePixelFormat::None;
    bool GenerateMips = true;
    TextureDistanceFieldType DistanceFieldType = TextureDistanceFieldType::None;

    ~DataTextureSpecification() {
        if (m_Data != nullptr)
            delete m_Data;
    }

    void set_data(void* data, size_t size) {
        m_Data = new uint8_t[size];
        std::memcpy(m_Data, data, size);
        m_DataSize = size;
    }
    const uint8_t* get_data() const { return m_Data; }
    const size_t get_data_size() const { return m_DataSize; }

   private:
    uint8_t* m_Data = nullptr;
    size_t m_DataSize = 0;
};

struct ImageTextureSpecification {
    std::string Name = std::string();
    UUID Id = UUID();
    system::Path Filepath = system::Path();
    TextureDistanceFieldType DistanceFieldType = TextureDistanceFieldType::None;
};

class Texture {
   public:
    Texture(const std::string& name, const UUID& id);
    Texture(const DataTextureSpecification& spec);
    Texture(const ImageTextureSpecification& spec);

    template <typename T>
    Texture(T*, const std::string& name, const UUID& id) : Texture(name, id) {
        m_TypeHash = typeid(T).hash_code();
    }
    template <typename T>
    Texture(T*, const DataTextureSpecification& spec) : Texture(spec) {
        m_TypeHash = typeid(T).hash_code();
    }
    template <typename T>
    Texture(T*, const ImageTextureSpecification& spec) : Texture(spec) {
        m_TypeHash = typeid(T).hash_code();
    }

    // TODO Bad design? Idk yet... definitely risky...
    //      Option(not good either...):
    //              make base destructor private and handle glDeleteTextures
    //              in each derived class, that owns the data.
    //      Option(rather annoying...):
    //              Sprites don't derive Texture and have to be handled with
    //              seperate renderer calls, asset cache and so on. Also each
    //              Font owns their own atlas and multiple fonts (like families)
    //              can not be batched into single atlas
    //      Option(sounds ok...):
    //              only base textures may own data, so all deriving classes
    //              may only hold references to a texture. get_renderer_id()
    //              becomes virtual, so a sprite returns its spritesheet's id,
    //              which again returns its texture's id. BUT: would introduce
    //              multiple vtable lookups for each renderer
    //                   draw call. Should not be a problem for this engines
    //                   performance goals, but people on the internet say
    //                   virtual functions are slow and evil...
    //
    /** IMPORTANT: if derived class is a pseudo texture that doesn't own the
     * data (e.g. sprite or Font) be sure to set m_RendererId = 0 in the
     * destructor, else the texture it references will be deleted! With id 0
     * opengl will silently ignore the glDeleteTextures call.
     */
    virtual ~Texture();

    void bind(uint32_t slot = 0) const;
    void set_data(const void* data, uint32_t size);
    uint32_t get_renderer_id() const { return m_RendererId; }

    /** unit: pixel */
    const math::ivec2& get_size() const { return m_Size; }
    const UUID& get_id() const { return m_Id; }
    const std::string& get_name() const { return m_Name; }
    const size_t get_type_hash() const { return m_TypeHash; }
    TextureDistanceFieldType get_distance_field_type() const {
        return m_DistanceFieldType;
    }
    /** unit: uv */
    const std::array<math::vec2, 4>& get_texture_coords() {
        return m_TextureCoords;
    }

   protected:
    std::string m_Name;
    UUID m_Id;

    size_t m_TypeHash = typeid(Texture).hash_code();
    uint32_t m_RendererId =
        0;  //!< The unique identifier for data in GPU memory.
    math::ivec2 m_Size = math::ivec2(0.0f);  //!< The texture size in pixel
    // TODO no need to store these?
    TextureDataFormat m_DataFormat = TextureDataFormat::None;
    TexturePixelFormat m_PixelFormat = TexturePixelFormat::None;
    TextureDistanceFieldType m_DistanceFieldType =
        TextureDistanceFieldType::None;

    std::array<math::vec2, 4> m_TextureCoords = {
        math::vec2{0.0f, 0.0f}, math::vec2{1.0f, 0.0f}, math::vec2{1.0f, 1.0f},
        math::vec2{0.0f, 1.0f}};
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_TEXTURE_H
