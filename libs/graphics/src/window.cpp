// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/event/key_event.hpp"
#include "rurouni/event/mouse_event.hpp"
#include "rurouni/event/window_event.hpp"
#include "rurouni/graphics/key_codes.hpp"
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/mouse_codes.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/math.hpp"

// external
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace rr::graphics {

static bool s_GLFWInitialized = false;

Window::Window(const WindowSpecification& spec,
               std::shared_ptr<event::EventSystem> eventSystem) {
    m_Data.EventSystem = eventSystem;
    m_Data.VSync = spec.VSync;
    m_Data.Fullscreen = spec.Fullscreen;
    m_Data.Maximized = spec.Maximized;
    m_Data.Iconified = false;
    m_Data.Focused = true;
    m_Data.Floating = spec.Floating;
    m_Data.Resizable = spec.Resizable;

    if (!s_GLFWInitialized) {
        // Set GLFW error callback, this is allowed before init
        glfwSetErrorCallback(glfw_error_callback);

        // initialize GLFW
        int success = glfwInit();
        require(success == GLFW_TRUE,
                "GLFW initialization fail! Check log message!");
        s_GLFWInitialized = true;
    }

    // Window Hints
    if (spec.Fullscreen) {
        // Fullscreen mode just takes the whole screen
        // non of these flags actually matter
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    } else {
        // windowed mode
        glfwWindowHint(GLFW_RESIZABLE, spec.Resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, spec.Floating ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_MAXIMIZED, spec.Maximized ? GLFW_TRUE : GLFW_FALSE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // For linux X11 window managers to set floating rules
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "rurouni");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, spec.Title.c_str());

    if (spec.Fullscreen) {
        // window creation fullscreen
        math::ivec2 size =
            spec.Size.value_or(math::ivec2{mode->width, mode->height});
        m_Window = glfwCreateWindow(size.x, size.y, spec.Title.c_str(), monitor,
                                    nullptr);
        require(m_Window, "Window creation failed! Check log message!");
        m_Data.WindowSize = size;
        m_Data.WindowPosition = {0, 0};
    } else {
        // windowed window
        math::ivec2 size;
        math::ivec2 position;

        // set maximium size as default
        glfwGetMonitorWorkarea(monitor, &position.x, &position.y, &size.x,
                               &size.y);

        // set custom size if awailable
        if (spec.Size.has_value()) {
            size = spec.Size.value();
        }

        m_Window = glfwCreateWindow(size.x, size.y, spec.Title.c_str(), nullptr,
                                    nullptr);
        require(m_Window, "Window creation failure! Check GLFW log message!");
        glfwSetWindowPos(m_Window, position.x, position.y);

        m_Data.WindowSize = size;
        m_Data.WindowPosition = position;
    }

    m_Data.Title = spec.Title;

    // ===== Graphics Context =====>
    glfwMakeContextCurrent(m_Window);
    int version = gladLoadGL(glfwGetProcAddress);
    // RR_INFO("GL Version {}.{}", GLAD_VERSION_MAJOR(version),
    //           GLAD_VERSION_MINOR(version));
    api::init();
    // <============================

    // framebuffer size
    glfwGetFramebufferSize(m_Window, &m_Data.FramebufferSize.x,
                           &m_Data.FramebufferSize.y);
    api::set_viewport(m_Data.FramebufferSize);

    // aspect ratio
    m_Data.AspectRatio =
        (float)m_Data.FramebufferSize.x / (float)m_Data.FramebufferSize.y;

    // content scale
    glfwGetMonitorContentScale(monitor, &m_Data.ContentScale.x,
                               &m_Data.ContentScale.y);

    // vsync
    set_vsync(spec.VSync);

    // user pointer for event callbacks
    glfwSetWindowUserPointer(m_Window, &m_Data);

    // set GLFW event callbacks
    glfwSetWindowSizeCallback(m_Window, window_size_callback);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetWindowCloseCallback(m_Window, window_close_callback);
    glfwSetWindowPosCallback(m_Window, window_pos_callback);
    glfwSetWindowFocusCallback(m_Window, window_focus_callback);
    glfwSetWindowMaximizeCallback(m_Window, window_maximize_callback);
    glfwSetWindowIconifyCallback(m_Window, window_iconified_callback);
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSetCharCallback(m_Window, char_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetCursorPosCallback(m_Window, cursor_position_callback);
    glfwSetWindowContentScaleCallback(m_Window, content_scale_callback);
}

Window::~Window() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::update(float dt) {
    if (m_Data.TrackClickTime)
        m_Data.TimeSinceClick += dt;

    if (m_Data.TimeSinceClick >= 1000.0f) {
        m_Data.TimeSinceClick = 0.0f;
        m_Data.TrackClickTime = false;
        m_Data.ClickCount = 0;
    }

    glfwPollEvents();
}

bool Window::is_key_pressed(input::KeyCodes key) const {
    if (m_Data.BlockInput)
        return false;

    auto state = glfwGetKey(m_Window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Window::is_button_pressed(input::MouseCodes button) const {
    if (m_Data.BlockInput)
        return false;

    auto state = glfwGetMouseButton(m_Window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

void Window::swap_buffers() {
    glfwSwapBuffers(m_Window);
}

void Window::block_input(bool block) {
    m_Data.BlockInput = block;
}

void Window::set_vsync(bool enabled) {
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    m_Data.VSync = enabled;
}

bool Window::is_minimized() const {
    return m_Data.Maximized;
}

bool Window::is_iconified() const {
    return m_Data.Iconified;
}

bool Window::is_focused() const {
    return m_Data.Focused;
}

bool Window::has_vsync() const {
    return m_Data.VSync;
}

float Window::get_aspect_ratio() const {
    return m_Data.AspectRatio;
}

GLFWwindow* Window::get_native_window() const {
    return m_Window;
}

const WindowData& Window::get_window_data() const {
    return m_Data;
}

GLFWwindow* Window::get_current_context() const {
    return glfwGetCurrentContext();
}

void Window::make_context_current(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
}

void Window::opengl_message_callback(unsigned source,
                                     unsigned type,
                                     unsigned id,
                                     unsigned severity,
                                     int length,
                                     const char* message,
                                     const void* userParam) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            critical("{}", message);
            return;
        case GL_DEBUG_SEVERITY_MEDIUM:
            error("{}", message);
            return;
        case GL_DEBUG_SEVERITY_LOW:
            warn("{}", message);
            return;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            info("{}", message);
            return;
    }
}

void Window::glfw_error_callback(int error, const char* description) {
    graphics::error("GLFW Error [{}]: {}", error, description);
}

void Window::window_size_callback(GLFWwindow* window, int width, int height) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    math::ivec2 newSize = {width, height};

    auto event =
        std::make_shared<event::WindowResize>(data.WindowSize, newSize);
    data.EventSystem->fire_event(event);

    data.WindowSize = newSize;
}

void Window::framebuffer_size_callback(GLFWwindow* window,
                                       int width,
                                       int height) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    math::ivec2 newSize = {width, height};
    api::set_viewport(newSize);

    auto event = std::make_shared<event::WindowFramebufferResize>(
        data.FramebufferSize, newSize);
    data.EventSystem->fire_event(event);

    data.FramebufferSize = newSize;
}

void Window::window_pos_callback(GLFWwindow* window, int xPos, int yPos) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    math::ivec2 newPosition = {xPos, yPos};

    auto event =
        std::make_shared<event::WindowMove>(data.WindowPosition, newPosition);
    data.EventSystem->fire_event(event);

    data.WindowPosition = newPosition;
}

void Window::window_focus_callback(GLFWwindow* window, int focused) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    if (focused) {
        auto event = std::make_shared<event::WindowFocus>();
        data.EventSystem->fire_event(event);
    } else {
        auto event = std::make_shared<event::WindowUnfocus>();
        data.EventSystem->fire_event(event);
    }

    data.Focused = focused;
}

