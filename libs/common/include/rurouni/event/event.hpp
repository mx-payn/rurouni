#ifndef RR_LIB_EVENT_EVENT_H
#define RR_LIB_EVENT_EVENT_H

#include <string>
#include <typeinfo>

namespace rr::event {

/** event base class. */
class Event {
   public:
    /** child type agnostic constructor.
     *
     *  a constructor taking the child type as argument to deferr its type.
     *  this enables the event dispatcher to compare generic events with
     *  specific event types to dispatch.
     */
    template <typename T>
    constexpr Event(T*)
        : m_Handled(false),
          m_TypeHash(typeid(T).hash_code()),
          m_Name(typeid(T).name()) {}
    virtual ~Event() = default;

    // getter
    std::size_t get_type_hash() const { return m_TypeHash; }
    bool is_handled() const { return m_Handled; }
    const std::string& get_name() const { return m_Name; }

    // debug
    [[nodiscard]] virtual std::string to_string() const { return m_Name; }

   private:
    bool m_Handled;          //!< status flag for receiving handlers
    std::size_t m_TypeHash;  //!< type hash of the deriving type
    std::string m_Name;      //!< debug identifier
};

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_EVENT_H
