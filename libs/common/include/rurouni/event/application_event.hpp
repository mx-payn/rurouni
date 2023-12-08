#ifndef RR_LIB_EVENT_APPLICATIONEVENT_H
#define RR_LIB_EVENT_APPLICATIONEVENT_H

// rurouni
#include "rurouni/event/event.hpp"

namespace rr::event {

/** application close event. */
class ApplicationClose : public Event {
   public:
    ApplicationClose() : Event(this) {}
};

}  // namespace rr::event

#endif  // !RR_LIB_EVENT_APPLICATIONEVENT_H
