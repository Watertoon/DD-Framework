 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
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
