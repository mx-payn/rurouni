#ifndef RR_LIB_GRAPHICS_MOUSE_CODES_H
#define RR_LIB_GRAPHICS_MOUSE_CODES_H

#include <cstdint>

namespace rr::input {

enum class MouseCodes {
    // From glfw3.h
    ButtonLeft = 0,
    ButtonRight = 1,
    ButtonMiddle = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Button6 = 6,
    ButtonLast = 7
};

}

#endif  // !RR_LIB_GRAPHICS_MOUSE_CODES_H
