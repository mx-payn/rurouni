#ifndef RR_LIB_GRAPHICS_CHARSETS_H
#define RR_LIB_GRAPHICS_CHARSETS_H

#include <cstdint>
#include <vector>

namespace rr::graphics {

// from imgui_draw.cpp GetGlyphRangesDefault()
static const std::vector<uint32_t> CHARSET_BASIC_LATIN = {0x0020, 0x00FF};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_CHARSETS_H
