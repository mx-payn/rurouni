#ifndef RR_LIB_GRAPHICS_WINDOW_H
#define RR_LIB_GRAPHICS_WINDOW_H

#include "rurouni/event/event_system.hpp"
#include "rurouni/graphics/key_codes.hpp"
#include "rurouni/graphics/mouse_codes.hpp"
#include "rurouni/math.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

class GLFWwindow;

namespace rr::graphics {

struct WindowConfig {
    std::string Title = "undefined";

    std::optional<math::ivec2> Size;

    bool Fullscreen = true;

    bool Maximized = false;
    /** always-on-top flag */
    bool Floating = true;
    bool Resizable = false;

    bool VSync = true;
};

struct WindowData {
    std::string Title;
    // window
    math::ivec2 WindowPosition;
    math::ivec2 WindowSize;
    math::ivec2 FramebufferSize;
    math::vec2 ContentScale;
    float AspectRatio;
    bool VSync;
    bool Fullscreen;
    bool Maximized;
    bool Iconified;
    bool Focused;
    bool Floating;
    bool Resizable;

    // mouse
    math::vec2 MousePosition;
    float TimeSinceClick;
    bool TrackClickTime;
    uint32_t ClickCount;

    bool BlockInput = false;

    std::shared_ptr<event::EventSystem> EventSystem;

    math::vec2 get_mouse_framebuffer_position() const {
        math::vec2 position = MousePosition;
        position.x = static_cast<float>(FramebufferSize.x) - position.x;
        position.y = static_cast<float>(FramebufferSize.y) - position.y;
        return position;
    }

    math::vec2 get_mouse_framebuffer_percentage() const {
        math::vec2 position = MousePosition;
        position.x /= static_cast<float>(FramebufferSize.x);
        position.y /= static_cast<float>(FramebufferSize.y);
        return position;
    }

    std::string to_string() const {
        std::stringstream ss;

        ss << "title    : " << Title << "\n"
           << "position : " << WindowPosition.x << ":" << WindowPosition.y
           << "\n"
           << "size     : " << WindowSize.x << ":" << WindowSize.y << "\n"
           << "fbSize   : " << FramebufferSize.x << ":" << FramebufferSize.y
           << "\n"
           << "scale    : " << ContentScale.x << ":" << ContentScale.y << "\n"
           << "ar       : " << AspectRatio << "\n"
           << "vsync    : " << VSync << "\n"
           << "fullscr  : " << Fullscreen << "\n"
           << "maximized: " << Maximized << "\n"
           << "iconified: " << Iconified << "\n"
           << "focused  : " << Focused << "\n"
           << "floating : " << Floating << "\n"
           << "resizable: " << Resizable;

        return ss.str();
    }
};

class Window {
   public:
    Window(const WindowConfig& config,
           std::shared_ptr<event::EventSystem> eventSystem);
    ~Window();

    void update(float dt);
    void swap_buffers();

    void block_input(bool block);
    void set_vsync(bool enabled);
    // TODO
    // void set_fullscreen(bool enabled);
    // void set_window_size(const math::ivec2& size);

    bool is_key_pressed(input::KeyCodes key) const;
    bool is_button_pressed(input::MouseCodes button) const;

    bool is_minimized() const;
    bool is_iconified() const;
    bool is_focused() const;
    bool has_vsync() const;
    float get_aspect_ratio() const;
    const WindowData& get_window_data() const;

    GLFWwindow* get_native_window() const;
    GLFWwindow* get_current_context() const;
    void make_context_current(GLFWwindow* window);

   private:
    static void glfw_error_callback(int error, const char* description);
    static void window_size_callback(GLFWwindow* window, int width, int height);
    static void framebuffer_size_callback(GLFWwindow* window,
                                          int width,
                                          int height);
    static void window_pos_callback(GLFWwindow* window, int xpos, int ypos);
    static void window_focus_callback(GLFWwindow* window, int focused);
    static void window_close_callback(GLFWwindow* window);
    static void window_maximize_callback(GLFWwindow* window, int maximized);
    static void window_iconified_callback(GLFWwindow* window, int iconified);
    static void key_callback(GLFWwindow* window,
                             int key,
                             int scancode,
                             int action,
                             int mods);
    static void char_callback(GLFWwindow* window, unsigned int keycode);
    static void mouse_button_callback(GLFWwindow* window,
                                      int button,
                                      int action,
                                      int mods);
    static void scroll_callback(GLFWwindow* window,
                                double xoffset,
                                double yoffset);
    static void cursor_position_callback(GLFWwindow* window,
                                         double xpos,
                                         double ypos);
    static void content_scale_callback(GLFWwindow* window,
                                       float xscale,
                                       float yscale);

    static void opengl_message_callback(unsigned source,
                                        unsigned type,
                                        unsigned id,
                                        unsigned severity,
                                        int length,
                                        const char* message,
                                        const void* userParam);

   private:
    GLFWwindow* m_Window;
    WindowData m_Data;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_WINDOW_H
