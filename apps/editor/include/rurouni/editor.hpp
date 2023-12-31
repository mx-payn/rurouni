#ifndef RR_EDITOR_H
#define RR_EDITOR_H

// editor
#include "rurouni/editor/ui.hpp"

// rurouni
#include "rurouni/core/asset_manager.hpp"
#include "rurouni/core/module.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/event/application_event.hpp"
#include "rurouni/event/event.hpp"
#include "rurouni/event/event_system.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/batch_renderer.hpp"
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

   private:  // function callbacks
    void exit_application();

    bool create_module(const system::Path& path, const std::string& name);
    bool import_module(const system::Path& path);
    bool open_module(const UUID& id);

    bool read_module_history();
    bool write_module_history();
    bool cleanup_module_history();

    bool change_scene(const system::Path& path);
    bool save_scene(const system::Path& path);
    void draw_scene();

    bool asset_import_texture(core::TextureSpecification& spec);
    bool asset_import_sprites(
        std::unordered_map<int, core::SpriteSpecification>& specs);
    bool asset_import_shader(core::ShaderSpecification& spec);
    bool asset_import_font(ui::FontImportSpecification& fontInput,
                           ui::FontImportConfiguration& config);

   private:
    // editor state
    bool m_Running;
    float m_DeltaTime;
    std::unordered_map<UUID, ModuleHistoryItem> m_ModuleHistory;

    // module state
    std::optional<core::Module> m_CurrentModule;
    std::unique_ptr<core::Scene> m_CurrentScene;
    std::unique_ptr<core::Scene> m_NextScene;

    // systems
    std::shared_ptr<graphics::Window> m_Window;
    std::shared_ptr<event::EventSystem> m_EventSystem;
    std::shared_ptr<graphics::BatchRenderer> m_Renderer;
    std::shared_ptr<core::AssetManager> m_AssetManager;

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
