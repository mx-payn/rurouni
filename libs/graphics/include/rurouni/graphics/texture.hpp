#ifndef RR_LIB_GRAPHICS_TEXTURE_H
#define RR_LIB_GRAPHICS_TEXTURE_H

// rurouni
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// std
#include <exception>

namespace rr::graphics {

// TODO exceptions for other construcors as well
//      and put them in a separate header
struct TextureException : public std::exception {
    const char* what() const throw() { return "texture failure, check log"; }
};

enum class ImageFormat { None = 0, R8, RGB8, RGBA8, RGBA32F };

struct TextureSpecification {
    math::ivec2 size{1};
    ImageFormat format = ImageFormat::RGBA8;
    bool generateMips = true;
};

/** \brief Abstract texture class providing GraphicsAPI dependent texture
 * functionalities.
 *
 *  Abstract class to hold GraphicsAPI independet Data and providing an
 * interface on GraphicsAPI dependent functions.
 *
 *  At the moment Spritesheets are also considered textures, which forces single
 *  textures to hold data only relevant for spritesheets, like subtextures or
 * cell size.
 */
class Texture {
   public:
    /** Creates texture from path
     *
     * \param[in] path The absolute path of the image file.
     */
    Texture(const system::Path& path, const UUID& uuid = UUID());

    /** Creates empty texture of given size
     *
     * Data can then be set via set_data() function
     */
    Texture(const math::ivec2& size, const UUID& uuid = UUID());

    Texture(const TextureSpecification& spec, const UUID& uuid = UUID());

    ~Texture();

    /** GPU call to Bind / Select a Texture for further processing.
     *  Textures have to be bound before data can be accessed on the GPU.
     *
     *  The slot defines the sample slot to be used for texture access by the
     *  batch renderer. A texture bound to slot 3 can be accessed by multiple
     *  QuadVertices. So the needed textures are bound once and Drawcalls
     * forward only the slot of the texture with other data to the GPU / Shader.
     *
     *  \param[in] slot The texture slot on the GPU to bind the texture to
     */
    void bind(uint32_t slot = 0) const;

    /** Uploads the given data to the texture location on the GPU.
     *  Can be used to first create a Texture with no data and dynamically
     *  setting the data during runtime.
     *  Textures must be bound before uploading data to the GPU!
     *
     *  \param[in] data The data to be sent to GPU memory at Bound texture
     * address. \param[in] size The size in bytes of the data to upload.
     */
    void set_data(void* data, uint32_t size);

    /** Retrieves the unique identification of texture data stored on the GPU.
     *  This is NOT equal to the UUID used by the program, but only a GPU
     *  generated identifier for each texture location on GPU.
     *  In program-space, the RendererID is used to access data on GPU memory,
     * UUID to compare textures on CPU accessed memory.
     *
     *  \returns The unique identifier to access texture data in GPU memory.
     */
    uint32_t get_renderer_id() const { return m_RendererID; }

    /** \returns The texture size in pixel. */
    const math::ivec2& get_size() const { return m_Size; }
    /** \returns The textures absolute path on disk. */
    const std::filesystem::path& get_path() const { return m_Path; }
    const UUID& get_uuid() const { return m_UUID; }

   private:
    math::ivec2 m_Size = {0.0f, 0.0f};  //!< The texture size in pixel
    system::Path m_Path = "";           //!< The absolute path of the image
    UUID m_UUID = UUID();

    uint32_t m_RendererID;  //!< The unique identifier for data in GPU memory.
    uint32_t m_InternalFormat;  //!< Data format of the image file.
    uint32_t m_DataFormat;      //!< Data format of the image file.
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_TEXTURE_H
