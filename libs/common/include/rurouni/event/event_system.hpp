#ifndef RR_LIB_EVENT_EVENT_SYSTEM_H
#define RR_LIB_EVENT_EVENT_SYSTEM_H

// rurouni
#include "rurouni/event/event.hpp"

// std
#include <cstddef>
#include <map>
#include <memory>
#include <utility>

namespace rr::event {

/** abstract event receiver. */
class Subscriber {
   public:
    Subscriber() = default;
    virtual ~Subscriber() = default;

    /** receiving event function.
     *
     *  use dispatch() function to dispatch event to handling
     *  functions.
     */
    virtual void on_event(std::shared_ptr<Event> event) = 0;
};

/** Event subscribing and forwarding. */
class EventSystem {
   public:
    EventSystem() = default;
    ~EventSystem();

    /** event type subscription.
     *
     *  saves a RAW pointer to subscriber type. Be aware that
     *  no runtime checks for validity of the cubscriber pointer are
     *  done, so always use unsubscribe_from_all() in the destructor.
     */
    template <typename T>
    void subscribe(Subscriber* subscriber) {
        std::size_t typeHash = typeid(T).hash_code();

        if (is_already_registered(typeHash, subscriber))
            return;

        m_Subscribers.insert(std::make_pair(typeHash, subscriber));
    }

    /** unsubscribe from specific type. */
    template <typename T>
    void unsubscribe_from(Subscriber* subscriber) {
        std::size_t typeHash = typeid(T).hash_code();
        auto range = m_Subscribers.equal_range(typeHash);

        for (auto iter = range.first; iter != range.second; iter++) {
            if ((*iter).second == subscriber) {
                iter = m_Subscribers.erase(iter);
                break;
            }
        }
    }

    /** unsubscribe from all event types. */
    void unsubscribe_from_all(Subscriber* subscriber);

    /** push given event to all subscribed objects. */
    void fire_event(std::shared_ptr<Event> event);

   private:
    /** util function to check subscriber map for present subscriber. */
    bool is_already_registered(std::size_t typeHash, Subscriber* subscriber);

   private:
    std::multimap<std::size_t, Subscriber*> m_Subscribers;  //!< subscriber map
};

/** event type agnostic function parameter forwarder.
 *
 *  dont' know what that means, but sounds cool.
 *
 *  takes a generic Event pointer and a function with derived
 *  Event parameter. dispatches Event to function, if the typeid
 *  check succeeds. else skips funtion call (wrong parameter type).
 *
 *  see Event for details for typeid check implementation.
 */
template <typename T, typename F>
static void dispatch(std::shared_ptr<Event> event, const F& func) {
    if (event->is_handled())
        return;

    if (typeid(T).hash_code() == event->get_type_hash()) {
        func(std::static_pointer_cast<T>(event));
    }
}

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_EVENT_SYSTEM_H
