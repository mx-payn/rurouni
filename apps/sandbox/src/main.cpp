// editor
#include "rurouni/sandbox.hpp"

// rurouni
#include "rurouni/graphics/window.hpp"

// std
#include <memory>

int main() {
    rr::graphics::WindowSpecification windowSpec;
    windowSpec.Title = "Rurouni Sandbox";
    windowSpec.Size = {1920, 1080};
    windowSpec.Floating = false;
    windowSpec.Maximized = false;
    windowSpec.Resizable = true;
    windowSpec.Fullscreen = false;
    windowSpec.VSync = true;

    rr::Sandbox sandbox(windowSpec);
    sandbox.run();

    return 0;
}
