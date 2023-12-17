#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// editor
#include "rurouni/editor/ui.hpp"
#include "rurouni/editor/ui_modals/module_create.hpp"

// rurouni
#include "rurouni/core/module.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace rr::editor {

class Editor : public event::Subscriber {
   public:
    Editor(const graphics::WindowSpecification& windowSpec);
    ~Editor();

    void run();

    virtual void on_event(std::shared_ptr<event::Event> event) override;

   private:
    void update(float dt);
    void render();

    void on_window_close_event(std::shared_ptr<event::WindowClose> event);
    void on_window_framebuffer_resize(
        std::shared_ptr<event::WindowFramebufferResize> event);
    void on_application_close_event(
        std::shared_ptr<event::ApplicationClose> event);

   private:
    void create_module(const system::Path& path, const std::string& name);
    void import_module(const system::Path& path, const std::string& name);
    void open_module(const UUID& id);

    void write_module_history();

   private:
    // editor state
    bool m_Running;
    float m_DeltaTime;
    std::unordered_map<UUID, ModuleHistoryItem> m_ModuleHistory;

    // module state
    UIState m_UIState;
    std::optional<core::Module> m_CurrentModule;
    std::vector<std::unique_ptr<core::Scene>> m_CurrentScenes;
    std::unique_ptr<core::Scene> m_NextScene;

    // systems
    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<event::EventSystem> m_EventSystem;

    // modals
    std::unique_ptr<ui::ModuleCreateModal> m_ModuleCreateModal;

    // editor paths
    std::string m_AppName;
    system::Path m_ExecPath;
    system::Path m_SharedDataDir;
    system::Path m_SharedConfigDir;
    system::Path m_UserDataDir;
    system::Path m_UserConfigDir;
};

}  // namespace rr::editor

#endif  // !RR_EDITOR_H
