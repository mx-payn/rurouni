#ifndef RR_LIBS_SYSTEM_DEBUG_BREAK
#define RR_LIBS_SYSTEM_DEBUG_BREAK

/** Cross platform macros for debug breaking. */
#if (defined(__debugbreak) || (_MSC_VER))
#define RR_DEBUG_BREAK __debugbreak()
#elif (defined(__breakpoint) || (__ARMCC_VERSION))
#define RR_DEBUGBREAK __breakpoint(42)
#else
#include <signal.h>
#if (defined(SIGTRAP))
#define RR_DEBUG_BREAK raise(SIGTRAP)
#else
#define RR_DEBUG_BREAK raise(SIGABRT)
#endif
#endif

#endif  // !RR_LIBS_SYSTEM_DEBUG_BREAK
