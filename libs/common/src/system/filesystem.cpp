// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/system/filesystem.hpp"

#include <unistd.h>
#include <cstdlib>
#include <filesystem>

namespace rr::system {

bool exists(const Path& path) {
    return std::filesystem::exists(path);
}

Path relative(const Path& path, const Path& base) {
    return std::filesystem::relative(path, base);
}

std::string relative_root_name(const Path& path, const Path& base) {
    Path relative = rr::system::relative(path, base);
    std::string relativeStr = relative.string();

    auto dirPos = relativeStr.find("/");

    // check if relative path is a file or
    // directory tree
    if (dirPos == relativeStr.npos) {
        // relative path is a file
        // return empty string
        // TODO do i want to return empty or the filename?
        return std::string();
    }

    // relative path is a directory tree
    // return the first string value
    return relativeStr.substr(0, dirPos);
}

bool is_directory(const Path& path) {
    return std::filesystem::is_directory(path);
}

bool is_regular_file(const Path& path) {
    return std::filesystem::is_regular_file(path);
}

bool create_directory(const Path& path) {
    return std::filesystem::create_directory(path);
}

bool create_directories(const Path& path) {
    return std::filesystem::create_directories(path);
}

void copy(const Path& from, const Path& to) {
    return std::filesystem::copy(from, to);
}

void copy(const Path& from, const Path& to, copy_options options) {
    return std::filesystem::copy(from, to, options);
}

void rename(const Path& from, const Path& to) {
    return std::filesystem::rename(from, to);
}

Path get_current_executable_path() {
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

    if (len != -1) {
        buffer[len] = '\0';
        return Path(buffer);
    }

    return Path();
}

Path get_app_user_data_dir(const std::string& appName) {
    if (const char* path = std::getenv("XDG_DATA_HOME")) {
        return Path(path) / appName;
    }

    const char* path = std::getenv("HOME");
    return Path(path) / ".local/share" / appName;
}

Path get_app_user_config_dir(const std::string& appName) {
    if (const char* path = std::getenv("XDG_CONFIG_HOME")) {
        return Path(path) / appName;
    }

    const char* path = std::getenv("HOME");
    return Path(path) / ".config" / appName;
}

}  // namespace rr::system
