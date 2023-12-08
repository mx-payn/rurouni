#ifndef RR_LIBS_COMMON_TYPES_UUID_H
#define RR_LIBS_COMMON_TYPES_UUID_H

// external
#include <uuid.h>

// std
#include <string>

namespace rr {

/** a wrapper around stduuid.
 *
 *  provides functions to construct and generate uuids.
 */
class UUID {
   public:
    /** constructs a null uuid.
     *
     *  null uuids have the form "00000000-0000-0000-0000-000000000000"
     */
    UUID();
    /** constructs a uuid from string
     *
     *  tries to parse the given string into a uuid. If parsing fails
     *  it constructs a null uuid.
     *  null uuids have the form "00000000-0000-0000-0000-000000000000"
     */
    UUID(const std::string& str);
    ~UUID() = default;

    /** comparing two uuids for hashing */
    friend bool operator==(const UUID& lhs, const UUID& rhs) {
        return lhs.m_UUID == rhs.m_UUID;
    }

    /** generates and overwrites the uuid member */
    void generate();
    /** creates uuid string */
    std::string to_string() const;
    /** tries to parse uuid from a string. If parsing fails, the
     *  uuid member is set to null uuid.
     *  \param str the uuid as string
     *  \returns true on success, false on failure.
     */
    bool from_string(const std::string& str);
    /** checks whether uuid has null value
     *
     *  null uuids have the form "00000000-0000-0000-0000-000000000000"
     *  \returns true if uuid has null form, false if not null form.
     */
    bool is_null() const;

    /** static uuid generator function.
     *
     *  constructs and returns a new uuid object and generates
     *  its uuid value.
     *  \returns a populated UUID object.
     */
    static UUID create() {
        UUID id;
        id.generate();
        return id;
    }

   private:
    friend class std::hash<UUID>;  //!< for std::hash
    uuids::uuid m_UUID;            //!< stduuid member
};

}  // namespace rr

/** defining std::hash behaviour for usage as map/unordered_map key */
template <>
struct std::hash<rr::UUID> {
    std::size_t operator()(const rr::UUID& k) const {
        auto hasher = std::hash<uuids::uuid>{};
        return hasher(k.m_UUID);
    }
};

#endif  // !RR_LIBS_COMMON_TYPES_UUID_H
