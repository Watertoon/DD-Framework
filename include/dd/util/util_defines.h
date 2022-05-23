 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

#define ALWAYS_INLINE __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#define NO_CONSTANT_PROPAGATION __attribute__((optimize("-fno-ipa-cp")))

#define DD_UNLIKELY(expression) __builtin_expect((expression), 0)
#define DD_LIKELY(expression)   __builtin_expect((expression), 1)

#if defined(DD_DEBUG)

    #define DD_ASSERT(expression) \
    { \
        const auto _temp_result = (expression); \
        if (DD_UNLIKELY(!_temp_result)) { \
            char buffer[0x200] = { '\0' }; \
            std::snprintf(buffer, sizeof(buffer), "%d, %s", __LINE__, __FILE__); \
            ::puts(buffer);\
        } \
    }

#else

    #define DD_ASSERT(expression) \
    { \
        const auto _temp_result = (expression); \
        if (DD_UNLIKELY(!_temp_result)) { \
        } \
    }

#endif

#define DD_ASSERT_PRINT(expression, ...) \
{ \
    const auto _temp_result = (expression); \
    if (DD_UNLIKELY(!_temp_result)) { \
        char buffer0[0x100] = { '\0' }; \
        char buffer1[0x200] = { '\0' }; \
        std::snprintf(buffer0, sizeof(buffer0), __VA_ARGS__); \
        std::snprintf(buffer1, sizeof(buffer1), "%s/n%d, %s",  buffer0, __LINE__, __FILE__); \
        ::puts(buffer1);\
    } \
}
