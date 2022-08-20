#pragma once

namespace dd::trace {

    #if defined(DD_DEBUG)
        
        void OnAssertFailure(int line, const char *file);
        void OnAssertFailureV(int line, const char *file, ...);
        
        #define DD_ASSERT(expression) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                dd::trace::OnAssertFailure(__LINE__, __FILE__); \
            } \
        }

        #define DD_ASSERT_PRINT(expression, ...) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                dd::trace::OnAssertFailureV(__LINE__, __FILE__, __VA_ARGS__); \
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
