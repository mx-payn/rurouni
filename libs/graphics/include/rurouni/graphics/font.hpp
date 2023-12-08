#ifndef RR_LIB_GRAPHICS_FONT_H
#define RR_LIB_GRAPHICS_FONT_H

// rurouni
#include "rurouni/graphics/texture.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// std
#include <array>
#include <unordered_map>

namespace rr::graphics {

struct MSDFData;

// TODO documentation
/** reads freetype fonts and provides a texture atlas in msdf style */
class Font {
   public:
    Font(const system::Path& filepath, const UUID& uuid, double size = 40.0f);
    ~Font();

    std::weak_ptr<Texture> get_atlas_texture() const { return m_Texture; }
    std::unordered_map<char, std::array<math::vec2, 4>>&
    get_char_subtexture_positions() {
        return m_CharSubtexturePositions;
    }

    const UUID& get_uuid() const { return m_UUID; }
    const MSDFData* get_msdf_data() const { return m_Data; }

   private:
    system::Path m_Filepath;
    UUID m_UUID;

    MSDFData* m_Data;

    std::shared_ptr<Texture> m_Texture;
    std::unordered_map<char, std::array<math::vec2, 4>>
        m_CharSubtexturePositions;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_FONT_H
