#ifndef RR_LIBS_COMMON_SYSTEM_FILESYSTEM_H
#define RR_LIBS_COMMON_SYSTEM_FILESYSTEM_H

// std
#include <filesystem>

namespace rr::system {

typedef std::filesystem::path Path;

typedef std::filesystem::copy_options copy_options;
typedef std::filesystem::directory_iterator directory_iterator;
typedef std::filesystem::directory_entry directory_entry;

/** checks whether a path leads to a valid file or directory */
bool exists(const Path& path);
/** checks whether a path leads to a directory */
bool is_directory(const Path& path);
/** checks whether a path leads to a file */
bool is_regular_file(const Path& path);

/** returns path with base cut off from the beginning */
Path relative(const Path& path, const Path& base);
/** returns first entry of path with base cut off from the beginning */
std::string relative_root_name(const Path& path, const Path& base);

/** creates the directory at given path (NO CHILD CREATE!) */
bool create_directory(const Path& path);
/** creates the directory at given path AND all children leading to it */
bool create_directories(const Path& path);

/** copies contents of source to target path */
void copy(const Path& from, const Path& to);
/** copies contents of source to target path */
void copy(const Path& from, const Path& to, copy_options options);

}  // namespace rr::system

/////////////////////////////////////////////////////////////////
/////////////////////  fmt format  //////////////////////////////
/////////////////////////////////////////////////////////////////
#include <fmt/format.h>

template <>
struct fmt::formatter<rr::system::Path> : fmt::formatter<std::string> {
    auto format(rr::system::Path path, format_context& ctx) const
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", path.string());
    }
};

#endif  // !RR_LIBS_COMMON_SYSTEM_FILESYSTEM_H
