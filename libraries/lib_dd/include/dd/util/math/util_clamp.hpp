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

namespace dd::util::math {

    template <typename T>
    constexpr T Min(T value0, T value1) {
        if (value0 > value1) {
            return value1;
        }
        return value0;
    }

    template <typename T>
    constexpr T Max(T value0, T value1) {
        if (value0 < value1) {
            return value1;
        }
        return value0;
    }

    template <typename T>
    constexpr T Clamp(T value, T min, T max) {
        if (value < min) {
            return min;
        }
        if (max < value) {
            return max;
        }
        return value;
    }
}
