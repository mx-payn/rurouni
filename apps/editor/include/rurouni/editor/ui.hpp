#ifndef RR_EDITOR_UI_H
#define RR_EDITOR_UI_H

#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"

#include <imgui/imgui.h>

namespace rr::editor::ui {

struct State {};

void init(graphics::Window& window,
          const system::Path& sharedDataDir,
          const system::Path& userDataDir,
          const system::Path& userConfigDir);
void terminate();
void set_display_size(const glm::ivec2 size);

void update(float dt, const system::Path& userConfigDir);
void begin();
void end();

void draw_dockspace(State& state);
// void draw_menu_bar(State& state);

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_H
