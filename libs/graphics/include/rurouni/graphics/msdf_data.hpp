#ifndef RR_LIB_GRAPHICS_MSDF_DATA_H
#define RR_LIB_GRAPHICS_MSDF_DATA_H

#undef INFINITE
#include <msdf-atlas-gen/msdf-atlas-gen.h>

#include <vector>

namespace rr::graphics {

struct MSDFData {
    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry fontGeometry;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_MSDF_DATA_H
