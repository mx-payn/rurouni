// runtime
#include "rurouni/runtime.hpp"

// rurouni
#include "rurouni/graphics/window.hpp"

// std
#include <memory>

int main() {
    rr::graphics::WindowSpecification windowSpec;
    windowSpec.Title = "Rurouni Runtime";
    windowSpec.Fullscreen = true;
    windowSpec.Floating = true;
    windowSpec.Maximized = false;
    windowSpec.Resizable = false;
    windowSpec.VSync = true;

    rr::Runtime runtime(windowSpec);
    runtime.run();

    return 0;
}
