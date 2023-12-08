// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/types/uuid.hpp"

// external
#include <uuid.h>

namespace rr {

UUID::UUID() {
    m_UUID = uuids::uuid();
}

UUID::UUID(const std::string& str) {
    auto maybe_uuid = uuids::uuid::from_string(str);
    if (maybe_uuid.has_value()) {
        m_UUID = maybe_uuid.value();
    } else {
        m_UUID = uuids::uuid();
    }
}

void UUID::generate() {
    // creating std random generator
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    // uuid random generator
    uuids::uuid_random_generator gen{generator};

    m_UUID = gen();
}

std::string UUID::to_string() const {
    return uuids::to_string(m_UUID);
}

bool UUID::from_string(const std::string& str) {
    auto maybe_uuid = uuids::uuid::from_string(str);
    if (maybe_uuid.has_value()) {
        m_UUID = maybe_uuid.value();
    } else {
        m_UUID = uuids::uuid();
        return false;
    }

    return true;
}

bool UUID::is_null() const {
    return m_UUID.is_nil();
}

}  // namespace rr
