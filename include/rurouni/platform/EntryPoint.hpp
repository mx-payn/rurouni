#ifndef RUROUNI_ENTRYPOINT_HPP
#define RUROUNI_ENTRYPOINT_HPP

/*!
 * Switches between the EntryPoints for each platform.
 * This enables different entries for e.g. windows, to get the HINSTANCE
 * from the get go.
 *
 * Platform detection is done by CMake and passed by definitions.
 */
#if defined(RR_PLATFORM_LINUX)
#   include "rurouni/platform/linux/linux_EntryPoint.hpp"
#elif defined(RR_PLATFORM_WINDOWS)
#   include "rurouni/platform/win/win_EntryPoint.hpp"
#elif defined(RR_PLATFORM_MACOS)
#   include "rurouni/platform/macos/macos_EntryPoint.hpp"
#else
#   error "At the moment your platform is not supported... Sorry!"
#endif

#endif //RUROUNI_ENTRYPOINT_HPP
