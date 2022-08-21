#pragma once

namespace dd::trace {

    #if defined(DD_DEBUG)
        
        #define DD_ASSERT(expression) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                dd::trace::impl::AbortImpl("Assert failed!!!", __PRETTY_FUNCTION__, __FILE__, __LINE__, dd::trace::ResultAssertFailed, "Failed: %s\n", TOSTRING(expression)); \
            } \
        }

        #define DD_ASSERT_PRINT(expression, ...) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                dd::trace::impl::AbortImpl("Assert failed!!!", __PRETTY_FUNCTION__, __FILE__, __LINE__, dd::trace::ResultAssertFailed, "Failed: %s\n", TOSTRING(expression)); \
            } \
        }

    #else

        #define DD_ASSERT(expression) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
            } \
        }

        #define DD_ASSERT_PRINT(expression, ...) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
            } \
        }

    #endif
}
