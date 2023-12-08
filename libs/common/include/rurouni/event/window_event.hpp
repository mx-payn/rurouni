#ifndef RR_LIB_EVENT_WINDOWEVENT_H
#define RR_LIB_EVENT_WINDOWEVENT_H

// rurouni
#include "rurouni/event/event.hpp"
#include "rurouni/math.hpp"

// std
#include <cstdint>
#include <sstream>

namespace rr::event {

/** window close event.
 *
 *  the window received a 'should close' signal. Nothing happens here,
 *  the application has to be shutdown explicitly.
 */
class WindowClose : public Event {
   public:
    WindowClose() : Event(this) {}
};

/** window minimized event. */
class WindowMinimize : public Event {
   public:
    WindowMinimize() : Event(this) {}
};

/** window maximized event. */
class WindowMaximize : public Event {
   public:
    WindowMaximize() : Event(this) {}
};

/** window restored event.
 *
 *  window that was iconified was reopened.
 */
class WindowRestore : public Event {
   public:
    WindowRestore() : Event(this) {}
};

/** window restored event.
 *
 *  window that was open was iconified.
 */
class WindowIconified : public Event {
   public:
    WindowIconified() : Event(this) {}
};

/** window unfocus event. */
class WindowUnfocus : public Event {
   public:
    WindowUnfocus() : Event(this) {}
};

/** window focus event. */
class WindowFocus : public Event {
   public:
    WindowFocus() : Event(this) {}
};

/** the default framebuffer has been resized.
 *
 *  framebuffer 0 (default) has been resized.
 *  size is provided in pixel.
 */
class WindowFramebufferResize : public Event {
   public:
    WindowFramebufferResize(const math::ivec2& oldSize,
                            const math::ivec2& newSize)
        : Event(this), m_OldSize(oldSize), m_NewSize(newSize) {}

    // getter
    const math::ivec2& get_old_size() const { return m_OldSize; }
    const math::ivec2& get_new_size() const { return m_NewSize; }

    // debug
    [[nodiscard]] virtual std::string to_string() const override {
        std::stringstream ss;
        ss << get_name() << " | "
           << "oldSize " << m_OldSize << ", "
           << "newSize " << m_NewSize;
        return ss.str();
    }

   private:
    math::ivec2 m_OldSize;  //!< size before resize
    math::ivec2 m_NewSize;  //!< size after resize
};

/** the window has been resized.
 *
 *  window pane has been reqized.
 *  size is provided in screen coords. this is probably the same as
 *  framebuffer size in px, but may be different!
 */
class WindowResize : public Event {
   public:
    WindowResize(const math::ivec2& oldSize, const math::ivec2& newSize)
        : Event(this), m_OldSize(oldSize), m_NewSize(newSize) {}

    // getter
    const math::ivec2& get_old_size() const { return m_OldSize; }
    const math::ivec2& get_new_size() const { return m_NewSize; }

    // debug
    [[nodiscard]] virtual std::string to_string() const override {
        std::stringstream ss;
        ss << get_name() << " | "
           << "oldSize " << m_OldSize << ", "
           << "newSize " << m_NewSize;
        return ss.str();
    }

   private:
    math::ivec2 m_OldSize;  //!< size before resize
    math::ivec2 m_NewSize;  //!< size after resize
};

/** window pane was moved.
 *
 *  window was moved to different position. size is provided
 *  in screen coordinates.
 */
class WindowMove : public Event {
   public:
    WindowMove(const math::ivec2& oldPosition, const math::ivec2& newPosition)
        : Event(this), m_OldPosition(oldPosition), m_NewPosition(newPosition) {}

    const math::ivec2& get_old_position() const { return m_OldPosition; }
    const math::ivec2& get_new_position() const { return m_NewPosition; }

    [[nodiscard]] virtual std::string to_string() const override {
        std::stringstream ss;
        ss << get_name() << " | "
           << "oldPosition " << m_OldPosition << ", "
           << "newPosition " << m_NewPosition;
        return ss.str();
    }

   private:
    math::ivec2 m_OldPosition;
    math::ivec2 m_NewPosition;
};

/** the content scale of the window has changed.
 *
 *  this is relevant for multi monitor setups, where the window is moved
 *  from a low/regular to high-DPI monitor.
 *  the content scale is the ratio between the current DPI and the platform's
 *  default DPI.
 */
class WindowContentScale : public Event {
   public:
    WindowContentScale(const math::vec2& oldScale, const math::vec2& newScale)
        : Event(this), m_OldScale(oldScale), m_NewScale(newScale) {}

    // getter
    const math::ivec2& get_old_scale() const { return m_OldScale; }
    const math::ivec2& get_new_scale() const { return m_NewScale; }

    // debug
    [[nodiscard]] virtual std::string to_string() const override {
        std::stringstream ss;
        ss << get_name()
           << " | "
              "oldScale "
           << m_OldScale << ", "
           << "newScale " << m_NewScale;
        return ss.str();
    }

   private:
    math::ivec2 m_OldScale;  //!< previous scale
    math::ivec2 m_NewScale;  //!< current scale
};

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_WINDOWEVENT_H
