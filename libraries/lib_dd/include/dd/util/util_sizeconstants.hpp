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

namespace dd::util {

    constexpr ALWAYS_INLINE size_t Size1MB  = 0x10'0000;
    constexpr ALWAYS_INLINE size_t Size2MB  = 0x20'0000;
    constexpr ALWAYS_INLINE size_t Size4MB  = 0x40'0000;
    constexpr ALWAYS_INLINE size_t Size8MB  = 0x80'0000;
    constexpr ALWAYS_INLINE size_t Size16MB = 0x100'0000;
    constexpr ALWAYS_INLINE size_t Size32MB = 0x200'0000;
}
