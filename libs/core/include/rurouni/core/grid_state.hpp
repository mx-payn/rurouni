#ifndef RR_LIBS_CORE_GRID_STATE_H
#define RR_LIBS_CORE_GRID_STATE_H

#include "rurouni/math/vec.hpp"

namespace rr::core {

struct GridState {
    math::ivec2 CellCount;
    math::ivec2 CellSize_px;
};

}  // namespace rr::core

#endif  // !RR_LIBS_CORE_GRID_STATE_H
