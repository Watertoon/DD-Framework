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
 *  if not, see <https://www.gnu.org/licenses/>
 */
#pragma once

namespace dd {

    namespace result {

        constexpr ALWAYS_INLINE u32 ModuleBits      = 9;
        constexpr ALWAYS_INLINE u32 DescriptionBits = 13;
        constexpr ALWAYS_INLINE u32 ReserveBits     = 9;

        constexpr ALWAYS_INLINE GetModule(Result result) {
            return result & ModuleBits;
        }

        constexpr ALWAYS_INLINE GetDescription(Result result) {
            return (result >> ModuleBits) & DescriptionBits;
        }

        /*NO_RETURN void OnUnhandledResult(Result result) {
            if (result.IsSuccess() == true) {
                ::ExitProcess(1);
            }
            dd::trace::Abort("", "", "", result, "");
        }

        NO_RETURN u32 ConvertInvalidImpl(Result result) {
            OnUnhandledResult(result);
            return result;
        }*/
    }

    constexpr ALWAYS_INLINE Result ResultSuccess = 0;

    #define RESULT_RETURN_IF(expression, return_result) \
    { \
        if ((expression) == true) { \
            return return_result; \
        } \
    }
    #define RESULT_RETURN_UNLESS(expression, return_result)  \
    { \
        if ((expression) == false) { \
            return return_result; \
        } \
    }

    #define RESULT_ABORT_UNLESS(expression, expected_result)  \
    { \
        const Result result = expression; \
        if (DD_UNLIKELY(result != expected_result)) { \
            dd::trace::impl::AbortImpl(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, result, "Failed: %s\n  Module: %d\n  Description %d\n", TOSTRING(expression), dd::result::GetModule(result), dd::result::GetDescription(result)); \
        } \
    }

    #define RESULT_RANGE_ABORT_UNLESS(expression, expected_result_range) \
        { \
            const Result result = (expression); \
            if (result < Result##expected_result_range##Start || Result##expected_result_range##End < result) { \
                dd::trace::impl::AbortImpl(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, result, "Failed: %s\n  Module: %d\n  Description %d\n", TOSTRING(expression), dd::result::GetModule(result), dd::result::GetDescription(result)); \
            } \
        }

    #define DECLARE_RESULT_MODULE(module_num) \
        constexpr ALWAYS_INLINE u32 ResultModule = module_num;

    #define DECLARE_RESULT(result_name, description) \
        constexpr ALWAYS_INLINE u32 Result##result_name =  ((description & dd::result::DescriptionBits) << dd::result::ModuleBits) | (ResultModule & dd::result::ModuleBits);

    #define DECLARE_RESULT_RANGE(result_name, description_start, description_end) \
        constexpr ALWAYS_INLINE u32 Result##result_name##Start =  ((description_start & dd::result::DescriptionBits) << dd::result::ModuleBits) | (ResultModule & dd::result::ModuleBits); \
        constexpr ALWAYS_INLINE u32 Result##result_name##End   =  ((description_end & dd::result::DescriptionBits) << dd::result::ModuleBits) | (ResultModule & dd::result::ModuleBits);
}
