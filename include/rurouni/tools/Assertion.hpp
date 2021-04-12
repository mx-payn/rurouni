#ifndef RUROUNI_ASSERTION_HPP
#define RUROUNI_ASSERTION_HPP

//  --== Includes ==--
#include "rr_export.h"
#include "rurouni/tools/Logger.hpp"
#include "rurouni/platform/platform_macros.hpp"

namespace Rurouni {

    class RR_EXPORT Assertion {
    public:

        /*!
         * An assertion function that logs debug information on failed assertions.
         *
         * @param exp   the expression to be asserted
         * @param desc  a short description about the assertion. will be printed on failure
         * @param line  the line number of the assertion function
         * @param file  the filePath of the assertion function
         * @return  True if provided expression is true, False if expression is false
         */
        static bool AssertFn(bool exp, const std::string& desc, int line, const std::string& file)
        {
            if (exp)
                return true;

            // void call needed to suppress unknown parameters warning
            // NOTE: don't know yet, why those parameters are unused, even though
            //       RR_ENGINE_CRITICAL gets those passed...
            (void) desc;
            (void) line;
            (void) file;

            RR_LOG_ERROR("ASSERTION FAILED -- \"%s\" -- in line [%d] in file [%s]", desc.c_str(), line, file.c_str())
            return false;
        }

    private:
        Assertion() = default;
    };

}
//  --== MACROS ==--
#if defined(RR_ENABLE_ASSERT)
#    define RR_ASSERT_TRUE(exp, description)    \
    if(!::Rurouni::Assertion::AssertFn(     \
                      (int)(exp),          \
                      description,         \
                      __LINE__,            \
                      __FILE__))           \
        { RR_DEBUG_BREAK; }
#    define RR_ASSERT_FALSE(exp, description)    \
    if(!::Rurouni::Assertion::AssertFn(     \
                      (int)(!(exp)),          \
                      description,         \
                      __LINE__,            \
                      __FILE__))           \
        { RR_DEBUG_BREAK }
#    define RR_ASSERT_EQ(exp1, exp2, description)    \
    if(!::Rurouni::Assertion::AssertFn(     \
                      (int)((exp1) == (exp2)),          \
                      description,         \
                      __LINE__,            \
                      __FILE__))           \
        { RR_DEBUG_BREAK }
#    define RR_ASSERT_NOT_EQ(exp1, exp2, description)    \
    if(!::Rurouni::Assertion::AssertFn(     \
                      (int)((exp1) != (exp2)),          \
                      description,         \
                      __LINE__,            \
                      __FILE__))           \
        { RR_DEBUG_BREAK }
#else
#    define RR_ASSERT_TRUE(exp, description)
#    define RR_ASSERT_FALSE(exp, description)
#    define RR_ASSERT_EQ(exp, description)
#    define RR_ASSERT_NOT_EQ(exp, description)
#endif

#endif //RUROUNI_ASSERTION_HPP
