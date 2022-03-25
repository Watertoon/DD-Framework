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

#define DD_UNLIKELY(expression) __builtin_expect((expression), 0)
#define DD_LIKELY(expression)   __builtin_expect((expression), 1)

#define DD_ASSERT(expression) \
{ \
    const auto _temp_result = (expression); \
    if (DD_UNLIKELY(!_temp_result)) { \
        std::cout << __LINE__ << ", " <<  __FILE__ << std::endl;\
    } \
}
