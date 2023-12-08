#ifndef RR_LIB_EVENT_KEY_EVENT_H
#define RR_LIB_EVENT_KEY_EVENT_H

// rurouni
#include "rurouni/event/event.hpp"

// std
#include <cstdint>
#include <sstream>

namespace rr::event {

/** event for key presses. */
class KeyPressed : public Event {
   public:
    KeyPressed(int32_t keycode,
               int32_t scancode,
               int32_t mods,
               bool isRepeat = false)
        : Event(this),
          m_Keycode(keycode),
          m_Scancode(scancode),
          m_Mods(mods),
          m_IsRepeat(isRepeat) {}

    // getter
    int32_t get_keycode() const { return m_Keycode; }
    int32_t get_scancode() const { return m_Scancode; }
    int32_t get_modifiers() const { return m_Mods; }
    bool is_repeated() const { return m_IsRepeat; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << get_name() << " | "
           << "keycode(" << m_Keycode << "), "
           << "scancode(" << m_Scancode << "), "
           << "modifiers(" << m_Mods << "), "
           << "repeated(" << m_IsRepeat << ")";
        return ss.str();
    }

   private:
    int32_t m_Keycode;   //!< system dependent identifier
    int32_t m_Scancode;  //!< system independent identifier
    int32_t m_Mods;      //!< modifier mask (super, ctrl, alt, ...)
    bool m_IsRepeat;     //!< keypress repeated flag
};

/** event for key releases. */
class KeyReleased : public Event {
   public:
    KeyReleased(int32_t keycode, int32_t scancode)
        : Event(this), m_Keycode(keycode), m_Scancode(scancode) {}

    // getter
    int32_t get_keycode() const { return m_Keycode; }
    int32_t get_scancode() const { return m_Scancode; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << get_name() << " | "
           << "keycode(" << m_Keycode << "), "
           << "scancode(" << m_Scancode << ")";
        return ss.str();
    }

   private:
    int32_t m_Keycode;   //!< system dependent identifier
    int32_t m_Scancode;  //!< system independent identifier
};

/** event for key presses producing a character. */
class CharTyped : public Event {
   public:
    CharTyped(int32_t keycode) : Event(this), m_Keycode(keycode) {}

    // getter
    int32_t get_keycode() const { return m_Keycode; }

    // debug
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << get_name() << " | "
           << "keycode(" << m_Keycode << ")";
        return ss.str();
    }

   private:
    int32_t m_Keycode;  //!< system dependent identifier
};

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_KEY_EVENT_H
