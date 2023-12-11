// editor
#include "rurouni/editor.hpp"

// rurouni
#include "rurouni/graphics/window.hpp"

// std
#include <memory>

int main() {
    rr::graphics::WindowSpecification windowSpec;
    windowSpec.Title = "Rurouni Editor";
    windowSpec.Size = {1920, 1080};
    windowSpec.Floating = false;
    windowSpec.Maximized = false;
    windowSpec.Resizable = true;
    windowSpec.Fullscreen = false;
    windowSpec.VSync = true;

    rr::Editor editor(windowSpec);
    editor.run();

    return 0;
}