void Window::window_close_callback(GLFWwindow* window) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    auto event = std::make_shared<event::WindowClose>();
    data.EventSystem->fire_event(event);
}

void Window::window_maximize_callback(GLFWwindow* window, int maximized) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    if (maximized) {
        auto event = std::make_shared<event::WindowMaximize>();
        data.EventSystem->fire_event(event);
    } else {
        auto event = std::make_shared<event::WindowRestore>();
        data.EventSystem->fire_event(event);
    }

    data.Maximized = maximized;
}

void Window::window_iconified_callback(GLFWwindow* window, int iconified) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    if (iconified) {
        auto event = std::make_shared<event::WindowIconified>();
        data.EventSystem->fire_event(event);
    } else {
        auto event = std::make_shared<event::WindowRestore>();
        data.EventSystem->fire_event(event);
    }

    data.Iconified = true;
}

void Window::key_callback(GLFWwindow* window,
                          int key,
                          int scancode,
                          int action,
                          int mods) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        auto event =
            std::make_shared<event::KeyPressed>(key, scancode, mods, false);
        data.EventSystem->fire_event(event);
    } else if (action == GLFW_REPEAT) {
        auto event =
            std::make_shared<event::KeyPressed>(key, scancode, mods, true);
        data.EventSystem->fire_event(event);
    } else if (action == GLFW_RELEASE) {
        auto event = std::make_shared<event::KeyReleased>(key, scancode);
        data.EventSystem->fire_event(event);
    }
}

void Window::char_callback(GLFWwindow* window, unsigned int keycode) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    auto event = std::make_shared<event::CharTyped>(keycode);
    data.EventSystem->fire_event(event);
}

void Window::mouse_button_callback(GLFWwindow* window,
                                   int button,
                                   int action,
                                   int mods) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        auto event = std::make_shared<event::ButtonPressed>(
            button, mods, data.MousePosition, data.ClickCount);
        data.EventSystem->fire_event(event);
    } else if (action == GLFW_RELEASE) {
        auto event =
            std::make_shared<event::ButtonReleased>(button, data.MousePosition);
        data.EventSystem->fire_event(event);
    }
}

void Window::scroll_callback(GLFWwindow* window,
                             double xOffset,
                             double yOffset) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    auto event = std::make_shared<event::MouseScrolled>(xOffset, yOffset);
    data.EventSystem->fire_event(event);
}

void Window::cursor_position_callback(GLFWwindow* window,
                                      double xPos,
                                      double yPos) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    math::vec2 newPosition = {xPos, yPos};

    auto event =
        std::make_shared<event::MouseMoved>(data.MousePosition, newPosition);
    data.EventSystem->fire_event(event);

    data.MousePosition = newPosition;
}

void Window::content_scale_callback(GLFWwindow* window,
                                    float xscale,
                                    float yscale) {
    WindowData& data =
        *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

    math::vec2 newScale = {xscale, yscale};

    auto event = std::make_shared<event::WindowContentScale>(data.ContentScale,
                                                             newScale);
    data.EventSystem->fire_event(event);

    data.ContentScale = newScale;
}

}  // namespace rr::graphics
