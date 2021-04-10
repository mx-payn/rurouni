#ifndef RUROUNI_PLATFORM_MACROS_HPP
#define RUROUNI_PLATFORM_MACROS_HPP

//  --== DEBUG BREAKING ==--
#if defined(RR_PLATFORM_WINDOWS)
#    define RR_DEBUG_BREAK __debugbreak
#else
#    include <signal.h>
#    define RR_DEBUG_BREAK raise(SIGTRAP);
#endif

//  --== FUNCTION SIGNATURE ==--
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#    define RR_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#    define RR_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#    define RR_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#    define RR_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#    define RR_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#    define RR_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#    define RR_FUNC_SIG __func__
#else
#    define RR_FUNC_SIG "RR_FUNC_SIG unknown!"
#endif

#endif //RUROUNI_PLATFORM_MACROS_HPP
