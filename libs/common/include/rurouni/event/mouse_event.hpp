#ifndef RR_LIB_EVENT_MOUSE_EVENT_H
#define RR_LIB_EVENT_MOUSE_EVENT_H

// ruronui
#include "rurouni/event/event.hpp"
#include "rurouni/math.hpp"

// std
#include <cstdint>
#include <sstream>

namespace rr::event {

/** mouse movement event. */
class MouseMoved : public Event {
   public:
    MouseMoved(const math::vec2& oldPosition, const math::vec2& newPosition)
        : Event(this), m_OldPosition(oldPosition), m_NewPosition(newPosition) {}

    // getter
    const math::vec2& get_old_position() const { return m_OldPosition; }
    const math::vec2& get_new_position() const { return m_NewPosition; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;

        ss << get_name() << " | "
           << "oldPos " << m_OldPosition << ", "
           << "newPos " << m_NewPosition;

        return ss.str();
    }

   private:
    math::vec2 m_OldPosition;  //!< position before movement
    math::vec2 m_NewPosition;  //!< position after movement
};

/** mouse whell scroll event. */
class MouseScrolled : public Event {
   public:
    MouseScrolled(float xOffset, float yOffset)
        : Event(this), m_XOffset(xOffset), m_YOffset(yOffset) {}

    // getter
    float get_x_offset() const { return m_XOffset; }
    float get_y_offset() const { return m_YOffset; }

    // setter
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;

        ss << get_name() << " | "
           << "offset(x: " << m_XOffset << ", y: " << m_YOffset << ")";

        return ss.str();
    }

   private:
    float m_XOffset;  //!< scroll amount horizontally
    float m_YOffset;  //!< scroll amount vertically
};

/** mouse button pressed event. */
class ButtonPressed : public Event {
   public:
    ButtonPressed(int32_t button,
                  int32_t mods,
                  const math::vec2& mousePosition,
                  int32_t clicks = 1)
        : Event(this),
          m_Button(button),
          m_Mods(mods),
          m_MousePosition(mousePosition),
          m_Clicks(clicks) {}

    // getter
    int32_t get_button() const { return m_Button; }
    int32_t get_modifiers() const { return m_Mods; }
    int32_t get_clicks() const { return m_Clicks; }
    const math::vec2& get_mouse_position() const { return m_MousePosition; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;

        ss << get_name() << " | "
           << "button(" << m_Button << "), "
           << "modifiers(" << m_Mods << "), "
           << "clicks(" << m_Clicks << "), "
           << "mousePos " << m_MousePosition;

        return ss.str();
    }

   private:
    int32_t m_Button;            //!< the button code
    int32_t m_Mods;              //!< modifier mask (super, ctrl, alt, ...)
    math::vec2 m_MousePosition;  //!< mouse position at click time
    int32_t m_Clicks;            //!< number of successive clicks
};

/** mouse button released event. */
class ButtonReleased : public Event {
   public:
    ButtonReleased(int32_t button, const math::vec2& mousePosition)
        : Event(this), m_Button(button), m_MousePosition(mousePosition) {}

    // getter
    int32_t get_button() const { return m_Button; }
    const math::vec2& get_mouse_position() const { return m_MousePosition; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;

        ss << get_name() << " | "
           << "button(" << m_Button << "), "
           << "mousePos " << m_MousePosition;

        return ss.str();
    }

   private:
    int32_t m_Button;            //!< button code
    math::vec2 m_MousePosition;  //!< mouse position at time of release
};

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_MOUSE_EVENT_H
