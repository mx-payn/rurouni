// pch
#include "rurouni/pch.hpp"
//-----------------------

// ruroni
#include "rurouni/event/event_system.hpp"

namespace rr::event {

EventSystem::~EventSystem() {
    m_Subscribers.clear();
}

void EventSystem::unsubscribe_from_all(Subscriber* subscriber) {
    auto iter = m_Subscribers.begin();

    // loop whole map and delete subscriber occurances
    while (iter != m_Subscribers.end()) {
        if ((*iter).second == subscriber) {
            iter = m_Subscribers.erase(iter);
        } else {
            iter++;
        }
    }
}

void EventSystem::fire_event(std::shared_ptr<Event> event) {
    // get range for subscribers of events underlying type
    auto range = m_Subscribers.equal_range(event->get_type_hash());

    for (auto iter = range.first; iter != range.second; iter++) {
        (*iter).second->on_event(event);
    }
}

bool EventSystem::is_already_registered(std::size_t typeHash,
                                        Subscriber* subscriber) {
    // get range over typehash
    auto range = m_Subscribers.equal_range(typeHash);

    for (auto iter = range.first; iter != range.second; iter++) {
        if ((*iter).second == subscriber) {
            return true;
        }
    }

    return false;
}

}  // namespace rr::event
